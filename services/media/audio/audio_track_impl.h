// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_MEDIA_AUDIO_AUDIO_TRACK_IMPL_H_
#define SERVICES_MEDIA_AUDIO_AUDIO_TRACK_IMPL_H_

#include <deque>
#include <set>

#include "base/synchronization/lock.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/public/cpp/bindings/callback.h"
#include "mojo/services/media/audio/interfaces/audio_track.mojom.h"
#include "mojo/services/media/common/cpp/linear_transform.h"
#include "services/media/audio/audio_pipe.h"
#include "services/media/audio/fwd_decls.h"
#include "services/media/common/rate_control_base.h"

namespace mojo {
namespace media {
namespace audio {

class AudioTrackImpl : public AudioTrack {
 public:
  // TODO(johngro): Find a better place for this constant.  It affects the
  // behavior of more than just the Audio Track implementation.
  static constexpr size_t PTS_FRACTIONAL_BITS = 12;

  ~AudioTrackImpl() override;
  static AudioTrackImplPtr Create(InterfaceRequest<AudioTrack> iface,
                                  AudioServerImpl* owner);

  // Shutdown the audio track, unlinking it from all outputs, closing
  // connections to all clients and removing it from its owner server's list.
  void Shutdown();

  // Methods used by the output manager to link this track to different outputs.
  void AddOutput(AudioTrackToOutputLinkPtr link);
  void RemoveOutput(AudioTrackToOutputLinkPtr link);

  // Accessors used by AudioOutputs during mixing to access parameters which are
  // important for the mixing process.
  void SnapshotRateTrans(LinearTransform* out, uint32_t* generation = nullptr) {
    rate_control_.SnapshotCurrentTransform(out, generation);
  }

  const LinearTransform::Ratio& FractionalFrameToMediaTimeRatio() const {
    return frame_to_media_ratio_;
  }

  uint32_t BytesPerFrame() const { return bytes_per_frame_; }
  const AudioMediaTypeDetailsPtr& Format() const { return format_; }
  float DbGain() const { return db_gain_; }

 private:
  friend class AudioPipe;

  AudioTrackImpl(InterfaceRequest<AudioTrack> track,
                 AudioServerImpl* owner);

  // Implementation of AudioTrack interface.
  void Describe(const DescribeCallback& cbk) override;
  void Configure(AudioTrackConfigurationPtr configuration,
                 InterfaceRequest<MediaConsumer> req) override;
  void GetRateControl(InterfaceRequest<RateControl> req) override;
  void SetGain(float db_gain) override;

  // Methods called by our AudioPipe.
  //
  // TODO(johngro): MI is banned by style, but multiple interface inheritance
  // (inheriting for one or more base classes consisting only of pure virtual
  // methods) is allowed.  Consider defining an interface for AudioPipe
  // encapsulation so that AudioPipe does not have to know that we are an
  // AudioTrackImpl (just that we implement its interface).
  void OnPacketReceived(AudioPipe::AudioPacketRefPtr packet);
  bool OnFlushRequested(const MediaConsumer::FlushCallback& cbk);

  AudioTrackImplWeakPtr     weak_this_;
  AudioServerImpl*          owner_;
  Binding<AudioTrack>       binding_;
  AudioPipe                 pipe_;
  RateControlBase           rate_control_;
  LinearTransform::Ratio    frame_to_media_ratio_;
  uint32_t                  bytes_per_frame_ = 1;
  AudioMediaTypeDetailsPtr  format_;
  AudioTrackToOutputLinkSet outputs_;
  float                     db_gain_ = 0.0;
};

}  // namespace audio
}  // namespace media
}  // namespace mojo

#endif  // SERVICES_MEDIA_AUDIO_AUDIO_TRACK_IMPL_H_
