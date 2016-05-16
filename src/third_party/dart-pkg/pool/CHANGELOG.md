## 1.2.1

* Internal changes only.

## 1.2.0

* Add `Pool.close()`, which forbids new resource requests and releases all
  releasable resources.

## 1.1.0

* Add `PoolResource.allowRelease()`, which allows a resource to indicate that it
  can be released without forcing it to deallocate immediately.

## 1.0.2

* Fixed the homepage.

## 1.0.1

* A `TimeoutException` is now correctly thrown if the pool detects a deadlock.
