package ccbr.utoronto.ca.publicFunction;

import java.util.ArrayList;

public class arrange {

	ArrayList<ArrayList<int[]>> combination;
	ArrayList<ArrayList<Integer>> enumeration;
	
	public arrange(ArrayList<ArrayList<int[]>> combination)
	{
		this.combination = combination;
		enumeration = new ArrayList<ArrayList<Integer>>();
	}
	
	public void  sequentialSelect()
	{
		ArrayList<Integer> altogether = new ArrayList<Integer>();
		iterative(0, combination, altogether);
		//outputEnumeration();
	}
	
	public void iterative(int pointer, ArrayList<ArrayList<int[]>> combination, ArrayList<Integer> altogether)
	{
		if(pointer == combination.size())
		{
			enumeration.add(altogether);
			return;
		}
		ArrayList<int[]> subset = combination.get(pointer);
		for(int j = 0;j < subset.size();j++)
		{						
			ArrayList<Integer> copy = addValue(subset.get(j), altogether);
			
			iterative(pointer + 1, combination, copy);
		}
	}
	
	public ArrayList<Integer> addValue(int[] subset, ArrayList<Integer> altogether)
	{
		ArrayList<Integer> copy = new ArrayList<Integer>();
		copy.addAll(altogether);
		for(int i = 0;i < subset.length;i++)
		{
			//System.out.print(subset[i] + " ");
			copy.add(subset[i]);
		}
		return copy;
	}
	
	public ArrayList<ArrayList<Integer>> getEnumeration()
	{
		sequentialSelect();
		return enumeration;
	}
	
	public void outputEnumeration()
	{
		for(int i = 0;i < enumeration.size();i++)
		{
			for(int j = 0;j < enumeration.get(i).size();j++)
			{
				System.out.print(enumeration.get(i).get(j) + ",");
			}
			System.out.println();
		}
	}
	
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		int[] a = {1};
		//int[] aa = {2, 1};
		int[] b = {3, 4};
		int[] bb = {4, 3};
		int[] d = {5,6,7};
		int[] dd = {5,7,6};
		int[] ddd = {7,6,5};
		
		ArrayList<int[]> aaa = new ArrayList<int[]>();
		aaa.add(a);
		//aaa.add(aa);
		
		ArrayList<int[]> bbb = new ArrayList<int[]>();
		bbb.add(b);
		bbb.add(bb);
		
		ArrayList<int[]> dddd = new ArrayList<int[]>();
		dddd.add(d);
		dddd.add(dd);
		dddd.add(ddd);
		
		ArrayList<ArrayList<int[]>> ccc = new ArrayList<ArrayList<int[]>>();
		ccc.add(aaa);
		ccc.add(bbb);
		ccc.add(dddd);
		
		arrange ar = new arrange(ccc);
		ar.sequentialSelect();
	}

}
