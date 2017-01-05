package ccbr.utoronto.ca.networkRandomization;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Random;

import ccbr.utoronto.ca.dataStructure.edgeAtom;

/* This file is derived from the "maximumDifferenceRandomization.java file".
 * */

public class MDR_multipleEdgeTypes {

	int[][] adjMatrix;//This is the original matrix.
	public int[][] randomMatrix;//This is the new matrix after shuffling.

	/*int successAttempt = 0;
	int totalNumofEdges = 0;*/
	
	int maxTrials = 100;
	HashMap<String, ArrayList<edgeAtom>> edgeListMap;
	
	HashMap<Integer, Integer> nodeTypeMap;
	private String randfilename;
	
	public MDR_multipleEdgeTypes(int[][] adjMatrix, HashMap<Integer, Integer> nodeTypeMap)
	{
		this.adjMatrix = adjMatrix;
		this.nodeTypeMap = nodeTypeMap;
		
		randomMatrix = copyMatrix(adjMatrix);
	}
	
	public void setMaxTrials(int maxTrials)
	{
		this.maxTrials = maxTrials;
	}
	
	public void run()
	{
		System.out.println("===========Started to randomly shuffle the network.===========");
		
		edgeListMap = getDifferentEdgeGroups(adjMatrix);
		
		System.out.println("There are " + edgeListMap.size() + " types of edges.");
		
		//We shuffle the edges in each group separately.
		for(Iterator<String> itKey = edgeListMap.keySet().iterator();itKey.hasNext();)
		{
			String key = itKey.next();
			ArrayList<edgeAtom> currentEdgeList = edgeListMap.get(key);			
			shuffleEdges(currentEdgeList, key.split("_")[1]);
		}
		
		//Further check the network;
		validateDifferentEdgeGroups();		
		//validateRandomization();
		
		/** Added by Mitra */
		outputRandFile(randfilename);
		System.out.println("===========Successfully shuffled the network.===========");
	}
	
	public HashMap<String, ArrayList<edgeAtom>> getDifferentEdgeGroups(int[][] adjMatrix)
	{
		HashMap<String, ArrayList<edgeAtom>> edgeListMap = new HashMap<String, ArrayList<edgeAtom>>();
		
		for(int i = 0;i < adjMatrix.length;i++)
		{
			for(int j = 0;j < adjMatrix[i].length;j++)
			{
				//This is for normal regulations between miRNAs -> genes/TFs and TF -> miRNAs/TFs/genes;
				if(adjMatrix[i][j] == 1 && adjMatrix[j][i] == 0)
				{
					//regTypeKey should be "01_10" or "10_10";
					String regTypeKey = String.valueOf(nodeTypeMap.get(i)) + String.valueOf(nodeTypeMap.get(j));
					regTypeKey += "_" + String.valueOf(adjMatrix[i][j]) + String.valueOf(adjMatrix[j][i]);
					
					if(edgeListMap.containsKey(regTypeKey))
					{
						ArrayList<edgeAtom> curRegArray = edgeListMap.get(regTypeKey);
						edgeAtom nt = new edgeAtom(i,j);
						curRegArray.add(nt);
					}
					else
					{
						ArrayList<edgeAtom> curRegArray = new ArrayList<edgeAtom>();
						edgeAtom nt = new edgeAtom(i,j);
						curRegArray.add(nt);
						edgeListMap.put(regTypeKey, curRegArray);
					}
				}
				//This is for bidirected regulations between miRNAs/TFs and TFs;
				else if(adjMatrix[i][j] == 1 && adjMatrix[j][i] == 1)
				{
					if(i >= j)
					{
						//Skip the self-regulations and only count once for bidirected(undirected) edges.
						continue;
					}
					//regTypeKey should be "01_11" or "11_11";
					String regTypeKey = String.valueOf(nodeTypeMap.get(i)) + String.valueOf(nodeTypeMap.get(j));
					regTypeKey += "_" + String.valueOf(adjMatrix[i][j]) + String.valueOf(adjMatrix[j][i]);
					
					if(edgeListMap.containsKey(regTypeKey))
					{
						ArrayList<edgeAtom> curRegArray = edgeListMap.get(regTypeKey);
						edgeAtom nt = new edgeAtom(i,j);
						curRegArray.add(nt);
					}
					else
					{
						ArrayList<edgeAtom> curRegArray = new ArrayList<edgeAtom>();
						edgeAtom nt = new edgeAtom(i,j);
						curRegArray.add(nt);
						edgeListMap.put(regTypeKey, curRegArray);
					}			
				}
				//This if for the gene-gene interactions;
				else if(adjMatrix[i][j] == 2 && adjMatrix[j][i] == 2)
				{
					if(i >= j)
					{
						//Skip the self-regulations and only count once for gene interactions.
						continue;
					}
					
					//regTypeKey should be "22_22";
					String regTypeKey = String.valueOf(nodeTypeMap.get(i)) + String.valueOf(nodeTypeMap.get(j));
					regTypeKey += "_" + String.valueOf(adjMatrix[i][j]) + String.valueOf(adjMatrix[j][i]);
					
					if(edgeListMap.containsKey(regTypeKey))
					{
						ArrayList<edgeAtom> curRegArray = edgeListMap.get(regTypeKey);
						edgeAtom nt = new edgeAtom(i,j);
						curRegArray.add(nt);
					}
					else
					{
						ArrayList<edgeAtom> curRegArray = new ArrayList<edgeAtom>();
						edgeAtom nt = new edgeAtom(i,j);
						curRegArray.add(nt);
						edgeListMap.put(regTypeKey, curRegArray);
					}			
				}
				else if(adjMatrix[i][j] == 3 && adjMatrix[j][i] == 2)
				{
					//regTypeKey should be "11_32" or "12_32"
					String regTypeKey = String.valueOf(nodeTypeMap.get(i)) + String.valueOf(nodeTypeMap.get(j));
					regTypeKey += "_" + String.valueOf(adjMatrix[i][j]) + String.valueOf(adjMatrix[j][i]);
					
					if(edgeListMap.containsKey(regTypeKey))
					{
						ArrayList<edgeAtom> curRegArray = edgeListMap.get(regTypeKey);
						edgeAtom nt = new edgeAtom(i,j);
						curRegArray.add(nt);
					}
					else
					{
						ArrayList<edgeAtom> curRegArray = new ArrayList<edgeAtom>();
						edgeAtom nt = new edgeAtom(i,j);
						curRegArray.add(nt);
						edgeListMap.put(regTypeKey, curRegArray);
					}
				}
				else if(adjMatrix[i][j] == 3 && adjMatrix[j][i] == 3)
				{
					if(i >= j)
					{
						continue;
					}
					//regTypeKey should be "11_33";
					String regTypeKey = String.valueOf(nodeTypeMap.get(i)) + String.valueOf(nodeTypeMap.get(j));
					regTypeKey += "_" + String.valueOf(adjMatrix[i][j]) + String.valueOf(adjMatrix[j][i]);
					
					if(edgeListMap.containsKey(regTypeKey))
					{
						ArrayList<edgeAtom> curRegArray = edgeListMap.get(regTypeKey);
						edgeAtom nt = new edgeAtom(i,j);
						curRegArray.add(nt);
					}
					else
					{
						ArrayList<edgeAtom> curRegArray = new ArrayList<edgeAtom>();
						edgeAtom nt = new edgeAtom(i,j);
						curRegArray.add(nt);
						edgeListMap.put(regTypeKey, curRegArray);
					}
				}
			}
		}
		
		return edgeListMap;
	}
	
	public void shuffleEdges(ArrayList<edgeAtom> edgeList, String edgeType)
	{
		System.out.println("Edge type: " + edgeType + "; " + "node type: " + nodeTypeMap.get(edgeList.get(0).sourceID) 
				+ nodeTypeMap.get(edgeList.get(0).targetID)	+ "; there are " + edgeList.size() + " edges in the current edge list.");
		
		int failedAttempts = 0;
		int maxDiff = edgeList.size();
		//int maxTrials = 100;
		
		ArrayList<edgeAtom> shuffledEdges = new ArrayList<edgeAtom>();
		Random rd = new Random();
		
		while(failedAttempts < maxTrials && edgeList.size() > 0)
		{			
			int firstPos = -1, secondPos = -1;
			edgeAtom firstEdge = null, secondEdge = null;
			boolean switchFlag = false;
			
			double prob = ((double) edgeList.size()) / ((double) (edgeList.size() + shuffledEdges.size()));
			if(rd.nextDouble() < prob)
			{
				firstPos = rd.nextInt(edgeList.size());
				secondPos = rd.nextInt(edgeList.size());			
				firstEdge = edgeList.get(firstPos);
				secondEdge = edgeList.get(secondPos);
			}
			else
			{
				firstPos = rd.nextInt(edgeList.size());
				secondPos = rd.nextInt(shuffledEdges.size());
				firstEdge = edgeList.get(firstPos);
				secondEdge = shuffledEdges.get(secondPos);
				
				switchFlag = true;
			}
				
			//System.out.println("Choosing two edges from " + switchFlag);
			/*System.out.println("The first edge is " + firstEdge.sourceID + "==>" + firstEdge.targetID 
					+ " && the second edge is " + secondEdge.sourceID + "==>" + secondEdge.targetID);*/
			
			//Add one more restriction;
			if(firstEdge.sourceID == secondEdge.sourceID || firstEdge.targetID == secondEdge.targetID)
			{
				failedAttempts++;
				continue;
			}
			
			edgeAtom copyFirstEdge = new edgeAtom(firstEdge.sourceID, firstEdge.targetID);
			edgeAtom copySecondEdge = new edgeAtom(secondEdge.sourceID, secondEdge.targetID);
			
			//Switch the two ends of the two edges.
			firstEdge.targetID = copySecondEdge.targetID;
			secondEdge.targetID = copyFirstEdge.targetID;
			
			boolean successFlag = true;
			
			if(randomMatrix[firstEdge.sourceID][firstEdge.targetID] > 0 || randomMatrix[secondEdge.sourceID][secondEdge.targetID] > 0 
					|| randomMatrix[firstEdge.targetID][firstEdge.sourceID] > 0 || randomMatrix[secondEdge.targetID][secondEdge.sourceID] > 0
					|| firstEdge.sourceID == firstEdge.targetID || secondEdge.sourceID == secondEdge.targetID)
			{
				successFlag = false;
			}
			else
			{
				if(edgeType.compareTo("10") == 0)
				{
					if(adjMatrix[firstEdge.sourceID][firstEdge.targetID] > 0 || adjMatrix[secondEdge.sourceID][secondEdge.targetID] > 0)
					{
						successFlag = false;
					}
				}
				else
				{
					if(adjMatrix[firstEdge.sourceID][firstEdge.targetID] > 0 || adjMatrix[secondEdge.sourceID][secondEdge.targetID] > 0 
							|| adjMatrix[firstEdge.targetID][firstEdge.sourceID] > 0 || adjMatrix[secondEdge.targetID][secondEdge.sourceID] > 0)
					{
						successFlag = false;
					}
				}
				/*else
				{
					System.out.println("No edge types assigned.");
					System.exit(0);
				}*/
			}
			
			if(successFlag == true)
			{
				//Update the random matrix
				updateRandomMatrix(firstEdge, secondEdge, copyFirstEdge, copySecondEdge, edgeType);
				
				if(switchFlag == false)
				{
					if(firstPos > secondPos)
					{
						edgeList.remove(firstPos);
						edgeList.remove(secondPos);
					}
					else
					{
						edgeList.remove(secondPos);
						edgeList.remove(firstPos);
					}
					shuffledEdges.add(firstEdge);
					shuffledEdges.add(secondEdge);
					
					//successAttempt = successAttempt + 2;
				}
				else
				{
					edgeList.remove(firstPos);
					shuffledEdges.add(firstEdge);
					
					//successAttempt++;
				}	
				
				failedAttempts = 0;
			}
			else
			{
				//Switch the edge back;
				firstEdge.targetID = copyFirstEdge.targetID;
				secondEdge.targetID = copySecondEdge.targetID;
				
				failedAttempts++;
			}
		}
		
		System.out.println("Shuffled " + (maxDiff - edgeList.size()) + " number of edges in the current edge list.");
	}
	
	private void dumpRandMatrix() {
		
		
	}

	public void updateRandomMatrix(edgeAtom firstEdge, edgeAtom secondEdge, edgeAtom copyFirstEdge, edgeAtom copySecondEdge, String edgeType)
	{
		//Update the newMatrix
		randomMatrix[firstEdge.sourceID][firstEdge.targetID] = randomMatrix[copyFirstEdge.sourceID][copyFirstEdge.targetID];
		randomMatrix[secondEdge.sourceID][secondEdge.targetID] = randomMatrix[copySecondEdge.sourceID][copySecondEdge.targetID];
		
		randomMatrix[copyFirstEdge.sourceID][copyFirstEdge.targetID] = 0;
		randomMatrix[copySecondEdge.sourceID][copySecondEdge.targetID] = 0;
		
		if(edgeType.compareTo("10") != 0)
		{
			//This is for gene-gene interactions or bidirected regulations;
			randomMatrix[firstEdge.targetID][firstEdge.sourceID] = randomMatrix[copyFirstEdge.targetID][copyFirstEdge.sourceID];
			randomMatrix[secondEdge.targetID][secondEdge.sourceID] = randomMatrix[copySecondEdge.targetID][copySecondEdge.sourceID];
			
			randomMatrix[copyFirstEdge.targetID][copyFirstEdge.sourceID] = 0;
			randomMatrix[copySecondEdge.targetID][copySecondEdge.sourceID] = 0;
		}
	}
	
	public int[][] copyMatrix(int[][] adjMatrix)
	{
		//int[][] newMatrix = adjMatrix.clone();
		int[][] newMatrix = new int[adjMatrix.length][adjMatrix.length];
		for(int i = 0;i < adjMatrix.length;i++)
		{
			for(int j = 0;j < adjMatrix[i].length;j++)
			{
				newMatrix[i][j] = adjMatrix[i][j];
			}
		}
		return newMatrix;
	}
	
	public void validateDifferentEdgeGroups()
	{
		HashMap<String, ArrayList<edgeAtom>> tempEdgeListMap = getDifferentEdgeGroups(randomMatrix);
		edgeListMap = getDifferentEdgeGroups(adjMatrix);
		for(Iterator<String> itKey = tempEdgeListMap.keySet().iterator();itKey.hasNext();)
		{
			String key = itKey.next();
			if(tempEdgeListMap.get(key).size() != edgeListMap.get(key).size())
			{
				System.out.println("The number of edges in different groups are not the same.");
				System.exit(0);
			}
		}
	}
	
	public void validateRandomization()
	{
		//count the number of edges in the original matrix
		for(int i = 0;i < adjMatrix.length;i++)
		{
			int originalEdges = 0;
			int shuffledEdges = 0;
			for(int j = 0;j < adjMatrix[i].length;j++)
			{
				if(adjMatrix[i][j] > 0)
				{
					originalEdges++;
				}
				if(randomMatrix[i][j] > 0)
				{
					shuffledEdges++;
				}
				/*if(adjMatrix[i][j] == 1 && randomMatrix[i][j] == 1)
				{
					System.out.println("There are still same edges in the randomized network as in the original network.");
					System.exit(0);
				}*/
			}
			if(shuffledEdges != originalEdges)
			{
				System.err.println("Validation edge numbers per node is wrong!");
				System.exit(0);
			}
		}		
	}
	
	/** Added by Mitra */
	public void setRandfileName(String randFileName) {
		this.randfilename = randFileName;
	}
	
	/**
	 * @author mitra
	 * @param fileName
	 */
	public void outputRandFile(String fileName)
	{
		try
		{
			//fileName = "output/subgraphCount/" + fileName;
			File newFile = new File(fileName);
			if(newFile.exists())
			{
				newFile.delete();
			}
			FileWriter fos = new FileWriter(newFile);
			for (int i = 0; i < randomMatrix.length; i++) {
				for (int j = 0; j < randomMatrix[i].length; j++) {
					if (randomMatrix[i][j] > 0) {
						String edge = i + "\t" + j + "\n";
						fos.write(edge);
					}
				}
			}
			fos.flush();
			fos.close();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
}
