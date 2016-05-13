// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_MEDIA_AUDIO_AUDIO_SERVER_APP_H_
#define SERVICES_MEDIA_AUDIO_AUDIO_SERVER_APP_H_

#include "mojo/common/binding_set.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/interface_factory.h"
#include "mojo/services/media/audio/interfaces/audio_server.mojom.h"
#include "services/media/audio/audio_server_impl.h"

namespace mojo {
namespace media {
namespace audio {

class AudioServerApp : public ApplicationDelegate,
                       public InterfaceFactory<AudioServer> {
 public:
  AudioServerApp();
  ~AudioServerApp() override;

  // ApplicationDelegate
  void Initialize(ApplicationImpl* app) override;
  bool ConfigureIncomingConnection(ApplicationConnection* connection) override;
  void Quit() override;

  // InterfaceFactory<AudioServer>
  void Create(const ConnectionContext& connection_context,
              InterfaceRequest<AudioServer> request) override;

 private:
  AudioServerImpl server_impl_;
  BindingSet<AudioServer> bindings_;
};

}  // namespace audio
}  // namespace media
}  // namespace mojo

#endif  // SERVICES_MEDIA_AUDIO_AUDIO_SERVER_APP_H_
