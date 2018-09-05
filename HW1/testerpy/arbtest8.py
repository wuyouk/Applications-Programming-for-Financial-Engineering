#!/usr/bin/python

import sys
from arbwriter import writelp
from mysolver import lpsolver
import random
import numpy as np

if len(sys.argv) != 3:
    sys.exit("usage: arbtest.py datafilename lpfilename\n")

#now open and read data file
try:

    datafile = open(sys.argv[1], 'r') # opens the data file
except IOError:
    sys.exit("Cannot open file %s\n" % sys.argv[1])

lines = datafile.readlines();
datafile.close()

#print lines[0]
firstline = lines[0].split()
#print "first line is", firstline

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
    # print "line number", k+1,"is", thisline
    # should check that the line contains numsec + 1 words

    j = 1

    p[k*(1 + numsec)] = 1 + r*(k != 0)

    while j <= numsec:
        value = float(thisline[j])
        p[k*(1 + numsec) + j] = value
#        print " sec ", j, " -> ", p[k*(1 + numsec) + j]
        j += 1
    k += 1

#now write LP file, now done in a separate function (should read data this way, as well)
lpwritecode = writelp(sys.argv[2], p, numsec, numscen)

print "wrote LP to file", sys.argv[2],"with code", lpwritecode

#now solve lp

# Assuming that there is arbitrage, suppose that x^* (x) is the vector of positions
lpsolvecode, x = lpsolver(sys.argv[2], "test.log")
print "solved LP at", sys.argv[2],"with code", lpsolvecode

scores = []

random.seed(1)
# 100 trials
for i in range(100):

    score = 0
    # for each scenario
    for s in range(1, numscen + 1):
        prices = p[s * (numsec + 1) : (1 + s) * (numsec + 1)]
        for i, v in enumerate(prices):
            # randomly draw new prices from range (0.95, 1.05)
            if i != 0:
                tem = random.uniform(-0.05, 0.05)

                prices[i] = v * (1+tem)
        # compute new portfolio value in this scenario
        value = np.dot(x, prices)
        # increase score if portfolio value is positive
        if value > 0:
            score += 1
    # collect score for this trial
    scores += [score]

# draw histogram of scores

print 'score'.ljust(8), 'counts'
for i in range(0, numscen + 1):
    count = scores.count(i)
    print str(i).ljust(8), str(count).ljust(3), '|',
    for c in range(count):
        sys.stdout.write('=')
    print ''

# output histogram to output.txt

try:
    opfile = open('output.txt', 'w')
except IOError:
    print("Cannot open output file %s for writing\n")

opfile.write('score'.ljust(8))
opfile.write('counts\n')
for i in range(0, numscen + 1):
    count = scores.count(i)

    opfile.write(str(i).ljust(8))
    opfile.write(str(count).ljust(3))
    opfile.write('|')
    for c in range(count):
        opfile.write('=')
    opfile.write('\n')
