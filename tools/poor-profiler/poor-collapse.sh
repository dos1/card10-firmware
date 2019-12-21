#!/usr/bin/env bash

cat "$1" | \
    sort | \
    uniq -c | \
    sort -r -n -k 1,1 | \
    sed 's/^ \+\([0-9]\+\) \(.*\)$/\2 \1/g' | \
    sort
