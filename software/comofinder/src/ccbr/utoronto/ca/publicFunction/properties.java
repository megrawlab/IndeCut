package ccbr.utoronto.ca.publicFunction;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import ccbr.utoronto.ca.dataStructure.comNodeAtom;

public class properties {

	public static void getNumofNodesfromDifferentTypes(ArrayList<comNodeAtom> nodeList)
	{
		int mCount = 0, tCount = 0, gCount = 0;
		for(int i = 0;i < nodeList.size();i++)
		{
			if(nodeList.get(i).nodeType == 0)
			{
				mCount++;
			}
			else if(nodeList.get(i).nodeType == 1)
			{
				tCount++;
			}
			else if(nodeList.get(i).nodeType == 2)
			{
				gCount++;
			}
			else
			{
				System.err.println("There are more than three types of nodes. Please Check the co-regulatory network!");
				System.exit(0);
			}
		}
		System.out.println("There are " + mCount + " microRNAs, " + tCount + " TFs and " + gCount + " genes.");
	}
	
	public static int getEdgeNums(int[][] adjMatrix)
	{
		int edgeNum = 0;
		for(int i = 0;i < adjMatrix.length;i++)
		{
			for(int j = 0;j < adjMatrix[0].length;j++)
			{
				if(adjMatrix[i][j] == 1)
				{
					edgeNum++;
				}
			}
		}
		
		//System.out.println("There are " + edgeNum + " edges in the matrix.");
		return edgeNum;
	}
	
	public static void getDifferentTypeofEdgeNums(int[][] adjMatrix, ArrayList<comNodeAtom> nodeList)
	{
		HashMap<String, Integer> differentTypeofEdges = new HashMap<String, Integer>();
		for(int i = 0;i < adjMatrix.length;i++)
		{
			for(int j = 0;j < adjMatrix[0].length;j++)
			{
				if(adjMatrix[i][j] == 1)
				{
					String edgeType = String.valueOf(nodeList.get(i).nodeType) + String.valueOf(nodeList.get(j).nodeType);
					if(differentTypeofEdges.containsKey(edgeType))
					{
						int currentNum = differentTypeofEdges.get(edgeType);
						currentNum++;
						differentTypeofEdges.put(edgeType, currentNum);
					}
					else
					{
						differentTypeofEdges.put(edgeType, 1);
					}
				}
			}
		}
		
		for(Iterator<String> itKey = differentTypeofEdges.keySet().iterator();itKey.hasNext();)
		{
			System.out.print("There are :");
			String key = itKey.next();
			int edgeNum = differentTypeofEdges.get(key);
			if(key.compareTo("01") == 0)
			{
				System.out.print(" " + edgeNum + " miRNA->TFs regulations;");
			}
			else if(key.compareTo("02") == 0)
			{
				System.out.print(" " + edgeNum + " miRNA->genes regulations;");
			}
			else if(key.compareTo("10") == 0)
			{
				System.out.print(" " + edgeNum + " TF->miRNAs regulations;");
			}
			else if(key.compareTo("12") == 0)
			{
				System.out.print(" " + edgeNum + " TF->genes regulations;");
			}
		}
	}
	
	/*This function is used to test whether a gene is purely a gene or actually a TF.*/
	public static ArrayList<Integer> testGeneProperties(ArrayList<comNodeAtom> nodeList)
	{
		ArrayList<Integer> TFGeneList = new ArrayList<Integer>();
		for(int i = 0;i < nodeList.size();i++)
		{
			if(nodeList.get(i).nodeType == 2)
			{
				comNodeAtom gene = nodeList.get(i);
				for(int j = 0;j < gene.nodeInteraction.size();j++)
				{
					if(nodeList.get(gene.nodeInteraction.get(j)).nodeType != 2)
					{
						System.err.println(gene.nodeName + " has an interaction with a TF " + nodeList.get(gene.nodeInteraction.get(j)).nodeName
								+ "_" + nodeList.get(gene.nodeInteraction.get(j)).nodeType);
						//TFGeneList.add(gene.nodeInteraction.get(j));
						binarySearch.sort(TFGeneList, gene.nodeInteraction.get(j));
					}
				}
			}
		}
		return TFGeneList;
	}
	
	public static void extendSelfRegulationtoNodeList(ArrayList<comNodeAtom> nodeList, HashMap<Integer, Integer> nodeTypeMap, ArrayList<Integer> selfRegulationList)
	{		
		System.out.println("There are " + selfRegulationList.size() + " self-regulators.");
		for(int i = 0;i < selfRegulationList.size();i++)
		{
			comNodeAtom atom = nodeList.get(selfRegulationList.get(i));
			
			comNodeAtom copyAtom = new comNodeAtom();
			copyAtom.nodeID = nodeList.size();
			copyAtom.nodeType = 2;
			//Here we give the TF a new name just to differ them from their gene forms.
			//copyAtom.nodeName = atom.nodeName + "_gene";
			copyAtom.nodeName = atom.nodeName;
			copyAtom.nodeInteraction = new ArrayList<Integer>();
			
			nodeList.add(copyAtom);
			nodeTypeMap.put(copyAtom.nodeID, copyAtom.nodeType);
			
			for(int j = 0;j < nodeList.size();j++)
			{
				comNodeAtom regulator = nodeList.get(j);
				if(binarySearch.contain(regulator.nodeInteraction, atom.nodeID) == true)
				{
					binarySearch.sort(regulator.nodeInteraction, copyAtom.nodeID);
				}
			}
			
			binarySearch.remove(atom.nodeInteraction, atom.nodeID);
			binarySearch.sort(atom.nodeInteraction, copyAtom.nodeID);
		}
	}
	
	public static void extendNodeList(ArrayList<comNodeAtom> nodeList, HashMap<Integer, Integer> nodeTypeMap, ArrayList<Integer> TFGeneList, ArrayList<Integer> selfRegulationList)
	{		
		int TFasGenes = TFGeneList.size();
		for(int i = 0;i < TFGeneList.size();i++)
		{
			comNodeAtom atom = nodeList.get(TFGeneList.get(i));
			comNodeAtom copyAtom = null;
			
			if(binarySearch.contain(selfRegulationList, atom.nodeID) == true)
			{
				TFasGenes--;
				for(int j = (nodeList.size() - selfRegulationList.size());j < nodeList.size();j++)
				{
					if(nodeList.get(j).nodeName.compareToIgnoreCase(atom.nodeName) == 0)
					{
						copyAtom = nodeList.get(j);
						break;
					}
				}
			}
			else
			{
				copyAtom = new comNodeAtom();
				copyAtom.nodeID = nodeList.size();
				copyAtom.nodeType = 2;
				//Here we give the TF a new name just to differ them from their gene forms.
				//copyAtom.nodeName = atom.nodeName + "_gene";
				copyAtom.nodeName = atom.nodeName;
				copyAtom.nodeInteraction = new ArrayList<Integer>();
				
				nodeList.add(copyAtom);
				nodeTypeMap.put(copyAtom.nodeID, copyAtom.nodeType);
				
			}
			
			for(int j = 0;j < nodeList.size();j++)
			{
				comNodeAtom node = nodeList.get(j);
				if(binarySearch.contain(node.nodeInteraction, atom.nodeID) == true)
				{
					binarySearch.sort(node.nodeInteraction, copyAtom.nodeID);
					if(node.nodeType == 2)
					{
						binarySearch.sort(copyAtom.nodeInteraction, node.nodeID);
						binarySearch.remove(node.nodeInteraction, atom.nodeID);
					}
				}
			}
		}
		
		System.out.println("There are " + TFasGenes + " TFs acting like genes at the same time.");
	}
	
}
