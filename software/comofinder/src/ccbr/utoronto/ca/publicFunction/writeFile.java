package ccbr.utoronto.ca.publicFunction;

import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.HashMap;

import ccbr.utoronto.ca.dataStructure.comNodeAtom;
import ccbr.utoronto.ca.dataStructure.edgeAtom;

public class writeFile {

	public static void writeRegulations(ArrayList<comNodeAtom> nodeList, String fileName)
	{
		try
		{
			FileOutputStream fos = new FileOutputStream(fileName);
			for(int i = 0;i < nodeList.size();i++)
			{
				comNodeAtom node = nodeList.get(i);
				for(int j = 0;j < node.nodeInteraction.size();j++)
				{
					comNodeAtom partner = nodeList.get(node.nodeInteraction.get(j));
					fos.write(String.valueOf(node.nodeID).getBytes());
					fos.write("\t".getBytes());
					fos.write(String.valueOf(partner.nodeID).getBytes());
					fos.write("\t".getBytes());
					fos.write(String.valueOf(node.nodeType).getBytes());
					fos.write("\t".getBytes());
					fos.write(String.valueOf(partner.nodeType).getBytes());
					fos.write("\n".getBytes());
				}
			}
			fos.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static void writeRegulationsOnly(HashMap<Integer, Integer> nodeTypeMap, int[][] adjMatrix, String fileName)
	{
		try
		{
			FileOutputStream fos = new FileOutputStream(fileName);
			for(int i = 0;i < adjMatrix.length;i++)
			{
				for(int j = 0;j < adjMatrix[i].length;j++)
				{
					if(adjMatrix[i][j] > 0)
					{
						fos.write(String.valueOf(i).getBytes());
						fos.write("\t".getBytes());
						fos.write(String.valueOf(j).getBytes());
						fos.write("\n".getBytes());
					}
				}
			}
			fos.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static void writeRegulationsOnly(ArrayList<edgeAtom> edgeList, String fileName)
	{
		try
		{
			FileOutputStream fos = new FileOutputStream(fileName);
			for(int i = 0;i < edgeList.size();i++)
			{
				fos.write(String.valueOf(edgeList.get(i).sourceID).getBytes());
				fos.write("\t".getBytes());
				fos.write(String.valueOf(edgeList.get(i).targetID).getBytes());
				fos.write("\n".getBytes());
			}
			fos.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static void writeRegulationsOnly(int[][] adjMatrix, String fileName)
	{
		try
		{
			FileOutputStream fos = new FileOutputStream(fileName);
			for(int i = 0;i < adjMatrix.length;i++)
			{
				for(int j = 0;j < adjMatrix[i].length;j++)
				{
					if(adjMatrix[i][j] != 0)
					{
						fos.write(String.valueOf(i).getBytes());
						fos.write("\t".getBytes());
						fos.write(String.valueOf(j).getBytes());
						fos.write("\n".getBytes());
					}
				}
				
			}
			fos.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static void writeRegulationswithEdgeTypes(ArrayList<comNodeAtom> nodeList, int[][] adjMatrix, String fileName)
	{
		try
		{
			FileOutputStream fos = new FileOutputStream(fileName);
			for(int i = 0;i < adjMatrix.length;i++)
			{
				for(int j = 0;j < adjMatrix[i].length;j++)
				{
					if(adjMatrix[i][j] > 0)
					{
						fos.write(String.valueOf(i).getBytes());
						fos.write("\t".getBytes());
						fos.write(String.valueOf(j).getBytes());
						fos.write("\t".getBytes());
						fos.write(String.valueOf(nodeList.get(i).nodeType).getBytes());
						fos.write("\t".getBytes());
						fos.write(String.valueOf(nodeList.get(j).nodeType).getBytes());
						fos.write("\t".getBytes());
						fos.write(String.valueOf(adjMatrix[i][j]).getBytes());
						fos.write("\n".getBytes());
					}
				}
			}
			fos.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static void writeRegulationswithEdgeTypes(HashMap<Integer, Integer> nodeTypeMap, int[][] adjMatrix, String fileName)
	{
		try
		{
			FileOutputStream fos = new FileOutputStream(fileName);
			for(int i = 0;i < adjMatrix.length;i++)
			{
				for(int j = 0;j < adjMatrix[i].length;j++)
				{
					if(adjMatrix[i][j] > 0)
					{
						fos.write(String.valueOf(i).getBytes());
						fos.write("\t".getBytes());
						fos.write(String.valueOf(j).getBytes());
						fos.write("\t".getBytes());
						fos.write(String.valueOf(nodeTypeMap.get(i)).getBytes());
						fos.write("\t".getBytes());
						fos.write(String.valueOf(nodeTypeMap.get(j)).getBytes());
						fos.write("\t".getBytes());
						fos.write(String.valueOf(adjMatrix[i][j]).getBytes());
						fos.write("\n".getBytes());
					}
				}
			}
			fos.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static void writeNodeIDsandNames(ArrayList<comNodeAtom> nodeList, String fileName)
	{
		try
		{
			FileOutputStream fos = new FileOutputStream(fileName);
			for(int i = 0;i < nodeList.size();i++)
			{
				comNodeAtom node = nodeList.get(i);
				fos.write(String.valueOf(node.nodeID).getBytes());
				fos.write("\t".getBytes());
				fos.write(node.nodeName.getBytes());
				fos.write("\n".getBytes());
			}
			fos.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}	
	
	public static void writeEdgesandVerticesSeparately(ArrayList<comNodeAtom> nodeList, int[][] adjMatrix, String fileName, boolean header)
	{
		try
		{
			String verticesFile = fileName + ".vertices";
			FileOutputStream fosV = new FileOutputStream(verticesFile);
			String edgesFile = fileName + ".edges";
			FileOutputStream fosE = new FileOutputStream(edgesFile);
			
			if(header == true)
			{
				fosV.write("name\tcolor\n".getBytes());
			}
			
			for(int i = 0;i < adjMatrix.length;i++)
			{
				fosV.write(String.valueOf(nodeList.get(i).nodeID).getBytes());
				fosV.write("\t".getBytes());
				fosV.write(String.valueOf(nodeList.get(i).nodeType).getBytes());
				fosV.write("\n".getBytes());
				
				for(int j = 0;j < adjMatrix[i].length;j++)
				{
					if(adjMatrix[i][j] > 0)
					{
						fosE.write(String.valueOf(i).getBytes());
						fosE.write("\t".getBytes());
						fosE.write(String.valueOf(j).getBytes());
						fosE.write("\n".getBytes());
					}
				}
			}
			fosE.close();
			fosV.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static void writeEdgesandVerticesSeparately(HashMap<Integer, Integer> nodeTypeMap, int[][] adjMatrix, String fileName)
	{
		try
		{
			String verticesFile = fileName + ".vertices";
			FileOutputStream fosV = new FileOutputStream(verticesFile);
			String edgesFile = fileName + ".edges";
			FileOutputStream fosE = new FileOutputStream(edgesFile);
			
			fosV.write("name\tcolor\n".getBytes());
			for(int i = 0;i < adjMatrix.length;i++)
			{
				fosV.write(String.valueOf(i).getBytes());
				fosV.write("\t".getBytes());
				fosV.write(String.valueOf(nodeTypeMap.get(i)).getBytes());
				fosV.write("\n".getBytes());
				
				for(int j = 0;j < adjMatrix[i].length;j++)
				{
					if(adjMatrix[i][j] > 0)
					{
						fosE.write(String.valueOf(i).getBytes());
						fosE.write("\t".getBytes());
						fosE.write(String.valueOf(j).getBytes());
						fosE.write("\n".getBytes());
					}
				}
			}
			fosE.close();
			fosV.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
}
