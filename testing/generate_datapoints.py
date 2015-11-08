#!/usr/bin/python
'''
Read README.md for details about using this script.
'''

import os
import random
import sys
import gflags
import re
import json


gflags.DEFINE_string('metrics_template', None, 'Metrics template filename')
gflags.DEFINE_string('entities', None, 'File containing primary entity labels')
gflags.DEFINE_list('entity_labels', None, 'Files containing secondary entity labels')
gflags.DEFINE_integer('duration', 10, 'Default duration for all datapoints')
gflags.DEFINE_integer('num_datapoints', 1, 'How many random datapoints to '
                      'generate for each series')
gflags.DEFINE_bool('sparse', False, 'If true, --duration is ignored and randomized')
gflags.DEFINE_integer('base_timestamp', 0, 'Timestamp of first generated datapoint')
gflags.DEFINE_bool('json', False, 'Output in json format')

gflags.MarkFlagAsRequired('metrics_template')
gflags.MarkFlagAsRequired('entities')
gflags.MarkFlagAsRequired('entity_labels')

FLAGS = gflags.FLAGS


def Entries(filename):
  for line in open(filename):
    line = line.strip()
    if line and not line.startswith('#'):
      yield line


def Memoize(func):
  results = {}
  def wrapper(*args):
    if args not in results:
      results[args] = func(*args)
    return results[args]
  return wrapper


def ParseTemplate(filename):
  labels_list = []
  current_regex = None

  for line in open(filename):
    line = line.strip()

    if line.startswith('#'):
      continue

    if not line:
      current_regex = None
      continue

    if line.startswith('~ '):
      current_regex = CreateRegex(line[2:].strip())
      continue

    # Awkward way of saying labels are optional.
    try:
      what_expr, labels_expr = line.split(None, 1)
    except ValueError:
      what_expr = line
      labels = {}
    else:
      labels = ParseLabels(labels_expr)

    if current_regex:
      match = current_regex.search(what_expr)
      assert match, "regex=\"%s\" failed: %s" % (current_regex.pattern, what_expr)
      labels.update(match.groupdict())
    else:
      labels['what'] = what_expr

    labels_list.append(labels)

  return labels_list


def ParseLabels(labels_expr):
  labels = {}
  match = re.match('{(.*?)}', labels_expr)
  assert match, "Invalid labels expression: %s" % labels_expr

  inner_expr = match.group(1)

  skip_space = r'\s*'
  quoted_string = r'"(?:\"|[^"])*?"' # works for all non-pathological cases
  unquoted_string = r'''[a-zA-Z_/][^'" {},=]*'''
  either_string = r'(%s|%s)' % (quoted_string, unquoted_string)
  equals = r'\s*=\s*'
  maybe_comma = r'\s*,?'
  extract_label = re.compile(
    skip_space +
    either_string +
    equals +
    either_string +
    maybe_comma)

  matched_bytes = 0
  for match in extract_label.finditer(inner_expr):
    key = match.group(1).strip('"')
    value = match.group(2).strip('"')
    labels[key] = value
    matched_bytes += match.end() - match.start()

  if matched_bytes < len(inner_expr):
    raise ValueError("Failed to parse labels: %s" % labels_expr)

  return labels


regexifier = re.compile('<([a-z0-9_-]+)>', re.I)

@Memoize
def CreateRegex(pattern):
  for char in '.+*?':
    pattern = pattern.replace(char, '\\' + char)
  re_pattern = regexifier.sub(r'(?P<\1>[^<]+)', pattern)  # read until next <pattern>
  return re.compile(re_pattern)


def RandomValue():
  value = random.randint(-100, 100)
  if random.random() > 0.8:
    value *= random.random()
  return value
 

def GenerateDatapoints():
  if FLAGS.sparse:
    timestamp = FLAGS.base_timestamp
    for i in xrange(FLAGS.num_datapoints):
      duration = 1
      if random.random() > 0.5:
        duration = random.randint(1, 15)
      yield (timestamp, duration, RandomValue())
      timestamp += duration + random.randint(1, 100)
  else:
    base_timestamp = FLAGS.base_timestamp - (FLAGS.base_timestamp % FLAGS.duration)
    for i in xrange(FLAGS.num_datapoints):
      yield (i * FLAGS.duration + base_timestamp, FLAGS.duration, RandomValue())


def main(argv):
  template_labels_list = ParseTemplate(FLAGS.metrics_template)

  ent_labels = {os.path.basename(filename) : list(Entries(filename))
               for filename in FLAGS.entity_labels}

  # "key" == "primary label"
  ent_key = os.path.basename(FLAGS.entities)
  for ent_key_value in Entries(FLAGS.entities):
    base_labels = {ent_key: ent_key_value}

    for ent_label, values in ent_labels.iteritems():
      base_labels[ent_label] = random.choice(values)

    for template_labels in template_labels_list:
      labels = base_labels.copy()
      labels.update(template_labels)
      datapoints = list(GenerateDatapoints())

      if FLAGS.json:
        obj = {'labels': labels, 'datapoints': datapoints}
        print json.dumps(obj)
        continue

      what = labels.pop('what')
      labels_text = what + '{%s}' % ', '.join('%s="%s"' % item for item in sorted(labels.items()))

      if FLAGS.sparse:
        datapoints_text = ' '.join('%d+%d=%s' % point if point[1] != 1 else '%d:%s' % (point[0], point[2])
                                   for point in datapoints)
      else:
        first_timestamp = datapoints[0][0]
        datapoints_text = '%d+%d=' % (first_timestamp, FLAGS.duration) + ' '.join(
            str(v) for t,d,v in datapoints)
      print labels_text, datapoints_text


if __name__ == '__main__':
  main(FLAGS(sys.argv))
