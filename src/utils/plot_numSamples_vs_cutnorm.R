#!/usr/bin/Rscript

compute_z_bounds <- function() {
  results <- read.table(cutnorm_results, header = T, fill = T, check.names = F)
  z_bounds <- as.character(results[1,7])
  s <- substr(z_bounds, 2, nchar(z_bounds) - 1)
  zcutnorm <- strsplit(s, split = ",")
  return(zcutnorm[[1]][1])
}


args <- commandArgs(trailingOnly = TRUE)
graphname <- args[1]
nsamples_total <- args[2]
nsubsamples <- args[3]
cutnorm_results <- args[4] # we get z_cutnorm bounds 
displayName <- graphname
path <- args[5] # out dir of 4 files related to each subsampling cutnorms
outfile <- args[6]

zcutnorm <- as.numeric(compute_z_bounds()[[1]])


cf_path <- paste(path, "/", graphname, "_comofinder_", nsamples_total,"_", nsubsamples, ".txt", sep = "")
di_path <- paste(path, "/", graphname, "_diamcis_", nsamples_total,"_", nsubsamples, ".txt", sep = "")
wr_path <- paste(path, "/", graphname, "_warswap_", nsamples_total,"_", nsubsamples, ".txt", sep = "")
fn_path <- paste(path, "/", graphname, "_fanmod_", nsamples_total,"_", nsubsamples, ".txt", sep = "")

cf <- read.csv(cf_path, header=FALSE)
di <- read.csv(di_path, header=FALSE)
fn <- read.csv(fn_path, header=FALSE)
wr <- read.csv(wr_path, header=FALSE)


colnames(cf) <- c("CoMoFinder_lowerBound", "CoMoFinder_upperBound")
colnames(fn) <- c("FANMOD_lowerBound", "FANMOD_upperBound")
colnames(di) <- c("DIA-MCIS_lowerBound", "DIA-MCIS_upperBound")
colnames(wr) <- c("WaRSwap_lowerBound", "WaRSwap_upperBound")


alldata <- cbind(wr, fn, cf, di)
alldata$sampleCount <- as.numeric(row.names(alldata)) * nsubsamples

library(reshape2)
library(ggplot2)

mdata <- melt(alldata, id=c("sampleCount"))
mdata$value <- mdata$value / zcutnorm
mdata$graph <- displayName


allg <- ggplot(mdata, aes(sampleCount, value, group = variable)) + geom_line(aes(color=variable, linetype=variable)) +
  scale_color_manual(values=c("firebrick1","firebrick1", "chartreuse3","chartreuse3", "cyan3","cyan3", "darkorchid1", "darkorchid1") , name ="algorithm/cutnorm-bound") +
  scale_linetype_manual(values = c("solid","twodash", "solid","twodash", "solid","twodash", "solid","twodash"), name ="algorithm/cutnorm-bound") +
  xlab("Number of sampled graphs") + ylab("Cut norm estimate") + theme_bw() + 
  theme(plot.title = element_text(lineheight=.8, face="italic")) + facet_wrap(~graph, ncol = 1, scales = "free")

ggsave(outfile, allg)

