package ccbr.utoronto.ca.publicFunction;

import java.util.ArrayList;
import java.util.HashMap;

import ccbr.utoronto.ca.dataStructure.comNodeAtom;

public class getCanonicalLabel {

	static int colorTypes = 3;
	
	String maximumLabel = "";
	
	ArrayList<Integer> canonicalGraph = null;
	
	public getCanonicalLabel(ArrayList<Integer> subgraph, int[][] adjMatrix, HashMap<Integer, Integer> nodeTypeMap)
	{
		ArrayList<ArrayList<Integer>> colorGroups = new ArrayList<ArrayList<Integer>>();
		for(int i = 0;i < colorTypes;i++)
		{
			ArrayList<Integer> newColorGroup = new ArrayList<Integer>();
			colorGroups.add(newColorGroup);
		}
		
		for(int i = 0;i < subgraph.size();i++)
		{
			int currentColor = nodeTypeMap.get(subgraph.get(i));
			colorGroups.get(currentColor).add(subgraph.get(i));
		}
		
		//Get the canonical labels: maximum lexicographic string
		ArrayList<ArrayList<int[]>> colorCombination = new ArrayList<ArrayList<int[]>>();
		for(int i = 0;i < colorGroups.size();i++)
		{
			ArrayList<Integer> currentColorGroup = colorGroups.get(i);
			permutate pm = new permutate(currentColorGroup);
			ArrayList<int[]> currentCombination = pm.getFullCombination();
			colorCombination.add(currentCombination);
		}
		
		arrange ar = new arrange(colorCombination);
		ArrayList<ArrayList<Integer>> shuffle = ar.getEnumeration();
		
		maximumLabel = ""; 
		canonicalGraph = null;
		
		for(int i = 0;i < shuffle.size();i++)
		{
			ArrayList<Integer> currentShuffle = shuffle.get(i);
			String currentLabel = "";
			String nodeType = "";
			for(int j = 0;j < currentShuffle.size();j++)
			{
				for(int k = 0;k < currentShuffle.size();k++)
				{
					currentLabel += String.valueOf(adjMatrix[currentShuffle.get(j)][currentShuffle.get(k)]);
				}
				nodeType += String.valueOf(nodeTypeMap.get(currentShuffle.get(j)));
			}
			currentLabel += "_" + nodeType;
			if(currentLabel.compareTo(maximumLabel) > 0)
			{
				maximumLabel = currentLabel;
				canonicalGraph = currentShuffle;
			}
		}
		//System.out.println("The canonical label is : " + maximumLabel);
	}
	
	public getCanonicalLabel(ArrayList<Integer> subgraph, int[][] adjMatrix, ArrayList<comNodeAtom> nodeList)
	{
		ArrayList<ArrayList<Integer>> colorGroups = new ArrayList<ArrayList<Integer>>();
		for(int i = 0;i < colorTypes;i++)
		{
			ArrayList<Integer> newColorGroup = new ArrayList<Integer>();
			colorGroups.add(newColorGroup);
		}
		
		for(int i = 0;i < subgraph.size();i++)
		{
			int currentColor = nodeList.get(subgraph.get(i)).nodeType;
			colorGroups.get(currentColor).add(subgraph.get(i));
		}
		
		//Get the canonical labels: maximum lexicographic string
		ArrayList<ArrayList<int[]>> colorCombination = new ArrayList<ArrayList<int[]>>();
		for(int i = 0;i < colorGroups.size();i++)
		{
			ArrayList<Integer> currentColorGroup = colorGroups.get(i);
			permutate pm = new permutate(currentColorGroup);
			ArrayList<int[]> currentCombination = pm.getFullCombination();
			colorCombination.add(currentCombination);
		}
		
		arrange ar = new arrange(colorCombination);
		ArrayList<ArrayList<Integer>> shuffle = ar.getEnumeration();
		
		maximumLabel = ""; 
		canonicalGraph = null;
		
		for(int i = 0;i < shuffle.size();i++)
		{
			ArrayList<Integer> currentShuffle = shuffle.get(i);
			String currentLabel = "";
			String nodeType = "";
			for(int j = 0;j < currentShuffle.size();j++)
			{
				for(int k = 0;k < currentShuffle.size();k++)
				{
					currentLabel += String.valueOf(adjMatrix[currentShuffle.get(j)][currentShuffle.get(k)]);
				}
				nodeType += String.valueOf(nodeList.get(currentShuffle.get(j)).nodeType);
			}
			currentLabel += "_" + nodeType;
			if(currentLabel.compareTo(maximumLabel) > 0)
			{
				maximumLabel = currentLabel;
				canonicalGraph = currentShuffle;
			}
		}
		//System.out.println("The canonical label is : " + maximumLabel);
	}
	
	public getCanonicalLabel(String subgraphLabel)
	{
		String[] labelInfo = subgraphLabel.split("_");
		String subMatrixInfo = labelInfo[0];
		String colorInfo = labelInfo[1];
		
		int length = colorInfo.length();
		int[][] adjMatrix = new int[length][length];
		for(int i = 0;i < adjMatrix.length;i++)
		{
			for(int j = 0;j < adjMatrix[i].length;j++)
			{
				adjMatrix[i][j] = Integer.valueOf(String.valueOf(subMatrixInfo.charAt(i * length + j)));
			}
		}
		
		ArrayList<ArrayList<Integer>> colorGroups = new ArrayList<ArrayList<Integer>>();
		for(int i = 0;i < colorTypes;i++)
		{
			ArrayList<Integer> newColorGroup = new ArrayList<Integer>();
			colorGroups.add(newColorGroup);
		}
		
		for(int i = 0;i < labelInfo[1].length();i++)
		{
			int currentColor = Integer.valueOf(String.valueOf(colorInfo.charAt(i)));
			colorGroups.get(currentColor).add(i);
		}
		
		//Get the canonical labels: maximum lexicographic string
		ArrayList<ArrayList<int[]>> colorCombination = new ArrayList<ArrayList<int[]>>();
		for(int i = 0;i < colorGroups.size();i++)
		{
			ArrayList<Integer> currentColorGroup = colorGroups.get(i);
			permutate pm = new permutate(currentColorGroup);
			ArrayList<int[]> currentCombination = pm.getFullCombination();
			colorCombination.add(currentCombination);
		}
		
		arrange ar = new arrange(colorCombination);
		ArrayList<ArrayList<Integer>> shuffle = ar.getEnumeration();
		
		maximumLabel = ""; 
		canonicalGraph = null;
		
		for(int i = 0;i < shuffle.size();i++)
		{
			ArrayList<Integer> currentShuffle = shuffle.get(i);
			String currentLabel = "";
			String nodeType = "";
			for(int j = 0;j < currentShuffle.size();j++)
			{
				for(int k = 0;k < currentShuffle.size();k++)
				{
					currentLabel += String.valueOf(adjMatrix[currentShuffle.get(j)][currentShuffle.get(k)]);
				}
				nodeType += String.valueOf(colorInfo.charAt(currentShuffle.get(j)));
			}
			currentLabel += "_" + nodeType;
			if(currentLabel.compareTo(maximumLabel) > 0)
			{
				maximumLabel = currentLabel;
				canonicalGraph = currentShuffle;
			}
		}
		//System.out.println("The canonical label is : " + maximumLabel);
	}
	
	public static void setColorTypes(int num)
	{
		colorTypes = num; 
	}
	
	public String canonicalLabel()
	{
		return maximumLabel;
	}
	
	public ArrayList<Integer> canonicalGraph()
	{
		return canonicalGraph;
	}
	
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		
		//int[][] adjMatrix = {{0,1,1,1}, {0,0,1,1}, {0,0,0,0}, {0,0,0,0}};
		//int[][] adjMatrix = {{0,0,0,3}, {0,0,0,0}, {4,0,0,1}, {0,0,0,0}};
		int[][] adjMatrix = {{0,0,0,1}, {0,0,0,1}, {0,0,0,1}, {0,0,0,0}};
		ArrayList<Integer> subgraph = new ArrayList<Integer>();
		HashMap<Integer, Integer> nodeTypeMap = new HashMap<Integer, Integer>(); 
		for(int i = 0;i < adjMatrix.length;i++)
		{
			subgraph.add(i);
			if(i == 0 || i == 1)
			{
				nodeTypeMap.put(i, 0);
			}
			else if(i == 2)
			{
				nodeTypeMap.put(i, 0);
			}
			else
			{
				nodeTypeMap.put(i, 0);
			}
		}
		
		getCanonicalLabel gcl = new getCanonicalLabel(subgraph, adjMatrix, nodeTypeMap);
		
		System.out.println(gcl.canonicalLabel());
	}

}
