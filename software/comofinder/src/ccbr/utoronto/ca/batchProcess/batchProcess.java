package ccbr.utoronto.ca.batchProcess;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.HashMap;

import ccbr.utoronto.ca.comoFinderAlgorithm.motifDiscoveryParallel;
import ccbr.utoronto.ca.dataStructure.comNodeAtom;
import ccbr.utoronto.ca.networkRandomization.MDR_multipleEdgeTypes;
import ccbr.utoronto.ca.publicFunction.properties;
import ccbr.utoronto.ca.publicFunction.writeFile;

public class batchProcess {

	int[][] adjMatrix;
	int motifSize;
	ArrayList<comNodeAtom> nodeList;
	HashMap<Integer, Integer> nodeTypeMap;//node type never changes during the shuffling process.
	//HashMap<String, Integer> countSubgraph;
	int batchNum;
	File subgraphFileDir;
	HashMap<String, Integer> knownCountSubgraph;
	
	int maxTrials = 100;
	
	int nrofProcessors = -1;
	
	public batchProcess(int motifSize, int batchNum)
	{
		this.motifSize = motifSize;
		this.batchNum = batchNum;
	}
	
	public void initialize(int[][] adjMatrix, ArrayList<comNodeAtom> nodeList, HashMap<Integer, Integer> nodeTypeMap)
	{
		this.adjMatrix = adjMatrix;
		this.nodeList = nodeList;
		this.nodeTypeMap = nodeTypeMap;
	}
	
	public void readKnownCountSubgraph(String knownCountFileName)
	{
		knownCountSubgraph = new HashMap<String, Integer>();
		try
		{
			//We read the file sequentially: first microRNA, second TF and third genes.
			BufferedReader br = new BufferedReader(new FileReader(knownCountFileName));
			String temp = null;
			while((temp = br.readLine()) != null)
			{
				String[] canLabelInfo = temp.split("\t");
				knownCountSubgraph.put(canLabelInfo[0], 0);
			}
			br.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public void setSubgraphFileDir(String fileDir)
	{
		subgraphFileDir = new File(fileDir);
		if(!subgraphFileDir.exists())
		{
			if(subgraphFileDir.mkdir() == false)
			{
				System.err.println("Fail to make the file directory : " + subgraphFileDir + ". Please check again.");
				System.exit(0);
			}
		}
		if(subgraphFileDir.isFile())
		{
			System.err.println(subgraphFileDir + " is not a directory!");
			System.exit(0);
		}
	}
	
	public void setNumofProcessors(int nrofProcessors)
	{
		this.nrofProcessors = nrofProcessors;
	}
	
	public void setMaxTrials(int maxTrials)
	{
		this.maxTrials = maxTrials;
	}
	
	/**
	 * @author mitra
	 * This method is added by Mitra to output the random networks into files
	 */
	public void runSingleTaskinParallel()
	{	
		
		String batchFileName = subgraphFileDir.getAbsolutePath() + "/" + motifSize + "_batch_" + String.valueOf(batchNum) + ".count";		
		
		/** added by Mitra */
		String randFileName = subgraphFileDir.getAbsolutePath() + "/" + "comof.rand." + String.valueOf(batchNum);		
		
		MDR_multipleEdgeTypes mdr = new MDR_multipleEdgeTypes(adjMatrix, nodeTypeMap);
		mdr.setMaxTrials(maxTrials);
		
		/** added by Mitra */
		mdr.setRandfileName(randFileName);
		mdr.run();
		
//		motifDiscoveryParallel tmdp = new motifDiscoveryParallel(mdr.randomMatrix, nodeList);
//		tmdp.setMotifSize(motifSize);
//		tmdp.setNumofProcessors(nrofProcessors);
//		//We need to renew the nodeInteraction for each node!!
//		tmdp.renewNodeInteraction();
//		tmdp.run();
//		tmdp.outputSubgraphCounttoFile(batchFileName);
	}
	
}
