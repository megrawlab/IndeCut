# -*- coding: utf-8 -*-
"""
Spyder Editor

This is a temporary script file.
"""
import numpy as np
import sys

infile_v = sys.argv[1]
infile_e = sys.argv[2]
outfile = sys.argv[3]

f = open(infile_v, "r")
lines = f.readlines()
f.close()

n1 = len(lines[0].rstrip().split(","))
n2 = len(lines[1].rstrip().split(","))

m = np.zeros(shape = (n1,n2), dtype = int)

f = open(infile_e, "r")
for line in iter(f):
    line = line.rstrip()
    print(line)
    e = line.split("\t")
    m[int(e[0]),int(e[1]) - n1] = 1

np.savetxt(outfile,m, delimiter=",",fmt = "%d")
