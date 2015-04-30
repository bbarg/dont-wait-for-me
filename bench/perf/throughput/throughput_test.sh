#!/bin/bash

user=ben
test_server='128.59.19.36'
test_dir='/home/'$user'/dont-wait-for-me/bench/perf/throughput/'
dump_file='httperf.dump'

function run_one_test() {
    local server=$1
    local nthreads=$2
    local port=$3
    local uri=$4
    local events=$5
    local rate=$6
    local timeout=$7
    local num_conns=$8

    local command=' \
      cd $test_dir; \
      . ../perf-functions; \
      start_server $server $nthreads $port; \
      sleep 2; \
      perf record -e $events -p $pid -- sleep 5 &'

    ssh $user@$test_server $command

    timeout 30s -- \
	    httperf --hog --server $server --port $port --uri $uri \
	    --rate $rate --num-conns $num_conns --timeout $timeout \
	    --think-timeout 0 &>> $dump_file

    ssh $user@$test_server 'killall $server'
}
