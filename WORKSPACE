
bind(
    name = "protobuf_clib",
    actual = "//third_party/protobuf",
)

new_http_archive(
    name = "gtest",
    url = "https://googletest.googlecode.com/files/gtest-1.7.0.zip",
    sha256 = "247ca18dd83f53deb1328be17e4b1be31514cedfc1e3424f672bf11fd7e0d60d",
    build_file = "gtest.BUILD",

    ### WARNING: The 'strip_prefix' feature only works with very recent bazel
    # binaries (as of Nov 8 2015 I had to build from trunk for it to work).
    # If your attempts to run tests fail this may be why.
    strip_prefix = "gtest-1.7.0",
)
