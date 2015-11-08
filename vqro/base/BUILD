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
