#!/bin/sh -e

find . -type f \( -name "*.c" -o -name "*.h" \) \
       -not \( -path "./stb_image/*" -o -path "./build/*" -o -path "./.venv/*" \) | \
xargs -n1 sh -c '
    clang-format --dry-run --Werror "$1" || {
        echo "Formatting error in $1"
        exit 1
    }
' _
