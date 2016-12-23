graph.empty <- function(n=0, directed=TRUE) {
  # Argument checks
  n <- as.numeric(n)
  directed <- as.logical(directed)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_empty", n, directed,
        PACKAGE="igraph")
  res
}

vcount <- function(graph) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_vcount", graph,
        PACKAGE="igraph")
  res
}

graph.full.citation <- function(n, directed=TRUE) {
  # Argument checks
  n <- as.numeric(n)
  directed <- as.logical(directed)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_full_citation", n, directed,
        PACKAGE="igraph")
  res
}

graph.lcf <- function(n, shifts, repeats=1) {
  # Argument checks
  n <- as.numeric(n)
  shifts <- as.numeric(shifts)
  repeats <- as.numeric(repeats)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_lcf_vector", n, shifts, repeats,
        PACKAGE="igraph")
  res
}

graph.adjlist <- function(adjlist, directed=TRUE, duplicate=TRUE) {
  # Argument checks
  adjlist <- lapply(adjlist, as.numeric)
  directed <- as.logical(directed)
  duplicate <- as.logical(duplicate)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_adjlist", adjlist, directed, duplicate,
        PACKAGE="igraph")
  res
}

forest.fire.game <- function(nodes, fw.prob, bw.factor=1, ambs=1, directed=TRUE, verbose=igraph.par("verbose")) {
  # Argument checks
  nodes <- as.numeric(nodes)
  fw.prob <- as.numeric(fw.prob)
  bw.factor <- as.numeric(bw.factor)
  ambs <- as.numeric(ambs)
  directed <- as.logical(directed)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_forest_fire_game", nodes, fw.prob, bw.factor, ambs, directed, as.logical(verbose),
        PACKAGE="igraph")
  res
}

closeness.estimate <- function(graph, vids=V(graph), mode=c("out", "in", "all", "total"), cutoff) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  vids <- as.igraph.vs(graph, vids)
  mode <- switch(igraph.match.arg(mode), "out"=1, "in"=2, "all"=3, "total"=3)
  cutoff <- as.numeric(cutoff)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_closeness_estimate", graph, vids, mode, cutoff,
        PACKAGE="igraph")
  res
}

betweenness.estimate <- function(graph, vids=V(graph), directed=TRUE, cutoff, verbose=igraph.par("verbose")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  vids <- as.igraph.vs(graph, vids)
  directed <- as.logical(directed)
  cutoff <- as.numeric(cutoff)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_betweenness_estimate", graph, vids, directed, cutoff, as.logical(verbose),
        PACKAGE="igraph")
  res
}

edge.betweenness.estimate <- function(graph, directed=TRUE, cutoff) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  directed <- as.logical(directed)
  cutoff <- as.numeric(cutoff)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_edge_betweenness_estimate", graph, directed, cutoff,
        PACKAGE="igraph")
  res
}

page.rank.old <- function(graph, vids=V(graph), directed=TRUE, niter=1000, eps=0.001, damping=0.85, old=FALSE) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  vids <- as.igraph.vs(graph, vids)
  directed <- as.logical(directed)
  niter <- as.numeric(niter)
  eps <- as.numeric(eps)
  damping <- as.numeric(damping)
  old <- as.logical(old)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_pagerank_old", graph, vids, directed, niter, eps, damping, old,
        PACKAGE="igraph")
  res
}

page.rank <- function(graph, vids=V(graph), directed=TRUE, damping=0.85, weights=NULL, options=igraph.arpack.default) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  vids <- as.igraph.vs(graph, vids)
  directed <- as.logical(directed)
  damping <- as.numeric(damping)
  if (is.null(weights) && "weight" %in% list.edge.attributes(graph)) { 
  weights <- E(graph)$weight 
  } 
  if (!is.null(weights) && any(!is.na(weights))) { 
  weights <- as.numeric(weights) 
  } else { 
  weights <- NULL 
  }
  options.tmp <- igraph.arpack.default; options.tmp[ names(options) ] <- options ; options <- options.tmp

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_pagerank", graph, vids, directed, damping, weights, options,
        PACKAGE="igraph")
  res
}

path.length.hist <- function(graph, directed=TRUE, verbose=igraph.par("verbose")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  directed <- as.logical(directed)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_path_length_hist", graph, directed, as.logical(verbose),
        PACKAGE="igraph")
  res
}

is.simple <- function(graph) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_is_simple", graph,
        PACKAGE="igraph")
  res
}

evcent <- function(graph, scale=TRUE, weights=NULL, options=igraph.arpack.default) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  scale <- as.logical(scale)
  if (is.null(weights) && "weight" %in% list.edge.attributes(graph)) { 
  weights <- E(graph)$weight 
  } 
  if (!is.null(weights) && any(!is.na(weights))) { 
  weights <- as.numeric(weights) 
  } else { 
  weights <- NULL 
  }
  options.tmp <- igraph.arpack.default; options.tmp[ names(options) ] <- options ; options <- options.tmp

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_eigenvector_centrality", graph, scale, weights, options,
        PACKAGE="igraph")
  res
}

hub.score <- function(graph, scale=TRUE, options=igraph.arpack.default) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  scale <- as.logical(scale)
  options.tmp <- igraph.arpack.default; options.tmp[ names(options) ] <- options ; options <- options.tmp

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_hub_score", graph, scale, options,
        PACKAGE="igraph")
  res
}

authority.score <- function(graph, scale=TRUE, options=igraph.arpack.default) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  scale <- as.logical(scale)
  options.tmp <- igraph.arpack.default; options.tmp[ names(options) ] <- options ; options <- options.tmp

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_authority_score", graph, scale, options,
        PACKAGE="igraph")
  res
}

arpack.unpack.complex <- function(vectors, values, nev) {
  # Argument checks
  vectors <- structure(as.double(vectors), dim=dim(vectors))
  values <- structure(as.double(values), dim=dim(values))
  nev <- as.numeric(nev)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_arpack_unpack_complex", vectors, values, nev,
        PACKAGE="igraph")
  res
}

unfold.tree <- function(graph, mode=c("all", "out", "in", "total"), roots) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  mode <- switch(igraph.match.arg(mode), "out"=1, "in"=2, "all"=3, "total"=3)
  roots <- as.numeric(roots)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_unfold_tree", graph, mode, roots,
        PACKAGE="igraph")
  res
}

is.mutual <- function(graph, es=E(graph)) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  es <- as.igraph.es(es)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_is_mutual", graph, es,
        PACKAGE="igraph")
  res
}

graph.knn <- function(graph, vids=V(graph), weights=NULL) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  vids <- as.igraph.vs(graph, vids)
  if (is.null(weights) && "weight" %in% list.edge.attributes(graph)) { 
  weights <- E(graph)$weight 
  } 
  if (!is.null(weights) && any(!is.na(weights))) { 
  weights <- as.numeric(weights) 
  } else { 
  weights <- NULL 
  }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_avg_nearest_neighbor_degree", graph, vids, weights,
        PACKAGE="igraph")
  res
}

graph.strength <- function(graph, vids=V(graph), mode=c("all", "out", "in", "total"), loops=TRUE, weights=NULL) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  vids <- as.igraph.vs(graph, vids)
  mode <- switch(igraph.match.arg(mode), "out"=1, "in"=2, "all"=3, "total"=3)
  loops <- as.logical(loops)
  if (is.null(weights) && "weight" %in% list.edge.attributes(graph)) { 
  weights <- E(graph)$weight 
  } 
  if (!is.null(weights) && any(!is.na(weights))) { 
  weights <- as.numeric(weights) 
  } else { 
  weights <- NULL 
  }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_strength", graph, vids, mode, loops, weights,
        PACKAGE="igraph")
  res
}

bipartite.projection.size <- function(graph, types=NULL) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  if (is.null(types) && "type" %in% list.vertex.attributes(graph)) { 
  types <- V(graph)$type 
  } 
  if (!is.null(types)) { 
  types <- as.logical(types) 
  } else { 
  stop("Not a bipartite graph, supply `types' argument") 
  }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_bipartite_projection_size", graph, types,
        PACKAGE="igraph")
  res
}

bipartite.projection <- function(graph, types=NULL, probe1=-1) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  if (is.null(types) && "type" %in% list.vertex.attributes(graph)) { 
  types <- V(graph)$type 
  } 
  if (!is.null(types)) { 
  types <- as.logical(types) 
  } else { 
  stop("Not a bipartite graph, supply `types' argument") 
  }
  probe1 <- as.numeric(probe1)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_bipartite_projection", graph, types, probe1,
        PACKAGE="igraph")
  res
}

is.bipartite <- function(graph) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_is_bipartite", graph,
        PACKAGE="igraph")
  res
}

clusters <- function(graph, mode=c("weak", "strong")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  mode <- switch(igraph.match.arg(mode), "weak"=1, "strong"=2)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_clusters", graph, mode,
        PACKAGE="igraph")
  res
}

articulation.points <- function(graph) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_articulation_points", graph,
        PACKAGE="igraph")
  res
}

biconnected.components <- function(graph) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_biconnected_components", graph,
        PACKAGE="igraph")
  res
}

layout.star <- function(graph, center=V(graph)[0], order=NULL) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  center <- as.igraph.vs(graph, center)
  if (!is.null(order)) order <- as.numeric(order)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_layout_star", graph, center, order,
        PACKAGE="igraph")
  res
}

similarity.jaccard <- function(graph, vids=V(graph), mode=c("all", "out", "in", "total"), loops=FALSE) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  vids <- as.igraph.vs(graph, vids)
  mode <- switch(igraph.match.arg(mode), "out"=1, "in"=2, "all"=3, "total"=3)
  loops <- as.logical(loops)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_similarity_jaccard", graph, vids, mode, loops,
        PACKAGE="igraph")
  res
}

similarity.dice <- function(graph, vids=V(graph), mode=c("all", "out", "in", "total"), loops=FALSE) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  vids <- as.igraph.vs(graph, vids)
  mode <- switch(igraph.match.arg(mode), "out"=1, "in"=2, "all"=3, "total"=3)
  loops <- as.logical(loops)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_similarity_dice", graph, vids, mode, loops,
        PACKAGE="igraph")
  res
}

similarity.invlogweighted <- function(graph, vids=V(graph), mode=c("all", "out", "in", "total")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  vids <- as.igraph.vs(graph, vids)
  mode <- switch(igraph.match.arg(mode), "out"=1, "in"=2, "all"=3, "total"=3)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_similarity_inverse_log_weighted", graph, vids, mode,
        PACKAGE="igraph")
  res
}

community.le.to.membership <- function(merges, steps, membership) {
  # Argument checks
  merges <- structure(as.double(merges), dim=dim(merges))
  steps <- as.numeric(steps)
  membership <- as.numeric(membership)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_le_community_to_membership", merges, steps, membership,
        PACKAGE="igraph")
  res
}

modularity <- function(graph, membership, weights=NULL) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  membership <- as.numeric(membership)
  if (!is.null(weights)) weights <- as.numeric(weights)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_modularity", graph, membership, weights,
        PACKAGE="igraph")
  res
}

leading.eigenvector.community <- function(graph, steps=-1, options=igraph.arpack.default) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  steps <- as.numeric(steps)
  options.tmp <- igraph.arpack.default; options.tmp[ names(options) ] <- options ; options <- options.tmp

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_community_leading_eigenvector", graph, steps, options,
        PACKAGE="igraph")
  class(res) <- "igraph.eigenc"
  res
}

leading.eigenvector.community.naive <- function(graph, steps=-1, options=igraph.arpack.default) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  steps <- as.numeric(steps)
  options.tmp <- igraph.arpack.default; options.tmp[ names(options) ] <- options ; options <- options.tmp

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_community_leading_eigenvector_naive", graph, steps, options,
        PACKAGE="igraph")
  class(res) <- "igraph.eigenc"
  res
}

label.propagation.community <- function(graph, weights=NULL, initial=NULL, fixed=NULL) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  if (is.null(weights) && "weight" %in% list.edge.attributes(graph)) { 
  weights <- E(graph)$weight 
  } 
  if (!is.null(weights) && any(!is.na(weights))) { 
  weights <- as.numeric(weights) 
  } else { 
  weights <- NULL 
  }
  if (!is.null(initial)) initial <- as.numeric(initial)
  if (!is.null(fixed)) fixed <- as.logical(fixed)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_community_label_propagation", graph, weights, initial, fixed,
        PACKAGE="igraph")
  res
}

dyad.census <- function(graph) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_dyad_census", graph,
        PACKAGE="igraph")
  res
}

triad.census <- function(graph) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_triad_census", graph,
        PACKAGE="igraph")
  res
}

graph.isoclass <- function(graph) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_isoclass", graph,
        PACKAGE="igraph")
  res
}

graph.isomorphic <- function(graph1, graph2) {
  # Argument checks
  if (!is.igraph(graph1)) { stop("Not a graph object") }
  if (!is.igraph(graph2)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_isomorphic", graph1, graph2,
        PACKAGE="igraph")
  res
}

graph.isoclass.subgraph <- function(graph, vids) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  vids <- as.numeric(vids)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_isoclass_subgraph", graph, vids,
        PACKAGE="igraph")
  res
}

graph.isocreate <- function(size, number, directed=TRUE) {
  # Argument checks
  size <- as.numeric(size)
  number <- as.numeric(number)
  directed <- as.logical(directed)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_isoclass_create", size, number, directed,
        PACKAGE="igraph")
  res
}

graph.isomorphic.vf2 <- function(graph1, graph2) {
  # Argument checks
  if (!is.igraph(graph1)) { stop("Not a graph object") }
  if (!is.igraph(graph2)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_isomorphic_vf2", graph1, graph2,
        PACKAGE="igraph")
  res
}

graph.count.isomorphisms.vf2 <- function(graph1, graph2) {
  # Argument checks
  if (!is.igraph(graph1)) { stop("Not a graph object") }
  if (!is.igraph(graph2)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_count_isomorphisms_vf2", graph1, graph2,
        PACKAGE="igraph")
  res
}

graph.get.isomorphisms.vf2 <- function(graph1, graph2) {
  # Argument checks
  if (!is.igraph(graph1)) { stop("Not a graph object") }
  if (!is.igraph(graph2)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_get_isomorphisms_vf2", graph1, graph2,
        PACKAGE="igraph")
  res
}

graph.subisomorphic.vf2 <- function(graph1, graph2) {
  # Argument checks
  if (!is.igraph(graph1)) { stop("Not a graph object") }
  if (!is.igraph(graph2)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_subisomorphic_vf2", graph1, graph2,
        PACKAGE="igraph")
  res
}

graph.count.subisomorphisms.vf2 <- function(graph1, graph2) {
  # Argument checks
  if (!is.igraph(graph1)) { stop("Not a graph object") }
  if (!is.igraph(graph2)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_count_subisomorphisms_vf2", graph1, graph2,
        PACKAGE="igraph")
  res
}

graph.get.subisomorphisms.vf2 <- function(graph1, graph2) {
  # Argument checks
  if (!is.igraph(graph1)) { stop("Not a graph object") }
  if (!is.igraph(graph2)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_get_subisomorphisms_vf2", graph1, graph2,
        PACKAGE="igraph")
  res
}

graph.isomorphic.34 <- function(graph1, graph2) {
  # Argument checks
  if (!is.igraph(graph1)) { stop("Not a graph object") }
  if (!is.igraph(graph2)) { stop("Not a graph object") }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_isomorphic_34", graph1, graph2,
        PACKAGE="igraph")
  res
}

canonical.permutation <- function(graph, sh="fm") {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  sh <- switch(igraph.match.arg(sh), "f"=0, "fl"=1, "fs"=2, "fm"=3, "flm"=4, "fsm"=5)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_canonical_permutation", graph, sh,
        PACKAGE="igraph")
  res
}

permute.vertices <- function(graph, permutation) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  permutation <- as.numeric(permutation)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_permute_vertices", graph, permutation,
        PACKAGE="igraph")
  res
}

graph.isomorphic.bliss <- function(graph1, graph2, sh1="fm", sh2="fm") {
  # Argument checks
  if (!is.igraph(graph1)) { stop("Not a graph object") }
  if (!is.igraph(graph2)) { stop("Not a graph object") }
  sh1 <- switch(igraph.match.arg(sh1), "f"=0, "fl"=1, "fs"=2, "fm"=3, "flm"=4, "fsm"=5)
  sh2 <- switch(igraph.match.arg(sh2), "f"=0, "fl"=1, "fs"=2, "fm"=3, "flm"=4, "fsm"=5)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_isomorphic_bliss", graph1, graph2, sh1, sh2,
        PACKAGE="igraph")
  res
}

graph.automorphisms <- function(graph, sh="fm") {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  sh <- switch(igraph.match.arg(sh), "f"=0, "fl"=1, "fs"=2, "fm"=3, "flm"=4, "fsm"=5)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_automorphisms", graph, sh,
        PACKAGE="igraph")
  res
}

revolver.ml.d <- function(graph, niter, delta=1e-10, filter=NULL, verbose=igraph.par("verbose")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  niter <- as.numeric(niter)
  delta <- as.numeric(delta)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_d", graph, niter, delta, filter, as.logical(verbose),
        PACKAGE="igraph")
  res
}

revolver.probs.d <- function(graph, kernel, ntk=FALSE) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  kernel <- as.numeric(kernel)
  ntk <- as.logical(ntk)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_probs_d", graph, kernel, ntk,
        PACKAGE="igraph")
  res
}

revolver.ml.de <- function(graph, niter, cats, delta=1e-10, filter=NULL, verbose=igraph.par("verbose")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  niter <- as.numeric(niter)
  cats <- as.numeric(cats)
  delta <- as.numeric(delta)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_de", graph, niter, cats, delta, filter, as.logical(verbose),
        PACKAGE="igraph")
  res
}

revolver.probs.de <- function(graph, kernel, cats) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  kernel <- structure(as.double(kernel), dim=dim(kernel))
  cats <- as.numeric(cats)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_probs_de", graph, kernel, cats,
        PACKAGE="igraph")
  res
}

revolver.ml.ade <- function(graph, niter, cats, agebins=300, delta=1e-10, filter=NULL, verbose=igraph.par("verbose")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  niter <- as.numeric(niter)
  cats <- as.numeric(cats)
  agebins <- as.numeric(agebins)
  delta <- as.numeric(delta)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_ade", graph, niter, cats, agebins, delta, filter, as.logical(verbose),
        PACKAGE="igraph")
  res
}

revolver.probs.ade <- function(graph, kernel, cats) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  kernel <- structure(as.double(kernel), dim=dim(kernel))
  cats <- as.numeric(cats)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_probs_ade", graph, kernel, cats,
        PACKAGE="igraph")
  res
}

revolver.ml.f <- function(graph, niter, delta=1e-10, verbose=igraph.par("verbose")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  niter <- as.numeric(niter)
  delta <- as.numeric(delta)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_f", graph, niter, delta, as.logical(verbose),
        PACKAGE="igraph")
  res
}

revolver.ml.df <- function(graph, niter, delta=1e-10, verbose=igraph.par("verbose")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  niter <- as.numeric(niter)
  delta <- as.numeric(delta)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_df", graph, niter, delta, as.logical(verbose),
        PACKAGE="igraph")
  res
}

revolver.ml.l <- function(graph, niter, agebins=300, delta=1e-10) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  niter <- as.numeric(niter)
  agebins <- as.numeric(agebins)
  delta <- as.numeric(delta)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_l", graph, niter, agebins, delta,
        PACKAGE="igraph")
  res
}

revolver.ml.ad <- function(graph, niter, agebins=300, delta=1e-10, filter=NULL, verbose=igraph.par("verbose")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  niter <- as.numeric(niter)
  agebins <- as.numeric(agebins)
  delta <- as.numeric(delta)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_ad", graph, niter, agebins, delta, filter, as.logical(verbose),
        PACKAGE="igraph")
  res
}

revolver.probs.ad <- function(graph, kernel, ntk=FALSE) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  kernel <- structure(as.double(kernel), dim=dim(kernel))
  ntk <- as.logical(ntk)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_probs_ad", graph, kernel, ntk,
        PACKAGE="igraph")
  res
}

revolver.ml.D.alpha <- function(graph, alpha, abstol=1e-8, reltol=1e-8, maxit=1000, filter=NULL, verbose=igraph.par("verbose")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  alpha <- as.numeric(alpha)
  abstol <- as.numeric(abstol)
  reltol <- as.numeric(reltol)
  maxit <- as.integer(maxit)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_D_alpha", graph, alpha, abstol, reltol, maxit, filter, as.logical(verbose),
        PACKAGE="igraph")
  res
}

revolver.ml.D.alpha.a <- function(graph, alpha, a, abstol=1e-8, reltol=1e-8, maxit=1000, filter=NULL, verbose=igraph.par("verbose")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  alpha <- as.numeric(alpha)
  a <- as.numeric(a)
  abstol <- as.numeric(abstol)
  reltol <- as.numeric(reltol)
  maxit <- as.integer(maxit)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_D_alpha_a", graph, alpha, a, abstol, reltol, maxit, filter, as.logical(verbose),
        PACKAGE="igraph")
  res
}

revolver.ml.DE.alpha.a <- function(graph, cats, alpha, a, coeffs, abstol=1e-8, reltol=1e-8, maxit=1000, filter=NULL) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  cats <- as.numeric(cats)
  alpha <- as.numeric(alpha)
  a <- as.numeric(a)
  coeffs <- as.numeric(coeffs)
  abstol <- as.numeric(abstol)
  reltol <- as.numeric(reltol)
  maxit <- as.integer(maxit)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_DE_alpha_a", graph, cats, alpha, a, coeffs, abstol, reltol, maxit, filter,
        PACKAGE="igraph")
  res
}

revolver.ml.AD.alpha.a.beta <- function(graph, alpha, a, beta, abstol=1e-8, reltol=1e-8, maxit=1000, agebins=300, filter=NULL, verbose=igraph.par("verbose")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  alpha <- as.numeric(alpha)
  a <- as.numeric(a)
  beta <- as.numeric(beta)
  abstol <- as.numeric(abstol)
  reltol <- as.numeric(reltol)
  maxit <- as.integer(maxit)
  agebins <- as.integer(agebins)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_AD_alpha_a_beta", graph, alpha, a, beta, abstol, reltol, maxit, agebins, filter, as.logical(verbose),
        PACKAGE="igraph")
  res
}

revolver.ml.AD.dpareto <- function(graph, alpha, a, paralpha, parbeta, parscale, abstol=1e-8, reltol=1e-8, maxit=1000, agebins=300, filter=NULL, verbose=igraph.par("verbose")) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  alpha <- as.numeric(alpha)
  a <- as.numeric(a)
  paralpha <- as.numeric(paralpha)
  parbeta <- as.numeric(parbeta)
  parscale <- as.numeric(parscale)
  abstol <- as.numeric(abstol)
  reltol <- as.numeric(reltol)
  maxit <- as.integer(maxit)
  agebins <- as.integer(agebins)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_AD_dpareto", graph, alpha, a, paralpha, parbeta, parscale, abstol, reltol, maxit, agebins, filter, as.logical(verbose),
        PACKAGE="igraph")
  res
}

revolver.ml.AD.dpareto.eval <- function(graph, alpha, a, paralpha, parbeta, parscale, agebins=300, filter=NULL) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  alpha <- as.numeric(alpha)
  a <- as.numeric(a)
  paralpha <- as.numeric(paralpha)
  parbeta <- as.numeric(parbeta)
  parscale <- as.numeric(parscale)
  agebins <- as.integer(agebins)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_AD_dpareto_eval", graph, alpha, a, paralpha, parbeta, parscale, agebins, filter,
        PACKAGE="igraph")
  res
}

revolver.ml.ADE.alpha.a.beta <- function(graph, cats, alpha, a, beta, coeffs, abstol=1e-8, reltol=1e-8, maxit=1000, agebins=300, filter=NULL) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  cats <- as.numeric(cats)
  alpha <- as.numeric(alpha)
  a <- as.numeric(a)
  beta <- as.numeric(beta)
  coeffs <- as.numeric(coeffs)
  abstol <- as.numeric(abstol)
  reltol <- as.numeric(reltol)
  maxit <- as.integer(maxit)
  agebins <- as.integer(agebins)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_ADE_alpha_a_beta", graph, cats, alpha, a, beta, coeffs, abstol, reltol, maxit, agebins, filter,
        PACKAGE="igraph")
  res
}

revolver.ml.ADE.dpareto <- function(graph, cats, alpha, a, paralpha, parbeta, parscale, coeffs, abstol=1e-8, reltol=1e-8, maxit=1000, agebins=300, filter=NULL) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  cats <- as.numeric(cats)
  alpha <- as.numeric(alpha)
  a <- as.numeric(a)
  paralpha <- as.numeric(paralpha)
  parbeta <- as.numeric(parbeta)
  parscale <- as.numeric(parscale)
  coeffs <- as.numeric(coeffs)
  abstol <- as.numeric(abstol)
  reltol <- as.numeric(reltol)
  maxit <- as.integer(maxit)
  agebins <- as.integer(agebins)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_ADE_dpareto", graph, cats, alpha, a, paralpha, parbeta, parscale, coeffs, abstol, reltol, maxit, agebins, filter,
        PACKAGE="igraph")
  res
}

revolver.ml.ADE.dpareto.eval <- function(graph, cats, alpha, a, paralpha, parbeta, parscale, coeffs, agebins=300, filter=NULL) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  cats <- as.numeric(cats)
  alpha <- as.numeric(alpha)
  a <- as.numeric(a)
  paralpha <- as.numeric(paralpha)
  parbeta <- as.numeric(parbeta)
  parscale <- as.numeric(parscale)
  coeffs <- as.numeric(coeffs)
  agebins <- as.integer(agebins)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_ADE_dpareto_eval", graph, cats, alpha, a, paralpha, parbeta, parscale, coeffs, agebins, filter,
        PACKAGE="igraph")
  res
}

revolver.ml.ADE.dpareto.evalf <- function(graph, cats, par, agebins, filter=NULL) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  cats <- as.numeric(cats)
  par <- structure(as.double(par), dim=dim(par))
  agebins <- as.integer(agebins)
  if (!is.null(filter)) filter <- as.numeric(filter)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_ml_ADE_dpareto_evalf", graph, cats, par, agebins, filter,
        PACKAGE="igraph")
  res
}

revolver.probs.ADE.dpareto <- function(graph, par, cats, gcats, agebins) {
  # Argument checks
  if (!is.igraph(graph)) { stop("Not a graph object") }
  par <- structure(as.double(par), dim=dim(par))
  cats <- as.numeric(cats)
  gcats <- as.numeric(gcats)
  agebins <- as.integer(agebins)

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  # Function call
  res <- .Call("R_igraph_revolver_probs_ADE_dpareto", graph, par, cats, gcats, agebins,
        PACKAGE="igraph")
  res
}

