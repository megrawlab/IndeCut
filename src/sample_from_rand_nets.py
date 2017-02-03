import sys
import numpy as np
import shutil
import os

if __name__ == '__main__':
    args = sys.argv[1:]
    ntotal = int(args[0])
    nsamples = int(args[1])
    outdir = args[2]
    inbase = args[3]
    rand_file_suffix = args[4]
    
    rand_index_arr = np.random.choice(ntotal, nsamples, replace = False)
    for rand_idx in iter(rand_index_arr):
        if rand_idx == 0:
            rand_idx = 1
        src_file = inbase + str(rand_idx) + rand_file_suffix
        shutil.copy(src_file, outdir)
    
    