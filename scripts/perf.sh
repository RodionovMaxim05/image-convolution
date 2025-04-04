#!/bin/sh -e

BASEDIR=$(realpath "$(dirname "$0")")
ROOTDIR=$(realpath "$BASEDIR/..")

sudo python3 "$ROOTDIR/tests/benchmarks/comparison_using_perf.py"