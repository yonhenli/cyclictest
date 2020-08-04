#!/bin/bash

# Run Cylictest in the kernel.

if [ $# -ne 3 ]; then
        echo "Usage: $0 <cpu> <duration> <output>"
        exit 1
fi
cpu=$1
duration=$2
output=$3

if [ ! -d "trace" ]; then
        echo "tracefs?"
        exit 1
fi

echo '' > trace/trace
taskset -c $cpu make insert
sleep $duration
make remove
cat trace/trace > $output
