# This script takes an input seed inputSeed for the random number generator,
# draws ndraws random integers from [minSeed, maxSeed]
# and writes the drawn integers to output file outFname.
# If inputStartingDraw is greater than 1, draws indexed by
# inputStartingDraw:(inputStartingDraw + ndraws - 1) are output.

args <- commandArgs()

# For batch mode
myargs <- args[4:length(args)]

minSeed <- as.integer(myargs[1])
maxSeed <- as.integer(myargs[2])
inputSeed <- as.integer(myargs[3])
inputStartingDraw <- as.integer(myargs[4])
ndraws <- as.integer(myargs[5])
outFname <- myargs[6]

set.seed(inputSeed)
if (inputStartingDraw < 1) {
  inputStartingDraw <- 1
}
totaldraws <- inputStartingDraw + ndraws - 1
tmpseeds <- round(runif(totaldraws, min=minSeed, max=maxSeed))
seeds <- tmpseeds[inputStartingDraw:totaldraws]
write.table(seeds, outFname, row.names=F, col.names=F, quote=F)
