// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/debug/stack_trace.h"
#include "base/logging.h"
#include "mojo/services/media/common/cpp/local_time.h"
#include "services/media/common/rate_control_base.h"

namespace mojo {
namespace media {

static inline int64_t LocalTimeNow() {
  return LocalClock::now().time_since_epoch().count();
}

RateControlBase::RateControlBase()
  : binding_(this)
  , current_transform_(0, 1) {
}

RateControlBase::~RateControlBase() {
  Reset();
}

bool RateControlBase::Bind(InterfaceRequest<RateControl> request) {
  Reset();

  binding_.Bind(request.Pass());
  binding_.set_connection_error_handler([this]() -> void {
    Reset();
  });

  return true;
}

void RateControlBase::SnapshotCurrentTransform(LinearTransform* out,
                                               uint32_t* generation) {
  DCHECK(out);
  base::AutoLock lock(transform_lock_);
  ApplyPendingChangesLocked(LocalTimeNow());
  *out = current_transform_;
  if (generation) {
    *generation = generation_;
  }
}

void RateControlBase::GetCurrentTransform(
    const GetCurrentTransformCallback& cbk) {
  TimelineTransformPtr ret(TimelineTransform::New());
  ret->quad = TimelineQuad::New();

  LinearTransform trans;
  SnapshotCurrentTransform(&trans);
  ret->quad->target_offset    = trans.a_zero;
  ret->quad->reference_offset = trans.b_zero;
  ret->quad->target_delta     = trans.scale.denominator;
  ret->quad->reference_delta  = trans.scale.numerator;
  ret->reference_timeline_id  = TimelineTransform::kContextual;
  ret->target_timeline_id     = TimelineTransform::kLocalTimeID;

  cbk.Run(ret.Pass());
}

// TODO(johngro): implement or remove.  Until we have the ability to query the
// clock in the target timeline (or at least, transform local time to the target
// timeline), we have no way to apply scheduled changes.
void RateControlBase::SetTargetTimelineID(uint32_t id) {
  if (id != TimelineTransform::kLocalTimeID) {
    LOG(ERROR) << "Unsupported target timeline id ("
               << id << ") during SetTargetTimelineID";
    Reset();
  }
}

void RateControlBase::SetCurrentQuad(TimelineQuadPtr quad) {
  // A target delta of zero means that the transformation from the target
  // timeline to the media timeline is singular.  This is not permitted, log an
  // error and close the connection if someone attempts to do this.
  if (!quad->target_delta) {
    OnIllegalRateChange(quad->reference_delta, quad->target_delta);
    return;
  } else {
    base::AutoLock lock(transform_lock_);

    reference_pending_changes_.clear();
    target_pending_changes_.clear();

    current_transform_.a_zero = quad->target_offset;
    current_transform_.b_zero = quad->reference_offset;

    if (quad->reference_delta) {
      current_transform_.scale =
        LinearTransform::Ratio(quad->reference_delta, quad->target_delta);
    } else {
      current_transform_.scale.numerator = 0;
      current_transform_.scale.denominator = 1;
    }

    AdvanceGenerationLocked();
  }
}

void RateControlBase::SetRate(uint32_t reference_delta, uint32_t target_delta) {
  // Only rate changes with a non-zero target_delta are permitted.  See comment
  // in SetCurrentQuad.
  if (!target_delta) {
    OnIllegalRateChange(reference_delta, target_delta);
    return;
  } else {
    base::AutoLock lock(transform_lock_);

    // Make sure we are up to date.
    int64_t target_now = LocalTimeNow();
    ApplyPendingChangesLocked(target_now);

    DCHECK(current_transform_.scale.denominator);
    int64_t reference_now;
    if (!current_transform_.DoForwardTransform(target_now, &reference_now)) {
      // TODO(johngro): we cannot apply this transformation because of
      // overflow, so we are forced to skip it.  Should we introduce a callback
      // to allow the user to know that their transformation was skipped?
      // Alternatively, should we log something about how the transformation was
      // skipped?
      return;
    }

    current_transform_.a_zero = target_now;
    current_transform_.b_zero = reference_now;
    current_transform_.scale.numerator = reference_delta;
    current_transform_.scale.denominator = target_delta;

    AdvanceGenerationLocked();
  }
}

void RateControlBase::SetRateAtReferenceTime(uint32_t reference_delta,
                                             uint32_t target_delta,
                                             int64_t  reference_time) {
  // Only rate changes with a non-zero target_delta are permitted.  See comment
  // in SetCurrentQuad.
  if (!target_delta) {
    OnIllegalRateChange(reference_delta, target_delta);
    return;
  } else {
    base::AutoLock lock(transform_lock_);

    // If the user tries to schedule a change which takes place before any
    // already scheduled change, ignore it.
    if (reference_pending_changes_.size() &&
        reference_pending_changes_.back().b_zero >= reference_time) {
      return;
    }

    reference_pending_changes_.emplace_back(0,
                                            reference_delta,
                                            target_delta,
                                            reference_time);
  }
}

void RateControlBase::SetRateAtTargetTime(uint32_t reference_delta,
                                          uint32_t target_delta,
                                          int64_t  target_time) {
  // Only rate changes with a non-zero target_delta are permitted.  See comment
  // in SetCurrentQuad.
  if (!target_delta) {
    OnIllegalRateChange(reference_delta, target_delta);
    return;
  } else {
    base::AutoLock lock(transform_lock_);

    // If the user tries to schedule a change which takes place before any
    // already scheduled change, ignore it.
    if (target_pending_changes_.size() &&
        target_pending_changes_.back().a_zero >= target_time) {
      return;
    }

    target_pending_changes_.emplace_back(target_time,
                                         reference_delta,
                                         target_delta,
                                         0);
  }
}

void RateControlBase::CancelPendingChanges() {
  base::AutoLock lock(transform_lock_);

  reference_pending_changes_.clear();
  target_pending_changes_.clear();
}

void RateControlBase::ApplyPendingChangesLocked(int64_t target_now) {
  bool advance_generation = false;

  do {
    // Grab a pointer to the next pending target scheduled transform which is
    // not in the future, if any.
    int64_t target_age;
    const LinearTransform* target_trans = nullptr;
    if (target_pending_changes_.size() &&
       (target_now >= target_pending_changes_.front().a_zero)) {
      target_trans = &target_pending_changes_.front();
      target_age = target_now - target_trans->a_zero;
    }

    // Grab a pointer to the next pending reference scheduled transform which is
    // not in the future, if any.
    //
    // TODO(johngro): Optimize this.  When we have pending reference scheduled
    // transformations, we don't have to compute this each and every time.  We
    // could just keep the time of the next reference scheduled change
    // (expressed in target time) pre-computed, and only update it when the
    // current transformation actually changes.
    int64_t reference_age;
    int64_t next_reference_change_target_time;
    const LinearTransform* reference_trans = nullptr;
    if (reference_pending_changes_.size()) {
      if (current_transform_.DoReverseTransform(
            reference_pending_changes_.front().b_zero,
            &next_reference_change_target_time)) {
        if (target_now >= next_reference_change_target_time) {
          reference_age = target_now - next_reference_change_target_time;
          reference_trans = &reference_pending_changes_.front();
        }
      }
    }

    if (target_trans && (!reference_trans || (reference_age <= target_age))) {
      // If we have a target scheduled transform which should be applied, and we
      // either have no reference scheduled transform which should be applied,
      // or we have a reference scheduled transform which should be applied
      // after the pending target scheduled transform, go ahead and apply the
      // target transform.
      //
      // Note: if we cannot apply this transformation due to overflow, we have a
      // serious problem.  For now, we just purge the scheduled transformation
      // and move on, but this is something which should never happen.  We
      // should probably signal an error up to the user somehow.
      int64_t next_target_change_reference_time;

      if (current_transform_.DoForwardTransform(
            target_trans->a_zero,
            &next_target_change_reference_time)) {
        current_transform_.a_zero = target_trans->a_zero;
        current_transform_.b_zero = next_target_change_reference_time;
        current_transform_.scale.numerator = target_trans->scale.numerator;
        current_transform_.scale.denominator = target_trans->scale.denominator;
        DCHECK(current_transform_.scale.denominator);
      }

      advance_generation = true;
      target_pending_changes_.pop_front();
    } else if (reference_trans) {
      // We have a reference scheduled transformation which should be applied
      // before any pending target scheduled transformation.  Do so now.  No
      // need to compute the splice point for the function, we have already done
      // so when determining if we should apply this transformation or not.
      current_transform_.a_zero = next_reference_change_target_time;
      current_transform_.b_zero = reference_trans->a_zero;
      current_transform_.scale.numerator = reference_trans->scale.numerator;
      current_transform_.scale.denominator = reference_trans->scale.denominator;
      DCHECK(current_transform_.scale.denominator);

      advance_generation = true;
      reference_pending_changes_.pop_front();
    } else {
      // We have no transformations which need to be applied at the moment.  We
      // are done for now.
      break;
    }
  } while (true);

  // If we have applied any changes, advance the transformation generation
  if (advance_generation) {
    AdvanceGenerationLocked();
  }
}

void RateControlBase::OnIllegalRateChange(uint32_t numerator,
                                          uint32_t denominator) {
  LOG(ERROR) << "Illegal rate change requested ("
             << numerator << "/" << denominator << ")";
  Reset();
}

void RateControlBase::Reset() {
  CancelPendingChanges();
  SetRate(0, 1);

  if (binding_.is_bound()) {
    binding_.set_connection_error_handler(mojo::Closure());
    binding_.Close();
  }
}

}  // namespace media
}  // namespace mojo
