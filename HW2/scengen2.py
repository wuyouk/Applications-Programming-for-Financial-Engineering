#!/usr/bin/python

import sys
import math
import random

if len(sys.argv) < 5:
    print('Usage: scengen.py numsec numscen r outfile')
    exit(0)
print sys.argv

numsec = int(sys.argv[1])
numscen = int(sys.argv[2])
r = float(sys.argv[3])

thisoutput = open(sys.argv[4], "w")



print "numsec ", numsec, " numscen ", numscen, " r", r

thisoutput.write("securities " + str(numsec) + " scenarios " + str(numscen) + " r " + str(r) + "\n")

price0 = [0.0]*(1 + numsec)

price0[0] = 1
j = 1
while j <= numsec:
    draw1 = random.uniform(0,1)
    base = 10
    if draw1 < .5:
        base = 0
    top = base + 20
    mid = .5*(base + top)
    draw2 = random.uniform (base, top)
    price0[j] = math.ceil(1000*draw2)/1000.0
    j += 1

#write today's prices
thisoutput.write("today: ")
j = 1
while j <= numsec:
    thisoutput.write(str(price0[j]) + " ")
    j += 1
thisoutput.write("\n")
k = 1
while k <= numscen:
    thisoutput.write("scen_"+str(k)+": ")
    j = 1
    while j <= numsec:
        newvalue = (1 + r - 0.5*random.uniform(0,1))*price0[j]
        newvalue = math.ceil(1000*newvalue)/1000.0
        thisoutput.write(str(newvalue) + " ")
        j += 1
    thisoutput.write("\n")
    k += 1

#compute and write other prices
thisoutput.write("\nEnd\n")
thisoutput.close()
