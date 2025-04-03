#!/bin/sh -e

find . -type f -name "*.c" -not \( -path "./tests/*" -o -path "./build/*" \) | xargs clang-tidy -p ./build
