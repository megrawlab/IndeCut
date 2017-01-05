package ccbr.utoronto.ca.dataStructure;

import java.util.ArrayList;

/* "comNode" means composite node.*/

public class comNodeAtom {

	public String nodeName;
	public int nodeID;//the node index
	public int inDegree;
	public int outDegree;
	//public double nodeCoEff;//the cluster coefficient of the node
	//public int nodeBetweenness;
	
	public int actualID;//This variant stores the nodeID before filtering
	
	public int degree;//node degree
	
	public ArrayList<Integer> nodeInteraction;//this is for regulations;
	
	public ArrayList<Integer> geneInteraction;//this is specifically for gene interactions;
	
	public int nodeType = -1;//0: microRNA; 1: transcription factor; 2: target genes
	public int edgeType = -1;//0: no edges; 1: regulations; 2: interactions; 3: regulations plus interactions;

	public ArrayList<comNodeAtom> nodeAtomRegulation;
	public ArrayList<comNodeAtom> nodeAtomBeingRegulated;
	
	//The average degree of the neighbors of the current node.
	//public double averageNeighDegree;
	
	public double nodeWeight;//Means the weight of the node
	
	//The following two members are for the WaRSwap randomization algorithm.
	public int capacity;
	public double samplingWeight;
	
	public String familyID = null;	
	
}
