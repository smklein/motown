// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <list>

#include "base/bind_helpers.h"
#include "base/logging.h"
#include "mojo/public/cpp/application/connect.h"
#include "mojo/services/media/audio/interfaces/audio_server.mojom.h"
#include "mojo/services/media/audio/interfaces/audio_track.mojom.h"
#include "services/media/factory_service/audio_track_controller.h"
#include "services/media/framework_mojo/mojo_type_conversions.h"

namespace mojo {
namespace media {

AudioTrackController::AudioTrackController(const String& url,
                                           ApplicationImpl* app) {
  // TODO(dalesat): Handle connection errors.
  DCHECK(app);

  AudioServerPtr audio_server;
  ConnectToService(app->shell(), url, GetProxy(&audio_server));
  audio_server->CreateTrack(GetProxy(&audio_track_));
}

AudioTrackController::~AudioTrackController() {}

void AudioTrackController::GetSupportedMediaTypes(
    const GetSupportedMediaTypesCallback& callback) {
  // Query the track's format capabilities.
  audio_track_->Describe([this, callback](AudioTrackDescriptorPtr descriptor) {
    callback(descriptor->supported_media_types.To<std::unique_ptr<
                 std::vector<std::unique_ptr<media::StreamTypeSet>>>>());
  });
}

void AudioTrackController::Configure(
    const std::unique_ptr<StreamType>& stream_type,
    const ConfigureCallback& callback) {
  AudioTrackConfigurationPtr config = AudioTrackConfiguration::New();
  config->media_type = MediaType::From(stream_type);

  MediaConsumerPtr consumer;
  audio_track_->Configure(config.Pass(), GetProxy(&consumer));

  RateControlPtr rate_control;
  audio_track_->GetRateControl(GetProxy(&rate_control));

  callback(consumer.Pass(), rate_control.Pass());
}

}  // namespace media
}  // namespace mojo
