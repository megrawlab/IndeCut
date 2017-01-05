# IndeCUT
A cut norm based method to evaluate uniform and independent sampling in network motif discovery algorithms.

## Requirements
+ Linux operating system
+ Java 1.6+
+ 4GB of RAM minimum
+ [R](www.r-project.org) core libraries

### Network motif discovery tools
Four network motif discover tools along with their source codes are provided under `software` folder. 

1. FANMOD
  This software package is located under `software/fanmod` folder. The `source-code` folder contains required `C` and `C++` libraries and source codes to make an executable `FANMOD` tool (`makefile` is located at `software/fanmod/source-code`).
  
2. DIA-MCIS
  This software package is located under `software/diamcis` folder. The `source-code` folder contains required `C` libraries and source codes to make an executable `DIA-MCIS` tool (`makefile` is located at `software/diamcis/source-code`).
  
3. CoMoFinder
  This software package is located under `software/comofinder` folder. This software is written in `Java` and required libraries to compile and make an executable `jar` file is located at `software/comofinder/`.
  
4. WaRSwap
  This software package is located under `software/warswap` folder. This software package is written in `R` and doesn't need any compilation.

## Installing and Running IndeCut
1. Download/clone this repository into your working directory. 
2. Open a command-line terminal
3. Change directory to the `software` folder as follows:
  ```bash
  cd software
  ```
  
4. Open the file `makefile` and perform the following changes (if required):
  1. set PYTHON variable to the path that Python is installed on your computer.
    ```bash
    # Example
    PYTHON=/usr/bin/python
    ```
    
  2. The input graph is a bipartite graph that is defined by a name, source out-degree and target in-degree sequences (comma separated sequence). 
  ```bash
  # Default settings
  graphname = graph1
  src_degdist = 6,1,1,1,1,1,1,6,1,1,1,1,1,1
  target_degdist = 6,1,1,1,1,1,1,6,1,1,1,1,1,1
  ngraphs = 5000
  ```
  
    You can change the above configurations as follows: 
    1. change `graphname` to any arbitrary name. The name can contain characters and numbers. 
    2. change `src_degdist` to a new source out-degree sequence. 
    3. change `target_degdist` to a new target in-degree sequence.
    4. change number of samples by mofiying the `ngraphs` variable.

5. Running IndeCut:
  ```bash
  make all
  ```
  The `make all` command starts compiling all the network motif discovery tools and then running each tool to generate the samples. It then computes average matrices `A` for each network motif discovery samples and computes maximum entropy matrix `Z`. At the end, it computes cut norm extimates for each algorithm and records the results into `results` folder for each graph separately.

7. The output file will be saved into `results` directory (`results/$graphname.cutnorms.txt`).

## License
Please see the LICENSE file for copyright and distribution rights.

