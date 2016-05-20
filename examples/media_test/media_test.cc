// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/media_test/media_test.h"
#include "mojo/public/cpp/application/connect.h"
#include "mojo/services/media/common/cpp/linear_transform.h"
#include "mojo/services/media/common/cpp/local_time.h"
#include "mojo/services/media/control/interfaces/media_factory.mojom.h"

namespace mojo {
namespace media {
namespace examples {

// static
std::unique_ptr<MediaTest> MediaTest::Create(
    mojo::ApplicationImpl* app,
    const std::string& input_file_name) {
  return std::unique_ptr<MediaTest>(new MediaTest(app, input_file_name));
}

MediaTest::MediaTest(mojo::ApplicationImpl* app,
                     const std::string& input_file_name)
    : state_(MediaState::UNPREPARED) {
  MediaFactoryPtr factory;
  ConnectToService(app->shell(), "mojo:media_factory", GetProxy(&factory));

  SeekingReaderPtr reader;
  factory->CreateNetworkReader(input_file_name, GetProxy(&reader));
  factory->CreatePlayer(reader.Pass(), GetProxy(&media_player_));

  HandleStatusUpdates();
}

MediaTest::~MediaTest() {}

int64_t MediaTest::position_ns() const {
  // Apply the transform to the current time.
  int64_t position;
  transform_.DoForwardTransform(LocalClock::now().time_since_epoch().count(),
                                &position);

  if (position < 0) {
    position = 0;
  }

  if (metadata_ && static_cast<uint64_t>(position) > metadata_->duration) {
    position = metadata_->duration;
  }

  return position;
}

const MediaMetadataPtr& MediaTest::metadata() const {
  return metadata_;
}

void MediaTest::HandleStatusUpdates(uint64_t version,
                                    MediaPlayerStatusPtr status) {
  if (status) {
    // Process status received from the player.
    previous_state_ = state_;
    state_ = status->state;

    // Create a linear transform that translates local time to presentation
    // time. Note that 'reference' here refers to the presentation time, and
    // 'target' refers to the local time.
    if (status->timeline_transform) {
      transform_ =
          LinearTransform(status->timeline_transform->quad->target_offset,
                          status->timeline_transform->quad->reference_delta,
                          status->timeline_transform->quad->target_delta,
                          status->timeline_transform->quad->reference_offset);
    }

    metadata_ = status->metadata.Pass();

    if (update_callback_ != nullptr) {
      update_callback_();
    }
  }

  // Request a status update.
  media_player_->GetStatus(
      version, [this](uint64_t version, MediaPlayerStatusPtr status) {
        HandleStatusUpdates(version, status.Pass());
      });
}

}  // namespace examples
}  // namespace media
}  // namespace mojo
