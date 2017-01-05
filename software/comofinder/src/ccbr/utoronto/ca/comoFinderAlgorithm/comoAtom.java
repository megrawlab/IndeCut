package ccbr.utoronto.ca.comoFinderAlgorithm;

import java.io.File;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import ccbr.utoronto.ca.dataStructure.comNodeAtom;
import ccbr.utoronto.ca.publicFunction.binarySearch;
import ccbr.utoronto.ca.publicFunction.conversion;
import ccbr.utoronto.ca.publicFunction.getCanonicalLabel;
import ccbr.utoronto.ca.publicFunction.nCombinations;
import ccbr.utoronto.ca.publicFunction.output;
import jsr166y.RecursiveTask;

public class comoAtom extends RecursiveTask{

public static int SEQUENTIAL_THRESHOLD;
	
	int start;
	int end;
	//int currentThreadLevel;
	
	ArrayList<Integer> Vext;
	ArrayList<Integer> Vsub;
	ArrayList<Integer> Vopen;
	
	//int colorTypes = 3;
	
	ArrayList<ArrayList<Integer>> summationsList; 
	
	int[][] adjMatrix;
	int motifSize;
	ArrayList<comNodeAtom> nodeList;
	ArrayList<ArrayList<Integer>> Vset;
	//ArrayList<ArrayList<Integer>> subGraphSet;
	
	ArrayList<int[]> seedGraphList;
	ArrayList<Integer> microRNAList;
	ArrayList<Integer> tFactorList;
	ArrayList<Integer> geneList;
	
	boolean[] visited;
	
	//int subGraphNum = 0;
	FileOutputStream fos;
	String outputFileDir;
	
	//HashMap<Integer, Integer> nodeTypeMap;
	HashMap<String, Integer> countSubgraph;
	
	boolean debug = false;

	public comoAtom(int[][] adjMatrix, ArrayList<Integer> microRNAList, ArrayList<comNodeAtom> nodeList, 
			int motifSize, int start, int end)
			//HashMap<Integer, Integer> nodeTypeMap, int motifSize, int start, int end)
	{
		this.adjMatrix = adjMatrix;
		this.microRNAList = microRNAList;
		this.nodeList = nodeList;
		//this.nodeTypeMap = nodeTypeMap;
		this.motifSize = motifSize;
		//this.currentThreadLevel = currentThreadLevel;
		this.start = start;
		this.end = end;
		
		Vsub = new ArrayList<Integer>();
		Vext = new ArrayList<Integer>();
		Vopen = new ArrayList<Integer>();

		countSubgraph = new HashMap<String, Integer>();
	}
	
	public comoAtom(int[][] adjMatrix, ArrayList<Integer> microRNAList, HashMap<Integer, Integer> nodeTypeMap,
			int motifSize, int start, int end, String outputFileDir)
	{
		this.adjMatrix = adjMatrix;
		this.microRNAList = microRNAList;
		//this.nodeTypeMap = nodeTypeMap;
		this.motifSize = motifSize;
		//this.currentThreadLevel = currentThreadLevel;
		this.start = start;
		this.end = end;
		this.outputFileDir = outputFileDir;

		Vsub = new ArrayList<Integer>();
		Vext = new ArrayList<Integer>();
		Vopen = new ArrayList<Integer>();

		countSubgraph = new HashMap<String, Integer>();
	}
	
	protected HashMap<String, Integer> compute()
	{
		final int length = end - start;
		if(length <= SEQUENTIAL_THRESHOLD)
		{
			computeDirectly();
		}
		else
		{
			try
			{
				final int split = length / 2;
				final comoAtom left = new comoAtom(adjMatrix, microRNAList, nodeList,
						motifSize, start, start + split);
				final comoAtom right = new comoAtom(adjMatrix, microRNAList, nodeList,
						motifSize, start + split, end);
				invokeAll(left, right);
				HashMap<String, Integer> leftCountSubgraph = (HashMap<String, Integer>)left.get();
				HashMap<String, Integer> rightCountSubgraph = (HashMap<String, Integer>)right.get();
				addCountSubgraphResults(leftCountSubgraph, rightCountSubgraph);
			}
			catch(Exception e)
			{
				e.printStackTrace();
			}
		}
		return countSubgraph;
	}
		
	protected void computeDirectly()
	{
		System.out.println("Start to extend the seed graphs from " + start + " to " + end + "...");
		//String fileName = outputFileDir + "Thread_" + String.valueOf(start) + ".sg";
		//initialFileOutputStream(fileName);
		enumerateSubgraphs();
		//closeFile();
		System.out.println("Finish extending the current seed graphs...");
	}
	
	public void enumerateSubgraphs()
	{
		//for each vertex v belongs to V, do
		for(int i = start;i < end;i++)
		{
			int curMiRNAID = microRNAList.get(i);
			
			//if(debug == true)
			{
				System.out.println("=======Extending from node : " + curMiRNAID + " =========");
			}

			Vext.clear();
			Vsub.clear();
			Vopen.clear();
			
			comNodeAtom curMiRNA = nodeList.get(curMiRNAID);
			for(int j = 0;j < curMiRNA.nodeInteraction.size();j++)
			{
				int currentPart = curMiRNA.nodeInteraction.get(j);
				if(currentPart > curMiRNAID)
				{
					/*binarySearch.sort(Vext, currentPart);
					binarySearch.sort(Vopen, currentPart);*/
					Vext.add(currentPart);
					Vopen.add(currentPart);
				}
			}
			Vsub.add(curMiRNAID);
			
			extendSubgraph(Vsub, Vext, Vopen, curMiRNAID);
		}
	}

	public void extendSubgraph(ArrayList<Integer> Vsub, ArrayList<Integer> Vext, ArrayList<Integer> Vopen, int nodeV)
	{
		
		if(debug == true)
		{
			System.out.println("Vsub : ");
			output.outputArrayList(Vsub);
			System.out.println("Vext : ");
			output.outputArrayList(Vext);
			System.out.println("Vopen : ");
			output.outputArrayList(Vopen);
		}
		
		while(Vext.size() != 0)
		{
			int nodeW = Vext.get(0);
			Vext.remove(0);
			
			int freeFlag = -1;
			
			if(Vsub.size() == motifSize - 2)
			{
				boolean[] sat = {false, false, false};
				if(Vsub.size() > 1)
				{
					for(int i = 1;i < Vsub.size();i++)
					{
						int nodeType = nodeList.get(Vsub.get(i)).nodeType;
						sat[nodeType] = true;
					}
				}

				int nodeType = nodeList.get(nodeW).nodeType;
				sat[nodeType] = true;				

				if(sat[1] == false)
				{
					freeFlag = 1;
				}
				else if(sat[2] == false)
				{
					freeFlag = 2;
				}
			}
			
		    //find all the nodes u from Nexcl(w,Vsubgraph) and u > v
			//double startTime2 = new java.util.Date().getTime();
			
			ArrayList<Integer> newVext = duplicateArrayList(Vext);
			ArrayList<Integer> newVopen = duplicateArrayList(Vopen);
			findInNexcl(nodeW, nodeV, Vsub, newVext, newVopen, freeFlag);
			
			if(newVext.size() == 0)
			{
				continue;
			}
			
			ArrayList<Integer> newVsub = duplicateArrayList(Vsub);
			//newVsub.add(nodeW);
			binarySearch.sort(newVsub, nodeW);
			binarySearch.remove(newVopen, nodeW);
			
			if(newVsub.size() == motifSize - 1)
			{
				countSubgraph(newVsub, newVext);
				//countSubgraphTest(newVsub, newVext);
			}
			else
			{
				extendSubgraph(newVsub, newVext, newVopen, nodeV);
			}
		}
	}
	
	public void findInNexcl(int nodeW, int nodeV, ArrayList<Integer> Vsub, ArrayList<Integer> Vext, 
				ArrayList<Integer> Vopen, int freeFlag)
	{
		//We need to remove those nodes that do not meet our requirements from the Vext;
		if(freeFlag != -1)
		{
			for(int i = 0;i < Vext.size();)
			{
				int curNode = Vext.get(i);
				if(nodeList.get(curNode).nodeType != freeFlag)
				{
					Vext.remove(i);
				}
				else
				{
					i++;
				}
			}
		}
		
		for(int i = 0;i < nodeList.get(nodeW).nodeInteraction.size();i++)
		{
			int currentPartner = nodeList.get(nodeW).nodeInteraction.get(i);
			if(currentPartner > nodeV 
					&& binarySearch.contain(Vsub, currentPartner) == false && binarySearch.sort(Vopen, currentPartner) == false)
			{
				if(freeFlag == -1)
				{
					binarySearch.sort(Vext, currentPartner);
					//At the same time, update the Vopen;
					//binarySearch.sort(Vopen, currentPartner);
				}
				else
				{
					if(nodeList.get(currentPartner).nodeType == freeFlag)
					{
						binarySearch.sort(Vext, currentPartner);
					}
				}
			}
		}
	}
	
	public ArrayList<Integer> duplicateArrayList(ArrayList<Integer> originalList)
	{
		ArrayList<Integer> newArrayList = new ArrayList<Integer>();

		newArrayList.addAll(originalList);

		return newArrayList;
	}
	
	public void countSubgraph(ArrayList<Integer> Vsub, ArrayList<Integer> Vext)
	{		
		for(int i = 0;i < Vext.size();i++)
		{
			ArrayList<Integer> newVsub = duplicateArrayList(Vsub);
			newVsub.add(Vext.get(i));
			
			countSubgraph(newVsub);
		}
	}
	
	public void countSubgraphTest(ArrayList<Integer> Vsub, ArrayList<Integer> Vext)
	{
		double startTime = new java.util.Date().getTime();
		int lastElement = Vsub.size();
		Vsub.add(0);
		
		for(int i = 0;i < Vext.size();i++)
		{
			Vsub.set(lastElement, Vext.get(i));	
			
			String subGraphCanonicalLabel = conversion.getLabel(adjMatrix, nodeList, Vsub);			
			if(countSubgraph.containsKey(subGraphCanonicalLabel))
			{
				int count = countSubgraph.get(subGraphCanonicalLabel);
				count = count + 1;
				countSubgraph.put(subGraphCanonicalLabel, count);
				//isoGraphMap.get(canLabel).add(subGraphSet.get(currentSubgraphPointer));
			}
			else
			{
				countSubgraph.put(subGraphCanonicalLabel, 1);
			}
		}
		
		Double secs = new Double((new java.util.Date().getTime() - startTime)*0.001);
		System.out.println("Count subgraphs took " + secs + " secs." + " The Vext size is : " + Vext.size());
	}
	
	public void countGraphIso(ArrayList<Integer> curSubgraph)
	{
		//long startTime = new java.util.Date().getTime();
		
		//getCanonicalLabel gcl = new getCanonicalLabel(curSubgraph, adjMatrix, nodeTypeMap);
		String subGraphCanonicalLabel = "";
		
		getCanonicalLabel gcl = new getCanonicalLabel(curSubgraph, adjMatrix, nodeList);
		subGraphCanonicalLabel = gcl.canonicalLabel();
					
		if(countSubgraph.containsKey(subGraphCanonicalLabel))
		{
			int count = countSubgraph.get(subGraphCanonicalLabel);
			count = count + 1;
			countSubgraph.put(subGraphCanonicalLabel, count);
			//isoGraphMap.get(canLabel).add(subGraphSet.get(currentSubgraphPointer));
		}
		else
		{
			countSubgraph.put(subGraphCanonicalLabel, 1);
		}
		
		/*Double secs = new Double((new java.util.Date().getTime() - startTime) * 0.001);
		System.out.println("The graph isomorphism test took " + secs + " secs to finish.");*/
	}
	
	public void countSubgraph(ArrayList<Integer> curSubgraph)
	{
		String subGraphCanonicalLabel = conversion.getLabel(adjMatrix, nodeList, curSubgraph);
		
		if(countSubgraph.containsKey(subGraphCanonicalLabel))
		{
			int count = countSubgraph.get(subGraphCanonicalLabel);
			count = count + 1;
			countSubgraph.put(subGraphCanonicalLabel, count);
			//isoGraphMap.get(canLabel).add(subGraphSet.get(currentSubgraphPointer));
		}
		else
		{
			countSubgraph.put(subGraphCanonicalLabel, 1);
		}
	}
	
	public boolean checkNodeTypeConstraints(ArrayList<Integer> curSubgraph)
	{
		boolean satFlag = true;
		boolean[] sat = {false, false, false};
		for(int i = 0;i < curSubgraph.size();i++)
		{
			sat[nodeList.get(curSubgraph.get(i)).nodeType] = true;
		}
		for(int i = 0;i < sat.length;i++)
		{
			if(sat[i] == false)
			{
				satFlag = false;
				break;
			}
		}
		
		return satFlag;
	}
	
	public int[][] convertArrayToMatrix(ArrayList<Integer> currentSubgraph)
	{
		int[][] subMatrix = new int[currentSubgraph.size()][currentSubgraph.size()];
		for(int i = 0;i < subMatrix.length;i++)
		{
			for(int j = 0;j < subMatrix[i].length;j++)
			{
				if(adjMatrix[currentSubgraph.get(i)][currentSubgraph.get(j)] == 1)
				{
					subMatrix[i][j] = 1;
				}
			}
		}
		return subMatrix;
	}	
	
	public void initialFileOutputStream(String fileName)
	{
		try
		{
			//fileName = "output/parallel/" + fileName;
			File newFile = new File(fileName);
			if(newFile.exists())
			{
				newFile.delete();
			}
			fos = new FileOutputStream(newFile);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public void closeFile()
	{
		try
		{
			fos.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public void addCountSubgraphResults(HashMap<String, Integer> leftCountSubgraph, HashMap<String, Integer> rightCountSubgraph)
	{
		countSubgraph.putAll(rightCountSubgraph);
		for(Iterator<String> leftKey = leftCountSubgraph.keySet().iterator();leftKey.hasNext();)
		{
			String leftCanonicalLabel = leftKey.next();
			int leftCount = leftCountSubgraph.get(leftCanonicalLabel);
			if(rightCountSubgraph.containsKey(leftCanonicalLabel))
			{				
				int rightCount = rightCountSubgraph.get(leftCanonicalLabel);
				int totalCount = leftCount + rightCount;
				countSubgraph.put(leftCanonicalLabel, totalCount);
			}
			else
			{
				countSubgraph.put(leftCanonicalLabel, leftCount);
			}
		}
	}
	
	public void writeCurSubgraphtoFile(ArrayList<Integer> curSubGraph)
	{
		try
		{
			for(int j = 0;j < curSubGraph.size();j++)
			{
				fos.write(String.valueOf(curSubGraph.get(j)).getBytes());
				fos.write("\t".getBytes());
			}
			fos.write("\n".getBytes());
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public void binaryInsertion(ArrayList<Integer> array, int node)
	{     
		int low = 0;
        int high = array.size() - 1;

        while(low <= high) 
        {
            int mid = low + (high - low) / 2;
            if(node < array.get(mid))
            {
            	high = mid - 1;
            }
            else if(node > array.get(mid))
            {
            	low = mid + 1;
            }
        }
        
        array.add(low, node);
	}
	
}
