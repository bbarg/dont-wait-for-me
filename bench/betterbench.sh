#!/bin/bash
#
# usage: ./betterbench.sh <min rate> <max rate> <rate step> <time per test>

default_log="./betterbench.log"

if [[ $# -ne "4" ]]; then
    echo "usage: ./betterbench.sh <min rate> <max rate> <rate step> <time per test>"
    exit 1
fi

min_rate=$1
max_rate=$2
rate_step=$3
time_per_test=$4

server="128.59.19.36"		# os2 test server
port=8888
uri="/10b" 			# for now, adjust this manually
timeout=1			# " "

echo "-- Running test with"             >> $default_log
echo "\tmin rate:\t$min_rate"           >> $default_log
echo "\tmax rate:\t$max_rate"           >> $default_log
echo "\trate step:\t$rate_step"         >> $default_log
echo "\ttime per test:\t$time_per_test" >> $default_log

for rate in `seq -w $min_rate $rate_step $max_rate`; do

    echo "-- Running iteration with"    >> $default_log
    echo "\trate:\t$rate"               >> $default_log

    connections=$(( $rate * $time_per_test ))
    httperf --hog \
    	    --server $server \
    	    --port $port \
    	    --uri $uri \
    	    --num-conns $connections \
    	    --timeout $timeout
done

