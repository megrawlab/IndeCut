# This script takes an integer, draws this number of integers uniformly
# from [-2147483647, 2147483647]
# and writes the drawn integers to an output file.

args <- commandArgs()

# For batch mode
myargs <- args[4:length(args)]

ndraws <- myargs[1]
outFname <- myargs[2]

seeds <- round(runif(as.integer(ndraws), min=-2147483647, max=2147483647))
write.table(seeds, outFname, row.names=F, col.names=F, quote=F)
