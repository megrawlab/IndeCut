# Generate plots for the number of samples vs. cutnorm estimates
import os, sys, copy, subprocess, scipy, scipy.linalg
import numpy as np
import multiprocessing
from multiprocessing import Pool
from itertools import *

#################################
# Cut norm functions
def calc_cutnorm(file_path_max_ent, S, file_path_output, CSDP_exec_path, i):
    #Read in files
    Z = np.genfromtxt(file_path_max_ent, delimiter=',')
    num_rows = np.shape(Z)[0]
    num_columns = np.shape(Z)[1]
    num_var = num_rows+num_columns
    if np.shape(Z) != np.shape(S):
        print("Error: shape of matrices are not the same")
        sys.exit(2)
    #Make input file for CSDP
    base_path = os.path.dirname(os.path.abspath(file_path_output))
    CSDP_input_file = os.path.join(base_path, os.path.splitext(os.path.basename(file_path_output))[0]+'_'+str(i)+'_CSDPinput.dat-s')
    CSDP_output_file = os.path.join(base_path, os.path.splitext(os.path.basename(file_path_output))[0]+'_'+str(i)+'_CSDPoutput.txt')
    fid = open(CSDP_input_file,'w')
    D = Z-S
    is_exact = (D>=0).all()
    if is_exact:
        exact_cutnorm = float(D.sum()/4.0)
    make_CSDP_input(fid,D)
    fid.close()
    #Run CSDP
    #cmd = CSDP_exec_path + " -ds " + CSDP_input_file + " -o " + CSDP_output_file + " " + "-numThreads " + str(num_threads) + " -dimacs -pt 1" #-pt 1 fast but unstable, -pt 2 stable/slow, -pt 0 default
    cmd = CSDP_exec_path + " " + CSDP_input_file + " " + CSDP_output_file
    res = subprocess.check_output(cmd, shell = True)
    #Get primal value from res
    CSDP_primal_value = float(find_between(res,"Primal objective value: ", " \n"))/2
    #Parse CSDP output
    fid = open(CSDP_output_file,"r")
    CSDP_output = fid.read()
    fid.close()
    CSDP_output_split = CSDP_output.split("\n")
    #First line is optimal y, matrix 1 is optimal Z, matrix 2 is optimal X
    Z = np.zeros((num_var,num_var))
    X = np.zeros((num_var,num_var))
    for line in CSDP_output_split[1:]:
        line_split = line.split(' ') #csdp output has a trailing blank line
        if line_split[0] != '':
            if float(line_split[0]) == 1:
                i = int(line_split[2])-1 #csdp output uses 1-based indexing
                j = int(line_split[3])-1
                val = float(line_split[4])
                Z[i,j] = val
                Z[j,i] = val
            elif float(line_split[0]) == 2:
                i = int(line_split[2])-1
                j = int(line_split[3])-1
                val = float(line_split[4])
                X[i,j] = val
                X[j,i] = val
    Y = X #I was using Y before, so this is just me being lazy
    #eigen_values,eigen_vectors = np.linalg.eig(Y) #could probably use eigh
    #eigen_values,eigen_vectors = scipy.linalg.eigh(Y,eigvals=(num_var-2,num_var-1)) #assumes Y symmetric, use this and a loop if it's too inneficient to compute ALL the eigenvalues, just loop through getting the top k until the are <=0
    eigen_values,eigen_vectors = scipy.linalg.eigh(Y)
    idx = eigen_values.argsort()[::-1]
    eigen_values = eigen_values[idx]
    all_eigen_values = eigen_values[:]
    eigen_vectors = eigen_vectors[:,idx]
    for positive_index in range(len(eigen_values)):
        if eigen_values[positive_index]<=0:
            break
    eigen_values = eigen_values[0:positive_index]
    eigen_vectors = eigen_vectors[:,0:positive_index]
    soln = np.zeros(np.shape(eigen_vectors))
    for i in range(len(eigen_values)):
        soln[:,i] = np.sqrt(eigen_values[i])*eigen_vectors[:,i] #coords/variables are row-wise. eg: [u1;u2;v1;v2]
    #Perform Alon & Noar rounding
    approx_opt = 0
    uis_opt = list()
    vjs_opt = list()
    for dummy in range(1,10000): #I'll use the max value, can make this a parameter later.
        G = np.zeros(positive_index)
        for i in range(len(G)):
            G[i] = np.random.normal()
        rounded_soln = np.sign(soln.dot(G))
        uis = rounded_soln[0:num_rows]
        vjs = rounded_soln[num_rows:num_rows+num_columns]
        #Calculate approximation
        approx = 0
        for i in range(num_rows):
            for j in range(num_columns):
                approx = approx + D[i,j]*uis[i]*vjs[j]
        approx = np.abs(approx)
        if approx > approx_opt:
            approx_opt = copy.copy(approx)
            uis_opt = copy.copy(uis)
            vjs_opt = copy.copy(vjs)
    if is_exact:
        return (exact_cutnorm, exact_cutnorm) #Exact since all entries nonnegative
    else:
        return (approx_opt/4, CSDP_primal_value/4)


def make_CSDP_input(fid, D):
    n_rows = np.shape(D)[0]
    n_columns = np.shape(D)[1]
    num_var = n_columns+n_rows
    fid.write("\"Example\"\n")
    fid.write("%d = mDIM\n" % num_var)
    fid.write("1 = nBLOCK\n")
    fid.write("%d = bLOCKsTRUCT\n" % num_var)
    for it in range(n_rows+n_columns):
        fid.write("1 ")
    fid.write("\n")
    Is = list()
    Js = list()
    for i in range(n_rows):
        for j in range(n_columns):
            fid.write("0 1 %d %d %f\n" % (i+1, j+n_rows+1, D[i,j]))
            Is.append(i+1)
            Js.append(j+n_rows+1)
    Is = list(set(Is))
    Js = list(set(Js))
    for i in Is:
        fid.write("%d 1 %d %d 1\n" % (i,i,i))
    for j in Js:
        fid.write("%d 1 %d %d 1\n" % (j,j,j))


def find_between(s, first, last):
    try:
        start = s.index(first) + len(first)
        end = s.index(last, start)
        return s[start:end]
    except ValueError:
        return ""


##################################
# Functions to create the plot data
def do_n_star(args):
    return do_n(*args)


# Parallelize it
def do_n(n, graph_directory, files_sorted):
    for i in range(n):
        fid = open(os.path.join(graph_directory, files_sorted[i]), 'r')
        #print(files_sorted[i])
        rows = []
        columns = []
        for line in fid.readlines():
            line_split = line.strip().split()
            rows.append(int(line_split[0]))
            columns.append(int(line_split[1]))
        #print(rows)
        #print(columns)
        fid.close()
        rows = np.array(rows)
        columns = np.array(columns)
        columns = columns - max(rows) - 1
        matrix = np.zeros((max(rows) + 1, max(columns) + 1))
        if len(rows) != len(columns):
            raise Exception("Error, unequal number of rows in .igraph")
        for j in range(len(rows)):
            matrix[rows[j], columns[j]] = 1
        matrices.append(matrix)
    S = np.zeros((max(rows) + 1, max(columns) + 1))
    for matrix in matrices:
        S = S + matrix
    S = S / float(len(matrices))
    (left_end, right_end) = calc_cutnorm(file_path_max_ent, S, file_path_output, CSDP_exec_path, i)
    return [left_end, right_end]

##################################
graph = 'Encode_comoF_mir-GENE'
zout = 'Encode_comoF_mir-GENE'
thisdir = '/nfs0/BPP/Megraw_Lab/mitra/Projects/NetMotif/WaRSwap_uniformity/test_independence'
nsamples_total = 5000
nsubsamples = 50

algDict = {}
algDict["fanmod"] = thisdir + '/fanmod/fn_out/' + graph + '.5000/' + graph + '.5000.rand.fanmod'
algDict['warswap'] = thisdir + '/warswap/wr_out/' + graph + '/' + graph + '.rand.igraph'
algDict['diamcis'] = thisdir + '/diamcis/di_out/' + graph + '.5000/' + graph + '.5000.rand.diamcis'
algDict['comofinder'] = thisdir + '/comofinder/cf_out/' + graph + '.5000'

#method = 'fanmod'
for method in ['fanmod', 'warswap', 'diamcis', 'comofinder']:
    if method == 'fanmod':
        graph_directory = algDict['fanmod']
    elif method == 'warswap':
        graph_directory = algDict['warswap']
    elif method == 'diamcis':
        graph_directory = algDict['diamcis']
    elif method == 'comofinder':
        graph_directory = algDict['comofinder']
        
    file_path_max_ent = thisdir + '/output/' + zout + '_Z_5000.csv'
    CSDP_exec_path = '/nfs1/Koslicki_Lab/koslickd/CSDP/Csdp-6.1.1/solver/csdp'
    
    dum_outdir = 'cutnorm_dummies/' + graph  
    if not os.path.exists(dum_outdir):
        os.makedirs(dum_outdir)
        
    file_path_output = dum_outdir + "/dum"
    files = os.listdir(graph_directory)
    if method == 'warswap':
        graph_nums = [int(file.split('.')[2]) for file in files]
    elif method == 'fanmod' or method == 'diamcis' or method == 'comofinder':
        graph_nums = [int(file.split('.')[-1]) for file in files]
    files_sorted = [i[1] for i in sorted(zip(graph_nums, files))]
    matrices = []
    endpoints = []
    pool = Pool(processes=multiprocessing.cpu_count())
    do_list = range(1, nsamples_total, nsubsamples)
    res = pool.map(do_n_star, izip(do_list, repeat(graph_directory), repeat(files_sorted)), chunksize=1)
    pool.close()
    pool.terminate()
    
    outdir = thisdir + "/samples_vs_cutnorms_out"
    if not os.path.exists(outdir):
        os.makedirs(outdir)
    
    outfile = outdir + "/" + graph + "_" + method + "_" + str(nsamples_total) + "_" + str(nsubsamples) + ".txt"
    fid = open(outfile, 'w')
    for item in res:
        fid.write('%f,%f\n' % (item[0], item[1]))
    fid.close()

