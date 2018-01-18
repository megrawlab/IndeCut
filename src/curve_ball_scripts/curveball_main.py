'''Supplementary Software 2 Python script to generate a set of null matrices using the Curveball Algorithm'''


from optparse import OptionParser
import sys
import os
import csv
from csv import Error as SniffError
import numpy as np

from scanutils import add_column_header, has_header, guess_delimiter
import curveball as cb


def read_input_w_no_delimiters(csvfile):
    delimiter = None
    row_names = None
    lineterminator = '\n'
    column_names = []
    matrix_started = False
    csvfile.seek(0)
    matrix = []
    for line in csvfile:
        line = line.strip()
        if not line[0] in ('0', '1') and not matrix_started:
            column_names.append(line)
        else:
            matrix_started = True
            matrix.append(np.array(list(line), dtype='int8'))
    matrix = np.array(matrix)
    return matrix, delimiter, lineterminator, row_names, column_names


def with_no_delimiters(csvfile):
    wo_delimiters = False
    csvfile.seek(0)
    for line in csvfile:
        # raw_input(line)
        line = line.strip()
        if not line[0] in ('0', '1'):
            #a comment or header
            continue
        else:
            if line[0] in ('0', '1') and line[1] in ('0', '1') and line[-1] in ('0', '1') and line[-2] in ('0', '1'):
                wo_delimiters = True
                break
    return wo_delimiters


def read_matrix_from(dat_file, delimiter=None):
    #returns numpy matrix and delimiter into original txt file
    column_names = None
    row_names = None
    cols_skip_row_names = None
    skip_header = 0
    with open(dat_file, 'rb') as csvfile:
        try:
            if not delimiter:
                dialect = csv.Sniffer().sniff(csvfile.read())
            elif delimiter:
                # mock dialect
                dialect = csv.excel()
                dialect.delimiter = delimiter
                dialect.lineterminator = '\n'
                dialect.quotechar = '"'
                dialect.doublequote = True
                dialect.skipinitialspace = False
                dialect.quoting = csv.QUOTE_MINIMAL
            csvfile.seek(0)
            if has_header(csvfile.read(), dialect):
                skip_header = 1
                csvfile.seek(0)
                column_names = csvfile.readline().strip().split(dialect.delimiter)[1:]
                second_splits = csvfile.readline().split(dialect.delimiter)
                third_splits = csvfile.readline().split(dialect.delimiter)

                if second_splits[0] not in ('0', '1') and third_splits[0] not in ('0', '1'):
                    #csv has row names
                    row_names = []
                    cols_skip_row_names = range(1, len(second_splits))
                    csvfile.seek(0)
                    #csvfile.readline() # skip 1 line header
                    for i in csvfile:
                        row_names.append(i.split(dialect.delimiter)[0])

            csvfile.seek(0)
            #load real matrix into numpy 2D array
            matrix = np.loadtxt(dat_file, delimiter=dialect.delimiter, skiprows=skip_header, usecols=cols_skip_row_names)
            return matrix, dialect.delimiter, dialect.lineterminator, row_names, column_names
        except SniffError as exc:
            # no delimiter found
            if not delimiter:
                # trying to guess delimiter...
                csvfile.seek(0)
                csvfile.readline()
                delimiter = guess_delimiter(csvfile.readline())
                csvfile.seek(0)

            if not delimiter:
                if with_no_delimiters(csvfile):
                    return read_input_w_no_delimiters(csvfile)
                else:
                    #this should be the case with column headers without the first name
                    new_dat_file, delimiter = add_column_header(dat_file, '__')
                    csvfile.seek(0)
                    matrix, delimiter, lineterminator, row_names, column_names = read_matrix_from(new_dat_file, delimiter=delimiter)
                    os.remove(new_dat_file)
                    return matrix, delimiter, lineterminator, row_names, column_names
            elif delimiter is not None:
                #delimiter was found with guess_delimiter
                # or by passing directly the delimiter (as in the recursive call above)
                return read_matrix_from(dat_file, delimiter=delimiter)

def add_headers(col_names, out_mat, row_names):
    if col_names and len(col_names) > 0:
        orig_shape = out_mat.shape
        temp_mat = out_mat.tolist()
        temp_mat.insert(0, col_names)
        out_mat = np.array(temp_mat, dtype='|S4').reshape((orig_shape[0] + 1, orig_shape[1]))
    if row_names is not None:
        #must modify the txt numpy file to add row and column names
        #as in the original input matrixs
        out_mat = np.column_stack((row_names, out_mat))
    return out_mat


def generate_matrix(col_names, delimiter, mat, options, presences, row_names):
    # generating matrices
    # having %s as format it degrades performances in numpy.savetxt method!
    # it's necessary in case of headers
    fmt_ = '%s' if (col_names and len(col_names) > 0) or row_names else '%d'
    if not delimiter:
        delimiter = ''
    for i in range(options.num_matrices):
        #r_presences = list(np.array(presences).copy())	#se no non sono indipendenti
        r_presences = presences[:]
        out_mat = cb.curve_ball(mat, r_presences, num_iterations=options.num_iterations)
        # print('generated matrix in ' + str(time.time() - start))
        if (col_names and len(col_names) > 0) or row_names:
            out_mat = add_headers(col_names, out_mat, row_names)
        file_out = os.path.join(options.output_folder, 'out_mat_' + str(i + 1) + '.txt')
        np.savetxt(file_out, out_mat, delimiter=delimiter, fmt=fmt_)
        print 'Saved ' + file_out


def generate_headers(col_names, row_names):
    if row_names:
        row_names = np.array(row_names)
    if col_names:
        col_names = list(col_names)
    return col_names, row_names


def main():
    parser = OptionParser()
    parser.add_option("-m", "--matrix", dest="input_matrix",
                      help="Co-occurrence matrix input file")
    parser.add_option("-o", "--output-folder",
                      dest="output_folder", default='./null_matrices',
                      help="Folder where random null matrixes will be saved")
    parser.add_option("-n", "--num",
                      dest="num_matrices", default=100, type="int",
                      help="Number of null matrices to generate")
    parser.add_option("-i", "--num-iterations",
                      dest="num_iterations", default=-1, type="int",
                      help="Number of pair comparisons for each matrix generation")

    (options, args) = parser.parse_args()

    # operations to make once
    if not os.path.exists(options.output_folder):
        os.makedirs(options.output_folder)
    mat, delimiter, lineterminator, row_names, col_names = read_matrix_from(options.input_matrix)
    presences = cb.find_presences(mat)
    col_names, row_names = generate_headers(col_names, row_names)

    #this generate method will iterate to produce the desired number of null matrices
    generate_matrix(col_names, delimiter, mat, options, presences, row_names)

if __name__ == '__main__':
    sys.exit(main())
