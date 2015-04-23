import os
import csv

def rows_from_filename(name):
    name = name[:-4]            # chop of the '.tsv'
    return name.split('_')      # fields delimited by '_'

header = ''
all_rows = []

for subdir, dirs, files in os.walk("./"):
    for f in files:
        with open(f, 'rb') as csvfile:
            if f[-4:] != ".tsv":
                continue
            reader = csv.reader(csvfile, delimiter="\t")

            rows = [row for row in reader]
            if not header:
                header = rows[0]
            rows = rows[1:]     # chop off header
            rows = [rows_from_filename(f) + row for row in rows]

            print "- Done processing " + f

            all_rows = all_rows + rows

with open('big-table.tsv','wb') as csvfile:
    writer = csv.writer(csvfile, delimiter="\t")

    header = ['server','ncores','nthreads','filesize'] + header
    writer.writerow(header)
    for row in all_rows:
        writer.writerow(row)
