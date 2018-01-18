'''Supplementary Software 3 A possible implementation of the Curveball Algorithm in Python programming language'''

from random import sample, randint, shuffle
import numpy as np


def find_presences(input_matrix):
	num_rows, num_cols = input_matrix.shape
	hp = []
	iters = num_rows if num_cols >= num_rows else num_cols
	input_matrix_b = input_matrix if num_cols >= num_rows else np.transpose(input_matrix)
	for r in range(iters):
		hp.append(list(np.where(input_matrix_b[r] == 1)[0]))
	return hp


def curve_ball(input_matrix, r_hp, num_iterations=-1):
	num_rows, num_cols = input_matrix.shape
	l = range(len(r_hp))
	num_iters = 5*min(num_rows, num_cols) if num_iterations == -1 else num_iterations
	for rep in range(num_iters):
		AB = sample(l, 2)
		a = AB[0]
		b = AB[1]
		ab = set(r_hp[a])&set(r_hp[b]) # common elements
		l_ab=len(ab)
		l_a=len(r_hp[a])
		l_b=len(r_hp[b])
		if l_ab not in [l_a,l_b]:		
			tot=list(set(r_hp[a]+r_hp[b])-ab)
			ab=list(ab)	
			shuffle(tot)
			L=l_a-l_ab
			r_hp[a] = ab+tot[:L]
			r_hp[b] = ab+tot[L:]
	out_mat = np.zeros(input_matrix.shape, dtype='int8') if num_cols >= num_rows else np.zeros(input_matrix.T.shape, dtype='int8')
	for r in range(min(num_rows, num_cols)):
		out_mat[r, r_hp[r]] = 1
	result = out_mat if num_cols >= num_rows else out_mat.T
	return result
