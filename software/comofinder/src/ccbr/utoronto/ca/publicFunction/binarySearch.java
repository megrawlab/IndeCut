package ccbr.utoronto.ca.publicFunction;

import java.util.ArrayList;

import ccbr.utoronto.ca.dataStructure.comNodeAtom;

public class binarySearch {

	//If array doesn't contain the node, it will insert the node into the array and return false;
	//Otherwise it will return true;
	public static boolean sort(ArrayList<Integer> array, int node)
	{
		int low = 0;
        int high = array.size() - 1;

        //left pointer is the final position for the node to be inserted.
        while(low <= high) 
        {
            int mid = low + (high - low) / 2;
            //int mid = Math.floor((high + low) / 2);
            if(node < array.get(mid))
            {
            	high = mid - 1;
            }
            else if(node > array.get(mid))
            {
            	low = mid + 1;
            }
            else
            {
            	return true;
            }
        }
        
        array.add(low, node);
        
        return false;
	}
	
	public static boolean contain(ArrayList<Integer> array, int node)
	{
		int low = 0;
        int high = array.size() - 1;

        //left pointer is the final position for the node to be inserted.
        while(low <= high) 
        {
            int mid = low + (high - low) / 2;
            //int mid = Math.floor((high + low) / 2);
            if(node < array.get(mid))
            {
            	high = mid - 1;
            }
            else if(node > array.get(mid))
            {
            	low = mid + 1;
            }
            else
            {
            	return true;
            }
        }
        
        return false;
	}
	
	public static boolean contain(ArrayList<String> array, String node)
	{
		int low = 0;
        int high = array.size() - 1;

        //left pointer is the final position for the node to be inserted.
        while(low <= high) 
        {
            int mid = low + (high - low) / 2;
            //int mid = Math.floor((high + low) / 2);
            //int comparison = node.compareToIgnoreCase(array.get(mid));
            int comparison = node.compareTo(array.get(mid));
            if(comparison < 0)
            {
            	high = mid - 1;
            }
            else if(comparison > 0)
            {
            	low = mid + 1;
            }
            else
            {
            	return true;
            }
        }
        
        return false;
	}
	
	public static int containPosition(ArrayList<Integer> array, int node)
	{
		int low = 0;
        int high = array.size() - 1;

        //left pointer is the final position for the node to be inserted.
        while(low <= high) 
        {
            int mid = low + (high - low) / 2;
            //int mid = Math.floor((high + low) / 2);
            if(node < array.get(mid))
            {
            	high = mid - 1;
            }
            else if(node > array.get(mid))
            {
            	low = mid + 1;
            }
            else
            {
            	return mid;
            }
        }
        
        return -1;
	}
	
	public static boolean remove(ArrayList<Integer> array, int node)
	{
		int low = 0;
        int high = array.size() - 1;

        //left pointer is the final position for the node to be inserted.
        while(low <= high) 
        {
            int mid = low + (high - low) / 2;
            //int mid = Math.floor((high + low) / 2);
            if(node < array.get(mid))
            {
            	high = mid - 1;
            }
            else if(node > array.get(mid))
            {
            	low = mid + 1;
            }
            else
            {
            	array.remove(mid);
            	return true;
            }
        }
        
        return false;
	}
	
	public static boolean remove(ArrayList<String> array, String node)
	{
		int low = 0;
        int high = array.size() - 1;

        //left pointer is the final position for the node to be inserted.
        while(low <= high) 
        {
            int mid = low + (high - low) / 2;
            //int mid = Math.floor((high + low) / 2);
            int comparison = node.compareTo(array.get(mid));
            if(comparison < 0)
            {
            	high = mid - 1;
            }
            else if(comparison > 0)
            {
            	low = mid + 1;
            }
            else
            {
            	array.remove(mid);
            	return true;
            }
        }
        
        return false;
	}
	
	public static boolean sort(ArrayList<String> array, String node)
	{
		int low = 0;
        int high = array.size() - 1;

        //left pointer is the final position for the node to be inserted.
        while(low <= high) 
        {
            int mid = low + (high - low) / 2;
            //int mid = Math.floor((high + low) / 2);
            //int comparison = node.compareToIgnoreCase(array.get(mid));
            int comparison = node.compareTo(array.get(mid));
            if(comparison < 0)
            {
            	high = mid - 1;
            }
            else if(comparison > 0)
            {
            	low = mid + 1;
            }
            else
            {
            	return true;
            }
        }
        
        array.add(low, node);
        
        return false;
	}
	
	//We sort the nodes in a decreasing order according to their degrees.
	public static boolean sortAccordingtoDegrees(ArrayList<comNodeAtom> array, comNodeAtom node)
	{
		int low = 0;
        int high = array.size() - 1;

        //left pointer is the final position for the node to be inserted.
        while(low <= high) 
        {
            int mid = low + (high - low) / 2;
            //int mid = Math.floor((high + low) / 2);
            if(node.degree > array.get(mid).degree)
            {
            	high = mid - 1;
            }
            else if(node.degree < array.get(mid).degree)
            {
            	low = mid + 1;
            }
            else
            {
            	low = mid;
            	break;
            }
        }
        
        array.add(low, node);
        
        return false;
	}
	
	public static boolean sortAccordingtoSamplingWeights(ArrayList<comNodeAtom> array, comNodeAtom node)
	{
		int low = 0;
        int high = array.size() - 1;

        //left pointer is the final position for the node to be inserted.
        while(low <= high) 
        {
            int mid = low + (high - low) / 2;
            //int mid = Math.floor((high + low) / 2);
            if(node.samplingWeight > array.get(mid).samplingWeight)
            {
            	high = mid - 1;
            }
            else if(node.samplingWeight < array.get(mid).samplingWeight)
            {
            	low = mid + 1;
            }
            else
            {
            	return true;
            }
        }
        
        array.add(low, node);
        
        return false;
	}
}

