#!/usr/bin/env bash

nsamples=4096
sleeptime=0.05

for x in $(seq 1 $nsamples); do
    printf "Capturing %6u/%u ...\n" "$x" "$nsamples" >&2
    arm-none-eabi-gdb -nx -x init.gdb -ex "set pagination 0" -ex "bt" -ex "continue&" -batch build/epicardium/epicardium.elf 2>/dev/null | \
        grep -E '^#' | \
        tac | \
        sed 's/^#[0-9]\+ \+0x.* in \(.*\) (.*$/\1/g;s/^#[0-9]\+ \+\(.*\) (.*$/\1/g' | \
        tr '\n' ';' | \
        sed 's/^\(.*\);$/\1\n/g'
    sleep $sleeptime
done
