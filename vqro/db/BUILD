package(default_visibility = ["//visibility:public"])


cc_library(
    name = "db",
    srcs = [
        "constant_file.cc",
        "constant_file.h",
        "datapoint_buffer.h",
        "datapoint_directory.cc",
        "datapoint_directory.h",
        "datapoint_file.cc",
        "datapoint_file.h",
        "db.cc",
        "dense_file.cc",
        "dense_file.h",
        "raw_buffer.h",
        "read_op.h",
        "series.cc",
        "series.h",
        "search_engine.cc",
        "search_engine.h",
        "sparse_file.cc",
        "sparse_file.h",
        "sql_statement.h",
        "storage_optimizer.cc",
        "storage_optimizer.h",
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
    linkopts = [
        "-lre2",
        "-lsqlite3",
    ],
)
