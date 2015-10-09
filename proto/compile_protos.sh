#!/bin/bash
#
# This script is a temporary hack until bazel has better out-of-the-box
# support for compiling protobufs for C++.
#
# See https://github.com/bazelbuild/bazel/issues/52
#
# In the mean time a genproto() rule that actually works would be a
# welcome replacement.

VQRO_ROOT=$(python -c "from os.path import *; print dirname(dirname(abspath('$0')))")

# Specify PROTOC and GRPC_CPP_PLUGIN paths as environment variables if the
# binaries aren't on your PATH.
PROTOC="${PROTOC:-protoc}"
GRPC_CPP_PLUGIN="${GRPC_CPP_PLUGIN:-grpc_cpp_plugin}"

function run() {
  echo $*
  $*
}

function build_proto() {
  filename=$1
  run cd $VQRO_ROOT/proto
  run $PROTOC --cpp_out=../vqro/rpc/ $filename || exit $?
  if grep -qi "^service " "$filename"; then
    run $PROTOC --plugin=protoc-gen-grpc=`which $GRPC_CPP_PLUGIN` --grpc_out=../vqro/rpc/ $filename || exit $?
  fi
  run cd -
}

for p in *.proto; do
  build_proto $p
done
echo "Generated protobuf and grpc code successfully."
