// Copyright (c) 2015, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

/// Helper utilities for testing.
library async.test.util;

import "dart:async";
import "package:test/test.dart";

/// A zero-millisecond timer should wait until after all microtasks.
Future flushMicrotasks() => new Future.delayed(Duration.ZERO);

/// A generic unreachable callback function.
///
/// Returns a function that fails the test if it is ever called.
unreachable(String name) => ([a, b]) => fail("Unreachable: $name");

/// A badly behaved stream which throws if it's ever listened to.
///
/// Can be used to test cases where a stream should not be used.
class UnusableStream extends Stream {
  listen(onData, {onError, onDone, cancelOnError}) {
    throw new UnimplementedError("Gotcha!");
  }
}
