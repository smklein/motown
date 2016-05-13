## 1.0.5

* Narrow the dependency on `path`. Previously, this allowed versions that didn't
  support all the functionality this package needs.

* Upgrade to the new test runner.

## 1.0.4

* Added overlooked `collection` dependency.

## 1.0.3

* Fix a bug where `Glob.list()` and `Glob.listSync()` would incorrectly throw
  exceptions when a directory didn't exist on the filesystem.

## 1.0.2

* Fixed `Glob.list()` on Windows.

## 1.0.1

* Fix several analyzer warnings.

* Fix the tests on Windows.
