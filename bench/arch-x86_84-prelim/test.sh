#!/bin/bash

if [[ -z $1 ]]; then
    echo "usage: ./test.sh <output file>"
    exit 1
fi

OUTFILE=`pwd`/$1
touch $OUTFILE
function decho () {
    echo $1
    echo $1 >> $OUTFILE
}

function die () {
    echo -n "Are you sure you want to exit the test [Y/n]? "
    read res
    if [[ res -ne "n" ]]; then
	exit 1
    fi
}

set -e
trap die SIGINT

echo "----- Running test for arch-x86_64-prelim -----"

cd ../../lcrq-server/
make clean
make

# Parameters
servers='http-server lcrq-server msq'
rates='50 100 125 150 175 200'

for server in $servers; do

    decho "----- testing $server ----"

    # start server
    random_port=$(( ($RANDOM % 8000) + 2000 ))
    ./$server $random_port ~/html &

    # warmup
    decho "----- warmup for $server -----"
    httperf --hog \
	    --server localhost \
	    --port $random_port \
	    --uri /10.html \
	    --rate 100 \
	    --num-conn 1000 \
	    --num-call 1 &> $OUTFILE
    
    for rate in $rates; do
	decho "----- rate $rate for $server -----"
	httperf --hog \
		--server localhost \
		--port $random_port \
		--uri /10.html \
		--rate $rate \
		--num-conn 5000 \
		--num-call 1 &> $OUTFILE
	decho ""
    done

    killall $server &> /dev/null
    decho "----- done with testing for $server -----"
done

