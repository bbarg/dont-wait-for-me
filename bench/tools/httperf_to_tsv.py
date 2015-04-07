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

values = ['connection_rate_per_sec',
          'connection_rate_ms_conn',
          'connection_time_min',
          'connection_time_avg',
          'connection_time_max',
          'connection_time_median'] # put the values you wish to print here

if len(sys.argv) != 2:
    print("usage: python2.7 httperf_to_tsv.py <httperf dump file>\n")
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

with open('out.tsv', 'wb') as outFile:
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

        
