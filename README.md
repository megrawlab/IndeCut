# IndeCUT
A cut norm based method to evaluate uniform and independent sampling in network motif discovery algorithms.

## Requirements
+ Linux operating system
+ 4GB of RAM minimum
+ [Java](https://java.com/en/download/) runtime enviornment 
+ [R](www.r-project.org) core libraries

### Installation
1. Download/clone this repository into your working directory. 
2. Open a command-line terminal
3. Change directory to the src/software folder as follows:
  ```bash
  cd src/software
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


