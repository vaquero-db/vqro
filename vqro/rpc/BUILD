package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "vqro_cc_proto",
    srcs = [
        "controller.pb.cc",
        "core.pb.cc",
        "search.pb.cc",
        "storage.pb.cc",
    ],
    hdrs = [
        "controller.pb.h",
        "core.pb.h",
        "search.pb.h",
        "storage.pb.h",
    ],
    linkopts = [
        "-lm",
        "-lgrpc",
        "-lgrpc++",
    ],
    deps = [
        "//third_party/protobuf:protoc_lib",
    ],
)


cc_library(
    name = "search_service",
    srcs = [
        "search.grpc.pb.cc",
    ],
    hdrs = [
        "search.grpc.pb.h",
        "search_service.h",
    ],
    linkopts = [
        "-lm",
        "-lgrpc",
        "-lgrpc++",
    ],
    deps = [
        ":vqro_cc_proto",
        "//vqro/db",
        "//third_party/protobuf:protoc_lib",
    ],
)


cc_library(
    name = "storage_service",
    srcs = [
        "storage.grpc.pb.cc",
    ],
    hdrs = [
        "storage.grpc.pb.h",
        "storage_service.h",
    ],
    linkopts = [
        "-lm",
        "-lgrpc",
        "-lgrpc++",
    ],
    deps = [
        ":vqro_cc_proto",
        "//vqro/db",
        "//third_party/protobuf:protoc_lib",
    ],
)


cc_library(
    name = "controller_service",
    srcs = [
        "controller.grpc.pb.cc",
    ],
    hdrs = [
        "controller.grpc.pb.h",
        "controller_service.h",
    ],
    linkopts = [
        "-lm",
        "-lgrpc",
        "-lgrpc++",
    ],
    deps = [
        ":vqro_cc_proto",
        "//vqro/db",
        "//third_party/protobuf:protoc_lib",
    ],
)
