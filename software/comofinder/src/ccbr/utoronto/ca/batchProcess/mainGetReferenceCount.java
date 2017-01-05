package ccbr.utoronto.ca.batchProcess;

import java.util.ArrayList;
import java.util.HashMap;

import ccbr.utoronto.ca.comoFinderAlgorithm.motifDiscoveryParallel;
import ccbr.utoronto.ca.dataStructure.comNodeAtom;
import ccbr.utoronto.ca.publicFunction.analyzeParameters;
import ccbr.utoronto.ca.readData.readStandardInput;

/* The whole process of our algorithm is : 1, "mainGetReferenceCount.java" will generate all the subgraph counts
 * in the real network; 2, "mainBatchProcess.java" will randomize the real network and gene all the subgraph counts 
 * in the random network; 3, "mainBPComputeSignificance.java" will calculate the statistics of the subgraphs, i.e.
 * Z-score, p-value and etc..*/

public class mainGetReferenceCount {

	/**
	 * @param args
	 * 
	 * -ms: motif size;
	 * -nr: specify the number of processors;
	 * 
	 */
	
	//static String knownCountFileName;
	
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		
		double startTime = new java.util.Date().getTime();
		
		int motifSize = 3;//default
		int nrofProcessors = -1;//We recommend this setting to be default;
		String inputFilePath = null;
		
		String edgesFile = null;
		String verticesFile = null;
		
		String outputdir = null;
		String subgraphCountFileName = null;
		
		//double percentage = -1;
		
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
		if(ap.hasOption("scn") == true)
		{
			subgraphCountFileName = ap.getValue("scn");
		}
		
		System.out.println("Motif size: " + motifSize + "; number of specified processors: " + nrofProcessors + "; input file: " + inputFilePath
				+ "; subgraphCount file name: " + subgraphCountFileName);
		
		readStandardInput rzd = null;
		if(inputFilePath != null)
		{
			rzd = new readStandardInput(inputFilePath);
		}
			
		int[][] adjMatrix = rzd.getAdjMatrix();
		ArrayList<comNodeAtom> nodeList = rzd.getNodeList();
		
		motifDiscoveryParallel tmdp = new motifDiscoveryParallel(adjMatrix, nodeList);
		tmdp.setMotifSize(motifSize);
		tmdp.setNumofProcessors(nrofProcessors);
		tmdp.run();
		tmdp.outputSubgraphCounttoFile(subgraphCountFileName);
		
		Double secs = new Double((new java.util.Date().getTime() - startTime)*0.001);
		System.out.println("The whole algorithm took " + secs + " secs.");
	}

}
