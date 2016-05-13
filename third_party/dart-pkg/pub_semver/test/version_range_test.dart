// Copyright (c) 2014, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

library pub_semver.test.version_range_test;

import 'package:test/test.dart';

import 'package:pub_semver/pub_semver.dart';

import 'utils.dart';

main() {
  group('constructor', () {
    test('takes a min and max', () {
      var range = new VersionRange(min: v123, max: v124);
      expect(range.isAny, isFalse);
      expect(range.min, equals(v123));
      expect(range.max, equals(v124));
    });

    test('allows omitting max', () {
      var range = new VersionRange(min: v123);
      expect(range.isAny, isFalse);
      expect(range.min, equals(v123));
      expect(range.max, isNull);
    });

    test('allows omitting min and max', () {
      var range = new VersionRange();
      expect(range.isAny, isTrue);
      expect(range.min, isNull);
      expect(range.max, isNull);
    });

    test('takes includeMin', () {
      var range = new VersionRange(min: v123, includeMin: true);
      expect(range.includeMin, isTrue);
    });

    test('includeMin defaults to false if omitted', () {
      var range = new VersionRange(min: v123);
      expect(range.includeMin, isFalse);
    });

    test('takes includeMax', () {
      var range = new VersionRange(max: v123, includeMax: true);
      expect(range.includeMax, isTrue);
    });

    test('includeMax defaults to false if omitted', () {
      var range = new VersionRange(max: v123);
      expect(range.includeMax, isFalse);
    });

    test('throws if min > max', () {
      expect(() => new VersionRange(min: v124, max: v123), throwsArgumentError);
    });
  });

  group('allows()', () {
    test('version must be greater than min', () {
      var range = new VersionRange(min: v123);

      expect(range, allows(
          new Version.parse('1.3.3'),
          new Version.parse('2.3.3')));
      expect(range, doesNotAllow(
          new Version.parse('1.2.2'),
          new Version.parse('1.2.3')));
    });

    test('version must be min or greater if includeMin', () {
      var range = new VersionRange(min: v123, includeMin: true);

      expect(range, allows(
          new Version.parse('1.2.3'),
          new Version.parse('1.3.3'),
          new Version.parse('2.3.3')));
      expect(range, doesNotAllow(new Version.parse('1.2.2')));
    });

    test('pre-release versions of inclusive min are excluded', () {
      var range = new VersionRange(min: v123, includeMin: true);

      expect(range, allows(new Version.parse('1.2.4-dev')));
      expect(range, doesNotAllow(new Version.parse('1.2.3-dev')));
    });

    test('version must be less than max', () {
      var range = new VersionRange(max: v234);

      expect(range, allows(new Version.parse('2.3.3')));
      expect(range, doesNotAllow(
          new Version.parse('2.3.4'),
          new Version.parse('2.4.3')));
    });

    test('pre-release versions of non-pre-release max are excluded', () {
      var range = new VersionRange(max: v234);

      expect(range, allows(new Version.parse('2.3.3')));
      expect(range, doesNotAllow(
          new Version.parse('2.3.4-dev'),
          new Version.parse('2.3.4')));
    });

    test('pre-release versions of non-pre-release max are included if min is a '
        'pre-release of the same version', () {
      var range = new VersionRange(
          min: new Version.parse('2.3.4-dev.0'), max: v234);

      expect(range, allows(new Version.parse('2.3.4-dev.1')));
      expect(range, doesNotAllow(
          new Version.parse('2.3.3'),
          new Version.parse('2.3.4-dev'),
          new Version.parse('2.3.4')));
    });

    test('pre-release versions of pre-release max are included', () {
      var range = new VersionRange(max: new Version.parse('2.3.4-dev.2'));

      expect(range, allows(
          new Version.parse('2.3.4-dev.1')));
      expect(range, doesNotAllow(
          new Version.parse('2.3.4-dev.2'),
          new Version.parse('2.3.4-dev.3')));
    });

    test('version must be max or less if includeMax', () {
      var range = new VersionRange(min: v123, max: v234, includeMax: true);

      expect(range, allows(
          new Version.parse('2.3.3'),
          new Version.parse('2.3.4'),
          // Pre-releases of the max are allowed.
          new Version.parse('2.3.4-dev')));
      expect(range, doesNotAllow(new Version.parse('2.4.3')));
    });

    test('has no min if one was not set', () {
      var range = new VersionRange(max: v123);

      expect(range, allows(new Version.parse('0.0.0')));
      expect(range, doesNotAllow(new Version.parse('1.2.3')));
    });

    test('has no max if one was not set', () {
      var range = new VersionRange(min: v123);

      expect(range, allows(
          new Version.parse('1.3.3'),
          new Version.parse('999.3.3')));
      expect(range, doesNotAllow(new Version.parse('1.2.3')));
    });

    test('allows any version if there is no min or max', () {
      var range = new VersionRange();

      expect(range, allows(
          new Version.parse('0.0.0'),
          new Version.parse('999.99.9')));
    });
  });

  group('allowsAll()', () {
    test('allows an empty constraint', () {
      expect(
          new VersionRange(min: v123, max: v250)
              .allowsAll(VersionConstraint.empty),
          isTrue);
    });

    test('allows allowed versions', () {
      var range = new VersionRange(min: v123, max: v250, includeMax: true);
      expect(range.allowsAll(v123), isFalse);
      expect(range.allowsAll(v124), isTrue);
      expect(range.allowsAll(v250), isTrue);
      expect(range.allowsAll(v300), isFalse);
    });

    test('with no min', () {
      var range = new VersionRange(max: v250);
      expect(range.allowsAll(new VersionRange(min: v080, max: v140)), isTrue);
      expect(range.allowsAll(new VersionRange(min: v080, max: v300)), isFalse);
      expect(range.allowsAll(new VersionRange(max: v140)), isTrue);
      expect(range.allowsAll(new VersionRange(max: v300)), isFalse);
      expect(range.allowsAll(range), isTrue);
      expect(range.allowsAll(VersionConstraint.any), isFalse);
    });

    test('with no max', () {
      var range = new VersionRange(min: v010);
      expect(range.allowsAll(new VersionRange(min: v080, max: v140)), isTrue);
      expect(range.allowsAll(new VersionRange(min: v003, max: v140)), isFalse);
      expect(range.allowsAll(new VersionRange(min: v080)), isTrue);
      expect(range.allowsAll(new VersionRange(min: v003)), isFalse);
      expect(range.allowsAll(range), isTrue);
      expect(range.allowsAll(VersionConstraint.any), isFalse);
    });

    test('with a min and max', () {
      var range = new VersionRange(min: v010, max: v250);
      expect(range.allowsAll(new VersionRange(min: v080, max: v140)), isTrue);
      expect(range.allowsAll(new VersionRange(min: v080, max: v300)), isFalse);
      expect(range.allowsAll(new VersionRange(min: v003, max: v140)), isFalse);
      expect(range.allowsAll(new VersionRange(min: v080)), isFalse);
      expect(range.allowsAll(new VersionRange(max: v140)), isFalse);
      expect(range.allowsAll(range), isTrue);
    });

    test("allows a bordering range that's not more inclusive", () {
      var exclusive = new VersionRange(min: v010, max: v250);
      var inclusive = new VersionRange(
          min: v010, includeMin: true, max: v250, includeMax: true);
      expect(inclusive.allowsAll(exclusive), isTrue);
      expect(inclusive.allowsAll(inclusive), isTrue);
      expect(exclusive.allowsAll(inclusive), isFalse);
      expect(exclusive.allowsAll(exclusive), isTrue);
    });

    test('allows unions that are completely contained', () {
      var range = new VersionRange(min: v114, max: v200);
      expect(
          range.allowsAll(new VersionRange(min: v123, max: v124).union(v140)),
          isTrue);
      expect(
          range.allowsAll(new VersionRange(min: v010, max: v124).union(v140)),
          isFalse);
      expect(
          range.allowsAll(new VersionRange(min: v123, max: v234).union(v140)),
          isFalse);
    });
  });

  group('allowsAny()', () {
    test('disallows an empty constraint', () {
      expect(
          new VersionRange(min: v123, max: v250)
              .allowsAny(VersionConstraint.empty),
          isFalse);
    });

    test('allows allowed versions', () {
      var range = new VersionRange(min: v123, max: v250, includeMax: true);
      expect(range.allowsAny(v123), isFalse);
      expect(range.allowsAny(v124), isTrue);
      expect(range.allowsAny(v250), isTrue);
      expect(range.allowsAny(v300), isFalse);
    });

    test('with no min', () {
      var range = new VersionRange(max: v200);
      expect(range.allowsAny(new VersionRange(min: v140, max: v300)), isTrue);
      expect(range.allowsAny(new VersionRange(min: v234, max: v300)), isFalse);
      expect(range.allowsAny(new VersionRange(min: v140)), isTrue);
      expect(range.allowsAny(new VersionRange(min: v234)), isFalse);
      expect(range.allowsAny(range), isTrue);
    });

    test('with no max', () {
      var range = new VersionRange(min: v072);
      expect(range.allowsAny(new VersionRange(min: v003, max: v140)), isTrue);
      expect(range.allowsAny(new VersionRange(min: v003, max: v010)), isFalse);
      expect(range.allowsAny(new VersionRange(max: v080)), isTrue);
      expect(range.allowsAny(new VersionRange(max: v003)), isFalse);
      expect(range.allowsAny(range), isTrue);
    });

    test('with a min and max', () {
      var range = new VersionRange(min: v072, max: v200);
      expect(range.allowsAny(new VersionRange(min: v003, max: v140)), isTrue);
      expect(range.allowsAny(new VersionRange(min: v140, max: v300)), isTrue);
      expect(range.allowsAny(new VersionRange(min: v003, max: v010)), isFalse);
      expect(range.allowsAny(new VersionRange(min: v234, max: v300)), isFalse);
      expect(range.allowsAny(new VersionRange(max: v010)), isFalse);
      expect(range.allowsAny(new VersionRange(min: v234)), isFalse);
      expect(range.allowsAny(range), isTrue);
    });

    test('allows a bordering range when both are inclusive', () {
      expect(new VersionRange(max: v250).allowsAny(new VersionRange(min: v250)),
          isFalse);

      expect(new VersionRange(max: v250, includeMax: true)
              .allowsAny(new VersionRange(min: v250)),
          isFalse);

      expect(new VersionRange(max: v250)
              .allowsAny(new VersionRange(min: v250, includeMin: true)),
          isFalse);

      expect(new VersionRange(max: v250, includeMax: true)
              .allowsAny(new VersionRange(min: v250, includeMin: true)),
          isTrue);

      expect(new VersionRange(min: v250).allowsAny(new VersionRange(max: v250)),
          isFalse);

      expect(new VersionRange(min: v250, includeMin: true)
              .allowsAny(new VersionRange(max: v250)),
          isFalse);

      expect(new VersionRange(min: v250)
              .allowsAny(new VersionRange(max: v250, includeMax: true)),
          isFalse);

      expect(new VersionRange(min: v250, includeMin: true)
              .allowsAny(new VersionRange(max: v250, includeMax: true)),
          isTrue);
    });

    test('allows unions that are partially contained', () {
      var range = new VersionRange(min: v114, max: v200);
      expect(
          range.allowsAny(new VersionRange(min: v010, max: v080).union(v140)),
          isTrue);
      expect(
          range.allowsAny(new VersionRange(min: v123, max: v234).union(v300)),
          isTrue);
      expect(
          range.allowsAny(new VersionRange(min: v234, max: v300).union(v010)),
          isFalse);
    });
  });

  group('intersect()', () {
    test('two overlapping ranges', () {
      var a = new VersionRange(min: v123, max: v250);
      var b = new VersionRange(min: v200, max: v300);
      var intersect = a.intersect(b);
      expect(intersect.min, equals(v200));
      expect(intersect.max, equals(v250));
      expect(intersect.includeMin, isFalse);
      expect(intersect.includeMax, isFalse);
    });

    test('a non-overlapping range allows no versions', () {
      var a = new VersionRange(min: v114, max: v124);
      var b = new VersionRange(min: v200, max: v250);
      expect(a.intersect(b).isEmpty, isTrue);
    });

    test('adjacent ranges allow no versions if exclusive', () {
      var a = new VersionRange(min: v114, max: v124, includeMax: false);
      var b = new VersionRange(min: v124, max: v200, includeMin: true);
      expect(a.intersect(b).isEmpty, isTrue);
    });

    test('adjacent ranges allow version if inclusive', () {
      var a = new VersionRange(min: v114, max: v124, includeMax: true);
      var b = new VersionRange(min: v124, max: v200, includeMin: true);
      expect(a.intersect(b), equals(v124));
    });

    test('with an open range', () {
      var open = new VersionRange();
      var a = new VersionRange(min: v114, max: v124);
      expect(open.intersect(open), equals(open));
      expect(a.intersect(open), equals(a));
    });

    test('returns the version if the range allows it', () {
      expect(new VersionRange(min: v114, max: v124).intersect(v123),
          equals(v123));
      expect(new VersionRange(min: v123, max: v124).intersect(v114).isEmpty,
          isTrue);
    });
  });

  group('union()', () {
    test("with a version returns the range if it contains the version", () {
      var range = new VersionRange(min: v114, max: v124);
      expect(range.union(v123), equals(range));
    });

    test("with a version on the edge of the range, expands the range", () {
      expect(new VersionRange(min: v114, max: v124).union(v124),
          equals(new VersionRange(min: v114, max: v124, includeMax: true)));
      expect(new VersionRange(min: v114, max: v124).union(v114),
          equals(new VersionRange(min: v114, max: v124, includeMin: true)));
    });

    test("with a version allows both the range and the version if the range "
        "doesn't contain the version", () {
      var result = new VersionRange(min: v003, max: v114).union(v124);
      expect(result, allows(v010));
      expect(result, doesNotAllow(v123));
      expect(result, allows(v124));
    });

    test("returns a VersionUnion for a disjoint range", () {
      var result = new VersionRange(min: v003, max: v114)
          .union(new VersionRange(min: v130, max: v200));
      expect(result, allows(v080));
      expect(result, doesNotAllow(v123));
      expect(result, allows(v140));
    });

    test("considers open ranges disjoint", () {
      var result = new VersionRange(min: v003, max: v114)
          .union(new VersionRange(min: v114, max: v200));
      expect(result, allows(v080));
      expect(result, doesNotAllow(v114));
      expect(result, allows(v140));

      result = new VersionRange(min: v114, max: v200)
          .union(new VersionRange(min: v003, max: v114));
      expect(result, allows(v080));
      expect(result, doesNotAllow(v114));
      expect(result, allows(v140));
    });

    test("returns a merged range for an overlapping range", () {
      var result = new VersionRange(min: v003, max: v114)
          .union(new VersionRange(min: v080, max: v200));
      expect(result, equals(new VersionRange(min: v003, max: v200)));
    });

    test("considers closed ranges overlapping", () {
      var result = new VersionRange(min: v003, max: v114, includeMax: true)
          .union(new VersionRange(min: v114, max: v200));
      expect(result, equals(new VersionRange(min: v003, max: v200)));

      result = new VersionRange(min: v003, max: v114)
          .union(new VersionRange(min: v114, max: v200, includeMin: true));
      expect(result, equals(new VersionRange(min: v003, max: v200)));

      result = new VersionRange(min: v114, max: v200)
          .union(new VersionRange(min: v003, max: v114, includeMax: true));
      expect(result, equals(new VersionRange(min: v003, max: v200)));

      result = new VersionRange(min: v114, max: v200, includeMin: true)
          .union(new VersionRange(min: v003, max: v114));
      expect(result, equals(new VersionRange(min: v003, max: v200)));
    });

    test("includes edges if either range does", () {
      var result = new VersionRange(min: v003, max: v114, includeMin: true)
          .union(new VersionRange(min: v003, max: v114, includeMax: true));
      expect(result, equals(new VersionRange(
          min: v003, max: v114, includeMin: true, includeMax: true)));
    });
  });

  test('isEmpty', () {
    expect(new VersionRange().isEmpty, isFalse);
    expect(new VersionRange(min: v123, max: v124).isEmpty, isFalse);
  });
}
