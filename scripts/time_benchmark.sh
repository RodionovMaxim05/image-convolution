#!/bin/sh -e

BASEDIR=$(realpath "$(dirname "$0")")
ROOTDIR=$(realpath "$BASEDIR/..")

python3 "$ROOTDIR/tests/benchmark/comparison_of_times.py"