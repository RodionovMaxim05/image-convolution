#!/bin/sh -e

find . -type f -name "*.c" -not \( -path "./tests/*" -o -path "./build/*" -o -path "./.venv/*" \) | xargs clang-tidy -p ./build
