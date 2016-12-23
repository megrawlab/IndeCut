#!/usr/bin/Rscript

args <- commandArgs(trailingOnly = TRUE)

sourceOutDeg <- as.numeric(unlist(strsplit(args[1], ",")))
targetInDeg <- as.numeric(unlist(strsplit(args[2], ",")))
outgraphName <- args[3]

if (sum(sourceOutDeg) != sum(targetInDeg)) {
    stop("deg sequences are not valid!")
}

#sourceOutDeg <- as.numeric(unlist(strsplit("2,2,2", ",")))
#targetInDeg <- as.numeric(unlist(strsplit("2,2,2", ",")))

sourceV <- seq(0, length(sourceOutDeg) -1)
targetV <- seq(1, length(targetInDeg)) + max(sourceV)

targetInDegNew <- targetInDeg
curTargetInDeg <- rep(0, length(targetInDegNew))

sortedSourceOutDegIdxs <- sort(sourceOutDeg, decreasing=TRUE, index.return=TRUE)$ix

efOUT <- data.frame()
exitVal <- 0
countSwaps <- 0
for (i in sortedSourceOutDegIdxs) {
#i=1
  deg <- sourceOutDeg[i]
  if (deg == 0) {
    next
  }

#    print(paste("i=", toString(i), " deg=", toString(deg), sep=""))
#    print(paste("targetInDegNew", toString(targetInDegNew), sep="="))
#    print(paste("curTargetInDeg", toString(curTargetInDeg), sep="="))
  
  unsatTargetInDegIdx <- which(curTargetInDeg < targetInDegNew)
  curTargetCapacities <-  targetInDegNew - curTargetInDeg
  
  # compute corrected weights for sampling
  unsatTargetInDegIdxCapacities <- curTargetCapacities[unsatTargetInDegIdx]
  unsatTargetInDegIdxOrigWeights <- targetInDegNew[unsatTargetInDegIdx]
  #correctionFactors <- (1 - deg*unsatTargetInDegIdxOrigWeights/divFac)
  samplingWeights <- unsatTargetInDegIdxCapacities
#    print(paste("samplingWeights", toString(samplingWeights), sep="="))
  
  unsatTargetInDegIdx <- which(curTargetInDeg < targetInDegNew)
  curTargetCapacities <-  targetInDegNew - curTargetInDeg
  
  if (deg > length(unsatTargetInDegIdx)) {
    # Need to Perform Swaps:
    #   first place edges to each available target node
    availableIdxs <- unsatTargetInDegIdx
    sNodes <- rep(sourceV[i], length(availableIdxs))
    tNodes <- targetV[availableIdxs]
    newRows <- cbind(sNodes, tNodes, 0, 1)
    efOUT <- rbind(efOUT, newRows)
    #   update capacity of these used target nodes
    curTargetInDeg[availableIdxs] <- curTargetInDeg[availableIdxs] + 1
    unsatTargetInDegIdx <- which(curTargetInDeg < targetInDegNew)
    curTargetCapacities <-  targetInDegNew - curTargetInDeg
    
    #   update corrected weights for sampling
    unsatTargetInDegIdxCapacities <- curTargetCapacities[unsatTargetInDegIdx]
    unsatTargetInDegIdxOrigWeights <- targetInDegNew[unsatTargetInDegIdx]
    samplingWeights <- unsatTargetInDegIdxCapacities
    
    #   for each unplaced edge,
    for (j in 1:(deg - length(availableIdxs))) {
      #   draw a permutation of unsaturated target nodes (weighted by available capacity)
      #   traverse the permutation to find an unsaturated node that has an allowable swap
      if (length(unsatTargetInDegIdx) == 1) {
        drawnIdxPerm <- unsatTargetInDegIdx
      }
      else {
        #drawnIdxPerm <- sample(unsatTargetInDegIdx, prob=samplingWeights)
        drawnIdxPerm <- sample(unsatTargetInDegIdx)
      }
      for (k in 1:length(drawnIdxPerm)) {
        drawnIdx <- drawnIdxPerm[k]
        #   identify source nodes that have targets not already hit by the current source,
        #   and that do NOT target the drawn target node,
        #   draw one
        drawnName <- targetV[drawnIdx]
        sourceNamesTargeting <- efOUT$sNodes[efOUT$tNodes == drawnName]
        allSourceNamesTargetingSomething <- unique(efOUT$sNodes)
        sourceNamesNotTargeting <- setdiff(allSourceNamesTargetingSomething, sourceNamesTargeting)
        
        targetsOfCurSource <- efOUT$tNodes[efOUT$sNodes == sourceV[i]]
        allTargetNamesHitBySomething <- unique(efOUT$tNodes)
        nonTargetsOfCurSource <- as.vector(setdiff(allTargetNamesHitBySomething, targetsOfCurSource))
        sourcesHittingNonTargetsOfCurSource <- unique(efOUT$sNodes[efOUT$tNodes %in% nonTargetsOfCurSource])
        
        allowableSources <- as.vector(intersect(sourceNamesNotTargeting, sourcesHittingNonTargetsOfCurSource))
        allowableSourceIdxs <- which(sourceV %in% allowableSources)
        targetsOfCurSource <- efOUT$tNodes[efOUT$sNodes == sourceV[i]]
        sourceOutDegTmp <- sourceOutDeg
        for (tgtOfCurSource in targetsOfCurSource) {
          sourceNamesTargetingThisTarget <- unique(efOUT$sNodes[efOUT$tNodes == tgtOfCurSource])
          sourcesTargetingThisTargetIdxs <- which(sourceV %in% sourceNamesTargetingThisTarget)
          sourceOutDegTmp[sourcesTargetingThisTargetIdxs] <- sourceOutDegTmp[sourcesTargetingThisTargetIdxs] - 1
        }
        allowableSourceWeights <- sourceOutDegTmp[allowableSourceIdxs]
#          print(paste("k", toString(k), sep="="))
#          print(paste("sourceOutDeg", toString(sourceOutDeg), sep="="))
#          print(paste("sourceOutDegTmp", toString(sourceOutDegTmp), sep="="))
#          print(paste("allowableSources", toString(allowableSources), sep="="))
#          print(paste("allowableSourceIdxs", toString(allowableSourceIdxs), sep="="))
#          print(paste("allowableSourceWeights", toString(allowableSourceWeights), sep="="))
        
        if (length(allowableSources) > 0) {
          #   allowable swap exists
          #   update capacity of drawn unsaturated target node with allowable swap
          curTargetInDeg[drawnIdx] <- curTargetInDeg[drawnIdx] + 1
          unsatTargetInDegIdx <- which(curTargetInDeg < targetInDegNew)
          curTargetCapacities <-  targetInDegNew - curTargetInDeg
          unsatTargetInDegIdxCapacities <- curTargetCapacities[unsatTargetInDegIdx]
          
          #   update corrected weights for sampling
          unsatTargetInDegIdxOrigWeights <- targetInDegNew[unsatTargetInDegIdx]
          samplingWeights <- unsatTargetInDegIdxCapacities
          
          if (length(allowableSources) == 1) {
            swapSourceName <- allowableSources
          }
          else {
            # weighted choice of swap sources (necessary in order to choose equally among all valid swap edges)
            swapSourceName <- sample(allowableSources, 1)
            #swapSourceName <- sample(allowableSources, 1, prob=allowableSourceWeights)
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
          e1 <- c(swapSourceName, drawnName, 0, 1)
          e2 <- c(sourceV[i], swapSourceTargetName, 0, 1)
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
    drawnIdxs <- sample(unsatTargetInDegIdx, deg)
  }
  curTargetInDeg[drawnIdxs] <- curTargetInDeg[drawnIdxs] + 1
  
#    print(paste("unsatTargetInDegIdx", toString(unsatTargetInDegIdx), sep="="))
#    print(paste("drawnIdxs", toString(drawnIdxs), sep="="))
#    print(paste("curTargetInDeg", toString(curTargetInDeg), sep="="))
  
  sNodes <- rep(sourceV[i], deg)
  tNodes <- targetV[drawnIdxs]
  newRows <- cbind(sNodes, tNodes, 0, 1)
  efOUT <- rbind(efOUT, newRows)
}
names(efOUT) <- c("V1", "V2")

print(paste(toString(countSwaps), " Swaps Performed ", sep=""))
write.table(efOUT, file=outgraphName, quote = F, sep = "\t", col.names = F, row.names = F)
