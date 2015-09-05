package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "vqro_cc_proto",
    srcs = [
        "vqro.pb.cc",
        "vqro.grpc.pb.cc",
    ],
    hdrs = [
        "vqro.pb.h",
        "vqro.grpc.pb.h",
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
