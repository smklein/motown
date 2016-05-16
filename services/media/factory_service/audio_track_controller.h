// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_MEDIA_FACTORY_SERVICE_AUDIO_TRACK_CONTROLLER_H_
#define SERVICES_MEDIA_FACTORY_SERVICE_AUDIO_TRACK_CONTROLLER_H_

#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/services/media/audio/interfaces/audio_track.mojom.h"
#include "mojo/services/media/common/interfaces/media_transport.mojom.h"
#include "services/media/framework/types/stream_type.h"

namespace mojo {
namespace media {

// Controls an audio track.
class AudioTrackController {
 public:
  using GetSupportedMediaTypesCallback = std::function<void(
      std::unique_ptr<std::vector<std::unique_ptr<StreamTypeSet>>>)>;
  using ConfigureCallback =
      std::function<void(MediaConsumerPtr, RateControlPtr)>;

  AudioTrackController(const String& url, ApplicationImpl* app);

  ~AudioTrackController();

  // Gets the media types supported by the audio track.
  void GetSupportedMediaTypes(const GetSupportedMediaTypesCallback& callback);

  // Configures the controller.
  void Configure(const std::unique_ptr<StreamType>& stream_type,
                 const ConfigureCallback& callback);

 private:
  AudioTrackPtr audio_track_;
};

}  // namespace media
}  // namespace mojo

#endif  // SERVICES_MEDIA_FACTORY_SERVICE_AUDIO_TRACK_CONTROLLER_H_
