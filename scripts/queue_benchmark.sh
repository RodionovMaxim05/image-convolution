#!/bin/sh -e

BASEDIR=$(realpath "$(dirname "$0")")
ROOTDIR=$(realpath "$BASEDIR/..")

if [ -z "$1" ]; then
    echo "Usage: $0 <num_of_imgs> <mem_lim>"
    echo "Note that images must be in the 'input_queue_mode'"
    exit 1
fi

NUM_OF_IMGS="$1"
MEM_LIM="$2"

python3 "$ROOTDIR/tests/benchmarks/comparison_of_queue_mode.py" "$NUM_OF_IMGS" "$MEM_LIM"
