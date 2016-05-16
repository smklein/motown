// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_MEDIA_FACTORY_MEDIA_PLAYER_IMPL_H_
#define MOJO_SERVICES_MEDIA_FACTORY_MEDIA_PLAYER_IMPL_H_

#include <limits>
#include <vector>

#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/services/media/common/interfaces/media_transport.mojom.h"
#include "mojo/services/media/control/interfaces/media_factory.mojom.h"
#include "mojo/services/media/core/interfaces/seeking_reader.mojom.h"
#include "services/media/factory_service/factory_service.h"
#include "services/media/factory_service/mojo_publisher.h"

namespace mojo {
namespace media {

// Mojo agent that renders streams from an origin specified by URL.
class MediaPlayerImpl : public MediaFactoryService::Product<MediaPlayer>,
                        public MediaPlayer {
 public:
  static std::shared_ptr<MediaPlayerImpl> Create(
      InterfaceHandle<SeekingReader> reader,
      InterfaceRequest<MediaPlayer> request,
      MediaFactoryService* owner);

  ~MediaPlayerImpl() override;

  // MediaPlayer implementation.
  void GetStatus(uint64_t version_last_seen,
                 const GetStatusCallback& callback) override;

  void Play() override;

  void Pause() override;

  void Seek(int64_t position) override;

 private:
  const int64_t kNotSeeking = std::numeric_limits<int64_t>::max();

  // Internal state.
  enum class State {
    kWaiting,  // Waiting for some work to complete.
    kPaused,
    kWaitingForSinksToPlay,
    kPlaying,
    kWaitingForSinksToPause
  };

  // For matching sink states.
  enum class SinkState {
    kPaused,
    kPlaying,
    kEnded,
    kPausedOrEnded,
    kPlayingOrEnded
  };

  struct Stream {
    Stream(size_t index, MediaTypePtr media_type);
    ~Stream();
    size_t index_;
    bool enabled_ = false;
    MediaState state_ = MediaState::UNPREPARED;
    MediaTypePtr media_type_;
    MediaTypeConverterPtr decoder_;
    MediaSinkPtr sink_;
    MediaProducerPtr encoded_producer_;
    MediaProducerPtr decoded_producer_;
  };

  MediaPlayerImpl(InterfaceHandle<SeekingReader> reader,
                  InterfaceRequest<MediaPlayer> request,
                  MediaFactoryService* owner);

  // Takes action based on current state.
  void Update();

  // Handles seeking in paused state.
  void WhenPausedAndSeeking();

  // Handles seeking in paused state with flushed pipeline.
  void WhenFlushedAndSeeking();

  // Tells the sinks to change state.
  void ChangeSinkStates(MediaState media_state);

  // Determines if all the enabled sinks have the specified state.
  bool AllSinksAre(SinkState sink_state);

  // Sets the reported_media_state_ field, calling StatusUpdated as needed.
  void SetReportedMediaState(MediaState media_state);

  // Prepares a stream.
  void PrepareStream(Stream* stream,
                     const String& url,
                     const std::function<void()>& callback);

  // Creates a sink for a stream.
  void CreateSink(Stream* stream,
                  const MediaTypePtr& input_media_type,
                  const String& url,
                  const std::function<void()>& callback);

  // Handles a metadata update from the demux. When called with the default
  // argument values, initiates demux metadata updates.
  void HandleDemuxMetadataUpdates(
      uint64_t version = MediaDemux::kInitialMetadata,
      MediaMetadataPtr metadata = nullptr);

  // Handles a status update from a sink. When called with the default
  // argument values, initiates sink status updates.
  void HandleSinkStatusUpdates(Stream* stream,
                               uint64_t version = MediaSink::kInitialStatus,
                               MediaSinkStatusPtr status = nullptr);

  MediaFactoryPtr factory_;
  MediaDemuxPtr demux_;
  std::vector<std::unique_ptr<Stream>> streams_;
  State state_ = State::kWaiting;
  bool flushed_ = true;
  MediaState reported_media_state_ = MediaState::UNPREPARED;
  MediaState target_state_ = MediaState::PAUSED;
  int64_t target_position_ = kNotSeeking;
  TimelineTransformPtr transform_;
  MediaMetadataPtr metadata_;
  MojoPublisher<GetStatusCallback> status_publisher_;
};

}  // namespace media
}  // namespace mojo

#endif  // MOJO_SERVICES_MEDIA_FACTORY_MEDIA_PLAYER_IMPL_H_
