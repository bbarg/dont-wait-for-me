#!/bin/bash

source ../perf-functions

function start_latency_workload () {
    port=$1
    timeout 30s httperf --hog --server localhost --port $port --uri /281b --num-conns 10000 \
	     &> httperf-$server-$NTHREADS.log &
    export httperf_pid=$!
}

# run with full ncores since we know that performance is flat across
# ncores with latency workload

for nthreads in 1 2 4 8 16 32 64
do
    for server in $servers
    do
	echo "-- Running test for $server with $nthreads threads"
	port=$(( ($RANDOM % 10000) + 2000 ))

	start_server $server $nthreads $port # sets $server_pid and $NTHREADS
	start_latency_workload $port 
	sleep 1
	start_record $server	# relies on $server_pid
	wait $httperf_pid $perf_pid
    done
done
