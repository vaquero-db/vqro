# http://bazel.io/docs/build-encyclopedia.html

cc_binary(
    name = "vqro-server",
    args = [
        "--alsologtostderr",
    ],
    srcs = ["vqro_server.cc"],
    deps = [
        "//vqro/db",
        "//vqro/rpc:search_service",
        "//vqro/rpc:storage_service",
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
    name = "vqro-controller",
    args = [
        "--alsologtostderr",
    ],
    srcs = ["vqro_controller.cc"],
    deps = [
        "//vqro/control:controller",
        "//vqro/rpc:controller_service",
        "//vqro/rpc:search_service",
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
        "//vqro/rpc:storage_service",
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
        "//vqro/rpc:search_service",
        "//vqro/rpc:storage_service",
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
