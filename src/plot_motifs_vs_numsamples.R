data <- read.table(file = "~/Downloads/motifs.txt", sep = "\t", header = T)

library(ggplot2)
library(plyr)

total_samples = 21 * 50
motif_freq = count(data, 'motif_id')

ggplot(motif_freq, aes(motif_id, freq/total_samples)) + geom_point(color="blue") +
  theme(axis.text.x = element_text(angle = 90, hjust = 1)) +
  ggtitle("Motifs vs. %observations in 21 samples with 50 iterations")

d <- data.frame(table(data$motif_id, data$nsample))
colnames(d) <- c("motif_id", "nsamples", "freq")

ggplot(d, aes(nsamples, freq)) + geom_line() + 
  facet_wrap(~motif_id, ncol = 2)

motif="38_001101001"
motif_data <- subset(data, data$motif_id == motif)
motif_sample_freq <- count(motif_data, 'nsamples')
ggplot(motif_sample_freq, aes(nsamples, freq/50)) + geom_line() + geom_point() +
  ylim(c(0,1))
       