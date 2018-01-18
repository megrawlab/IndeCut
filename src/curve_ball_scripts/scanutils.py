'''Supplementary Software 4 Additional python module needed by the file curveball_main.py'''

import csv
from cStringIO import StringIO

__author__ = 'v-user'


def guess_delimiter(row):
    #fallback method if csv.Sniffer() doesn't work properly
    if row.strip().startswith('0') or row.strip().startswith('1'):
        if len(row.split(',')) > 1:
            return ','
        elif len(row.split('\t')) > 1:
            return '\t'
        elif len(row.split(';')) > 1:
            return ';'
        elif len(row.split('|')) > 1:
            return '|'
        elif len(row.split(' ')) > 1:
            return ' '
        else:
            return None
    else:
        return None


def has_header(sample, dialect=None):
        # Creates a dictionary of types of data in each column. If any
        # column is of a single type (say, integers), *except* for the first
        # row, then the first row is presumed to be labels. If the type
        # can't be determined, it is assumed to be a string in which case
        # the length of the string is the determining factor: if all of the
        # rows except for the first are the same length, it's a header.
        # Finally, a 'vote' is taken at the end for each column, adding or
        # subtracting from the likelihood of the first row being a header.
        if dialect is None:
            rdr = csv.reader(StringIO(sample), csv.Sniffer().sniff(sample))
        else:
            rdr = csv.reader(StringIO(sample), dialect)

        header = rdr.next()  # assume first row is header

        columns = len(header)
        column_types = {}
        for i in range(columns):
            column_types[i] = None

        checked = 0
        for row in rdr:
            # arbitrary number of rows to check, to keep it sane
            if checked > 20:
                break
            checked += 1

            if len(row) != columns:
                continue  # skip rows that have irregular number of columns

            for col in column_types.keys():

                for this_type in [int, long, float, complex]:
                    try:
                        this_type(row[col])
                        break
                    except (ValueError, OverflowError):
                        pass
                else:
                    # fallback to length of string
                    this_type = len(row[col])

                # treat longs as ints
                if this_type == long:
                    this_type = int

                if this_type != column_types[col]:
                    if column_types[col] is None: # add new column type
                        column_types[col] = this_type
                    else:
                        # type is inconsistent, remove column from
                        # consideration
                        del column_types[col]

        # finally, compare results against first row and "vote"
        # on whether it's a header
        hasheader = 0
        for col, colType in column_types.items():
            if type(colType) == type(0):  # it's a length
                if len(header[col]) != colType:
                    hasheader += 1
                else:
                    hasheader -= 1
            else: # attempt typecast
                try:
                    colType(header[col])
                except (ValueError, TypeError):
                    hasheader += 1
                else:
                    hasheader -= 1

        return hasheader > 0


def add_column_header(dat_file, new_header=''):
    new_file = (dat_file+'_mod')
    with open(dat_file, 'rb') as infile, open(dat_file+'_mod', 'wb') as outfile:
        delimiter = guess_delimiter(infile.readline())
        infile.seek(0)
        r = csv.reader(infile, delimiter=delimiter)
        headers = r.next()
        # raw_input(headers)
        #delimiter = _guess_delimiter(headers)
        headers.insert(0, new_header)
        # raw_input(headers)
        w = csv.writer(outfile, delimiter=delimiter)
        w.writerow(headers)
        for row in r:
            w.writerow(row)
    return new_file, delimiter