graphname = graph1
src_degdist = 6,1,1,1,1,1,1,6,1,1,1,1,1,1
target_degdist = 6,1,1,1,1,1,1,6,1,1,1,1,1,1
ngraphs = 5000
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

z_outfile = $(indecut_outdir)/$(graphname)_Z_$(ngraphs).csv
z_cutnorm = $(indecut_outdir)/$(graphname)_Z_$(ngraphs)_cutnorm.csv

all: clean make_diamcis make_fanmod make_comofinder generate_graph_from_deg run_netmotifs compute_A compute_Z compute_cutnorms generate_results

compute_A: 
	$(PYTHON) compute_average_matrix.py $(wr_rand_outdir) $(src_degdist) $(target_degdist) $(wr_A_matrix)
	$(PYTHON) compute_average_matrix.py $(fn_rand_outdir) $(src_degdist) $(target_degdist) $(fn_A_matrix)
	$(PYTHON) compute_average_matrix.py $(cf_rand_outdir) $(src_degdist) $(target_degdist) $(cf_A_matrix)
	$(PYTHON) compute_average_matrix.py $(di_rand_outdir) $(src_degdist) $(target_degdist) $(di_A_matrix)

compute_Z:
	echo $(src_degdist) >> tmp1.txt
	echo $(target_degdist) >> tmp2.txt
	$(PYTHON) MaxEntMatrix.py -c tmp2.txt -r tmp1.txt -o $(z_outfile)
	rm -f tmp1.txt tmp2.txt

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
	bipartite_graph_generation.R $(src_degdist) $(target_degdist) "$(graphs_dir)/$(graphname).fanmod"

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

compute_cutnorms:
	$(PYTHON) CutnormApprox.py -m $(z_outfile) -s $(fn_A_matrix) -o $(fn_cutnorm) -e $(CSDP)
	$(PYTHON) CutnormApprox.py -m $(z_outfile) -s $(wr_A_matrix) -o $(wr_cutnorm) -e $(CSDP)
	$(PYTHON) CutnormApprox.py -m $(z_outfile) -s $(cf_A_matrix) -o $(cf_cutnorm) -e $(CSDP) 
	$(PYTHON) CutnormApprox.py -m $(z_outfile) -s $(di_A_matrix) -o $(di_cutnorm) -e $(CSDP)
	$(PYTHON) CutnormApprox.py -m $(z_outfile) -o $(z_cutnorm) -e $(CSDP)

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