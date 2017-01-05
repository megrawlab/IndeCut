package ccbr.utoronto.ca.comoFinderAlgorithm;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import ccbr.utoronto.ca.publicFunction.getCanonicalLabel;
import jsr166y.RecursiveTask;

public class removeIsomorphismParallelTask extends RecursiveTask{
	
	public static int SEQUENTIAL_THRESHOLD;
	
	int start;
	int end;
	
	HashMap<String, Integer> countSubgraph;
	HashMap<String, Integer> newCountSubgraph;
	ArrayList<String> labelList;
	
	public removeIsomorphismParallelTask(HashMap<String, Integer> countSubgraph, ArrayList<String> labelList, int start, int end)
	{
		this.countSubgraph = countSubgraph;
		this.labelList = labelList;
		this.start = start;
		this.end = end;
		
		newCountSubgraph = new HashMap<String, Integer>();
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
				final removeIsomorphismParallelTask left = new removeIsomorphismParallelTask(countSubgraph,	labelList, start, start + split);
				final removeIsomorphismParallelTask right = new removeIsomorphismParallelTask(countSubgraph, labelList, start + split, end);
				invokeAll(left, right);
				//subGraphNum = (Integer)left.get() + (Integer)right.get();
				HashMap<String, Integer> leftCountSubgraph = (HashMap<String, Integer>)left.get();
				HashMap<String, Integer> rightCountSubgraph = (HashMap<String, Integer>)right.get();
				addCountSubgraphResults(leftCountSubgraph, rightCountSubgraph);
			}
			catch(Exception e)
			{
				e.printStackTrace();
			}
		}
		return newCountSubgraph;
	}
	
	public void computeDirectly()
	{
		long startTime = new java.util.Date().getTime();
		for(int i = start;i < end;i++)
		{
			String curLabel = labelList.get(i);
			int count = countSubgraph.get(curLabel);
			
			getCanonicalLabel gcl = new getCanonicalLabel(curLabel);
			String canonicalLabel = gcl.canonicalLabel();
			
			if(newCountSubgraph.containsKey(canonicalLabel))
			{
				int newCount = newCountSubgraph.get(canonicalLabel);
				count = count + newCount;			
			}
			
			newCountSubgraph.put(canonicalLabel, count);
		}
		Double secs = new Double((new java.util.Date().getTime() - startTime) * 0.001);
		System.out.println("Removing isomorphism in this thread took " + secs + " secs.");
	}
	
	public void addCountSubgraphResults(HashMap<String, Integer> leftCountSubgraph, HashMap<String, Integer> rightCountSubgraph)
	{
		System.out.println("The size of rightCountSubgraph is : " + rightCountSubgraph.size());
		System.out.println("The size of leftCountSubgraph is : " + leftCountSubgraph.size());
		long startTime = new java.util.Date().getTime();
		
		newCountSubgraph.putAll(leftCountSubgraph);
		for(Iterator<String> rightKey = rightCountSubgraph.keySet().iterator();rightKey.hasNext();)
		{
			String rightCanonicalLabel = rightKey.next();
			int rightCount = rightCountSubgraph.get(rightCanonicalLabel);
			
			if(newCountSubgraph.containsKey(rightCanonicalLabel))
			{
				int newCount = newCountSubgraph.get(rightCanonicalLabel);
				rightCount = rightCount + newCount;
			}
			
			newCountSubgraph.put(rightCanonicalLabel, rightCount);
		}
		
		Double secs = new Double((new java.util.Date().getTime() - startTime) * 0.001);
		System.out.println("Adding the compositeMotiftogether took " + secs + " secs.");
		//return newCountSubgraph;
	}
}
