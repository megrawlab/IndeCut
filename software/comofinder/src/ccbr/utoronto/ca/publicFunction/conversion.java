package ccbr.utoronto.ca.publicFunction;

import java.util.ArrayList;
import java.util.HashMap;

import ccbr.utoronto.ca.dataStructure.comNodeAtom;

public class conversion {

	public static String getLabel(int[][] adjMatrix, HashMap<Integer, Integer> nodeTypeMap, int[] curSubgraph)
	{
		String label = "";
		String nodeType = "";

		for(int m = 0;m < curSubgraph.length;m++)
		{
			for(int n = 0;n < curSubgraph.length;n++)
			{
				label += String.valueOf(adjMatrix[curSubgraph[m]][curSubgraph[n]]);
			}
			nodeType += String.valueOf(nodeTypeMap.get(curSubgraph[m]));
		}
		
		label += "_" + nodeType;
		
		return label;
	}
	
	public static String getLabel(int[][] adjMatrix, ArrayList<comNodeAtom> nodeList, int[] curSubgraph)
	{
		String label = "";
		String nodeType = "";

		for(int m = 0;m < curSubgraph.length;m++)
		{
			for(int n = 0;n < curSubgraph.length;n++)
			{
				label += String.valueOf(adjMatrix[curSubgraph[m]][curSubgraph[n]]);
			}
			nodeType += String.valueOf(nodeList.get(curSubgraph[m]).nodeType);
		}
		
		label += "_" + nodeType;
		
		return label;
	}
	
	public static String getLabel(int[][] adjMatrix, HashMap<Integer, Integer> nodeTypeMap, ArrayList<Integer> curSubgraph)
	{
		String label = "";
		String nodeType = "";

		for(int m = 0;m < curSubgraph.size();m++)
		{
			for(int n = 0;n < curSubgraph.size();n++)
			{
				label += String.valueOf(adjMatrix[curSubgraph.get(m)][curSubgraph.get(n)]);
			}
			nodeType += String.valueOf(nodeTypeMap.get(curSubgraph.get(m)));
		}
		
		label += "_" + nodeType;
		
		return label;
	}
	
	public static String getLabel(int[][] adjMatrix, ArrayList<comNodeAtom> nodeList, ArrayList<Integer> curSubgraph)
	{
		String label = "";
		String nodeType = "";

		for(int m = 0;m < curSubgraph.size();m++)
		{
			for(int n = 0;n < curSubgraph.size();n++)
			{
				label += String.valueOf(adjMatrix[curSubgraph.get(m)][curSubgraph.get(n)]);
			}
			nodeType += String.valueOf(nodeList.get(curSubgraph.get(m)).nodeType);
		}
		
		label += "_" + nodeType;
		
		return label;
	}	
}
