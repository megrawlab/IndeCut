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
    
  2. Specify a name for graph/network, enter the path to the file containing degree sequence of input bipartite graph as follows:
  ```bash
  # graphname (dronet example as default)
  graphname = dronet_miR-TF 

  # Path to a file that has two lines: 
  # 1st line: comma separated "out-degrees" corresponding to source nodes
  # 2nd line: comma separated "in-degrees" corresponding to target nodes
  input_degree_seq_file = input_networks/dronet_miR-TF.csv

  # Number of samples to generate for each algorithm
  ngraphs = 5000
  ```
  
5. Running IndeCut:
  ```bash
  make all_parallel   # For parallel executaion on multiple processors
  ```
  The `make all_parallel` command starts compiling all the network motif discovery tools and then running each tool to generate the samples. It then computes average matrices `A` for each network motif discovery samples and computes maximum entropy matrix `Z`. At the end, it computes cut norm extimates for each algorithm and records the results into `results` folder for each graph separately.

7. The cut norm estimates for each algorithm  will be saved into `results` directory (`results/$graphname.cutnorms.txt`).

## License
Please see the LICENSE file for copyright and distribution rights.

