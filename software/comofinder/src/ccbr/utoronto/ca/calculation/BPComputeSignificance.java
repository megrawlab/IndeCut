package ccbr.utoronto.ca.calculation;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;

import ccbr.utoronto.ca.publicFunction.binarySearch;
import ccbr.utoronto.ca.publicFunction.fileOperations;

/* From wikipedia:network motif
 * The number of appearances of graph G' in G is called the frequency FG of G' in G. */

public class BPComputeSignificance {

	String batchFileDir;
	
	int motifSize;
	int ensembleSize;
	double[] pValue;
	double[] ZScore;
	double[][] Frequency;
	double occupancy;
	double[] meanFrequency;
	double[] standardDeviation;
	HashMap<String, Integer> countSubgraph = new HashMap<String, Integer>();
	
	HashMap<String, Integer> labelIndex = new HashMap<String, Integer>();
	HashMap<Integer, String> labelIndexReverse = new HashMap<Integer, String>();
	
	//boolean frequencyFlag = false;
	
	public void setCountSubgraph(HashMap<String, Integer> countSubgraph)
	{
		this.countSubgraph = countSubgraph;
	}
	
	public void readReferenceCount(String fileName)
	{
		try
		{
			BufferedReader br = new BufferedReader(new FileReader(fileName));
			String temp = null;
			while((temp = br.readLine()) != null)
			{
				String[] countInfo = temp.split("\t");
				countSubgraph.put(countInfo[0], Integer.valueOf(countInfo[1]));
			}
			br.close();
			
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public void initialization()
	{
		pValue = new double[countSubgraph.keySet().size()];
		ZScore = new double[countSubgraph.keySet().size()];
		Frequency = new double[ensembleSize][countSubgraph.keySet().size()];
		occupancy = 0;
		
		//Need to establish the relationship between certain subgraph ID and the pointer;
		int pointer = 0;	
		labelIndex.clear();
		labelIndexReverse.clear();

		for(Iterator<String> itKey = countSubgraph.keySet().iterator();itKey.hasNext();)
		{
			String tempKey = itKey.next();
			labelIndex.put(tempKey, pointer);
			labelIndexReverse.put(pointer, tempKey);
			pointer = pointer + 1;
		}
	}
	
	public void analyzeBatchFiles(String batchFileDirStr)
	{
		try
		{
			File batchFileDir = new File(batchFileDirStr);
			if(batchFileDir.isDirectory() == false)
			{
				System.err.println("The batchFileDir is wrong.");
				System.exit(0);
			}
			File[] batchFiles = batchFileDir.listFiles();

			ensembleSize = batchFiles.length;
			System.out.println("The ensembleSize is : " + ensembleSize);
			initialization();
			
			for(int i = 0;i < batchFiles.length;i++)
			{
				HashMap<String, Integer> batchCountSubgraph = new HashMap<String, Integer>();
				BufferedReader br = new BufferedReader(new FileReader(batchFiles[i]));
				String temp = null;
				while((temp = br.readLine()) != null)
				{
					String[] countInfo = temp.split("\t");
					batchCountSubgraph.put(countInfo[0], Integer.valueOf(countInfo[1]));
				}
				
				for(Iterator<String> itKey = countSubgraph.keySet().iterator();itKey.hasNext();)
				{
					String tempSubgraphCanLabel = itKey.next();
					if(batchCountSubgraph.containsKey(tempSubgraphCanLabel))
					{
						//Frequency[i][pointer] = currentNet.countSubgraph.get(tempSubgraphCanLabel);
						Frequency[i][labelIndex.get(tempSubgraphCanLabel)] = batchCountSubgraph.get(tempSubgraphCanLabel);
					}
					else
					{
						System.err.println("Did not find " + tempSubgraphCanLabel + " in the " + i + "th randomized network.");
					}
				}
				br.close();
			}
			
			System.out.println("Finish reading all the batch files.");		
			
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public void runStatisticalAnalysis()
	{
		//Set<String> degKeySet = countSubgraph.keySet();
		for(Iterator<String> itKey = countSubgraph.keySet().iterator();itKey.hasNext();)
		{
			String tempSubgraphCanLabel = itKey.next();
			occupancy = occupancy + countSubgraph.get(tempSubgraphCanLabel);
		}

		//double[] amountInEachRanNet = new double[ensembleSize];
		meanFrequency = new double[ZScore.length];
		for(int i = 0;i < Frequency.length;i++)
		{
			for(int j = 0;j < Frequency[i].length;j++)
			{				
				meanFrequency[j] += Frequency[i][j];
				
				if(Frequency[i][j] >= Double.valueOf(countSubgraph.get(labelIndexReverse.get(j))))
				{
					pValue[j] = pValue[j] + 1;
				}
			}
			
		}
		
		standardDeviation = new double[ZScore.length];
		for(int j = 0;j < Frequency[0].length;j++)
		{
			//---------compute mean frequency----------
			meanFrequency[j] = meanFrequency[j] / Double.valueOf(ensembleSize);
			//---------compute mean frequency end------
			
			//---------compute the ZScore----------
			double Np = Double.valueOf(countSubgraph.get(labelIndexReverse.get(j)));

			for(int i = 0;i < Frequency.length;i++)
			{
				//standardDeviation[j] += (Double.valueOf(Frequency[i][j] / amountInEachRanNet[i]) - meanFrequency[j]) * (Double.valueOf(Frequency[i][j] / amountInEachRanNet[i]) - meanFrequency[j]);
				standardDeviation[j] += (Double.valueOf(Frequency[i][j]) - meanFrequency[j]) * (Double.valueOf(Frequency[i][j]) - meanFrequency[j]);
			}
			standardDeviation[j] = Math.sqrt(standardDeviation[j] / (Frequency.length - 1)); 

			ZScore[j] = (Np - meanFrequency[j]) / standardDeviation[j];
			//---------compute ZScore end------------
			
			//---------compute pValue in a more reliable way-------------
			pValue[j] = ( pValue[j] + 1) / Double.valueOf(ensembleSize + 1);
			//---------compute pValue end-----------
		}
		
		System.out.println("Finish computing the significance.");
	}
	
	public void countOccurrencesofMotifs(String outputDir)
	{
		fileOperations.createDirectorys(outputDir);
		for(Iterator<String> itKey = countSubgraph.keySet().iterator();itKey.hasNext();)
		{
			String tempSubgraphCanLabel = itKey.next();
			HashMap<Integer, Integer> occurrenceDistribution = new HashMap<Integer, Integer>();
			ArrayList<Integer> countOrder = new ArrayList<Integer>();
			int subGraphIndex = labelIndex.get(tempSubgraphCanLabel);
			for(int i = 0;i < Frequency.length;i++)
			{
				int curCount = (int)Frequency[i][subGraphIndex];
				if(occurrenceDistribution.containsKey(curCount))
				{
					int en = occurrenceDistribution.get(curCount);
					en++;
					occurrenceDistribution.put(curCount, en);
				}
				else
				{
					occurrenceDistribution.put(curCount, 1);
					binarySearch.sort(countOrder, curCount);
				}
			}
			outputOccurrenceDistributiontoFile(outputDir + "/" + tempSubgraphCanLabel, countOrder, occurrenceDistribution);
		}
	}
	
	public void outputOccurrenceDistributiontoFile(String fileName, ArrayList<Integer> countOrder, HashMap<Integer, Integer> distribution)
	{
		try
		{
			File writeFile = new File(fileName);
			if(writeFile.exists())
			{
				writeFile.delete();
			}
			//FileOutputStream fos = new FileOutputStream(fileName);
			FileOutputStream fos = new FileOutputStream(writeFile);
			for(int i = 0;i < countOrder.size();i++)
			{
				fos.write(String.valueOf(countOrder.get(i)).getBytes());
				fos.write("\t".getBytes());
				fos.write(String.valueOf(distribution.get(countOrder.get(i))).getBytes());
				fos.write("\n".getBytes());
			}
			fos.close();
		}
		catch(IOException e)
		{
			System.err.println("error outputing!");
		}
	}
	
	public void outputMotifs(String fileName)
	{
		try
		{
			File writeFile = new File(fileName);
			if(writeFile.exists())
			{
				writeFile.delete();
			}
			FileOutputStream fos = new FileOutputStream(writeFile);
			Set<String> degKeySet = countSubgraph.keySet();
			
			fos.write("Subgraph_Label".getBytes());
			fos.write("\t".getBytes());
			fos.write("occupancy".getBytes());
			fos.write("\t".getBytes());
			fos.write("mean_frequency".getBytes());
			fos.write("\t".getBytes());
			fos.write("standard_deviation".getBytes());
			fos.write("\t".getBytes());
			fos.write("ZScore".getBytes());
			fos.write("\t".getBytes());
			fos.write("pValue".getBytes());
			fos.write("\t".getBytes());
			fos.write("frequency_in_real_network".getBytes());
			fos.write("\n".getBytes());
			
			for(Iterator<String> itKey = degKeySet.iterator();itKey.hasNext();)
			{
				String tempSubgraphCanLabel = itKey.next();
				int pointer = labelIndex.get(tempSubgraphCanLabel);
				fos.write(tempSubgraphCanLabel.getBytes());
				fos.write("\t".getBytes());
				fos.write(String.valueOf(Double.valueOf(countSubgraph.get(tempSubgraphCanLabel)) / occupancy).getBytes());
				fos.write("\t".getBytes());
				fos.write(String.valueOf(meanFrequency[pointer]).getBytes());
				fos.write("\t".getBytes());
				fos.write(String.valueOf(standardDeviation[pointer]).getBytes());
				fos.write("\t".getBytes());
				fos.write(String.valueOf(ZScore[pointer]).getBytes());
				fos.write("\t".getBytes());
				fos.write(String.valueOf(pValue[pointer]).getBytes());
				fos.write("\t".getBytes());
				fos.write(String.valueOf(countSubgraph.get(tempSubgraphCanLabel)).getBytes());
				fos.write("\n".getBytes());
				pointer = pointer + 1;
			}
			fos.close();
		}
		catch(IOException e)
		{
			System.err.println("error outputing!");
			e.printStackTrace();
		}
	}
}
