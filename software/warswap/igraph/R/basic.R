
#   IGraph R package
#   Copyright (C) 2005  Gabor Csardi <csardi@rmki.kfki.hu>
#   MTA RMKI, Konkoly-Thege Miklos st. 29-33, Budapest 1121, Hungary
#   
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#   
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc.,  51 Franklin Street, Fifth Floor, Boston, MA
#   02110-1301 USA
#
###################################################################

is.igraph <- function(graph){
  "igraph" %in% class(graph)
}

is.directed <- function(graph) {

  if (!is.igraph(graph)) {
    stop("Not a graph object")
  }
  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  .Call("R_igraph_is_directed", graph,
        PACKAGE="igraph")
}

get.edge <- function(graph, id) {

  if (!is.igraph(graph)) {
    stop("Not a graph object")
  }

  id <- as.numeric(id)
  ec <- ecount(graph)
  
  if (id < 0 || id >= ec) {
    stop("No such edge")
  }

  on.exit( .Call("R_igraph_finalizer", PACKAGE="igraph") )
  .Call("R_igraph_get_edge", graph, as.numeric(id),
        PACKAGE="igraph")
}


