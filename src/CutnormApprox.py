import os, sys, shutil, subprocess, getopt, math, copy
import numpy as np
from cStringIO import StringIO
import scipy
import scipy.linalg

# file_path_max_ent='MedBilayerG1_Z.csv'
# file_path_sample='MedBilayerG1_warswap.out_E_out.csv'
# CSDP_exec_path = "/home/david/Desktop/CSDP/CSDP-7.3.8/./CSDP"
# file_path_output='test.txt'


def main(argv):
	file_path_max_ent = None
	file_path_sample = None
	file_path_output = None
	CSDP_exec_path = "CSDP"
	try:
		opts, args = getopt.getopt(argv, "m:s:o:e:", ["MaxEntropyMatrix=", "SampleAveMatrix=", "Output=","CSDPExecutable="])
	except getopt.GetoptError:
		print 'Call using: python CutnormApproxCSDP.py -m <MaxEntropyMatrix.csv> -s <SampleAveMatrix.csv> -o <Output.txt> -e <ExecutableForCSDP>'
		sys.exit(2)
	for opt, arg in opts:
		if opt == '-h':
			print 'Call using: python CutnormApproxCSDP.py -m <MaxEntropyMatrix.csv> -s <SampleAveMatrix.csv> -o <Output.txt> -e <ExecutableForCSDP>'
			sys.exit(2)
		elif opt in ("-m", "--MaxEntropyMatrix"):
			file_path_max_ent = arg
		elif opt in ("-s", "--SampleAveMatrix"):
			file_path_sample = arg
		elif opt in ("-o", "--Output"):
			file_path_output = arg
		elif opt in ("-e", "--CSDPExecutable"):
			CSDP_exec_path = arg
	#Run the main program
	calc_cutnorm(file_path_max_ent, file_path_sample, file_path_output,CSDP_exec_path)

def calc_cutnorm(file_path_max_ent, file_path_sample, file_path_output,CSDP_exec_path):
	assert isinstance(file_path_max_ent,basestring), file_path_max_ent
#	assert isinstance(file_path_sample,basestring), file_path_sample
	assert isinstance(file_path_output,basestring), file_path_output
	
	#Read in files
	Z = np.genfromtxt(file_path_max_ent,delimiter=',')
	if file_path_sample == None:
		S = 0*Z
	else:
		S = np.genfromtxt(file_path_sample,delimiter=',')
	num_rows = np.shape(Z)[0]
	num_columns = np.shape(Z)[1]
	num_var = num_rows+num_columns
	if np.shape(Z) != np.shape(S):
		print("Error: shape of matrices are not the same")
		sys.exit(2)
	#Make input file for CSDP
	base_path = os.path.dirname(os.path.abspath(file_path_output))
	CSDP_input_file = os.path.join(base_path,os.path.splitext(os.path.basename(file_path_output))[0]+'_CSDPinput.dat-s')
	CSDP_output_file = os.path.join(base_path,os.path.splitext(os.path.basename(file_path_output))[0]+'_CSDPoutput.txt')
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
	#y_mat_text = find_between(CSDP_output,"yMat = \n{\n{ ", "   }\n}\n    main").replace("{","").replace("}","").replace(" ","").replace(",\n","\n") 	#Yes, the CSDP output format (if you can call it that) is a BEAR to parse!
	#y_mat_text = find_between(CSDP_output,"xMat = \n{\n{ ", "   }\n}\nyMat").replace("{","").replace("}","").replace(" ","").replace(",\n","\n") 	#Yes, the CSDP output format (if you can call it that) is a BEAR to parse!
	#Y = np.genfromtxt(StringIO(y_mat_text),delimiter=',')
	
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

	#Save output
	fid = open(file_path_output,'w')
	fid.write("#Approximation of infinity-to-one norm\n")
	if is_exact:
		fid.write("%f\n" % (exact_cutnorm*4.0))
	else:
		fid.write("%f\n" % approx_opt)
	
	fid.write("#Interval of cut norm approximation\n")
	#fid.write("[%f,%f]\n" % (approx_opt/4, np.minimum(CSDP_primal_value/4, approx_opt/(4*(4/np.pi-1))))) #bounds from Alon and Noar 2004 paper
	if is_exact:
		fid.write("[%f,%f]\n" % (exact_cutnorm, exact_cutnorm)) #Exact since all entries nonnegative
	else:
		fid.write("[%f,%f]\n" % (approx_opt/4, CSDP_primal_value/4)) #bounds from Alon and Noar 2004 paper will always be larger than the SDR norm
	
#	fid.write("#Ymat\n") #Too large to save
#	for i in range(num_var):
#		for j in range(num_var):
#			if j == num_var-1:
#				fid.write("%f" % Y[i,j])
#			else:
#				fid.write("%f," % Y[i,j])
#		fid.write("\n")

	fid.write("#ui's (rows)\n")
	for i in range(len(uis_opt)):
		if i == len(uis_opt)-1:
			fid.write("%f\n" % uis_opt[i])
		else:
			fid.write("%f," % uis_opt[i])

	fid.write("#vj's (columns)\n")
	for i in range(len(vjs_opt)):
		if i == len(vjs_opt)-1:
			fid.write("%f\n" % vjs_opt[i])
		else:
			fid.write("%f," % vjs_opt[i])

	fid.write("#Eigenvalues. If there are only two distinct eigenvalues, then the lower bound of the approximation of the cut norm is exact\n")
	for i in range(len(all_eigen_values)):
		if i == len(all_eigen_values)-1:
			fid.write("%f\n" % all_eigen_values[i])
		else:
			fid.write("%f," % all_eigen_values[i])


	fid.close()


#Attempt at primal, can't figure out what's wrong with it...so let's do the dual
#def make_CSDP_input(fid,D):
#	n_rows = np.shape(D)[0]
#	n_columns = np.shape(D)[1]
#	num_var = n_columns*n_rows
#	fid.write("\"Example\"\n")
#	fid.write("%d = mDIM\n" % num_var)
#	fid.write("1 = nBLOCK\n")
#	fid.write("%d = bLOCKsTRUCT\n" % num_var)
#	#for it in range(n_rows*n_columns):
#	#	fid.write("1 ")
#	for i in range(n_rows):
#		for j in range(n_columns):
#			fid.write("%f " % -D[i,j]) #minimize the negative => maximize
#	fid.write("\n")
#	for i in range(num_var):
#		fid.write("0 1 %d %d -1\n" % (i+1,i+1)) #Make sure the inner products are 1 (u_1 . u_1 = 1)
#	iter = 1
#	for i in range(n_rows):
#		for j in range(n_columns):
#			fid.write("%d 1 %d %d 1\n" % (iter, i+1, j+n_rows+1)) #Make sure the x_i's correspond to entries in the variable matrix X
#			iter = iter + 1

def make_CSDP_input(fid,D):
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




def find_between( s, first, last ):
    try:
        start = s.index( first ) + len( first )
        end = s.index( last, start )
        return s[start:end]
    except ValueError:
        return ""




if __name__ == "__main__":
	main(sys.argv[1:])

