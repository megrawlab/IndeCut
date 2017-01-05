package ccbr.utoronto.ca.serialProcess;

import java.util.ArrayList;
import java.util.HashMap;

import ccbr.utoronto.ca.batchProcess.batchProcess;
import ccbr.utoronto.ca.calculation.BPComputeSignificance;
import ccbr.utoronto.ca.comoFinderAlgorithm.motifDiscoveryParallel;
import ccbr.utoronto.ca.dataStructure.comNodeAtom;
import ccbr.utoronto.ca.publicFunction.analyzeParameters;
import ccbr.utoronto.ca.readData.readStandardInput;

public class mainSerialProcess {

	public static void main(String[] args) {
		// TODO Auto-generated method stub

		double startTime = new java.util.Date().getTime();
		
		int motifSize = 3;//default
		int nrofProcessors = -1;//We recommend this setting to be default;
		String inputFilePath = null;
		
		String edgesFile = null;
		String verticesFile = null;
		
		String outputdir = null;
		String batchFileDir = null;
		
		int ensembleSize = 1000;
		int maxIterations = 100; 
		
		String motifFileName = "finalMotif.txt";
		
		analyzeParameters ap = new analyzeParameters(args);
		if(ap.hasOption("ms") == true)
		{
			motifSize = Integer.valueOf(ap.getValue("ms"));
		}
		if(ap.hasOption("nr") == true)
		{
			nrofProcessors = Integer.valueOf(ap.getValue("nr"));
		}	
		if(ap.hasOption("fp") == true)
		{
			inputFilePath = ap.getValue("fp");
		}
		if(ap.hasOption("ef") == true)
		{
			edgesFile = ap.getValue("ef");
		}
		if(ap.hasOption("vf") == true)
		{
			verticesFile = ap.getValue("vf");
		}
		if(ap.hasOption("opd") == true)
		{
			outputdir = ap.getValue("opd");
		}
		if(ap.hasOption("bfd") == true)
		{
			batchFileDir = ap.getValue("bfd");
		}
		if(ap.hasOption("ens") == true)
		{
			ensembleSize = Integer.valueOf(ap.getValue("ens"));
		}
		if(ap.hasOption("mfn") == true)
		{
			motifFileName = ap.getValue("mfn");
		}
		if(ap.hasOption("ite") == true)
		{
			maxIterations = Integer.valueOf(ap.getValue("ite"));
		}
		
		
		/*System.out.println("Motif size: " + motifSize + "; Consider self-regulations: " + considerSelfRegulations + "; " 
				+ "Consider PPIs: " + considerPPIs + "; " + "Number of specified processors: " + nrOfProcessors + "; file name: " + inputFilePath);*/
		
		System.out.println("Motif size: " + motifSize + "; number of specified processors: " + nrofProcessors + "; input file: " + inputFilePath
				+ "; size of random network ensemble: " + ensembleSize + "; batch files directory: " + batchFileDir + "; the maximal number of iterations is :"
				+ maxIterations);
		
		readStandardInput rzd = null;
		if(inputFilePath != null)
		{
			rzd = new readStandardInput(inputFilePath);
		}		
		
		int[][] adjMatrix = rzd.getAdjMatrix();
		ArrayList<comNodeAtom> nodeList = rzd.getNodeList();
		HashMap<Integer, Integer> nodeTypeMap = rzd.getNodeTypeMap();
		
		motifDiscoveryParallel tmdp = new motifDiscoveryParallel(adjMatrix, nodeList);
		tmdp.setMotifSize(motifSize);
		tmdp.setNumofProcessors(nrofProcessors);
		tmdp.run();
		
		for(int index = 0;index < ensembleSize;index++)
		{
			System.out.println("Started to generate and process the " + index + "th randomized network...");
			batchProcess bp = new batchProcess(motifSize, index);
			bp.setSubgraphFileDir(batchFileDir);
			bp.setNumofProcessors(nrofProcessors);
			bp.initialize(adjMatrix, nodeList, nodeTypeMap);
			bp.runSingleTaskinParallel();
			System.out.println("Finished processing the " + index + "th randomzied network...");
		}		
		
		BPComputeSignificance bpcs = new BPComputeSignificance();
		bpcs.setCountSubgraph(tmdp.getCountSubgraph());
		bpcs.analyzeBatchFiles(batchFileDir);
		bpcs.runStatisticalAnalysis();
		bpcs.outputMotifs(motifFileName);
		
		Double secs = new Double((new java.util.Date().getTime() - startTime)*0.001);
		System.out.println("The whole algorithm took " + secs + " secs.");
		
	}

}
