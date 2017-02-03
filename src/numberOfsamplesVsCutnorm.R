#!/usr/bin/Rscript
graphname <- "Encode_comoF_TF-Gene"
nsamples_total <- 5000
nsubsamples <- 1000
zcutnorm <- 6437.000388
displayName <- "Human_TF->Gene"

path <- "Downloads/samples_vs_cutnorms_out/"

cf_path <- paste(path, graphname, "_comofinder_", nsamples_total,"_", nsubsamples, ".txt", sep = "")
di_path <- paste(path, graphname, "_diamcis_", nsamples_total,"_", nsubsamples, ".txt", sep = "")
wr_path <- paste(path, graphname, "_warswap_", nsamples_total,"_", nsubsamples, ".txt", sep = "")
fn_path <- paste(path, graphname, "_fanmod_", nsamples_total,"_", nsubsamples, ".txt", sep = "")

cf <- read.csv(cf_path, header=FALSE)
di <- read.csv(di_path, header=FALSE)
fn <- read.csv(fn_path, header=FALSE)
wr <- read.csv(wr_path, header=FALSE)

di <- wr
di$V1 <- 0
di$V2 <- 0

colnames(cf) <- c("CoMoFinder_lowerBound", "CoMoFinder_upperBound")
colnames(fn) <- c("FANMOD_lowerBound", "FANMOD_upperBound")
colnames(di) <- c("*DIA-MCIS_lowerBound", "*DIA-MCIS_upperBound")
colnames(wr) <- c("WaRSwap_lowerBound", "WaRSwap_upperBound")


alldata <- cbind(wr, fn, cf, di)
#alldata <- cbind(wr, fn, cf)
alldata$sampleCount <- as.numeric(row.names(alldata)) * nsubsamples

library(reshape2)
library(ggplot2)

mdata <- melt(alldata, id=c("sampleCount"))
mdata$value <- mdata$value / zcutnorm
#mdata$graph <- displayName
mdata$graph <- paste("A-", displayName, sep = "")
mdata <- subset(mdata, mdata$sampleCount > 200)
#mdata <- subset(mdata, mdata$value > 0)

alldata <- cbind(wr, fn, di)
alldata$sampleCount <- as.numeric(row.names(alldata)) * nsubsamples
mdata2 <- melt(alldata, id=c("sampleCount"))
mdata2$value <- mdata2$value / zcutnorm
mdata2$graph <- paste("B-", displayName, sep = "")
mdata2 <- subset(mdata2, mdata2$sampleCount > 200)

mdata_merged <- rbind(mdata, mdata2)
allg <- ggplot(mdata_merged, aes(sampleCount, value, group = variable)) + geom_line(aes(color=variable, linetype=variable)) +
  scale_color_manual(values=c("firebrick1","firebrick1", "chartreuse3","chartreuse3", "cyan3","cyan3", "darkorchid1", "darkorchid1") , name ="algorithm/cutnorm-bound") +
  scale_linetype_manual(values = c("solid","twodash", "solid","twodash", "solid","twodash", "solid","twodash"), name ="algorithm/cutnorm-bound") +
  xlab("Number of sampled graphs") + ylab("Cut norm estimate") + theme_bw() + 
  theme(plot.title = element_text(lineheight=.8, face="italic")) + facet_wrap(~graph, ncol = 1, scales = "free")

allg

threeallg <- ggplot(mdata, aes(sampleCount, value, group = variable)) + geom_line(aes(color=variable, linetype=variable)) + 
  scale_color_manual(values=c("firebrick1","firebrick1", "chartreuse3","chartreuse3", "darkorchid1","darkorchid1")) +
  scale_linetype_manual(values = c("solid","twodash", "solid","twodash", "solid","twodash")) +
  xlab("Number of sampled graphs") + ylab("Cut norm estimate") +
  theme(plot.title = element_text(lineheight=.8, face="italic"))+ facet_wrap(~graph, ncol = 1) + theme_bw()

threeallg


g1file <- paste(graphname, "_alltimes_allAlgs.png", sep = "")
ggsave(file=g1file, allg)

mdata_20 <- subset(mdata, mdata$sampleCount <110)
all20g <- ggplot(mdata_20, aes(sampleCount, value, group = variable)) + geom_line(aes(color=variable, linetype=variable)) + 
  scale_color_manual(values = c("firebrick1", "firebrick1", "cyan3", "cyan3", "darkorchid1", "darkorchid1", "chartreuse3", "chartreuse3")) +
  scale_linetype_manual(values = c("solid","twodash", "solid","twodash", "solid","twodash", "solid","twodash")) +
  xlab("Number of sampled graphs") + ylab("Cut norm estimate") +
  ggtitle(paste("Cutnorm estimate vs. number of sampled graphs\ngraphname : ", displayName, sep="")) + 
  theme(plot.title = element_text(lineheight=.8, face="italic"))

all20g
g2file <- paste(graphname, "_+20_allAlgs.png", sep = "")
ggsave(file=g2file, all20g)

mdata_20_nocf <- subset(mdata_20, !(mdata_20$variable %in% "cf_lower_bound") & !(mdata_20$variable %in% "cf_upper_bound"))
all20g_nocf <- ggplot(mdata_20_nocf, aes(sampleCount, value, group = variable)) + geom_line(aes(color=variable, linetype=variable)) + 
  scale_color_manual(values = c("red", "red", "green", "green", "black", "black")) +
  scale_linetype_manual(values = c("solid","twodash", "solid","twodash", "solid","twodash")) +
  xlab("Number of Graphs") + ylab("Cut norm estimate") + ggtitle(graphname)

all20g_nocf
g3file <- paste(graphname, "_+20_-comoFinder.png", sep = "")
ggsave(file=g3file, all20g_nocf)


