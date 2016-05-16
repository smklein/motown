// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "mojo/services/media/common/cpp/linear_transform.h"
#include "mojo/services/media/common/cpp/local_time.h"
#include "services/media/factory_service/media_sink_impl.h"
#include "services/media/framework/util/conversion_pipeline_builder.h"
#include "services/media/framework_mojo/mojo_type_conversions.h"

namespace mojo {
namespace media {

// static
std::shared_ptr<MediaSinkImpl> MediaSinkImpl::Create(
    const String& destination_url,
    MediaTypePtr media_type,
    InterfaceRequest<MediaSink> request,
    MediaFactoryService* owner) {
  return std::shared_ptr<MediaSinkImpl>(new MediaSinkImpl(
      destination_url, media_type.Pass(), request.Pass(), owner));
}

MediaSinkImpl::MediaSinkImpl(const String& destination_url,
                             MediaTypePtr media_type,
                             InterfaceRequest<MediaSink> request,
                             MediaFactoryService* owner)
    : MediaFactoryService::Product<MediaSink>(this, request.Pass(), owner),
      consumer_(MojoConsumer::Create()),
      producer_(MojoProducer::Create()) {
  DCHECK(destination_url);
  DCHECK(media_type);

  status_publisher_.SetCallbackRunner(
      [this](const GetStatusCallback& callback, uint64_t version) {
        MediaSinkStatusPtr status = MediaSinkStatus::New();
        status->state = (producer_state_ == MediaState::PAUSED && rate_ != 0.0)
                            ? MediaState::PLAYING
                            : producer_state_;
        status->timeline_transform = status_transform_.Clone();
        callback.Run(version, status.Pass());
      });

  PartRef consumer_ref = graph_.Add(consumer_);
  PartRef producer_ref = graph_.Add(producer_);

  consumer_->SetPrimeRequestedCallback(
      [this](const MediaConsumer::PrimeCallback& callback) {
        ready_.When([this, callback]() {
          DCHECK(producer_);
          producer_->PrimeConnection(callback);
        });
      });
  consumer_->SetFlushRequestedCallback(
      [this, consumer_ref](const MediaConsumer::FlushCallback& callback) {
        ready_.When([this, consumer_ref, callback]() {
          DCHECK(producer_);
          graph_.FlushOutput(consumer_ref.output());
          producer_->FlushConnection(callback);
          flushed_ = true;
        });
      });

  producer_->SetStatusCallback([this](MediaState state) {
    producer_state_ = state;
    status_publisher_.SendUpdates();
    if (state == MediaState::ENDED) {
      Pause();
    }
  });

  // TODO(dalesat): Temporary, remove.
  if (destination_url == "nowhere") {
    // Throwing away the content.
    graph_.ConnectParts(consumer_ref, producer_ref);
    graph_.Prepare();
    ready_.Occur();
    return;
  }

  RCHECK(destination_url == "mojo:audio_server");

  // TODO(dalesat): Once we have c++14, get rid of this shared pointer hack.
  std::shared_ptr<StreamType> captured_stream_type(
      media_type.To<std::unique_ptr<StreamType>>().release());

  // An AudioTrackController knows how to talk to an audio track, interrogating
  // it for supported stream types and configuring it for the chosen stream
  // type.
  controller_.reset(new AudioTrackController(destination_url, app()));

  controller_->GetSupportedMediaTypes([this, consumer_ref, producer_ref,
                                       captured_stream_type](
      std::unique_ptr<std::vector<std::unique_ptr<StreamTypeSet>>>
          supported_stream_types) {
    std::unique_ptr<StreamType> producer_stream_type;

    // Add transforms to the pipeline to convert from stream_type to a
    // type supported by the track.
    OutputRef out = consumer_ref.output();
    bool result =
        BuildConversionPipeline(*captured_stream_type, *supported_stream_types,
                                &graph_, &out, &producer_stream_type);
    if (!result) {
      // Failed to build conversion pipeline.
      producer_state_ = MediaState::FAULT;
      status_publisher_.SendUpdates();
      return;
    }

    graph_.ConnectOutputToPart(out, producer_ref);

    if (producer_stream_type->medium() == StreamType::Medium::kAudio) {
      frames_per_second_ = producer_stream_type->audio()->frames_per_second();
    } else {
      // Unsupported producer stream type.
      LOG(ERROR) << "unsupported producer stream type";
      abort();
    }

    controller_->Configure(
        std::move(producer_stream_type),
        [this](MediaConsumerPtr consumer, RateControlPtr rate_control) {
          DCHECK(consumer);
          DCHECK(rate_control);
          rate_control_ = rate_control.Pass();
          producer_->Connect(consumer.Pass(), [this]() {
            graph_.Prepare();
            ready_.Occur();
            MaybeSetRate();
          });
        });
  });
}

MediaSinkImpl::~MediaSinkImpl() {}

void MediaSinkImpl::GetConsumer(InterfaceRequest<MediaConsumer> consumer) {
  consumer_->AddBinding(consumer.Pass());
}

void MediaSinkImpl::GetStatus(uint64_t version_last_seen,
                              const GetStatusCallback& callback) {
  status_publisher_.Get(version_last_seen, callback);
}

void MediaSinkImpl::Play() {
  target_rate_ = 1.0;
  MaybeSetRate();
}

void MediaSinkImpl::Pause() {
  target_rate_ = 0.0;
  MaybeSetRate();
}

void MediaSinkImpl::MaybeSetRate() {
  if (producer_state_ < MediaState::PAUSED || rate_ == target_rate_) {
    return;
  }

  if (!rate_control_) {
    rate_ = target_rate_;
    status_publisher_.SendUpdates();
    return;
  }

  // Desired rate in frames per second.
  LinearTransform::Ratio rate_frames_per_second(
      static_cast<uint32_t>(frames_per_second_ * target_rate_), 1);

  // Local time rate in seconds_per_tick.
  LinearTransform::Ratio local_seconds_per_tick(LocalDuration::period::num,
                                                LocalDuration::period::den);

  // Desired rate in frames per local tick.
  LinearTransform::Ratio rate_frames_per_tick;
  bool success = LinearTransform::Ratio::Compose(
      local_seconds_per_tick, rate_frames_per_second, &rate_frames_per_tick);
  DCHECK(success)
      << "LinearTransform::Ratio::Compose reports loss of precision";

  // TODO(dalesat): start_local_time should be supplied via the mojo interface.
  // For now, it's hard-coded to be 30ms in the future.
  // The local time when we want the rate to change.
  int64_t start_local_time =
      (LocalClock::now().time_since_epoch() + std::chrono::milliseconds(30))
          .count();

  // The media time corresponding to start_local_time.
  int64_t start_media_time;
  if (flushed_ && producer_->GetFirstPtsSinceFlush() != Packet::kUnknownPts) {
    // We're getting started initially or after a flush/prime, so the media
    // time corresponding to start_local_time should be the PTS of
    // the first packet.
    start_media_time = producer_->GetFirstPtsSinceFlush();
  } else {
    // We're resuming, so the media time corresponding to start_local_time can
    // be calculated using the existing transform.
    success =
        transform_.DoForwardTransform(start_local_time, &start_media_time);
    DCHECK(success)
        << "LinearTransform::DoForwardTransform reports loss of precision";
  }

  flushed_ = false;

  // Update the transform.
  transform_ =
      LinearTransform(start_local_time, rate_frames_per_tick, start_media_time);

  // Set the rate.
  TimelineQuadPtr rate_quad = TimelineQuad::New();
  rate_quad->reference_offset = start_media_time;
  rate_quad->target_offset = start_local_time;
  rate_quad->reference_delta = rate_frames_per_tick.numerator;
  rate_quad->target_delta = rate_frames_per_tick.denominator;

  rate_control_->SetCurrentQuad(rate_quad.Pass());

  // Get the frame rate in frames per local tick.
  LinearTransform::Ratio frame_rate_frames_per_second(frames_per_second_, 1);
  LinearTransform::Ratio frame_rate_frames_per_tick;
  success = LinearTransform::Ratio::Compose(local_seconds_per_tick,
                                            frame_rate_frames_per_second,
                                            &frame_rate_frames_per_tick);
  DCHECK(success)
      << "LinearTransform::Ratio::Compose reports loss of precision";

  // Create a LinearTransform to translate from presentation units to
  // local duration units.
  LinearTransform local_to_presentation(0, frame_rate_frames_per_tick, 0);

  status_transform_ = TimelineTransform::New();
  status_transform_->quad = TimelineQuad::New();

  // Translate the current transform quad so the presentation time units
  // are the same as the local time units.
  success = local_to_presentation.DoReverseTransform(
      start_media_time, &status_transform_->quad->reference_offset);
  DCHECK(success)
      << "LinearTransform::DoReverseTransform reports loss of precision";
  status_transform_->quad->target_offset = start_local_time;
  int64_t presentation_delta;
  success = local_to_presentation.DoReverseTransform(
      static_cast<int64_t>(rate_frames_per_tick.numerator),
      &presentation_delta);
  DCHECK(success)
      << "LinearTransform::DoReverseTransform reports loss of precision";
  status_transform_->quad->reference_delta =
      static_cast<uint32_t>(presentation_delta);
  status_transform_->quad->target_delta = rate_frames_per_tick.denominator;
  LinearTransform::Ratio::Reduce(&status_transform_->quad->reference_delta,
                                 &status_transform_->quad->target_delta);

  rate_ = target_rate_;
  status_publisher_.SendUpdates();
}

}  // namespace media
}  // namespace mojo
