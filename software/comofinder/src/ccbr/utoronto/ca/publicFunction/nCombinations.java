package ccbr.utoronto.ca.publicFunction;

import java.util.ArrayList;

public class nCombinations {

	int combinationSize;
	int elementsNum;
	ArrayList<int[]> combinationList;
	
	public nCombinations(int combinationSize, int elementsNum)
	{
		this.combinationSize = combinationSize;
		this.elementsNum = elementsNum;
		combinationList = new ArrayList<int[]>();
	}
	
	public void run()
	{
		//long startTime = System.currentTimeMillis();
		for(int i = 0;i < combinationSize;i++)
		{
			int startNode = i;
			int[] curCom = new int[elementsNum];
			curCom[0] = startNode;
			//Here 0 means current level of the array
			extend(curCom, startNode, 0);
		}
		//long endTime = System.currentTimeMillis();
		//System.out.println("The run time is : " + (int)((endTime - startTime)/1000) + " seconds");
	}
	
	public void extend(int[] curCom, int startNode, int currentLevel)
	{
		if(currentLevel == (elementsNum - 1))
		{
			combinationList.add(curCom);
			return;
		}
		else
		{
			currentLevel = currentLevel + 1;
			for(int i = startNode + 1;i < combinationSize;i++)
			{
				int[] newCom = duplicateArray(curCom);
				newCom[currentLevel] = i;
				extend(newCom, i, currentLevel);
			}
		}
	}
	
	public int[] duplicateArray(int[] originalList)
	{
		int[] newArrayList = new int[elementsNum];
		for(int i = 0;i < originalList.length;i++)
		{
			newArrayList[i] = originalList[i];
		}
		return newArrayList;
	}
	
	public ArrayList<int[]> getCombinationList()
	{
		return combinationList;
	}
	
	public static void main(String[] args)
	{
		nCombinations nc = new nCombinations(10, 2);
		nc.run();	
		nc.getCombinationList();
	}
	
}
