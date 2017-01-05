package ccbr.utoronto.ca.publicFunction;

import java.io.File;

public class fileOperations {

	static boolean flag;
	static File file;
    
    public static boolean DeleteFolder(String sPath) 
    {
        flag = false;
        file = new File(sPath);
        if (!file.exists())
        {
            return flag;
        }
        else 
        {
            if (file.isFile()) 
            {
                return deleteFile(sPath);
            }
            else 
            {
                return deleteDirectory(sPath);
            }
        }
    }
    
    public static boolean deleteFile(String sPath) 
    {
        flag = false;
        file = new File(sPath);
        if (file.isFile() && file.exists()) 
        {
            file.delete();
            flag = true;
        }
        return flag;
    }
    
    public static boolean deleteDirectory(String sPath) 
    {
        if (!sPath.endsWith(File.separator)) 
        {
            sPath = sPath + File.separator;
        }
        File dirFile = new File(sPath);

        if (!dirFile.exists() || !dirFile.isDirectory()) 
        {
            return false;
        }
        flag = true;

        File[] files = dirFile.listFiles();
        for (int i = 0; i < files.length; i++) 
        {
            if (files[i].isFile()) 
            {
                flag = deleteFile(files[i].getAbsolutePath());
                if (!flag) break;
            }
            else 
            {
                flag = deleteDirectory(files[i].getAbsolutePath());
                if (!flag) break;
            }
        }
        if (!flag) return false;

        if (dirFile.delete()) 
        {
            return true;
        }
        else 
        {
            return false;
        }
    }
    
    public static boolean cleanDirectory(String sPath) 
    {
        if (!sPath.endsWith(File.separator)) 
        {
            sPath = sPath + File.separator;
        }
        File dirFile = new File(sPath);

        if (!dirFile.exists() || !dirFile.isDirectory()) 
        {
            return false;
        }
        flag = true;

        File[] files = dirFile.listFiles();
        for (int i = 0; i < files.length; i++)
        {
            if (files[i].isFile())
            {
                flag = deleteFile(files[i].getAbsolutePath());
                if (!flag) break;
            }
            else 
            {
                flag = deleteDirectory(files[i].getAbsolutePath());
                if (!flag) break;
            }
        }
        if (!flag) return false;
       
        return true;
    }
	
    public static boolean createDirectorys(String sPath)
    {
    	try
    	{
    		File dirFile = new File(sPath);
    		if(dirFile.isDirectory())
    		{
    			return true;
    		}
    		else
    		{
    			return dirFile.mkdirs();
    		}
    		
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}
    	
		return true;
    }
}
