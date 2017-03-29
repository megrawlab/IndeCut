#!/usr/bin/Rscript
library(igraph)

# Handle command line arguments
myargs <- commandArgs(trailingOnly = TRUE)
#myargs <- args[4:length(args)]
edgefile <- myargs[1]
vertexfile <- myargs[2]
edgefileout <- myargs[3]
inputSeed <- myargs[4]
layerNo <- 2

#print(layerNo)
# Set parameters
FAC <- 6

# drawRandGraph function
drawRandGraphWithSwaps <- function(edgefileIn, vertexfileIn, edgefileOut) {

  ef <- read.table(edgefileIn)
  vf <- read.table(vertexfileIn, header=TRUE)
  g <- graph.data.frame(ef, directed=TRUE, vertices=vf)
  
  tfColor<- 0
  mirColor <- 1
  geneColor <- 2
  
  drawPlots <- FALSE
  dBug <- FALSE

  layer = NULL
  fail = NULL;

  if (layerNo == 1 | layerNo == "all") {
  	efLayer1 <- sortedLayerDrawWithSwaps(g, tfColor, tfColor, drawPlots, dBug)
  	fail1 <- all(is.na(efLayer1))
	layer = efLayer1
	fail = fail1
  }
  
  if (layerNo == 2 | layerNo == "all") {
  	efLayer2 <- sortedLayerDrawWithSwaps(g, tfColor, mirColor, drawPlots, dBug)
  	fail2 <- all(is.na(efLayer2))
	layer = efLayer2
	fail = fail2
  }
  
  if (layerNo == 3 | layerNo == "all") {
	efLayer3 <- sortedLayerDrawWithSwaps(g, tfColor, geneColor, drawPlots, dBug)
  	fail3 <- all(is.na(efLayer3))
	layer = efLayer3
        fail = fail3
  }

  if (layerNo == 4 | layerNo == "all") {
  	efLayer4 <- sortedLayerDrawWithSwaps(g, mirColor, tfColor, drawPlots, dBug)
  	fail4 <- all(is.na(efLayer4))
	layer = efLayer4
        fail = fail4
  }
  
  if (layerNo == 5 | layerNo == "all") {
  	efLayer5 <- sortedLayerDrawWithSwaps(g, mirColor, geneColor, drawPlots, dBug)
  	fail5 <- all(is.na(efLayer5))
	layer = efLayer5
        fail = fail5
  }
  
  success <- FALSE
  if (layerNo == "all") {
  	if (fail1 == FALSE & fail2 == FALSE & fail3 == FALSE & fail4 == FALSE & fail5 == FALSE) {
		success <- TRUE
		efFinal <- rbind(efLayer1, efLayer2, efLayer3, efLayer4, efLayer5)
		write.table(efFinal, file=edgefileOut, row.names=FALSE, col.names=FALSE, quote=FALSE, sep="\t")
	}
  } else {
  	if (fail == FALSE ) {
    		success <- TRUE
    		efFinal <- layer
    		write.table(efFinal, file=edgefileOut, row.names=FALSE, col.names=FALSE, quote=FALSE, sep="\t")
  	}
  }
  return(success)
}

# sortedLayerDrawWithSwaps function
sortedLayerDrawWithSwaps <- function(gIN, color1, color2, drawPlot, dBug) {

  # Create graph gLayer, containing only vertices of the two colors
  # and only edges directed from color1 to color2 vertices
  sourceV <- V(gIN)[color == color1]
  targetV <- V(gIN)[color == color2]
  gLayer <- subgraph(gIN, c(sourceV, targetV))
  layerEN <- E(gLayer)[!(V(gLayer)[color==color1] %->% V(gLayer)[color==color2])]
  gLayer <- delete.edges(gLayer, layerEN)

  if (drawPlot == TRUE) {
    par(mfrow=c(2,1))
    plot(gLayer, layout=layout.circle, vertex.label=V(gLayer)$name)
  }
  
  sourceVLayer <- V(gLayer)[color == color1]
  targetVLayer <- V(gLayer)[color == color2]
  sourceOutDeg <- degree(gLayer, v=sourceVLayer, mode="out")
  targetInDeg <- degree(gLayer, v=targetVLayer, mode="in")

  # Compute number of edges in graph,
  # along with sampling weight correction factor
  m <- sum(sourceOutDeg)

  # Added By Mitra: Calculate the minimum allowed FAC
  smax <- max(sourceOutDeg)
  tmax <- max(targetInDeg)
  tmin <- min(targetInDeg)  
  if (tmin == 0) {
	tmin <- sort(unique(targetInDeg))[2]
  }

  FAC_min <-(smax * tmax)/m + 0.1
  p1 <- tmin * (1-((smax*tmin)/(FAC_min*m))) *2
  t1 <- (smax*tmax)/m
  t2 <- (1-(p1/tmax))
  FAC <- t1/t2
#EXTRA FOR SMALL GRAPHS TEST (should be deleted for orig net
  FAC <- FAC + 3
  
  #print(paste("**** M = ", m, sep=""))
  #print(paste("Max_out = ", smax, sep=""))
  #print(paste("Max_in = ", tmax, sep=""))
  #print(paste("Min_in = ", tmin, sep=""))
  #print(paste("FAC_MIN = ", FAC_min, sep=""))
  #print(paste("<<< New FAC = ", FAC, " >>>", sep=""))

  divFac <- FAC*m
  if (divFac == 0) return(TRUE)

  #if (length(targetInDeg) == 1) {
  targetInDegNew <- targetInDeg
  #}
  #else {
  #  targetInDegNew <- sample(targetInDeg)
  #}
  curTargetInDeg <- rep(0, length(targetInDegNew))
  
  if (dBug == TRUE) {
    #print(paste("sourceOutDeg", toString(sourceOutDeg), sep="="))
    #print(paste("targetInDeg", toString(targetInDeg), sep="="))
    #print(paste("targetInDegNew", toString(targetInDegNew), sep="="))
    #Added by Mitra to get infor for IIRG
    #dBug <- FALSE
  }

    # check acceptability of sampling weight correction factors
  didjMat <- sourceOutDeg %*% t(targetInDegNew)
  cMat <- didjMat/divFac
  if (!(all(cMat < 1))) {
    errString <- paste("Unacceptable Correction Factor Matrix for Layer ", color1, ",", color2, sep="");
    print(errString)
    return(NA)
  }
  
  sortedSourceOutDegIdxs <- sort(sourceOutDeg, decreasing=TRUE, index.return=TRUE)$ix

  efOUT <- data.frame()
  exitVal <- 0  
  countSwaps <- 0
  for (i in sortedSourceOutDegIdxs) {
    deg <- sourceOutDeg[i]
    if (deg == 0) {
      next
    }
    if (dBug == TRUE) {
      print(paste("i=", toString(i), " deg=", toString(deg), sep="")) 
      print(paste("targetInDegNew", toString(targetInDegNew), sep="="))
      print(paste("curTargetInDeg", toString(curTargetInDeg), sep="="))
    }
    
    unsatTargetInDegIdx <- which(curTargetInDeg < targetInDegNew)
    curTargetCapacities <-  targetInDegNew - curTargetInDeg

    # compute corrected weights for sampling
    unsatTargetInDegIdxCapacities <- curTargetCapacities[unsatTargetInDegIdx]
    unsatTargetInDegIdxOrigWeights <- targetInDegNew[unsatTargetInDegIdx]
    correctionFactors <- (1 - deg*unsatTargetInDegIdxOrigWeights/divFac)
    samplingWeights <- unsatTargetInDegIdxCapacities*correctionFactors
    if (dBug == TRUE) {
      print(paste("samplingWeights", toString(samplingWeights), sep="="))
    }

    if (deg > length(unsatTargetInDegIdx)) {
      # Need to Perform Swaps:
      #   first place edges to each available target node
      availableIdxs <- unsatTargetInDegIdx
      sNodes <- rep(sourceV$name[i], length(availableIdxs))
      tNodes <- targetV$name[availableIdxs]
      newRows <- cbind(sNodes, tNodes)
      efOUT <- rbind(efOUT, newRows)
      #   update capacity of these used target nodes
      curTargetInDeg[availableIdxs] <- curTargetInDeg[availableIdxs] + 1
      unsatTargetInDegIdx <- which(curTargetInDeg < targetInDegNew)
      curTargetCapacities <-  targetInDegNew - curTargetInDeg

      #   update corrected weights for sampling
      unsatTargetInDegIdxCapacities <- curTargetCapacities[unsatTargetInDegIdx]
      unsatTargetInDegIdxOrigWeights <- targetInDegNew[unsatTargetInDegIdx]
      correctionFactors <- (1 - deg*unsatTargetInDegIdxOrigWeights/divFac)
      samplingWeights <- unsatTargetInDegIdxCapacities*correctionFactors
      
      #   for each unplaced edge,
      for (j in 1:(deg - length(availableIdxs))) {
        #   draw a permutation of unsaturated target nodes (weighted by available capacity)
        #   traverse the permutation to find an unsaturated node that has an allowable swap
        if (length(unsatTargetInDegIdx) == 1) {
          drawnIdxPerm <- unsatTargetInDegIdx
        }
        else {
          drawnIdxPerm <- sample(unsatTargetInDegIdx, prob=samplingWeights)
        }
        for (k in 1:length(drawnIdxPerm)) {
          drawnIdx <- drawnIdxPerm[k]
          #   identify source nodes that have targets not already hit by the current source,
          #   and that do NOT target the drawn target node,
          #   draw one
          drawnName <- targetV$name[drawnIdx]
          sourceNamesTargeting <- efOUT$sNodes[efOUT$tNodes == drawnName]
          allSourceNamesTargetingSomething <- unique(efOUT$sNodes)
          sourceNamesNotTargeting <- setdiff(allSourceNamesTargetingSomething, sourceNamesTargeting)
          
          targetsOfCurSource <- efOUT$tNodes[efOUT$sNodes == sourceV$name[i]]
          allTargetNamesHitBySomething <- unique(efOUT$tNodes)
          nonTargetsOfCurSource <- as.vector(setdiff(allTargetNamesHitBySomething, targetsOfCurSource))
          sourcesHittingNonTargetsOfCurSource <- unique(efOUT$sNodes[efOUT$tNodes %in% nonTargetsOfCurSource])
          
          allowableSources <- as.vector(intersect(sourceNamesNotTargeting, sourcesHittingNonTargetsOfCurSource))
          allowableSourceIdxs <- which(sourceVLayer$name %in% allowableSources)
          targetsOfCurSource <- efOUT$tNodes[efOUT$sNodes == sourceV$name[i]]
          sourceOutDegTmp <- sourceOutDeg
          for (tgtOfCurSource in targetsOfCurSource) {
            sourceNamesTargetingThisTarget <- unique(efOUT$sNodes[efOUT$tNodes == tgtOfCurSource])
            sourcesTargetingThisTargetIdxs <- which(sourceVLayer$name %in% sourceNamesTargetingThisTarget)
            sourceOutDegTmp[sourcesTargetingThisTargetIdxs] <- sourceOutDegTmp[sourcesTargetingThisTargetIdxs] - 1
          }
          allowableSourceWeights <- sourceOutDegTmp[allowableSourceIdxs]
          if (dBug == TRUE) {
            print(paste("k", toString(k), sep="="))
            print(paste("sourceOutDeg", toString(sourceOutDeg), sep="="))
            print(paste("sourceOutDegTmp", toString(sourceOutDegTmp), sep="="))
            print(paste("allowableSources", toString(allowableSources), sep="="))
            print(paste("allowableSourceIdxs", toString(allowableSourceIdxs), sep="="))
            print(paste("allowableSourceWeights", toString(allowableSourceWeights), sep="="))
          }
          
          if (length(allowableSources) > 0) {
            #   allowable swap exists
            #   update capacity of drawn unsaturated target node with allowable swap
            curTargetInDeg[drawnIdx] <- curTargetInDeg[drawnIdx] + 1
            unsatTargetInDegIdx <- which(curTargetInDeg < targetInDegNew)
            curTargetCapacities <-  targetInDegNew - curTargetInDeg
            unsatTargetInDegIdxCapacities <- curTargetCapacities[unsatTargetInDegIdx]

	    #   update corrected weights for sampling
      	    unsatTargetInDegIdxOrigWeights <- targetInDegNew[unsatTargetInDegIdx]
      	    correctionFactors <- (1 - deg*unsatTargetInDegIdxOrigWeights/divFac)
      	    samplingWeights <- unsatTargetInDegIdxCapacities*correctionFactors
             
            if (length(allowableSources) == 1) {
              swapSourceName <- allowableSources
            }
            else {
              # weighted choice of swap sources (necessary in order to choose equally among all valid swap edges)
              swapSourceName <- sample(allowableSources, 1, prob=allowableSourceWeights)
            }
            swapSourceName <- as.vector(swapSourceName)
            #   identify targets of the drawn swap source NOT already targeted by current source, draw one
            swapSourceTargetNames <- efOUT$tNodes[efOUT$sNodes == swapSourceName]
            allowableTargets <- as.vector(setdiff(swapSourceTargetNames, targetsOfCurSource))
            if (length(allowableTargets) == 1) {
              swapSourceTargetName <- allowableTargets
            }
            else {
              swapSourceTargetName <- sample(allowableTargets, 1)
            }
            swapSourceTargetName <- as.vector(swapSourceTargetName)
            #   create new edges from swap source to drawn unsaturated target node
            #                and from current node to drawn target of swap source
            e1 <- c(swapSourceName, drawnName)
            e2 <- c(sourceV$name[i], swapSourceTargetName)
            efOUT <- rbind(efOUT, e1)
            efOUT <- rbind(efOUT, e2)
            #   remove edge from swap source to drawn target of swap source
            remRowIdx <- which(efOUT$sNodes==swapSourceName & efOUT$tNodes==swapSourceTargetName)
            efOUT <- efOUT[-remRowIdx,]
	    countSwaps <- countSwaps + 1
            break 
          }
        }
      }

      next
    }

    if (length(unsatTargetInDegIdx) == 1) {
      drawnIdxs <- unsatTargetInDegIdx
    }
    else {

      drawnIdxs <- sample(unsatTargetInDegIdx, deg, prob=samplingWeights)

    }
    curTargetInDeg[drawnIdxs] <- curTargetInDeg[drawnIdxs] + 1
    
    if (dBug == TRUE) {
      print(paste("unsatTargetInDegIdx", toString(unsatTargetInDegIdx), sep="="))
      print(paste("drawnIdxs", toString(drawnIdxs), sep="="))
      print(paste("curTargetInDeg", toString(curTargetInDeg), sep="="))
    }
    
    sNodes <- rep(sourceV$name[i], deg)
    tNodes <- targetV$name[drawnIdxs]
    newRows <- cbind(sNodes, tNodes)
    efOUT <- rbind(efOUT, newRows)
  }
  names(efOUT) <- c("V1", "V2")

  if (drawPlot == TRUE && exitVal == 0) {
    name <- V(gLayer)$name
    color <- V(gLayer)$color
    vf <- data.frame(name, color)
    gNew <- graph.data.frame(efOUT, directed=TRUE, vertices=vf)
    plot(gNew, layout=layout.circle, vertex.label=V(gNew)$name)
  }

#  print(paste(toString(countSwaps), " Swaps Performed in Layer ", color1, ",", color2, sep=""))

  if (!(all(curTargetInDeg == targetInDegNew))) {
    exitVal <- 1
  }
  if (exitVal == 0) {
    return(efOUT)
  }
  else {
    return(NA)
  }
  
}

# Program body
set.seed(as.integer(inputSeed))
#print(Sys.time())
success <- drawRandGraphWithSwaps(edgefile, vertexfile, edgefileout)
#print(paste("success", toString(success), sep="="))
if (success == "TRUE") {
	print(paste("warswap: Success in generating random graph (", edgefileout,")", sep=""))
} else {
	print(paste("warswap: Failure in generating random graph (", edgefileout,")", sep=""))
}
#print(Sys.time())

