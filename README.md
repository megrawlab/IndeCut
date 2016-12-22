# IndeCUT
A cut norm based method to evaluate uniform and independent sampling in network motif discovery algorithms.

## Requirements
+ Linux operating system
+ 4GB of RAM minimum
+ [Java](https://java.com/en/download/) runtime enviornment 
+ [R](www.r-project.org) core libraries

### Installation
+ Download/clone this repository into your working directory. 
+ Open a command-line terminal
+ Change directory to the src/software folder as follows:
```
cd src/software
```
+ Open the file `start_running_indecut.sh` and make the following changes if applies:
..4. set PYTHON variable to the path that Python is installed on your computer.
..4. The input graph is a bipartite graph that is defined by a name, source out-degree and target in-degree sequences (comma separated sequence). You can enter your own input graph's properties as follows: 
..* change `graphname` to any arbitarary name. The name can contain characters and numbers. 
..* change `src_degdist` to a new source out-degree sequence. 
..* change `target_degdist` to a new target in-degree sequence.

5. Run IndeCUT:
```
./start_running_indecut.sh
```
..* This code will compute cutnorm estimates for an example graph with degree sequence of R=C={10,1,1,1,1,1,1,1,1,1,1}. You can change your desired input degree sequence 

