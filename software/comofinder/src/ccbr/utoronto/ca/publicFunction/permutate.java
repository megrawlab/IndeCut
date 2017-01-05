package ccbr.utoronto.ca.publicFunction;

import java.util.ArrayList;

public class permutate {
	
	//public static int total = 0;
	
	ArrayList<int[]> fullCombination;
	
	int[] subgraph;
	
	public permutate(ArrayList<Integer> subgraph)
	{
		fullCombination = new ArrayList<int[]>();
		this.subgraph = new int[subgraph.size()];
		for(int i = 0;i < subgraph.size();i++)
		{
			this.subgraph[i] = subgraph.get(i);
		}
	}
	
	public permutate(int[] subgraph)
	{
		fullCombination = new ArrayList<int[]>();
		this.subgraph = subgraph;
	}
	
	public void swap(int[] str, int i, int j)
	{
		int temp = -1;
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
	}
	
	public void arrange(int[] str, int st, int len)
	{
		if (st == len - 1)
		{
			int[] com = new int[str.length];
			for (int i = 0; i < len; i ++)
			{
				//System.out.print(str[i]+ "  ");
				com[i] = str[i];
			}
			//System.out.println();
			fullCombination.add(com);
		}
		else
		{
			for (int i = st; i < len; i ++)
			{
				swap(str, st, i);
				arrange(str, st + 1, len);
				swap(str, st, i);
			}
		}		
	}
	
	public ArrayList<int[]> getFullCombination()
	{
		arrange(subgraph, 0, subgraph.length);
		
		return fullCombination;
	}
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		 int str[] = {4};
		 
		 permutate pm = new permutate(str);
		 pm.arrange(str, 0, str.length);
		 
		 System.out.println("Shall we test here?");
	}
}
