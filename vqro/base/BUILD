package(default_visibility = ["//visibility:public"])

cc_library(
    name = "base",
    srcs = [
        "base.cc",
        "fileutil.cc",
    ],
    hdrs = [
        "base.h",
        "fileutil.h",
        "floatutil.h",
        "worker.h",
    ],
    linkopts = [
        "-lglog",
        "-lgflags",
        "-lm",
        "-lpthread",
        "-ltcmalloc",
    ],
)


cc_test(
    name = "base_test",
    size = "small",
    srcs = ["base_test.cc"],
    deps = [
        ":base",
        "@gtest//:main",
    ],
)


cc_test(
    name = "worker_test",
    size = "small",
    srcs = ["worker_test.cc"],
    deps = [
        ":base",
        "@gtest//:main",
    ],
)


cc_test(
    name = "fileutil_test",
    size = "small",
    srcs = ["fileutil_test.cc"],
    deps = [
        ":base",
        "@gtest//:main",
    ],
)


cc_test(
    name = "floatutil_test",
    size = "small",
    srcs = ["floatutil_test.cc"],
    deps = [
        ":base",
        "@gtest//:main",
    ],
)
