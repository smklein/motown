// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_MEDIA_COMMON_RATE_CONTROL_BASE_H_
#define SERVICES_MEDIA_COMMON_RATE_CONTROL_BASE_H_

#include <deque>

#include "base/synchronization/lock.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/services/media/common/cpp/linear_transform.h"
#include "mojo/services/media/common/interfaces/media_common.mojom.h"
#include "mojo/services/media/common/interfaces/rate_control.mojom.h"

namespace mojo {
namespace media {

class RateControlBase : public RateControl {
 public:
  // Default constructor and destructor
  RateControlBase();
  ~RateControlBase() override;

  bool Bind(InterfaceRequest<RateControl> request);
  bool is_bound() const { return binding_.is_bound(); }

  // Close any existing connections to clients, clear any pending rate changes
  // and set the clock rate to 0/1.
  void Reset();

  // TODO(johngro): snapshotting the current transform requires an evaluation of
  // all the pending timeline transformations.  Currently, we allow users to
  // schedule an arbitrary number of pending transformations.  This could cause
  // DoS hazards if a malicious (or just poorly written) application is
  // schedules a ton of transformations, and then something like the mixer
  // threads in the audio server is forced to collapse all of these
  // transformations in order to mix then next set of outbound audio frames.
  //
  // A simple way to avoid this would be to allow the user to have only one
  // pending transformation at any point in time.
  //
  // It would be nice to be able to simply return the transformation and use
  // Rvalue references in calling code to access the temporary snapshot, but
  // style does not permit us to use Rvalue references in such a way.
  //
  // Also; the way pending transformations get applied probably needs to be
  // re-worked.  Currently, when we snapshot, we collapse any pending
  // transformations which should have occurred relative to LocalClock::now()
  // into the current transformation.  For both video and audio, however, we are
  // always mixing/composing for a point in time in the near future, not for
  // right now.  We really want to given the mixer compositor a view of what the
  // transformation is going to be at the mix/composition point, not what it is
  // now.  Additionally, since audio process many frames at a time, we need to
  // give the audio mixer some knowledge of when we think the snapshotted
  // transformation is going to change next.  The audio mixer wants to mix up to
  // that point, but not past it, and then fetch the new transformation before
  // proceeding.
  void SnapshotCurrentTransform(LinearTransform* out,
                                uint32_t* generation = nullptr);

  // RateControl interface
  //
  void GetCurrentTransform(const GetCurrentTransformCallback& cbk) override;
  void SetTargetTimelineID(uint32_t id) override;
  void SetCurrentQuad(TimelineQuadPtr quad) override;
  void SetRate(uint32_t reference_delta, uint32_t target_delta) override;
  void SetRateAtReferenceTime(uint32_t reference_delta,
                              uint32_t target_delta,
                              int64_t  reference_time) override;
  void SetRateAtTargetTime(uint32_t reference_delta,
                           uint32_t target_delta,
                           int64_t  target_time) override;
  void CancelPendingChanges() override;

 protected:
  void ApplyPendingChangesLocked(int64_t target_now);
  void AdvanceGenerationLocked() {
    // bump the generation counter.  Do not use the value 0.
    while (!(++generation_)) {}
  }
  void OnIllegalRateChange(uint32_t numerator, uint32_t denominator);

  Binding<RateControl> binding_;
  uint32_t target_timeline_id = TimelineTransform::kLocalTimeID;

  //  Transformation state.
  //
  //  Note: We use the LinearTransforms such that space A is the target timeline
  //  and space B is the reference timeline.  Applying this convention,
  //  transforming from target to reference is the "forward" transformation and
  //  is always defined.  Transforming from reference to target is the "reverse"
  //  transformation, and is only defined when we are not paused.
  //  <pedantic>
  //  OK; It is defined, but the equation has a singularity and the mapping is
  //  not 1-to-1.
  //  </pedantic>.
  base::Lock transform_lock_;
  LinearTransform current_transform_;
  std::deque<LinearTransform> reference_pending_changes_;
  std::deque<LinearTransform> target_pending_changes_;
  uint32_t generation_ = 1;
};

}  // namespace media
}  // namespace mojo

#endif  // SERVICES_MEDIA_COMMON_RATE_CONTROL_BASE_H_
