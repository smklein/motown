// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/environment/environment.h"

#include <assert.h>

#include "mojo/public/c/environment/logger.h"
#include "mojo/public/cpp/environment/lib/default_async_waiter.h"
#include "mojo/public/cpp/environment/lib/default_logger.h"
#include "mojo/public/cpp/utility/run_loop.h"

namespace mojo {

namespace {

const MojoAsyncWaiter* g_default_async_waiter = nullptr;
const MojoLogger* g_default_logger = nullptr;

void Init(const MojoAsyncWaiter* default_async_waiter,
          const MojoLogger* default_logger) {
  g_default_async_waiter = default_async_waiter
                               ? default_async_waiter
                               : &internal::kDefaultAsyncWaiter;
  g_default_logger =
      default_logger ? default_logger : &internal::kDefaultLogger;

  RunLoop::SetUp();
}

}  // namespace

Environment::Environment() {
  Init(nullptr, nullptr);
}

Environment::Environment(const MojoAsyncWaiter* default_async_waiter,
                         const MojoLogger* default_logger) {
  Init(default_async_waiter, default_logger);
}

Environment::~Environment() {
  RunLoop::TearDown();

  // TODO(vtl): Maybe we should allow nesting, and restore previous default
  // async waiters and loggers?
  g_default_async_waiter = nullptr;
  g_default_logger = nullptr;
}

// static
const MojoAsyncWaiter* Environment::GetDefaultAsyncWaiter() {
  assert(g_default_async_waiter);  // Fails if not "inside" |Environment|.
  return g_default_async_waiter;
}

// static
const MojoLogger* Environment::GetDefaultLogger() {
  assert(g_default_logger);  // Fails if not "inside" |Environment|.
  return g_default_logger;
}

// static
void Environment::SetDefaultLogger(const MojoLogger* logger) {
  assert(g_default_logger);  // Fails if not "inside" |Environment|.
  g_default_logger = logger ? logger : &internal::kDefaultLogger;
}

// static
void Environment::InstantiateDefaultRunLoop() {
  assert(!RunLoop::current());
  // Not leaked: accessible from |RunLoop::current()|.
  RunLoop* run_loop = new RunLoop();
  MOJO_ALLOW_UNUSED_LOCAL(run_loop);
  assert(run_loop == RunLoop::current());
}

// static
void Environment::DestroyDefaultRunLoop() {
  assert(RunLoop::current());
  delete RunLoop::current();
  assert(!RunLoop::current());
}

}  // namespace mojo
