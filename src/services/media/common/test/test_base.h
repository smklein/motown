// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_MEDIA_COMMON_CPP_TEST_TEST_BASE_H_
#define MOJO_SERVICES_MEDIA_COMMON_CPP_TEST_TEST_BASE_H_

#include "mojo/public/cpp/application/application_test_base.h"

namespace mojo {
namespace media {
namespace {

class TestBase : public test::ApplicationTestBase {
 public:
  TestBase() {}
  ~TestBase() override {}

 private:
  MOJO_DISALLOW_COPY_AND_ASSIGN(TestBase);
};

}  // namespace
}  // namespace media
}  // namespace mojo

#endif  // MOJO_SERVICES_MEDIA_COMMON_CPP_TEST_TEST_BASE_H_
