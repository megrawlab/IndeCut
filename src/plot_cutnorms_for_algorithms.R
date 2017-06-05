#!/usr/bin/Rscript

args <- commandArgs(trailingOnly = TRUE)
if (length(args) < 1) {
    cat("Usage ./plot_cutnorms_for_algorithms.R [cutnorm_infile.txt]\n")
    quit(save="no", status=1)
}

cutnorm_infile <- "Downloads/even_graphs_cutnorms.txt"

library(ggplot2)
path <- "."
outgraphs <- read.table(paste(cutnorm_infile ,sep = ""), sep="\t", header = T)
dmain <- data.frame(outgraphs)
data <- data.frame(graph=c(), algorithm = c(), lower_bound=c(), upper_bound=c(), row.names = NULL)
algorithms <- c("WaRSwap", "FANMOD", "CoMoFinder", "DIA-MCIS")

for (i in 1:nrow(dmain)) {
  count = 1
  for (al in c("WR", "FN", "comoF", "diamcis")) {
    lname <- paste(al, "_low_norm_cutnorm", sep="")
    uname <- paste(al, "_up_norm_cutnorm", sep="")
    d <- dmain[i, c(lname, uname, "graphName")]
    ndata <- data.frame(graph=d[1,3], algorithm = algorithms[count], lower_bound=d[1,1], upper_bound=d[1,2])
    data <- rbind(data, ndata)
    count = count + 1
  }
}
Algorithm = factor(data$algorithm)

g1 <- ggplot(data, aes(y = Algorithm)) + labs(x = "cutnorm", y = "algorithm")  + 
  geom_segment(aes(x = data$lower_bound, y = Algorithm, xend = data$upper_bound, yend = Algorithm, color = Algorithm), size = 1)  +
  geom_point(aes(x = data$lower_bound, color = Algorithm), size = 5, shape = 'I') +
  geom_point(aes(x = data$upper_bound, color = Algorithm), size = 5, shape = 'I') + 
  facet_wrap(~graph, ncol = 1) + scale_x_continuous(breaks=seq(0,0.9, 0.08)) +
  theme_bw() +
  scale_color_manual(values=c("firebrick1", "chartreuse3", "cyan3", "darkorchid1")) +
  theme(axis.text = element_text(size=9, color="black",face="bold")) + 
  theme(legend.text=element_text(size=9,face="bold")) +
  theme(strip.text = element_text(size=9,face="bold"))
  
g1
ggsave(g1, file=paste(cutnorm_infile, "_4alg_plot.jpg", sep = ""))

###### Empty values in ComoFinder related fields
comoNull <- data
comoNull[comoNull$algorithm=="CoMoFinder", c("lower_bound", "upper_bound")] <- NA
comoNull$algorithm <- ifelse(comoNull$algorithm == "CoMoFinder", "*CoMoFinder", algorithms[comoNull$algorithm])

Algorithm = factor(comoNull$algorithm)

g3 <- ggplot(comoNull, aes(y = Algorithm)) + labs(x = "cutnorm", y = "algorithm")  + 
  geom_segment(aes(x = lower_bound, y = Algorithm, xend = upper_bound, yend = Algorithm, color = Algorithm), size = 1)  +
  geom_point(aes(x = lower_bound, color = Algorithm), size = 4, shape = 'I') +
  geom_point(aes(x = upper_bound, color = Algorithm), size = 4, shape = 'I') + 
  facet_wrap(~graph, ncol = 1) + 
  #scale_x_continuous(breaks=seq(0,0.9, 0.04)) +
  theme_bw() +
  scale_color_manual(values=c("cyan3", "darkorchid1", "chartreuse3","firebrick1")) +
  theme(axis.text = element_text(size=10, color="black",face="bold")) + 
  theme(legend.text=element_text(size=10,face="bold")) +
  theme(strip.text = element_text(size=10,face="bold"))
g3

ggsave(g3, file=paste(cutnorm_infile, "_comoFNull_plot.jpg", sep = ""), height = 6)
###########

threealg <- subset(data, !(data$algorithm %in% "*CoMoFinder"))
Algorithm = factor(threealg$algorithm)

g2 <- ggplot(threealg, aes(y = Algorithm)) + labs(x = "cutnorm", y = "algorithm")  + 
  geom_segment(aes(x = lower_bound, y = Algorithm, xend = upper_bound, yend = Algorithm, color = Algorithm), size = 1)  +
  geom_point(aes(x = lower_bound, color = Algorithm), size = 4, shape = 'I') +
  geom_point(aes(x = upper_bound, color = Algorithm), size = 4, shape = 'I') + 
  facet_wrap(~graph, ncol = 1) + 
  #scale_x_continuous(breaks=seq(0,0.9, 0.04)) +
  theme_bw() +
  scale_color_manual(values=c("firebrick1", "chartreuse3", "darkorchid1")) +
  theme(axis.text = element_text(size=9, color="black",face="bold")) + 
  theme(legend.text=element_text(size=9,face="bold")) +
  theme(strip.text = element_text(size=9,face="bold"))
g2

ggsave(g2, file=paste(cutnorm_infile, "_3alg_plot.jpg", sep = ""))

