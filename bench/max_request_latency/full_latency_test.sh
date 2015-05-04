#!/bin/bash 

./latency_test.sh http-server-signal 8888 >> http-server.out 2>&1 
./latency_test.sh msq-server-signal 9999 >> msq-server.out 2>&1 
./latency_test.sh lcrq-server-signal 7777 >> lcrq-server.out 2>&1

./latency_test.sh http-server-broadcast 8888 >> http-server.out 2>&1 
./latency_test.sh msq-server-broadcast 9999 >> msq-server.out 2>&1 
./latency_test.sh lcrq-server-broadcast 7777 >> lcrq-server.out 2>&1 
