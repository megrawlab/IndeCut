package ccbr.utoronto.ca.readData;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import ccbr.utoronto.ca.dataStructure.comNodeAtom;
import ccbr.utoronto.ca.dataStructure.edgeAtom;
import ccbr.utoronto.ca.publicFunction.binarySearch;

public class readStandardInput {

	HashMap<Integer, Integer> nodeTypeMap = new HashMap<Integer, Integer>();
	//HashMap<String, Integer> edgeTypeMap = new HashMap<String, Integer>();
	ArrayList<edgeAtom> edgeList = new ArrayList<edgeAtom>();
	ArrayList<comNodeAtom> nodeList = new ArrayList<comNodeAtom>();
	ArrayList<Integer> nodeIDList = new ArrayList<Integer>();
	int[][] adjMatrix;
	
	public readStandardInput(String fileName)
	{
		try
		{
			BufferedReader br = new BufferedReader(new FileReader(fileName));
			String temp = null;
			while((temp = br.readLine()) != null)
			{
				String[] info = temp.split("\t");
				int firstNodeID = Integer.valueOf(info[0]);
				int secondNodeID = Integer.valueOf(info[1]);
				int firstNodeType = Integer.valueOf(info[2]);
				int secondNodeType = Integer.valueOf(info[3]);
				if(nodeTypeMap.containsKey(firstNodeID) == false)
				{
					//binarySearch.sort(nodeIDList, firstNodeID);
					nodeTypeMap.put(firstNodeID, firstNodeType);
				}
				if(nodeTypeMap.containsKey(secondNodeID) == false)
				{
					//binarySearch.sort(nodeIDList, secondNodeID);
					nodeTypeMap.put(secondNodeID, secondNodeType);
				}
				
				int edgeType = -1;
				if(info.length == 4)
				{
					edgeType = 1;
				}
				else
				{
					edgeType = Integer.valueOf(info[4]);
				}
				/*String edgeKey = firstNodeID + "_" + secondNodeID;
				edgeTypeMap.put(edgeKey, edgeType);*/
				edgeAtom edge = new edgeAtom(firstNodeID, secondNodeID);
				edge.edgeType = edgeType;
				edgeList.add(edge);
			}
			br.close();
			
			getNodesandEdges();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public readStandardInput(String edgesFile, String verticesFile)
	{
		try
		{
			BufferedReader br = new BufferedReader(new FileReader(verticesFile));
			String temp = null;
			while((temp = br.readLine()) != null)
			{
				String[] info = temp.split("\t");
				nodeTypeMap.put(Integer.valueOf(info[0]), Integer.valueOf(info[1]));
			}
			br.close();
			
			br = new BufferedReader(new FileReader(edgesFile));
			temp = null;
			while((temp = br.readLine()) != null)
			{
				String[] info = temp.split("\t");
				int firstNodeID = Integer.valueOf(info[0]);
				int secondNodeID = Integer.valueOf(info[1]);

				int edgeType = -1;
				if(info.length < 5)
				{
					edgeType = 1;
				}
				else
				{
					edgeType = Integer.valueOf(info[4]);
				}
				/*String edgeKey = firstNodeID + "_" + secondNodeID;
				edgeTypeMap.put(edgeKey, edgeType);*/
				edgeAtom edge = new edgeAtom(firstNodeID, secondNodeID);
				edge.edgeType = edgeType;
				edgeList.add(edge);
			}
			br.close();
			
			getNodesandEdges();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public void getNodesandEdges()
	{
		for(int i = 0;i < nodeTypeMap.size();i++)
		{
			comNodeAtom atom = new comNodeAtom();
			atom.nodeID = i;
			atom.nodeInteraction = new ArrayList<Integer>();
			if(nodeTypeMap.containsKey(i) == false)
			{
				System.err.println(i + " no node type;");
				System.exit(0);
			}
			atom.nodeType = nodeTypeMap.get(i);
			nodeList.add(atom);
		}
		
		adjMatrix = new int[nodeTypeMap.size()][nodeTypeMap.size()];
		
		for(int i = 0;i < edgeList.size();i++)
		{
			edgeAtom edge = edgeList.get(i);
			adjMatrix[edge.sourceID][edge.targetID] = edge.edgeType;
			
			binarySearch.sort(nodeList.get(edge.sourceID).nodeInteraction, edge.targetID);
			binarySearch.sort(nodeList.get(edge.targetID).nodeInteraction, edge.sourceID);
		}
	}
	
	public void setNodeNames(String fileName)
	{
		try
		{
			BufferedReader br = new BufferedReader(new FileReader(fileName));
			String temp = null;
			while((temp = br.readLine()) != null)
			{
				String[] info = temp.split("\t");
				int nodeID = Integer.valueOf(info[0]);
				String nodeName = info[1];
				nodeList.get(nodeID).nodeName = nodeName;
			}
			br.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public HashMap<Integer, Integer> getNodeTypeMap()
	{
		return nodeTypeMap;
	}
	
	public ArrayList<comNodeAtom> getNodeList()
	{
		return nodeList;
	}
	
	public int[][] getAdjMatrix()
	{
		return adjMatrix;
	}
}
