// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file tests the C API.

#include <string.h>

#include "mojo/public/c/system/buffer.h"
#include "mojo/public/c/system/data_pipe.h"
#include "mojo/public/c/system/handle.h"
#include "mojo/public/c/system/message_pipe.h"
#include "mojo/public/c/system/result.h"
#include "mojo/public/c/system/time.h"
#include "mojo/public/c/system/wait.h"
#include "testing/gtest/include/gtest/gtest.h"

// Defined in core_unittest_pure_c.c.
extern "C" const char* MinimalCTest(void);

// Defined in core_unittest_pure_cpp.cc.
const char* MinimalCppTest();

namespace mojo {
namespace {

TEST(CoreTest, GetTimeTicksNow) {
  const MojoTimeTicks start = MojoGetTimeTicksNow();
  EXPECT_NE(static_cast<MojoTimeTicks>(0), start)
      << "MojoGetTimeTicksNow should return nonzero value";
}

// The only handle that's guaranteed to be invalid is |MOJO_HANDLE_INVALID|.
// Tests that everything that takes a handle properly recognizes it.
TEST(CoreTest, InvalidHandle) {
  MojoHandleSignals sig;
  char buffer[10] = {0};
  uint32_t buffer_size;
  void* write_pointer;
  const void* read_pointer;

  // Close:
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT, MojoClose(MOJO_HANDLE_INVALID));

  // Wait:
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoWait(MOJO_HANDLE_INVALID, ~MOJO_HANDLE_SIGNAL_NONE, 1000000u,
                     nullptr));

  const MojoHandle h = MOJO_HANDLE_INVALID;
  sig = ~MOJO_HANDLE_SIGNAL_NONE;
  EXPECT_EQ(
      MOJO_RESULT_INVALID_ARGUMENT,
      MojoWaitMany(&h, &sig, 1u, MOJO_DEADLINE_INDEFINITE, nullptr, nullptr));

  // Message pipe:
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoWriteMessage(MOJO_HANDLE_INVALID, buffer, 0u, nullptr, 0u,
                             MOJO_WRITE_MESSAGE_FLAG_NONE));
  buffer_size = static_cast<uint32_t>(sizeof(buffer));
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoReadMessage(MOJO_HANDLE_INVALID, buffer, &buffer_size, nullptr,
                            nullptr, MOJO_READ_MESSAGE_FLAG_NONE));

  // Data pipe:
  MojoDataPipeProducerOptions dpp_options = {
      sizeof(MojoDataPipeProducerOptions), 0u};
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoSetDataPipeProducerOptions(MOJO_HANDLE_INVALID, &dpp_options));
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoGetDataPipeProducerOptions(
                MOJO_HANDLE_INVALID, &dpp_options,
                static_cast<uint32_t>(sizeof(dpp_options))));
  buffer_size = static_cast<uint32_t>(sizeof(buffer));
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoWriteData(MOJO_HANDLE_INVALID, buffer, &buffer_size,
                          MOJO_WRITE_DATA_FLAG_NONE));
  write_pointer = nullptr;
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoBeginWriteData(MOJO_HANDLE_INVALID, &write_pointer,
                               &buffer_size, MOJO_WRITE_DATA_FLAG_NONE));
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoEndWriteData(MOJO_HANDLE_INVALID, 1u));
  buffer_size = static_cast<uint32_t>(sizeof(buffer));
  MojoDataPipeConsumerOptions dpc_options = {
      sizeof(MojoDataPipeConsumerOptions), 0u};
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoSetDataPipeConsumerOptions(MOJO_HANDLE_INVALID, &dpc_options));
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoGetDataPipeConsumerOptions(
                MOJO_HANDLE_INVALID, &dpc_options,
                static_cast<uint32_t>(sizeof(dpc_options))));
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoReadData(MOJO_HANDLE_INVALID, buffer, &buffer_size,
                         MOJO_READ_DATA_FLAG_NONE));
  read_pointer = nullptr;
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoBeginReadData(MOJO_HANDLE_INVALID, &read_pointer, &buffer_size,
                              MOJO_READ_DATA_FLAG_NONE));
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoEndReadData(MOJO_HANDLE_INVALID, 1u));

  // Shared buffer:
  MojoHandle out_handle = MOJO_HANDLE_INVALID;
  EXPECT_EQ(
      MOJO_RESULT_INVALID_ARGUMENT,
      MojoDuplicateBufferHandle(MOJO_HANDLE_INVALID, nullptr, &out_handle));
  MojoBufferInformation buffer_info = {};
  EXPECT_EQ(
      MOJO_RESULT_INVALID_ARGUMENT,
      MojoGetBufferInformation(MOJO_HANDLE_INVALID, &buffer_info,
                               static_cast<uint32_t>(sizeof(buffer_info))));
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoMapBuffer(MOJO_HANDLE_INVALID, 0u, 1u, &write_pointer,
                          MOJO_MAP_BUFFER_FLAG_NONE));
  // This isn't an "invalid handle" test, but we'll throw it in here anyway
  // (since it involves a look-up).
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT, MojoUnmapBuffer(nullptr));
}

TEST(CoreTest, BasicMessagePipe) {
  MojoHandle h0, h1;
  MojoHandleSignals sig;
  char buffer[10] = {0};
  uint32_t buffer_size;

  h0 = MOJO_HANDLE_INVALID;
  h1 = MOJO_HANDLE_INVALID;
  EXPECT_EQ(MOJO_RESULT_OK, MojoCreateMessagePipe(nullptr, &h0, &h1));
  EXPECT_NE(h0, MOJO_HANDLE_INVALID);
  EXPECT_NE(h1, MOJO_HANDLE_INVALID);

  // Shouldn't be readable, we haven't written anything.
  MojoHandleSignalsState state;
  EXPECT_EQ(MOJO_RESULT_DEADLINE_EXCEEDED,
            MojoWait(h0, MOJO_HANDLE_SIGNAL_READABLE, 0, &state));
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_WRITABLE, state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_WRITABLE |
                MOJO_HANDLE_SIGNAL_PEER_CLOSED,
            state.satisfiable_signals);

  // Should be writable.
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoWait(h0, MOJO_HANDLE_SIGNAL_WRITABLE, 0, &state));
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_WRITABLE, state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_WRITABLE |
                MOJO_HANDLE_SIGNAL_PEER_CLOSED,
            state.satisfiable_signals);

  // Last parameter is optional.
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoWait(h0, MOJO_HANDLE_SIGNAL_WRITABLE, 0, nullptr));

  // Try to read.
  buffer_size = static_cast<uint32_t>(sizeof(buffer));
  EXPECT_EQ(MOJO_RESULT_SHOULD_WAIT,
            MojoReadMessage(h0, buffer, &buffer_size, nullptr, nullptr,
                            MOJO_READ_MESSAGE_FLAG_NONE));

  // Write to |h1|.
  static const char kHello[] = "hello";
  buffer_size = static_cast<uint32_t>(sizeof(kHello));
  EXPECT_EQ(MOJO_RESULT_OK, MojoWriteMessage(h1, kHello, buffer_size, nullptr,
                                             0, MOJO_WRITE_MESSAGE_FLAG_NONE));

  // |h0| should be readable.
  uint32_t result_index = 1;
  MojoHandleSignalsState states[1];
  sig = MOJO_HANDLE_SIGNAL_READABLE;
  EXPECT_EQ(MOJO_RESULT_OK, MojoWaitMany(&h0, &sig, 1, MOJO_DEADLINE_INDEFINITE,
                                         &result_index, states));
  EXPECT_EQ(0u, result_index);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_WRITABLE,
            states[0].satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_WRITABLE |
                MOJO_HANDLE_SIGNAL_PEER_CLOSED,
            states[0].satisfiable_signals);

  // Read from |h0|.
  buffer_size = static_cast<uint32_t>(sizeof(buffer));
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoReadMessage(h0, buffer, &buffer_size, nullptr, nullptr,
                            MOJO_READ_MESSAGE_FLAG_NONE));
  EXPECT_EQ(static_cast<uint32_t>(sizeof(kHello)), buffer_size);
  EXPECT_STREQ(kHello, buffer);

  // |h0| should no longer be readable.
  EXPECT_EQ(MOJO_RESULT_DEADLINE_EXCEEDED,
            MojoWait(h0, MOJO_HANDLE_SIGNAL_READABLE, 10, &state));
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_WRITABLE, state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_WRITABLE |
                MOJO_HANDLE_SIGNAL_PEER_CLOSED,
            state.satisfiable_signals);

  // Close |h0|.
  EXPECT_EQ(MOJO_RESULT_OK, MojoClose(h0));

  // |h1| should no longer be readable or writable.
  EXPECT_EQ(
      MOJO_RESULT_FAILED_PRECONDITION,
      MojoWait(h1, MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_WRITABLE,
               1000, &state));
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_PEER_CLOSED, state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_PEER_CLOSED, state.satisfiable_signals);

  EXPECT_EQ(MOJO_RESULT_OK, MojoClose(h1));
}

// TODO(ncbray): enable this test once NaCl supports the corresponding APIs.
#ifdef __native_client__
#define MAYBE_BasicDataPipe DISABLED_BasicDataPipe
#else
#define MAYBE_BasicDataPipe BasicDataPipe
#endif
TEST(CoreTest, MAYBE_BasicDataPipe) {
  MojoHandle hp, hc;
  MojoHandleSignals sig;
  char buffer[20] = {0};
  uint32_t buffer_size;
  void* write_pointer;
  const void* read_pointer;

  hp = MOJO_HANDLE_INVALID;
  hc = MOJO_HANDLE_INVALID;
  EXPECT_EQ(MOJO_RESULT_OK, MojoCreateDataPipe(nullptr, &hp, &hc));
  EXPECT_NE(hp, MOJO_HANDLE_INVALID);
  EXPECT_NE(hc, MOJO_HANDLE_INVALID);

  // The consumer |hc| shouldn't be readable.
  MojoHandleSignalsState state;
  EXPECT_EQ(MOJO_RESULT_DEADLINE_EXCEEDED,
            MojoWait(hc, MOJO_HANDLE_SIGNAL_READABLE, 0, &state));
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_NONE, state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_PEER_CLOSED |
                MOJO_HANDLE_SIGNAL_READ_THRESHOLD,
            state.satisfiable_signals);

  // The producer |hp| should be writable.
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoWait(hp, MOJO_HANDLE_SIGNAL_WRITABLE, 0, &state));
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_WRITABLE | MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD,
            state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_WRITABLE | MOJO_HANDLE_SIGNAL_PEER_CLOSED |
                MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD,
            state.satisfiable_signals);

  // Try to read from |hc|.
  buffer_size = static_cast<uint32_t>(sizeof(buffer));
  EXPECT_EQ(MOJO_RESULT_SHOULD_WAIT,
            MojoReadData(hc, buffer, &buffer_size, MOJO_READ_DATA_FLAG_NONE));

  // Try to begin a two-phase read from |hc|.
  read_pointer = nullptr;
  EXPECT_EQ(MOJO_RESULT_SHOULD_WAIT,
            MojoBeginReadData(hc, &read_pointer, &buffer_size,
                              MOJO_READ_DATA_FLAG_NONE));

  // Write to |hp|.
  static const char kHello[] = "hello ";
  // Don't include terminating null.
  buffer_size = static_cast<uint32_t>(strlen(kHello));
  EXPECT_EQ(MOJO_RESULT_OK, MojoWriteData(hp, kHello, &buffer_size,
                                          MOJO_WRITE_MESSAGE_FLAG_NONE));

  // |hc| should be(come) readable.
  uint32_t result_index = 1;
  MojoHandleSignalsState states[1];
  sig = MOJO_HANDLE_SIGNAL_READABLE;
  EXPECT_EQ(MOJO_RESULT_OK, MojoWaitMany(&hc, &sig, 1, MOJO_DEADLINE_INDEFINITE,
                                         &result_index, states));
  EXPECT_EQ(0u, result_index);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_READ_THRESHOLD,
            states[0].satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_PEER_CLOSED |
                MOJO_HANDLE_SIGNAL_READ_THRESHOLD,
            states[0].satisfiable_signals);

  // Do a two-phase write to |hp|.
  EXPECT_EQ(MOJO_RESULT_OK, MojoBeginWriteData(hp, &write_pointer, &buffer_size,
                                               MOJO_WRITE_DATA_FLAG_NONE));
  static const char kWorld[] = "world";
  ASSERT_GE(buffer_size, sizeof(kWorld));
  // Include the terminating null.
  memcpy(write_pointer, kWorld, sizeof(kWorld));
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoEndWriteData(hp, static_cast<uint32_t>(sizeof(kWorld))));

  // Read one character from |hc|.
  memset(buffer, 0, sizeof(buffer));
  buffer_size = 1;
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoReadData(hc, buffer, &buffer_size, MOJO_READ_DATA_FLAG_NONE));

  // Close |hp|.
  EXPECT_EQ(MOJO_RESULT_OK, MojoClose(hp));

  // |hc| should still be readable.
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoWait(hc, MOJO_HANDLE_SIGNAL_READABLE, 0, &state));
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_PEER_CLOSED |
                MOJO_HANDLE_SIGNAL_READ_THRESHOLD,
            state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_PEER_CLOSED |
                MOJO_HANDLE_SIGNAL_READ_THRESHOLD,
            state.satisfiable_signals);

  // Do a two-phase read from |hc|.
  read_pointer = nullptr;
  EXPECT_EQ(MOJO_RESULT_OK, MojoBeginReadData(hc, &read_pointer, &buffer_size,
                                              MOJO_READ_DATA_FLAG_NONE));
  ASSERT_LE(buffer_size, sizeof(buffer) - 1);
  memcpy(&buffer[1], read_pointer, buffer_size);
  EXPECT_EQ(MOJO_RESULT_OK, MojoEndReadData(hc, buffer_size));
  EXPECT_STREQ("hello world", buffer);

  // |hc| should no longer be readable.
  EXPECT_EQ(MOJO_RESULT_FAILED_PRECONDITION,
            MojoWait(hc, MOJO_HANDLE_SIGNAL_READABLE, 1000, &state));
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_PEER_CLOSED, state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_PEER_CLOSED, state.satisfiable_signals);

  EXPECT_EQ(MOJO_RESULT_OK, MojoClose(hc));

  // TODO(vtl): Test the other way around -- closing the consumer should make
  // the producer never-writable?
}

TEST(CoreTest, DataPipeWriteThreshold) {
  const MojoCreateDataPipeOptions options = {
      static_cast<uint32_t>(
          sizeof(MojoCreateDataPipeOptions)),   // |struct_size|.
      MOJO_CREATE_DATA_PIPE_OPTIONS_FLAG_NONE,  // |flags|.
      2u,                                       // |element_num_bytes|.
      4u                                        // |capacity_num_bytes|.
  };
  MojoHandle hp = MOJO_HANDLE_INVALID;
  MojoHandle hc = MOJO_HANDLE_INVALID;
  EXPECT_EQ(MOJO_RESULT_OK, MojoCreateDataPipe(&options, &hp, &hc));
  EXPECT_NE(hp, MOJO_HANDLE_INVALID);
  EXPECT_NE(hc, MOJO_HANDLE_INVALID);
  EXPECT_NE(hc, hp);

  MojoDataPipeProducerOptions popts;
  static const uint32_t kPoptsSize = static_cast<uint32_t>(sizeof(popts));

  // Check the current write threshold; should be the default.
  memset(&popts, 255, kPoptsSize);
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoGetDataPipeProducerOptions(hp, &popts, kPoptsSize));
  EXPECT_EQ(kPoptsSize, popts.struct_size);
  EXPECT_EQ(0u, popts.write_threshold_num_bytes);

  // Should already have the write threshold signal.
  MojoHandleSignalsState state = {};
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoWait(hp, MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD, 0, &state));
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_WRITABLE | MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD,
            state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_WRITABLE | MOJO_HANDLE_SIGNAL_PEER_CLOSED |
                MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD,
            state.satisfiable_signals);

  // Try setting the write threshold to something invalid.
  popts.struct_size = kPoptsSize;
  popts.write_threshold_num_bytes = 1u;
  EXPECT_EQ(MOJO_RESULT_INVALID_ARGUMENT,
            MojoSetDataPipeProducerOptions(hp, &popts));
  // It shouldn't change the options.
  memset(&popts, 255, kPoptsSize);
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoGetDataPipeProducerOptions(hp, &popts, kPoptsSize));
  EXPECT_EQ(kPoptsSize, popts.struct_size);
  EXPECT_EQ(0u, popts.write_threshold_num_bytes);

  // Write an element.
  static const uint16_t kTestElem = 12345u;
  uint32_t num_bytes = 2u;
  EXPECT_EQ(MOJO_RESULT_OK, MojoWriteData(hp, &kTestElem, &num_bytes,
                                          MOJO_WRITE_MESSAGE_FLAG_NONE));
  EXPECT_EQ(2u, num_bytes);

  // Should still have the write threshold signal.
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoWait(hp, MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD, 0, nullptr));

  // Write another element.
  static const uint16_t kAnotherTestElem = 12345u;
  num_bytes = 2u;
  EXPECT_EQ(MOJO_RESULT_OK, MojoWriteData(hp, &kAnotherTestElem, &num_bytes,
                                          MOJO_WRITE_MESSAGE_FLAG_NONE));
  EXPECT_EQ(2u, num_bytes);

  // Should no longer have the write threshold signal.
  state = MojoHandleSignalsState();
  EXPECT_EQ(MOJO_RESULT_DEADLINE_EXCEEDED,
            MojoWait(hp, MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD, 0, &state));
  EXPECT_EQ(0u, state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_WRITABLE | MOJO_HANDLE_SIGNAL_PEER_CLOSED |
                MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD,
            state.satisfiable_signals);

  // Set the write threshold to 2 (one element).
  popts.struct_size = kPoptsSize;
  popts.write_threshold_num_bytes = 2u;
  EXPECT_EQ(MOJO_RESULT_OK, MojoSetDataPipeProducerOptions(hp, &popts));
  // It should actually change the options.
  memset(&popts, 255, kPoptsSize);
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoGetDataPipeProducerOptions(hp, &popts, kPoptsSize));
  EXPECT_EQ(kPoptsSize, popts.struct_size);
  EXPECT_EQ(2u, popts.write_threshold_num_bytes);

  // Should still not have the write threshold signal.
  EXPECT_EQ(MOJO_RESULT_DEADLINE_EXCEEDED,
            MojoWait(hp, MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD, 0, nullptr));

  // Read an element.
  uint16_t read_elem = 0u;
  num_bytes = 2u;
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoReadData(hc, &read_elem, &num_bytes, MOJO_READ_DATA_FLAG_NONE));
  EXPECT_EQ(2u, num_bytes);
  EXPECT_EQ(kTestElem, read_elem);

  // Should get the write threshold signal now.
  state = MojoHandleSignalsState();
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoWait(hp, MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD, 1000, &state));
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_WRITABLE | MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD,
            state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_WRITABLE | MOJO_HANDLE_SIGNAL_PEER_CLOSED |
                MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD,
            state.satisfiable_signals);

  // Set the write threshold to 4 (two elements).
  popts.struct_size = kPoptsSize;
  popts.write_threshold_num_bytes = 4u;
  EXPECT_EQ(MOJO_RESULT_OK, MojoSetDataPipeProducerOptions(hp, &popts));

  // Should again not have the write threshold signal.
  EXPECT_EQ(MOJO_RESULT_DEADLINE_EXCEEDED,
            MojoWait(hp, MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD, 0, nullptr));

  // Close the consumer.
  EXPECT_EQ(MOJO_RESULT_OK, MojoClose(hc));

  // The write threshold signal should now be unsatisfiable.
  state = MojoHandleSignalsState();
  EXPECT_EQ(MOJO_RESULT_FAILED_PRECONDITION,
            MojoWait(hp, MOJO_HANDLE_SIGNAL_WRITE_THRESHOLD, 0, &state));
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_PEER_CLOSED, state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_PEER_CLOSED, state.satisfiable_signals);

  EXPECT_EQ(MOJO_RESULT_OK, MojoClose(hp));
}

TEST(CoreTest, DataPipeReadThreshold) {
  MojoHandle hp = MOJO_HANDLE_INVALID;
  MojoHandle hc = MOJO_HANDLE_INVALID;
  EXPECT_EQ(MOJO_RESULT_OK, MojoCreateDataPipe(nullptr, &hp, &hc));
  EXPECT_NE(hp, MOJO_HANDLE_INVALID);
  EXPECT_NE(hc, MOJO_HANDLE_INVALID);
  EXPECT_NE(hc, hp);

  MojoDataPipeConsumerOptions copts;
  static const uint32_t kCoptsSize = static_cast<uint32_t>(sizeof(copts));

  // Check the current read threshold; should be the default.
  memset(&copts, 255, kCoptsSize);
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoGetDataPipeConsumerOptions(hc, &copts, kCoptsSize));
  EXPECT_EQ(kCoptsSize, copts.struct_size);
  EXPECT_EQ(0u, copts.read_threshold_num_bytes);

  // Shouldn't have the read threshold signal yet.
  EXPECT_EQ(MOJO_RESULT_DEADLINE_EXCEEDED,
            MojoWait(hc, MOJO_HANDLE_SIGNAL_READ_THRESHOLD, 1000, nullptr));

  // Write a byte to |hp|.
  static const char kAByte = 'A';
  uint32_t num_bytes = 1u;
  EXPECT_EQ(MOJO_RESULT_OK, MojoWriteData(hp, &kAByte, &num_bytes,
                                          MOJO_WRITE_MESSAGE_FLAG_NONE));
  EXPECT_EQ(1u, num_bytes);

  // Now should have the read threshold signal.
  MojoHandleSignalsState state;
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoWait(hc, MOJO_HANDLE_SIGNAL_READ_THRESHOLD, 1000, &state));
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_READ_THRESHOLD,
            state.satisfied_signals);
  EXPECT_EQ(MOJO_HANDLE_SIGNAL_READABLE | MOJO_HANDLE_SIGNAL_PEER_CLOSED |
                MOJO_HANDLE_SIGNAL_READ_THRESHOLD,
            state.satisfiable_signals);

  // Set the read threshold to 3, and then check it.
  copts.struct_size = kCoptsSize;
  copts.read_threshold_num_bytes = 3u;
  EXPECT_EQ(MOJO_RESULT_OK, MojoSetDataPipeConsumerOptions(hc, &copts));

  memset(&copts, 255, kCoptsSize);
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoGetDataPipeConsumerOptions(hc, &copts, kCoptsSize));
  EXPECT_EQ(kCoptsSize, copts.struct_size);
  EXPECT_EQ(3u, copts.read_threshold_num_bytes);

  // Shouldn't have the read threshold signal again.
  EXPECT_EQ(MOJO_RESULT_DEADLINE_EXCEEDED,
            MojoWait(hc, MOJO_HANDLE_SIGNAL_READ_THRESHOLD, 0, nullptr));

  // Write another byte to |hp|.
  static const char kBByte = 'B';
  num_bytes = 1u;
  EXPECT_EQ(MOJO_RESULT_OK, MojoWriteData(hp, &kBByte, &num_bytes,
                                          MOJO_WRITE_MESSAGE_FLAG_NONE));
  EXPECT_EQ(1u, num_bytes);

  // Still shouldn't have the read threshold signal.
  EXPECT_EQ(MOJO_RESULT_DEADLINE_EXCEEDED,
            MojoWait(hc, MOJO_HANDLE_SIGNAL_READ_THRESHOLD, 1000, nullptr));

  // Write a third byte to |hp|.
  static const char kCByte = 'C';
  num_bytes = 1u;
  EXPECT_EQ(MOJO_RESULT_OK, MojoWriteData(hp, &kCByte, &num_bytes,
                                          MOJO_WRITE_MESSAGE_FLAG_NONE));
  EXPECT_EQ(1u, num_bytes);

  // Now should have the read threshold signal.
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoWait(hc, MOJO_HANDLE_SIGNAL_READ_THRESHOLD, 1000, nullptr));

  // Read a byte.
  char read_byte = 'x';
  num_bytes = 1u;
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoReadData(hc, &read_byte, &num_bytes, MOJO_READ_DATA_FLAG_NONE));
  EXPECT_EQ(1u, num_bytes);
  EXPECT_EQ(kAByte, read_byte);

  // Shouldn't have the read threshold signal again.
  EXPECT_EQ(MOJO_RESULT_DEADLINE_EXCEEDED,
            MojoWait(hc, MOJO_HANDLE_SIGNAL_READ_THRESHOLD, 0, nullptr));

  // Set the read threshold to 2.
  copts.struct_size = kCoptsSize;
  copts.read_threshold_num_bytes = 2u;
  EXPECT_EQ(MOJO_RESULT_OK, MojoSetDataPipeConsumerOptions(hc, &copts));

  // Should have the read threshold signal again.
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoWait(hc, MOJO_HANDLE_SIGNAL_READ_THRESHOLD, 0, nullptr));

  // Set the read threshold to the default by passing null, and check it.
  EXPECT_EQ(MOJO_RESULT_OK, MojoSetDataPipeConsumerOptions(hc, nullptr));

  memset(&copts, 255, kCoptsSize);
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoGetDataPipeConsumerOptions(hc, &copts, kCoptsSize));
  EXPECT_EQ(kCoptsSize, copts.struct_size);
  EXPECT_EQ(0u, copts.read_threshold_num_bytes);

  // Should still have the read threshold signal.
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoWait(hc, MOJO_HANDLE_SIGNAL_READ_THRESHOLD, 0, nullptr));

  EXPECT_EQ(MOJO_RESULT_OK, MojoClose(hp));
  EXPECT_EQ(MOJO_RESULT_OK, MojoClose(hc));
}

// TODO(ncbray): enable this test once NaCl supports the corresponding APIs.
#ifdef __native_client__
#define MAYBE_BasicSharedBuffer DISABLED_BasicSharedBuffer
#else
#define MAYBE_BasicSharedBuffer BasicSharedBuffer
#endif
TEST(CoreTest, MAYBE_BasicSharedBuffer) {
  MojoHandle h0, h1;
  void* pointer;

  // Create a shared buffer (|h0|).
  h0 = MOJO_HANDLE_INVALID;
  EXPECT_EQ(MOJO_RESULT_OK, MojoCreateSharedBuffer(nullptr, 100, &h0));
  EXPECT_NE(h0, MOJO_HANDLE_INVALID);

  // Check information about the buffer from |h0|.
  MojoBufferInformation info = {};
  static const uint32_t kInfoSize = static_cast<uint32_t>(sizeof(info));
  EXPECT_EQ(MOJO_RESULT_OK, MojoGetBufferInformation(h0, &info, kInfoSize));
  EXPECT_EQ(kInfoSize, info.struct_size);
  EXPECT_EQ(MOJO_BUFFER_INFORMATION_FLAG_NONE, info.flags);
  EXPECT_EQ(100u, info.num_bytes);

  // Map everything.
  pointer = nullptr;
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoMapBuffer(h0, 0, 100, &pointer, MOJO_MAP_BUFFER_FLAG_NONE));
  ASSERT_TRUE(pointer);
  static_cast<char*>(pointer)[50] = 'x';

  // Duplicate |h0| to |h1|.
  h1 = MOJO_HANDLE_INVALID;
  EXPECT_EQ(MOJO_RESULT_OK, MojoDuplicateBufferHandle(h0, nullptr, &h1));
  EXPECT_NE(h1, MOJO_HANDLE_INVALID);

  // Check information about the buffer from |h1|.
  info = MojoBufferInformation();
  EXPECT_EQ(MOJO_RESULT_OK, MojoGetBufferInformation(h1, &info, kInfoSize));
  EXPECT_EQ(kInfoSize, info.struct_size);
  EXPECT_EQ(MOJO_BUFFER_INFORMATION_FLAG_NONE, info.flags);
  EXPECT_EQ(100u, info.num_bytes);

  // Close |h0|.
  EXPECT_EQ(MOJO_RESULT_OK, MojoClose(h0));

  // The mapping should still be good.
  static_cast<char*>(pointer)[51] = 'y';

  // Unmap it.
  EXPECT_EQ(MOJO_RESULT_OK, MojoUnmapBuffer(pointer));

  // Map half of |h1|.
  pointer = nullptr;
  EXPECT_EQ(MOJO_RESULT_OK,
            MojoMapBuffer(h1, 50, 50, &pointer, MOJO_MAP_BUFFER_FLAG_NONE));
  ASSERT_TRUE(pointer);

  // It should have what we wrote.
  EXPECT_EQ('x', static_cast<char*>(pointer)[0]);
  EXPECT_EQ('y', static_cast<char*>(pointer)[1]);

  // Unmap it.
  EXPECT_EQ(MOJO_RESULT_OK, MojoUnmapBuffer(pointer));

  EXPECT_EQ(MOJO_RESULT_OK, MojoClose(h1));
}

// This checks that things actually work in C (not C++).
TEST(CoreTest, MinimalCTest) {
  const char* failure = MinimalCTest();
  EXPECT_FALSE(failure) << failure;
}

TEST(CoreTest, MinimalCppTest) {
  const char* failure = MinimalCppTest();
  EXPECT_FALSE(failure) << failure;
}

// TODO(vtl): Add multi-threaded tests.

}  // namespace
}  // namespace mojo
