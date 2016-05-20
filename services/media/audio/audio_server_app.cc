// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "mojo/application/application_runner_chromium.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "services/media/audio/audio_server_app.h"

namespace mojo {
namespace media {
namespace audio {

AudioServerApp::AudioServerApp() {}
AudioServerApp::~AudioServerApp() {}

void AudioServerApp::Initialize(ApplicationImpl* app) {
  server_impl_.Initialize();
}

bool AudioServerApp::ConfigureIncomingConnection(
    ServiceProviderImpl* service_provider_impl) {
  service_provider_impl->AddService<AudioServer>(
      [this](const ConnectionContext& connection_context,
             InterfaceRequest<AudioServer> audio_server_request) {
        bindings_.AddBinding(&server_impl_, audio_server_request.Pass());
      });
  return true;
}

void AudioServerApp::Quit() {
}

}  // namespace audio
}  // namespace media
}  // namespace mojo

MojoResult MojoMain(MojoHandle app_request) {
  mojo::ApplicationRunnerChromium runner(
      new mojo::media::audio::AudioServerApp);
  return runner.Run(app_request);
}
