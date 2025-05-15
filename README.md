# boost-bazel-starter

Starter project of boost (bazel) application.

See more:

- [registry.bazel.build](https://registry.bazel.build)
- [boost.asio](https://www.boost.org/libs/asio)
- [boost.asio bazel](https://registry.bazel.build/modules/boost.asio)

## boost.mysql support

build mysql need ssl support, so we need select `boringssl` or `openssl` as ssl library.
> "Support for OpenSSL is temporarily unavailable(`boost.mysql 1.87`). OpenSSL support will be added in version `1.88` after Windows support is finalized via (https://github.com/bazelbuild/bazel-central-registry/pull/4436)."

use :
```shell
bazel build --@boost.asio//:ssl=boringssl --@boost.mysql//:ssl=boringssl example/mysql:tutorial_sync
```

or add .bazelrc 
```
build --@boost.asio//:ssl=boringssl
build --@boost.mysql//:ssl=boringssl
```

## Overview

### Project layout

```
|- MODULE.bazel                          // Bazel module file 
|- app/
|    |
|    |- App.cpp                          // main() is here
|    |- BUILD.bazel                      // Bazel target file for app
|- patches                               // Patch files to apply to third-party
```

---

### Build and Run

#### Using Bazel

**Requires**

- `bazel` installed. choose one:
    - https://github.com/bazelbuild/bazel
    - https://github.com/bazelbuild/bazelisk

```shell
# build and run app
bazel run //app:app
```
