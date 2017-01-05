package ccbr.utoronto.ca.publicFunction;

import java.util.ArrayList;

import ccbr.utoronto.ca.dataStructure.comNodeAtom;

public class output {

	public static void outputInteractionNodeNames(int nodeID, ArrayList<comNodeAtom> nodeList)
	{
		comNodeAtom node = nodeList.get(nodeID);
		System.out.println("The current node is " + node.nodeName + ". The interaction list is: ");
		for(int i = 0;i < node.nodeInteraction.size();i++)
		{
			System.out.println(nodeList.get(node.nodeInteraction.get(i)).nodeName);
		}
	}
	
	public static void outputInteractionNodeNames(String nodeName, ArrayList<comNodeAtom> nodeList)
	{
		for(int j = 0;j < nodeList.size();j++)
		{
			if(nodeList.get(j).nodeName.compareToIgnoreCase(nodeName) == 0)
			{
				comNodeAtom node = nodeList.get(j);
				System.out.println("The current node is " + nodeName + ". The interaction list is: ");
				for(int i = 0;i < node.nodeInteraction.size();i++)
				{
					System.out.println(nodeList.get(node.nodeInteraction.get(i)).nodeName);
				}
			}
		}		
	}
	
	public static void outputArrayList(ArrayList<Integer> list)
	{
		for(int i = 0;i < list.size();i++)
		{
			System.out.print(list.get(i) + ", ");
		}
		System.out.println();
	}
	
	public static void outputArrayList(int[] array)
	{
		for(int i = 0;i < array.length;i++)
		{
			System.out.print(array[i] + ", ");
		}
		System.out.println();
	}
	
}
