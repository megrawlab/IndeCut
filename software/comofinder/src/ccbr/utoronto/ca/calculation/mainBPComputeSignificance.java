package ccbr.utoronto.ca.calculation;

import ccbr.utoronto.ca.publicFunction.analyzeParameters;

public class mainBPComputeSignificance {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		
		int motifSize = 3;//default	
		String batchFileDirStr = null;
		String subgraphCountFileName = null;
		String method = null;
		String outputdir = null;
		String motifFileName = null;
		
		analyzeParameters ap = new analyzeParameters(args);
		if(ap.hasOption("ms") == true)
		{
			motifSize = Integer.valueOf(ap.getValue("ms"));
		}
		if(ap.hasOption("bfd") == true)
		{
			batchFileDirStr = ap.getValue("bfd");
		}
		if(ap.hasOption("method") == true)
		{
			method = ap.getValue("method");
		}		
		if(ap.hasOption("opd") == true)
		{
			outputdir = ap.getValue("opd");
		}		
		if(ap.hasOption("scn") == true)
		{
			subgraphCountFileName = ap.getValue("scn");
		}
		if(ap.hasOption("mfn") == true)
		{
			motifFileName = ap.getValue("mfn");
		}
		
		System.out.println("Subgraph count file name: " + subgraphCountFileName + "; batch files directory: " + batchFileDirStr + "; motif file name: " + motifFileName);
		
		BPComputeSignificance bpcs = new BPComputeSignificance();
		bpcs.readReferenceCount(subgraphCountFileName);
		bpcs.analyzeBatchFiles(batchFileDirStr);
		bpcs.runStatisticalAnalysis();
		bpcs.outputMotifs(motifFileName);

	}

}
