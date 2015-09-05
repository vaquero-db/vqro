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
        "-lglog",
        "-lgflags",
        "-lgpr",
        "-lgrpc",
        "-lgrpc++",
        "-lpthread",
        "-lre2",
        "-lsqlite3",
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
        "-lglog",
        "-lgflags",
        "-lgpr",
        "-lgrpc",
        "-lgrpc++",
        "-ljsoncpp",
        "-lpthread",
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
        "-lglog",
        "-lgflags",
        "-lgpr",
        "-lgrpc",
        "-lgrpc++",
        "-ljsoncpp",
        "-lpthread",
        "-lre2",
        "-ltcmalloc",
    ],
)
