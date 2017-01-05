/* -*- mode: C -*-  */
/* 
   IGraph library.
   Copyright (C) 2007  Gabor Csardi <csardi@rmki.kfki.hu>
   MTA RMKI, Konkoly-Thege Miklos st. 29-33, Budapest 1121, Hungary
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc.,  51 Franklin Street, Fifth Floor, Boston, MA 
   02110-1301 USA

*/

#include "types.h"
#include "config.h"

#define BASE_IGRAPH_REAL
#include "igraph_pmt.h"
#include "vector.pmt"
#include "igraph_pmt_off.h"
#undef BASE_IGRAPH_REAL

#define BASE_LONG
#include "igraph_pmt.h"
#include "vector.pmt"
#include "igraph_pmt_off.h"
#undef BASE_LONG

#define BASE_CHAR
#include "igraph_pmt.h"
#include "vector.pmt"
#include "igraph_pmt_off.h"
#undef BASE_CHAR

#define BASE_BOOL
#include "igraph_pmt.h"
#include "vector.pmt"
#include "igraph_pmt_off.h"
#undef BASE_BOOL

int igraph_vector_order2(igraph_vector_t *v) {

  igraph_indheap_t heap;
  
  igraph_indheap_init_array(&heap, VECTOR(*v), igraph_vector_size(v));
  IGRAPH_FINALLY(igraph_indheap_destroy, &heap);

  igraph_vector_clear(v);
  while (!igraph_indheap_empty(&heap)) {
    IGRAPH_CHECK(igraph_vector_push_back(v, igraph_indheap_max_index(&heap)-1));
    igraph_indheap_delete_max(&heap);
  }
  
  igraph_indheap_destroy(&heap);
  IGRAPH_FINALLY_CLEAN(1);
  return 0;
}

/**
 * \ingroup vector
 * \function igraph_vector_order
 * \brief Calculate the order of the elements in a vector.
 *
 * </para><para>
 * The smallest element will have order zero, the second smallest
 * order one, etc. 
 * \param v The original \type igraph_vector_t object.
 * \param res An initialized \type igraph_vector_t object, it will be
 *    resized to match the size of \p v. The
 *    result of the computation will be stored here.
 * \param nodes Hint, the largest element in \p v.
 * \return Error code:
 *         \c IGRAPH_ENOMEM: out of memory
 * 
 * Time complexity: O()
 */

int igraph_vector_order(const igraph_vector_t* v, 
			const igraph_vector_t *v2,
			igraph_vector_t* res, igraph_real_t nodes) {
  long int edges=igraph_vector_size(v);
  igraph_vector_t ptr;
  igraph_vector_t rad;
  long int i, j;

  assert(v!=NULL);
  assert(v->stor_begin != NULL);

  IGRAPH_VECTOR_INIT_FINALLY(&ptr, nodes+1);
  IGRAPH_VECTOR_INIT_FINALLY(&rad, edges);
  IGRAPH_CHECK(igraph_vector_resize(res, edges));
  
  for (i=0; i<edges; i++) {
    long int radix=v2->stor_begin[i];
    if (VECTOR(ptr)[radix]!=0) {
      VECTOR(rad)[i]=VECTOR(ptr)[radix];
    }
    VECTOR(ptr)[radix]=i+1;
  }  

  j=0;
  for (i=0; i<nodes+1; i++) {
    if (VECTOR(ptr)[i] != 0) {
      long int next=VECTOR(ptr)[i]-1;
      res->stor_begin[j++]=next;
      while (VECTOR(rad)[next] != 0) {
	next=VECTOR(rad)[next]-1;
	res->stor_begin[j++]=next;
      }
    }
  }

  igraph_vector_null(&ptr);
  igraph_vector_null(&rad);

  for (i=0; i<edges; i++) {
    long int edge=VECTOR(*res)[edges-i-1];
    long int radix=VECTOR(*v)[edge];
    if (VECTOR(ptr)[radix]!= 0) {
      VECTOR(rad)[edge]=VECTOR(ptr)[radix];
    }
    VECTOR(ptr)[radix]=edge+1;
  }
  
  j=0;
  for (i=0; i<nodes+1; i++) {
    if (VECTOR(ptr)[i] != 0) {
      long int next=VECTOR(ptr)[i]-1;
      res->stor_begin[j++]=next;
      while (VECTOR(rad)[next] != 0) {
	next=VECTOR(rad)[next]-1;
	res->stor_begin[j++]=next;
      }
    }
  } 
  
  igraph_vector_destroy(&ptr);
  igraph_vector_destroy(&rad);
  IGRAPH_FINALLY_CLEAN(2);
  
  return 0;
}

int igraph_vector_order1(const igraph_vector_t* v,
			 igraph_vector_t* res, igraph_real_t nodes) {
  long int edges=igraph_vector_size(v);
  igraph_vector_t ptr;
  igraph_vector_t rad;
  long int i, j;

  assert(v!=NULL);
  assert(v->stor_begin != NULL);

  IGRAPH_VECTOR_INIT_FINALLY(&ptr, nodes+1);
  IGRAPH_VECTOR_INIT_FINALLY(&rad, edges);
  IGRAPH_CHECK(igraph_vector_resize(res, edges));
  
  for (i=0; i<edges; i++) {
    long int radix=v->stor_begin[i];
    if (VECTOR(ptr)[radix]!=0) {
      VECTOR(rad)[i]=VECTOR(ptr)[radix];
    }
    VECTOR(ptr)[radix]=i+1;
  }
  
  j=0;
  for (i=0; i<nodes+1; i++) {
    if (VECTOR(ptr)[i] != 0) {
      long int next=VECTOR(ptr)[i]-1;
      res->stor_begin[j++]=next;
      while (VECTOR(rad)[next] != 0) {
	next=VECTOR(rad)[next]-1;
	res->stor_begin[j++]=next;
      }
    }
  }
  
  igraph_vector_destroy(&ptr);
  igraph_vector_destroy(&rad);
  IGRAPH_FINALLY_CLEAN(2);
  
  return 0;
}

int igraph_vector_rank(const igraph_vector_t *v, igraph_vector_t *res,
		       long int nodes) {
  
  igraph_vector_t rad;
  igraph_vector_t ptr;
  long int edges = igraph_vector_size(v);
  long int i, c=0;
  
  IGRAPH_VECTOR_INIT_FINALLY(&rad, nodes);
  IGRAPH_VECTOR_INIT_FINALLY(&ptr, edges);
  IGRAPH_CHECK(igraph_vector_resize(res, edges));
	       
  for (i=0; i<edges; i++) {
    long int elem=VECTOR(*v)[i];
    VECTOR(ptr)[i] = VECTOR(rad)[elem];
    VECTOR(rad)[elem] = i+1;
  }
  
  for (i=0; i<nodes; i++) {
    long int p=VECTOR(rad)[i];
    while (p != 0) {      
      VECTOR(*res)[p-1]=c++;
      p=VECTOR(ptr)[p-1];
    }
  }

  igraph_vector_destroy(&ptr);
  igraph_vector_destroy(&rad);
  IGRAPH_FINALLY_CLEAN(2);
  return 0;
}

