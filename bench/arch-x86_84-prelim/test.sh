#!/bin/bash

echo "----- Running test for arch-x86_64-prelim -----"

if [[ -z $1 ]]; then
    echo "usage: ./test.sh <output file>"
    exit 1
fi

OUTFILE=$1
function decho () {
    echo $1
    echo $1 >> $OUTFILE
}

set -e

cd ../../lcrq-server/
make clean
make

# Parameters
servers='http-server lcrq-server msq'
rates='50 100 125 150 175 200'

for server in servers; do

    decho "----- testing $server ----"

    # start server
    ./$server 8888 ~/html &> /dev/null &

    # warmup
    decho "----- warmup for $server -----"
    httperf --hog \
	    --server localhost \
	    --port 8888 \
	    --uri /10.html \
	    --rate 100 \
	    --num-conn 1000 \
	    --num-call 1 &> $OUTFILE
    
    for rate in rates; do
	decho "----- rate $rate for $server -----"
	httperf --hog \
		--server localhost \
		--port 8888 \
		--uri /10.html \
		--rate $rate \
		--num-conn 5000 \
		--num-call 1 &> $OUTFILE
	decho ""
    done

    killall $server &> /dev/null
    decho "----- done with testing for $server -----"
done

