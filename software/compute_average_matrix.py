import numpy as np
import sys, os
import math
from numpy import dtype, float32

def create_net_from_file(degseq_file, input_dir, outfile):
    assert isinstance(degseq_file, basestring), degseq_file
    assert isinstance(input_dir, basestring), input_dir
    assert isinstance(outfile, basestring), outfile
    
    fp = open(degseq_file, "r")
    nsources = len(fp.readline().split(","))
    ntargets = len(fp.readline().split(","))
    
    sum_matrix = np.zeros((nsources, ntargets), dtype = float32)
    ngraphs = len(os.listdir(input_dir))
    for randfile in os.listdir(input_dir):
        infile = open(input_dir + "/" + randfile, "r")
        for line in iter(infile):
            line = line.rstrip()
            src = int(line.split("\t")[0])
            tgt = int(line.split("\t")[1])
            sum_matrix[src, tgt - nsources] = sum_matrix[src, tgt - nsources] + 1
        
    sum_matrix = sum_matrix / ngraphs
    np.savetxt(outfile, sum_matrix, delimiter = ",")
    return sum_matrix
    
if __name__ == '__main__':
    print "main"
    in_arr =  sys.argv[1:]
    indir = in_arr[0]
    degseq_file = in_arr[1]
    outfile = in_arr[2]
    create_net_from_file(degseq_file, indir, outfile)
