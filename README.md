# IndeCUT
A cut norm based method to evaluate uniform and independent sampling in network motif discovery algorithms.

## Requirements
+ Linux operating system
+ Java 1.8+
+ 4GB of RAM minimum
+ [R](www.r-project.org) core libraries
+ GNU gmp library
+ Python2.7+

### Network motif discovery tools
Four network motif discover tools along with their source codes are provided under `software` folder. 

1. FANMOD
  This software package is located under `software/fanmod` folder. The `source-code` folder contains required `C` and `C++` libraries and source codes to make an executable `FANMOD` tool (`makefile` is located at `software/fanmod/source-code`).
  
2. DIA-MCIS
  This software package is located under `software/diamcis` folder. The `source-code` folder contains required `C` libraries and source codes to make an executable `DIA-MCIS` tool (`makefile` is located at `software/diamcis/source-code`).
  
3. CoMoFinder
  This software package is located under `software/comofinder` folder. This software is written in `Java` and required libraries to compile and make an executable `jar` file is located at `software/comofinder/`.
  
4. WaRSwap
  This software package is located under `software/warswap` folder. This software package is written in `R` and doesn't need any compilation. WaRSwap uses "igraph" library which requires that "libgmp.so.3" be available on your machine under /usr/lib/ directory. 

## Installing and Running IndeCut
1. Download/clone this repository into your working directory. 
2. Open a command-line terminal
3. Change directory to the `software` folder as follows:
  ```bash
  cd IndeCut/software
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
  
  or
  
  make all_serial   # For serial executaion which is slower than parallel executaion
  ```
  The `make all_parallel` command starts compiling all the network motif discovery tools and then running each tool to generate the samples. It then computes average matrices `A` for each network motif discovery samples and computes maximum entropy matrix `Z`. At the end, it computes cut norm extimates for each algorithm and records the results into `results` folder for each graph separately.

7. The cut norm estimates for each algorithm  will be saved into `results` directory (`results/$graphname.cutnorms.txt`).

## Troubleshooting
IndeCut package contains different network motif tools which have their own requirements. Some sommon problem may arise while running given package which we provide solution for them as follows:

* comoFinder compile error:
  Why: This happens for two reasons: 1) The java is not installed, or 2) Installed java version is older than 1.8
  Solution: Install java or update it to newest version
  
* WaRSwap error:
  Why: R is not installed in your machine or GNU gmp library is not installed or `libgmp.so.3` is not in the library path (such as /usr/bin)
  Solution: Download and install R from https://www.r-project.org.
  If R is already installed on your machine, download and install GNU gmp package from https://gmplib.org. Follow the instructions to make sure that the `gmp` libs are installed. If you still get the error from WaRSwap, it means you need to create a softlink (or copy the library from where it is installed) to libgmp.so.4 in the `/usr/lib` directory.
 

### datasets
Human regulatory network is published under Encode project and we downloaded it from: `http://encodenets.gersteinlab.org`.

## License
Please see the LICENSE file for copyright and distribution rights.

