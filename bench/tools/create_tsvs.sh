#!/bin/bash

set -e

dumps="/home/ruchir/dont-wait-for-me/bench/max_request_latency/script_dump/"
servers="http-server-signal http-server-broadcast lcrq-server-signal lcrq-server-broadcast msq-server-signal msq-server-broadcast"

for server in $servers
do
    for file in `ls $dumps/$server/`
    do
	python2.7 httperf_to_tsv.py $dumps/$server/$file $file.tsv
	echo "-- Done processing $file"
    done
done
