package ccbr.utoronto.ca.batchProcess;

import java.util.ArrayList;
import java.util.HashMap;

import ccbr.utoronto.ca.dataStructure.comNodeAtom;
import ccbr.utoronto.ca.publicFunction.analyzeParameters;
import ccbr.utoronto.ca.readData.readStandardInput;

public class mainBatchProcess {

	/**
	 * @param args
	 * 
	 * -ms: motif size;
	 * -bn: batch number;
	 * ###Not_in_use### -nrofProcessors: specify the number of processors;
	 * 
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		
		double startTime = new java.util.Date().getTime();
		
		int motifSize = 3;//default
		int batchNum = -1;
		
		String inputFilePath = null;
		String edgesFile = null;
		String verticesFile = null;
		
		String batchFileDir = "output/subgraphCount/";
		int nrofProcessors = -1;
		int maxTrials = 100;
		
		analyzeParameters ap = new analyzeParameters(args);
		if(ap.hasOption("ms") == true)
		{
			motifSize = Integer.valueOf(ap.getValue("ms"));
		}
		if(ap.hasOption("bn") == true)
		{
			batchNum = Integer.valueOf(ap.getValue("bn"));
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
		if(ap.hasOption("bfd") == true)
		{
			batchFileDir = ap.getValue("bfd");
		}
		if(ap.hasOption("nr") == true)
		{
			nrofProcessors = Integer.valueOf(ap.getValue("nr"));
		}
		if(ap.hasOption("ite") == true)
		{
			maxTrials = Integer.valueOf(ap.getValue("ite"));
		}		
		
		System.out.println("Motif size: " + motifSize + "; current batch No.: " + batchNum + "; " + " input file path: " + inputFilePath
							+ "; maximal number of iterations: " + maxTrials);
		
		readStandardInput rzd = null;
		if(inputFilePath != null)
		{
			rzd = new readStandardInput(inputFilePath);
		}
		else
		{
			rzd = new readStandardInput(edgesFile, verticesFile);
		}
		
		int[][] adjMatrix = rzd.getAdjMatrix();
		ArrayList<comNodeAtom> nodeList = rzd.getNodeList();
		HashMap<Integer, Integer> nodeTypeMap = rzd.getNodeTypeMap();
			
		batchProcess bp = new batchProcess(motifSize, batchNum);
		bp.setSubgraphFileDir(batchFileDir);
		bp.setNumofProcessors(nrofProcessors);
		bp.setMaxTrials(maxTrials);
		bp.initialize(adjMatrix, nodeList, nodeTypeMap);
		bp.runSingleTaskinParallel();
		Double secs = new Double((new java.util.Date().getTime() - startTime)*0.001);
		System.out.println("Bacth No." + batchNum + " took " + secs + " secs to finish.");
	}
}
