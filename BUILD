# http://bazel.io/docs/build-encyclopedia.html

cc_binary(
    name = "vqro-server",
    args = [
        "--alsologtostderr",
    ],
    srcs = ["vqro_server.cc"],
    deps = [
        "//vqro/db",
        "//vqro/rpc:vqro_cc_proto",
    ],
    linkopts = [
        "-lgpr",
        "-lgrpc",
        "-lgrpc++",
        "-ltcmalloc",
    ],
)


cc_binary(
    name = "vqro-write",
    srcs = ["vqro_write.cc"],
    deps = [
        "//vqro/base",
        "//vqro/rpc:vqro_cc_proto",
    ],
    linkopts = [
        "-lgpr",
        "-lgrpc",
        "-lgrpc++",
        "-ljsoncpp",
        "-ltcmalloc",
    ],
)


cc_binary(
    name = "vqro-read",
    srcs = ["vqro_read.cc"],
    deps = [
        "//vqro/base",
        "//vqro/rpc:vqro_cc_proto",
    ],
    linkopts = [
        "-lgpr",
        "-lgrpc",
        "-lgrpc++",
        "-ljsoncpp",
        "-lre2",
        "-ltcmalloc",
    ],
)
