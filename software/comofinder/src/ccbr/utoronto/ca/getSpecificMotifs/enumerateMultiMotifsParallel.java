package ccbr.utoronto.ca.getSpecificMotifs;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import jsr166y.ForkJoinPool;
import ccbr.utoronto.ca.dataStructure.comNodeAtom;
import ccbr.utoronto.ca.publicFunction.binarySearch;
import ccbr.utoronto.ca.publicFunction.fileOperations;

public class enumerateMultiMotifsParallel {

	ArrayList<String> specificMotifs;
	int motifSize;
	
	ArrayList<ArrayList<Integer>> summationsList = new ArrayList<ArrayList<Integer>>(); 
	
	int[][] adjMatrix;
	ArrayList<comNodeAtom> nodeList;
	ArrayList<ArrayList<Integer>> Vset;
	ArrayList<ArrayList<Integer>> subGraphSet;
	
	ArrayList<Integer> microRNAList;
	ArrayList<Integer> tFactorList;
	ArrayList<Integer> geneList;
	
	//Store the number of each subgraph
	/*HashMap<String, ArrayList<ArrayList<Integer>>> isoGraphMap;
	HashMap<String, Integer> countSubgraph;*/
	
	ArrayList<Integer> overallGeneList;
	
	//boolean mergeMotifs = true;
	
	//int subGraphNum = 0;
	FileOutputStream fos;
	String outputFileDir;
	
	long totalSubgraphNum = 0;
	HashMap<Integer, Integer> nodeTypeMap;//There are three types of nodes: 0, miRNA; 1, TF; 2, target gene.
	
	public enumerateMultiMotifsParallel(int[][] adjMatrix, ArrayList<comNodeAtom> nodeList,
			HashMap<Integer, Integer> nodeTypeMap)
	{
		this.adjMatrix = adjMatrix;
		this.nodeList = nodeList;
		this.nodeTypeMap = nodeTypeMap;
		//isoGraphMap = new HashMap<String, ArrayList<ArrayList<Integer>>>();
		//countSubgraph = new HashMap<String, Integer>();
		microRNAList = new ArrayList<Integer>();
		tFactorList = new ArrayList<Integer>();
		geneList = new ArrayList<Integer>();
		
		overallGeneList = new ArrayList<Integer>();
		
		for(int i = 0;i < nodeList.size();i++)
		{
			comNodeAtom curAtom = nodeList.get(i);
			if(curAtom.nodeType == 0)
			{
				microRNAList.add(curAtom.nodeID);
			}
			else if(curAtom.nodeType == 1)
			{
				tFactorList.add(curAtom.nodeID);
			}
			else if(curAtom.nodeType == 2)
			{
				geneList.add(curAtom.nodeID);
			}
			else
			{
				System.err.println("Oh you must be kidding me.");
			}
		}
		
	}
	
	public void readSpecificMotif(String fileName)
	{
		try
		{
			specificMotifs = new ArrayList<String>();
			BufferedReader br = new BufferedReader(new FileReader(fileName));
			String temp = null;
			while((temp = br.readLine()) != null)
			{
				String motif = temp;
				//specificMotifs.add(motif);
				binarySearch.sort(specificMotifs, motif);
			}
			motifSize = (int) Math.sqrt(specificMotifs.get(0).split("_")[0].length());
			br.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	/*public void setMotifSize(int k)
	{
		motifSize = k;
	}*/
	
	public void setOutputFileDir(String outputFileDir)
	{
		this.outputFileDir = outputFileDir;
		File fileDir = new File(outputFileDir);
		if(!fileDir.exists())
		{
			if(fileDir.mkdirs() == false)
			{
				System.err.println("Fail to make the file directory : " + fileDir + ". Please check again.");
				System.exit(0);
			}
		}
		else
		{
			fileOperations.cleanDirectory(outputFileDir);
		}
	}
	
	@SuppressWarnings("unchecked")
	public void run()
	{
		int nrOfProcessors = Runtime.getRuntime().availableProcessors();
		System.out.println("The current available processors are : " + nrOfProcessors);
				
		ForkJoinPool fjPool = new ForkJoinPool();
		
		//fileOperations.cleanDirectory(outputFileDir);
		enumerateMultiMotifsFJTask.SEQUENTIAL_THRESHOLD = (int) Math.ceil((double) microRNAList.size() / nrOfProcessors);
		enumerateMultiMotifsFJTask fjTask = new enumerateMultiMotifsFJTask(adjMatrix, microRNAList, nodeList,
				specificMotifs, motifSize, 0, microRNAList.size(), outputFileDir);
		totalSubgraphNum = (Integer)fjPool.invoke(fjTask);

		System.out.println("Multi-threads run success. Found " + totalSubgraphNum + " subgraphs for all the specific motifs.");
		fjPool.shutdown();
		
		/*if(mergeMotifs == true)
		{
			writeOverallGeneList(String.valueOf(motifSize) + "_genes.txt");
		}*/
	}
}
