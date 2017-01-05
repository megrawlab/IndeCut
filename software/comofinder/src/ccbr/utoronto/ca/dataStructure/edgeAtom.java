package ccbr.utoronto.ca.dataStructure;

public class edgeAtom
{
	public int sourceID;
	public int targetID;
	
	public int edgeType = -1;//0: undirected or bidirected; 1: directed; -1: unassigned.
	
	public double samplingWeight = 0;
	
	public String sourceName;
	public String targetName;
	
	public edgeAtom(int source, int target)
	{
		this.sourceID = source;
		this.targetID = target;
	}
	
	public edgeAtom(String sourceName, String targetName)
	{
		this.sourceName = sourceName;
		this.targetName = targetName;
	}
}
