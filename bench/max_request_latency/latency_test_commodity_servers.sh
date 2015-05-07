#!/bin/bash 

server='localhost' 
#server_name=$1
#starting_port=$2
#path="/home/ruchir/dont-wait-for-me/bench/max_request_latency/script_dump/"$server_name"/"
time=600


iter_requests() {



local server_name=$1
local ncores=8
local path="/home/ruchir/dont-wait-for-me/bench/max_request_latency/script_dump/"$server_name"/"
local req_file=$2
local port=$3 
local nthreads=8
local uri="/"$2


local filename=$server_name"_"$ncores"_"$nthreads"_"$req_file 

echo "Testing " $filename

for i in 10 100  
do 
    timeout 60s httperf --hog --server $server --port $port --uri $uri --num-conns $i --timeout $time > /dev/null 2>&1

done 
num_conns=10000

#echo "Testing $server_name ncores: $ncores uri: $uri nthreads: $nthreads port $port num-conns $num_conns"
#timeout 60s httperf --hog --server $server --port $port --uri $uri --num-conns $num_conns --timeout $time >> $path$filename 2>/dev/null 

for i in {0..7..1}
do
    #echo $i
    echo "Testing $server_name ncores: $ncores uri: $uri nthreads: $nthreads port $port num-conns $num_conns"
    timeout 60s httperf --hog --server $server --port $port --uri $uri --num-conns $num_conns --timeout $time >> $path$filename 2>/dev/null 
    sleep 45 
    
done 

}



iter_nthreads() {

local ncores=$1
local uri=$2
local port=$starting_port

for i in 1 4 8 16 64
do
    echo "NTHREADS: " $i "PORT: " $port  
    export NTHREADS=$i
    local core_range=$((ncores-1)) 
    echo "taskset -c 0-$core_range $server_name $port ~/html/ >>/dev/null 2>&1 &" 
    taskset -c 0-$core_range $server_name $port ~/html/ >>/dev/null 2>&1 &
    iter_requests $ncores $uri $port $i 
    killall $server_name
    ((port++))
    
done 

}


iter_uris() {

local ncores=$1 

for uri in "281b" 
do    
    echo "NCORES: " $ncores "URI: " $uri 
    iter_nthreads $ncores $uri 

done
}

iter_ncores() { 

for cores in 8 
do 
    iter_uris $cores 
done

}

#iter_ncores

#lighttpd -D -f ~/lighttpd.conf &  
#iter_requests lighttpd 281b 2222
#killall lighttpd 

sudo nginx 
iter_requests nginx 281b 4444
sudo killall nginx 


sudo apache2ctl start 
iter_requests apache 281b 3333
sudo apache2ctl stop 





