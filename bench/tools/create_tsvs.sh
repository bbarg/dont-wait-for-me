#!/bin/bash

set -e

dumps=$1
servers="http-server-signal http-server-broadcast lcrq-server-signal lcrq-server-broadcast msq-server-signal msq-server-broadcast apache nginx lighttpd"

for server in $servers
do
    for file in `ls $dumps/$server/`
    do
	python2.7 httperf_to_tsv.py $dumps/$server/$file $file.tsv
	echo "-- Done processing $file"
    done
done
