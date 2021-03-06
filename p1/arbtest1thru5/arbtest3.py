#!/usr/bin/python

import sys

if len(sys.argv) != 3:
    sys.exit("usage: arbtest.py datafilename lpfilename\n")

#now open and read data file
try:
    datafile = open(sys.argv[1], 'r') # opens the data file
except IOError:
    sys.exit("Cannot open file %s\n" % sys.argv[1])

lines = datafile.readlines();
datafile.close()

print lines[0]
firstline = lines[0].split()
print "first line is", firstline

numsec = int(firstline[1])
numscen = int(firstline[3])
r = float(firstline[5])
print "\n"
print "number of securities:", numsec,"number of scenarios", numscen,"r",r
print "\n"

#allocate prices as one-dim array
p = [0]*(1 + numsec)*(1 + numscen)
k = 0
# line k+1 has scenario k (0 = today)
while k <= numscen:
    thisline = lines[k + 1].split()
    print "line number", k+1,"is", thisline
    # should check that the line contains numsec + 1 words
    j = 1
    print "scenario", k,":"
    p[k*(1 + numsec)] = 1 + r*(k != 0)
    while j <= numsec:
        value = float(thisline[j])
        p[k*(1 + numsec) + j] = value
        print " sec ", j, " -> ", p[k*(1 + numsec) + j]
        j += 1
    k += 1
