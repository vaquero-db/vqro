
cc_library(
    name = "main",
    srcs = glob(
        ["src/*.cc"],
        exclude = ["src/gtest-all.cc"]
    ) + glob(["src/*.h"]),
    hdrs = glob(["include/**/*.h"]),
    copts = [
        "-Iexternal/gtest",
        "-Iexternal/gtest/include"
    ],
    includes = ["include"],
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
)
