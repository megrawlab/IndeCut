# IndeCUT
IndeCut is the first and only method to date that evaluates the performance of network motif discovery algorithms on any network of interest. IndeCut takes advantage of a novel cut-norm based approach to evaluate independent and uniform background network generation.

## Requirements
+ Linux operating system
+ Java 1.8+
+ 4GB of RAM minimum
+ [R](www.r-project.org) core libraries
+ GNU gmp library
+ Python2.7+

### Network motif discovery tools
Four network motif discovery tools along with their source code are provided in the `software` folder. 

1. FANMOD
  This software package is located in the `software/fanmod` folder. The `source-code` folder contains required `C` and `C++` libraries and source code to make an executable `FANMOD` tool (`makefile` is located at `software/fanmod/source-code`).
  
2. DIA-MCIS
  This software package is located in the `software/diamcis` folder. The `source-code` folder contains required `C` libraries and source code to make an executable `DIA-MCIS` tool (`makefile` is located at `software/diamcis/source-code`).
  
3. CoMoFinder
  This software package is located in the `software/comofinder` folder. This software is written in `Java` and required libraries to compile and make an executable `jar` file is located at `software/comofinder/`.
  
4. WaRSwap
  This software package is located in the `software/warswap` folder. This software package is written in `R` and doesn't need any compilation. WaRSwap uses the "igraph" library, which requires that "libgmp.so.3" be available on your machine in /usr/lib/ . 

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
IndeCut package contains different network motif tools which have their own requirements. Some common problems may arise, we provide solutions as follows:

* comoFinder compiliation error: `javac -g -d classes -cp lib/jsr166y-1.7.0.jar -target 1.8 src/ccbr/utoronto/ca/*/*.java javac: invalid target release: 1.8`

  This happens for two reasons: 1) The java is not installed, or 2) Installed java version is older than 1.8
  
  Solution: Install java or update it to newest version
  
* WaRSwap error: `Error : .onLoad failed in loadNamespace() for 'igraph', details`:
  Why: R is not installed on your machine, or GNU gmp library is not installed, or `libgmp.so.3` is not in the library path (such as /usr/bin)
  
  Solution: Download and install R from https://www.r-project.org.
  If R is already installed on your machine, download and install GNU gmp package from https://gmplib.org. Follow the instructions to make sure that the `gmp` libs are installed. If you still get the error from WaRSwap, it means you need to create a softlink (or copy the library from where it is installed) to libgmp.so.4 in the `/usr/lib` directory.

* CSDP Error: `error while loading shared libraries: liblapack.so.3: cannot open shared object file: No such file or directory`

  The pre-compiled CSDP is known to run successfully on newer operating system versions; if you have an older operating system version, check compatibility and install CSDP from its source available at https://projects.coin-or.org/Csdp/ .

### Datasets
The human regulatory network dataset example used in IndeCut's manuscript description is published under the Encode project, it was obtained from: `http://encodenets.gersteinlab.org`.

## License
Please see the LICENSE file for copyright and distribution rights.

