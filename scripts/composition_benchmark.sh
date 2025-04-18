#!/bin/sh -e

BASEDIR=$(realpath "$(dirname "$0")")
ROOTDIR=$(realpath "$BASEDIR/..")

if [ -z "$1" ]; then
    echo "Usage: $0 <image_name>"
    echo "Note that image must be in the 'images' directory"
    exit 1
fi

IMAGE_NAME="$1"

sudo python3 "$ROOTDIR/tests/benchmarks/comparison_of_composition.py" "$IMAGE_NAME"
