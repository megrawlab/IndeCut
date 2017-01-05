# Modifications to this project by Megrawlab team:

We modified the following files to output the randomly generated networks into files. These random networks orginally were kept in memory while running the algorithm.

1. modified: `ccbr.utoronto.ca.batchProcess.batchProcess.runSingleTaskinParallel()`

	added: String randFileName = subgraphFileDir.getAbsolutePath() + "/" + "comof.rand." + String.valueOf(batchNum);
	added: mdr.setRandfileName(randFileName);

2. modified: `ccbr.utoronto.ca.networkRandomization.run()`

	added: outputRandFile(randfilename);
	added: setRandfileName(String randFileName)
	
## Compile the source files into a `jar` file `comofinder_randout.jar`

