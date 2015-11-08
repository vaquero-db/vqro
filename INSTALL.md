# vqro
Vaquero consists of 3 binaries:

- `vqro-server` - The vaquero database server.
- `vqro-read` - A client for reading / searching a vqro-server via rpc.
- `vqro-write` - A client for writing datapoints to a vqro-server via rpc.

Both clients speak json over stdio so they should be pretty easy to integrate
with other programs. Since vaquero uses gRPC it is very easy to use vaquero
from [a bunch of languages](http://www.grpc.io/) already.


# Dependencies
- Linux
- [Bazel](https://github.com/bazelbuild/bazel) (required to build it, not to run it)
- [gflags](https://github.com/gflags/gflags)
- [glog](https://github.com/google/glog)
- [gperftools](https://github.com/gperftools/gperftools)
- [Protocol Buffers v3](https://github.com/google/protobuf)
- [gRPC](https://github.com/grpc/grpc)
- [RE2](https://github.com/google/re2)
- [sqlite3](https://www.sqlite.org/)
- [JsonCpp](https://github.com/open-source-parsers/jsoncpp)


# Is there a DEB/RPM package?

Not yet. Vaquero depends on a few bleeding edge libraries like protobuf 3 and
gRPC. Once there are distribution packages for those I will put one together.
So for now you have to compile it.


# Setting up your bazel workspace

Bazel is an awesome build system. This is coming from a person who absolutely
loathes build systems. I could rant for hours about the short-comings of
most build systems but in the case of bazel, it will only take a second.

With bazel it is pretty easy to manage code dependencies as long as they are
either being built by bazel or are already installed on your system in a
standard location (ie. `/usr`). It is much less convenient when you have
headers/libraries installed into non-standard locations (ie. `/opt`). So to
minimize complexity of our `BUILD` files I have written them under the
assumption that all of Vaquero's dependencies (except protobuf 3) are installed
in a standard system location so no extra compiler hints (`-I` or `-L`) are
needed.

Once you checkout the vqro repository you will need to create a symlink under
the `third_party` directory called `protobuf` that points to your checkout of
[https://github.com/google/protobuf].

Here is a quick overview of the steps to get you up and running:

```
$ GIT_DIR="<where-i-put-my-git-checkouts>"

# First things first, download and build bazel
$ cd $GIT_DIR
$ git clone https://github.com/bazelbuild/bazel
$ cd bazel
$ ./compile.sh  # this creates your bazel binary under ./output/

# You probably want to copy the bazel binary into a directory in your $PATH


# Now let's check out vaquero and set up the tools symlink
$ cd $GIT_DIR
$ git clone https://github.com/vaquero-db/vqro
$ ln -s $GIT_DIR/bazel/tools $GIT_DIR/vqro/

# Download protobuf and link it into third_party
$ cd $GIT_DIR
$ git clone https://github.com/google/protobuf
$ ln -s $GIT_DIR/protobuf $GIT_DIR/vqro/third_party/
```

At this point you're pretty much ready to compile vaquero, except you need to
make sure you have all the dependencies installed on your system. Here is a
list of ubuntu packages you'll need (plus the usual build tool chain, g++, etc):

- libgflags-dev
- libgoogle-glog-dev
- libgoogle-perftools-dev
- libre2-dev
- libsqlite3-dev
- libjsoncpp-dev
- libgflags-dev
- libgtest-dev
- zlib1g-dev

Additionally you will need to build and install gRPC (into `/usr/local` or some
other location on your system include/library path). The steps for doing that
are laid out here, [https://github.com/grpc/grpc/blob/master/INSTALL].


```
# Time to compile vqro
$ cd $GIT_DIR/vqro
$ bazel build :all  # creates binaries under ./bazel-bin/
```

That should get you up and running. In the future the process will be simpler.
