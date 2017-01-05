package ccbr.utoronto.ca.getSpecificMotifs;

import java.io.File;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.HashMap;

import jsr166y.RecursiveTask;
import ccbr.utoronto.ca.dataStructure.comNodeAtom;
import ccbr.utoronto.ca.publicFunction.binarySearch;
import ccbr.utoronto.ca.publicFunction.getCanonicalLabel;
import ccbr.utoronto.ca.publicFunction.nCombinations;

/* This file deals with multiple motifs at a time; it's more powerful.*/

public class enumerateMultiMotifsFJTask extends RecursiveTask{

private static final long serialVersionUID = 1L;
	
	public static int SEQUENTIAL_THRESHOLD;
	
	int start;
	int end;
	//int currentThreadLevel;
	
	ArrayList<ArrayList<Integer>> summationsList = new ArrayList<ArrayList<Integer>>(); 
	
	int[][] adjMatrix;
	int motifSize;
	ArrayList<comNodeAtom> nodeList;
	
	ArrayList<Integer> microRNAList;
	ArrayList<Integer> tFactorList;
	ArrayList<Integer> geneList;
	
	boolean[] visited;
	
	//int subGraphNum = 0;
	FileOutputStream fos;
	String outputFileDir;
	
	ArrayList<Integer> Vext;
	ArrayList<Integer> Vsub;
	ArrayList<Integer> Vopen;
	
	int subGraphNum = 0;
	ArrayList<String> motifLabel;

	public enumerateMultiMotifsFJTask(int[][] adjMatrix, ArrayList<Integer> microRNAList, ArrayList<comNodeAtom> nodeList,
			ArrayList<String> motifLabel, int motifSize, int start, int end)
	{
		this.adjMatrix = adjMatrix;
		this.microRNAList = microRNAList;
		this.nodeList = nodeList;
		this.motifSize = motifSize;
		this.motifLabel = motifLabel;
		this.start = start;
		this.end = end;

		Vsub = new ArrayList<Integer>();
		Vext = new ArrayList<Integer>();
		Vopen = new ArrayList<Integer>();
	}
	
	public enumerateMultiMotifsFJTask(int[][] adjMatrix, ArrayList<Integer> microRNAList, ArrayList<comNodeAtom> nodeList,
			ArrayList<String> motifLabel, int motifSize, int start, int end, String outputFileDir)
	{
		this.adjMatrix = adjMatrix;
		this.microRNAList = microRNAList;
		this.nodeList = nodeList;
		this.motifSize = motifSize;
		this.motifLabel = motifLabel;
		this.start = start;
		this.end = end;
		this.outputFileDir = outputFileDir;

		Vsub = new ArrayList<Integer>();
		Vext = new ArrayList<Integer>();
		Vopen = new ArrayList<Integer>();
	}
	
	protected Integer compute()
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
				final enumerateMultiMotifsFJTask left = new enumerateMultiMotifsFJTask(adjMatrix, microRNAList, nodeList,
						motifLabel, motifSize, start, start + split, outputFileDir);
				final enumerateMultiMotifsFJTask right = new enumerateMultiMotifsFJTask(adjMatrix, microRNAList, nodeList,
						motifLabel, motifSize, start + split, end, outputFileDir);
				invokeAll(left, right);
				subGraphNum = (Integer)left.get() + (Integer)right.get();
			}
			catch(Exception e)
			{
				e.printStackTrace();
			}
		}
		return subGraphNum;
	}
	
	
	protected void computeDirectly()
	{
		String fileName = outputFileDir + "/" + "thread_" + String.valueOf(start) + ".sg";
		initialFileOutputStream(fileName);
		enumerateSubgraphs();
		closeFile();
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
	
	public void enumerateSubgraphs()
	{
		System.out.println("Start to extend the seed graphs...");
		
		//for each seed graph v belongs to seedGraphList, do
		for(int i = start;i < end;i++)
		{
			//long startTime = new java.util.Date().getTime();
			int curMiRNAID = microRNAList.get(i);
			
			Vext.clear();
			Vsub.clear();
			Vopen.clear();
			
			comNodeAtom curMiRNA = nodeList.get(curMiRNAID);
			for(int j = 0;j < curMiRNA.nodeInteraction.size();j++)
			{
				int currentPart = curMiRNA.nodeInteraction.get(j);
				if(currentPart > curMiRNAID)
				{
					Vext.add(currentPart);
					Vopen.add(currentPart);
				}
			}
			Vsub.add(curMiRNAID);
			
			extendSubgraph(Vsub, Vext, Vopen, curMiRNAID);		
			
		}
		System.out.println("Finish extending the seed graphs...");
	}

	public void extendSubgraph(ArrayList<Integer> Vsub, ArrayList<Integer> Vext, ArrayList<Integer> Vopen, int nodeV)
	{
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
		/*for(int i = 0;i < originalList.size();i++)
		{
			newArrayList.add(originalList.get(i));
		}*/
		return newArrayList;
	}
	
	public void countSubgraph(ArrayList<Integer> Vsub, ArrayList<Integer> Vext)
	{
		//double startTime = new java.util.Date().getTime();
		
		for(int i = 0;i < Vext.size();i++)
		{
			ArrayList<Integer> newVsub = duplicateArrayList(Vsub);
			//binarySearch.sort(newVsub, Vext.get(i));
			newVsub.add(Vext.get(i));		
			countGraphIso(newVsub);
			//countSubgraph(newVsub);
		}
		
		/*Double secs = new Double((new java.util.Date().getTime() - startTime)*0.001);
		System.out.println("Count subgraphs took " + secs + " secs." + " The Vext size is : " + Vext.size());*/
	}
	
	public void countGraphIso(ArrayList<Integer> curSubgraph)
	{
		//double startTime = new java.util.Date().getTime();
		
		getCanonicalLabel gcl = new getCanonicalLabel(curSubgraph, adjMatrix, nodeList);
		
		String curLabel = gcl.canonicalLabel();
		
		//if(motifLabel.compareTo(curLabel) == 0)
		if(binarySearch.contain(motifLabel, curLabel) == true)
		{
			subGraphNum++;
			//output the subgraph to file;
			//writeCurSubgraphtoFile(curSubgraph);
			writeCurSubgraphtoFile(gcl.canonicalGraph(), curLabel);
		}
		
		/*Double secs = new Double((new java.util.Date().getTime() - startTime) * 0.001);
		System.out.println("The graph isomorphism test took " + secs + " secs to finish.");*/
	}

	public void writeCurSubgraphtoFile(ArrayList<Integer> curSubGraph, String canonicalLabel)
	{
		try
		{
			for(int j = 0;j < curSubGraph.size();j++)
			{
				fos.write(String.valueOf(curSubGraph.get(j)).getBytes());
				fos.write("\t".getBytes());
			}
			fos.write(canonicalLabel.getBytes());
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
