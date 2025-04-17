#!/bin/sh -e

BASEDIR=$(realpath "$(dirname "$0")")
ROOTDIR=$(realpath "$BASEDIR/..")

BUILD_TYPE="Debug"

if [ "$1" = "--release" ]; then
    BUILD_TYPE="Release"
else
    echo "No build type specified, defaulting to Debug."
fi

cmake -S . -B "$ROOTDIR/build" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$ROOTDIR/build"
