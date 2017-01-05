import numpy as np
import sys, os
import math
from numpy import dtype, float32

nsources = 0
ntargets = 0
def create_net_from_file(input_dir, outfile):
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
    src_outdeg = in_arr[1]
    tgt_indeg = in_arr[2]
    outfile = in_arr[3]
    nsources = len(src_outdeg.split(","))
    ntargets = len(tgt_indeg.split(","))
    create_net_from_file(indir, outfile)
