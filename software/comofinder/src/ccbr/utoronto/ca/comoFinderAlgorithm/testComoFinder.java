package ccbr.utoronto.ca.comoFinderAlgorithm;

import java.util.ArrayList;
import java.util.HashMap;

import ccbr.utoronto.ca.dataStructure.comNodeAtom;
import ccbr.utoronto.ca.publicFunction.analyzeParameters;
import ccbr.utoronto.ca.readData.readStandardInput;

/* All the files in this package have been verified. We could use it directly.*/

public class testComoFinder {

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		
		double startTime = new java.util.Date().getTime();

		int motifSize = 4;
		String inputFilePath = "/home/mitra/workspace/comoFinder/sample_input_network.txt";
		String knownCountFileName = "Ref_test_" + motifSize + ".txt";
		
		int nrofProcessors = -1;//We recommend this setting to be default;
		
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
		
		readStandardInput rzd = new readStandardInput(inputFilePath);
		
		int[][] adjMatrix = rzd.getAdjMatrix();
		ArrayList<comNodeAtom> nodeList = rzd.getNodeList();
		//HashMap<Integer, Integer> nodeTypeMap = rzd.getNodeTypeMap();
		
		//double startTime = new java.util.Date().getTime();
		motifDiscoveryParallel tmdp = new motifDiscoveryParallel(adjMatrix, nodeList);
		tmdp.setMotifSize(motifSize);
		tmdp.setNumofProcessors(nrofProcessors);
		tmdp.run();
		tmdp.outputSubgraphCounttoFile(knownCountFileName);
		
		Double secs = new Double((new java.util.Date().getTime() - startTime)*0.001);
		System.out.println("The test improvements took " + secs + " secs.");
	}
}
