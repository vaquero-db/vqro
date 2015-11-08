#!/usr/bin/python
'''
Read README.md for details about using this script.
'''

import random
import sys


def hex_string(size):
  return ''.join(random.choice('0123456789abcdef') for _ in range(size))


def instance():
  return 'i-' + hex_string(8)


def ami():
  return 'ami-' + hex_string(6)


what = sys.argv[1]
count = int(sys.argv[2])
for i in range(count):
  print eval(what + '()')
