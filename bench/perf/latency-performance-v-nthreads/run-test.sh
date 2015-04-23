#!/bin/bash

source ../perf-functions

function start_latency_workload () {
    httperf --hog --server localhost --port 8888 --uri /281b --num-conns 10000 \
	     &> httperf-$server-$NTHREADS.log &
    export $httperf_pid=$!
}

# run with full ncores since we know that performance is flat across
# ncores with latency workload

for nthreads in "1 2 4 8 16 32 64"
do
    for server in $servers
    do
	echo "-- Running test for $server with $nthreads threads"
	start_server $server $nthreads # sets $server_pid and $NTHREADS
	start_latency_workload
	sleep 1
	start_report $server	# relies on $server_pid
	wait $server_pid $httperf_pid
    done
done
