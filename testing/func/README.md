This is where functional test scripts go.

The `basic_server_func.py` script is intended to cover all the APIs but only
serially. I need to make a fancier test that emphasizes concurrency and
stress on resources.

To run the test, cd to the root of your vqro checkout and run:

```
$ cd ~/git/vqro/  # I assume everyone does it this way
$ bazel build :all
$ testing/func/basic_server_func.py --bin_dir=bazel-bin --testdata=testing/func/testdata/basic.json
```

Observe the wondrous functional magic (or fix whatever broke).

This test has flaked on me a few times, where by "flaked" I mean exposed the
existence of bugs that I could not track down because of a lack of logging.
