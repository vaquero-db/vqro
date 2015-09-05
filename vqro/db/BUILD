package(default_visibility = ["//visibility:public"])


cc_library(
    name = "db",
    srcs = [
        "datapoint_directory.cc",
        "datapoint_directory.h",
        "datapoint_file.h",
        "db.cc",
        "dense_file.cc",
        "dense_file.h",
        "read_op.h",
        "series.cc",
        "series.h",
        "search_engine.cc",
        "search_engine.h",
        "sparse_file.cc",
        "sparse_file.h",
        "sql_statement.h",
        "write_buffer.cc",
        "write_buffer.h",
        "write_op.h",
    ],
    hdrs = [
        "datapoint.h",
        "db.h",
    ],
    deps = [
        "//vqro/base",
        "//vqro/rpc:vqro_cc_proto",
    ],
)
