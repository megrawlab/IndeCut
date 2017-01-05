package ccbr.utoronto.ca.comoFinderAlgorithm;

import java.io.File;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import jsr166y.ForkJoinPool;
import ccbr.utoronto.ca.dataStructure.comNodeAtom;
import ccbr.utoronto.ca.publicFunction.binarySearch;

public class motifDiscoveryParallel {

ArrayList<ArrayList<Integer>> summationsList = new ArrayList<ArrayList<Integer>>(); 
	
	int[][] adjMatrix;
	int motifSize;
	ArrayList<comNodeAtom> nodeList;
	ArrayList<ArrayList<Integer>> Vset;
	ArrayList<ArrayList<Integer>> subGraphSet;
	
	ArrayList<Integer> microRNAList;
	ArrayList<Integer> tFactorList;
	ArrayList<Integer> geneList;
	
	//Store the number of each subgraph
	HashMap<String, ArrayList<ArrayList<Integer>>> isoGraphMap;
	HashMap<String, Integer> countSubgraph;
	
	ArrayList<int[]> seedGraphList;
	
	boolean[] visited;
	
	//int subGraphNum = 0;
	FileOutputStream fos;
	String outputFileDir;
	
	long totalSubgraphNum = 0;
	HashMap<Integer, Integer> nodeTypeMap;//There are three types of nodes: 0, miRNA; 1, TF; 2, target gene.
	
	HashMap<String, Integer> knownCountSubgraph = null;
	
	int nrOfProcessors = -1;
	
	//static int singleThreadThreshold = 1000;
	
	public motifDiscoveryParallel(int[][] adjMatrix, ArrayList<comNodeAtom> nodeList)
	{
		this.adjMatrix = adjMatrix;
		this.nodeList = nodeList;
		//isoGraphMap = new HashMap<String, ArrayList<ArrayList<Integer>>>();
		//countSubgraph = new HashMap<String, Integer>();
		microRNAList = new ArrayList<Integer>();
		tFactorList = new ArrayList<Integer>();
		geneList = new ArrayList<Integer>();
		
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
	
	public void setMotifSize(int k)
	{
		motifSize = k;
	}
	
	public void setNumofProcessors(int nrOfProcessors)
	{
		this.nrOfProcessors = nrOfProcessors;
	}
	
	public void setOutputFileDir(String outputFileDir)
	{
		this.outputFileDir = outputFileDir;
	}
	
	public void setKnownCountSubgraph(HashMap<String, Integer> knownCountSubgraph)
	{
		this.knownCountSubgraph = knownCountSubgraph;
	}
	
	public void renewNodeInteraction()
	{
		for(int i = 0;i < nodeList.size();i++)
		{
			nodeList.get(i).nodeInteraction.clear();
		}
		
		for(int i = 0;i < nodeList.size();i++)
		{
			comNodeAtom source = nodeList.get(i);
			for(int j = 0;j < nodeList.size();j++)
			{
				comNodeAtom target = nodeList.get(j);
				if(adjMatrix[source.nodeID][target.nodeID] != 0)
				{
					binarySearch.sort(source.nodeInteraction, target.nodeID);
					binarySearch.sort(target.nodeInteraction, source.nodeID);
				}
			}
		}
	}
	
	@SuppressWarnings("unchecked")
	public void run()
	{
		if(nrOfProcessors < 0)
		{
			nrOfProcessors = Runtime.getRuntime().availableProcessors();
		}
		System.out.println("The current available processors are : " + nrOfProcessors);	
		
		double startTime = new java.util.Date().getTime();
		
		ForkJoinPool fjPool = new ForkJoinPool();
		comoAtom.SEQUENTIAL_THRESHOLD = (int) Math.ceil((double) microRNAList.size() / nrOfProcessors);
		System.out.println("The sequential threshold is : " + comoAtom.SEQUENTIAL_THRESHOLD);
		comoAtom fjTask = new comoAtom(adjMatrix, microRNAList, nodeList, motifSize, 0, microRNAList.size());
		countSubgraph = (HashMap<String, Integer>)fjPool.invoke(fjTask);
		
		Double secs = new Double((new java.util.Date().getTime() - startTime)*0.001);
		System.out.println("The enumeration process took " + secs + " secs.");
		
		startTime = new java.util.Date().getTime();
		//if(motifSize > 3)
		{
			removeIsomorphismParallelTask.SEQUENTIAL_THRESHOLD = (int) Math.ceil((double) countSubgraph.size() / nrOfProcessors);
			if(removeIsomorphismParallelTask.SEQUENTIAL_THRESHOLD < nrOfProcessors)
			{
				removeIsomorphismParallelTask.SEQUENTIAL_THRESHOLD = countSubgraph.size();
			}
			ArrayList<String> labelList = new ArrayList<String>();
			for(Iterator<String> itKey = countSubgraph.keySet().iterator();itKey.hasNext();)
			{
				String label = itKey.next();
				labelList.add(label);
			}
			removeIsomorphismParallelTask rip = new removeIsomorphismParallelTask(countSubgraph, labelList, 0, countSubgraph.size());
			countSubgraph = (HashMap<String, Integer>)fjPool.invoke(rip);
		}
		
		secs = new Double((new java.util.Date().getTime() - startTime)*0.001);
		System.out.println("The isomorphism removal process took " + secs + " secs.");
		
		fjPool.shutdown();
	}
	
	public void outputSubgraphCounttoFile(String fileName)
	{
		try
		{
			//fileName = "output/subgraphCount/" + fileName;
			File newFile = new File(fileName);
			if(newFile.exists())
			{
				newFile.delete();
			}
			FileOutputStream fos = new FileOutputStream(newFile);
			for(Iterator<String> itKey = countSubgraph.keySet().iterator();itKey.hasNext();)
			{
				String curKey = itKey.next();
				fos.write(curKey.getBytes());
				fos.write("\t".getBytes());
				fos.write(String.valueOf(countSubgraph.get(curKey)).getBytes());
				fos.write("\n".getBytes());
			}
			fos.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public HashMap<String, Integer> getCountSubgraph()
	{
		return countSubgraph;
	}
	
	public void setCountSubgraph(HashMap<String, Integer> countSubgraph)
	{
		this.countSubgraph = countSubgraph;
	}
	
}
