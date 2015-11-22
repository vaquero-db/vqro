#!/usr/bin/env python
'''
This script is intended to serve as a basic functional test of vqro-server,
vqro-read, and vqro-write.

To be honest most of the time this script catches flakiness in grpc. I have
faith grpc will get better over time at reliability...

TODO: Add a fancier test that stresses concurrency, reading and writing
simultaneously. Because the more often a test fails the more useful it is.
'''

import gflags
import json
import os
import shutil
import socket
import subprocess
import sys
import tempfile
import time
import traceback


FLOAT_ABSOLUTE_TOLERANCE = 1e-9


gflags.DEFINE_string(
    'testdata',
    'testdata/basic.json',
    'Path to file containing json output of generate_datapoints.py')
gflags.DEFINE_string(
    'bin_dir',
    '.',
    'Path to directory containing vqro binaries')
gflags.DEFINE_string(
    'tmp_dir',
    None,
    'Path to a writable directory in which a temp dir will be created for '
    'vqro-server to store data in')
gflags.DEFINE_bool(
    'preserve_on_error',
    True,
    'If true, do not delete the temp directory used by vqro-server when an '
    'error is encountered during the test.')

FLAGS = gflags.FLAGS


def RunCommand(cmd):
  cmd[0] = os.path.join(FLAGS.bin_dir or '.', cmd[0])
  print "Executing command: %s" % ' '.join(cmd)
  return subprocess.Popen(
      cmd,
      stdin=subprocess.PIPE,
      stdout=subprocess.PIPE,
      stderr=subprocess.STDOUT)


def StartServer(port, data_dir):
  vqro_server = RunCommand([
      'vqro-server',
      '--listen_port=%d' % port,
      '--data_directory=' + data_dir])

  for i in range(10):
    time.sleep(0.1)
    if vqro_server.poll() is not None:
      print "Failure, vqro-server returned code %d" % vqro_server.returncode
      print vqro_server.stdout.read()
      sys.exit(1)

  print "vqro_server started"
  return vqro_server


def KillServer(vqro_server):
  print 'Shutting down vqro-server... (pid=%d)' % vqro_server.pid
  vqro_server.terminate()
  vqro_server.wait()
  print 'vqro-server returned %d' % vqro_server.returncode


def EnsureEmptyDatabase(port, testdata):
  # Do a search and ensure there are no results
  first = json.loads(testdata[0])
  a_label = first['labels'].keys()[0]

  vqro_read = RunCommand([
      'vqro-read',
      '-server_port=%d' % port,
      '-search_series',
      '-json',
      '%s=~.*' % a_label])
  result_lines = vqro_read.stdout.readlines()
  vqro_read.wait()
  assert not result_lines, "Database not empty!"


def WriteTestData(port, testdata):
  print 'Writing test data...'
  vqro_write = RunCommand([
      'vqro-write',
      '-server_port=%d' % port])
  vqro_write.stdin.writelines(testdata)
  vqro_write.stdin.close()
  vqro_write.wait()


# Calling .readlines() on a pipe FD will only yield the lines that have already
# been written to the pipe. This function exists to ensure that we always read
# a stream until EOF.
def ReadAllLines(f):
  lines = []
  while True:
    newlines = f.readlines()
    if newlines:
      lines += newlines
    else:
      break
  return lines


def ReadBackData(port, testdata):
  print 'Reading back and checking test data...'
  first = json.loads(testdata[0])

  # Do labels search
  a_label = first['labels'].keys()[0]
  vqro_read = RunCommand([
      'vqro-read',
      '-server_port=%d' % port,
      '-search_labels',
      a_label])
  output = [line.strip() for line in vqro_read.stdout.readlines()]
  vqro_read.wait()
  assert a_label in output, "Search for label '%s' failed, vqro-read output=%s" % (
      a_label, output)

  # Do series search
  vqro_read = RunCommand([
      'vqro-read',
      '-server_port=%d' % port,
      '-search_series',
      '-json',
      '%s=~.*' % a_label])
  result_lines = vqro_read.stdout.readlines()
  vqro_read.wait()
  for result_line in result_lines:
    try:
      result = json.loads(result_line)
    except:
      print "Failed to decode vqro-read result line:"
      print '-' * 80
      print result_line
      print '-' * 80
      traceback.print_exc()
      continue
    if first['labels'] in result:
      break
  else:
    assert False, "Search for series %s failed" % str(first['labels'])

  # Read back all the data we wrote.
  vqro_read = RunCommand([
      'vqro-read',
      '-server_port=%d' % port,
      '-start', 'min',
      '-end', 'max',
      '-json',
      '-series_list_from_stdin'])

  for series_json in testdata:
    series = json.loads(series_json)
    vqro_read.stdin.write(json.dumps(series['labels']) + '\n')
  vqro_read.stdin.close()

  returned_data = ReadAllLines(vqro_read.stdout)
  print 'Comparing %d returned series data...' % len(returned_data)
  error_msg = ("Read validation failed:\nexpected %s\n  !=\nreturned %s" %
      (str(testdata)[:500], str(returned_data)[:500]))
  assert DatasetEquals(testdata, returned_data)
  return True


def DatasetEquals(dataset_a, dataset_b):
  for i, series_a_json in enumerate(dataset_a):
    series_a = json.loads(series_a_json)
    try:
      series_b = json.loads(dataset_b[i])
    except KeyError:
      print "missing i=%d" % i
      return False

    if not SeriesEquals(series_a, series_b):
      return False
  return True


def SeriesEquals(series_a, series_b):
  if series_a['labels'] != series_b['labels']:
    return False

  for i, a in enumerate(series_a['datapoints']):
    try:
      b = series_b['datapoints'][i]
    except KeyError:
      return False

  return (a[0] == b[0] and
          a[1] == b[1] and
          AlmostEquals(a[2], b[2]))


def AlmostEquals(float_a, float_b):
  return abs(float_a - float_b) < FLOAT_ABSOLUTE_TOLERANCE


def RunTest(port, testdata, data_dir):
  server = None
  ret = 1
  try:
    server = StartServer(port, data_dir)
    EnsureEmptyDatabase(port, testdata)
    WriteTestData(port, testdata)
    ReadBackData(port, testdata)
    ret = 0
  except StandardError:
    traceback.print_exc()
  finally:
    if server is not None:
      KillServer(server)

  return ret


def GetAvailablePort():
  for port in xrange(1025, 65000):
    try:
      s = socket.socket()
      s.bind(('', port))
      s.close()
      return port
    except:
      pass
  print "Failed to bind() to any ports!"
  sys.exit(1)


def main(argv):
  port = GetAvailablePort()
  testdata = open(FLAGS.testdata, 'r').readlines()
  data_dir = tempfile.mkdtemp(dir=FLAGS.tmp_dir)
  delete_data_dir = True

  try:
    return RunTest(port, testdata, data_dir)
  except:
    if FLAGS.preserve_on_error:
      delete_data_dir = False
    raise
  finally:
    if delete_data_dir:
      print 'Cleaning up data directory: %s' % data_dir
      shutil.rmtree(data_dir)
    else:
      print 'Preserving data directory: %s' % data_dir
      print 'You must clean this up yourself.'


if __name__ == '__main__':
  sys.exit(main(FLAGS(sys.argv)))
