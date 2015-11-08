This directory contains some tools that are useful in testing.


# Generating Test Data

To generate datapoints that can be sent to vqro-server using vqro-write there is
the `generate_datapoints.py` script. When stressing vqro-server it is often useful
to emphasize different characteristics in your test data, like having a lot of
unique series or fewer series with higher frequency of datapoints, etc. I also think
it is easier to reason about data when it looks realistic so to make it easy to generate
realistic looking data I came up with a kind of convoluted system but it's very easy
to work with once you understand how it works.

For example, let's say you want to generate some test data for server metrics
on Amazon EC2 instances. If you look in the `amazon_labels` directory you'll
see a bunch of text files containing label data. Every EC2 instance has an
instance ID, uses a particular image (AMI), it lives in a particular region,
in a particular zone. So to model that we have a file for each of the labels:
`instance`, `ami`, `region`, and `zone`. The contents of each file is a list
of values for that label. This is the test data for "entity labels", or any
labels that every "entity" has. In our example EC2 instances are the entities.

Each entity will have many metrics we want to measure about it. Unlike entity
labels which are always present, some metrics will have distinct labels of their
own. For instance a `bytes_written` metric might comes with a `dev` label that
specifies which disk device the bytes were written to. However an `uptime`
metric probably would not have a `dev` label. Basically the "label topology"
is trivial for entities but non-trivial for metrics.

To solve this we simply write a file called a _metrics template_ that specifies
the non-entity labels of all the metrics that each entity will have.

The idea is that we will invoke our script like so:

```
$ cd amazon_labels
$ ../generate_datapoints.py \
    --metrics_template=../machine.template \
    --entities=instance \
    --entity_labels=ami,region,zone \
    --num_datapoints=5
```

What it will do is read `machine.template` and learn all the metrics
that each entity has. Entities come from the `instance` file, where
each line corresponds to one entity that has a label `instance=<line>`.
Every entity will have an `ami` label with a value randomly chosen from the
`ami` file. Similarly for `region` and `zone`. It will generate 5 datapoints
for each entity.

Sample output looks like this.


```
rate{ami="ami-bc7de3", disk="/dev/sda1", instance="i-5de14b58", region="eu-central-1", resource="disk_write", time_unit="sec", unit="ops", zone="e"} 0+10=-57 -38 1.6 -29.4 -74
utilization{ami="ami-bc7de3", instance="i-86487a81", region="eu-west-1", resource="cpu_time", state="system", zone="a"} 0+10=-27 -92 -10.4 -1.85 -75
latency{ami="ami-f5d011", disk="/dev/sda1", instance="i-f34b156a", region="us-west-2", resource="disk_write", statistic="mean", zone="f"} 0+10=46 -12 -42 -81 -81
rate{ami="ami-cb5f98", disk="/dev/sda1", instance="i-96ffd6b3", region="ap-southeast-2", resource="disk_read", time_unit="sec", unit="ops", zone="d"} 0+10=85 8 30 -52 41
used{ami="ami-845ec8", instance="i-232966d3", region="eu-west-1", resource="memory", type[sum]="buffers", unit="MB", zone="a"} 0+10=-12 -25 -39 -65 -2
rate{ami="ami-845ec8", flow="rx", instance="i-232966d3", interface="eth0", region="eu-west-1", time_unit="sec", unit="packets", zone="a"} 0+10=18 -55 5.6 98 18
uptime{ami="ami-b9f320", instance="i-a939ff74", region="eu-west-1", zone="f"} 0+10=-48 8 -94 4 94
uptime{ami="ami-7807c1", instance="i-7ad9d2f2", region="ap-northeast-1", zone="a"} 0+10=-63 56 92 28 57
utilization{ami="ami-f5d011", instance="i-dae3317a", region="eu-central-1", resource="cpu_time", state="idle", zone="d"} 0+10=-6 -33 71 -43 89.07
total{ami="ami-836d01", filesystem="/", instance="i-c842b412", region="eu-central-1", resource="space", unit="MB", zone="b"} 0+10=-52 77 -22 -83 77
```

## Metrics Template File Syntax

Here's a snippet of `machine.template`:

```
# Disk I/O
~ <what>.<unit>/<time_unit>
rate.ops/sec {resource=disk_read, disk=/dev/sda1}
rate.MB/sec  {resource=disk_read, disk=/dev/sda1}
rate.ops/sec {resource=disk_write, disk=/dev/sda1}
rate.MB/sec  {resource=disk_write, disk=/dev/sda1}

~ <what>.<statistic>
latency.min  {resource=disk_read, disk=/dev/sda1}
latency.mean {resource=disk_read, disk=/dev/sda1}
latency.max  {resource=disk_read, disk=/dev/sda1}
latency.min  {resource=disk_write, disk=/dev/sda1}
latency.mean {resource=disk_write, disk=/dev/sda1}
latency.max  {resource=disk_write, disk=/dev/sda1}
```

Lines beginning with `#` are comments. Lines beginning with `~` define "the
current regex". You may have noticed that what follows the `~` is not regex
syntax. It is a short-hand notation that works like so:

```
The string "<what>.<unit>/<time_unit>" becomes the following regex:

(?P<what>[^<]+)\.(?P<unit>[^<]+)/(?P<time_unit>[^<]+)

The string "<what>.<statistic>" becomes:

(?P<what>[^<]+)\.(?P<statistic>[^<]+)
```

This makes it easy to express several labels of a metric in a concise and
natural format but also get structured labels for free. Given this input:

```
~ <what>.<unit>/<time_unit>
rate.ops/sec {resource=disk_read, disk=/dev/sda1}
```

We end up with this:

```
rate{disk="/dev/sda1", resource="disk_read", time_unit="sec", unit="ops"}
```

Then `generate_datapoints.py` sprinkles on some entity labels and voila.


## What?

The `what` label is ubiquitous. Every series in vaquero should have a
canonical label that is the "primary" one. Some people call it "name", others
call it "var". I am going with "what" for now. We'll see. The only thing
that actually cares about the primary label for now is `generate_datapoints.py`
but that will probably change soon. You have to specify the `what` label in
every series.


## Generating Entity Label Files

The `amazon_labels/instance` file would be pretty painful to write by hand,
and why have everyone re-write a script to generate appropriate random values?
That's where `generate_label_values.py` comes in. It is an extremely simple
and hacky script. There's a function for each label that generates a random
value appropriate for that label. You call the script by telling it what
label you want to generate values for and how many values you want.

Open up `generate_lable_values.py` and add a function like this.

```
def ticker():
  return ''.join(random.sample(string.letters, 4)).upper()
```

Now you can generate a file to model a hypothethical stock market of 10,000
companies like so.

```
$ ./generate_label_values.py ticker 10000 > ticker
```

Maybe you do another one for "sector", though that is probably easier
to just hand-write than to code. You can end up using the files like so:

```
$ ./generate_datapoints.py \
    --metrics_template=stock.template \
    --entities=ticker \
    --entity_labels=sector \
    --num_datapoints=5
```

Of course you'd have to write a `stock.template` file that enumerates all
the metrics you'd be measuring for each stock ticker.
