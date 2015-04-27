#!/bin/bash

set -e

dumps="/home/ruchir/dont-wait-for-me/bench/max_request_latency/script_dump/"
servers="http-server lcrq-server msq"

for server in $servers
do
    for file in `ls $dumps/$server/`
    do
	python2.7 httperf_to_tsv.py $dumps/$server/$file $file.tsv
	echo "-- Done processing $file"
    done
done
