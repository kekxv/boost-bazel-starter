# boost-bazel-starter

Starter project of boost (bazel) application.

See more:

- [registry.bazel.build](https://registry.bazel.build)
- [boost.asio](https://www.boost.org/libs/asio)
- [boost.asio bazel](https://registry.bazel.build/modules/boost.asio)

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
