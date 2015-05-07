# httperf_to_tsv.py
#
# Requires the httperfpy package, which can be installed with
#
#   sudo pip install httperfpy
#
# usage: python httperf_to_tsv.py <httperf dump file>

from httperfpy import HttperfParser as parser
import sys
import csv

# all possible values to extract
values = ['total_connections','total_requests','total_replies','total_test_duration','connection_rate_per_sec','connection_time_avg','connection_time_max','reply_time_response','reply_time_transfer','reply_size_header','reply_size_content','reply_size_footer','reply_size_total','reply_status_1xx','reply_status_2xx','reply_status_3xx','reply_status_4xx','reply_status_5xx','cpu_time_user_sec','cpu_time_system_sec','cpu_time_user_pct','cpu_time_system_pct','cpu_time_total_pct','net_io_kb_sec','net_io_bps','errors_total','errors_client_timeout','errors_socket_timeout','errors_conn_refused','errors_conn_reset','errors_fd_unavail','errors_addr_unavail','errors_ftab_full','errors_other']

if len(sys.argv) != 3:
    print("usage: python2.7 httperf_to_tsv.py <httperf dump file> <output tsv>\n")
    exit(1)

with open(sys.argv[1], 'rb') as dumpFile:

    perfStrings = []
    currentString = ""

    for line in dumpFile:
        if "httperf: warning: " in line: # this is the FD_SETSIZE error
            continue
        if "httperf " in line:
            if currentString:   # don't add an empty string 
                perfStrings.append(currentString)
                currentString = ""

        currentString += line

    dumpFile.close()

with open(sys.argv[2], 'wb') as outFile:
     writer = csv.writer(outFile, delimiter='\t')

     writer.writerow(values)

     for string in perfStrings:
         results = parser.parse(string)
         row = []
         for v in values:
             try:
                 row.append(results[v])
             except KeyError:
                 row.append('N/A')
         writer.writerow(row)

        
