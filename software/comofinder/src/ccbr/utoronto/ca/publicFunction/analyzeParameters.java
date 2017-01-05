package ccbr.utoronto.ca.publicFunction;

import java.util.HashMap;

public class analyzeParameters {

	String[] args;
	HashMap<String, String> paraMap;
	
	public analyzeParameters(String[] args)
	{
		this.args = args;
		paraMap = new HashMap<String, String>();
		
		for(int i = 0;i < args.length;i = i + 2)
		{
			String currentPara = args[i];
			//String[] test = currentPara.split("-");
			currentPara = currentPara.substring(1, currentPara.length());
			String currentValue = args[i + 1];
			
			paraMap.put(currentPara, currentValue);
		}
	}
	
	public boolean hasOption(String currentPara)
	{
		boolean paraFlag = true;
		if(!paraMap.containsKey(currentPara))
		{
			paraFlag = false;
		}
		return paraFlag;
	}

	public String getValue(String currentPara)
	{
		return paraMap.get(currentPara);
	}
	
}
