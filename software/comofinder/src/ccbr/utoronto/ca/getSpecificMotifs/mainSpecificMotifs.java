package ccbr.utoronto.ca.getSpecificMotifs;

import ccbr.utoronto.ca.publicFunction.analyzeParameters;
import ccbr.utoronto.ca.readData.readStandardInput;

public class mainSpecificMotifs {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub

		//String motifFileName = args[0];
		String motifFileName = "motifs_4.txt";
		String inputFilePath = null;
		//String nodeIDsandNames = "encode_nodeIDsandNames.txt";
		String outputFileDir = "output/subgraphList/";
		
		analyzeParameters ap = new analyzeParameters(args);
		if(ap.hasOption("fp") == true)
		{
			inputFilePath = ap.getValue("fp");
		}
		if(ap.hasOption("opd") == true)
		{
			outputFileDir = ap.getValue("opd");
		}
		if(ap.hasOption("list") == true)
		{
			motifFileName = ap.getValue("list");
		}
		
		readStandardInput rzd = new readStandardInput(inputFilePath);
		//rzd.setNodeNames(nodeIDsandNames);
		
		enumerateMultiMotifsParallel emp = new enumerateMultiMotifsParallel(rzd.getAdjMatrix(),
				rzd.getNodeList(), rzd.getNodeTypeMap());
		
		emp.readSpecificMotif(motifFileName);
		emp.setOutputFileDir(outputFileDir);
		//emp.setMergeMotifs(true);
		emp.run();
		
	}
}
