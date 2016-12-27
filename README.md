# IndeCUT
A cut norm based method to evaluate uniform and independent sampling in network motif discovery algorithms.

## Requirements
+ Linux operating system
+ 4GB of RAM minimum
+ [Java](https://java.com/en/download/) runtime enviornment 
+ [R](www.r-project.org) core libraries

### Required Tools and Libraries
IndeCUT performs uniform/independent sampling evaluation on a set of graphs that is produces by each network motif discovery algorithm. In this package we provide the following pre-compiled network motif discovery tools which are mentioned in the paper:

+ FANMOD

  1. The binary files required for running `FANMOD` are located at `fanmod` folder.
  2. `fanmodmultrg` is a pre-compiled `C` program which outputs randomly generated graphs by `FANMOD` algorithm into `fanmod/fn_out` folder.
+ WaRSwap

  1. `WaRSwap` is implemented in `R` and is located at `warswap` folder.
  2. This program outputs the random graphs into `warswap/wr_out` folder.  
+ DIA-MCIS

  1. The binary files required for running `DIA-MCIS` are located at `diamcis` folder.
  2. `dia-mcis-standard` is a pre-compiled `C` program which outputs randomly generated graphs by `DIA-MCIS` algorithm into `diamcis/di_out` folder.
+ CoMoFinder

  1. This tool is implemented in java and the binary classes required for running `CoMoFinder` are located at `comofinder` folder.
  2. `comoFinder_randout.jar` is a pre-compiled `Java` program which outputs randomly generated graphs by `CoMoFinder` algorithm into `comofinder/cf_out` folder.
  
+ CSDP

## Installation
1. Download/clone this repository into your working directory. 
2. Open a command-line terminal
3. Change directory to the src/software folder as follows:
  ```bash
  cd software
  ```
  
4. Open the file `start_running_indecut.sh` and make the following changes (if requires):
  1. set PYTHON variable to the path that Python is installed on your computer.
    ```bash
    # Example
    PYTHON=/usr/bin/python
    ```
    
  2. The input graph is a bipartite graph that is defined by a name, source out-degree and target in-degree sequences (comma separated sequence). 
  ```bash
  # Default settings
  graphname="graph1"
  src_degdist="10,1,1,1,1,1,1,1,1,1,1"
  target_degdist="10,1,1,1,1,1,1,1,1,1,1"
  ```
  
    You can change the above configurations as follows: 
    1. change `graphname` to any arbitarary name. The name can contain characters and numbers. 
    2. change `src_degdist` to a new source out-degree sequence. 
    3. change `target_degdist` to a new target in-degree sequence.

5. Run IndeCUT:
  ```bash
  ./start_running_indecut.sh
  ```
6. When the running finishes the following text message wil appear on the terminal:
  ```bash
  "Program finished successfully and results saved into cutnorm_results/$graphname.cutnorms.csv file!"
  ```
  
7. The output file will be saved into `cutnorm_results` directory (`cutnorm_results/$graphname.cutnorms.csv`).

## License
Please see the LICENSE file for copyright and distribution rights.

