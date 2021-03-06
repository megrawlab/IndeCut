# This is the main exe file for whole program. Set required arguments and run it under linux OS.

graphname = dronet_miR-TF
input_degree_seq_file = input_networks/$(graphname).csv

ngraphs = 50
result_outdir = results
result_outfile = $(result_outdir)/$(graphname)_$(ngraphs).cutnorms.txt

graphs_dir = graphs
indecut_outdir = output
PYTHON = /local/cluster/bin/python
CSDP = Csdp-6.1.1/solver/csdp 

diamcis_srcdir = diamcis/source-code/
fanmod_srcdir = fanmod/source-code/
comofinder_srcdir = comofinder

wr_outdir = warswap/wr_out
fn_outdir = fanmod/fn_out
cf_outdir = comofinder/cf_out
di_outdir = diamcis/di_out

wr_rand_outdir = $(wr_outdir)/$(graphname).$(ngraphs)/$(graphname).rand.igraph
fn_rand_outdir = $(fn_outdir)/$(graphname).$(ngraphs)/$(graphname).rand.fanmod
cf_rand_outdir = $(cf_outdir)/$(graphname).$(ngraphs)/$(graphname).rand.comofinder
di_rand_outdir = $(di_outdir)/$(graphname).$(ngraphs)/$(graphname).rand.diamcis

wr_A_matrix = $(indecut_outdir)/$(graphname)_warswap_$(ngraphs)_A.csv
fn_A_matrix = $(indecut_outdir)/$(graphname)_fanmod_$(ngraphs)_A.csv
cf_A_matrix = $(indecut_outdir)/$(graphname)_comofinder_$(ngraphs)_A.csv
di_A_matrix = $(indecut_outdir)/$(graphname)_diamcis_$(ngraphs)_A.csv

wr_cutnorm = $(indecut_outdir)/$(graphname)_warswap_$(ngraphs)_cutnorm.csv
fn_cutnorm = $(indecut_outdir)/$(graphname)_fanmod_$(ngraphs)_cutnorm.csv
cf_cutnorm = $(indecut_outdir)/$(graphname)_comofinder_$(ngraphs)_cutnorm.csv
di_cutnorm = $(indecut_outdir)/$(graphname)_diamcis_$(ngraphs)_cutnorm.csv 

z_outfile = $(indecut_outdir)/$(graphname)_Z1_$(ngraphs).csv
z_cutnorm = $(indecut_outdir)/$(graphname)_Z_$(ngraphs)_cutnorm.csv

dummy_outdir = cutnorm_dummies/$(graphname)
subsample_counts_outdir = sample_counts_out

init:
Rcore := $(shell command -v R 2> /dev/null)
JAVA := $(shell command -v java 2> /dev/null)

ifndef Rcore
	$(error "R is not available please install R")
endif
$(info "Checking R .. yes!")
ifndef JAVA
    	$(error "JAVA is not available please install java 1.8+")
endif
$(info "Checking Java .. yes!")

JVERSION := $(shell java -version 2>&1 | awk -F '\"' '/version/ {print $$2}')
JAVAV := $(shell expr `java -version 2>&1 | awk -F '\"' '/version/ {print $$2}'` \>= 1.8 )

ifeq "$(JAVAV)" "1"
   	$(info "Checking java version .. $(JVERSION)")
endif

GMPLIB = "/usr/lib/libgmp.so.3"
ifneq ("$(wildcard $(GMPLIB))","")
FILE_EXISTS = 1
else
$(warning "Checking GMP library .. Failed! Install GNU gmp or copy libgmp.so.3 into /usr/lib/")
endif

#	$(MAKE) -f Makefile_check
#### For serial execution of steps ####
all_serial: compile_tools generate_graph_from_deg run_netmotifs compute_A compute_Z compute_cutnorms generate_results plot_subsamples

#### Runs each step in parallel to speed up the whole procedure ####
all_parallel:
	$(MAKE) compile_tools
	$(MAKE) generate_graph_from_deg
	$(MAKE) run_netmotifs -j 4
	$(MAKE) compute_A -j 5
	$(MAKE) compute_cutnorms -j 5
	$(MAKE) generate_results
	

#### Get the plots for number of samples vs. performance
num_subsamples = 10

num_samples_analysis:
	$(PYTHON) NumGraphsVsCutNorm.py $(graphname) $(ngraphs) $(num_subsamples) $(fn_rand_outdir) $(wr_rand_outdir) $(di_rand_outdir) $(cf_rand_outdir) $(z_outfile) $(CSDP) $(dummy_outdir) $(subsample_counts_outdir)

plot_subsamples: num_samples_analysis
	./plot_numSamples_vs_cutnorm.R $(graphname) $(ngraphs) $(num_subsamples) $(result_outfile) $(subsample_counts_outdir) $(subsample_counts_outdir)/$(graphname)_samples_vs_cutnorms.png

##### Compile programs #######
compile_tools: init make_diamcis make_fanmod make_comofinder
	chmod -R 755 .
make_diamcis: 
	make -C $(diamcis_srcdir) clean
	make -C $(diamcis_srcdir) all

make_fanmod: 
	make -C $(fanmod_srcdir) clean
	make -C $(fanmod_srcdir) all

make_comofinder:
	make -C $(comofinder_srcdir) clean
	make -C $(comofinder_srcdir) all


generate_graph_from_deg:
	if [ ! -d $(graphs_dir) ]; then mkdir -p $(graphs_dir); fi
	./bipartite_graph_generation.R $(input_degree_seq_file) "$(graphs_dir)/$(graphname).fanmod"

##### Run network motifs discovery algorithms ####
run_netmotifs: run_fanmod run_diamcis run_warswap run_comofinder

run_fanmod:
	cd fanmod; \
	./run_fanmod.sh $(graphname) $(ngraphs) 

run_diamcis:
	cd diamcis; \
        ./run_diamcis.sh $(graphname) $(ngraphs) 

run_warswap:
	cd warswap; \
	./run_warswap.sh $(graphname) $(ngraphs) 

run_comofinder:
	cd comofinder; \
	./run_comofinder.sh $(graphname) $(ngraphs)

##### Compute average matrices for each algorithm outcome #####
compute_A: compute_wr_A compute_fn_A compute_di_A compute_cf_A compute_Z

compute_wr_A:
	$(PYTHON) compute_average_matrix.py $(wr_rand_outdir) $(input_degree_seq_file) $(wr_A_matrix)

compute_fn_A:
	$(PYTHON) compute_average_matrix.py $(fn_rand_outdir) $(input_degree_seq_file) $(fn_A_matrix)

compute_cf_A:
	$(PYTHON) compute_average_matrix.py $(cf_rand_outdir) $(input_degree_seq_file) $(cf_A_matrix)

compute_di_A:
	$(PYTHON) compute_average_matrix.py $(di_rand_outdir) $(input_degree_seq_file) $(di_A_matrix)

compute_Z:
	$(PYTHON) MaxEntMatrix.py -i $(input_degree_seq_file) -o $(z_outfile)

##### Compute cutnorm estimates #####
compute_cutnorms: compute_cutnorms_wr compute_cutnorms_fn compute_cutnorms_cf compute_cutnorms_di compute_cutnorms_z

compute_cutnorms_fn:
	$(PYTHON) CutnormApprox.py -m $(z_outfile) -s $(fn_A_matrix) -o $(fn_cutnorm) -e $(CSDP)

compute_cutnorms_wr:
	$(PYTHON) CutnormApprox.py -m $(z_outfile) -s $(wr_A_matrix) -o $(wr_cutnorm) -e $(CSDP)

compute_cutnorms_cf:
	$(PYTHON) CutnormApprox.py -m $(z_outfile) -s $(cf_A_matrix) -o $(cf_cutnorm) -e $(CSDP) 

compute_cutnorms_di:
	$(PYTHON) CutnormApprox.py -m $(z_outfile) -s $(di_A_matrix) -o $(di_cutnorm) -e $(CSDP)

compute_cutnorms_z:
	$(PYTHON) CutnormApprox.py -m $(z_outfile) -o $(z_cutnorm) -e $(CSDP)


##### Result processing ####
generate_results:
	if [ ! -d $(result_outdir) ]; then mkdir  -p $(result_outdir); fi; \
	./process_cutnorms.sh $(wr_cutnorm) $(fn_cutnorm) $(cf_cutnorm) $(di_cutnorm) $(z_cutnorm) $(result_outfile) $(graphname) $(ngraphs)


clean:
	rm -f -r $(fn_outdir)/$(graphname).$(ngraphs)
	rm -f -r $(di_outdir)/$(graphname).$(ngraphs)
	rm -f -r $(wr_outdir)/$(graphname).$(ngraphs)
	rm -f -r $(cf_outdir)/$(graphname).$(ngraphs)
	rm -f warswap/*.txt warswap/*.Rout
	rm -f $(graphs_dir)/$(graphname)*
	rm -f $(indecut_outdir)/$(graphname)_*
