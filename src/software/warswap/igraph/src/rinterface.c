/* -*- mode: C -*-  */
/* 
   IGraph library R interface.
   Copyright (C) 2005  Gabor Csardi <csardi@rmki.kfki.hu>
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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
   02110-1301 USA 

*/

#include "igraph.h"
#include "error.h"

#include "config.h"

#define USE_RINTERNALS
#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

#include <stdio.h>

int igraph_free(void *p);

SEXP R_igraph_vector_to_SEXP(igraph_vector_t *v);
SEXP R_igraph_vector_bool_to_SEXP(igraph_vector_bool_t *v);
SEXP R_igraph_0orvector_bool_to_SEXP(igraph_vector_bool_t *v);
SEXP R_igraph_matrix_to_SEXP(igraph_matrix_t *m);
SEXP R_igraph_strvector_to_SEXP(const igraph_strvector_t *m);
SEXP R_igraph_to_SEXP(igraph_t *graph);
SEXP R_igraph_vectorlist_to_SEXP(const igraph_vector_ptr_t *ptr);
void R_igraph_vectorlist_destroy(igraph_vector_ptr_t *ptr);

int R_igraph_SEXP_to_strvector(SEXP rval, igraph_strvector_t *sv);
int R_igraph_SEXP_to_strvector_copy(SEXP rval, igraph_strvector_t *sv);
int R_SEXP_to_vector(SEXP sv, igraph_vector_t *v);
int R_SEXP_to_vector_copy(SEXP sv, igraph_vector_t *v);
int R_SEXP_to_matrix(SEXP pakl, igraph_matrix_t *akl);
int R_SEXP_to_igraph_matrix_copy(SEXP pakl, igraph_matrix_t *akl);
int R_SEXP_to_igraph(SEXP graph, igraph_t *res);
int R_SEXP_to_igraph_copy(SEXP graph, igraph_t *res);
int R_SEXP_to_igraph_vs(SEXP rit, igraph_t *graph, igraph_vs_t *it);
int R_SEXP_to_igraph_es(SEXP rit, igraph_t *graph, igraph_es_t *it);
int R_SEXP_to_igraph_adjlist(SEXP vectorlist, igraph_adjlist_t *ptr);
int R_SEXP_to_vector_bool(SEXP sv, igraph_vector_bool_t *v);

/* get the list element named str, or return NULL */
/* from the R Manual */

SEXP R_igraph_getListElement(SEXP list, const char *str)
{
  SEXP elmt = R_NilValue, names = getAttrib(list, R_NamesSymbol);
  int i;
  
  for (i = 0; i < length(list); i++)
    if(strcmp(CHAR(STRING_ELT(names, i)), str) == 0) {
      elmt = VECTOR_ELT(list, i);
      break;
    }
  return elmt;
}

/******************************************************
 * Attributes                                         *
 *****************************************************/

int R_igraph_attribute_init(igraph_t *graph, igraph_vector_ptr_t *attr) {
  SEXP result, names, gal;
  long int i;
  long int attrno;
  PROTECT(result=NEW_LIST(4));
  SET_VECTOR_ELT(result, 0, NEW_NUMERIC(2));
  REAL(VECTOR_ELT(result, 0))[0]=0; /* R objects */
  REAL(VECTOR_ELT(result, 0))[1]=1; /* igraph_t objects */
  for (i=1; i<3; i++) {
    SET_VECTOR_ELT(result, i+1, NEW_LIST(0)); /* gal, val, eal */
  }
  graph->attr=result;

  /* Add graph attributes */
  attrno= attr==NULL ? 0 : igraph_vector_ptr_size(attr);
  SET_VECTOR_ELT(result, 1, NEW_LIST(attrno));
  gal=VECTOR_ELT(result, 1);
  PROTECT(names=NEW_CHARACTER(attrno));
  for (i=0; i<attrno; i++) {
    igraph_i_attribute_record_t *rec=VECTOR(*attr)[i];
    igraph_vector_t *vec;
    igraph_strvector_t *strvec;
    SET_STRING_ELT(names, i, mkChar(rec->name));
    SET_VECTOR_ELT(gal, i, R_NilValue);
    switch (rec->type) {
    case IGRAPH_ATTRIBUTE_NUMERIC:
      vec=(igraph_vector_t*) rec->value;
      if (igraph_vector_size(vec) > 0) {
	SET_VECTOR_ELT(gal, i, NEW_NUMERIC(1));
	REAL(VECTOR_ELT(gal, i))[0]=VECTOR(*vec)[0];
      }
      break;
    case IGRAPH_ATTRIBUTE_STRING:
      strvec=(igraph_strvector_t*) rec->value;
      if (igraph_strvector_size(strvec) > 0) {
	SET_VECTOR_ELT(gal, i, NEW_CHARACTER(1));
	SET_STRING_ELT(VECTOR_ELT(gal,i), 0, mkChar(STR(*strvec, 0)));
      }
      break;
    case IGRAPH_ATTRIBUTE_R_OBJECT:
      IGRAPH_ERROR("R_objects not implemented yet", IGRAPH_UNIMPLEMENTED);
      break;
    }
  }
  SET_NAMES(gal, names);
  
  UNPROTECT(1);
  return 0;
}

void R_igraph_attribute_destroy(igraph_t *graph) {
  SEXP attr=graph->attr;
  REAL(VECTOR_ELT(attr, 0))[1] -= 1; /* refcount for igraph_t */
  if (REAL(VECTOR_ELT(attr, 0))[1]==0) {
    UNPROTECT_PTR(attr);
  }
  graph->attr=0;
}

int R_igraph_attribute_copy(igraph_t *to, const igraph_t *from,
			    igraph_bool_t ga, igraph_bool_t va, igraph_bool_t ea) {
  SEXP fromattr=from->attr;
  if (ga && va && ea) {
    to->attr=from->attr;
    REAL(VECTOR_ELT(fromattr, 0))[1] += 1; /* refcount only */
    if (REAL(VECTOR_ELT(fromattr, 0))[1] == 1) {
      PROTECT(to->attr);
    }
  } else {
    R_igraph_attribute_init(to,0); /* Sets up many things */
    SEXP toattr=to->attr;
    if (ga) {
      SET_VECTOR_ELT(toattr, 1, duplicate(VECTOR_ELT(fromattr, 1)));
    } 
    if (va) {
      SET_VECTOR_ELT(toattr, 2, duplicate(VECTOR_ELT(fromattr, 2)));      
    } 
    if (ea) {
      SET_VECTOR_ELT(toattr, 3, duplicate(VECTOR_ELT(fromattr, 3)));      
    }
  }
  return 0;
}

int R_igraph_attribute_add_vertices(igraph_t *graph, long int nv, 
				    igraph_vector_ptr_t *nattr) {
  SEXP attr=graph->attr;
  SEXP val, rep=0, names, newnames;
  igraph_vector_t news;
  long int valno, i, origlen, nattrno, newattrs;
  if (REAL(VECTOR_ELT(attr, 0))[0]+REAL(VECTOR_ELT(attr, 0))[1] > 1) {
    SEXP newattr;
    PROTECT(newattr=duplicate(attr));
    REAL(VECTOR_ELT(attr, 0))[1] -= 1;
    if (REAL(VECTOR_ELT(attr, 0))[1] == 0) {
      UNPROTECT_PTR(attr);
    }
    REAL(VECTOR_ELT(newattr, 0))[0] = 0;
    REAL(VECTOR_ELT(newattr, 0))[1] = 1;
    attr=graph->attr=newattr;
  }

  val=VECTOR_ELT(attr, 2);
  valno=GET_LENGTH(val);  
  names=GET_NAMES(val);
  if (nattr==NULL) { 
    nattrno=0;
  } else {
    nattrno=igraph_vector_ptr_size(nattr); 
  }
  origlen=igraph_vcount(graph)-nv;

  /* First add the new attributes, if any */
  newattrs=0;
  IGRAPH_VECTOR_INIT_FINALLY(&news, 0);
  for (i=0; i<nattrno; i++) {
    igraph_i_attribute_record_t *nattr_entry=VECTOR(*nattr)[i];
    const char *nname=nattr_entry->name;
    long int j; 
    igraph_bool_t l=0;
    for (j=0; !l && j<valno; j++) {
      l=!strcmp(nname, CHAR(STRING_ELT(names, j)));
    }
    if (!l) {
      newattrs++;
      IGRAPH_CHECK(igraph_vector_push_back(&news, i));
    }
  }
  if (newattrs != 0) {
    SEXP app, newval;
    PROTECT(app=NEW_LIST(newattrs));
    PROTECT(newnames=NEW_CHARACTER(newattrs));
    PROTECT(rep=EVAL(lang3(install("rep"), ScalarLogical(NA_LOGICAL), 
			   ScalarInteger(origlen))));
    for (i=0; i<newattrs; i++) {
      igraph_i_attribute_record_t *tmp=
	VECTOR(*nattr)[(long int)VECTOR(news)[i]];
      SET_VECTOR_ELT(app, i, rep);
      SET_STRING_ELT(newnames, i, CREATE_STRING_VECTOR(tmp->name));
    }
    UNPROTECT(1); 		/* rep */
    PROTECT(newval=EVAL(lang3(install("c"), val, app)));
    PROTECT(newnames=EVAL(lang3(install("c"), names, newnames)));
    SET_NAMES(newval, newnames);
    SET_VECTOR_ELT(attr, 2, newval);
    val=VECTOR_ELT(attr, 2);    
    valno=GET_LENGTH(val);  
    names=GET_NAMES(val);
    UNPROTECT(4);
    rep=0;
  }
  igraph_vector_destroy(&news);
  IGRAPH_FINALLY_CLEAN(1);	/* news */

  /* Now append the new values */
  for (i=0; i<valno; i++) {
    SEXP oldva=VECTOR_ELT(val, i), newva;
    const char *sexpname=CHAR(STRING_ELT(names,i));
    igraph_bool_t l=0;
    long int j;
    for (j=0; !l && j<nattrno; j++) {
      igraph_i_attribute_record_t *tmp=VECTOR(*nattr)[j];
      l=!strcmp(sexpname, tmp->name);
    }
    if (l) {
      /* This attribute is present in nattr */
      SEXP app=0;
      igraph_i_attribute_record_t *tmprec=VECTOR(*nattr)[j-1];
      switch (tmprec->type) {
      case IGRAPH_ATTRIBUTE_NUMERIC:
	if (nv != igraph_vector_size(tmprec->value)) {
	  IGRAPH_ERROR("Invalid attribute length", IGRAPH_EINVAL);
	}
	PROTECT(app=NEW_NUMERIC(nv));
	igraph_vector_copy_to(tmprec->value, REAL(app));
	break;
      case IGRAPH_ATTRIBUTE_STRING:
	if (nv != igraph_strvector_size(tmprec->value)) {
	  IGRAPH_ERROR("Invalid attribute length", IGRAPH_EINVAL);
	}
	PROTECT(app=R_igraph_strvector_to_SEXP(tmprec->value));
	break;
      case IGRAPH_ATTRIBUTE_R_OBJECT:
	/* TODO */
	IGRAPH_ERROR("R_objects not implemented yet", IGRAPH_UNIMPLEMENTED);
	break;
      default:
	warning("Ignoring unknown attribute type");
	break;
      }
      if (app!=0) {
	PROTECT(newva=EVAL(lang3(install("c"), oldva, app)));
	SET_VECTOR_ELT(val, i, newva);
	UNPROTECT(2);		/* app & newva */
      }
    } else {
      /* No such attribute, append NA's */
      if (rep==0) {
	PROTECT(rep=EVAL(lang3(install("rep"), ScalarLogical(NA_LOGICAL), 
			       ScalarInteger(nv))));
      }
      PROTECT(newva=EVAL(lang3(install("c"), oldva, rep)));
      SET_VECTOR_ELT(val, i, newva);
      UNPROTECT(1); 		/* newva */
    }
  }
  if (rep != 0) {
    UNPROTECT(1);
  } 
  
  return 0;
}

void R_igraph_attribute_delete_vertices(igraph_t *graph, 
					const igraph_vector_t *eidx,
					const igraph_vector_t *vidx) {
  SEXP attr=graph->attr;
  SEXP eal, val;
  long int valno, ealno, i;
  if (REAL(VECTOR_ELT(attr, 0))[0]+REAL(VECTOR_ELT(attr, 0))[1] > 1) {
    SEXP newattr;
    PROTECT(newattr=duplicate(attr));
    REAL(VECTOR_ELT(attr, 0))[1] -= 1;
    if (REAL(VECTOR_ELT(attr, 0))[1] == 0) {
      UNPROTECT_PTR(attr);
    }
    REAL(VECTOR_ELT(newattr, 0))[0] = 0;
    REAL(VECTOR_ELT(newattr, 0))[1] = 1;
    attr=graph->attr=newattr;
  }

  /* Vertices */
  val=VECTOR_ELT(attr, 2);
  valno=GET_LENGTH(val);
  for (i=0; i<valno; i++) {
    SEXP oldva=VECTOR_ELT(val, i), newva, ss;
    long int origlen=GET_LENGTH(oldva);
    long int newlen=0, j;
    for (j=0; j<igraph_vector_size(vidx); j++) {
      if (VECTOR(*vidx)[j] > 0) {
	newlen++;
      }
    }
    PROTECT(ss=NEW_NUMERIC(newlen));
    for (j=0; j<origlen; j++) {
      if (VECTOR(*vidx)[j]>0) {
	REAL(ss)[(long int)VECTOR(*vidx)[j]-1]=j+1;
      }
    }
    PROTECT(newva=EVAL(lang3(install("["), oldva, ss)));
    SET_VECTOR_ELT(val, i, newva);
    UNPROTECT(2);
  }    

  /* Edges */
  eal=VECTOR_ELT(attr, 3);
  ealno=GET_LENGTH(eal);
  for (i=0; i<ealno; i++) {
    SEXP oldea=VECTOR_ELT(eal, i), newea, ss;
    long int origlen=GET_LENGTH(oldea);
    long int newlen=0, j;
    /* calculate new length */
    for (j=0; j<origlen; j++) {
      if (VECTOR(*eidx)[j] > 0) {
	newlen++;
      }
    }    
    PROTECT(ss=NEW_NUMERIC(newlen));
    for (j=0; j<origlen; j++) {
      if (VECTOR(*eidx)[j]>0) {
	REAL(ss)[(long int)VECTOR(*eidx)[j]-1]=j+1;
      }
    }
    PROTECT(newea=EVAL(lang3(install("["), oldea, ss)));
    SET_VECTOR_ELT(eal, i, newea);
    UNPROTECT(2);
  }
}


int R_igraph_attribute_add_edges(igraph_t *graph, 
				 const igraph_vector_t *edges,
				 igraph_vector_ptr_t *nattr) {
  SEXP attr=graph->attr;
  SEXP eal, rep=0, names, newnames;
  igraph_vector_t news;
  long int ealno, i, origlen, nattrno, newattrs;  
  long int ne=igraph_vector_size(edges)/2;
  if (REAL(VECTOR_ELT(attr, 0))[0]+REAL(VECTOR_ELT(attr, 0))[1] > 1) {
    SEXP newattr;
    PROTECT(newattr=duplicate(attr));
    REAL(VECTOR_ELT(attr, 0))[1] -= 1;
    if (REAL(VECTOR_ELT(attr, 0))[1] == 0) {
      UNPROTECT_PTR(attr);
    }
    REAL(VECTOR_ELT(newattr, 0))[0] = 0;
    REAL(VECTOR_ELT(newattr, 0))[1] = 1;
    attr=graph->attr=newattr;
  }

  eal=VECTOR_ELT(attr, 3);
  ealno=GET_LENGTH(eal);
  names=GET_NAMES(eal);
  if (nattr==NULL) {
    nattrno=0; 
  } else {
    nattrno=igraph_vector_ptr_size(nattr);
  }
  origlen=igraph_ecount(graph)-ne;

  /* First add the new attributes, if any */
  newattrs=0;
  IGRAPH_VECTOR_INIT_FINALLY(&news, 0);
  for (i=0; i<nattrno; i++) {
    igraph_i_attribute_record_t *nattr_entry=VECTOR(*nattr)[i];
    const char *nname=nattr_entry->name;
    long int j;
    igraph_bool_t l=0;
    for (j=0; !l && j<ealno; j++) {
      l=!strcmp(nname, CHAR(STRING_ELT(names, j)));
    }
    if (!l) {
      newattrs++;
      IGRAPH_CHECK(igraph_vector_push_back(&news, i));
    }
  }
  if (newattrs != 0) {
    SEXP app, neweal;
    PROTECT(app=NEW_LIST(newattrs));
    PROTECT(newnames=NEW_CHARACTER(newattrs));
    PROTECT(rep=EVAL(lang3(install("rep"), ScalarLogical(NA_LOGICAL),
			   ScalarInteger(origlen))));
    for (i=0; i<newattrs; i++) {
      igraph_i_attribute_record_t *tmp=
	VECTOR(*nattr)[ (long int) VECTOR(news)[i]];
      SET_VECTOR_ELT(app, i, rep);
      SET_STRING_ELT(newnames, i, CREATE_STRING_VECTOR(tmp->name));
    }
    UNPROTECT(1);		/* rep */
    PROTECT(neweal=EVAL(lang3(install("c"), eal, app)));
    PROTECT(newnames=EVAL(lang3(install("c"), names, newnames)));
    SET_NAMES(neweal, newnames);
    SET_VECTOR_ELT(attr, 3, neweal);
    eal=VECTOR_ELT(attr, 3);
    ealno=GET_LENGTH(eal);
    names=GET_NAMES(eal);
    UNPROTECT(4);
    rep=0;
  }
  igraph_vector_destroy(&news);
  IGRAPH_FINALLY_CLEAN(1);

  /* Now append the new values */
  for (i=0; i<ealno; i++) {
    SEXP oldea=VECTOR_ELT(eal, i), newea;
    const char *sexpname=CHAR(STRING_ELT(names, i));
    igraph_bool_t l=0;
    long int j;
    for (j=0; !l && j<nattrno; j++) {
      igraph_i_attribute_record_t *tmp=VECTOR(*nattr)[j];
      l=!strcmp(sexpname, tmp->name);
    }
    if (l) {
      /* This attribute is present in nattr */
      SEXP app=0;
      igraph_i_attribute_record_t *tmprec=VECTOR(*nattr)[j-1];
      switch (tmprec->type) {
      case IGRAPH_ATTRIBUTE_NUMERIC:
	if (ne != igraph_vector_size(tmprec->value)) {
	  IGRAPH_ERROR("Invalid attribute length", IGRAPH_EINVAL);
	}
	PROTECT(app=NEW_NUMERIC(ne));
	igraph_vector_copy_to(tmprec->value, REAL(app));
	break;
      case IGRAPH_ATTRIBUTE_STRING:
	if (ne != igraph_strvector_size(tmprec->value)) {
	  IGRAPH_ERROR("Invalid attribute length", IGRAPH_EINVAL);
	}
	PROTECT(app=R_igraph_strvector_to_SEXP(tmprec->value));
	break;
      case IGRAPH_ATTRIBUTE_R_OBJECT:
	/* TODO */
	IGRAPH_ERROR("R objects not implemented yet", IGRAPH_UNIMPLEMENTED);
	break;
      default:
	warning("Ignoring unknown attribute type");
	break;
      }
      if (app!=0) {
	PROTECT(newea=EVAL(lang3(install("c"), oldea, app)));
	SET_VECTOR_ELT(eal, i, newea);
	UNPROTECT(2);		/* app & newea */
      }
    } else {
      /* No such attribute, append NA's */
      if (rep==0) {
	PROTECT(rep=EVAL(lang3(install("rep"), ScalarLogical(NA_LOGICAL),
			       ScalarInteger(ne))));
      }
      PROTECT(newea=EVAL(lang3(install("c"), oldea, rep)));
      SET_VECTOR_ELT(eal, i, newea);
      UNPROTECT(1);		/* newea */
    }
  }
  if (rep != 0) {
    UNPROTECT(1);
  }

  return 0;
}

void R_igraph_attribute_delete_edges(igraph_t *graph, 
				     const igraph_vector_t *idx) {
  SEXP attr=graph->attr;
  SEXP eal;
  long int ealno, i;
  if (REAL(VECTOR_ELT(attr, 0))[0]+REAL(VECTOR_ELT(attr, 0))[1] > 1) {
    SEXP newattr;
    PROTECT(newattr=duplicate(attr));
    REAL(VECTOR_ELT(attr, 0))[1] -= 1;
    if (REAL(VECTOR_ELT(attr, 0))[1] == 0) {
      UNPROTECT_PTR(attr);
    }
    REAL(VECTOR_ELT(newattr, 0))[0] = 0;
    REAL(VECTOR_ELT(newattr, 0))[1] = 1;
    attr=graph->attr=newattr;
  }

  eal=VECTOR_ELT(attr, 3);
  ealno=GET_LENGTH(eal);
  for (i=0; i<ealno; i++) {
    SEXP oldea=VECTOR_ELT(eal, i), newea, ss;
    long int origlen=GET_LENGTH(oldea);
    long int newlen=0, j;
    /* create subscript vector */
    for (j=0; j<origlen; j++) {
      if (VECTOR(*idx)[j] > 0) {
	newlen++;
      }
    }
    PROTECT(ss=NEW_NUMERIC(newlen));
    for (j=0; j<origlen; j++) {
      if (VECTOR(*idx)[j] > 0) {
	REAL(ss)[(long int)VECTOR(*idx)[j]-1] = j+1;
      }
    }
    PROTECT(newea=EVAL(lang3(install("["), oldea, ss)));
    SET_VECTOR_ELT(eal, i, newea);
    UNPROTECT(2);
  }
}

int R_igraph_attribute_permute_edges(igraph_t *graph,
				     const igraph_vector_t *idx) {
  SEXP attr=graph->attr;
  SEXP eal;
  long int ealno, i;
  if (REAL(VECTOR_ELT(attr, 0))[0]+REAL(VECTOR_ELT(attr, 0))[1] > 1) {
    SEXP newattr;
    PROTECT(newattr=duplicate(attr));
    REAL(VECTOR_ELT(attr, 0))[1] -= 1;
    if (REAL(VECTOR_ELT(attr, 0))[1] == 0) {
      UNPROTECT_PTR(attr);
    }
    REAL(VECTOR_ELT(newattr, 0))[0] = 0;
    REAL(VECTOR_ELT(newattr, 0))[1] = 1;
    attr=graph->attr=newattr;
  }

  eal=VECTOR_ELT(attr, 3);
  ealno=GET_LENGTH(eal);
  for (i=0; i<ealno; i++) {
    SEXP oldea=VECTOR_ELT(eal, i), newea, ss;
    long int j;
    long int newlen=igraph_vector_size(idx);
    /* create subscript vector */
    PROTECT(ss=NEW_NUMERIC(newlen));
    for (j=0; j<newlen; j++) {
      REAL(ss)[j] = VECTOR(*idx)[j];
    }
    PROTECT(newea=EVAL(lang3(install("["), oldea, ss)));
    SET_VECTOR_ELT(eal, i, newea);
    UNPROTECT(2);
  }
  return 0;
}

int R_igraph_attribute_get_info(const igraph_t *graph,
				igraph_strvector_t *gnames,
				igraph_vector_t *gtypes,
				igraph_strvector_t *vnames,
				igraph_vector_t *vtypes,
				igraph_strvector_t *enames,
				igraph_vector_t *etypes) {
  igraph_strvector_t *names[3] = { gnames, vnames, enames };
  igraph_vector_t *types[3] = { gtypes, vtypes, etypes };
  long int i, j;

  SEXP attr=graph->attr;

  for (i=0; i<3; i++) {
    igraph_strvector_t *n=names[i];
    igraph_vector_t *t=types[i];
    SEXP al=VECTOR_ELT(attr, i+1);

    if (n) {			/* return names */
      SEXP names=GET_NAMES(al);
      R_igraph_SEXP_to_strvector_copy(names, n);
    }

    if (t) {			/* return types */
      igraph_vector_resize(t, GET_LENGTH(al));
      for (j=0; j<GET_LENGTH(al); j++) {
	SEXP a=VECTOR_ELT(al, j);
	if (TYPEOF(a)==REALSXP || TYPEOF(a)==INTSXP) {
	  VECTOR(*t)[j]=IGRAPH_ATTRIBUTE_NUMERIC;
	} else if (IS_CHARACTER(a)) {
	  VECTOR(*t)[j]=IGRAPH_ATTRIBUTE_STRING;
	} else {
	  VECTOR(*t)[j]=IGRAPH_ATTRIBUTE_R_OBJECT;
	}
      }
    }
  }

  return 0;
}

igraph_bool_t R_igraph_attribute_has_attr(const igraph_t *graph,
					  igraph_attribute_elemtype_t type,
					  const char *name) {
  long int attrnum;
  SEXP res;
  
  switch (type) {
  case IGRAPH_ATTRIBUTE_GRAPH:
    attrnum=1;
    break;
  case IGRAPH_ATTRIBUTE_VERTEX:
    attrnum=2;
    break;
  case IGRAPH_ATTRIBUTE_EDGE:
    attrnum=3;
    break;
  default:
    IGRAPH_ERROR("Unkwown attribute element type", IGRAPH_EINVAL);
    break;
  }
  
  res=R_igraph_getListElement(VECTOR_ELT(graph->attr, attrnum), name);
  return res != R_NilValue;
}

int R_igraph_attribute_gettype(const igraph_t *graph,
			       igraph_attribute_type_t *type,
			       igraph_attribute_elemtype_t elemtype,
			       const char *name) {
  long int attrnum;
  SEXP res;
  
  switch (elemtype) {
  case IGRAPH_ATTRIBUTE_GRAPH:
    attrnum=1;
    break;
  case IGRAPH_ATTRIBUTE_VERTEX:
    attrnum=2;
    break;
  case IGRAPH_ATTRIBUTE_EDGE:
    attrnum=3;
    break;
  default:
    IGRAPH_ERROR("Unkwown attribute element type", IGRAPH_EINVAL);
    break;
  }
  
  res=R_igraph_getListElement(VECTOR_ELT(graph->attr, attrnum), name);
  if (IS_NUMERIC(res) || IS_INTEGER(res)) {
    *type=IGRAPH_ATTRIBUTE_NUMERIC;
  } else if (IS_CHARACTER(res)) {
    *type=IGRAPH_ATTRIBUTE_STRING;
  } else {
    *type=IGRAPH_ATTRIBUTE_R_OBJECT;
  }
  return 0;
}

int R_igraph_attribute_get_numeric_graph_attr(const igraph_t *graph,
					      const char *name, 
					      igraph_vector_t *value) {
  SEXP gal=VECTOR_ELT(graph->attr, 1);
  SEXP ga=R_igraph_getListElement(gal, name);
  
  if (ga == R_NilValue) {
    IGRAPH_ERROR("No such attribute", IGRAPH_EINVAL);
  }

  PROTECT(ga=AS_NUMERIC(ga));
  
  IGRAPH_CHECK(igraph_vector_resize(value, 1));
  VECTOR(*value)[0]=REAL(ga)[0];

  UNPROTECT(1);

  return 0;
}

int R_igraph_attribute_get_string_graph_attr(const igraph_t *graph,
					     const char *name,
					     igraph_strvector_t *value) {
  /* TODO: serialization */
  SEXP gal=VECTOR_ELT(graph->attr, 1);
  SEXP ga=R_igraph_getListElement(gal, name);
  
  if (ga == R_NilValue) {
    IGRAPH_ERROR("No such attribute", IGRAPH_EINVAL);
  }

  PROTECT(ga=AS_CHARACTER(ga));

  IGRAPH_CHECK(igraph_strvector_resize(value, 1));
  IGRAPH_CHECK(igraph_strvector_set(value, 0, CHAR(STRING_ELT(ga, 0))));

  UNPROTECT(1);

  return 0;
}

int R_igraph_attribute_get_numeric_vertex_attr(const igraph_t *graph, 
					       const char *name,
					       igraph_vs_t vs,
					       igraph_vector_t *value) {
  /* TODO: serialization */
  SEXP val=VECTOR_ELT(graph->attr, 2);
  SEXP va=R_igraph_getListElement(val, name);
  igraph_vector_t newvalue;

  if (va == R_NilValue) {
    IGRAPH_ERROR("No such attribute", IGRAPH_EINVAL);
  }
  PROTECT(va=AS_NUMERIC(va));

  if (igraph_vs_is_all(&vs)) {
    R_SEXP_to_vector_copy(va, &newvalue);
    igraph_vector_destroy(value);
    *value=newvalue;
  } else {
    igraph_vit_t it;
    long int i=0;
    IGRAPH_CHECK(igraph_vit_create(graph, vs, &it));
    IGRAPH_FINALLY(igraph_vit_destroy, &it);
    IGRAPH_CHECK(igraph_vector_resize(value, IGRAPH_VIT_SIZE(it)));
    while (!IGRAPH_VIT_END(it)) {
      long int v=IGRAPH_VIT_GET(it);
      VECTOR(*value)[i]=REAL(va)[v];
      IGRAPH_VIT_NEXT(it);
      i++;
    }
    igraph_vit_destroy(&it);
    IGRAPH_FINALLY_CLEAN(1);
  }

  UNPROTECT(1);
  return 0;
}

int R_igraph_attribute_get_string_vertex_attr(const igraph_t *graph, 
					      const char *name,
					      igraph_vs_t vs,
					      igraph_strvector_t *value) {
  /* TODO: serialization */
  SEXP val, va;

  val=VECTOR_ELT(graph->attr, 2);  
  va=R_igraph_getListElement(val, name);
  if (va == R_NilValue) {
    IGRAPH_ERROR("No such attribute", IGRAPH_EINVAL);
  }

  PROTECT(va=AS_CHARACTER(va));
  
  if (igraph_vs_is_all(&vs)) {
    R_igraph_SEXP_to_strvector_copy(va, value);
  } else {
    igraph_vit_t it;
    long int i=0;
    IGRAPH_CHECK(igraph_vit_create(graph, vs, &it));
    IGRAPH_FINALLY(igraph_vit_destroy, &it);
    IGRAPH_CHECK(igraph_strvector_resize(value, IGRAPH_VIT_SIZE(it)));
    while (!IGRAPH_VIT_END(it)) {
      long int v=IGRAPH_VIT_GET(it);
      const char *str=CHAR(STRING_ELT(va, v));
      IGRAPH_CHECK(igraph_strvector_set(value, i, str));
      IGRAPH_VIT_NEXT(it);
      i++;
    }
    igraph_vit_destroy(&it);
    IGRAPH_FINALLY_CLEAN(1);
  }

  UNPROTECT(1);

  return 0;
}

int R_igraph_attribute_get_numeric_edge_attr(const igraph_t *graph,
					     const char *name,
					     igraph_es_t es,
					     igraph_vector_t *value) {
  /* TODO: serialization */
  SEXP eal=VECTOR_ELT(graph->attr, 3);
  SEXP ea=R_igraph_getListElement(eal, name);
  igraph_vector_t newvalue;

  if (ea == R_NilValue) {
    IGRAPH_ERROR("No such attribute", IGRAPH_EINVAL);
  }
  PROTECT(ea=AS_NUMERIC(ea));
  
  if (igraph_es_is_all(&es)) {    
    R_SEXP_to_vector_copy(AS_NUMERIC(ea), &newvalue);
    igraph_vector_destroy(value);
    *value=newvalue;
  } else {
    igraph_eit_t it;
    long int i=0;
    IGRAPH_CHECK(igraph_eit_create(graph, es, &it));
    IGRAPH_FINALLY(igraph_eit_destroy, &it);
    IGRAPH_CHECK(igraph_vector_resize(value, IGRAPH_EIT_SIZE(it)));
    while (!IGRAPH_EIT_END(it)) {
      long int e=IGRAPH_EIT_GET(it);
      VECTOR(*value)[i]=REAL(ea)[e];
      IGRAPH_EIT_NEXT(it);
      i++;
    }
    igraph_eit_destroy(&it);
    IGRAPH_FINALLY_CLEAN(1);
  }

  UNPROTECT(1);
  return 0;
}

int R_igraph_attribute_get_string_edge_attr(const igraph_t *graph,
					    const char *name,
					    igraph_es_t es,
					    igraph_strvector_t *value) {
  /* TODO: serialization */
  SEXP eal=VECTOR_ELT(graph->attr, 3);
  SEXP ea=R_igraph_getListElement(eal, name);
  
  if (ea == R_NilValue) {
    IGRAPH_ERROR("No such attribute", IGRAPH_EINVAL);
  }

  PROTECT(ea=AS_CHARACTER(ea));
  
  if (igraph_es_is_all(&es)) {
    R_igraph_SEXP_to_strvector_copy(ea, value);
  } else {
    igraph_eit_t it;
    long int i=0;
    IGRAPH_CHECK(igraph_eit_create(graph, es, &it));
    IGRAPH_FINALLY(igraph_eit_destroy, &it);
    IGRAPH_CHECK(igraph_strvector_resize(value, IGRAPH_EIT_SIZE(it)));
    while (!IGRAPH_EIT_END(it)) {
      long int e=IGRAPH_EIT_GET(it);
      const char *str=CHAR(STRING_ELT(ea, e));
      IGRAPH_CHECK(igraph_strvector_set(value, i, str));
      IGRAPH_EIT_NEXT(it);
      i++;
    }
    igraph_eit_destroy(&it);
    IGRAPH_FINALLY_CLEAN(1);
  }
  
  UNPROTECT(1);
  
  return 0;
}

igraph_attribute_table_t R_igraph_attribute_table={
  &R_igraph_attribute_init, &R_igraph_attribute_destroy,
  &R_igraph_attribute_copy, &R_igraph_attribute_add_vertices,
  &R_igraph_attribute_delete_vertices, &R_igraph_attribute_add_edges,
  &R_igraph_attribute_delete_edges, &R_igraph_attribute_permute_edges,
  &R_igraph_attribute_get_info,
  &R_igraph_attribute_has_attr, &R_igraph_attribute_gettype,
  &R_igraph_attribute_get_numeric_graph_attr,
  &R_igraph_attribute_get_string_graph_attr,
  &R_igraph_attribute_get_numeric_vertex_attr,
  &R_igraph_attribute_get_string_vertex_attr,
  &R_igraph_attribute_get_numeric_edge_attr,
  &R_igraph_attribute_get_string_edge_attr
};

igraph_attribute_table_t *R_igraph_attribute_oldtable;

/******************************************************
 * things to do before and after                      *
 * calling an interface function                      *
 *****************************************************/

igraph_error_handler_t *R_igraph_oldhandler;

void R_igraph_myhandler (const char *reason, const char *file,
			 int line, int igraph_errno) {
  IGRAPH_FINALLY_FREE();
  error("At %s:%i : %s, %s", file, line, reason, 
	igraph_strerror(igraph_errno));
}

void R_igraph_warning_handler(const char *reason, const char *file,
			      int line, int igraph_errno) {
  warning("At %s:%i :%s", file, line, reason);
}

igraph_interruption_handler_t *R_igraph_oldinterrupt;
igraph_progress_handler_t *R_igraph_oldprogress;
igraph_warning_handler_t *R_igraph_oldwarning;

extern int R_interrupts_pending;

int R_igraph_interrupt_handler(void *data) {
#if  ( defined(HAVE_AQUA) || defined(Win32) )
  /* TODO!!!: proper interrupt handling for these systems, 
     right now memory is not deallocated!!! */
  R_CheckUserInterrupt();
#else
  if (R_interrupts_pending) {
    IGRAPH_FINALLY_FREE();
    R_CheckUserInterrupt();
  }
#endif
  return 0;
}

int R_igraph_progress_handler(const char *message, igraph_real_t percent,
			      void * data) {
  static igraph_real_t last=0;
  if (percent == 0) {
    int i, l=strlen(message);
    last=0;
    fprintf(stderr, 
	    "                    : --------------------------------------------------|\r");
    for (i=0; i<l && i<20; i++) {
      fputc(message[i], stderr);
    }
    for (; i<20; i++) {
      fputc(' ', stderr);
    }
    fputc(':', stderr); fputc(' ', stderr);
  } 
  while (last < percent) {
    fputc('*', stderr);
    last+=2;
  }
  return 0;
}

static R_INLINE void R_igraph_before() {
  R_igraph_oldhandler=igraph_set_error_handler(R_igraph_myhandler);
  R_igraph_oldwarning=igraph_set_warning_handler(R_igraph_warning_handler);
  R_igraph_oldinterrupt=
    igraph_set_interruption_handler(R_igraph_interrupt_handler);
  R_igraph_attribute_oldtable=
    igraph_i_set_attribute_table(&R_igraph_attribute_table);
}

static R_INLINE void R_igraph_before2(SEXP verbose, const char *str) {
  R_igraph_oldhandler=igraph_set_error_handler(R_igraph_myhandler);
  R_igraph_oldwarning=igraph_set_warning_handler(R_igraph_warning_handler);
  R_igraph_oldinterrupt=
    igraph_set_interruption_handler(R_igraph_interrupt_handler);
  R_igraph_attribute_oldtable=
    igraph_i_set_attribute_table(&R_igraph_attribute_table);
  if (LOGICAL(verbose)[0]) {
    R_igraph_oldprogress=igraph_set_progress_handler(R_igraph_progress_handler);
  }
}  

static R_INLINE void R_igraph_after() {
  igraph_set_error_handler(R_igraph_oldhandler);
  igraph_set_warning_handler(R_igraph_oldwarning);
  igraph_set_interruption_handler(R_igraph_oldinterrupt);
  igraph_i_set_attribute_table(R_igraph_attribute_oldtable);
}

static R_INLINE void R_igraph_after2(SEXP verbose) {
  igraph_set_error_handler(R_igraph_oldhandler);
  igraph_set_warning_handler(R_igraph_oldwarning);
  igraph_set_interruption_handler(R_igraph_oldinterrupt);
  igraph_i_set_attribute_table(R_igraph_attribute_oldtable);
  if (LOGICAL(verbose)[0]) {
    igraph_set_progress_handler(R_igraph_oldprogress);
    fputc('\n', stderr);
  }
}

SEXP R_igraph_finalizer() {
  IGRAPH_FINALLY_FREE();
  return R_NilValue;
}

/******************************************************
 * functions to convert igraph objects to SEXP
 *****************************************************/

SEXP R_igraph_vector_to_SEXP(igraph_vector_t *v) {
  SEXP result;
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(v)));
  igraph_vector_copy_to(v, REAL(result));
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_0orvector_to_SEXP(igraph_vector_t *v) {
  SEXP result;
  if (v) {
    PROTECT(result=R_igraph_vector_to_SEXP(v));
  } else {
    PROTECT(result=R_NilValue);
  }
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_vector_bool_to_SEXP(igraph_vector_bool_t *v) {
  SEXP result;
  long int n=igraph_vector_bool_size(v);
  
  PROTECT(result=NEW_LOGICAL(n));
  if (sizeof(igraph_bool_t)==sizeof(LOGICAL(result)[0])) {
    igraph_vector_bool_copy_to(v, LOGICAL(result));
  } else {
    long int i;
    for (i=0; i<n; i++) {
      LOGICAL(result)[i] = VECTOR(*v)[i];
    }
  }
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_0orvector_bool_to_SEXP(igraph_vector_bool_t *v) {
  SEXP result;
  if (v) {
    PROTECT(result=R_igraph_vector_bool_to_SEXP(v));
  } else {
    PROTECT(result=R_NilValue);
  }
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_matrix_to_SEXP(igraph_matrix_t *m) {

  SEXP result, dim; 
  
  PROTECT(result=NEW_NUMERIC(igraph_matrix_size(m)));
  igraph_matrix_copy_to(m, REAL(result));
  PROTECT(dim=NEW_INTEGER(2));
  INTEGER(dim)[0]=igraph_matrix_nrow(m);
  INTEGER(dim)[1]=igraph_matrix_ncol(m);
  SET_DIM(result, dim);

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_0ormatrix_to_SEXP(igraph_matrix_t *m) {
  SEXP result;
  if (m) {
    PROTECT(result=R_igraph_matrix_to_SEXP(m));
  } else {
    PROTECT(result=R_NilValue);
  }
  UNPROTECT(1);
  return result;
}


SEXP R_igraph_array3_to_SEXP(igraph_array3_t *a) {
  SEXP result, dim;
  
  PROTECT(result=NEW_NUMERIC(igraph_array3_size(a)));
  igraph_vector_copy_to(&a->data, REAL(result));
  PROTECT(dim=NEW_INTEGER(3));
  INTEGER(dim)[0]=igraph_array3_n(a, 1);
  INTEGER(dim)[1]=igraph_array3_n(a, 2);
  INTEGER(dim)[2]=igraph_array3_n(a, 3);
  SET_DIM(result, dim);
  
  UNPROTECT(2);
  return result;
}

SEXP R_igraph_0orarray3_to_SEXP(igraph_array3_t *a) {
  SEXP result;
  if (a) {
    PROTECT(result=R_igraph_array3_to_SEXP(a));
  } else {
    PROTECT(result=R_NilValue);
  }
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_strvector_to_SEXP(const igraph_strvector_t *m) {
  SEXP result;
  long int i;
  char *str;
  long int len;
  
  len=igraph_strvector_size(m);
  PROTECT(result=NEW_CHARACTER(len));
  for (i=0; i<len; i++) {
    igraph_strvector_get(m, i, &str);
    SET_STRING_ELT(result, i, CREATE_STRING_VECTOR(str));
  }
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_to_SEXP(igraph_t *graph) {
  
  SEXP result;
  long int no_of_nodes=igraph_vcount(graph);
  long int no_of_edges=igraph_ecount(graph);
  
  PROTECT(result=NEW_LIST(9));
  SET_VECTOR_ELT(result, 0, NEW_NUMERIC(1));
  SET_VECTOR_ELT(result, 1, NEW_LOGICAL(1));
  SET_VECTOR_ELT(result, 2, NEW_NUMERIC(no_of_edges));
  SET_VECTOR_ELT(result, 3, NEW_NUMERIC(no_of_edges));
  SET_VECTOR_ELT(result, 4, NEW_NUMERIC(no_of_edges));
  SET_VECTOR_ELT(result, 5, NEW_NUMERIC(no_of_edges));
  SET_VECTOR_ELT(result, 6, NEW_NUMERIC(no_of_nodes+1));
  SET_VECTOR_ELT(result, 7, NEW_NUMERIC(no_of_nodes+1));

  REAL(VECTOR_ELT(result, 0))[0]=no_of_nodes;
  LOGICAL(VECTOR_ELT(result, 1))[0]=graph->directed;
  memcpy(REAL(VECTOR_ELT(result, 2)), graph->from.stor_begin, 
	 sizeof(igraph_real_t)*no_of_edges);
  memcpy(REAL(VECTOR_ELT(result, 3)), graph->to.stor_begin, 
	 sizeof(igraph_real_t)*no_of_edges);
  memcpy(REAL(VECTOR_ELT(result, 4)), graph->oi.stor_begin, 
	 sizeof(igraph_real_t)*no_of_edges);
  memcpy(REAL(VECTOR_ELT(result, 5)), graph->ii.stor_begin, 
	 sizeof(igraph_real_t)*no_of_edges);
  memcpy(REAL(VECTOR_ELT(result, 6)), graph->os.stor_begin, 
	 sizeof(igraph_real_t)*(no_of_nodes+1));
  memcpy(REAL(VECTOR_ELT(result, 7)), graph->is.stor_begin, 
	 sizeof(igraph_real_t)*(no_of_nodes+1));
  
  SET_CLASS(result, ScalarString(CREATE_STRING_VECTOR("igraph")));

  /* Attributes */
  SET_VECTOR_ELT(result, 8, graph->attr);
  REAL(VECTOR_ELT(graph->attr, 0))[0] += 1;
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_vectorlist_to_SEXP(const igraph_vector_ptr_t *ptr) {
  SEXP result;
  long int i, n=igraph_vector_ptr_size(ptr);
  
  PROTECT(result=NEW_LIST(n));
  for (i=0; i<n; i++) {
    igraph_vector_t *v=VECTOR(*ptr)[i];
    SET_VECTOR_ELT(result, i, R_igraph_vector_to_SEXP(v));
  }
  
  UNPROTECT(1);
  return result;
}

void R_igraph_vectorlist_destroy(igraph_vector_ptr_t *ptr) {
  long int i, n=igraph_vector_ptr_size(ptr);
  
  for (i=0; i<n; i++) {
    igraph_vector_t *v=VECTOR(*ptr)[i];
    igraph_vector_destroy(v);
    igraph_free(v);
  }
  igraph_vector_ptr_destroy(ptr);
}

int R_SEXP_to_igraph_adjlist(SEXP vectorlist, igraph_adjlist_t *ptr) {
  long int length=GET_LENGTH(vectorlist);
  long int i;

  ptr->length=length;
  ptr->adjs = (igraph_vector_t*) R_alloc(length, sizeof(igraph_vector_t));
  for (i=0; i<length; i++) {
    SEXP vec=VECTOR_ELT(vectorlist, i);
    igraph_vector_view(&ptr->adjs[i], REAL(vec), GET_LENGTH(vec));
  }
  return 0;
}

int R_igraph_SEXP_to_strvector(SEXP rval, igraph_strvector_t *sv) {
  long int i;
  sv->len=GET_LENGTH(rval);
  sv->data=(char**) R_alloc(sv->len, sizeof(char*));
  for (i=0; i<sv->len; i++) {
    sv->data[i]=(char*) CHAR(STRING_ELT(rval, i));
  }

  return 0;
}

int R_igraph_SEXP_to_strvector_copy(SEXP rval, igraph_strvector_t *sv) {
  long int i;
  igraph_strvector_init(sv, GET_LENGTH(rval));
  for (i=0; i<sv->len; i++) {
    igraph_strvector_set(sv, i, CHAR(STRING_ELT(rval, i)));
  }
  
  return 0;
}

int R_SEXP_to_vector(SEXP sv, igraph_vector_t *v) {
  v->stor_begin=REAL(sv);
  v->stor_end=v->stor_begin+GET_LENGTH(sv);
  v->end=v->stor_end;
  return 0;
}

int R_SEXP_to_vector_copy(SEXP sv, igraph_vector_t *v) {
  return igraph_vector_init_copy(v, REAL(sv), GET_LENGTH(sv));  
}

int R_SEXP_to_vector_bool(SEXP sv, igraph_vector_bool_t *v) {
  v->stor_begin=LOGICAL(sv);
  v->stor_end=v->stor_begin+GET_LENGTH(sv);
  v->end=v->stor_end;
  return 0;
}

int R_SEXP_to_matrix(SEXP pakl, igraph_matrix_t *akl) {
  R_SEXP_to_vector(pakl, &akl->data);
  akl->nrow=INTEGER(GET_DIM(pakl))[0];
  akl->ncol=INTEGER(GET_DIM(pakl))[1];

  return 0;
}

int R_SEXP_to_igraph_matrix_copy(SEXP pakl, igraph_matrix_t *akl) {
  igraph_vector_init_copy(&akl->data, REAL(pakl), GET_LENGTH(pakl));
  akl->nrow=INTEGER(GET_DIM(pakl))[0];
  akl->ncol=INTEGER(GET_DIM(pakl))[1];

  return 0;
}

int R_igraph_SEXP_to_array3(SEXP rval, igraph_array3_t *a) {
  R_SEXP_to_vector(rval, &a->data);
  a->n1=INTEGER(GET_DIM(rval))[0];
  a->n2=INTEGER(GET_DIM(rval))[1];
  a->n3=INTEGER(GET_DIM(rval))[2];
  a->n1n2=(a->n1) * (a->n2);

  return 0;
}

int R_igraph_SEXP_to_array3_copy(SEXP rval, igraph_array3_t *a) {
  igraph_vector_init_copy(&a->data, REAL(rval), GET_LENGTH(rval));
  a->n1=INTEGER(GET_DIM(rval))[0];
  a->n2=INTEGER(GET_DIM(rval))[1];
  a->n3=INTEGER(GET_DIM(rval))[2];
  a->n1n2=(a->n1) * (a->n2);

  return 0;
}

int R_SEXP_to_igraph(SEXP graph, igraph_t *res) {
  
  res->n=REAL(VECTOR_ELT(graph, 0))[0];
  res->directed=LOGICAL(VECTOR_ELT(graph, 1))[0];
  R_SEXP_to_vector(VECTOR_ELT(graph, 2), &res->from);
  R_SEXP_to_vector(VECTOR_ELT(graph, 3), &res->to);
  R_SEXP_to_vector(VECTOR_ELT(graph, 4), &res->oi);
  R_SEXP_to_vector(VECTOR_ELT(graph, 5), &res->ii);
  R_SEXP_to_vector(VECTOR_ELT(graph, 6), &res->os);
  R_SEXP_to_vector(VECTOR_ELT(graph, 7), &res->is);
  
  /* attributes */
  REAL(VECTOR_ELT(VECTOR_ELT(graph, 8), 0))[0] = 1; /* R objects refcount */
  REAL(VECTOR_ELT(VECTOR_ELT(graph, 8), 0))[1] = 0; /* igraph_t objects */
  res->attr=VECTOR_ELT(graph, 8);
  
  return 0;
}

int R_SEXP_to_igraph_copy(SEXP graph, igraph_t *res) {
  
  res->n=REAL(VECTOR_ELT(graph, 0))[0];
  res->directed=LOGICAL(VECTOR_ELT(graph, 1))[0];
  igraph_vector_init_copy(&res->from, REAL(VECTOR_ELT(graph, 2)), 
		   GET_LENGTH(VECTOR_ELT(graph, 2)));
  igraph_vector_init_copy(&res->to, REAL(VECTOR_ELT(graph, 3)), 
		   GET_LENGTH(VECTOR_ELT(graph, 3)));
  igraph_vector_init_copy(&res->oi, REAL(VECTOR_ELT(graph, 4)), 
		   GET_LENGTH(VECTOR_ELT(graph, 4)));
  igraph_vector_init_copy(&res->ii, REAL(VECTOR_ELT(graph, 5)), 
		   GET_LENGTH(VECTOR_ELT(graph, 5)));
  igraph_vector_init_copy(&res->os, REAL(VECTOR_ELT(graph, 6)), 
		   GET_LENGTH(VECTOR_ELT(graph, 6)));
  igraph_vector_init_copy(&res->is, REAL(VECTOR_ELT(graph, 7)),
		   GET_LENGTH(VECTOR_ELT(graph, 7)));

  /* attributes */
  REAL(VECTOR_ELT(VECTOR_ELT(graph, 8), 0))[0] = 1; /* R objects */
  REAL(VECTOR_ELT(VECTOR_ELT(graph, 8), 0))[1] = 1; /* igraph_t objects */
  PROTECT(res->attr=VECTOR_ELT(graph, 8));  

  return 0;
}

/* 
 * We have only vector type
 */

int R_SEXP_to_igraph_vs(SEXP rit, igraph_t *graph, igraph_vs_t *it) {
  
  igraph_vector_t *tmpv=(igraph_vector_t*)R_alloc(1,sizeof(igraph_vector_t));
  igraph_vs_vector(it, igraph_vector_view(tmpv, REAL(rit), 
					  GET_LENGTH(rit)));
  return 0;
}

/* 
 * We have only vector type
 */

int R_SEXP_to_igraph_es(SEXP rit, igraph_t *graph, igraph_es_t *it) {

  igraph_vector_t *tmpv=(igraph_vector_t*)R_alloc(1,sizeof(igraph_vector_t));
  igraph_es_vector(it, igraph_vector_view(tmpv, REAL(rit),
					  GET_LENGTH(rit)));
  return 0;
}

int R_SEXP_to_igraph_layout_drl_options(SEXP in, igraph_layout_drl_options_t *opt) {
  opt->edge_cut = REAL(AS_NUMERIC(R_igraph_getListElement(in, "edge.cut")))[0];
  opt->init_iterations   = REAL(AS_NUMERIC(R_igraph_getListElement(in, "init.iterations")))[0];
  opt->init_temperature  = REAL(AS_NUMERIC(R_igraph_getListElement(in, "init.temperature")))[0];
  opt->init_attraction   = REAL(AS_NUMERIC(R_igraph_getListElement(in, "init.attraction")))[0];
  opt->init_damping_mult = REAL(AS_NUMERIC(R_igraph_getListElement(in, "init.damping.mult")))[0];
  opt->liquid_iterations   = REAL(AS_NUMERIC(R_igraph_getListElement(in, "liquid.iterations")))[0];
  opt->liquid_temperature  = REAL(AS_NUMERIC(R_igraph_getListElement(in, "liquid.temperature")))[0];
  opt->liquid_attraction   = REAL(AS_NUMERIC(R_igraph_getListElement(in, "liquid.attraction")))[0];
  opt->liquid_damping_mult = REAL(AS_NUMERIC(R_igraph_getListElement(in, "liquid.damping.mult")))[0];
  opt->expansion_iterations   = REAL(AS_NUMERIC(R_igraph_getListElement(in, "expansion.iterations")))[0];
  opt->expansion_temperature  = REAL(AS_NUMERIC(R_igraph_getListElement(in, "expansion.temperature")))[0];
  opt->expansion_attraction   = REAL(AS_NUMERIC(R_igraph_getListElement(in, "expansion.attraction")))[0];
  opt->expansion_damping_mult = REAL(AS_NUMERIC(R_igraph_getListElement(in, "expansion.damping.mult")))[0];
  opt->cooldown_iterations   = REAL(AS_NUMERIC(R_igraph_getListElement(in, "cooldown.iterations")))[0];
  opt->cooldown_temperature  = REAL(AS_NUMERIC(R_igraph_getListElement(in, "cooldown.temperature")))[0];
  opt->cooldown_attraction   = REAL(AS_NUMERIC(R_igraph_getListElement(in, "cooldown.attraction")))[0];
  opt->cooldown_damping_mult = REAL(AS_NUMERIC(R_igraph_getListElement(in, "cooldown.damping.mult")))[0];
  opt->crunch_iterations   = REAL(AS_NUMERIC(R_igraph_getListElement(in, "crunch.iterations")))[0];
  opt->crunch_temperature  = REAL(AS_NUMERIC(R_igraph_getListElement(in, "crunch.temperature")))[0];
  opt->crunch_attraction   = REAL(AS_NUMERIC(R_igraph_getListElement(in, "crunch.attraction")))[0];
  opt->crunch_damping_mult = REAL(AS_NUMERIC(R_igraph_getListElement(in, "crunch.damping.mult")))[0];
  opt->simmer_iterations   = REAL(AS_NUMERIC(R_igraph_getListElement(in, "simmer.iterations")))[0];
  opt->simmer_temperature  = REAL(AS_NUMERIC(R_igraph_getListElement(in, "simmer.temperature")))[0];
  opt->simmer_attraction   = REAL(AS_NUMERIC(R_igraph_getListElement(in, "simmer.attraction")))[0];
  opt->simmer_damping_mult = REAL(AS_NUMERIC(R_igraph_getListElement(in, "simmer.damping.mult")))[0];

  return 0;
}

int R_SEXP_to_igraph_arpack_options(SEXP in, igraph_arpack_options_t *opt) {
  SEXP tmp;
  const char *tmpstr;
  igraph_arpack_options_init(opt);
  opt -> bmat[0] = CHAR(STRING_ELT(AS_CHARACTER
				   (R_igraph_getListElement(in, "bmat")), 0))[0];
  opt -> n = INTEGER(AS_INTEGER(R_igraph_getListElement(in, "n")))[0];
  tmpstr=CHAR(STRING_ELT(AS_CHARACTER(R_igraph_getListElement(in, "which")), 0));
  opt -> which[0]=tmpstr[0]; opt -> which[1]=tmpstr[1];
  opt -> nev = INTEGER(AS_INTEGER(R_igraph_getListElement(in, "nev")))[0];
  opt -> tol = REAL(AS_NUMERIC(R_igraph_getListElement(in, "tol")))[0];
  opt -> ncv = INTEGER(AS_INTEGER(R_igraph_getListElement(in, "ncv")))[0];
  opt -> ldv = INTEGER(AS_INTEGER(R_igraph_getListElement(in, "ldv")))[0];
  opt -> ishift = INTEGER(AS_INTEGER(R_igraph_getListElement(in, "ishift")))[0];
  opt -> mxiter = INTEGER(AS_INTEGER(R_igraph_getListElement(in, "maxiter")))[0];
  opt -> nb = INTEGER(AS_INTEGER(R_igraph_getListElement(in, "nb")))[0];
  opt -> mode = INTEGER(AS_INTEGER(R_igraph_getListElement(in, "mode")))[0];
  opt -> start = INTEGER(AS_INTEGER(R_igraph_getListElement(in, "start")))[0];
  opt -> lworkl = 0;
  opt -> sigma = REAL(AS_NUMERIC(R_igraph_getListElement(in, "sigma")))[0];
  opt -> sigmai = REAL(AS_NUMERIC(R_igraph_getListElement(in, "sigmai")))[0];
  opt -> info = opt -> start;

  opt->iparam[0]=opt->ishift;
  opt->iparam[2]=opt->mxiter; 
  opt->iparam[3]=opt->nb;
  opt->iparam[6]=opt->mode;
  
  return 0;
}

SEXP R_igraph_arpack_options_to_SEXP(igraph_arpack_options_t *opt) {
  SEXP result, names;
  char bmat[2], which[3];

  bmat[0]=opt->bmat[0]; bmat[1]='\0';
  which[0]=opt->which[0]; which[1]=opt->which[1]; which[2]='\0';
  
  PROTECT(result = NEW_LIST(20));
  SET_VECTOR_ELT(result, 0, NEW_CHARACTER(1)); SET_STRING_ELT(VECTOR_ELT(result, 0), 0, mkChar(bmat));
  SET_VECTOR_ELT(result, 1, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 1))[0]=opt->n;
  SET_VECTOR_ELT(result, 2, NEW_CHARACTER(1)); SET_STRING_ELT(VECTOR_ELT(result, 2), 0, mkChar(which));
  SET_VECTOR_ELT(result, 3, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 3))[0]=opt->nev;
  SET_VECTOR_ELT(result, 4, NEW_NUMERIC(1)); REAL(VECTOR_ELT(result, 4))[0]=opt->tol;
  SET_VECTOR_ELT(result, 5, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 5))[0]=opt->ncv;
  SET_VECTOR_ELT(result, 6, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 6))[0]=opt->ldv;
  SET_VECTOR_ELT(result, 7, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 7))[0]=opt->ishift;
  SET_VECTOR_ELT(result, 8, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 8))[0]=opt->mxiter;
  SET_VECTOR_ELT(result, 9, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 9))[0]=opt->nb;
  SET_VECTOR_ELT(result, 10, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 10))[0]=opt->mode;
  SET_VECTOR_ELT(result, 11, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 11))[0]=opt->start;
  SET_VECTOR_ELT(result, 12, NEW_NUMERIC(1)); REAL(VECTOR_ELT(result, 12))[0]=opt->sigma;
  SET_VECTOR_ELT(result, 13, NEW_NUMERIC(1)); REAL(VECTOR_ELT(result, 13))[0]=opt->sigmai;

  SET_VECTOR_ELT(result, 14, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 14))[0]=opt->info;
  SET_VECTOR_ELT(result, 15, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 15))[0]=opt->iparam[2];/* mxiter */
  SET_VECTOR_ELT(result, 16, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 16))[0]=opt->iparam[4];/* nconv */
  SET_VECTOR_ELT(result, 17, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 17))[0]=opt->iparam[8];/* numop */
  SET_VECTOR_ELT(result, 18, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 18))[0]=opt->iparam[9];/* numopb */
  SET_VECTOR_ELT(result, 19, NEW_INTEGER(1)); INTEGER(VECTOR_ELT(result, 19))[0]=opt->iparam[10];/* numreo */
  
  PROTECT(names=NEW_CHARACTER(20));
  SET_STRING_ELT(names, 0, mkChar("bmat"));
  SET_STRING_ELT(names, 1, mkChar("n"));
  SET_STRING_ELT(names, 2, mkChar("which"));
  SET_STRING_ELT(names, 3, mkChar("nev"));
  SET_STRING_ELT(names, 4, mkChar("tol"));
  SET_STRING_ELT(names, 5, mkChar("ncv"));
  SET_STRING_ELT(names, 6, mkChar("ldv"));
  SET_STRING_ELT(names, 7, mkChar("ishift"));
  SET_STRING_ELT(names, 8, mkChar("maxiter"));
  SET_STRING_ELT(names, 9, mkChar("nb"));
  SET_STRING_ELT(names, 10, mkChar("mode"));
  SET_STRING_ELT(names, 11, mkChar("start"));
  SET_STRING_ELT(names, 12, mkChar("sigma"));
  SET_STRING_ELT(names, 13, mkChar("sigmai"));
  SET_STRING_ELT(names, 14, mkChar("info"));
  SET_STRING_ELT(names, 15, mkChar("iter"));
  SET_STRING_ELT(names, 16, mkChar("nconv"));
  SET_STRING_ELT(names, 17, mkChar("numop"));
  SET_STRING_ELT(names, 18, mkChar("numopb"));
  SET_STRING_ELT(names, 19, mkChar("numreo"));
  SET_NAMES(result, names);
  
  UNPROTECT(2);
  return result;  
}

SEXP R_igraph_bliss_info_to_SEXP(igraph_bliss_info_t *info) {
  SEXP result, names;
  
  PROTECT(result=NEW_LIST(6));
  SET_VECTOR_ELT(result, 0, NEW_NUMERIC(1)); REAL(VECTOR_ELT(result, 0))[0]=info->nof_nodes;
  SET_VECTOR_ELT(result, 1, NEW_NUMERIC(1)); REAL(VECTOR_ELT(result, 1))[0]=info->nof_leaf_nodes;
  SET_VECTOR_ELT(result, 2, NEW_NUMERIC(1)); REAL(VECTOR_ELT(result, 2))[0]=info->nof_bad_nodes;
  SET_VECTOR_ELT(result, 3, NEW_NUMERIC(1)); REAL(VECTOR_ELT(result, 3))[0]=info->nof_canupdates;
  SET_VECTOR_ELT(result, 4, NEW_NUMERIC(1)); REAL(VECTOR_ELT(result, 4))[0]=info->max_level;
  if (info->group_size) {
    SET_VECTOR_ELT(result, 5, NEW_CHARACTER(1));
    SET_STRING_ELT(VECTOR_ELT(result, 5), 0, mkChar(info->group_size));
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  
  PROTECT(names=NEW_CHARACTER(6));
  SET_STRING_ELT(names, 0, mkChar("nof_nodes"));
  SET_STRING_ELT(names, 1, mkChar("nof_leaf_nodes"));
  SET_STRING_ELT(names, 2, mkChar("nof_bad_nodes"));
  SET_STRING_ELT(names, 3, mkChar("nof_canupdates"));
  SET_STRING_ELT(names, 4, mkChar("max_level"));
  SET_STRING_ELT(names, 5, mkChar("group_size"));
  SET_NAMES(result, names);

  UNPROTECT(2);
  return result;
}

/*******************************************************************/

SEXP R_igraph_add_edges(SEXP graph, SEXP edges) {
  
  igraph_vector_t v;			/* do NOT destroy! */
  igraph_t g;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_vector(edges, &v);
  R_SEXP_to_igraph_copy(graph, &g);
  igraph_add_edges(&g, &v, 0);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_add_vertices(SEXP graph, SEXP pnv) {
  
  igraph_integer_t nv;
  igraph_t g;
  SEXP result;
  
  R_igraph_before();
  
  nv=REAL(pnv)[0];
  R_SEXP_to_igraph_copy(graph, &g);
  igraph_add_vertices(&g, nv, 0);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_ecount(SEXP graph) {
  
  igraph_t g;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=igraph_ecount(&g);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_neighbors(SEXP graph, SEXP pvid, SEXP pmode) {
  
  igraph_t g;
  igraph_vector_t neis;
  SEXP result;
  igraph_real_t vid;
  igraph_integer_t mode;
  
  R_igraph_before();
  
  igraph_vector_init(&neis, 0);
  vid=REAL(pvid)[0];
  mode=REAL(pmode)[0];
  R_SEXP_to_igraph(graph, &g);
  igraph_neighbors(&g, &neis, vid, mode);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&neis)));
  igraph_vector_copy_to(&neis, REAL(result));
  igraph_vector_destroy(&neis);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_delete_edges(SEXP graph, SEXP edges) {
  
  igraph_es_t es;
  igraph_t g;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph_copy(graph, &g);
  R_SEXP_to_igraph_es(edges, &g, &es);
  igraph_delete_edges(&g, es);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_delete_vertices(SEXP graph, SEXP vertices) {
  
  igraph_vs_t vs;
  igraph_t g;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph_copy(graph, &g);
  R_SEXP_to_igraph_vs(vertices, &g, &vs);
  igraph_delete_vertices(&g, vs);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  igraph_vs_destroy(&vs);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_is_directed(SEXP graph) {
  
  igraph_t g;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  PROTECT(result=NEW_LOGICAL(1));
  LOGICAL(result)[0]=igraph_is_directed(&g);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_create(SEXP edges, SEXP pn, SEXP pdirected) {
  
  igraph_t g;
  igraph_vector_t v;
  igraph_integer_t n=REAL(pn)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_vector(edges, &v);
  igraph_create(&g, &v, n, directed);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_degree(SEXP graph, SEXP vids, SEXP pmode, SEXP ploops) {
  
  igraph_t g;
  igraph_vs_t vs;
  igraph_vector_t res;
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_bool_t loops=LOGICAL(ploops)[0];
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(vids, &g, &vs);
  igraph_vector_init(&res, 0);
  igraph_degree(&g, &res, vs, mode, loops);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  igraph_vs_destroy(&vs);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_is_connected(SEXP graph, SEXP pmode) {

  igraph_t g;
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_bool_t res;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_is_connected(&g, &res, mode);
  
  PROTECT(result=NEW_LOGICAL(1));
  LOGICAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_diameter(SEXP graph, SEXP pdirected, SEXP punconnected, 
		       SEXP pweights) {
  
  igraph_t g;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_bool_t unconnected=LOGICAL(punconnected)[0];
  igraph_vector_t weights;
  igraph_integer_t res;  
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  if (!isNull(pweights)) {
    R_SEXP_to_vector(pweights, &weights);
  }
  igraph_diameter_dijkstra(&g, isNull(pweights) ? 0 : &weights, 
			   &res, 0, 0, 0, directed, unconnected);
  
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_get_diameter(SEXP graph, SEXP pdirected, SEXP punconnected, 
			   SEXP pweights) {
  
  igraph_t g;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_bool_t unconnected=LOGICAL(punconnected)[0];
  igraph_vector_t weights;
  igraph_vector_t res;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  if (!isNull(pweights)) {
    R_SEXP_to_vector(pweights, &weights);
  }
  igraph_vector_init(&res, 0);
  igraph_diameter_dijkstra(&g, isNull(pweights) ? 0 : &weights, 
			   0, 0, 0, &res, directed, unconnected);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_farthest_points(SEXP graph, SEXP pdirected, SEXP punconnected, 
			      SEXP pweights) {
  
  igraph_t g;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_bool_t unconnected=LOGICAL(punconnected)[0];
  igraph_vector_t weights;
  igraph_integer_t from, to, len;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  if (!isNull(pweights)) {
    R_SEXP_to_vector(pweights, &weights);
  }
  igraph_diameter_dijkstra(&g, isNull(pweights) ? 0 : &weights, 
			   &len, &from, &to, 0, directed, unconnected);
  
  PROTECT(result=NEW_NUMERIC(3));
  if (from < 0) {
    REAL(result)[0]=REAL(result)[1]=REAL(result)[2]=NA_REAL;
  } else {
    REAL(result)[0]=from;
    REAL(result)[1]=to;
    REAL(result)[2]=len;
  }
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_closeness(SEXP graph, SEXP pvids, SEXP pmode) {
  
  igraph_t g;
  igraph_vs_t vs;
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();

  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pvids, &g, &vs);
  igraph_vector_init(&res, 0);
  igraph_closeness(&g, &res, vs, mode);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  igraph_vs_destroy(&vs);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_subcomponent(SEXP graph, SEXP pvertex, SEXP pmode) {
  
  igraph_t g;
  igraph_real_t vertex=REAL(pvertex)[0];
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&res, 0);
  igraph_subcomponent(&g, &res, vertex, mode);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_betweenness(SEXP graph, SEXP pvids, SEXP pdirected, SEXP verbose) {
  
  igraph_t g;
  igraph_vs_t vs;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before2(verbose, "Betweenness");

  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pvids, &g, &vs);
  igraph_vector_init(&res, 0);
  igraph_betweenness(&g, &res, vs, directed);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  igraph_vs_destroy(&vs);

  R_igraph_after2(verbose);
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_running_mean(SEXP pdata, SEXP pbinwidth) {
  
  igraph_vector_t data;
  igraph_integer_t binwidth=REAL(pbinwidth)[0];
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  R_SEXP_to_vector(pdata, &data);
  
  igraph_vector_init(&res, 0);
  igraph_running_mean(&data, &res, binwidth);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_cocitation(SEXP graph, SEXP pvids) {

  igraph_t g;
  igraph_vs_t vs;
  igraph_matrix_t m;
  SEXP result;
  
  R_igraph_before();

  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pvids, &g, &vs);
  igraph_matrix_init(&m, 0, 0);
  igraph_cocitation(&g, &m, vs);
  
  PROTECT(result=R_igraph_matrix_to_SEXP(&m));
  igraph_matrix_destroy(&m);
  igraph_vs_destroy(&vs);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_bibcoupling(SEXP graph, SEXP pvids) {

  igraph_t g;
  igraph_vs_t vs;
  igraph_matrix_t m;
  SEXP result;
  
  R_igraph_before();

  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pvids, &g, &vs);
  igraph_matrix_init(&m, 0, 0);
  igraph_bibcoupling(&g, &m, vs);
  
  PROTECT(result=R_igraph_matrix_to_SEXP(&m));
  igraph_matrix_destroy(&m);
  igraph_vs_destroy(&vs);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_growing_random_game(SEXP pn, SEXP pm, SEXP pdirected,
				  SEXP pcitation) {
  
  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_integer_t m=REAL(pm)[0];
  igraph_bool_t citation=LOGICAL(pcitation)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  SEXP result;
  
  R_igraph_before();
  
  igraph_growing_random_game(&g, n, m, directed, citation);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;  
}

SEXP R_igraph_shortest_paths(SEXP graph, SEXP pvids, SEXP pmode, SEXP weights, 
			     SEXP palgo) {
  
  igraph_t g;
  igraph_vs_t vs;
  igraph_integer_t mode=REAL(pmode)[0];
  long int algo=REAL(palgo)[0];
  igraph_matrix_t res;
  igraph_vector_t w, *pw=&w;
  igraph_bool_t negw=0;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pvids, &g, &vs);
  if (isNull(weights)) {
    pw=0;
  } else {
    R_SEXP_to_vector(weights, &w);
    negw = igraph_vector_min(&w) < 0;
  }
  igraph_matrix_init(&res, 0, 0);
  switch (algo) {
  case 0: 			/* automatic */
    if (negw && mode==IGRAPH_OUT && GET_LENGTH(pvids)>100) {
      igraph_shortest_paths_johnson(&g, &res, vs, pw);
    } else if (negw) {
      igraph_shortest_paths_bellman_ford(&g, &res, vs, pw, mode);
    } else {
      /* This one chooses 'unweighted' if there are no weights */
      igraph_shortest_paths_dijkstra(&g, &res, vs, pw, mode);
    }
    break;
  case 1:			/* unweighted */
    igraph_shortest_paths(&g, &res, vs, mode);
    break;
  case 2:			/* dijkstra */
    igraph_shortest_paths_dijkstra(&g, &res, vs, pw, mode);
    break;
  case 3:			/* bellman-ford */
    igraph_shortest_paths_bellman_ford(&g, &res, vs, pw, mode);
    break;
  case 4:			/* johnson */
    igraph_shortest_paths_johnson(&g, &res, vs, pw);
    break;
  }
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  igraph_vs_destroy(&vs);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_lattice(SEXP pdimvector, SEXP pnei, SEXP pdirected,
		      SEXP pmutual, SEXP pcircular) {
  
  igraph_t g;
  igraph_vector_t dimvector;
  igraph_integer_t nei=REAL(pnei)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_bool_t mutual=LOGICAL(pmutual)[0];
  igraph_bool_t circular=LOGICAL(pcircular)[0];  
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(pdimvector, &dimvector);
  
  igraph_lattice(&g, &dimvector, nei, directed, mutual, circular);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;  
}

SEXP R_igraph_barabasi_game(SEXP pn, SEXP pm, SEXP poutseq,
			    SEXP poutpref, SEXP pdirected) {
  
  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_integer_t m=REAL(pm)[0]; 
  igraph_vector_t outseq;
  igraph_bool_t outpref=LOGICAL(poutpref)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  SEXP result;
  
  R_igraph_before();

  R_SEXP_to_vector(poutseq, &outseq);
  
  igraph_barabasi_game(&g, n, m, &outseq, outpref, directed);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_nonlinear_barabasi_game(SEXP pn, SEXP ppower, SEXP pm,
				      SEXP poutseq, SEXP poutpref, 
				      SEXP pzeroappeal,
				      SEXP pdirected) {
  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_real_t power=REAL(ppower)[0];
  igraph_integer_t m=REAL(pm)[0];
  igraph_vector_t outseq;
  igraph_bool_t outpref=LOGICAL(poutpref)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_real_t zeroappeal=REAL(pzeroappeal)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(poutseq, &outseq);
  
  igraph_nonlinear_barabasi_game(&g, n, power, m, &outseq, outpref, 
				 zeroappeal, directed);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_recent_degree_game(SEXP pn, SEXP ppower, SEXP pwindow,
				 SEXP pm, SEXP poutseq, SEXP poutpref,
				 SEXP pzero_appeal,
				 SEXP pdirected) {
  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_real_t power=REAL(ppower)[0];
  igraph_integer_t window=REAL(pwindow)[0];
  igraph_integer_t m=REAL(pm)[0];
  igraph_vector_t outseq;
  igraph_bool_t outpref=LOGICAL(poutpref)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_real_t zero_appeal=REAL(pzero_appeal)[0];
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_vector(poutseq, &outseq);
  
  igraph_recent_degree_game(&g, n, power, window, m, &outseq, outpref, 
			    zero_appeal, directed);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_layout_kamada_kawai(SEXP graph, SEXP pniter, SEXP pinitemp, 
				  SEXP pcoolexp, SEXP pkkconst, SEXP psigma, 
				  SEXP start,
				  SEXP verbose) {
  
  igraph_t g;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_real_t initemp=REAL(pinitemp)[0];
  igraph_real_t coolexp=REAL(pcoolexp)[0];
  igraph_real_t kkconst=REAL(pkkconst)[0];
  igraph_real_t sigma=REAL(psigma)[0];
  igraph_matrix_t res;
  SEXP result;
  
  R_igraph_before2(verbose, "KK layout");
  
  R_SEXP_to_igraph(graph, &g);
  if (isNull(start)) {
    igraph_matrix_init(&res, 0, 0);
  } else {
    R_SEXP_to_igraph_matrix_copy(start, &res);
  }
  igraph_layout_kamada_kawai(&g, &res, niter, sigma, 
			     initemp, coolexp, kkconst, !isNull(start));
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after2(verbose);
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_layout_kamada_kawai_3d(SEXP graph, SEXP pniter, SEXP pinitemp,
				     SEXP pcoolexp, SEXP pkkconst, 
				     SEXP psigma, SEXP start, SEXP verbose) {
  igraph_t g;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_real_t initemp=REAL(pinitemp)[0];
  igraph_real_t coolexp=REAL(pcoolexp)[0];
  igraph_real_t kkconst=REAL(pkkconst)[0];
  igraph_real_t sigma=REAL(psigma)[0];
  igraph_matrix_t res;
  SEXP result;
  
  R_igraph_before2(verbose, "3D KK layout");
  
  R_SEXP_to_igraph(graph, &g);
  if (isNull(start)) {
    igraph_matrix_init(&res, 0, 0);
  } else {
    R_SEXP_to_igraph_matrix_copy(start, &res);
  }
  igraph_layout_kamada_kawai_3d(&g, &res, niter, sigma, 
				initemp, coolexp, kkconst, !isNull(start));
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after2(verbose);
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_layout_graphopt(SEXP graph, SEXP pniter, SEXP pcharge,
			      SEXP pmass, SEXP pspring_length, 
			      SEXP pspring_constant, SEXP pmax_sa_movement,
			      SEXP start, SEXP verbose) {
  igraph_t g;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_real_t charge=REAL(pcharge)[0];
  igraph_real_t mass=REAL(pmass)[0];
  igraph_real_t spring_length=REAL(pspring_length)[0];
  igraph_real_t spring_constant=REAL(pspring_constant)[0];
  igraph_real_t max_sa_movement=REAL(pmax_sa_movement)[0];
  igraph_matrix_t res;
  SEXP result;
  
  R_igraph_before2(verbose, "Graphopt layout");
  
  R_SEXP_to_igraph(graph, &g);
  if (isNull(start)) {
    igraph_matrix_init(&res, 0, 0);
  } else {
    R_SEXP_to_igraph_matrix_copy(start, &res);
  }
  igraph_layout_graphopt(&g, &res, niter, charge, mass, spring_length,
			 spring_constant, max_sa_movement, !isNull(start));
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after2(verbose);
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_layout_lgl(SEXP graph, SEXP pmaxiter, SEXP pmaxdelta,
			 SEXP parea, SEXP pcoolexp, SEXP prepulserad,
			 SEXP pcellsize, SEXP proot) {
  
  igraph_t g;
  igraph_matrix_t res;
  igraph_integer_t maxiter=REAL(pmaxiter)[0];
  igraph_real_t maxdelta=REAL(pmaxdelta)[0];
  igraph_real_t area=REAL(parea)[0];
  igraph_real_t coolexp=REAL(pcoolexp)[0];
  igraph_real_t repulserad=REAL(prepulserad)[0];
  igraph_real_t cellsize=REAL(pcellsize)[0];
  igraph_integer_t root=REAL(proot)[0];
  SEXP result;

  R_igraph_before();

  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&res, 0, 0);
  igraph_layout_lgl(&g, &res, maxiter, maxdelta, area, coolexp, repulserad,
		    cellsize, root);
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}  

SEXP R_igraph_layout_fruchterman_reingold_grid(SEXP graph,
					       SEXP pniter,
					       SEXP pmaxdelta, SEXP parea,
					       SEXP pcoolexp, SEXP prepulserad,
					       SEXP pcellsize, SEXP start,
					       SEXP verbose) {

  igraph_t g;
  igraph_matrix_t res;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_real_t maxdelta=REAL(pmaxdelta)[0];
  igraph_real_t area=REAL(parea)[0];
  igraph_real_t coolexp=REAL(pcoolexp)[0];
  igraph_real_t repulserad=REAL(prepulserad)[0];
  igraph_real_t cellsize=REAL(pcellsize)[0];
  igraph_bool_t use_seed=!isNull(start);
  SEXP result;
  
  R_igraph_before2(verbose, "Grid-FR layout");

  R_SEXP_to_igraph(graph, &g);
  if (use_seed) {
    R_SEXP_to_igraph_matrix_copy(start, &res);
  } else {
    igraph_matrix_init(&res, 0, 0);
  }
  igraph_layout_grid_fruchterman_reingold(&g, &res, niter, maxdelta, area,
					  coolexp, repulserad, cellsize, 
					  use_seed);
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after2(verbose);
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_minimum_spanning_tree_unweighted(SEXP graph) {
  
  igraph_t g;
  igraph_t mst;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_minimum_spanning_tree_unweighted(&g, &mst);
  PROTECT(result=R_igraph_to_SEXP(&mst));
  igraph_destroy(&mst);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_minimum_spanning_tree_prim(SEXP graph, SEXP pweights) {
  
  igraph_t g;
  igraph_t mst;
  igraph_vector_t weights;
  SEXP result;
  
  R_igraph_before();

  R_SEXP_to_vector(pweights, &weights);
  
  R_SEXP_to_igraph(graph, &g);
  igraph_minimum_spanning_tree_prim(&g, &mst, &weights);
  PROTECT(result=R_igraph_to_SEXP(&mst));
  igraph_destroy(&mst);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_edge_betweenness(SEXP graph, SEXP pdirected) {
  
  igraph_t g;
  igraph_vector_t res;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&res, 0);
  igraph_edge_betweenness(&g, &res, directed);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_id(SEXP graph,
				  SEXP pst, SEXP pmaxind, SEXP psd,
				  SEXP pno, SEXP pcites, SEXP pdebug, 
				  SEXP pdebugdeg) {
  
  igraph_t g;
  igraph_matrix_t ak, sd, no;
  igraph_vector_t cites, debug;
  igraph_matrix_t *ppsd=&sd, *ppno=&no;
  igraph_vector_t *ppcites=&cites, *ppdebug=&debug;
  igraph_vector_t st;
  igraph_bool_t lsd=LOGICAL(psd)[0];
  igraph_bool_t lno=LOGICAL(pno)[0];
  igraph_bool_t lcites=LOGICAL(pcites)[0];
  igraph_bool_t ldebug=LOGICAL(pdebug)[0];
  igraph_integer_t maxind=REAL(pmaxind)[0];
  igraph_integer_t debugdeg=REAL(pdebugdeg)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(pst, &st);
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&ak, 0, 0);
  if (lsd) {
    igraph_matrix_init(&sd, 0, 0);
  } else {
    ppsd=0;
  }
  if (lno) {
    igraph_matrix_init(&no, 0, 0);
  } else {
    ppno=0;
  }
  if (lcites) { igraph_vector_init(&cites,0); } else { ppcites=0; }
  if (ldebug) { igraph_vector_init(&debug,0); } else { ppdebug=0; }
  igraph_measure_dynamics_id(&g, &ak, ppsd, ppno, ppcites, ppdebug, 
			     debugdeg, &st, maxind);
  PROTECT(result=NEW_LIST(5));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&ak));
  igraph_matrix_destroy(&ak);
  if (lsd) { 
    SET_VECTOR_ELT(result, 1, R_igraph_matrix_to_SEXP(&sd));
    igraph_matrix_destroy(&sd);
  } else {
    SET_VECTOR_ELT(result, 1, R_NilValue);
  }
  if (lno) {
    SET_VECTOR_ELT(result, 2, R_igraph_matrix_to_SEXP(&no));
    igraph_matrix_destroy(&no);
  } else {
    SET_VECTOR_ELT(result, 2, R_NilValue);
  }
  if (lcites) {    
    SET_VECTOR_ELT(result, 3, NEW_NUMERIC(igraph_vector_size(&cites)));
    igraph_vector_copy_to(&cites, REAL(VECTOR_ELT(result, 3)));
    igraph_vector_destroy(&cites);
  } else {
    SET_VECTOR_ELT(result, 3, R_NilValue);
  }
  if (ldebug) {
    SET_VECTOR_ELT(result, 4, NEW_NUMERIC(igraph_vector_size(&debug)));
    igraph_vector_copy_to(&debug, REAL(VECTOR_ELT(result, 4)));
    igraph_vector_destroy(&debug);
  } else {
    SET_VECTOR_ELT(result, 4, R_NilValue);
  }
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_id_st(SEXP graph, SEXP pak) {
  
  igraph_t g;
  igraph_matrix_t ak;
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_matrix(pak, &ak);
  igraph_vector_init(&res, 0);
  igraph_measure_dynamics_id_st(&g, &res, &ak);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_id_expected(SEXP graph, SEXP pak, 
					   SEXP pst, SEXP pmaxind, SEXP pwhich) {
  igraph_t g;
  igraph_vector_t ak, st;
  igraph_integer_t maxind=REAL(pmaxind)[0];
  igraph_vector_t err;
  igraph_integer_t which=REAL(pwhich)[0];
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pak, &ak);
  R_SEXP_to_vector(pst, &st);
  igraph_vector_init(&err, 0);
  if (which==1) {
    igraph_measure_dynamics_id_expected(&g, &err, &ak, &st, maxind);
  } else {
    igraph_measure_dynamics_id_expected2(&g, &err, &ak, &st, maxind);
  }
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&err)));
  igraph_vector_copy_to(&err, REAL(result));
  igraph_vector_destroy(&err);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_measure_dynamics_idwindow(SEXP graph,
					SEXP pst, SEXP pwindow, SEXP pmaxind,
					SEXP psd) {
  igraph_t g;
  igraph_matrix_t ak, sd;
  igraph_matrix_t *ppsd=&sd;
  igraph_vector_t st;
  igraph_bool_t lsd=LOGICAL(psd)[0];
  igraph_integer_t maxind=REAL(pmaxind)[0];
  igraph_real_t window=REAL(pwindow)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(pst, &st);
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&ak, 0, 0);
  if (lsd) {
    igraph_matrix_init(&sd, 0, 0);
  } else {
    ppsd=0;
  }
  igraph_measure_dynamics_idwindow(&g, &ak, ppsd,
				   &st, maxind, window);
  PROTECT(result=NEW_LIST(2));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&ak));
  igraph_matrix_destroy(&ak);
  if (lsd) {
    SET_VECTOR_ELT(result, 1, R_igraph_matrix_to_SEXP(&sd));
    igraph_matrix_destroy(&sd);
  }

  R_igraph_after();

  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_idwindow_st(SEXP graph, SEXP pak, 
					   SEXP pwindow) {
  igraph_t g;
  igraph_matrix_t ak;
  igraph_vector_t res;
  igraph_integer_t window=REAL(pwindow)[0];
  SEXP result;
  
  R_igraph_before();

  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_matrix(pak, &ak);
  igraph_vector_init(&res, 0);
  igraph_measure_dynamics_idwindow_st(&g, &res, &ak, window);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_idage(SEXP graph,
				     SEXP pst, SEXP pagebins,
				     SEXP pmaxind, SEXP psd, SEXP pno,
				     SEXP pcites,
				     SEXP ptime_window) {

  igraph_t g;
  igraph_matrix_t akl, sd, no, cites;
  igraph_matrix_t *ppsd=&sd, *ppno=&no, *ppcites=&cites;
  igraph_vector_t st;
  igraph_bool_t lsd=LOGICAL(psd)[0];
  igraph_bool_t lno=LOGICAL(pno)[0];
  igraph_bool_t lcites=LOGICAL(pcites)[0];
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_integer_t maxind=REAL(pmaxind)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(pst, &st);

  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&akl, 0, 0);
  if (lsd) {
    igraph_matrix_init(&sd, 0, 0);
  } else {
    ppsd=0;
  }
  if (lno) {
    igraph_matrix_init(&no, 0, 0);
  } else {
    ppno=0;
  }
  if (lcites) { igraph_matrix_init(&cites,0,0); } else { ppcites=0; }
  if (ptime_window==R_NilValue) {
    igraph_measure_dynamics_idage(&g, &akl, ppsd, ppno, ppcites,
				  &st, agebins, maxind);
  } else {
    igraph_measure_dynamics_idwindowage(&g, &akl, ppsd,
					&st, agebins, maxind,
					REAL(ptime_window)[0]);
  }
    
  PROTECT(result=NEW_LIST(4));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&akl));
  igraph_matrix_destroy(&akl);
  if (lsd) {
    SET_VECTOR_ELT(result, 1, R_igraph_matrix_to_SEXP(&sd));
    igraph_matrix_destroy(&sd);
  } else {
    SET_VECTOR_ELT(result, 1, R_NilValue);
  }
  if (lno) {
    SET_VECTOR_ELT(result, 2, R_igraph_matrix_to_SEXP(&no));
    igraph_matrix_destroy(&no);
  } else {
    SET_VECTOR_ELT(result, 2, R_NilValue);
  }
  if (lcites) {
    SET_VECTOR_ELT(result, 3, R_igraph_matrix_to_SEXP(&cites));
    igraph_matrix_destroy(&cites);
  } else {
    SET_VECTOR_ELT(result, 3, R_NilValue);
  }

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_idage_expected(SEXP graph, SEXP pakl,
					      SEXP pst, SEXP pmaxind) {
  igraph_t g;
  igraph_matrix_t akl;
  igraph_vector_t st;
  igraph_integer_t maxind=REAL(pmaxind)[0];
  igraph_matrix_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_matrix(pakl, &akl);
  R_SEXP_to_vector(pst, &st);
  igraph_matrix_init(&res, 0, 0);
  igraph_measure_dynamics_idage_expected(&g, &res, &akl, &st, maxind);
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after();

  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_idage_st(SEXP graph, SEXP pakl, 
					SEXP ptime_window) {
  
  igraph_t g;
  igraph_matrix_t akl;
  igraph_vector_t res;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_matrix(pakl, &akl);
  igraph_vector_init(&res, 0);
  if (ptime_window==R_NilValue) {    
    igraph_measure_dynamics_idage_st(&g, &res, &akl);
  } else {
    igraph_measure_dynamics_idwindowage_st(&g, &res, &akl, 
					   REAL(ptime_window)[0]);
  }
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  
  igraph_vector_destroy(&res);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_citedcat_id_age(SEXP graph,
					       SEXP pst, SEXP pcats, 
					       SEXP pcatno, SEXP pagebins,
					       SEXP pmaxind, SEXP psd) {
  igraph_t g;
  igraph_array3_t adkl, sd;
  igraph_vector_t st;
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_vector_t cats;
  igraph_integer_t catno=REAL(pcatno)[0];
  igraph_integer_t maxind=REAL(pmaxind)[0];
  igraph_bool_t lsd=LOGICAL(psd)[0];
  igraph_array3_t *ppsd=&sd;
  SEXP result;
  
  R_igraph_before();

  R_SEXP_to_vector(pst, &st);
  R_SEXP_to_vector(pcats, &cats);
  R_SEXP_to_igraph(graph, &g);
  
  igraph_array3_init(&adkl, 0, 0, 0);
  if (lsd) {    
    igraph_array3_init(&sd, 0, 0, 0);
  } else {
    ppsd=0;
  }
  igraph_measure_dynamics_citedcat_id_age(&g, &adkl, ppsd, 
					  &st, &cats, catno,
					  agebins, maxind);
  PROTECT(result=NEW_LIST(3));
  SET_VECTOR_ELT(result, 0, R_igraph_array3_to_SEXP(&adkl));
  igraph_array3_destroy(&adkl);
  if (lsd) {
    SET_VECTOR_ELT(result, 1, R_igraph_array3_to_SEXP(&sd));
    igraph_array3_destroy(&sd);
  } 
  
  R_igraph_after();

  UNPROTECT(1);
  return result;
} 

SEXP R_igraph_measure_dynamics_citedcat_id_age_st(SEXP graph, SEXP padkl,
						  SEXP pcats, SEXP pcatno) {
  igraph_t g;
  igraph_array3_t adkl;
  igraph_vector_t cats;
  igraph_integer_t catno=REAL(pcatno)[0];
  igraph_vector_t res;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_igraph_SEXP_to_array3(padkl, &adkl);
  R_SEXP_to_vector(pcats, &cats);
  igraph_vector_init(&res, 0);
  igraph_measure_dynamics_citedcat_id_age_st(&g, &res, &adkl, &cats, catno);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_citingcat_id_age(SEXP graph,
						SEXP pst, SEXP pcats, 
						SEXP pcatno, SEXP pagebins,
						SEXP pmaxind, SEXP psd) {
  igraph_t g;
  igraph_array3_t adkl, sd;
  igraph_vector_t st;
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_vector_t cats;
  igraph_integer_t catno=REAL(pcatno)[0];
  igraph_integer_t maxind=REAL(pmaxind)[0];
  igraph_bool_t lsd=LOGICAL(psd)[0];
  igraph_array3_t *ppsd=&sd;
  SEXP result;
  
  R_igraph_before();

  R_SEXP_to_vector(pst, &st);
  R_SEXP_to_vector(pcats, &cats);
  R_SEXP_to_igraph(graph, &g);
  
  igraph_array3_init(&adkl, 0, 0, 0);
  if (lsd) {
    igraph_array3_init(&sd, 0, 0, 0);
  } else {
    ppsd=0;
  }
  igraph_measure_dynamics_citingcat_id_age(&g, &adkl, ppsd, 
					   &st, &cats, catno,
					   agebins, maxind);
  PROTECT(result=NEW_LIST(3));
  SET_VECTOR_ELT(result, 0, R_igraph_array3_to_SEXP(&adkl));
  igraph_array3_destroy(&adkl);
  if (lsd) {
    SET_VECTOR_ELT(result, 1, R_igraph_array3_to_SEXP(&sd));
    igraph_array3_destroy(&sd);
  }
  
  R_igraph_after();

  UNPROTECT(1);
  return result;
} 

SEXP R_igraph_measure_dynamics_citingcat_id_age_st(SEXP graph, SEXP padkl,
						   SEXP pcats, SEXP pcatno) {
  igraph_t g;
  igraph_array3_t adkl;
  igraph_vector_t cats;
  igraph_integer_t catno=REAL(pcatno)[0];
  igraph_vector_t res;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_igraph_SEXP_to_array3(padkl, &adkl);
  R_SEXP_to_vector(pcats, &cats);
  igraph_vector_init(&res, 0);
  igraph_measure_dynamics_citingcat_id_age_st(&g, &res, &adkl, &cats, catno);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_measure_dynamics_d_d(SEXP graph, SEXP pntime, SEXP petime,
				   SEXP pevents, SEXP pst, SEXP pmaxdeg, 
				   SEXP psd) {
  igraph_t g;
  igraph_vector_t ntime, etime;
  igraph_integer_t events=REAL(pevents)[0];
  igraph_matrix_t akk, sd, *ppsd=0;
  igraph_vector_t st;
  igraph_integer_t maxdeg=REAL(pmaxdeg)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pntime, &ntime);
  R_SEXP_to_vector(petime, &etime);
  igraph_matrix_init(&akk, 0, 0);
  if (LOGICAL(psd)[0]) {
    igraph_matrix_init(&sd, 0, 0);
    ppsd=&sd;
  }
  R_SEXP_to_vector(pst, &st);
  igraph_measure_dynamics_d_d(&g, &ntime, &etime, events, &akk, 
			      ppsd, &st, maxdeg);
  PROTECT(result=NEW_LIST(3));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&akk));
  igraph_matrix_destroy(&akk);
  if (LOGICAL(psd)[0]) {
    SET_VECTOR_ELT(result, 1, R_igraph_matrix_to_SEXP(&sd));
    igraph_matrix_destroy(&sd);
  } else {
    SET_VECTOR_ELT(result, 1, R_NilValue);
  }
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
} 

SEXP R_igraph_measure_dynamics_d_d_st(SEXP graph, SEXP pntime, SEXP petime,
				      SEXP pakk, SEXP pevents, 
				      SEXP pmaxtotaldeg) {
  igraph_t g;
  igraph_vector_t ntime, etime;
  igraph_matrix_t akk;
  igraph_integer_t events=REAL(pevents)[0];
  igraph_integer_t maxtotaldeg=REAL(pmaxtotaldeg)[0];
  igraph_vector_t st;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pntime, &ntime);
  R_SEXP_to_vector(petime, &etime);
  R_SEXP_to_matrix(pakk, &akk);
  igraph_vector_init(&st, 0);
  igraph_measure_dynamics_d_d_st(&g, &ntime, &etime, &akk,
				 events, maxtotaldeg, &st);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&st)));
  igraph_vector_copy_to(&st, REAL(result));
  igraph_vector_destroy(&st);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_lastcit(SEXP graph, SEXP pst,
				       SEXP pagebins, SEXP psd, SEXP pno) {
  igraph_t g;
  igraph_vector_t al, sd, no;
  igraph_vector_t *ppsd=&sd, *ppno=&no;
  igraph_vector_t st;
  igraph_bool_t lsd=LOGICAL(psd)[0];
  igraph_bool_t lno=LOGICAL(pno)[0];
  igraph_integer_t agebins=REAL(pagebins)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(pst, &st);
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&al, 0);
  if (lsd) {
    igraph_vector_init(&sd, 0);
  } else {
    ppsd=0;
  }
  if (lno) {
    igraph_vector_init(&no, 0);
  } else {
    ppno=0;
  }
  igraph_measure_dynamics_lastcit(&g, &al, ppsd, ppno, &st, agebins);
  PROTECT(result=NEW_LIST(3));
  SET_VECTOR_ELT(result, 0, NEW_NUMERIC(igraph_vector_size(&al)));
  igraph_vector_copy_to(&al, REAL(VECTOR_ELT(result, 0)));
  igraph_vector_destroy(&al);
  if (lsd) {
    SET_VECTOR_ELT(result, 1, NEW_NUMERIC(igraph_vector_size(&sd)));
    igraph_vector_copy_to(&sd, REAL(VECTOR_ELT(result, 1)));
    igraph_vector_destroy(&sd);
  } else {
    SET_VECTOR_ELT(result, 1, R_NilValue);
  }
  if (lno) {
    SET_VECTOR_ELT(result, 2, NEW_NUMERIC(igraph_vector_size(&no)));
    igraph_vector_copy_to(&no, REAL(VECTOR_ELT(result, 2)));
    igraph_vector_destroy(&no);
  } else {
    SET_VECTOR_ELT(result, 2, R_NilValue);
  }
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_lastcit_st(SEXP graph, SEXP pal) {
  
  igraph_t g;
  igraph_vector_t al;
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pal, &al);
  igraph_vector_init(&res, 0);
  igraph_measure_dynamics_lastcit_st(&g, &res, &al);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_age(SEXP graph, SEXP pst, SEXP pagebins,
				   SEXP psd, SEXP pno) {
  igraph_t g;
  igraph_vector_t al, sd, no;
  igraph_vector_t *ppsd=&sd, *ppno=&no;
  igraph_vector_t st;
  igraph_bool_t lsd=LOGICAL(psd)[0];
  igraph_bool_t lno=LOGICAL(pno)[0];
  igraph_integer_t agebins=REAL(pagebins)[0];
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_vector(pst, &st);
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&al, 0);
  if (lsd) {
    igraph_vector_init(&sd, 0);
  } else {
    ppsd=0;
  }
  if (lno) {
    igraph_vector_init(&no, 0);
  } else {
    ppno=0;
  }
  igraph_measure_dynamics_age(&g, &al, ppsd, ppno, &st, agebins);
  PROTECT(result=NEW_LIST(3));
  SET_VECTOR_ELT(result, 0, NEW_NUMERIC(igraph_vector_size(&al)));
  igraph_vector_copy_to(&al, REAL(VECTOR_ELT(result, 0)));
  igraph_vector_destroy(&al);
  if (lsd) {
    SET_VECTOR_ELT(result, 1, NEW_NUMERIC(igraph_vector_size(&sd)));
    igraph_vector_copy_to(&sd, REAL(VECTOR_ELT(result, 1)));
    igraph_vector_destroy(&sd);
  } else {
    SET_VECTOR_ELT(result, 1, R_NilValue);
  }
  if (lno) {
    SET_VECTOR_ELT(result, 2, NEW_NUMERIC(igraph_vector_size(&no)));
    igraph_vector_copy_to(&no, REAL(VECTOR_ELT(result, 2)));
    igraph_vector_destroy(&no);
  } else {
    SET_VECTOR_ELT(result, 2, R_NilValue);
  }
  
  R_igraph_after();

  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_age_st(SEXP graph, SEXP pal) {
  
  igraph_t g;
  igraph_vector_t al;
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pal, &al);
  igraph_vector_init(&res, 0);
  igraph_measure_dynamics_age_st(&g, &res, &al);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_citedcat(SEXP graph, SEXP pcats,
					SEXP pnocats, SEXP pst, SEXP psd, SEXP pno) {
  igraph_t g;
  igraph_vector_t al, sd, no;
  igraph_vector_t *ppsd=&sd, *ppno=&no;
  igraph_vector_t st;
  igraph_vector_t cats;
  igraph_bool_t lsd=LOGICAL(psd)[0];
  igraph_bool_t lno=LOGICAL(pno)[0];
  igraph_integer_t nocats=REAL(pnocats)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(pst, &st);
  R_SEXP_to_vector(pcats, &cats);
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&al, 0);
  if (lsd) {
    igraph_vector_init(&sd,0);
  } else {
    ppsd=0;
  }
  if (lno) {
    igraph_vector_init(&no, 0); 
  } else {
    ppno=0;
  }
  igraph_measure_dynamics_citedcat(&g, &cats, nocats, &al, ppsd, ppno, &st);
  PROTECT(result=NEW_LIST(3));
  SET_VECTOR_ELT(result, 0, NEW_NUMERIC(igraph_vector_size(&al)));
  igraph_vector_copy_to(&al, REAL(VECTOR_ELT(result, 0)));
  igraph_vector_destroy(&al);
  if (lsd) {
    SET_VECTOR_ELT(result, 1, NEW_NUMERIC(igraph_vector_size(&sd)));
    igraph_vector_copy_to(&sd, REAL(VECTOR_ELT(result, 1)));
    igraph_vector_destroy(&sd);
  } else {
    SET_VECTOR_ELT(result, 1, R_NilValue);
  }
  if (lno) {
    SET_VECTOR_ELT(result, 2, NEW_NUMERIC(igraph_vector_size(&no)));
    igraph_vector_copy_to(&no, REAL(VECTOR_ELT(result, 2)));
    igraph_vector_destroy(&no);
  } else {
    SET_VECTOR_ELT(result, 2, R_NilValue);
  }
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_citedcat_st(SEXP graph, SEXP pal, SEXP pcats,
					   SEXP pnocats) {
  igraph_t g;
  igraph_vector_t al, cats;
  igraph_integer_t nocats=REAL(pnocats)[0];
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pal, &al);
  R_SEXP_to_vector(pcats, &cats);
  igraph_vector_init(&res, 0);
  igraph_measure_dynamics_citedcat_st(&g, &res, &al, &cats, nocats);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_citingcat_citedcat(SEXP graph, SEXP pcats,
						  SEXP pnocats, SEXP pst,
						  SEXP psd, SEXP pno) {
  igraph_t g;
  igraph_vector_t cats;
  igraph_integer_t nocats=REAL(pnocats)[0];
  igraph_vector_t st;
  igraph_matrix_t agd, sd, no;
  igraph_matrix_t *ppsd=&sd, *ppno=&no;
  igraph_bool_t lsd=LOGICAL(psd)[0];
  igraph_bool_t lno=LOGICAL(pno)[0];
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_vector(pcats, &cats);
  R_SEXP_to_vector(pst, &st);
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&agd, 0, 0);
  if (lsd) { igraph_matrix_init(&sd, 0, 0); } else { ppsd=0; }
  if (lno) { igraph_matrix_init(&no, 0, 0); } else { ppno=0; }
  igraph_measure_dynamics_citingcat_citedcat(&g, &agd, ppsd, 
					     ppno, &st, &cats, nocats);
  PROTECT(result=NEW_LIST(3));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&agd));
  igraph_matrix_destroy(&agd);
  if (lsd) {
    SET_VECTOR_ELT(result, 1, R_igraph_matrix_to_SEXP(&sd));
    igraph_matrix_destroy(&sd);
  } else {
    SET_VECTOR_ELT(result, 1, R_NilValue);
  }
  if (lno) {
    SET_VECTOR_ELT(result, 2, R_igraph_matrix_to_SEXP(&no));
    igraph_matrix_destroy(&no);
  } else {
    SET_VECTOR_ELT(result, 2, R_NilValue);
  }
  
  R_igraph_after();

  UNPROTECT(1);
  return result;
}

SEXP R_igraph_measure_dynamics_citingcat_citedcat_st(SEXP graph, 
						     SEXP pagd, SEXP pcats,
						     SEXP pnocats) {
  igraph_t g;
  igraph_matrix_t agd;
  igraph_vector_t cats;
  igraph_integer_t nocats=REAL(pnocats)[0];
  igraph_vector_t res;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_matrix(pagd, &agd);
  R_SEXP_to_vector(pcats, &cats);
  igraph_vector_init(&res, 0);
  igraph_measure_dynamics_citingcat_citedcat_st(&g, &res, &agd, &cats, nocats);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_get_shortest_paths(SEXP graph, SEXP pfrom, SEXP pto, 
				 SEXP pmode, SEXP pno, SEXP weights) {

  igraph_t g;
  igraph_integer_t from=REAL(pfrom)[0];
  igraph_vs_t to;
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_vector_t *vects;
  long int i;
  igraph_vector_ptr_t ptrvec;
  igraph_vector_t w, *pw=&w;
  SEXP result;
  
  long int no=REAL(pno)[0];
  
  R_igraph_before();  

  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pto, &g, &to);

  igraph_vector_ptr_init(&ptrvec, no);
  vects=(igraph_vector_t*) R_alloc(GET_LENGTH(pto), sizeof(igraph_vector_t));
  for (i=0; i<no; i++) {
    igraph_vector_init(&vects[i], 0);
    VECTOR(ptrvec)[i]=&vects[i];
  }
  if (isNull(weights)) {
    pw=0;
  } else {
    R_SEXP_to_vector(weights, &w);
  }
  igraph_get_shortest_paths_dijkstra(&g, &ptrvec, from, to, pw, mode);
  PROTECT(result=NEW_LIST(no));
  for (i=0; i<no; i++) {
    SET_VECTOR_ELT(result, i, NEW_NUMERIC(igraph_vector_size(&vects[i])));
    igraph_vector_copy_to(&vects[i], REAL(VECTOR_ELT(result, i)));
    igraph_vector_destroy(&vects[i]);
  }
  igraph_vector_ptr_destroy(&ptrvec);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_are_connected(SEXP graph, SEXP pv1, SEXP pv2) {
  
  igraph_t g;
  igraph_integer_t v1=REAL(pv1)[0];
  igraph_integer_t v2=REAL(pv2)[0];
  igraph_bool_t res;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  PROTECT(result=NEW_LOGICAL(1));
  igraph_are_connected(&g, v1, v2, &res);
  LOGICAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_graph_adjacency(SEXP adjmatrix, SEXP pmode) {
  
  igraph_t g;
  igraph_matrix_t adjm;
  igraph_integer_t mode=REAL(pmode)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_matrix(adjmatrix, &adjm);
  igraph_adjacency(&g, &adjm, mode);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_average_path_length(SEXP graph, SEXP pdirected, 
				  SEXP punconnected) {
  
  igraph_t g;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_bool_t unconnected=LOGICAL(punconnected)[0];
  igraph_real_t res;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_average_path_length(&g, &res, directed, unconnected);
  
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_star(SEXP pn, SEXP pmode, SEXP pcenter) {

  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_integer_t center=REAL(pcenter)[0];
  SEXP result;
  
  R_igraph_before();
  
  igraph_star(&g, n, mode, center);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_ring(SEXP pn, SEXP pdirected, SEXP pmutual, SEXP pcircular) {

  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_bool_t mutual=LOGICAL(pmutual)[0];
  igraph_bool_t circular=LOGICAL(pcircular)[0];
  SEXP result;
  
  R_igraph_before();
  
  igraph_ring(&g, n, directed, mutual, circular);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_tree(SEXP pn, SEXP pchildren, SEXP pmode) {
  
  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_integer_t children=REAL(pchildren)[0];
  igraph_integer_t mode=REAL(pmode)[0];
  SEXP result;

  R_igraph_before();
  
  igraph_tree(&g, n, children, mode);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_subgraph(SEXP graph, SEXP pvids) {
  
  igraph_t g;
  igraph_t sub;
  igraph_vs_t vs;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pvids, &g, &vs);
  igraph_subgraph(&g, &sub, vs);
  PROTECT(result=R_igraph_to_SEXP(&sub));
  igraph_destroy(&sub);
  igraph_vs_destroy(&vs);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_layout_random(SEXP graph) {
  
  igraph_t g;
  igraph_matrix_t res;
  SEXP result=R_NilValue;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&res, 0, 0);
  igraph_layout_random(&g, &res);
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_layout_circle(SEXP graph) {
  
  igraph_t g;
  igraph_matrix_t res;
  SEXP result=R_NilValue;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&res, 0, 0);
  igraph_layout_circle(&g, &res);
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_erdos_renyi_game(SEXP pn, SEXP ptype,
			       SEXP pporm, SEXP pdirected, SEXP ploops) {
  
  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_integer_t type=REAL(ptype)[0];
  igraph_real_t porm=REAL(pporm)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_bool_t loops=LOGICAL(ploops)[0];
  SEXP result;
  
  R_igraph_before();
  
  igraph_erdos_renyi_game(&g, type, n, porm, directed, loops);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_full(SEXP pn, SEXP pdirected, SEXP ploops) {
  
  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_bool_t loops=LOGICAL(ploops)[0];
  SEXP result;
  
  R_igraph_before();
  
  igraph_full(&g, n, directed, loops);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_random_sample(SEXP plow, SEXP phigh, SEXP plength) {
  
  igraph_vector_t res;
  igraph_integer_t low=REAL(plow)[0];
  igraph_integer_t high=REAL(phigh)[0];
  igraph_integer_t length=REAL(plength)[0];
  SEXP result;

  R_igraph_before();
  
  igraph_vector_init(&res, 0);
  igraph_random_sample(&res, low, high, length);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_get_edgelist(SEXP graph, SEXP pbycol) {
  
  igraph_t g;
  igraph_vector_t res;
  igraph_bool_t bycol=LOGICAL(pbycol)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&res, 0);
  igraph_get_edgelist(&g, &res, bycol);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_get_adjacency(SEXP graph, SEXP ptype) {
  
  igraph_t g;
  igraph_matrix_t res;
  igraph_integer_t type=REAL(ptype)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&res, 0, 0);
  igraph_get_adjacency(&g, &res, type);
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_simplify(SEXP graph, SEXP pmultiple, SEXP ploops) {
  
  igraph_t g;
  igraph_bool_t multiple=LOGICAL(pmultiple)[0];
  igraph_bool_t loops=LOGICAL(ploops)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph_copy(graph, &g);
  igraph_simplify(&g, multiple, loops);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_layout_fruchterman_reingold(SEXP graph, SEXP pniter, 
					  SEXP pmaxdelta, SEXP parea,
					  SEXP pcoolexp, SEXP prepulserad,
					  SEXP pweights, SEXP start,
					  SEXP verbose) {
  igraph_t g;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_real_t maxdelta=REAL(pmaxdelta)[0];
  igraph_real_t area=REAL(parea)[0];
  igraph_real_t coolexp=REAL(pcoolexp)[0];
  igraph_real_t repulserad=REAL(prepulserad)[0];
  igraph_vector_t weights, *ppweights=0;
  igraph_matrix_t res;
  SEXP result;
  
  R_igraph_before2(verbose, "3D FR layout");
  
  R_SEXP_to_igraph(graph, &g);
  if (!isNull(pweights)) { R_SEXP_to_vector(pweights, &weights);ppweights=&weights; }
  if (isNull(start)) {
    igraph_matrix_init(&res, 0, 0);
  } else {
    R_SEXP_to_igraph_matrix_copy(start, &res);
  }
  igraph_layout_fruchterman_reingold(&g, &res, niter, maxdelta, area, 
				     coolexp, repulserad, !isNull(start),
				     ppweights);
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after2(verbose);
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_layout_fruchterman_reingold_3d(SEXP graph, SEXP pniter, 
					     SEXP pmaxdelta, SEXP parea,
					     SEXP pcoolexp, SEXP prepulserad,
					     SEXP pweights,
					     SEXP verbose) {
  igraph_t g;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_real_t maxdelta=REAL(pmaxdelta)[0];
  igraph_real_t area=REAL(parea)[0];
  igraph_real_t coolexp=REAL(pcoolexp)[0];
  igraph_real_t repulserad=REAL(prepulserad)[0];
  igraph_vector_t weights, *ppweights=0;
  igraph_matrix_t res;
  SEXP result;
  
  R_igraph_before2(verbose, "3D FR layout");
  
  R_SEXP_to_igraph(graph, &g);
  if (!isNull(pweights)) { R_SEXP_to_vector(pweights, &weights);ppweights=&weights; }
  igraph_matrix_init(&res, 0, 0);
  igraph_layout_fruchterman_reingold_3d(&g, &res, niter, maxdelta, area, 
					coolexp, repulserad, 0, ppweights);
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after2(verbose);
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_degree_sequence_game(SEXP pout_seq, SEXP pin_seq, 
				   SEXP pmethod) {
  igraph_t g;
  igraph_vector_t outseq;
  igraph_vector_t inseq;
  igraph_integer_t method=REAL(pmethod)[0];
  SEXP result;

  R_igraph_before();

  R_SEXP_to_vector(pout_seq, &outseq);
  R_SEXP_to_vector(pin_seq, &inseq);
  igraph_degree_sequence_game(&g, &outseq, &inseq, method);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_transitivity_undirected(SEXP graph) {
  
  igraph_t g;
  igraph_real_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_transitivity_undirected(&g, &res);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_transitivity_avglocal_undirected(SEXP graph) {
  
  igraph_t g;
  igraph_real_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_transitivity_avglocal_undirected(&g, &res);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_transitivity_local_undirected(SEXP graph, SEXP pvids) {
  
  igraph_t g;
  igraph_vs_t vs;
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pvids, &g, &vs);
  igraph_vector_init(&res, 0);
  igraph_transitivity_local_undirected(&g, &res, vs);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  igraph_vs_destroy(&vs);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_transitivity_local_undirected_all(SEXP graph) {
  
  igraph_t g;
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&res, 0);
  igraph_transitivity_local_undirected(&g, &res, igraph_vss_all());
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_read_graph_edgelist(SEXP pvfile, SEXP pn, SEXP pdirected) {
  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  FILE *file;
  SEXP result;
  
  R_igraph_before();
  
#if HAVE_FMEMOPEN == 1
  file=fmemopen(RAW(pvfile), GET_LENGTH(pvfile), "r");
#else 
  file=fopen(CHAR(STRING_ELT(pvfile, 0)), "r");
#endif
  if (file==0) { igraph_error("Cannot read edgelist", __FILE__, __LINE__,
			      IGRAPH_EFILE); }
  igraph_read_graph_edgelist(&g, file, n, directed);
  fclose(file);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;  
}

SEXP R_igraph_read_graph_gml(SEXP pvfile) {

  igraph_t g;
  FILE *file;
  SEXP result;
  
  R_igraph_before();
  
#if HAVE_FMEMOPEN == 1
  file=fmemopen(RAW(pvfile), GET_LENGTH(pvfile), "r");
#else 
  file=fopen(CHAR(STRING_ELT(pvfile, 0)), "r");
#endif
  if (file==0) { igraph_error("Cannot read GML file", __FILE__, __LINE__,
			      IGRAPH_EFILE); }
  igraph_read_graph_gml(&g, file);
  fclose(file);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;  
}

SEXP R_igraph_read_graph_graphdb(SEXP pvfile, SEXP pdirected) {
  igraph_t g;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  FILE *file;
  SEXP result;
  
  R_igraph_before();
  
#if HAVE_FMEMOPEN == 1
  file=fmemopen(RAW(pvfile), GET_LENGTH(pvfile), "rb");
#else
  file=fopen(CHAR(STRING_ELT(pvfile, 0)), "rb");
#endif
  if (file==0) { igraph_error("Cannot read graphdb file", __FILE__, __LINE__,
			      IGRAPH_EFILE); }
  igraph_read_graph_graphdb(&g, file, directed);
  fclose(file);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_write_graph_edgelist(SEXP graph, SEXP file) {
  igraph_t g;
  FILE *stream;
  char *bp;
  size_t size;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
#if HAVE_OPEN_MEMSTREAM == 1
  stream=open_memstream(&bp, &size);
#else
  stream=fopen(CHAR(STRING_ELT(file, 0)), "w");
#endif
  if (stream==0) { igraph_error("Cannot write edgelist", __FILE__, __LINE__,
				IGRAPH_EFILE); }
  igraph_write_graph_edgelist(&g, stream);
  fclose(stream);
#if HAVE_OPEN_MEMSTREAM == 1
  PROTECT(result=allocVector(RAWSXP, size));
  memcpy(RAW(result), bp, sizeof(char)*size);
  free(bp);
#else 
  PROTECT(result=NEW_NUMERIC(0));
#endif
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_write_graph_gml(SEXP graph, SEXP file, SEXP pid, SEXP pcreator) {
  igraph_t g;
  FILE *stream;
  char *bp;
  size_t size;
  igraph_vector_t id, *ppid=0;
  const char *creator=0;
  SEXP result;
  
  R_igraph_before();
  
  if (!isNull(pid)) { R_SEXP_to_vector(pid, &id); ppid=&id; }
  if (!isNull(pcreator)) { creator=CHAR(STRING_ELT(pcreator, 0)); } 
  R_SEXP_to_igraph(graph, &g);
#if HAVE_OPEN_MEMSTREAM == 1
  stream=open_memstream(&bp, &size);
#else
  stream=fopen(CHAR(STRING_ELT(file, 0)), "w");
#endif
  if (stream==0) { igraph_error("Cannot write edgelist", __FILE__, __LINE__,
				IGRAPH_EFILE); }
  igraph_write_graph_gml(&g, stream, ppid, creator);
  fclose(stream);
#if HAVE_OPEN_MEMSTREAM == 1
  PROTECT(result=allocVector(RAWSXP, size));
  memcpy(RAW(result), bp, sizeof(char)*size);
  free(bp);
#else 
  PROTECT(result=NEW_NUMERIC(0));
#endif
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_write_graph_dot(SEXP graph, SEXP file) {
  igraph_t g;
  FILE *stream;
  char *bp;
  size_t size;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
#if HAVE_OPEN_MEMSTREAM == 1
  stream=open_memstream(&bp, &size);
#else
  stream=fopen(CHAR(STRING_ELT(file, 0)), "w");
#endif
  if (stream==0) { igraph_error("Cannot write edgelist", __FILE__, __LINE__,
				IGRAPH_EFILE); }
  igraph_write_graph_dot(&g, stream);
  fclose(stream);
#if HAVE_OPEN_MEMSTREAM == 1
  PROTECT(result=allocVector(RAWSXP, size));
  memcpy(RAW(result), bp, sizeof(char)*size);
  free(bp);
#else 
  PROTECT(result=NEW_NUMERIC(0));
#endif
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_write_graph_pajek(SEXP graph, SEXP file) {
  
  igraph_t g;
  FILE *stream;
  char *bp;
  size_t size;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
#if HAVE_OPEN_MEMSTREAM == 1
  stream=open_memstream(&bp, &size);
#else
  stream=fopen(CHAR(STRING_ELT(file, 0)), "wb");
#endif
  if (stream==0) { igraph_error("Cannot write oajek file", __FILE__, __LINE__,
				IGRAPH_EFILE); }
  igraph_write_graph_pajek(&g, stream);
  fclose(stream);
#if HAVE_OPEN_MEMSTREAM == 1
  PROTECT(result=allocVector(RAWSXP, size));
  memcpy(RAW(result), bp, sizeof(char)*size);
  free(bp);
#else 
  PROTECT(result=NEW_NUMERIC(0));
#endif
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_read_graph_ncol(SEXP pvfile, SEXP ppredef,
			      SEXP pnames, SEXP pweights,
			      SEXP pdirected) {
  igraph_t g;
  igraph_bool_t names=LOGICAL(pnames)[0];
  igraph_bool_t weights=LOGICAL(pweights)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  FILE *file;
  igraph_strvector_t predef, *predefptr=0;  
  SEXP result;
  
  R_igraph_before();
  
#if HAVE_FMEMOPEN == 1
  file=fmemopen(RAW(pvfile), GET_LENGTH(pvfile), "r");
#else 
  file=fopen(CHAR(STRING_ELT(pvfile, 0)), "r");
#endif
  if (file==0) { igraph_error("Cannot read edgelist", __FILE__, __LINE__,
			      IGRAPH_EFILE); }
  if (GET_LENGTH(ppredef)>0) {
    R_igraph_SEXP_to_strvector(ppredef, &predef);
    predefptr=&predef;
  } 
  igraph_read_graph_ncol(&g, file, predefptr, names, weights, directed);
  fclose(file);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;  
}

SEXP R_igraph_write_graph_ncol(SEXP graph, SEXP file, SEXP pnames, 
			       SEXP pweights) {
  igraph_t g;
  FILE *stream;
  char *bp;
  size_t size;
  const char *names, *weights;
  SEXP result;

  R_igraph_before();
  
  if (isNull(pnames)) {
    names=0; 
  } else {
    names=CHAR(STRING_ELT(pnames, 0));
  } 
  if (isNull(pweights)) {
    weights=0; 
  } else {
    weights=CHAR(STRING_ELT(pweights, 0));
  }   

  R_SEXP_to_igraph(graph, &g);
#if HAVE_OPEN_MEMSTREAM == 1
  stream=open_memstream(&bp, &size);
#else 
  stream=fopen(CHAR(STRING_ELT(file,0)), "w");
#endif
  if (stream==0) { igraph_error("Cannot write .ncol file", __FILE__, __LINE__,
				IGRAPH_EFILE); }
  igraph_write_graph_ncol(&g, stream, names, weights);
  fclose(stream);
#if HAVE_OPEN_MEMSTREAM == 1
  PROTECT(result=allocVector(RAWSXP, size));
  memcpy(RAW(result), bp, sizeof(char)*size);
  free(bp);
#else
  PROTECT(result=NEW_NUMERIC(0));
#endif
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_read_graph_lgl(SEXP pvfile, SEXP pnames, SEXP pweights) {
  igraph_t g;
  igraph_bool_t names=LOGICAL(pnames)[0];
  igraph_bool_t weights=LOGICAL(pweights)[0];
  FILE *file;
  SEXP result;
  
  R_igraph_before();
  
#if HAVE_FMEMOPEN == 1
  file=fmemopen(RAW(pvfile), GET_LENGTH(pvfile), "r");
#else 
  file=fopen(CHAR(STRING_ELT(pvfile, 0)), "r");
#endif
  if (file==0) { igraph_error("Cannot read edgelist", __FILE__, __LINE__,
			      IGRAPH_EFILE); }
  igraph_read_graph_lgl(&g, file, names, weights);
  fclose(file);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;  
}

SEXP R_igraph_write_graph_lgl(SEXP graph, SEXP file, SEXP pnames, 
			      SEXP pweights, SEXP pisolates) {
  igraph_t g;
  FILE *stream;
  char *bp;
  size_t size;
  const char *names, *weights;
  igraph_bool_t isolates=LOGICAL(pisolates)[0];
  SEXP result;

  R_igraph_before();
  
  if (isNull(pnames)) {
    names=0; 
  } else {
    names=CHAR(STRING_ELT(pnames, 0));
  } 
  if (isNull(pweights)) {
    weights=0; 
  } else {
    weights=CHAR(STRING_ELT(pweights, 0));
  }   

  R_SEXP_to_igraph(graph, &g);
#if HAVE_OPEN_MEMSTREAM == 1
  stream=open_memstream(&bp, &size);
#else
  stream=fopen(CHAR(STRING_ELT(file, 0)), "w");
#endif
  igraph_write_graph_lgl(&g, stream, names, weights, isolates);
  fclose(stream);
#if HAVE_OPEN_MEMSTREAM == 1
  PROTECT(result=allocVector(RAWSXP, size));
  memcpy(RAW(result), bp, sizeof(char)*size);
  free(bp);
#else
  PROTECT(result=NEW_NUMERIC(0));
#endif
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_read_graph_pajek(SEXP pvfile) {
  igraph_t g;
  FILE *file;  
  SEXP result;
  
  R_igraph_before();
  
#if HAVE_FMEMOPEN == 1
  file=fmemopen(RAW(pvfile), GET_LENGTH(pvfile), "r");
#else
  file=fopen(CHAR(STRING_ELT(pvfile, 0)), "r");
#endif
  if (file==0) { igraph_error("Cannot read Pajek file", __FILE__, __LINE__,
			      IGRAPH_EFILE); }
  igraph_read_graph_pajek(&g, file);
  fclose(file);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
} 

SEXP R_igraph_decompose(SEXP graph, SEXP pmode, SEXP pmaxcompno, 
			SEXP pminelements) {

  igraph_t g;
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_integer_t maxcompno=REAL(pmaxcompno)[0];
  igraph_integer_t minelements=REAL(pminelements)[0];
  igraph_vector_ptr_t comps;
  SEXP result;
  long int i;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_ptr_init(&comps, 0);
  IGRAPH_FINALLY(igraph_vector_ptr_destroy, &comps);
  igraph_decompose(&g, &comps, mode, maxcompno, minelements);
  PROTECT(result=NEW_LIST(igraph_vector_ptr_size(&comps)));
  for (i=0; i<igraph_vector_ptr_size(&comps); i++) {
    SET_VECTOR_ELT(result, i, R_igraph_to_SEXP(VECTOR(comps)[i]));
    igraph_destroy(VECTOR(comps)[i]);
    igraph_free(VECTOR(comps)[i]);
  }
  igraph_vector_ptr_destroy(&comps);
  IGRAPH_FINALLY_CLEAN(1);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;  
}

SEXP R_igraph_atlas(SEXP pno) {
  
  int no=REAL(pno)[0];
  igraph_t g;
  SEXP result;
  
  R_igraph_before();

  igraph_atlas(&g, no);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_layout_random_3d(SEXP graph) {
  
  igraph_t g;
  igraph_matrix_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&res, 0, 0);
  igraph_layout_random_3d(&g, &res);
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);  
  
  R_igraph_after();

  UNPROTECT(1);
  return result;
}

SEXP R_igraph_layout_sphere(SEXP graph) {

  igraph_t g;
  igraph_matrix_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&res, 0, 0);
  igraph_layout_sphere(&g, &res);
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_callaway_traits_game(SEXP pnodes, SEXP ptypes, 
				  SEXP pepers, SEXP ptype_dist,
				  SEXP pmatrix, SEXP pdirected) {

  igraph_t g;
  igraph_integer_t nodes=REAL(pnodes)[0];
  igraph_integer_t types=REAL(ptypes)[0];
  igraph_integer_t epers=REAL(pepers)[0];
  igraph_vector_t type_dist;
  igraph_matrix_t matrix;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  SEXP result; 

  R_igraph_before();
  
  R_SEXP_to_vector(ptype_dist, &type_dist);
  R_SEXP_to_matrix(pmatrix, &matrix);
  igraph_callaway_traits_game(&g, nodes, types, epers, &type_dist,
			      &matrix, directed);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_establishment_game(SEXP pnodes, SEXP ptypes, SEXP pk,
				 SEXP ptype_dist, SEXP pmatrix,
				 SEXP pdirected) {
  igraph_t g;
  igraph_integer_t nodes=REAL(pnodes)[0];
  igraph_integer_t types=REAL(ptypes)[0];
  igraph_integer_t k=REAL(pk)[0];
  igraph_vector_t type_dist;
  igraph_matrix_t matrix;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(ptype_dist, &type_dist);
  R_SEXP_to_matrix(pmatrix, &matrix);
  igraph_establishment_game(&g, nodes, types, k, &type_dist, &matrix,
			    directed);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
			    
SEXP R_igraph_motifs_randesu(SEXP graph, SEXP psize, SEXP pcutprob) {
  igraph_t g;
  igraph_integer_t size=REAL(psize)[0];
  igraph_vector_t cutprob;
  igraph_vector_t res;
  SEXP result;

  R_igraph_before();

  R_SEXP_to_vector(pcutprob, &cutprob);
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&res, 0);
  igraph_motifs_randesu(&g, &res, size, &cutprob);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_motifs_randesu_no(SEXP graph, SEXP psize, SEXP pcutprob) {
  igraph_t g;
  igraph_integer_t size=REAL(psize)[0];
  igraph_vector_t cutprob;
  igraph_integer_t res;
  SEXP result;

  R_igraph_before();

  R_SEXP_to_vector(pcutprob, &cutprob);
  R_SEXP_to_igraph(graph, &g);
  igraph_motifs_randesu_no(&g, &res, size, &cutprob);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_motifs_randesu_estimate(SEXP graph, SEXP psize, SEXP pcutprob,
				      SEXP psamplesize, SEXP psample) {
  igraph_t g;
  igraph_integer_t size=REAL(psize)[0];
  igraph_vector_t cutprob;
  igraph_integer_t samplesize=REAL(psamplesize)[0];
  igraph_vector_t sample;
  igraph_vector_t *sampleptr=0;
  igraph_integer_t res;
  SEXP result;
  
  R_igraph_before();

  R_SEXP_to_vector(pcutprob, &cutprob);
  if (GET_LENGTH(psample) != 0) {
    R_SEXP_to_vector(psample, &sample);
    sampleptr=&sample;
  }
  R_SEXP_to_igraph(graph, &g);
  igraph_motifs_randesu_estimate(&g, &res, size, &cutprob, samplesize,
				 sampleptr);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_get_all_shortest_paths(SEXP graph, SEXP pfrom, SEXP pto,
				     SEXP pmode) {
  
  igraph_t g;
  igraph_integer_t from=REAL(pfrom)[0];
  igraph_vs_t to;
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_vector_ptr_t res;
  SEXP result;
  long int i;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pto, &g, &to);
  igraph_vector_ptr_init(&res, 0);
  igraph_get_all_shortest_paths(&g, &res, 0, from, to, mode);
  PROTECT(result=NEW_LIST(igraph_vector_ptr_size(&res)));
  for (i=0; i<igraph_vector_ptr_size(&res); i++) {
    long int len=igraph_vector_size(VECTOR(res)[i]);
    SET_VECTOR_ELT(result, i, NEW_NUMERIC(len));
    igraph_vector_copy_to(VECTOR(res)[i], REAL(VECTOR_ELT(result, i)));
    igraph_vector_destroy(VECTOR(res)[i]);
    igraph_free(VECTOR(res)[i]);
  }
  igraph_vector_ptr_destroy(&res);
  igraph_vs_destroy(&to);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_layout_merge_dla(SEXP graphs, SEXP layouts, SEXP verbose) {

  igraph_vector_ptr_t graphvec;
  igraph_vector_ptr_t ptrvec;
  igraph_t *gras;
  igraph_matrix_t *mats;
  igraph_matrix_t res;
  long int i;
  SEXP result;
  
  R_igraph_before2(verbose, "Merge layouts");
  
  igraph_vector_ptr_init(&graphvec, GET_LENGTH(graphs));
  igraph_vector_ptr_init(&ptrvec, GET_LENGTH(layouts));
  gras=(igraph_t*)R_alloc(GET_LENGTH(graphs), sizeof(igraph_t));
  mats=(igraph_matrix_t*)R_alloc(GET_LENGTH(layouts),
				 sizeof(igraph_matrix_t));
  for (i=0; i<GET_LENGTH(graphs); i++) {
    R_SEXP_to_igraph(VECTOR_ELT(graphs, i), &gras[i]);
    VECTOR(graphvec)[i]=&gras[i];
  }
  for (i=0; i<GET_LENGTH(layouts); i++) {
    R_SEXP_to_matrix(VECTOR_ELT(layouts, i), &mats[i]);
    VECTOR(ptrvec)[i]=&mats[i];
  }
  igraph_matrix_init(&res, 0, 0);
  igraph_layout_merge_dla(&graphvec, &ptrvec, &res);
  igraph_vector_ptr_destroy(&graphvec);
  igraph_vector_ptr_destroy(&ptrvec);
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after2(verbose);
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_disjoint_union(SEXP pgraphs) {
  
  igraph_vector_ptr_t ptrvec;
  igraph_t *graphs;
  igraph_t res;
  long int i;
  SEXP result;

  R_igraph_before();
  
  igraph_vector_ptr_init(&ptrvec, GET_LENGTH(pgraphs));
  graphs=(igraph_t *)R_alloc(GET_LENGTH(pgraphs),
			     sizeof(igraph_t));
  for (i=0; i<GET_LENGTH(pgraphs); i++) {
    R_SEXP_to_igraph(VECTOR_ELT(pgraphs, i), &graphs[i]);
    VECTOR(ptrvec)[i]=&graphs[i];
  }
  igraph_disjoint_union_many(&res, &ptrvec);
  igraph_vector_ptr_destroy(&ptrvec);
  PROTECT(result=R_igraph_to_SEXP(&res));
  igraph_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_union(SEXP pgraphs) {
  
  igraph_vector_ptr_t ptrvec;
  igraph_t *graphs;
  igraph_t res;
  long int i;
  SEXP result;

  R_igraph_before();
  
  igraph_vector_ptr_init(&ptrvec, GET_LENGTH(pgraphs));
  graphs=(igraph_t *)R_alloc(GET_LENGTH(pgraphs),
			     sizeof(igraph_t));
  for (i=0; i<GET_LENGTH(pgraphs); i++) {
    R_SEXP_to_igraph(VECTOR_ELT(pgraphs, i), &graphs[i]);
    VECTOR(ptrvec)[i]=&graphs[i];
  }
  igraph_union_many(&res, &ptrvec);
  igraph_vector_ptr_destroy(&ptrvec);
  PROTECT(result=R_igraph_to_SEXP(&res));
  igraph_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_intersection(SEXP pgraphs) {
  
  igraph_vector_ptr_t ptrvec;
  igraph_t *graphs;
  igraph_t res;
  long int i;
  SEXP result;
  
  R_igraph_before();
  
  igraph_vector_ptr_init(&ptrvec, GET_LENGTH(pgraphs));
  graphs=(igraph_t *)R_alloc(GET_LENGTH(pgraphs),
			     sizeof(igraph_t));
  for (i=0; i<GET_LENGTH(pgraphs); i++) {
    R_SEXP_to_igraph(VECTOR_ELT(pgraphs, i), &graphs[i]);
    VECTOR(ptrvec)[i]=&graphs[i];
  }
  igraph_intersection_many(&res, &ptrvec);
  igraph_vector_ptr_destroy(&ptrvec);
  PROTECT(result=R_igraph_to_SEXP(&res));
  igraph_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_difference(SEXP pleft, SEXP pright) {
  
  igraph_t left, right;
  igraph_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(pleft, &left);
  R_SEXP_to_igraph(pright, &right);
  igraph_difference(&res, &left, &right);
  PROTECT(result=R_igraph_to_SEXP(&res));
  igraph_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_complementer(SEXP pgraph, SEXP ploops) {
  
  igraph_t g;
  igraph_t res;
  igraph_bool_t loops=LOGICAL(ploops)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(pgraph, &g);
  igraph_complementer(&res, &g, loops);
  PROTECT(result=R_igraph_to_SEXP(&res));
  igraph_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_compose(SEXP pleft, SEXP pright) {
  
  igraph_t left, right;
  igraph_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(pleft, &left);
  R_SEXP_to_igraph(pright, &right);
  igraph_compose(&res, &left, &right);
  PROTECT(result=R_igraph_to_SEXP(&res));
  igraph_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_barabasi_aging_game(SEXP pn, SEXP ppa_exp, SEXP paging_exp,
				  SEXP paging_bin, SEXP pm, SEXP pout_seq,
				  SEXP pout_pref, SEXP pzero_deg_appeal,
				  SEXP pzero_age_appeal, SEXP pdeg_coef,
				  SEXP page_coef,
				  SEXP pdirected) {
  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_real_t pa_exp=REAL(ppa_exp)[0];
  igraph_real_t aging_exp=REAL(paging_exp)[0];
  igraph_integer_t aging_bin=REAL(paging_bin)[0];
  igraph_integer_t m=REAL(pm)[0];
  igraph_vector_t out_seq;
  igraph_bool_t out_pref=LOGICAL(pout_pref)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_real_t zero_deg_appeal=REAL(pzero_deg_appeal)[0];
  igraph_real_t zero_age_appeal=REAL(pzero_age_appeal)[0];
  igraph_real_t deg_coef=REAL(pdeg_coef)[0];
  igraph_real_t age_coef=REAL(page_coef)[0];  
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(pout_seq, &out_seq);  
  
  igraph_barabasi_aging_game(&g, n, m, &out_seq, out_pref, pa_exp, aging_exp,
			     aging_bin, zero_deg_appeal, zero_age_appeal,
			     deg_coef, age_coef, directed);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_recent_degree_aging_game(SEXP pn, SEXP ppa_exp, SEXP paging_exp,
				       SEXP paging_bin, SEXP pm, SEXP pout_seq,
				       SEXP pout_pref, SEXP pzero_appeal, 
				       SEXP pdirected,
				       SEXP ptime_window) {
  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_real_t pa_exp=REAL(ppa_exp)[0];
  igraph_real_t aging_exp=REAL(paging_exp)[0];
  igraph_integer_t aging_bin=REAL(paging_bin)[0];
  igraph_integer_t m=REAL(pm)[0];
  igraph_vector_t out_seq;
  igraph_bool_t out_pref=LOGICAL(pout_pref)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_integer_t time_window=REAL(ptime_window)[0];
  igraph_real_t zero_appeal=REAL(pzero_appeal)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(pout_seq, &out_seq);  
  
  igraph_recent_degree_aging_game(&g, n, m, &out_seq, out_pref, pa_exp, 
				  aging_exp, aging_bin, time_window, 
				  zero_appeal, directed);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_get_edge(SEXP graph, SEXP peid) {
  
  igraph_t g;
  igraph_integer_t eid=REAL(peid)[0];
  igraph_integer_t from, to;
  SEXP result;
  
  R_igraph_before();

  R_SEXP_to_igraph(graph, &g);  
  igraph_edge(&g, eid, &from, &to);
  PROTECT(result=NEW_NUMERIC(2));
  REAL(result)[0]=from;
  REAL(result)[1]=to;
  
  R_igraph_after();

  UNPROTECT(1);
  return result;
}

SEXP R_igraph_edges(SEXP graph, SEXP eids) {
  igraph_t g;
  igraph_es_t es;
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_es(eids, &g, &es);
  igraph_vector_init(&res, 0);
  igraph_edges(&g, es, &res);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  igraph_es_destroy(&es);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}  

SEXP R_igraph_constraint(SEXP graph, SEXP vids, SEXP pweights) {
  
  igraph_t g;
  igraph_vs_t vs;
  igraph_vector_t weights, *wptr=0;
  igraph_vector_t res;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(vids, &g, &vs);
  if (GET_LENGTH(pweights) != 0) {
    R_SEXP_to_vector(pweights, &weights);
    wptr=&weights;
  } 
  igraph_vector_init(&res, 0);
  igraph_constraint(&g, &res, vs, wptr);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  igraph_vs_destroy(&vs);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_es_path(SEXP graph, SEXP pp, SEXP pdir) {
  
  igraph_t g;
  igraph_vector_t p;
  igraph_bool_t dir=LOGICAL(pdir)[0];
  igraph_es_t es;
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pp, &p);
  igraph_es_path(&es, &p, dir);
  igraph_vector_init(&res, 0);
  igraph_es_as_vector(&g, es, &res);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  igraph_es_destroy(&es);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_es_pairs(SEXP graph, SEXP pp, SEXP pdir) {

  igraph_t g;
  igraph_vector_t p;
  igraph_bool_t dir=LOGICAL(pdir)[0];
  igraph_es_t es;
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pp, &p);
  igraph_es_pairs(&es, &p, dir);
  igraph_vector_init(&res, 0);
  igraph_es_as_vector(&g, es, &res);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  igraph_es_destroy(&es);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_reciprocity(SEXP graph, SEXP pignore_loops) {
  
  igraph_t g;
  igraph_bool_t ignore_loops=LOGICAL(pignore_loops)[0];
  igraph_real_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_reciprocity(&g, &res, ignore_loops);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_layout_reingold_tilford(SEXP graph, SEXP proot, SEXP pcirc) {
  
  igraph_t g;
  igraph_integer_t root=REAL(proot)[0];
  igraph_matrix_t res;
  igraph_bool_t circ=LOGICAL(pcirc)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&res, 0, 0);
  if (!circ) {
    igraph_layout_reingold_tilford(&g, &res, root);
  } else {
    igraph_layout_reingold_tilford_circular(&g, &res, root);
  }
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_rewire(SEXP graph, SEXP pn, SEXP pmode) {
  
  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_rewiring_t mode=REAL(pmode)[0];
  SEXP result;
  
  R_igraph_before();

  R_SEXP_to_igraph_copy(graph, &g);
  igraph_rewire(&g, n, mode);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
  
SEXP R_igraph_to_directed(SEXP graph, SEXP pmode) {
  
  igraph_t g;
  igraph_integer_t mode=REAL(pmode)[0];
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph_copy(graph, &g);
  igraph_to_directed(&g, mode);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_to_undirected(SEXP graph, SEXP pmode) {
  
  igraph_t g;
  igraph_integer_t mode=REAL(pmode)[0];
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph_copy(graph, &g);
  igraph_to_undirected(&g, mode);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_read_graph_graphml(SEXP pvfile, SEXP pindex) {
  igraph_t g;
  int index=REAL(pindex)[0];
  FILE *file;
  SEXP result;

  R_igraph_before();
  
#if HAVE_FMEMOPEN == 1
  file=fmemopen(RAW(pvfile), GET_LENGTH(pvfile), "r");
#else
  file=fopen(CHAR(STRING_ELT(pvfile, 0)), "r");
#endif
  if (file==0) { igraph_error("Cannot open GraphML file", __FILE__, __LINE__,
			      IGRAPH_EFILE); }
  igraph_read_graph_graphml(&g, file, index);
  fclose(file);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_write_graph_graphml(SEXP graph, SEXP file) {
  
  igraph_t g;
  FILE *stream;
  char *bp;
  size_t size;
  SEXP result;
  
  R_igraph_before();

  R_SEXP_to_igraph(graph, &g);
#if HAVE_OPEN_MEMSTREAM == 1
  stream=open_memstream(&bp, &size);
#else
  stream=fopen(CHAR(STRING_ELT(file, 0)), "w");
#endif
  if (stream==0) { igraph_error("Cannot write GraphML file", __FILE__, 
				__LINE__, IGRAPH_EFILE); }
  igraph_write_graph_graphml(&g, stream);
  fclose(stream);
#if HAVE_OPEN_MEMSTREAM == 1
  PROTECT(result=allocVector(RAWSXP, size));
  memcpy(RAW(result), bp, sizeof(char)*size);
  free(bp);
#else
  PROTECT(result=NEW_NUMERIC(0));
#endif

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_vs_nei(SEXP graph, SEXP px, SEXP pv, SEXP pmode) {
  
  igraph_t g;
  igraph_vs_t v;
  igraph_integer_t mode=REAL(pmode)[0];
  SEXP result;

  igraph_vit_t vv;
  igraph_vector_t neis;
  long int i;
  
  R_igraph_before();

  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pv, &g, &v);

  igraph_vector_init(&neis, 0);  
  igraph_vit_create(&g, v, &vv);
  PROTECT(result=NEW_LOGICAL(igraph_vcount(&g)));
  memset(LOGICAL(result), 0, sizeof(LOGICAL(result)[0])*igraph_vcount(&g));
  
  while (!IGRAPH_VIT_END(vv)) {
    igraph_neighbors(&g, &neis, IGRAPH_VIT_GET(vv), mode);
    for (i=0; i<igraph_vector_size(&neis); i++) {
      long int nei=VECTOR(neis)[i];
      LOGICAL(result)[nei]=1;
    }
    IGRAPH_VIT_NEXT(vv);
  }

  igraph_vit_destroy(&vv);
  igraph_vector_destroy(&neis);
  igraph_vs_destroy(&v);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_vs_adj(SEXP graph, SEXP px, SEXP pe, SEXP pmode) {
  
  igraph_t g;
  igraph_es_t e;
  int mode=REAL(pmode)[0];
  SEXP result;

  igraph_integer_t from, to;
  igraph_eit_t ee;

  R_igraph_before();

  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_es(pe, &g, &e);
  
  igraph_eit_create(&g, e, &ee);
  PROTECT(result=NEW_LOGICAL(igraph_vcount(&g)));
  memset(LOGICAL(result), 0, sizeof(LOGICAL(result)[0])*igraph_vcount(&g));

  while (!IGRAPH_EIT_END(ee)) {
    igraph_edge(&g, IGRAPH_EIT_GET(ee), &from, &to);
    if (mode & 1) { 
      LOGICAL(result)[ (long int)from]=1;
    }
    if (mode & 2) {
      LOGICAL(result)[ (long int)to]=1;
    }
    IGRAPH_EIT_NEXT(ee);
  }
  
  igraph_eit_destroy(&ee);
  igraph_es_destroy(&e);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_es_adj(SEXP graph, SEXP x, SEXP pv, SEXP pmode) {
  
  igraph_t g;
  igraph_vs_t v;
  igraph_integer_t mode=REAL(pmode)[0];
  SEXP result;
  
  igraph_vector_t adje;
  igraph_vit_t vv;
  long int i;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pv, &g, &v);
  
  igraph_vit_create(&g, v, &vv);
  igraph_vector_init(&adje, 0);
  PROTECT(result=NEW_LOGICAL(igraph_ecount(&g)));
  memset(LOGICAL(result), 0, sizeof(LOGICAL(result)[0])*igraph_ecount(&g));
  
  while (!IGRAPH_VIT_END(vv)) {
    igraph_adjacent(&g, &adje, IGRAPH_VIT_GET(vv), mode);
    for (i=0; i<igraph_vector_size(&adje); i++) {
      long int edge=VECTOR(adje)[i];
      LOGICAL(result)[edge]=1;
    }
    IGRAPH_VIT_NEXT(vv);
  }

  igraph_vector_destroy(&adje);
  igraph_vit_destroy(&vv);
  igraph_vs_destroy(&v);

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_grg_game(SEXP pn, SEXP pradius, SEXP ptorus, 
		       SEXP pcoords) {
  
  igraph_t g;
  igraph_integer_t n=REAL(pn)[0];
  igraph_real_t radius=REAL(pradius)[0];
  igraph_bool_t torus=LOGICAL(ptorus)[0];
  igraph_bool_t coords=LOGICAL(pcoords)[0];
  igraph_vector_t x, y, *px=0, *py=0;
  SEXP result, names;

  R_igraph_before();
  
  if (coords) {
    igraph_vector_init(&x, 0);  px=&x;
    igraph_vector_init(&y, 0);  py=&y;
  }
  igraph_grg_game(&g, n, radius, torus, px, py);
  PROTECT(result=NEW_LIST(3));
  SET_VECTOR_ELT(result, 0, R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  SET_VECTOR_ELT(result, 1, R_igraph_0orvector_to_SEXP(px));
  if (coords) { igraph_vector_destroy(px); }
  SET_VECTOR_ELT(result, 2, R_igraph_0orvector_to_SEXP(py));
  if (coords) { igraph_vector_destroy(py); }
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_density(SEXP graph, SEXP ploops) {
  
  igraph_t g;
  igraph_bool_t loops=LOGICAL(ploops)[0];
  igraph_real_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_density(&g, &res, loops);
  
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_maxflow(SEXP graph, SEXP psource, SEXP ptarget, 
		      SEXP pcapacity) {

  igraph_t g;
  igraph_integer_t source=REAL(psource)[0], target=REAL(ptarget)[0];
  igraph_vector_t capacity, *ppcapacity=0;
  igraph_real_t value;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  if (!isNull(pcapacity)) {
    R_SEXP_to_vector(pcapacity, &capacity);
    ppcapacity=&capacity;
  }
  igraph_maxflow_value(&g, &value, source, target, ppcapacity);
  
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=value;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_read_graph_dimacs(SEXP pvfile, SEXP pdirected) {
  
  igraph_t g;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  FILE *file;
  igraph_vector_t label;
  igraph_strvector_t problem;
  igraph_integer_t source, target;
  igraph_vector_t cap;
  SEXP result;
  
  R_igraph_before();
  
#if HAVE_FMEMOPEN == 1
  file=fmemopen(RAW(pvfile), GET_LENGTH(pvfile), "r");
#else
  file=fopen(CHAR(STRING_ELT(pvfile, 0)), "r");
#endif
  if (file==0) { igraph_error("Cannot read edgelist", __FILE__, __LINE__,
			      IGRAPH_EFILE); 
  }
  igraph_vector_init(&label, 0);
  igraph_strvector_init(&problem, 0);
  igraph_vector_init(&cap, 0);
  igraph_read_graph_dimacs(&g, file, &problem, &label, 
			   &source, &target, &cap, directed);
  fclose(file);
  if (!strcmp(STR(problem, 0), "max")) {
    PROTECT(result=NEW_LIST(5));
    SET_VECTOR_ELT(result, 0, R_igraph_strvector_to_SEXP(&problem));
    igraph_strvector_destroy(&problem);
    SET_VECTOR_ELT(result, 1, R_igraph_to_SEXP(&g));
    igraph_destroy(&g);
    SET_VECTOR_ELT(result, 2, NEW_NUMERIC(1));
    REAL(VECTOR_ELT(result, 2))[0]=source;
    SET_VECTOR_ELT(result, 3, NEW_NUMERIC(1));
    REAL(VECTOR_ELT(result, 3))[0]=target;
    SET_VECTOR_ELT(result, 4, NEW_NUMERIC(igraph_vector_size(&cap)));
    igraph_vector_copy_to(&cap, REAL(VECTOR_ELT(result,4)));
    igraph_vector_destroy(&cap);
  } else if (!strcmp(STR(problem, 0), "edge")) {
    PROTECT(result=NEW_LIST(3));
    SET_VECTOR_ELT(result, 0, R_igraph_strvector_to_SEXP(&problem));    
    igraph_strvector_destroy(&problem);
    SET_VECTOR_ELT(result, 1, R_igraph_to_SEXP(&g));
    igraph_destroy(&g);
    SET_VECTOR_ELT(result, 2, R_igraph_vector_to_SEXP(&label));
    igraph_vector_destroy(&label);
  } else {
    /* This shouldn't happen */
    igraph_error("Invalid DIMACS file (problem) type", __FILE__, __LINE__,
		 IGRAPH_PARSEERROR);
  }

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_write_graph_dimacs(SEXP graph, SEXP file,
				 SEXP psource, SEXP ptarget,
				 SEXP pcap) {
  
  igraph_t g;
  FILE *stream;
  char *bp;
  size_t size;
  igraph_integer_t source=REAL(psource)[0];
  igraph_integer_t target=REAL(ptarget)[0];
  igraph_vector_t cap;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcap, &cap);
#if HAVE_OPEN_MEMSTREAM == 1
  stream=open_memstream(&bp, &size);
#else
  stream=fopen(CHAR(STRING_ELT(file, 0)), "w");
#endif
  if (stream==0) { igraph_error("Cannot write edgelist", __FILE__, __LINE__,
				IGRAPH_EFILE); 
  }
  igraph_write_graph_dimacs(&g, stream, source, target, &cap);
  fclose(stream);
#if HAVE_OPEN_MEMSTREAM == 1
  PROTECT(result=allocVector(RAWSXP, size));
  memcpy(RAW(result), bp, sizeof(char)*size);
  free(bp);
#else
  PROTECT(result=NEW_NUMERIC(0));
#endif
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_mincut(SEXP graph, SEXP pcapacity) {

  igraph_t g;
  igraph_vector_t capacity, *ppcapacity=0;
  igraph_real_t value;
  igraph_vector_t partition;
  igraph_vector_t partition2;
  igraph_vector_t cut;
  SEXP result, names;
  
  R_igraph_before();
  
  igraph_vector_init(&cut, 0);
  igraph_vector_init(&partition, 0);
  igraph_vector_init(&partition2, 0);

  R_SEXP_to_igraph(graph, &g);
  if (!isNull(pcapacity)) {
    R_SEXP_to_vector(pcapacity, &capacity);
    ppcapacity=&capacity;
  }

  igraph_mincut(&g, &value, &partition,
		&partition2, &cut, ppcapacity);

  PROTECT(result=NEW_LIST(4));
  PROTECT(names=NEW_CHARACTER(4));
  SET_VECTOR_ELT(result, 0, NEW_NUMERIC(1));
  REAL(VECTOR_ELT(result, 0))[0]=value;
  SET_VECTOR_ELT(result, 1, R_igraph_vector_to_SEXP(&cut));
  igraph_vector_destroy(&cut);
  SET_VECTOR_ELT(result, 2, R_igraph_vector_to_SEXP(&partition));
  igraph_vector_destroy(&partition);
  SET_VECTOR_ELT(result, 3, R_igraph_vector_to_SEXP(&partition2));
  igraph_vector_destroy(&partition2);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("value"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("cut"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("partition1"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("partition2"));
  SET_NAMES(result, names);

  R_igraph_after();

  UNPROTECT(2);
  return result;
}  
  
SEXP R_igraph_mincut_value(SEXP graph, SEXP pcapacity) {
  
  igraph_t g;
  igraph_vector_t capacity, *ppcapacity=0;
  igraph_integer_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  if (!isNull(pcapacity)) {
    R_SEXP_to_vector(pcapacity, &capacity);
    ppcapacity=&capacity;
  }
  igraph_mincut_value(&g, &res, ppcapacity);
  
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_st_vertex_connectivity(SEXP graph, SEXP psource, 
				     SEXP ptarget) {
  
  igraph_t g;
  igraph_integer_t source=REAL(psource)[0], target=REAL(ptarget)[0];
  igraph_integer_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_st_vertex_connectivity(&g, &res, source, target, 
				IGRAPH_VCONN_NEI_ERROR);
  
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_vertex_connectivity(SEXP graph, SEXP pchecks) {
  
  igraph_t g;
  igraph_integer_t res;
  igraph_bool_t checks=LOGICAL(pchecks)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vertex_connectivity(&g, &res, checks);
  
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;

  UNPROTECT(1);
  return result;
}

SEXP R_igraph_st_edge_connectivity(SEXP graph, SEXP psource, SEXP ptarget) {
  
  igraph_t g;
  igraph_integer_t source=REAL(psource)[0], target=REAL(ptarget)[0];
  igraph_real_t value;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_st_edge_connectivity(&g, &value, source, target);
  
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=value;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_edge_connectivity(SEXP graph, SEXP pchecks) {
  
  igraph_t g;
  igraph_integer_t res;
  igraph_bool_t checks=LOGICAL(pchecks)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_edge_connectivity(&g, &res, checks);
  
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_st_mincut_value(SEXP graph, SEXP psource, SEXP ptarget,
			      SEXP pcapacity) {
  igraph_t g;
  igraph_integer_t source=REAL(psource)[0];
  igraph_integer_t target=REAL(ptarget)[0];
  igraph_vector_t capacity, *ppcapacity=0;
  igraph_real_t res;
  SEXP result;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  if (!isNull(pcapacity)) {
    R_SEXP_to_vector(pcapacity, &capacity);
    ppcapacity=&capacity;
  }
  igraph_st_mincut_value(&g, &res, source, target, ppcapacity);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_edge_disjoint_paths(SEXP graph, SEXP psource, SEXP ptarget) {
  
  igraph_t g;
  igraph_integer_t source=REAL(psource)[0];
  igraph_integer_t target=REAL(ptarget)[0];
  igraph_integer_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_edge_disjoint_paths(&g, &res, source, target);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_vertex_disjoint_paths(SEXP graph, SEXP psource, SEXP ptarget) {
  
  igraph_t g;
  igraph_integer_t source=REAL(psource)[0];
  igraph_integer_t target=REAL(ptarget)[0];
  igraph_integer_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vertex_disjoint_paths(&g, &res, source, target);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;

  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_adhesion(SEXP graph, SEXP pchecks) {
  
  igraph_t g;
  igraph_integer_t res;
  igraph_bool_t checks=LOGICAL(pchecks)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_adhesion(&g, &res, checks);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_cohesion(SEXP graph, SEXP pchecks) {
  
  igraph_t g;
  igraph_integer_t res;
  igraph_bool_t checks=LOGICAL(pchecks)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_cohesion(&g, &res, checks);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_spinglass_community(SEXP graph, SEXP pweights,
				  SEXP pspins, SEXP pparupdate,
				  SEXP pstarttemp, SEXP pstoptemp,
				  SEXP pcoolfact, SEXP pupdate_rule,
				  SEXP pgamma) {
  igraph_t g;
  igraph_vector_t weights;
  igraph_integer_t spins=REAL(pspins)[0];
  igraph_bool_t parupdate=LOGICAL(pparupdate)[0];
  igraph_real_t starttemp=REAL(pstarttemp)[0];
  igraph_real_t stoptemp=REAL(pstoptemp)[0];
  igraph_real_t coolfact=REAL(pcoolfact)[0];
  igraph_spincomm_update_t update_rule=REAL(pupdate_rule)[0];
  igraph_real_t gamma=REAL(pgamma)[0];
  igraph_real_t modularity;
  igraph_real_t temperature;
  igraph_vector_t membership;
  igraph_vector_t csize;
  SEXP result, names;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pweights, &weights);
  igraph_vector_init(&membership, 0);
  igraph_vector_init(&csize, 0);
  igraph_community_spinglass(&g, &weights, 
			     &modularity, &temperature, 
			     &membership, &csize,
			     spins, parupdate, starttemp, stoptemp,
			     coolfact, update_rule, gamma);
  
  PROTECT(result=NEW_LIST(4));
  PROTECT(names=NEW_CHARACTER(4));
  SET_VECTOR_ELT(result, 0, NEW_NUMERIC(igraph_vector_size(&membership)));
  SET_VECTOR_ELT(result, 1, NEW_NUMERIC(igraph_vector_size(&csize)));
  SET_VECTOR_ELT(result, 2, NEW_NUMERIC(1));
  SET_VECTOR_ELT(result, 3, NEW_NUMERIC(1));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("membership"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("csize"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("modularity"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("temperature"));
  SET_NAMES(result, names);
  igraph_vector_copy_to(&membership, REAL(VECTOR_ELT(result, 0)));
  igraph_vector_copy_to(&csize, REAL(VECTOR_ELT(result, 1)));
  REAL(VECTOR_ELT(result, 2))[0]=modularity;
  REAL(VECTOR_ELT(result, 3))[0]=temperature;
  
  igraph_vector_destroy(&membership);
  igraph_vector_destroy(&csize);
  
  R_igraph_after();
  
  UNPROTECT(2);
  return result;
}

SEXP R_igraph_spinglass_my_community(SEXP graph, SEXP weights,
				     SEXP pvertex, SEXP pspins,
				     SEXP pupdate_rule, SEXP pgamma) {
  igraph_t g;
  igraph_vector_t v_weights, *pweights=0;
  igraph_integer_t vertex=REAL(pvertex)[0];
  igraph_integer_t spins=REAL(pspins)[0];
  igraph_spincomm_update_t update_rule=REAL(pupdate_rule)[0];
  igraph_real_t gamma=REAL(pgamma)[0];
  igraph_vector_t community;
  igraph_real_t cohesion;
  igraph_real_t adhesion;
  igraph_integer_t inner_links;
  igraph_integer_t outer_links;
  SEXP result, names;

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  if (!isNull(weights)) { 
    pweights=&v_weights; R_SEXP_to_vector(weights, &v_weights);
  }
  igraph_vector_init(&community, 0);
  igraph_community_spinglass_single(&g, pweights, vertex, &community,
				    &cohesion, &adhesion, &inner_links,
				    &outer_links, spins, update_rule, gamma);
  
  PROTECT(result=NEW_LIST(5));
  PROTECT(names=NEW_CHARACTER(5));
  SET_VECTOR_ELT(result, 0, NEW_NUMERIC(igraph_vector_size(&community)));
  SET_VECTOR_ELT(result, 1, NEW_NUMERIC(1));
  SET_VECTOR_ELT(result, 2, NEW_NUMERIC(1));
  SET_VECTOR_ELT(result, 3, NEW_NUMERIC(1));
  SET_VECTOR_ELT(result, 4, NEW_NUMERIC(1));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("community"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("cohesion"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("adhesion"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("inner.links"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("outer.links"));
  SET_NAMES(result, names);
  igraph_vector_copy_to(&community, REAL(VECTOR_ELT(result, 0)));
  REAL(VECTOR_ELT(result, 1))[0] = cohesion;
  REAL(VECTOR_ELT(result, 2))[0] = adhesion;
  REAL(VECTOR_ELT(result, 3))[0] = inner_links;
  REAL(VECTOR_ELT(result, 4))[0] = outer_links;
  
  igraph_vector_destroy(&community);
  
  R_igraph_after();
  
  UNPROTECT(2);
  return result;
}

SEXP R_igraph_extended_chordal_ring(SEXP pnodes, SEXP pw) {

  igraph_t g;
  igraph_integer_t nodes=REAL(pnodes)[0];
  igraph_matrix_t w;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_matrix(pw, &w);
  igraph_extended_chordal_ring(&g, nodes, &w);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_no_clusters(SEXP graph, SEXP pmode) {
  
  igraph_t g;
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_integer_t res;
  SEXP result;

  R_igraph_before();

  R_SEXP_to_igraph(graph, &g);
  igraph_clusters(&g, 0, 0, &res, mode);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_neighborhood_size(SEXP graph, SEXP pvids, SEXP porder, 
				SEXP pmode) {
  igraph_t g;
  igraph_vs_t vids;
  igraph_integer_t order=REAL(porder)[0];
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pvids, &g, &vids);
  igraph_vector_init(&res, 0);
  igraph_neighborhood_size(&g, &res, vids, order, mode);
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  igraph_vs_destroy(&vids);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_neighborhood(SEXP graph, SEXP pvids, SEXP porder, 
			   SEXP pmode) {
  igraph_t g;
  igraph_vs_t vids;
  igraph_integer_t order=REAL(porder)[0];
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_vector_ptr_t res;
  long int i;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pvids, &g, &vids);
  igraph_vector_ptr_init(&res, 0);
  igraph_neighborhood(&g, &res, vids, order, mode);
  PROTECT(result=NEW_LIST(igraph_vector_ptr_size(&res)));
  for (i=0; i<igraph_vector_ptr_size(&res); i++) {
    igraph_vector_t *v=VECTOR(res)[i];
    SET_VECTOR_ELT(result, i, NEW_NUMERIC(igraph_vector_size(v)));
    igraph_vector_copy_to(v, REAL(VECTOR_ELT(result, i)));
    igraph_vector_destroy(v);
    igraph_free(v);
  }
  igraph_vector_ptr_destroy(&res);
  igraph_vs_destroy(&vids);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_neighborhood_graphs(SEXP graph, SEXP pvids, SEXP porder, 
				  SEXP pmode) {
  igraph_t g;
  igraph_vs_t vids;
  igraph_integer_t order=REAL(porder)[0];
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_vector_ptr_t res;
  long int i;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_vs(pvids, &g, &vids);
  igraph_vector_ptr_init(&res, 0);
  igraph_neighborhood_graphs(&g, &res, vids, order, mode);
  PROTECT(result=NEW_LIST(igraph_vector_ptr_size(&res)));
  for (i=0; i<igraph_vector_ptr_size(&res); i++) {
    igraph_t *g=VECTOR(res)[i];
    SET_VECTOR_ELT(result, i, R_igraph_to_SEXP(g));
    igraph_destroy(g);
    igraph_free(g);
  }
  igraph_vector_ptr_destroy(&res);
  igraph_vs_destroy(&vids);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_preference_game(SEXP pnodes, SEXP ptypes,
			      SEXP ptype_dist, SEXP pmatrix,
			      SEXP pdirected, SEXP ploops) {
  igraph_t g;
  igraph_integer_t nodes=REAL(pnodes)[0];
  igraph_integer_t types=REAL(ptypes)[0];
  igraph_vector_t type_dist;
  igraph_matrix_t matrix;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  igraph_bool_t loops=LOGICAL(ploops)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(ptype_dist, &type_dist);
  R_SEXP_to_matrix(pmatrix, &matrix);
  igraph_preference_game(&g, nodes, types, &type_dist, &matrix, 0,
			 directed, loops);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_asymmetric_preference_game(SEXP pnodes, SEXP ptypes,
					 SEXP ptype_dist_matrix, SEXP pmatrix,
					 SEXP ploops) {
  
  igraph_t g;
  igraph_integer_t nodes=REAL(pnodes)[0];
  igraph_integer_t types=REAL(ptypes)[0];
  igraph_matrix_t type_dist_matrix;
  igraph_matrix_t matrix;
  igraph_bool_t loops=LOGICAL(ploops)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_matrix(ptype_dist_matrix, &type_dist_matrix);
  R_SEXP_to_matrix(pmatrix, &matrix);
  igraph_asymmetric_preference_game(&g, nodes, types, &type_dist_matrix,
				    &matrix, 0, 0, loops);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_laplacian(SEXP graph, SEXP pnormalized) {
  
  igraph_t g;
  igraph_matrix_t res;
  igraph_bool_t normalized=LOGICAL(pnormalized)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&res, 0, 0);
  igraph_laplacian(&g, &res, normalized);
  PROTECT(result=R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_connect_neighborhood(SEXP graph, SEXP porder, SEXP pmode) {
  
  igraph_t g;
  igraph_integer_t order=REAL(porder)[0];
  igraph_integer_t mode=REAL(pmode)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph_copy(graph, &g);
  igraph_connect_neighborhood(&g, order, mode);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_rewire_edges(SEXP graph, SEXP pprob) {
  
  igraph_t g;
  igraph_real_t prob=REAL(pprob)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph_copy(graph, &g);
  igraph_rewire_edges(&g, prob);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_watts_strogatz_game(SEXP pdim, SEXP psize, SEXP pnei, SEXP pp) {
  
  igraph_t g;
  igraph_integer_t dim=REAL(pdim)[0];
  igraph_integer_t size=REAL(psize)[0];
  igraph_integer_t nei=REAL(pnei)[0];
  igraph_real_t p=REAL(pp)[0];
  SEXP result;
  
  R_igraph_before();
  
  igraph_watts_strogatz_game(&g, dim, size, nei, p);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_coreness(SEXP graph, SEXP pmode) {
  
  igraph_t g;
  igraph_integer_t mode=REAL(pmode)[0];
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&res, 0);
  igraph_coreness(&g, &res, mode);
  
  PROTECT(result=NEW_NUMERIC(igraph_vector_size(&res)));
  igraph_vector_copy_to(&res, REAL(result));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_cliques(SEXP graph, SEXP pminsize, SEXP pmaxsize) {
  
  igraph_t g;
  igraph_vector_ptr_t ptrvec;
  igraph_integer_t minsize=REAL(pminsize)[0];
  igraph_integer_t maxsize=REAL(pmaxsize)[0];
  long int i;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_ptr_init(&ptrvec, 0);
  igraph_cliques(&g, &ptrvec, minsize, maxsize);
  PROTECT(result=NEW_LIST(igraph_vector_ptr_size(&ptrvec)));
  for (i=0; i<igraph_vector_ptr_size(&ptrvec); i++) {
    igraph_vector_t *vec=VECTOR(ptrvec)[i];
    SET_VECTOR_ELT(result, i, NEW_NUMERIC(igraph_vector_size(vec)));
    igraph_vector_copy_to(vec, REAL(VECTOR_ELT(result, i)));
    igraph_vector_destroy(vec);
    igraph_free(vec);
  }
  igraph_vector_ptr_destroy(&ptrvec);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_largest_cliques(SEXP graph) {
  
  igraph_t g;
  igraph_vector_ptr_t ptrvec;
  long int i;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_ptr_init(&ptrvec,0);
  igraph_largest_cliques(&g, &ptrvec);
  PROTECT(result=NEW_LIST(igraph_vector_ptr_size(&ptrvec)));
  for (i=0; i<igraph_vector_ptr_size(&ptrvec); i++) {
    igraph_vector_t *vec=VECTOR(ptrvec)[i];
    SET_VECTOR_ELT(result, i, NEW_NUMERIC(igraph_vector_size(vec)));
    igraph_vector_copy_to(vec, REAL(VECTOR_ELT(result, i)));
    igraph_vector_destroy(vec);
    igraph_free(vec);
  }
  igraph_vector_ptr_destroy(&ptrvec);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_maximal_cliques(SEXP graph) {
  
  igraph_t g;
  igraph_vector_ptr_t ptrvec;
  long int i;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_ptr_init(&ptrvec,0);
  igraph_maximal_cliques(&g, &ptrvec);
  PROTECT(result=NEW_LIST(igraph_vector_ptr_size(&ptrvec)));
  for (i=0; i<igraph_vector_ptr_size(&ptrvec); i++) {
    igraph_vector_t *vec=VECTOR(ptrvec)[i];
    SET_VECTOR_ELT(result, i, NEW_NUMERIC(igraph_vector_size(vec)));
    igraph_vector_copy_to(vec, REAL(VECTOR_ELT(result, i)));
    igraph_vector_destroy(vec);
    igraph_free(vec);
  }
  igraph_vector_ptr_destroy(&ptrvec);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_clique_number(SEXP graph) {
  
  igraph_t g;
  igraph_integer_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_clique_number(&g, &res);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_independent_vertex_sets(SEXP graph, 
				      SEXP pminsize, SEXP pmaxsize) {
  igraph_t g;
  igraph_vector_ptr_t ptrvec;
  igraph_integer_t minsize=REAL(pminsize)[0];
  igraph_integer_t maxsize=REAL(pmaxsize)[0];
  long int i;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_ptr_init(&ptrvec, 0);
  igraph_independent_vertex_sets(&g, &ptrvec, minsize, maxsize);
  PROTECT(result=NEW_LIST(igraph_vector_ptr_size(&ptrvec)));
  for (i=0; i<igraph_vector_ptr_size(&ptrvec); i++) {
    igraph_vector_t *vec=VECTOR(ptrvec)[i];
    SET_VECTOR_ELT(result, i, NEW_NUMERIC(igraph_vector_size(vec)));
    igraph_vector_copy_to(vec, REAL(VECTOR_ELT(result, i)));
    igraph_vector_destroy(vec);
    igraph_free(vec);
  }
  igraph_vector_ptr_destroy(&ptrvec);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_largest_independent_vertex_sets(SEXP graph) {
  igraph_t g;
  igraph_vector_ptr_t ptrvec;
  long int i;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_ptr_init(&ptrvec,0);
  igraph_largest_independent_vertex_sets(&g, &ptrvec);
  PROTECT(result=NEW_LIST(igraph_vector_ptr_size(&ptrvec)));
  for (i=0; i<igraph_vector_ptr_size(&ptrvec); i++) {
    igraph_vector_t *vec=VECTOR(ptrvec)[i];
    SET_VECTOR_ELT(result, i, NEW_NUMERIC(igraph_vector_size(vec)));
    igraph_vector_copy_to(vec, REAL(VECTOR_ELT(result, i)));
    igraph_vector_destroy(vec);
    igraph_free(vec);
  }
  igraph_vector_ptr_destroy(&ptrvec);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_maximal_independent_vertex_sets(SEXP graph) {
  igraph_t g;
  igraph_vector_ptr_t ptrvec;
  long int i;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_ptr_init(&ptrvec,0);
  igraph_maximal_independent_vertex_sets(&g, &ptrvec);
  PROTECT(result=NEW_LIST(igraph_vector_ptr_size(&ptrvec)));
  for (i=0; i<igraph_vector_ptr_size(&ptrvec); i++) {
    igraph_vector_t *vec=VECTOR(ptrvec)[i];
    SET_VECTOR_ELT(result, i, NEW_NUMERIC(igraph_vector_size(vec)));
    igraph_vector_copy_to(vec, REAL(VECTOR_ELT(result, i)));
    igraph_vector_destroy(vec);
    igraph_free(vec);
  }
  igraph_vector_ptr_destroy(&ptrvec);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_independence_number(SEXP graph) {
  igraph_t g;
  igraph_integer_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_independence_number(&g, &res);
  PROTECT(result=NEW_NUMERIC(1));
  REAL(result)[0]=res;
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_lastcit_game(SEXP pnodes, SEXP pedges, SEXP pagebins,
			   SEXP ppreference, SEXP pdirected) {
  
  igraph_t g;
  igraph_integer_t nodes=REAL(pnodes)[0];
  igraph_integer_t edges=REAL(pedges)[0];
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_vector_t preference;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(ppreference, &preference);
  igraph_lastcit_game(&g, nodes, edges, agebins, &preference, directed);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_cited_type_game(SEXP pnodes, SEXP pedges, SEXP ptypes,
			      SEXP ppref, SEXP pdirected) {
  igraph_t g;
  igraph_integer_t nodes=REAL(pnodes)[0];
  igraph_integer_t edges=REAL(pedges)[0];
  igraph_vector_t types, pref;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(ptypes, &types);
  R_SEXP_to_vector(ppref, &pref);
  igraph_cited_type_game(&g, nodes, &types, &pref, edges, directed);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
			      

SEXP R_igraph_citing_cited_type_game(SEXP pnodes, SEXP ptypes, SEXP ppref,
				     SEXP pedges, SEXP pdirected) {
  igraph_t g;
  igraph_integer_t nodes=REAL(pnodes)[0];
  igraph_integer_t edges=REAL(pedges)[0];
  igraph_vector_t types;
  igraph_matrix_t pref;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(ptypes, &types);
  R_SEXP_to_matrix(ppref, &pref);
  igraph_citing_cited_type_game(&g, nodes, &types, &pref, edges, directed);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_walktrap_community(SEXP graph, SEXP pweights,
				 SEXP psteps, SEXP pmerges, 
				 SEXP pmodularity) {
  igraph_t g;
  igraph_vector_t weights, *ppweights=0;
  igraph_integer_t steps=REAL(psteps)[0];
  igraph_matrix_t merges, *ppmerges=0;
  igraph_vector_t modularity, *ppmodularity=0;
  SEXP result, names;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  if (!isNull(pweights)) {
    ppweights=&weights;
    R_SEXP_to_vector(pweights, ppweights);
  }        
  if (LOGICAL(pmerges)[0]) {
    ppmerges=&merges;
    igraph_matrix_init(ppmerges, 0, 0);
  }
  if (LOGICAL(pmodularity)[0]) {
    ppmodularity=&modularity;
    igraph_vector_init(ppmodularity, 0);
  }

  igraph_community_walktrap(&g, ppweights, steps, ppmerges, ppmodularity);

  PROTECT(result=NEW_LIST(2));
  SET_VECTOR_ELT(result, 0, R_igraph_0ormatrix_to_SEXP(ppmerges));
  if (ppmerges) { igraph_matrix_destroy(ppmerges); }
  SET_VECTOR_ELT(result, 1, R_igraph_0orvector_to_SEXP(ppmodularity));
  if (ppmodularity) { igraph_vector_destroy(ppmodularity); }
  PROTECT(names=NEW_CHARACTER(2));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("merges"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("modularity"));
  SET_NAMES(result, names);
  
  R_igraph_after();
  
  UNPROTECT(2);
  return result;
} 

SEXP R_igraph_topological_sorting(SEXP graph, SEXP pneimode) {
  igraph_t g;
  igraph_vector_t res;
  igraph_integer_t mode=REAL(pneimode)[0];
  SEXP result;  

  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&res, 0);
  igraph_topological_sorting(&g, &res, mode);
  
  PROTECT(result=R_igraph_vector_to_SEXP(&res));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_community_edge_betweenness(SEXP graph, SEXP pdirected, 
					 SEXP peb, SEXP pmerges, SEXP pbridges) {
  igraph_t g;
  igraph_vector_t res;
  igraph_vector_t eb, *ppeb=0;
  igraph_matrix_t merges, *ppmerges=0;
  igraph_vector_t bridges, *ppbridges=0;
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  SEXP result, names;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&res, 0);
  if (LOGICAL(peb)[0]) {
    ppeb=&eb;
    igraph_vector_init(&eb, 0);
  }
  if (LOGICAL(pmerges)[0]) {
    ppmerges=&merges;
    igraph_matrix_init(&merges, 0, 0);
  }
  if (LOGICAL(pbridges)[0]) {
    ppbridges=&bridges;
    igraph_vector_init(&bridges, 0);
  }
  igraph_community_edge_betweenness(&g, &res, ppeb, ppmerges, ppbridges, 
				    directed);
  
  PROTECT(result=NEW_LIST(4));
  SET_VECTOR_ELT(result, 0, R_igraph_vector_to_SEXP(&res));
  igraph_vector_destroy(&res);
  SET_VECTOR_ELT(result, 1, R_igraph_0orvector_to_SEXP(ppeb));
  if (ppeb) { igraph_vector_destroy(ppeb); }
  SET_VECTOR_ELT(result, 2, R_igraph_0ormatrix_to_SEXP(ppmerges));
  if (ppmerges) { igraph_matrix_destroy(ppmerges); }
  SET_VECTOR_ELT(result, 3, R_igraph_0orvector_to_SEXP(ppbridges));
  if (ppbridges) { igraph_vector_destroy(ppbridges); }
  PROTECT(names=NEW_CHARACTER(4));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("removed.edges"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("edge.betweenness"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("merges"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("bridges"));
  SET_NAMES(result, names);
  
  R_igraph_after();
  
  UNPROTECT(2);
  return result;
}

SEXP R_igraph_community_eb_get_merges(SEXP graph, 
				      SEXP pedges) {
  
  igraph_t g;
  igraph_vector_t edges;
  igraph_vector_t bridges;
  igraph_matrix_t res;
  SEXP result, names;
  
  R_igraph_before();
 
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pedges, &edges);
  igraph_matrix_init(&res, 0, 0);
  igraph_vector_init(&bridges, 0);
  igraph_community_eb_get_merges(&g, &edges, &res, &bridges);
  PROTECT(result=NEW_LIST(2));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&res));
  igraph_matrix_destroy(&res);    
  SET_VECTOR_ELT(result, 1, R_igraph_vector_to_SEXP(&bridges));
  igraph_vector_destroy(&bridges);
  PROTECT(names=NEW_CHARACTER(2));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("merges"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("bridges"));  
  SET_NAMES(result, names);

  R_igraph_after();
  
  UNPROTECT(2);
  return result;
}

SEXP R_igraph_community_fastgreedy(SEXP graph, SEXP pmerges, SEXP pmodularity, 
				   SEXP pweights) {
  
  igraph_t g;
  igraph_matrix_t merges, *ppmerges=0;
  igraph_vector_t modularity, *ppmodularity=0;
  igraph_vector_t weights, *ppweights=0;
  SEXP result, names;
  
  R_igraph_before();
  
  if (!isNull(pweights)) {
    ppweights=&weights;
    R_SEXP_to_vector(pweights, ppweights);
  }        
  R_SEXP_to_igraph(graph, &g);
  if (LOGICAL(pmerges)[0]) {
    ppmerges=&merges;
    igraph_matrix_init(&merges, 0, 0);
  }
  if (LOGICAL(pmodularity)[0]) {
    ppmodularity=&modularity;
    igraph_vector_init(&modularity, 0);
  }
  igraph_community_fastgreedy(&g, ppweights, ppmerges, ppmodularity);
  PROTECT(result=NEW_LIST(2));
  SET_VECTOR_ELT(result, 0, R_igraph_0ormatrix_to_SEXP(ppmerges));
  if (ppmerges) { igraph_matrix_destroy(ppmerges); }
  SET_VECTOR_ELT(result, 1, R_igraph_0orvector_to_SEXP(ppmodularity));
  if (ppmodularity) { igraph_vector_destroy(ppmodularity); }
  PROTECT(names=NEW_CHARACTER(2));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("merges"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("modularity"));
  SET_NAMES(result, names);
  
  R_igraph_after();
  
  UNPROTECT(2);
  return result;
} 

SEXP R_igraph_community_to_membership(SEXP graph, SEXP pmerges, 
				      SEXP psteps, SEXP pmembership,
				      SEXP pcsize) {
  igraph_t g;
  igraph_integer_t nodes;
  igraph_matrix_t merges;
  igraph_integer_t steps=REAL(psteps)[0];
  igraph_vector_t membership, *ppmembership=0;
  igraph_vector_t csize, *ppcsize=0;
  SEXP result, names;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  nodes=igraph_vcount(&g);
  R_SEXP_to_matrix(pmerges, &merges);
  if (LOGICAL(pmembership)[0]) {
    ppmembership=&membership;
    igraph_vector_init(ppmembership, 0);
  } 
  if (LOGICAL(pcsize)[0]) {
    ppcsize=&csize;
    igraph_vector_init(ppcsize, 0);
  }
  igraph_community_to_membership(&merges, nodes, steps, ppmembership, ppcsize);
  PROTECT(result=NEW_LIST(2));
  SET_VECTOR_ELT(result, 0, R_igraph_0orvector_to_SEXP(ppmembership));
  if (ppmembership) { igraph_vector_destroy(ppmembership); }
  SET_VECTOR_ELT(result, 1, R_igraph_0orvector_to_SEXP(ppcsize)); 
  if (ppcsize) { igraph_vector_destroy(ppcsize); }
  PROTECT(names=NEW_CHARACTER(2));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("membership"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("csize"));
  SET_NAMES(result, names);
  
  R_igraph_after();
  
  UNPROTECT(2);
  return result;
}

SEXP R_igraph_girth(SEXP graph, SEXP pcircle) {
  
  igraph_t g;
  igraph_vector_t circle, *ppcircle=0;
  igraph_integer_t girth;
  SEXP result, names;
  
  R_igraph_before();

  R_SEXP_to_igraph(graph, &g);
  if (LOGICAL(pcircle)[0]) { igraph_vector_init(&circle, 0); ppcircle=&circle; }

  igraph_girth(&g, &girth, ppcircle);

  PROTECT(result=NEW_LIST(2));
  SET_VECTOR_ELT(result, 0, NEW_NUMERIC(1));
  REAL(VECTOR_ELT(result, 0))[0]=girth;
  SET_VECTOR_ELT(result, 1, R_igraph_0orvector_to_SEXP(ppcircle));
  if (ppcircle) { igraph_vector_destroy(ppcircle); }
  PROTECT(names=NEW_CHARACTER(2));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("girth"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("circle"));
  SET_NAMES(result, names);

  R_igraph_after();
  
  UNPROTECT(2);
  return result;
}

SEXP R_igraph_line_graph(SEXP graph) {
  
  igraph_t g, lg;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  igraph_linegraph(&g, &lg);
  PROTECT(result=R_igraph_to_SEXP(&lg));
  igraph_destroy(&lg);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_de_bruijn(SEXP pm, SEXP pn) {

  igraph_t g;
  igraph_integer_t m=REAL(pm)[0];
  igraph_integer_t n=REAL(pn)[0];
  SEXP result;
  
  R_igraph_before();
  
  igraph_de_bruijn(&g, m, n);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_kautz(SEXP pm, SEXP pn) {

  igraph_t g;
  igraph_integer_t m=REAL(pm)[0];
  igraph_integer_t n=REAL(pn)[0];
  SEXP result;
  
  R_igraph_before();
  
  igraph_kautz(&g, m, n);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_is_loop(SEXP graph, SEXP edges) {
  
  igraph_es_t es;
  igraph_t g;
  igraph_vector_bool_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_es(edges, &g, &es);
  igraph_vector_bool_init(&res, 0);
  igraph_is_loop(&g, &res, es);
  PROTECT(result=R_igraph_vector_bool_to_SEXP(&res));
  igraph_vector_bool_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_is_multiple(SEXP graph, SEXP edges) {
  
  igraph_es_t es;
  igraph_t g;
  igraph_vector_bool_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_es(edges, &g, &es);
  igraph_vector_bool_init(&res, 0);
  igraph_is_multiple(&g, &res, es);
  PROTECT(result=R_igraph_vector_bool_to_SEXP(&res));
  igraph_vector_bool_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_count_multiple(SEXP graph, SEXP edges) {
  
  igraph_es_t es;
  igraph_t g;
  igraph_vector_t res;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_igraph_es(edges, &g, &es);
  igraph_vector_init(&res, 0);
  igraph_count_multiple(&g, &res, es);
  PROTECT(result=R_igraph_vector_to_SEXP(&res));
  igraph_vector_destroy(&res);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_famous(SEXP name) {
  
  igraph_t g;
  SEXP result;
  
  R_igraph_before();
  
  igraph_famous(&g, CHAR(STRING_ELT(name, 0)));
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_get_adjlist(SEXP graph, SEXP pmode) {
  
  igraph_t g;
  igraph_integer_t mode=REAL(pmode)[0];
  SEXP result;
  long int i;
  long int no_of_nodes;
  igraph_vector_t neis;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  no_of_nodes=igraph_vcount(&g);
  igraph_vector_init(&neis, 0);
  PROTECT(result=NEW_LIST(no_of_nodes));
  for (i=0; i<no_of_nodes; i++) {
    igraph_neighbors(&g, &neis, i, mode);
    SET_VECTOR_ELT(result, i, R_igraph_vector_to_SEXP(&neis));
  }
  igraph_vector_destroy(&neis);
  
  R_igraph_after();

  UNPROTECT(1);
  return result;
} 

SEXP R_igraph_get_adjedgelist(SEXP graph, SEXP pmode) {
  
  igraph_t g;
  igraph_integer_t mode=REAL(pmode)[0];
  SEXP result;
  long int i;
  long int no_of_nodes;
  igraph_vector_t neis;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  no_of_nodes=igraph_vcount(&g);
  igraph_vector_init(&neis, 0);
  PROTECT(result=NEW_LIST(no_of_nodes));
  for (i=0; i<no_of_nodes; i++) {
    igraph_adjacent(&g, &neis, i, mode);
    SET_VECTOR_ELT(result, i, R_igraph_vector_to_SEXP(&neis));
  }
  igraph_vector_destroy(&neis);
  
  R_igraph_after();

  UNPROTECT(1);
  return result;
} 

typedef struct R_igraph_i_arpack_data_t {
  SEXP fun;
  SEXP extra;
  SEXP rho;
} R_igraph_i_arpack_data_t;

int R_igraph_i_arpack_callback(igraph_real_t *to, const igraph_real_t *from,
			       long int n, void *extra) {
  SEXP s_from, s_to;
  SEXP R_fcall;
  R_igraph_i_arpack_data_t *data=extra;

  PROTECT(s_from=NEW_NUMERIC(n));
  memcpy(REAL(s_from), from, sizeof(igraph_real_t)*n);
  
  PROTECT(R_fcall = lang3(data->fun, s_from, data->extra));
  PROTECT(s_to = eval(R_fcall, data->rho));
  memcpy(to, REAL(s_to), sizeof(igraph_real_t)*n);
  
  UNPROTECT(3);
  return 0;
}

SEXP R_igraph_arpack(SEXP function, SEXP extra, SEXP options, SEXP rho,
		     SEXP sym) {
  
  igraph_vector_t values;
  igraph_matrix_t vectors, values2;
  R_igraph_i_arpack_data_t data;
  igraph_arpack_options_t c_options;
  SEXP result, names;

  R_igraph_before();

  if (0 != igraph_matrix_init(&vectors, 0, 0)) {
    igraph_error("Cannot run ARPACK", __FILE__, __LINE__, IGRAPH_ENOMEM);
  }
  IGRAPH_FINALLY(igraph_matrix_destroy, &vectors);
  if (LOGICAL(sym)[0]) {
    if (0 != igraph_vector_init(&values, 0)) {
      igraph_error("Cannot run ARPACK", __FILE__, __LINE__, IGRAPH_ENOMEM);
    }
    IGRAPH_FINALLY(igraph_vector_destroy, &values);
  } else {
    if (0 != igraph_matrix_init(&values2, 0, 0)) {
      igraph_error("Cannot run ARPACK", __FILE__, __LINE__, IGRAPH_ENOMEM);
    }
    IGRAPH_FINALLY(igraph_matrix_destroy, &values2);
  }
  
  data.fun=function;
  data.extra=extra;
  data.rho=rho;
  
  R_SEXP_to_igraph_arpack_options(options, &c_options);
  if (LOGICAL(sym)[0]) {
    if (0 != igraph_arpack_rssolve(R_igraph_i_arpack_callback, &data,
				   &c_options, 0, &values, &vectors)) {
      igraph_error("ARPACK failed", __FILE__, __LINE__, IGRAPH_FAILURE);
    }
  } else {
    if (0 != igraph_arpack_rnsolve(R_igraph_i_arpack_callback, &data,
				   &c_options, 0, &values2, &vectors)) {
      igraph_error("ARPACK failed", __FILE__, __LINE__, IGRAPH_FAILURE);
    }
  }    

  PROTECT(result=NEW_LIST(3));
  if (LOGICAL(sym)[0]) {
    SET_VECTOR_ELT(result, 0, R_igraph_vector_to_SEXP(&values));
    igraph_vector_destroy(&values); IGRAPH_FINALLY_CLEAN(1);
  } else {
    SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&values2));
    igraph_matrix_destroy(&values2); IGRAPH_FINALLY_CLEAN(1);
  }    
  SET_VECTOR_ELT(result, 1, R_igraph_matrix_to_SEXP(&vectors));
  igraph_matrix_destroy(&vectors); IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 2, R_igraph_arpack_options_to_SEXP(&c_options));
  PROTECT(names=NEW_CHARACTER(3));
  SET_STRING_ELT(names, 0, mkChar("values"));
  SET_STRING_ELT(names, 1, mkChar("vectors"));
  SET_STRING_ELT(names, 2, mkChar("options"));
  SET_NAMES(result, names);

  R_igraph_after();
  
  UNPROTECT(2);
  return result;
}

SEXP R_igraph_revolver_d(SEXP graph, SEXP pniter, SEXP psd, SEXP pnorm,
			SEXP pcites, SEXP pexpected, SEXP perror, SEXP pdebug,
			SEXP verbose) {
  igraph_t g;
  igraph_vector_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_bool_t sd=LOGICAL(psd)[0];
  igraph_bool_t norm=LOGICAL(pnorm)[0];
  igraph_bool_t cites=LOGICAL(pcites)[0];
  igraph_bool_t expected=LOGICAL(pexpected)[0];
  igraph_vector_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_vector_t vsd, vnorm, vcites, vexpected;
  igraph_vector_t *pvsd=0, *pvnorm=0, *pvcites=0, *pvexpected=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver d ");

  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&kernel, 0);
  if (sd) { igraph_vector_init(&vsd, 0); pvsd=&vsd; }
  if (norm) { igraph_vector_init(&vnorm, 0); pvnorm=&vnorm; }
  if (cites) { igraph_vector_init(&vcites, 0); pvcites=&vcites; }
  if (expected) { igraph_vector_init(&vexpected, 0); pvexpected=&vexpected; }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_vector(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }

  igraph_revolver_d(&g, niter, &kernel, pvsd, pvnorm, pvcites, pvexpected,
		   pplogprob, pplognull, pplogmax, ppdebug, ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_vector_to_SEXP(&kernel));
  igraph_vector_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0orvector_to_SEXP(pvsd));
  if (pvsd) { igraph_vector_destroy(pvsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0orvector_to_SEXP(pvnorm));
  if (pvnorm) { igraph_vector_destroy(pvnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0orvector_to_SEXP(pvcites));
  if (pvcites) { igraph_vector_destroy(pvcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0orvector_to_SEXP(pvexpected));
  if (pvexpected) { igraph_vector_destroy(pvexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_revolver_error2_d(SEXP graph, SEXP pkernel) {

  igraph_t g;
  igraph_vector_t kernel;
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pkernel, &kernel);
  igraph_revolver_error2_d(&g, &kernel, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}   

SEXP R_igraph_revolver_ad(SEXP graph, SEXP pniter, SEXP pagebins,
			 SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			 SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_matrix_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_bool_t sd=LOGICAL(psd)[0];
  igraph_bool_t norm=LOGICAL(pnorm)[0];
  igraph_bool_t cites=LOGICAL(pcites)[0];
  igraph_bool_t expected=LOGICAL(pexpected)[0];
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_matrix_t vsd, vnorm, vcites, vexpected;
  igraph_matrix_t *pvsd=0, *pvnorm=0, *pvcites=0, *pvexpected=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver ad ");
  
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&kernel, 0, 0);
  if (sd) { igraph_matrix_init(&vsd, 0, 0); pvsd=&vsd; }
  if (norm) { igraph_matrix_init(&vnorm, 0, 0); pvnorm=&vnorm; }
  if (cites) { igraph_matrix_init(&vcites, 0, 0); pvcites=&vcites; }
  if (expected) { igraph_matrix_init(&vexpected, 0, 0); pvexpected=&vexpected; }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }

  igraph_revolver_ad(&g, niter, agebins, &kernel, pvsd, pvnorm, pvcites,
		     pvexpected, pplogprob, pplognull, pplogmax, 
		     ppdebug, ppdebugres);

  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&kernel));
  igraph_matrix_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0ormatrix_to_SEXP(pvsd));
  if (pvsd) { igraph_matrix_destroy(pvsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0ormatrix_to_SEXP(pvnorm));
  if (pvnorm) { igraph_matrix_destroy(pvnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0ormatrix_to_SEXP(pvcites));
  if (pvcites) { igraph_matrix_destroy(pvcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0ormatrix_to_SEXP(pvexpected));
  if (pvexpected) { igraph_matrix_destroy(pvexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
  
}

SEXP R_igraph_revolver_error2_ad(SEXP graph, SEXP pkernel) {

  igraph_t g;
  igraph_matrix_t kernel;
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_matrix(pkernel, &kernel);
  igraph_revolver_error2_ad(&g, &kernel, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}   

SEXP R_igraph_revolver_ade(SEXP graph, SEXP pcats, SEXP pniter, SEXP pagebins,
			  SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			  SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_vector_t cats;
  igraph_array3_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_array3_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver ade ");
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  igraph_array3_init(&kernel, 0, 0, 0);
  if (LOGICAL(psd)[0]) { igraph_array3_init(&vsd, 0, 0, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_array3_init(&vnorm, 0, 0, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_array3_init(&vcites, 0, 0, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_array3_init(&vexpected, 0, 0, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_ade(&g, niter, agebins, &cats, &kernel, ppsd, ppnorm,
		     ppcites, ppexpected, pplogprob, pplognull, pplogmax, ppdebug,
		     ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_array3_to_SEXP(&kernel));
  igraph_array3_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0orarray3_to_SEXP(ppsd));
  if (ppsd) { igraph_array3_destroy(ppsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0orarray3_to_SEXP(ppnorm));
  if (ppnorm) { igraph_array3_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0orarray3_to_SEXP(ppcites));
  if (ppcites) { igraph_array3_destroy(ppcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0orarray3_to_SEXP(ppexpected));
  if (ppexpected) { igraph_array3_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_revolver_error2_ade(SEXP graph, SEXP pkernel, SEXP pcats) {

  igraph_t g;
  igraph_array3_t kernel;
  igraph_vector_t cats;
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  R_igraph_SEXP_to_array3(pkernel, &kernel);
  igraph_revolver_error2_ade(&g, &kernel, &cats, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}   
  
SEXP R_igraph_revolver_e(SEXP graph, SEXP pcats, SEXP pniter, SEXP pst,
			SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_vector_t cats;
  igraph_vector_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_vector_t vst, *ppst=0;
  igraph_vector_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_vector_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver e ");
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  igraph_vector_init(&kernel, 0);
  if (LOGICAL(pst)[0]) { igraph_vector_init(&vst, 0); ppst=&vst; }
  if (LOGICAL(psd)[0]) { igraph_vector_init(&vsd, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_vector_init(&vnorm, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_vector_init(&vcites, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_vector_init(&vexpected, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_vector(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_e(&g, niter, &cats, &kernel, ppst, ppsd, ppnorm, ppcites, ppexpected,
		   pplogprob, pplognull, pplogmax, ppdebug, ppdebugres);
  
  PROTECT(result=NEW_LIST(8));
  SET_VECTOR_ELT(result, 0, R_igraph_vector_to_SEXP(&kernel));
  igraph_vector_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0orvector_to_SEXP(ppst));
  if (ppst) { igraph_vector_destroy(ppst); }
  SET_VECTOR_ELT(result, 2, R_igraph_0orvector_to_SEXP(ppsd));
  if (ppsd) { igraph_vector_destroy(ppsd); }
  SET_VECTOR_ELT(result, 3, R_igraph_0orvector_to_SEXP(ppnorm));
  if (ppnorm) { igraph_vector_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 4, R_igraph_0orvector_to_SEXP(ppcites));
  if (ppcites) { igraph_vector_destroy(ppcites); }
  SET_VECTOR_ELT(result, 5, R_igraph_0orvector_to_SEXP(ppexpected));
  if (ppexpected) { igraph_vector_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 7, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 7))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 7))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 7))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(8));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("st"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 7, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_revolver_error2_e(SEXP graph, SEXP pkernel, SEXP pcats) {

  igraph_t g;
  igraph_vector_t kernel;
  igraph_vector_t cats;
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  R_SEXP_to_vector(pkernel, &kernel);
  igraph_revolver_error2_e(&g, &kernel, &cats, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}   
  
SEXP R_igraph_revolver_de(SEXP graph, SEXP pcats, SEXP pniter,
			 SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			 SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_vector_t cats;
  igraph_matrix_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_matrix_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver de ");
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  igraph_matrix_init(&kernel, 0, 0);
  if (LOGICAL(psd)[0]) { igraph_matrix_init(&vsd, 0, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_matrix_init(&vnorm, 0, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_matrix_init(&vcites, 0, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_matrix_init(&vexpected, 0, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_de(&g, niter, &cats, &kernel, ppsd, ppnorm, ppcites,
		    ppexpected, pplogprob, pplognull, pplogmax, ppdebug, ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&kernel));
  igraph_matrix_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0ormatrix_to_SEXP(ppsd));
  if (ppsd) { igraph_matrix_destroy(ppsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0ormatrix_to_SEXP(ppnorm));
  if (ppnorm) { igraph_matrix_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0ormatrix_to_SEXP(ppcites));
  if (ppcites) { igraph_matrix_destroy(ppcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0ormatrix_to_SEXP(ppexpected));
  if (ppexpected) { igraph_matrix_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}
  
SEXP R_igraph_revolver_error2_de(SEXP graph, SEXP pkernel, SEXP pcats) {

  igraph_t g;
  igraph_matrix_t kernel;
  igraph_vector_t cats;
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  R_SEXP_to_matrix(pkernel, &kernel);
  igraph_revolver_error2_de(&g, &kernel, &cats, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_revolver_l(SEXP graph, SEXP pniter, SEXP pagebins,
			SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_vector_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_vector_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_vector_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver l ");
  
  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&kernel, 0);
  if (LOGICAL(psd)[0]) { igraph_vector_init(&vsd, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_vector_init(&vnorm, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_vector_init(&vcites, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_vector_init(&vexpected, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_vector(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_l(&g, niter, agebins, &kernel, ppsd, ppnorm, 
		   ppcites, ppexpected, pplogprob, pplognull, pplogmax, ppdebug,
		   ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_vector_to_SEXP(&kernel));
  igraph_vector_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0orvector_to_SEXP(ppsd));
  if (ppsd) { igraph_vector_destroy(ppsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0orvector_to_SEXP(ppnorm));
  if (ppnorm) { igraph_vector_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0orvector_to_SEXP(ppcites));
  if (ppcites) { igraph_vector_destroy(ppcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0orvector_to_SEXP(ppexpected));
  if (ppexpected) { igraph_vector_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_revolver_error2_l(SEXP graph, SEXP pkernel) {

  igraph_t g;
  igraph_vector_t kernel;
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pkernel, &kernel);
  igraph_revolver_error2_l(&g, &kernel, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_revolver_dl(SEXP graph, SEXP pniter, SEXP pagebins,
			 SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			 SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_matrix_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_matrix_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver dl ");
  
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&kernel, 0, 0);
  if (LOGICAL(psd)[0]) { igraph_matrix_init(&vsd, 0, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_matrix_init(&vnorm, 0, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_matrix_init(&vcites, 0, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_matrix_init(&vexpected, 0, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_dl(&g, niter, agebins, &kernel, ppsd, ppnorm, 
		   ppcites, ppexpected, pplogprob, pplognull, pplogmax, ppdebug,
		   ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&kernel));
  igraph_matrix_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0ormatrix_to_SEXP(ppsd));
  if (ppsd) { igraph_matrix_destroy(ppsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0ormatrix_to_SEXP(ppnorm));
  if (ppnorm) { igraph_matrix_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0ormatrix_to_SEXP(ppcites));
  if (ppcites) { igraph_matrix_destroy(ppcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0ormatrix_to_SEXP(ppexpected));
  if (ppexpected) { igraph_matrix_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}
  
SEXP R_igraph_revolver_error2_dl(SEXP graph, SEXP pkernel) {

  igraph_t g;
  igraph_matrix_t kernel;
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_matrix(pkernel, &kernel);
  igraph_revolver_error2_dl(&g, &kernel, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_revolver_el(SEXP graph, SEXP pcats, SEXP pniter, SEXP pagebins,
			 SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			 SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_vector_t cats;
  igraph_matrix_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_matrix_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver el ");
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  igraph_matrix_init(&kernel, 0, 0);
  if (LOGICAL(psd)[0]) { igraph_matrix_init(&vsd, 0, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_matrix_init(&vnorm, 0, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_matrix_init(&vcites, 0, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_matrix_init(&vexpected, 0, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_el(&g, niter, &cats, agebins, &kernel, ppsd, ppnorm, 
		   ppcites, ppexpected, pplogprob, pplognull, pplogmax, ppdebug,
		   ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&kernel));
  igraph_matrix_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0ormatrix_to_SEXP(ppsd));
  if (ppsd) { igraph_matrix_destroy(ppsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0ormatrix_to_SEXP(ppnorm));
  if (ppnorm) { igraph_matrix_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0ormatrix_to_SEXP(ppcites));
  if (ppcites) { igraph_matrix_destroy(ppcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0ormatrix_to_SEXP(ppexpected));
  if (ppexpected) { igraph_matrix_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_revolver_error2_el(SEXP graph, SEXP pkernel, SEXP pcats) {

  igraph_t g;
  igraph_matrix_t kernel;
  igraph_vector_t cats;
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  R_SEXP_to_matrix(pkernel, &kernel);
  igraph_revolver_error2_el(&g, &kernel, &cats, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_revolver_r(SEXP graph, SEXP pniter, SEXP pwindow,
			SEXP psd, SEXP pnorm,
			SEXP pcites, SEXP pexpected, SEXP perror, SEXP pdebug,
			SEXP verbose) {
  igraph_t g;
  igraph_vector_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_integer_t window=REAL(pwindow)[0];
  igraph_bool_t sd=LOGICAL(psd)[0];
  igraph_bool_t norm=LOGICAL(pnorm)[0];
  igraph_bool_t cites=LOGICAL(pcites)[0];
  igraph_bool_t expected=LOGICAL(pexpected)[0];
  igraph_vector_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_vector_t vsd, vnorm, vcites, vexpected;
  igraph_vector_t *pvsd=0, *pvnorm=0, *pvcites=0, *pvexpected=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver r ");

  R_SEXP_to_igraph(graph, &g);
  igraph_vector_init(&kernel, 0);
  if (sd) { igraph_vector_init(&vsd, 0); pvsd=&vsd; }
  if (norm) { igraph_vector_init(&vnorm, 0); pvnorm=&vnorm; }
  if (cites) { igraph_vector_init(&vcites, 0); pvcites=&vcites; }
  if (expected) { igraph_vector_init(&vexpected, 0); pvexpected=&vexpected; }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_vector(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }

  igraph_revolver_r(&g, niter, window, &kernel, pvsd, pvnorm, pvcites, pvexpected,
		   pplogprob, pplognull, pplogmax, ppdebug, ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_vector_to_SEXP(&kernel));
  igraph_vector_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0orvector_to_SEXP(pvsd));
  if (pvsd) { igraph_vector_destroy(pvsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0orvector_to_SEXP(pvnorm));
  if (pvnorm) { igraph_vector_destroy(pvnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0orvector_to_SEXP(pvcites));
  if (pvcites) { igraph_vector_destroy(pvcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0orvector_to_SEXP(pvexpected));
  if (pvexpected) { igraph_vector_destroy(pvexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_revolver_error2_r(SEXP graph, SEXP pkernel, SEXP pwindow) {

  igraph_t g;
  igraph_vector_t kernel;
  igraph_integer_t window=REAL(pwindow)[0];
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pkernel, &kernel);
  igraph_revolver_error2_r(&g, &kernel, window, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_revolver_ar(SEXP graph, SEXP pniter, SEXP pagebins, SEXP pwindow,
			 SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			 SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_matrix_t kernel;  
  igraph_integer_t niter=REAL(pniter)[0];  
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_integer_t window=REAL(pwindow)[0];
  igraph_bool_t sd=LOGICAL(psd)[0];
  igraph_bool_t norm=LOGICAL(pnorm)[0];
  igraph_bool_t cites=LOGICAL(pcites)[0];
  igraph_bool_t expected=LOGICAL(pexpected)[0];
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_matrix_t vsd, vnorm, vcites, vexpected;
  igraph_matrix_t *pvsd=0, *pvnorm=0, *pvcites=0, *pvexpected=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver ar ");
  
  R_SEXP_to_igraph(graph, &g);
  igraph_matrix_init(&kernel, 0, 0);
  if (sd) { igraph_matrix_init(&vsd, 0, 0); pvsd=&vsd; }
  if (norm) { igraph_matrix_init(&vnorm, 0, 0); pvnorm=&vnorm; }
  if (cites) { igraph_matrix_init(&vcites, 0, 0); pvcites=&vcites; }
  if (expected) { igraph_matrix_init(&vexpected, 0, 0); pvexpected=&vexpected; }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }

  igraph_revolver_ar(&g, niter, agebins, window, &kernel, pvsd, pvnorm, pvcites,
		    pvexpected, pplogprob, pplognull, pplogmax, ppdebug, ppdebugres);

  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&kernel));
  igraph_matrix_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0ormatrix_to_SEXP(pvsd));
  if (pvsd) { igraph_matrix_destroy(pvsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0ormatrix_to_SEXP(pvnorm));
  if (pvnorm) { igraph_matrix_destroy(pvnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0ormatrix_to_SEXP(pvcites));
  if (pvcites) { igraph_matrix_destroy(pvcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0ormatrix_to_SEXP(pvexpected));
  if (pvexpected) { igraph_matrix_destroy(pvexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
  
}

SEXP R_igraph_revolver_error2_ar(SEXP graph, SEXP pkernel, SEXP pwindow) {

  igraph_t g;
  igraph_matrix_t kernel;
  igraph_integer_t window=REAL(pwindow)[0];
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_matrix(pkernel, &kernel);
  igraph_revolver_error2_ar(&g, &kernel, window, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_revolver_di(SEXP graph, SEXP pcats, SEXP pniter,
			 SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			 SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_vector_t cats;
  igraph_matrix_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_matrix_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver di ");
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  igraph_matrix_init(&kernel, 0, 0);
  if (LOGICAL(psd)[0]) { igraph_matrix_init(&vsd, 0, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_matrix_init(&vnorm, 0, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_matrix_init(&vcites, 0, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_matrix_init(&vexpected, 0, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_di(&g, niter, &cats, &kernel, ppsd, ppnorm, ppcites,
		    ppexpected, pplogprob, pplognull, pplogmax, ppdebug, ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&kernel));
  igraph_matrix_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0ormatrix_to_SEXP(ppsd));
  if (ppsd) { igraph_matrix_destroy(ppsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0ormatrix_to_SEXP(ppnorm));
  if (ppnorm) { igraph_matrix_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0ormatrix_to_SEXP(ppcites));
  if (ppcites) { igraph_matrix_destroy(ppcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0ormatrix_to_SEXP(ppexpected));
  if (ppexpected) { igraph_matrix_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_revolver_error2_di(SEXP graph, SEXP pkernel, SEXP pcats) {

  igraph_t g;
  igraph_matrix_t kernel;
  igraph_vector_t cats;
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  R_SEXP_to_matrix(pkernel, &kernel);
  igraph_revolver_error2_di(&g, &kernel, &cats, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_revolver_adi(SEXP graph, SEXP pcats, SEXP pniter, SEXP pagebins,
			  SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			  SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_vector_t cats;
  igraph_array3_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_array3_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver adi ");
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  igraph_array3_init(&kernel, 0, 0, 0);
  if (LOGICAL(psd)[0]) { igraph_array3_init(&vsd, 0, 0, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_array3_init(&vnorm, 0, 0, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_array3_init(&vcites, 0, 0, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_array3_init(&vexpected, 0, 0, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_adi(&g, niter, agebins, &cats, &kernel, ppsd, ppnorm,
		     ppcites, ppexpected, pplogprob, pplognull, pplogmax, ppdebug,
		     ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_array3_to_SEXP(&kernel));
  igraph_array3_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0orarray3_to_SEXP(ppsd));
  if (ppsd) { igraph_array3_destroy(ppsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0orarray3_to_SEXP(ppnorm));
  if (ppnorm) { igraph_array3_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0orarray3_to_SEXP(ppcites));
  if (ppcites) { igraph_array3_destroy(ppcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0orarray3_to_SEXP(ppexpected));
  if (ppexpected) { igraph_array3_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_revolver_error2_adi(SEXP graph, SEXP pkernel, SEXP pcats) {

  igraph_t g;
  igraph_array3_t kernel;
  igraph_vector_t cats;
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  R_igraph_SEXP_to_array3(pkernel, &kernel);
  igraph_revolver_error2_adi(&g, &kernel, &cats, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_revolver_il(SEXP graph, SEXP pcats, SEXP pniter, SEXP pagebins,
			 SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			 SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_vector_t cats;
  igraph_matrix_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_matrix_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver il ");
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  igraph_matrix_init(&kernel, 0, 0);
  if (LOGICAL(psd)[0]) { igraph_matrix_init(&vsd, 0, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_matrix_init(&vnorm, 0, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_matrix_init(&vcites, 0, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_matrix_init(&vexpected, 0, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_il(&g, niter, agebins, &cats, &kernel, ppsd, ppnorm, 
		    ppcites, ppexpected, pplogprob, pplognull, pplogmax, ppdebug,
		    ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&kernel));
  igraph_matrix_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0ormatrix_to_SEXP(ppsd));
  if (ppsd) { igraph_matrix_destroy(ppsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0ormatrix_to_SEXP(ppnorm));
  if (ppnorm) { igraph_matrix_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0ormatrix_to_SEXP(ppcites));
  if (ppcites) { igraph_matrix_destroy(ppcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0ormatrix_to_SEXP(ppexpected));
  if (ppexpected) { igraph_matrix_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_revolver_error2_il(SEXP graph, SEXP pkernel, SEXP pcats) {

  igraph_t g;
  igraph_matrix_t kernel;
  igraph_vector_t cats;
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  R_SEXP_to_matrix(pkernel, &kernel);
  igraph_revolver_error2_il(&g, &kernel, &cats, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_revolver_ir(SEXP graph, SEXP pcats, SEXP pwindow, SEXP pniter,
			 SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			 SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_vector_t cats;
  igraph_integer_t window=REAL(pwindow)[0];
  igraph_matrix_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_matrix_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver di ");
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  igraph_matrix_init(&kernel, 0, 0);
  if (LOGICAL(psd)[0]) { igraph_matrix_init(&vsd, 0, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_matrix_init(&vnorm, 0, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_matrix_init(&vcites, 0, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_matrix_init(&vexpected, 0, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_ir(&g, niter, window, &cats, &kernel, ppsd, ppnorm, ppcites,
		    ppexpected, pplogprob, pplognull, pplogmax, ppdebug, ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&kernel));
  igraph_matrix_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0ormatrix_to_SEXP(ppsd));
  if (ppsd) { igraph_matrix_destroy(ppsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0ormatrix_to_SEXP(ppnorm));
  if (ppnorm) { igraph_matrix_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0ormatrix_to_SEXP(ppcites));
  if (ppcites) { igraph_matrix_destroy(ppcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0ormatrix_to_SEXP(ppexpected));
  if (ppexpected) { igraph_matrix_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_revolver_error2_ir(SEXP graph, SEXP pkernel, SEXP pcats, 
				 SEXP pwindow) {

  igraph_t g;
  igraph_matrix_t kernel;
  igraph_vector_t cats;
  igraph_integer_t window=REAL(pwindow)[0];
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  R_SEXP_to_matrix(pkernel, &kernel);
  igraph_revolver_error2_ir(&g, &kernel, &cats, window, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_revolver_air(SEXP graph, SEXP pcats, SEXP pwindow, 
			  SEXP pniter, SEXP pagebins,
			  SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			  SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_vector_t cats;
  igraph_integer_t window=REAL(pwindow)[0];
  igraph_array3_t kernel;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_integer_t agebins=REAL(pagebins)[0];
  igraph_array3_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0, rlogmax, *pplogmax=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revolver air ");
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  igraph_array3_init(&kernel, 0, 0, 0);
  if (LOGICAL(psd)[0]) { igraph_array3_init(&vsd, 0, 0, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_array3_init(&vnorm, 0, 0, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_array3_init(&vcites, 0, 0, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_array3_init(&vexpected, 0, 0, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; pplogmax=&rlogmax; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_air(&g, niter, window, agebins, &cats, &kernel, ppsd, ppnorm,
		     ppcites, ppexpected, pplogprob, pplognull, pplogmax, ppdebug,
		     ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_array3_to_SEXP(&kernel));
  igraph_array3_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0orarray3_to_SEXP(ppsd));
  if (ppsd) { igraph_array3_destroy(ppsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0orarray3_to_SEXP(ppnorm));
  if (ppnorm) { igraph_array3_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0orarray3_to_SEXP(ppcites));
  if (ppcites) { igraph_array3_destroy(ppcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0orarray3_to_SEXP(ppexpected));
  if (ppexpected) { igraph_array3_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(3));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
    REAL(VECTOR_ELT(result, 6))[2]=*pplogmax;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_revolver_error2_air(SEXP graph, SEXP pkernel, SEXP pcats, 
				  SEXP pwindow) {

  igraph_t g;
  igraph_array3_t kernel;
  igraph_vector_t cats;
  igraph_integer_t window=REAL(pwindow)[0];
  igraph_real_t logprob, lognull;
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pcats, &cats);
  R_igraph_SEXP_to_array3(pkernel, &kernel);
  igraph_revolver_error2_air(&g, &kernel, &cats, window, &logprob, &lognull);
  PROTECT(result=NEW_NUMERIC(2));  
  REAL(result)[0]=logprob;
  REAL(result)[1]=lognull;
   
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}

SEXP R_igraph_revolver_d_d(SEXP graph, SEXP pniter, SEXP pvtime, SEXP petime,
			   SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			   SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_vector_t vtime, etime;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_matrix_t kernel;
  igraph_matrix_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revover d-d ");
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pvtime, &vtime);
  R_SEXP_to_vector(petime, &etime);
  igraph_matrix_init(&kernel, 0, 0);
  if (LOGICAL(psd)[0]) { igraph_matrix_init(&vsd, 0, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_matrix_init(&vnorm, 0, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_matrix_init(&vcites, 0, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_matrix_init(&vexpected, 0, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_d_d(&g, niter, &vtime, &etime, &kernel, ppsd, ppnorm,
		      ppcites, ppexpected, pplogprob, pplognull,
		      ppdebug, ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&kernel));
  igraph_matrix_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0ormatrix_to_SEXP(ppsd));
  if (ppsd) { igraph_matrix_destroy(ppsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0ormatrix_to_SEXP(ppnorm));
  if (ppnorm) { igraph_matrix_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0ormatrix_to_SEXP(ppcites));
  if (ppcites) { igraph_matrix_destroy(ppcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0ormatrix_to_SEXP(ppexpected));
  if (ppexpected) { igraph_matrix_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(2));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}

SEXP R_igraph_evolver_d(SEXP pnodes, SEXP pkernel, SEXP poutseq, 
			SEXP poutdist, SEXP pm, SEXP pdirected) {
  
  igraph_t g;
  igraph_integer_t nodes=REAL(pnodes)[0];
  igraph_vector_t kernel;
  igraph_vector_t voutseq, *ppoutseq=0;
  igraph_vector_t voutdist, *ppoutdist=0;
  igraph_integer_t m=REAL(pm)[0];
  igraph_bool_t directed=LOGICAL(pdirected)[0];
  SEXP result;
  
  R_igraph_before();
  
  R_SEXP_to_vector(pkernel, &kernel);
  if (!isNull(poutseq)) {
    R_SEXP_to_vector(poutseq, &voutseq);
    ppoutseq=&voutseq;
  }
  if (!isNull(poutdist)) {
    R_SEXP_to_vector(poutdist, &voutdist);
    ppoutdist=&voutdist;
  }
  
  igraph_evolver_d(&g, nodes, &kernel, ppoutseq, ppoutdist, m, directed);
  PROTECT(result=R_igraph_to_SEXP(&g));
  igraph_destroy(&g);
  
  R_igraph_after();
  
  UNPROTECT(1);
  return result;
}
		   
SEXP R_igraph_revolver_p_p(SEXP graph, SEXP pniter, SEXP pvtime, SEXP petime,
			   SEXP pauthors, SEXP peventsizes,
			   SEXP psd, SEXP pnorm, SEXP pcites, SEXP pexpected,
			   SEXP perror, SEXP pdebug, SEXP verbose) {
  igraph_t g;
  igraph_vector_t vtime, etime;
  igraph_vector_t authors, eventsizes;
  igraph_integer_t niter=REAL(pniter)[0];
  igraph_matrix_t kernel;
  igraph_matrix_t vsd, *ppsd=0, vnorm, *ppnorm=0, vcites, *ppcites=0,
    vexpected, *ppexpected=0;
  igraph_matrix_t debug, *ppdebug=0;
  igraph_vector_ptr_t debugres, *ppdebugres=0;
  igraph_real_t rlogprob, rlognull, *pplogprob=0, *pplognull=0;
  SEXP result, names;
  
  R_igraph_before2(verbose, "Revover d-d ");
  
  R_SEXP_to_igraph(graph, &g);
  R_SEXP_to_vector(pvtime, &vtime);
  R_SEXP_to_vector(petime, &etime);
  R_SEXP_to_vector(pauthors, &authors);
  R_SEXP_to_vector(peventsizes, &eventsizes);
  igraph_matrix_init(&kernel, 0, 0);
  if (LOGICAL(psd)[0]) { igraph_matrix_init(&vsd, 0, 0); ppsd=&vsd; }
  if (LOGICAL(pnorm)[0]) { igraph_matrix_init(&vnorm, 0, 0); ppnorm=&vnorm; }
  if (LOGICAL(pcites)[0]) { igraph_matrix_init(&vcites, 0, 0); ppcites=&vcites; }
  if (LOGICAL(pexpected)[0]) { 
    igraph_matrix_init(&vexpected, 0, 0); 
    ppexpected=&vexpected; 
  }
  if (LOGICAL(perror)[0]) { pplogprob=&rlogprob; pplognull=&rlognull; }
  if (!isNull(pdebug) && GET_LENGTH(pdebug)!=0) {
    R_SEXP_to_matrix(pdebug, &debug); ppdebug=&debug; 
    igraph_vector_ptr_init(&debugres, 0); ppdebugres=&debugres;
  }
  
  igraph_revolver_p_p(&g, niter, &vtime, &etime, &authors, &eventsizes,
		      &kernel, ppsd, ppnorm,
		      ppcites, ppexpected, pplogprob, pplognull,
		      ppdebug, ppdebugres);
  
  PROTECT(result=NEW_LIST(7));
  SET_VECTOR_ELT(result, 0, R_igraph_matrix_to_SEXP(&kernel));
  igraph_matrix_destroy(&kernel);
  SET_VECTOR_ELT(result, 1, R_igraph_0ormatrix_to_SEXP(ppsd));
  if (ppsd) { igraph_matrix_destroy(ppsd); }
  SET_VECTOR_ELT(result, 2, R_igraph_0ormatrix_to_SEXP(ppnorm));
  if (ppnorm) { igraph_matrix_destroy(ppnorm); }
  SET_VECTOR_ELT(result, 3, R_igraph_0ormatrix_to_SEXP(ppcites));
  if (ppcites) { igraph_matrix_destroy(ppcites); }
  SET_VECTOR_ELT(result, 4, R_igraph_0ormatrix_to_SEXP(ppexpected));
  if (ppexpected) { igraph_matrix_destroy(ppexpected); }
  if (!isNull(pdebug) && GET_LENGTH(pdebug) != 0) {
    /* TODO */
  } else {
    SET_VECTOR_ELT(result, 5, R_NilValue);
  }
  if (pplogprob) {
    SET_VECTOR_ELT(result, 6, NEW_NUMERIC(2));
    REAL(VECTOR_ELT(result, 6))[0]=*pplogprob;
    REAL(VECTOR_ELT(result, 6))[1]=*pplognull;
  } else {
    SET_VECTOR_ELT(result, 6, R_NilValue);
  }
  PROTECT(names=NEW_CHARACTER(7));
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("sd"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("norm"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("expected"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("debug"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("error"));
  SET_NAMES(result, names);
  
  R_igraph_after2(verbose);  

  UNPROTECT(2);
  return result;
}		   
			 
/***********************************************/
/* THE REST IS GENERATED BY inger.py           */
/***********************************************/

/*-------------------------------------------/
/ igraph_empty                               /
/-------------------------------------------*/
SEXP R_igraph_empty(SEXP n, SEXP directed) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_n;
  igraph_bool_t c_directed;
  SEXP graph;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  c_n=REAL(n)[0];
  c_directed=LOGICAL(directed)[0];
                                        /* Call igraph */
  c_result=igraph_empty(&c_graph, c_n, c_directed);

                                        /* Convert output */
  IGRAPH_FINALLY(igraph_destroy, &c_graph); 
  PROTECT(graph=R_igraph_to_SEXP(&c_graph));  
  igraph_destroy(&c_graph); 
  IGRAPH_FINALLY_CLEAN(1);
  result=graph;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_vcount                              /
/-------------------------------------------*/
SEXP R_igraph_vcount(SEXP graph) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
                                        /* Call igraph */
  c_result=igraph_vcount(&c_graph);

                                        /* Convert output */

  PROTECT(result=NEW_NUMERIC(1)); 
  REAL(result)[0]=c_result;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_full_citation                       /
/-------------------------------------------*/
SEXP R_igraph_full_citation(SEXP n, SEXP directed) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_n;
  igraph_bool_t c_directed;
  SEXP graph;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  c_n=REAL(n)[0];
  c_directed=LOGICAL(directed)[0];
                                        /* Call igraph */
  c_result=igraph_full_citation(&c_graph, c_n, c_directed);

                                        /* Convert output */
  IGRAPH_FINALLY(igraph_destroy, &c_graph); 
  PROTECT(graph=R_igraph_to_SEXP(&c_graph));  
  igraph_destroy(&c_graph); 
  IGRAPH_FINALLY_CLEAN(1);
  result=graph;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_lcf_vector                          /
/-------------------------------------------*/
SEXP R_igraph_lcf_vector(SEXP n, SEXP shifts, SEXP repeats) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_n;
  igraph_vector_t c_shifts;
  igraph_integer_t c_repeats;
  SEXP graph;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  c_n=REAL(n)[0];
  R_SEXP_to_vector(shifts, &c_shifts);
  c_repeats=REAL(repeats)[0];
                                        /* Call igraph */
  c_result=igraph_lcf_vector(&c_graph, c_n, &c_shifts, c_repeats);

                                        /* Convert output */
  IGRAPH_FINALLY(igraph_destroy, &c_graph); 
  PROTECT(graph=R_igraph_to_SEXP(&c_graph));  
  igraph_destroy(&c_graph); 
  IGRAPH_FINALLY_CLEAN(1);
  result=graph;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_adjlist                             /
/-------------------------------------------*/
SEXP R_igraph_adjlist(SEXP adjlist, SEXP directed, SEXP duplicate) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_adjlist_t c_adjlist;
  igraph_bool_t c_directed;
  igraph_bool_t c_duplicate;
  SEXP graph;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  if (0 != R_SEXP_to_igraph_adjlist(adjlist, &c_adjlist)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  }
  c_directed=LOGICAL(directed)[0];
  c_duplicate=LOGICAL(duplicate)[0];
                                        /* Call igraph */
  c_result=igraph_adjlist(&c_graph, &c_adjlist, c_directed, c_duplicate);

                                        /* Convert output */
  IGRAPH_FINALLY(igraph_destroy, &c_graph); 
  PROTECT(graph=R_igraph_to_SEXP(&c_graph));  
  igraph_destroy(&c_graph); 
  IGRAPH_FINALLY_CLEAN(1);
  result=graph;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_full_bipartite                      /
/-------------------------------------------*/
SEXP R_igraph_full_bipartite(SEXP n1, SEXP n2, SEXP directed, SEXP mode) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_bool_t c_types;
  igraph_integer_t c_n1;
  igraph_integer_t c_n2;
  igraph_bool_t c_directed;
  igraph_integer_t c_mode;
  SEXP graph;
  SEXP types;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  if (0 != igraph_vector_bool_init(&c_types, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_bool_destroy, &c_types); 
  types=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  c_n1=REAL(n1)[0];
  c_n2=REAL(n2)[0];
  c_directed=LOGICAL(directed)[0];
  c_mode=REAL(mode)[0];
                                        /* Call igraph */
  c_result=igraph_full_bipartite(&c_graph, (isNull(types) ? 0 : &c_types), c_n1, c_n2, c_directed, c_mode);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(2));
  PROTECT(names=NEW_CHARACTER(2));
  IGRAPH_FINALLY(igraph_destroy, &c_graph); 
  PROTECT(graph=R_igraph_to_SEXP(&c_graph));  
  igraph_destroy(&c_graph); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(types=R_igraph_0orvector_bool_to_SEXP(&c_types)); 
  igraph_vector_bool_destroy(&c_types); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, graph);
  SET_VECTOR_ELT(result, 1, types);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("graph"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("types"));
  SET_NAMES(result, names);
  UNPROTECT(3);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_forest_fire_game                    /
/-------------------------------------------*/
SEXP R_igraph_forest_fire_game(SEXP nodes, SEXP fw_prob, SEXP bw_factor, SEXP ambs, SEXP directed, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_nodes;
  igraph_real_t c_fw_prob;
  igraph_real_t c_bw_factor;
  igraph_integer_t c_ambs;
  igraph_bool_t c_directed;
  SEXP graph;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  c_nodes=REAL(nodes)[0];
  c_fw_prob=REAL(fw_prob)[0];
  c_bw_factor=REAL(bw_factor)[0];
  c_ambs=REAL(ambs)[0];
  c_directed=LOGICAL(directed)[0];
                                        /* Call igraph */
  c_result=igraph_forest_fire_game(&c_graph, c_nodes, c_fw_prob, c_bw_factor, c_ambs, c_directed);

                                        /* Convert output */
  IGRAPH_FINALLY(igraph_destroy, &c_graph); 
  PROTECT(graph=R_igraph_to_SEXP(&c_graph));  
  igraph_destroy(&c_graph); 
  IGRAPH_FINALLY_CLEAN(1);
  result=graph;

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_closeness_estimate                  /
/-------------------------------------------*/
SEXP R_igraph_closeness_estimate(SEXP graph, SEXP vids, SEXP mode, SEXP cutoff) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_res;
  igraph_vs_t c_vids;
  igraph_integer_t c_mode;
  igraph_integer_t c_cutoff;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_res, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_res);
  R_SEXP_to_igraph_vs(vids, &c_graph, &c_vids);
  c_mode=REAL(mode)[0];
  c_cutoff=REAL(cutoff)[0];
                                        /* Call igraph */
  c_result=igraph_closeness_estimate(&c_graph, &c_res, c_vids, c_mode, c_cutoff);

                                        /* Convert output */
  PROTECT(res=R_igraph_vector_to_SEXP(&c_res)); 
  igraph_vector_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  igraph_vs_destroy(&c_vids);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_betweenness_estimate                /
/-------------------------------------------*/
SEXP R_igraph_betweenness_estimate(SEXP graph, SEXP vids, SEXP directed, SEXP cutoff, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_res;
  igraph_vs_t c_vids;
  igraph_bool_t c_directed;
  igraph_integer_t c_cutoff;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_res, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_res);
  R_SEXP_to_igraph_vs(vids, &c_graph, &c_vids);
  c_directed=LOGICAL(directed)[0];
  c_cutoff=REAL(cutoff)[0];
                                        /* Call igraph */
  c_result=igraph_betweenness_estimate(&c_graph, &c_res, c_vids, c_directed, c_cutoff);

                                        /* Convert output */
  PROTECT(res=R_igraph_vector_to_SEXP(&c_res)); 
  igraph_vector_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  igraph_vs_destroy(&c_vids);
  result=res;

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_edge_betweenness_estimate           /
/-------------------------------------------*/
SEXP R_igraph_edge_betweenness_estimate(SEXP graph, SEXP directed, SEXP cutoff) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_res;
  igraph_bool_t c_directed;
  igraph_integer_t c_cutoff;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_res, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_res);
  c_directed=LOGICAL(directed)[0];
  c_cutoff=REAL(cutoff)[0];
                                        /* Call igraph */
  c_result=igraph_edge_betweenness_estimate(&c_graph, &c_res, c_directed, c_cutoff);

                                        /* Convert output */
  PROTECT(res=R_igraph_vector_to_SEXP(&c_res)); 
  igraph_vector_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_pagerank_old                        /
/-------------------------------------------*/
SEXP R_igraph_pagerank_old(SEXP graph, SEXP vids, SEXP directed, SEXP niter, SEXP eps, SEXP damping, SEXP old) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_res;
  igraph_vs_t c_vids;
  igraph_bool_t c_directed;
  igraph_integer_t c_niter;
  igraph_real_t c_eps;
  igraph_real_t c_damping;
  igraph_bool_t c_old;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_res, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_res);
  R_SEXP_to_igraph_vs(vids, &c_graph, &c_vids);
  c_directed=LOGICAL(directed)[0];
  c_niter=REAL(niter)[0];
  c_eps=REAL(eps)[0];
  c_damping=REAL(damping)[0];
  c_old=LOGICAL(old)[0];
                                        /* Call igraph */
  c_result=igraph_pagerank_old(&c_graph, &c_res, c_vids, c_directed, c_niter, c_eps, c_damping, c_old);

                                        /* Convert output */
  PROTECT(res=R_igraph_vector_to_SEXP(&c_res)); 
  igraph_vector_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  igraph_vs_destroy(&c_vids);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_pagerank                            /
/-------------------------------------------*/
SEXP R_igraph_pagerank(SEXP graph, SEXP vids, SEXP directed, SEXP damping, SEXP weights, SEXP options) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_vector;
  igraph_real_t c_value;
  igraph_vs_t c_vids;
  igraph_bool_t c_directed;
  igraph_real_t c_damping;
  igraph_vector_t c_weights;
  igraph_arpack_options_t c_options;
  SEXP vector;
  SEXP value;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_vector, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_vector);
  R_SEXP_to_igraph_vs(vids, &c_graph, &c_vids);
  c_directed=LOGICAL(directed)[0];
  c_damping=REAL(damping)[0];
  if (!isNull(weights)) { R_SEXP_to_vector(weights, &c_weights); }
  R_SEXP_to_igraph_arpack_options(options, &c_options);
                                        /* Call igraph */
  c_result=igraph_pagerank(&c_graph, &c_vector, &c_value, c_vids, c_directed, c_damping, (isNull(weights) ? 0 : &c_weights), &c_options);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(vector=R_igraph_vector_to_SEXP(&c_vector)); 
  igraph_vector_destroy(&c_vector); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(value=NEW_NUMERIC(1)); 
  REAL(value)[0]=c_value;
  igraph_vs_destroy(&c_vids);
  PROTECT(options=R_igraph_arpack_options_to_SEXP(&c_options));
  SET_VECTOR_ELT(result, 0, vector);
  SET_VECTOR_ELT(result, 1, value);
  SET_VECTOR_ELT(result, 2, options);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("vector"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("value"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("options"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_path_length_hist                    /
/-------------------------------------------*/
SEXP R_igraph_path_length_hist(SEXP graph, SEXP directed, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_res;
  igraph_real_t c_unconnected;
  igraph_bool_t c_directed;
  SEXP res;
  SEXP unconnected;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_res, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_res);
  c_directed=LOGICAL(directed)[0];
                                        /* Call igraph */
  c_result=igraph_path_length_hist(&c_graph, &c_res, &c_unconnected, c_directed);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(2));
  PROTECT(names=NEW_CHARACTER(2));
  PROTECT(res=R_igraph_vector_to_SEXP(&c_res)); 
  igraph_vector_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(unconnected=NEW_NUMERIC(1)); 
  REAL(unconnected)[0]=c_unconnected;
  SET_VECTOR_ELT(result, 0, res);
  SET_VECTOR_ELT(result, 1, unconnected);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("res"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("unconnected"));
  SET_NAMES(result, names);
  UNPROTECT(3);

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_is_simple                           /
/-------------------------------------------*/
SEXP R_igraph_is_simple(SEXP graph) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_bool_t c_res;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
                                        /* Call igraph */
  c_result=igraph_is_simple(&c_graph, &c_res);

                                        /* Convert output */
  PROTECT(res=NEW_LOGICAL(1)); 
  LOGICAL(res)[0]=c_res;
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_eigenvector_centrality              /
/-------------------------------------------*/
SEXP R_igraph_eigenvector_centrality(SEXP graph, SEXP scale, SEXP weights, SEXP options) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_vector;
  igraph_real_t c_value;
  igraph_bool_t c_scale;
  igraph_vector_t c_weights;
  igraph_arpack_options_t c_options;
  SEXP vector;
  SEXP value;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_vector, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_vector);
  c_scale=LOGICAL(scale)[0];
  if (!isNull(weights)) { R_SEXP_to_vector(weights, &c_weights); }
  R_SEXP_to_igraph_arpack_options(options, &c_options);
                                        /* Call igraph */
  c_result=igraph_eigenvector_centrality(&c_graph, &c_vector, &c_value, c_scale, (isNull(weights) ? 0 : &c_weights), &c_options);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(vector=R_igraph_vector_to_SEXP(&c_vector)); 
  igraph_vector_destroy(&c_vector); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(value=NEW_NUMERIC(1)); 
  REAL(value)[0]=c_value;
  PROTECT(options=R_igraph_arpack_options_to_SEXP(&c_options));
  SET_VECTOR_ELT(result, 0, vector);
  SET_VECTOR_ELT(result, 1, value);
  SET_VECTOR_ELT(result, 2, options);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("vector"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("value"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("options"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_hub_score                           /
/-------------------------------------------*/
SEXP R_igraph_hub_score(SEXP graph, SEXP scale, SEXP options) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_vector;
  igraph_real_t c_value;
  igraph_bool_t c_scale;
  igraph_arpack_options_t c_options;
  SEXP vector;
  SEXP value;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_vector, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_vector);
  c_scale=LOGICAL(scale)[0];
  R_SEXP_to_igraph_arpack_options(options, &c_options);
                                        /* Call igraph */
  c_result=igraph_hub_score(&c_graph, &c_vector, &c_value, c_scale, &c_options);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(vector=R_igraph_vector_to_SEXP(&c_vector)); 
  igraph_vector_destroy(&c_vector); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(value=NEW_NUMERIC(1)); 
  REAL(value)[0]=c_value;
  PROTECT(options=R_igraph_arpack_options_to_SEXP(&c_options));
  SET_VECTOR_ELT(result, 0, vector);
  SET_VECTOR_ELT(result, 1, value);
  SET_VECTOR_ELT(result, 2, options);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("vector"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("value"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("options"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_authority_score                     /
/-------------------------------------------*/
SEXP R_igraph_authority_score(SEXP graph, SEXP scale, SEXP options) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_vector;
  igraph_real_t c_value;
  igraph_bool_t c_scale;
  igraph_arpack_options_t c_options;
  SEXP vector;
  SEXP value;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_vector, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_vector);
  c_scale=LOGICAL(scale)[0];
  R_SEXP_to_igraph_arpack_options(options, &c_options);
                                        /* Call igraph */
  c_result=igraph_authority_score(&c_graph, &c_vector, &c_value, c_scale, &c_options);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(vector=R_igraph_vector_to_SEXP(&c_vector)); 
  igraph_vector_destroy(&c_vector); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(value=NEW_NUMERIC(1)); 
  REAL(value)[0]=c_value;
  PROTECT(options=R_igraph_arpack_options_to_SEXP(&c_options));
  SET_VECTOR_ELT(result, 0, vector);
  SET_VECTOR_ELT(result, 1, value);
  SET_VECTOR_ELT(result, 2, options);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("vector"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("value"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("options"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_arpack_unpack_complex               /
/-------------------------------------------*/
SEXP R_igraph_arpack_unpack_complex(SEXP vectors, SEXP values, SEXP nev) {
                                        /* Declarations */
  igraph_matrix_t c_vectors;
  igraph_matrix_t c_values;
  igraph_real_t c_nev;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  if (0 != R_SEXP_to_igraph_matrix_copy(vectors, &c_vectors)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_vectors);
  if (0 != R_SEXP_to_igraph_matrix_copy(values, &c_values)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_values);
  c_nev=REAL(nev)[0];
                                        /* Call igraph */
  c_result=igraph_arpack_unpack_complex(&c_vectors, &c_values, c_nev);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(2));
  PROTECT(names=NEW_CHARACTER(2));
  PROTECT(vectors=R_igraph_matrix_to_SEXP(&c_vectors)); 
  igraph_matrix_destroy(&c_vectors); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(values=R_igraph_matrix_to_SEXP(&c_values)); 
  igraph_matrix_destroy(&c_values); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, vectors);
  SET_VECTOR_ELT(result, 1, values);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("vectors"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("values"));
  SET_NAMES(result, names);
  UNPROTECT(3);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_unfold_tree                         /
/-------------------------------------------*/
SEXP R_igraph_unfold_tree(SEXP graph, SEXP mode, SEXP roots) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_t c_tree;
  igraph_integer_t c_mode;
  igraph_vector_t c_roots;
  igraph_vector_t c_vertex_index;
  SEXP tree;
  SEXP vertex_index;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_mode=REAL(mode)[0];
  R_SEXP_to_vector(roots, &c_roots);
  if (0 != igraph_vector_init(&c_vertex_index, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_vertex_index); 
  vertex_index=NEW_NUMERIC(0); /* hack to have a non-NULL value */
                                        /* Call igraph */
  c_result=igraph_unfold_tree(&c_graph, &c_tree, c_mode, &c_roots, (isNull(vertex_index) ? 0 : &c_vertex_index));

                                        /* Convert output */
  PROTECT(result=NEW_LIST(2));
  PROTECT(names=NEW_CHARACTER(2));
  IGRAPH_FINALLY(igraph_destroy, &c_tree); 
  PROTECT(tree=R_igraph_to_SEXP(&c_tree));  
  igraph_destroy(&c_tree); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(vertex_index=R_igraph_0orvector_to_SEXP(&c_vertex_index)); 
  igraph_vector_destroy(&c_vertex_index); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, tree);
  SET_VECTOR_ELT(result, 1, vertex_index);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("tree"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("vertex_index"));
  SET_NAMES(result, names);
  UNPROTECT(3);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_is_mutual                           /
/-------------------------------------------*/
SEXP R_igraph_is_mutual(SEXP graph, SEXP es) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_bool_t c_res;
  igraph_es_t c_es;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_bool_init(&c_res, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_bool_destroy, &c_res);
  R_SEXP_to_igraph_es(es, &c_graph, &c_es);
                                        /* Call igraph */
  c_result=igraph_is_mutual(&c_graph, &c_res, c_es);

                                        /* Convert output */
  PROTECT(res=R_igraph_vector_bool_to_SEXP(&c_res)); 
  igraph_vector_bool_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  igraph_es_destroy(&c_es);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_avg_nearest_neighbor_degree         /
/-------------------------------------------*/
SEXP R_igraph_avg_nearest_neighbor_degree(SEXP graph, SEXP vids, SEXP weights) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vs_t c_vids;
  igraph_vector_t c_knn;
  igraph_vector_t c_knnk;
  igraph_vector_t c_weights;
  SEXP knn;
  SEXP knnk;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_igraph_vs(vids, &c_graph, &c_vids);
  if (0 != igraph_vector_init(&c_knn, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_knn); 
  knn=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_knnk, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_knnk); 
  knnk=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (!isNull(weights)) { R_SEXP_to_vector(weights, &c_weights); }
                                        /* Call igraph */
  c_result=igraph_avg_nearest_neighbor_degree(&c_graph, c_vids, (isNull(knn) ? 0 : &c_knn), (isNull(knnk) ? 0 : &c_knnk), (isNull(weights) ? 0 : &c_weights));

                                        /* Convert output */
  PROTECT(result=NEW_LIST(2));
  PROTECT(names=NEW_CHARACTER(2));
  igraph_vs_destroy(&c_vids);
  PROTECT(knn=R_igraph_0orvector_to_SEXP(&c_knn)); 
  igraph_vector_destroy(&c_knn); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(knnk=R_igraph_0orvector_to_SEXP(&c_knnk)); 
  igraph_vector_destroy(&c_knnk); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, knn);
  SET_VECTOR_ELT(result, 1, knnk);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("knn"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("knnk"));
  SET_NAMES(result, names);
  UNPROTECT(3);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_strength                            /
/-------------------------------------------*/
SEXP R_igraph_strength(SEXP graph, SEXP vids, SEXP mode, SEXP loops, SEXP weights) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_res;
  igraph_vs_t c_vids;
  igraph_integer_t c_mode;
  igraph_bool_t c_loops;
  igraph_vector_t c_weights;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_res, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_res);
  R_SEXP_to_igraph_vs(vids, &c_graph, &c_vids);
  c_mode=REAL(mode)[0];
  c_loops=LOGICAL(loops)[0];
  if (!isNull(weights)) { R_SEXP_to_vector(weights, &c_weights); }
                                        /* Call igraph */
  c_result=igraph_strength(&c_graph, &c_res, c_vids, c_mode, c_loops, (isNull(weights) ? 0 : &c_weights));

                                        /* Convert output */
  PROTECT(res=R_igraph_vector_to_SEXP(&c_res)); 
  igraph_vector_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  igraph_vs_destroy(&c_vids);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_bipartite_projection_size           /
/-------------------------------------------*/
SEXP R_igraph_bipartite_projection_size(SEXP graph, SEXP types) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_bool_t c_types;
  igraph_integer_t c_vcount1;
  igraph_integer_t c_ecount1;
  igraph_integer_t c_vcount2;
  igraph_integer_t c_ecount2;
  SEXP vcount1;
  SEXP ecount1;
  SEXP vcount2;
  SEXP ecount2;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (!isNull(types)) { R_SEXP_to_vector_bool(types, &c_types); }
                                        /* Call igraph */
  c_result=igraph_bipartite_projection_size(&c_graph, (isNull(types) ? 0 : &c_types), &c_vcount1, &c_ecount1, &c_vcount2, &c_ecount2);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(4));
  PROTECT(names=NEW_CHARACTER(4));
  PROTECT(vcount1=NEW_NUMERIC(1)); 
  REAL(vcount1)[0]=c_vcount1;
  PROTECT(ecount1=NEW_NUMERIC(1)); 
  REAL(ecount1)[0]=c_ecount1;
  PROTECT(vcount2=NEW_NUMERIC(1)); 
  REAL(vcount2)[0]=c_vcount2;
  PROTECT(ecount2=NEW_NUMERIC(1)); 
  REAL(ecount2)[0]=c_ecount2;
  SET_VECTOR_ELT(result, 0, vcount1);
  SET_VECTOR_ELT(result, 1, ecount1);
  SET_VECTOR_ELT(result, 2, vcount2);
  SET_VECTOR_ELT(result, 3, ecount2);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("vcount1"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("ecount1"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("vcount2"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("ecount2"));
  SET_NAMES(result, names);
  UNPROTECT(5);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_bipartite_projection                /
/-------------------------------------------*/
SEXP R_igraph_bipartite_projection(SEXP graph, SEXP types, SEXP probe1) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_bool_t c_types;
  igraph_t c_proj1;
  igraph_t c_proj2;
  igraph_integer_t c_probe1;
  SEXP proj1;
  SEXP proj2;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (!isNull(types)) { R_SEXP_to_vector_bool(types, &c_types); }
  c_probe1=REAL(probe1)[0];
                                        /* Call igraph */
  c_result=igraph_bipartite_projection(&c_graph, (isNull(types) ? 0 : &c_types), &c_proj1, &c_proj2, c_probe1);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(2));
  PROTECT(names=NEW_CHARACTER(2));
  IGRAPH_FINALLY(igraph_destroy, &c_proj1); 
  PROTECT(proj1=R_igraph_to_SEXP(&c_proj1));  
  igraph_destroy(&c_proj1); 
  IGRAPH_FINALLY_CLEAN(1);
  IGRAPH_FINALLY(igraph_destroy, &c_proj2); 
  PROTECT(proj2=R_igraph_to_SEXP(&c_proj2));  
  igraph_destroy(&c_proj2); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, proj1);
  SET_VECTOR_ELT(result, 1, proj2);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("proj1"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("proj2"));
  SET_NAMES(result, names);
  UNPROTECT(3);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_create_bipartite                    /
/-------------------------------------------*/
SEXP R_igraph_create_bipartite(SEXP types, SEXP edges, SEXP directed) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_bool_t c_types;
  igraph_vector_t c_edges;
  igraph_bool_t c_directed;
  SEXP graph;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_vector_bool(types, &c_types);
  R_SEXP_to_vector(edges, &c_edges);
  c_directed=LOGICAL(directed)[0];
                                        /* Call igraph */
  c_result=igraph_create_bipartite(&c_graph, &c_types, &c_edges, c_directed);

                                        /* Convert output */
  IGRAPH_FINALLY(igraph_destroy, &c_graph); 
  PROTECT(graph=R_igraph_to_SEXP(&c_graph));  
  igraph_destroy(&c_graph); 
  IGRAPH_FINALLY_CLEAN(1);
  result=graph;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_incidence                           /
/-------------------------------------------*/
SEXP R_igraph_incidence(SEXP incidence, SEXP directed, SEXP mode, SEXP multiple) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_bool_t c_types;
  igraph_matrix_t c_incidence;
  igraph_bool_t c_directed;
  igraph_integer_t c_mode;
  igraph_bool_t c_multiple;
  SEXP graph;
  SEXP types;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  if (0 != igraph_vector_bool_init(&c_types, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_bool_destroy, &c_types);
  R_SEXP_to_matrix(incidence, &c_incidence);
  c_directed=LOGICAL(directed)[0];
  c_mode=REAL(mode)[0];
  c_multiple=LOGICAL(multiple)[0];
                                        /* Call igraph */
  c_result=igraph_incidence(&c_graph, &c_types, &c_incidence, c_directed, c_mode, c_multiple);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(2));
  PROTECT(names=NEW_CHARACTER(2));
  IGRAPH_FINALLY(igraph_destroy, &c_graph); 
  PROTECT(graph=R_igraph_to_SEXP(&c_graph));  
  igraph_destroy(&c_graph); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(types=R_igraph_vector_bool_to_SEXP(&c_types)); 
  igraph_vector_bool_destroy(&c_types); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, graph);
  SET_VECTOR_ELT(result, 1, types);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("graph"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("types"));
  SET_NAMES(result, names);
  UNPROTECT(3);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_get_incidence                       /
/-------------------------------------------*/
SEXP R_igraph_get_incidence(SEXP graph, SEXP types) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_bool_t c_types;
  igraph_matrix_t c_res;
  igraph_vector_t c_row_ids;
  igraph_vector_t c_col_ids;
  SEXP res;
  SEXP row_ids;
  SEXP col_ids;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (!isNull(types)) { R_SEXP_to_vector_bool(types, &c_types); }
  if (0 != igraph_matrix_init(&c_res, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_res);
  if (0 != igraph_vector_init(&c_row_ids, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_row_ids); 
  row_ids=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_col_ids, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_col_ids); 
  col_ids=NEW_NUMERIC(0); /* hack to have a non-NULL value */
                                        /* Call igraph */
  c_result=igraph_get_incidence(&c_graph, (isNull(types) ? 0 : &c_types), &c_res, (isNull(row_ids) ? 0 : &c_row_ids), (isNull(col_ids) ? 0 : &c_col_ids));

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(res=R_igraph_matrix_to_SEXP(&c_res)); 
  igraph_matrix_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(row_ids=R_igraph_0orvector_to_SEXP(&c_row_ids)); 
  igraph_vector_destroy(&c_row_ids); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(col_ids=R_igraph_0orvector_to_SEXP(&c_col_ids)); 
  igraph_vector_destroy(&c_col_ids); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, res);
  SET_VECTOR_ELT(result, 1, row_ids);
  SET_VECTOR_ELT(result, 2, col_ids);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("res"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("row_ids"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("col_ids"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_is_bipartite                        /
/-------------------------------------------*/
SEXP R_igraph_is_bipartite(SEXP graph) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_bool_t c_res;
  igraph_vector_bool_t c_type;
  SEXP res;
  SEXP type;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_bool_init(&c_type, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_bool_destroy, &c_type); 
  type=NEW_NUMERIC(0); /* hack to have a non-NULL value */
                                        /* Call igraph */
  c_result=igraph_is_bipartite(&c_graph, &c_res, (isNull(type) ? 0 : &c_type));

                                        /* Convert output */
  PROTECT(result=NEW_LIST(2));
  PROTECT(names=NEW_CHARACTER(2));
  PROTECT(res=NEW_LOGICAL(1)); 
  LOGICAL(res)[0]=c_res;
  PROTECT(type=R_igraph_0orvector_bool_to_SEXP(&c_type)); 
  igraph_vector_bool_destroy(&c_type); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, res);
  SET_VECTOR_ELT(result, 1, type);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("res"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("type"));
  SET_NAMES(result, names);
  UNPROTECT(3);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_clusters                            /
/-------------------------------------------*/
SEXP R_igraph_clusters(SEXP graph, SEXP mode) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_membership;
  igraph_vector_t c_csize;
  igraph_integer_t c_no;
  igraph_connectedness_t c_mode;
  SEXP membership;
  SEXP csize;
  SEXP no;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_membership, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_membership);
  if (0 != igraph_vector_init(&c_csize, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_csize);
  c_mode=REAL(mode)[0];
                                        /* Call igraph */
  c_result=igraph_clusters(&c_graph, &c_membership, &c_csize, &c_no, c_mode);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(membership=R_igraph_vector_to_SEXP(&c_membership)); 
  igraph_vector_destroy(&c_membership); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(csize=R_igraph_vector_to_SEXP(&c_csize)); 
  igraph_vector_destroy(&c_csize); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(no=NEW_NUMERIC(1)); 
  REAL(no)[0]=c_no;
  SET_VECTOR_ELT(result, 0, membership);
  SET_VECTOR_ELT(result, 1, csize);
  SET_VECTOR_ELT(result, 2, no);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("membership"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("csize"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("no"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_articulation_points                 /
/-------------------------------------------*/
SEXP R_igraph_articulation_points(SEXP graph) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_res;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_res, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_res);
                                        /* Call igraph */
  c_result=igraph_articulation_points(&c_graph, &c_res);

                                        /* Convert output */
  PROTECT(res=R_igraph_vector_to_SEXP(&c_res)); 
  igraph_vector_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_biconnected_components              /
/-------------------------------------------*/
SEXP R_igraph_biconnected_components(SEXP graph) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_no;
  igraph_vector_ptr_t c_components;
  igraph_vector_t c_articulation_points;
  SEXP no;
  SEXP components;
  SEXP articulation_points;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_ptr_init(&c_components, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(R_igraph_vectorlist_destroy, &c_components);
  if (0 != igraph_vector_init(&c_articulation_points, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_articulation_points);
                                        /* Call igraph */
  c_result=igraph_biconnected_components(&c_graph, &c_no, &c_components, &c_articulation_points);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(no=NEW_NUMERIC(1)); 
  REAL(no)[0]=c_no;
  PROTECT(components=R_igraph_vectorlist_to_SEXP(&c_components)); 
  R_igraph_vectorlist_destroy(&c_components); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(articulation_points=R_igraph_vector_to_SEXP(&c_articulation_points)); 
  igraph_vector_destroy(&c_articulation_points); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, no);
  SET_VECTOR_ELT(result, 1, components);
  SET_VECTOR_ELT(result, 2, articulation_points);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("no"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("components"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("articulation_points"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_layout_star                         /
/-------------------------------------------*/
SEXP R_igraph_layout_star(SEXP graph, SEXP center, SEXP order) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_matrix_t c_res;
  igraph_integer_t c_center;
  igraph_vector_t c_order;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_matrix_init(&c_res, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_res);
  c_center=REAL(center)[0];
  if (!isNull(order)) { R_SEXP_to_vector(order, &c_order); }
                                        /* Call igraph */
  c_result=igraph_layout_star(&c_graph, &c_res, c_center, (isNull(order) ? 0 : &c_order));

                                        /* Convert output */
  PROTECT(res=R_igraph_matrix_to_SEXP(&c_res)); 
  igraph_matrix_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_layout_drl                          /
/-------------------------------------------*/
SEXP R_igraph_layout_drl(SEXP graph, SEXP res, SEXP use_seed, SEXP options, SEXP weights, SEXP fixed) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_matrix_t c_res;
  igraph_bool_t c_use_seed;
  igraph_layout_drl_options_t c_options;
  igraph_vector_t c_weights;
  igraph_vector_bool_t c_fixed;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != R_SEXP_to_igraph_matrix_copy(res, &c_res)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_res);
  c_use_seed=LOGICAL(use_seed)[0];
  R_SEXP_to_igraph_layout_drl_options(options, &c_options);
  if (!isNull(weights)) { R_SEXP_to_vector(weights, &c_weights); }
  if (!isNull(fixed)) { R_SEXP_to_vector_bool(fixed, &c_fixed); }
                                        /* Call igraph */
  c_result=igraph_layout_drl(&c_graph, &c_res, c_use_seed, &c_options, (isNull(weights) ? 0 : &c_weights), (isNull(fixed) ? 0 : &c_fixed));

                                        /* Convert output */
  PROTECT(res=R_igraph_matrix_to_SEXP(&c_res)); 
  igraph_matrix_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_layout_drl_3d                       /
/-------------------------------------------*/
SEXP R_igraph_layout_drl_3d(SEXP graph, SEXP res, SEXP use_seed, SEXP options, SEXP weights, SEXP fixed) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_matrix_t c_res;
  igraph_bool_t c_use_seed;
  igraph_layout_drl_options_t c_options;
  igraph_vector_t c_weights;
  igraph_vector_bool_t c_fixed;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != R_SEXP_to_igraph_matrix_copy(res, &c_res)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_res);
  c_use_seed=LOGICAL(use_seed)[0];
  R_SEXP_to_igraph_layout_drl_options(options, &c_options);
  if (!isNull(weights)) { R_SEXP_to_vector(weights, &c_weights); }
  if (!isNull(fixed)) { R_SEXP_to_vector_bool(fixed, &c_fixed); }
                                        /* Call igraph */
  c_result=igraph_layout_drl_3d(&c_graph, &c_res, c_use_seed, &c_options, (isNull(weights) ? 0 : &c_weights), (isNull(fixed) ? 0 : &c_fixed));

                                        /* Convert output */
  PROTECT(res=R_igraph_matrix_to_SEXP(&c_res)); 
  igraph_matrix_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_similarity_jaccard                  /
/-------------------------------------------*/
SEXP R_igraph_similarity_jaccard(SEXP graph, SEXP vids, SEXP mode, SEXP loops) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_matrix_t c_res;
  igraph_vs_t c_vids;
  igraph_integer_t c_mode;
  igraph_bool_t c_loops;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_matrix_init(&c_res, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_res);
  R_SEXP_to_igraph_vs(vids, &c_graph, &c_vids);
  c_mode=REAL(mode)[0];
  c_loops=LOGICAL(loops)[0];
                                        /* Call igraph */
  c_result=igraph_similarity_jaccard(&c_graph, &c_res, c_vids, c_mode, c_loops);

                                        /* Convert output */
  PROTECT(res=R_igraph_matrix_to_SEXP(&c_res)); 
  igraph_matrix_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  igraph_vs_destroy(&c_vids);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_similarity_dice                     /
/-------------------------------------------*/
SEXP R_igraph_similarity_dice(SEXP graph, SEXP vids, SEXP mode, SEXP loops) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_matrix_t c_res;
  igraph_vs_t c_vids;
  igraph_integer_t c_mode;
  igraph_bool_t c_loops;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_matrix_init(&c_res, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_res);
  R_SEXP_to_igraph_vs(vids, &c_graph, &c_vids);
  c_mode=REAL(mode)[0];
  c_loops=LOGICAL(loops)[0];
                                        /* Call igraph */
  c_result=igraph_similarity_dice(&c_graph, &c_res, c_vids, c_mode, c_loops);

                                        /* Convert output */
  PROTECT(res=R_igraph_matrix_to_SEXP(&c_res)); 
  igraph_matrix_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  igraph_vs_destroy(&c_vids);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_similarity_inverse_log_weighted     /
/-------------------------------------------*/
SEXP R_igraph_similarity_inverse_log_weighted(SEXP graph, SEXP vids, SEXP mode) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_matrix_t c_res;
  igraph_vs_t c_vids;
  igraph_integer_t c_mode;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_matrix_init(&c_res, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_res);
  R_SEXP_to_igraph_vs(vids, &c_graph, &c_vids);
  c_mode=REAL(mode)[0];
                                        /* Call igraph */
  c_result=igraph_similarity_inverse_log_weighted(&c_graph, &c_res, c_vids, c_mode);

                                        /* Convert output */
  PROTECT(res=R_igraph_matrix_to_SEXP(&c_res)); 
  igraph_matrix_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  igraph_vs_destroy(&c_vids);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_le_community_to_membership          /
/-------------------------------------------*/
SEXP R_igraph_le_community_to_membership(SEXP merges, SEXP steps, SEXP membership) {
                                        /* Declarations */
  igraph_matrix_t c_merges;
  igraph_integer_t c_steps;
  igraph_vector_t c_membership;
  igraph_vector_t c_csize;
  SEXP csize;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_matrix(merges, &c_merges);
  c_steps=REAL(steps)[0];
  if (0 != R_SEXP_to_vector_copy(membership, &c_membership)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_membership);
  if (0 != igraph_vector_init(&c_csize, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_csize); 
  csize=NEW_NUMERIC(0); /* hack to have a non-NULL value */
                                        /* Call igraph */
  c_result=igraph_le_community_to_membership(&c_merges, c_steps, &c_membership, (isNull(csize) ? 0 : &c_csize));

                                        /* Convert output */
  PROTECT(result=NEW_LIST(2));
  PROTECT(names=NEW_CHARACTER(2));
  PROTECT(membership=R_igraph_vector_to_SEXP(&c_membership)); 
  igraph_vector_destroy(&c_membership); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(csize=R_igraph_0orvector_to_SEXP(&c_csize)); 
  igraph_vector_destroy(&c_csize); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, membership);
  SET_VECTOR_ELT(result, 1, csize);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("membership"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("csize"));
  SET_NAMES(result, names);
  UNPROTECT(3);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_modularity                          /
/-------------------------------------------*/
SEXP R_igraph_modularity(SEXP graph, SEXP membership, SEXP weights) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_membership;
  igraph_real_t c_modularity;
  igraph_vector_t c_weights;
  SEXP modularity;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_vector(membership, &c_membership);
  if (!isNull(weights)) { R_SEXP_to_vector(weights, &c_weights); }
                                        /* Call igraph */
  c_result=igraph_modularity(&c_graph, &c_membership, &c_modularity, (isNull(weights) ? 0 : &c_weights));

                                        /* Convert output */
  PROTECT(modularity=NEW_NUMERIC(1)); 
  REAL(modularity)[0]=c_modularity;
  result=modularity;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_community_leading_eigenvector       /
/-------------------------------------------*/
SEXP R_igraph_community_leading_eigenvector(SEXP graph, SEXP steps, SEXP options) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_matrix_t c_merges;
  igraph_vector_t c_membership;
  igraph_integer_t c_steps;
  igraph_arpack_options_t c_options;
  SEXP merges;
  SEXP membership;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_matrix_init(&c_merges, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_merges);
  if (0 != igraph_vector_init(&c_membership, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_membership);
  c_steps=REAL(steps)[0];
  R_SEXP_to_igraph_arpack_options(options, &c_options);
                                        /* Call igraph */
  c_result=igraph_community_leading_eigenvector(&c_graph, &c_merges, &c_membership, c_steps, &c_options);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(merges=R_igraph_matrix_to_SEXP(&c_merges)); 
  igraph_matrix_destroy(&c_merges); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(membership=R_igraph_vector_to_SEXP(&c_membership)); 
  igraph_vector_destroy(&c_membership); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(options=R_igraph_arpack_options_to_SEXP(&c_options));
  SET_VECTOR_ELT(result, 0, merges);
  SET_VECTOR_ELT(result, 1, membership);
  SET_VECTOR_ELT(result, 2, options);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("merges"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("membership"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("options"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_community_leading_eigenvector_naive /
/-------------------------------------------*/
SEXP R_igraph_community_leading_eigenvector_naive(SEXP graph, SEXP steps, SEXP options) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_matrix_t c_merges;
  igraph_vector_t c_membership;
  igraph_integer_t c_steps;
  igraph_arpack_options_t c_options;
  SEXP merges;
  SEXP membership;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_matrix_init(&c_merges, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_merges);
  if (0 != igraph_vector_init(&c_membership, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_membership);
  c_steps=REAL(steps)[0];
  R_SEXP_to_igraph_arpack_options(options, &c_options);
                                        /* Call igraph */
  c_result=igraph_community_leading_eigenvector_naive(&c_graph, &c_merges, &c_membership, c_steps, &c_options);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(merges=R_igraph_matrix_to_SEXP(&c_merges)); 
  igraph_matrix_destroy(&c_merges); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(membership=R_igraph_vector_to_SEXP(&c_membership)); 
  igraph_vector_destroy(&c_membership); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(options=R_igraph_arpack_options_to_SEXP(&c_options));
  SET_VECTOR_ELT(result, 0, merges);
  SET_VECTOR_ELT(result, 1, membership);
  SET_VECTOR_ELT(result, 2, options);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("merges"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("membership"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("options"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_community_leading_eigenvector_step  /
/-------------------------------------------*/
SEXP R_igraph_community_leading_eigenvector_step(SEXP graph, SEXP membership, SEXP community, SEXP options) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_membership;
  igraph_integer_t c_community;
  igraph_bool_t c_split;
  igraph_vector_t c_eigenvector;
  igraph_real_t c_eigenvalue;
  igraph_arpack_options_t c_options;

  SEXP split;
  SEXP eigenvector;
  SEXP eigenvalue;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != R_SEXP_to_vector_copy(membership, &c_membership)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_membership);
  c_community=REAL(community)[0];
  if (0 != igraph_vector_init(&c_eigenvector, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_eigenvector); 
  eigenvector=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  R_SEXP_to_igraph_arpack_options(options, &c_options);
                                        /* Call igraph */
  c_result=igraph_community_leading_eigenvector_step(&c_graph, &c_membership, c_community, &c_split, (isNull(eigenvector) ? 0 : &c_eigenvector), &c_eigenvalue, &c_options, 0);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(5));
  PROTECT(names=NEW_CHARACTER(5));
  PROTECT(membership=R_igraph_vector_to_SEXP(&c_membership)); 
  igraph_vector_destroy(&c_membership); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(split=NEW_LOGICAL(1)); 
  LOGICAL(split)[0]=c_split;
  PROTECT(eigenvector=R_igraph_0orvector_to_SEXP(&c_eigenvector)); 
  igraph_vector_destroy(&c_eigenvector); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(eigenvalue=NEW_NUMERIC(1)); 
  REAL(eigenvalue)[0]=c_eigenvalue;
  PROTECT(options=R_igraph_arpack_options_to_SEXP(&c_options));
  SET_VECTOR_ELT(result, 0, membership);
  SET_VECTOR_ELT(result, 1, split);
  SET_VECTOR_ELT(result, 2, eigenvector);
  SET_VECTOR_ELT(result, 3, eigenvalue);
  SET_VECTOR_ELT(result, 4, options);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("membership"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("split"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("eigenvector"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("eigenvalue"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("options"));
  SET_NAMES(result, names);
  UNPROTECT(6);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_community_label_propagation         /
/-------------------------------------------*/
SEXP R_igraph_community_label_propagation(SEXP graph, SEXP weights, SEXP initial, SEXP fixed) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_membership;
  igraph_vector_t c_weights;
  igraph_vector_t c_initial;
  igraph_vector_bool_t c_fixed;
  SEXP membership;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_membership, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_membership);
  if (!isNull(weights)) { R_SEXP_to_vector(weights, &c_weights); }
  if (!isNull(initial)) { R_SEXP_to_vector(initial, &c_initial); }
  if (!isNull(fixed)) { R_SEXP_to_vector_bool(fixed, &c_fixed); }
                                        /* Call igraph */
  c_result=igraph_community_label_propagation(&c_graph, &c_membership, (isNull(weights) ? 0 : &c_weights), (isNull(initial) ? 0 : &c_initial), (isNull(fixed) ? 0 : &c_fixed));

                                        /* Convert output */
  PROTECT(membership=R_igraph_vector_to_SEXP(&c_membership)); 
  igraph_vector_destroy(&c_membership); 
  IGRAPH_FINALLY_CLEAN(1);
  result=membership;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_dyad_census                         /
/-------------------------------------------*/
SEXP R_igraph_dyad_census(SEXP graph) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_mut;
  igraph_integer_t c_asym;
  igraph_integer_t c_null;
  SEXP mut;
  SEXP asym;
  SEXP null;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
                                        /* Call igraph */
  c_result=igraph_dyad_census(&c_graph, &c_mut, &c_asym, &c_null);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(mut=NEW_NUMERIC(1)); 
  REAL(mut)[0]=c_mut;
  PROTECT(asym=NEW_NUMERIC(1)); 
  REAL(asym)[0]=c_asym;
  PROTECT(null=NEW_NUMERIC(1)); 
  REAL(null)[0]=c_null;
  SET_VECTOR_ELT(result, 0, mut);
  SET_VECTOR_ELT(result, 1, asym);
  SET_VECTOR_ELT(result, 2, null);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("mut"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("asym"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("null"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_triad_census                        /
/-------------------------------------------*/
SEXP R_igraph_triad_census(SEXP graph) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_res;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_res, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_res);
                                        /* Call igraph */
  c_result=igraph_triad_census(&c_graph, &c_res);

                                        /* Convert output */
  PROTECT(res=R_igraph_vector_to_SEXP(&c_res)); 
  igraph_vector_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_isoclass                            /
/-------------------------------------------*/
SEXP R_igraph_isoclass(SEXP graph) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_isoclass;
  SEXP isoclass;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
                                        /* Call igraph */
  c_result=igraph_isoclass(&c_graph, &c_isoclass);

                                        /* Convert output */
  PROTECT(isoclass=NEW_NUMERIC(1)); 
  REAL(isoclass)[0]=c_isoclass;
  result=isoclass;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_isomorphic                          /
/-------------------------------------------*/
SEXP R_igraph_isomorphic(SEXP graph1, SEXP graph2) {
                                        /* Declarations */
  igraph_t c_graph1;
  igraph_t c_graph2;
  igraph_bool_t c_iso;
  SEXP iso;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph1, &c_graph1);
  R_SEXP_to_igraph(graph2, &c_graph2);
                                        /* Call igraph */
  c_result=igraph_isomorphic(&c_graph1, &c_graph2, &c_iso);

                                        /* Convert output */
  PROTECT(iso=NEW_LOGICAL(1)); 
  LOGICAL(iso)[0]=c_iso;
  result=iso;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_isoclass_subgraph                   /
/-------------------------------------------*/
SEXP R_igraph_isoclass_subgraph(SEXP graph, SEXP vids) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_vids;
  igraph_integer_t c_isoclass;
  SEXP isoclass;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_vector(vids, &c_vids);
                                        /* Call igraph */
  c_result=igraph_isoclass_subgraph(&c_graph, &c_vids, &c_isoclass);

                                        /* Convert output */
  PROTECT(isoclass=NEW_NUMERIC(1)); 
  REAL(isoclass)[0]=c_isoclass;
  result=isoclass;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_isoclass_create                     /
/-------------------------------------------*/
SEXP R_igraph_isoclass_create(SEXP size, SEXP number, SEXP directed) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_size;
  igraph_integer_t c_number;
  igraph_bool_t c_directed;
  SEXP graph;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  c_size=REAL(size)[0];
  c_number=REAL(number)[0];
  c_directed=LOGICAL(directed)[0];
                                        /* Call igraph */
  c_result=igraph_isoclass_create(&c_graph, c_size, c_number, c_directed);

                                        /* Convert output */
  IGRAPH_FINALLY(igraph_destroy, &c_graph); 
  PROTECT(graph=R_igraph_to_SEXP(&c_graph));  
  igraph_destroy(&c_graph); 
  IGRAPH_FINALLY_CLEAN(1);
  result=graph;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_isomorphic_vf2                      /
/-------------------------------------------*/
SEXP R_igraph_isomorphic_vf2(SEXP graph1, SEXP graph2) {
                                        /* Declarations */
  igraph_t c_graph1;
  igraph_t c_graph2;
  igraph_bool_t c_iso;
  igraph_vector_t c_map12;
  igraph_vector_t c_map21;
  SEXP iso;
  SEXP map12;
  SEXP map21;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph1, &c_graph1);
  R_SEXP_to_igraph(graph2, &c_graph2);
  if (0 != igraph_vector_init(&c_map12, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_map12); 
  map12=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_map21, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_map21); 
  map21=NEW_NUMERIC(0); /* hack to have a non-NULL value */
                                        /* Call igraph */
  c_result=igraph_isomorphic_vf2(&c_graph1, &c_graph2, &c_iso, (isNull(map12) ? 0 : &c_map12), (isNull(map21) ? 0 : &c_map21));

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(iso=NEW_LOGICAL(1)); 
  LOGICAL(iso)[0]=c_iso;
  PROTECT(map12=R_igraph_0orvector_to_SEXP(&c_map12)); 
  igraph_vector_destroy(&c_map12); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(map21=R_igraph_0orvector_to_SEXP(&c_map21)); 
  igraph_vector_destroy(&c_map21); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, iso);
  SET_VECTOR_ELT(result, 1, map12);
  SET_VECTOR_ELT(result, 2, map21);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("iso"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("map12"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("map21"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_count_isomorphisms_vf2              /
/-------------------------------------------*/
SEXP R_igraph_count_isomorphisms_vf2(SEXP graph1, SEXP graph2) {
                                        /* Declarations */
  igraph_t c_graph1;
  igraph_t c_graph2;
  igraph_integer_t c_count;
  SEXP count;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph1, &c_graph1);
  R_SEXP_to_igraph(graph2, &c_graph2);
                                        /* Call igraph */
  c_result=igraph_count_isomorphisms_vf2(&c_graph1, &c_graph2, &c_count);

                                        /* Convert output */
  PROTECT(count=NEW_NUMERIC(1)); 
  REAL(count)[0]=c_count;
  result=count;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_get_isomorphisms_vf2                /
/-------------------------------------------*/
SEXP R_igraph_get_isomorphisms_vf2(SEXP graph1, SEXP graph2) {
                                        /* Declarations */
  igraph_t c_graph1;
  igraph_t c_graph2;
  igraph_vector_ptr_t c_maps;
  SEXP maps;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph1, &c_graph1);
  R_SEXP_to_igraph(graph2, &c_graph2);
  if (0 != igraph_vector_ptr_init(&c_maps, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(R_igraph_vectorlist_destroy, &c_maps);
                                        /* Call igraph */
  c_result=igraph_get_isomorphisms_vf2(&c_graph1, &c_graph2, &c_maps);

                                        /* Convert output */
  PROTECT(maps=R_igraph_vectorlist_to_SEXP(&c_maps)); 
  R_igraph_vectorlist_destroy(&c_maps); 
  IGRAPH_FINALLY_CLEAN(1);
  result=maps;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_subisomorphic_vf2                   /
/-------------------------------------------*/
SEXP R_igraph_subisomorphic_vf2(SEXP graph1, SEXP graph2) {
                                        /* Declarations */
  igraph_t c_graph1;
  igraph_t c_graph2;
  igraph_bool_t c_iso;
  igraph_vector_t c_map12;
  igraph_vector_t c_map21;
  SEXP iso;
  SEXP map12;
  SEXP map21;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph1, &c_graph1);
  R_SEXP_to_igraph(graph2, &c_graph2);
  if (0 != igraph_vector_init(&c_map12, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_map12); 
  map12=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_map21, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_map21); 
  map21=NEW_NUMERIC(0); /* hack to have a non-NULL value */
                                        /* Call igraph */
  c_result=igraph_subisomorphic_vf2(&c_graph1, &c_graph2, &c_iso, (isNull(map12) ? 0 : &c_map12), (isNull(map21) ? 0 : &c_map21));

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(iso=NEW_LOGICAL(1)); 
  LOGICAL(iso)[0]=c_iso;
  PROTECT(map12=R_igraph_0orvector_to_SEXP(&c_map12)); 
  igraph_vector_destroy(&c_map12); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(map21=R_igraph_0orvector_to_SEXP(&c_map21)); 
  igraph_vector_destroy(&c_map21); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, iso);
  SET_VECTOR_ELT(result, 1, map12);
  SET_VECTOR_ELT(result, 2, map21);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("iso"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("map12"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("map21"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_count_subisomorphisms_vf2           /
/-------------------------------------------*/
SEXP R_igraph_count_subisomorphisms_vf2(SEXP graph1, SEXP graph2) {
                                        /* Declarations */
  igraph_t c_graph1;
  igraph_t c_graph2;
  igraph_integer_t c_count;
  SEXP count;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph1, &c_graph1);
  R_SEXP_to_igraph(graph2, &c_graph2);
                                        /* Call igraph */
  c_result=igraph_count_subisomorphisms_vf2(&c_graph1, &c_graph2, &c_count);

                                        /* Convert output */
  PROTECT(count=NEW_NUMERIC(1)); 
  REAL(count)[0]=c_count;
  result=count;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_get_subisomorphisms_vf2             /
/-------------------------------------------*/
SEXP R_igraph_get_subisomorphisms_vf2(SEXP graph1, SEXP graph2) {
                                        /* Declarations */
  igraph_t c_graph1;
  igraph_t c_graph2;
  igraph_vector_ptr_t c_maps;
  SEXP maps;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph1, &c_graph1);
  R_SEXP_to_igraph(graph2, &c_graph2);
  if (0 != igraph_vector_ptr_init(&c_maps, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(R_igraph_vectorlist_destroy, &c_maps);
                                        /* Call igraph */
  c_result=igraph_get_subisomorphisms_vf2(&c_graph1, &c_graph2, &c_maps);

                                        /* Convert output */
  PROTECT(maps=R_igraph_vectorlist_to_SEXP(&c_maps)); 
  R_igraph_vectorlist_destroy(&c_maps); 
  IGRAPH_FINALLY_CLEAN(1);
  result=maps;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_isomorphic_34                       /
/-------------------------------------------*/
SEXP R_igraph_isomorphic_34(SEXP graph1, SEXP graph2) {
                                        /* Declarations */
  igraph_t c_graph1;
  igraph_t c_graph2;
  igraph_bool_t c_iso;
  SEXP iso;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph1, &c_graph1);
  R_SEXP_to_igraph(graph2, &c_graph2);
                                        /* Call igraph */
  c_result=igraph_isomorphic_34(&c_graph1, &c_graph2, &c_iso);

                                        /* Convert output */
  PROTECT(iso=NEW_LOGICAL(1)); 
  LOGICAL(iso)[0]=c_iso;
  result=iso;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_canonical_permutation               /
/-------------------------------------------*/
SEXP R_igraph_canonical_permutation(SEXP graph, SEXP sh) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_labeling;
  igraph_bliss_sh_t c_sh;
  igraph_bliss_info_t c_info;
  SEXP labeling;
  SEXP info;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  if (0 != igraph_vector_init(&c_labeling, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_labeling);
  c_sh=REAL(sh)[0];
                                        /* Call igraph */
  c_result=igraph_canonical_permutation(&c_graph, &c_labeling, c_sh, &c_info);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(2));
  PROTECT(names=NEW_CHARACTER(2));
  PROTECT(labeling=R_igraph_vector_to_SEXP(&c_labeling)); 
  igraph_vector_destroy(&c_labeling); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(info=R_igraph_bliss_info_to_SEXP(&c_info)); 
  if (c_info.group_size) { free(c_info.group_size); }
  SET_VECTOR_ELT(result, 0, labeling);
  SET_VECTOR_ELT(result, 1, info);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("labeling"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("info"));
  SET_NAMES(result, names);
  UNPROTECT(3);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_permute_vertices                    /
/-------------------------------------------*/
SEXP R_igraph_permute_vertices(SEXP graph, SEXP permutation) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_t c_res;
  igraph_vector_t c_permutation;
  SEXP res;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_vector(permutation, &c_permutation);
                                        /* Call igraph */
  c_result=igraph_permute_vertices(&c_graph, &c_res, &c_permutation);

                                        /* Convert output */
  IGRAPH_FINALLY(igraph_destroy, &c_res); 
  PROTECT(res=R_igraph_to_SEXP(&c_res));  
  igraph_destroy(&c_res); 
  IGRAPH_FINALLY_CLEAN(1);
  result=res;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_isomorphic_bliss                    /
/-------------------------------------------*/
SEXP R_igraph_isomorphic_bliss(SEXP graph1, SEXP graph2, SEXP sh1, SEXP sh2) {
                                        /* Declarations */
  igraph_t c_graph1;
  igraph_t c_graph2;
  igraph_bool_t c_iso;
  igraph_vector_t c_map12;
  igraph_vector_t c_map21;
  igraph_bliss_sh_t c_sh1;
  igraph_bliss_sh_t c_sh2;
  igraph_bliss_info_t c_info1;
  igraph_bliss_info_t c_info2;
  SEXP iso;
  SEXP map12;
  SEXP map21;
  SEXP info1;
  SEXP info2;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph1, &c_graph1);
  R_SEXP_to_igraph(graph2, &c_graph2);
  if (0 != igraph_vector_init(&c_map12, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_map12); 
  map12=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_map21, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_map21); 
  map21=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  c_sh1=REAL(sh1)[0];
  c_sh2=REAL(sh2)[0];
                                        /* Call igraph */
  c_result=igraph_isomorphic_bliss(&c_graph1, &c_graph2, &c_iso, (isNull(map12) ? 0 : &c_map12), (isNull(map21) ? 0 : &c_map21), c_sh1, c_sh2, &c_info1, &c_info2);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(5));
  PROTECT(names=NEW_CHARACTER(5));
  PROTECT(iso=NEW_LOGICAL(1)); 
  LOGICAL(iso)[0]=c_iso;
  PROTECT(map12=R_igraph_0orvector_to_SEXP(&c_map12)); 
  igraph_vector_destroy(&c_map12); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(map21=R_igraph_0orvector_to_SEXP(&c_map21)); 
  igraph_vector_destroy(&c_map21); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(info1=R_igraph_bliss_info_to_SEXP(&c_info1)); 
  if (c_info1.group_size) { free(c_info1.group_size); }
  PROTECT(info2=R_igraph_bliss_info_to_SEXP(&c_info2)); 
  if (c_info2.group_size) { free(c_info2.group_size); }
  SET_VECTOR_ELT(result, 0, iso);
  SET_VECTOR_ELT(result, 1, map12);
  SET_VECTOR_ELT(result, 2, map21);
  SET_VECTOR_ELT(result, 3, info1);
  SET_VECTOR_ELT(result, 4, info2);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("iso"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("map12"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("map21"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("info1"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("info2"));
  SET_NAMES(result, names);
  UNPROTECT(6);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_automorphisms                       /
/-------------------------------------------*/
SEXP R_igraph_automorphisms(SEXP graph, SEXP sh) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_bliss_sh_t c_sh;
  igraph_bliss_info_t c_info;
  SEXP info;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_sh=REAL(sh)[0];
                                        /* Call igraph */
  c_result=igraph_automorphisms(&c_graph, c_sh, &c_info);

                                        /* Convert output */
  PROTECT(info=R_igraph_bliss_info_to_SEXP(&c_info)); 
  if (c_info.group_size) { free(c_info.group_size); }
  result=info;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_d                       /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_d(SEXP graph, SEXP niter, SEXP delta, SEXP filter, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_niter;
  igraph_vector_t c_kernel;
  igraph_vector_t c_cites;
  igraph_real_t c_delta;
  igraph_vector_t c_filter;
  igraph_real_t c_logprob;
  igraph_real_t c_logmax;
  SEXP kernel;
  SEXP cites;
  SEXP logprob;
  SEXP logmax;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_niter=REAL(niter)[0];
  if (0 != igraph_vector_init(&c_kernel, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_kernel);
  if (0 != igraph_vector_init(&c_cites, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_cites); 
  cites=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  c_delta=REAL(delta)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_d(&c_graph, c_niter, &c_kernel, (isNull(cites) ? 0 : &c_cites), c_delta, (isNull(filter) ? 0 : &c_filter), &c_logprob, &c_logmax);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(4));
  PROTECT(names=NEW_CHARACTER(4));
  PROTECT(kernel=R_igraph_vector_to_SEXP(&c_kernel)); 
  igraph_vector_destroy(&c_kernel); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(cites=R_igraph_0orvector_to_SEXP(&c_cites)); 
  igraph_vector_destroy(&c_cites); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logprob=NEW_NUMERIC(1)); 
  REAL(logprob)[0]=c_logprob;
  PROTECT(logmax=NEW_NUMERIC(1)); 
  REAL(logmax)[0]=c_logmax;
  SET_VECTOR_ELT(result, 0, kernel);
  SET_VECTOR_ELT(result, 1, cites);
  SET_VECTOR_ELT(result, 2, logprob);
  SET_VECTOR_ELT(result, 3, logmax);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("logprob"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("logmax"));
  SET_NAMES(result, names);
  UNPROTECT(5);

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_probs_d                    /
/-------------------------------------------*/
SEXP R_igraph_revolver_probs_d(SEXP graph, SEXP kernel, SEXP ntk) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_kernel;
  igraph_vector_t c_probs;
  igraph_vector_t c_citedprobs;
  igraph_vector_t c_citingprobs;
  igraph_bool_t c_ntk;
  SEXP probs;
  SEXP citedprobs;
  SEXP citingprobs;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_vector(kernel, &c_kernel);
  if (0 != igraph_vector_init(&c_probs, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_probs); 
  probs=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_citedprobs, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_citedprobs); 
  citedprobs=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_citingprobs, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_citingprobs); 
  citingprobs=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  c_ntk=LOGICAL(ntk)[0];
                                        /* Call igraph */
  c_result=igraph_revolver_probs_d(&c_graph, &c_kernel, (isNull(probs) ? 0 : &c_probs), (isNull(citedprobs) ? 0 : &c_citedprobs), (isNull(citingprobs) ? 0 : &c_citingprobs), c_ntk);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(probs=R_igraph_0orvector_to_SEXP(&c_probs)); 
  igraph_vector_destroy(&c_probs); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(citedprobs=R_igraph_0orvector_to_SEXP(&c_citedprobs)); 
  igraph_vector_destroy(&c_citedprobs); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(citingprobs=R_igraph_0orvector_to_SEXP(&c_citingprobs)); 
  igraph_vector_destroy(&c_citingprobs); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, probs);
  SET_VECTOR_ELT(result, 1, citedprobs);
  SET_VECTOR_ELT(result, 2, citingprobs);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("probs"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("citedprobs"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("citingprobs"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_de                      /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_de(SEXP graph, SEXP niter, SEXP cats, SEXP delta, SEXP filter, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_niter;
  igraph_matrix_t c_kernel;
  igraph_vector_t c_cats;
  igraph_matrix_t c_cites;
  igraph_real_t c_delta;
  igraph_vector_t c_filter;
  igraph_real_t c_logprob;
  igraph_real_t c_logmax;
  SEXP kernel;
  SEXP cites;
  SEXP logprob;
  SEXP logmax;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_niter=REAL(niter)[0];
  if (0 != igraph_matrix_init(&c_kernel, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_kernel);
  R_SEXP_to_vector(cats, &c_cats);
  if (0 != igraph_matrix_init(&c_cites, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_cites); 
  cites=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  c_delta=REAL(delta)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_de(&c_graph, c_niter, &c_kernel, &c_cats, (isNull(cites) ? 0 : &c_cites), c_delta, (isNull(filter) ? 0 : &c_filter), &c_logprob, &c_logmax);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(4));
  PROTECT(names=NEW_CHARACTER(4));
  PROTECT(kernel=R_igraph_matrix_to_SEXP(&c_kernel)); 
  igraph_matrix_destroy(&c_kernel); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(cites=R_igraph_0ormatrix_to_SEXP(&c_cites)); 
  igraph_matrix_destroy(&c_cites); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logprob=NEW_NUMERIC(1)); 
  REAL(logprob)[0]=c_logprob;
  PROTECT(logmax=NEW_NUMERIC(1)); 
  REAL(logmax)[0]=c_logmax;
  SET_VECTOR_ELT(result, 0, kernel);
  SET_VECTOR_ELT(result, 1, cites);
  SET_VECTOR_ELT(result, 2, logprob);
  SET_VECTOR_ELT(result, 3, logmax);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("logprob"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("logmax"));
  SET_NAMES(result, names);
  UNPROTECT(5);

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_probs_de                   /
/-------------------------------------------*/
SEXP R_igraph_revolver_probs_de(SEXP graph, SEXP kernel, SEXP cats) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_matrix_t c_kernel;
  igraph_vector_t c_cats;
  igraph_vector_t c_logprobs;
  igraph_vector_t c_logcited;
  igraph_vector_t c_logciting;
  SEXP logprobs;
  SEXP logcited;
  SEXP logciting;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_matrix(kernel, &c_kernel);
  R_SEXP_to_vector(cats, &c_cats);
  if (0 != igraph_vector_init(&c_logprobs, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_logprobs); 
  logprobs=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_logcited, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_logcited); 
  logcited=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_logciting, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_logciting); 
  logciting=NEW_NUMERIC(0); /* hack to have a non-NULL value */
                                        /* Call igraph */
  c_result=igraph_revolver_probs_de(&c_graph, &c_kernel, &c_cats, (isNull(logprobs) ? 0 : &c_logprobs), (isNull(logcited) ? 0 : &c_logcited), (isNull(logciting) ? 0 : &c_logciting));

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(logprobs=R_igraph_0orvector_to_SEXP(&c_logprobs)); 
  igraph_vector_destroy(&c_logprobs); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logcited=R_igraph_0orvector_to_SEXP(&c_logcited)); 
  igraph_vector_destroy(&c_logcited); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logciting=R_igraph_0orvector_to_SEXP(&c_logciting)); 
  igraph_vector_destroy(&c_logciting); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, logprobs);
  SET_VECTOR_ELT(result, 1, logcited);
  SET_VECTOR_ELT(result, 2, logciting);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("logprobs"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("logcited"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("logciting"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_ade                     /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_ade(SEXP graph, SEXP niter, SEXP cats, SEXP agebins, SEXP delta, SEXP filter, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_niter;
  igraph_array3_t c_kernel;
  igraph_vector_t c_cats;
  igraph_array3_t c_cites;
  igraph_integer_t c_agebins;
  igraph_real_t c_delta;
  igraph_vector_t c_filter;
  igraph_real_t c_logprob;
  igraph_real_t c_logmax;
  SEXP kernel;
  SEXP cites;
  SEXP logprob;
  SEXP logmax;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_niter=REAL(niter)[0];
  if (0 != igraph_array3_init(&c_kernel, 0, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_array3_destroy, &c_kernel);
  R_SEXP_to_vector(cats, &c_cats);
  if (0 != igraph_array3_init(&c_cites, 0, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_array3_destroy, &c_cites); 
  cites=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  c_agebins=REAL(agebins)[0];
  c_delta=REAL(delta)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_ade(&c_graph, c_niter, &c_kernel, &c_cats, (isNull(cites) ? 0 : &c_cites), c_agebins, c_delta, (isNull(filter) ? 0 : &c_filter), &c_logprob, &c_logmax);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(4));
  PROTECT(names=NEW_CHARACTER(4));
  PROTECT(kernel=R_igraph_array3_to_SEXP(&c_kernel)); 
  igraph_array3_destroy(&c_kernel); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(cites=R_igraph_0orarray3_to_SEXP(&c_cites)); 
  igraph_array3_destroy(&c_cites); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logprob=NEW_NUMERIC(1)); 
  REAL(logprob)[0]=c_logprob;
  PROTECT(logmax=NEW_NUMERIC(1)); 
  REAL(logmax)[0]=c_logmax;
  SET_VECTOR_ELT(result, 0, kernel);
  SET_VECTOR_ELT(result, 1, cites);
  SET_VECTOR_ELT(result, 2, logprob);
  SET_VECTOR_ELT(result, 3, logmax);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("logprob"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("logmax"));
  SET_NAMES(result, names);
  UNPROTECT(5);

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_probs_ade                  /
/-------------------------------------------*/
SEXP R_igraph_revolver_probs_ade(SEXP graph, SEXP kernel, SEXP cats) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_array3_t c_kernel;
  igraph_vector_t c_cats;
  igraph_vector_t c_logprobs;
  igraph_vector_t c_logcited;
  igraph_vector_t c_logciting;
  SEXP logprobs;
  SEXP logcited;
  SEXP logciting;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_igraph_SEXP_to_array3(kernel, &c_kernel);
  R_SEXP_to_vector(cats, &c_cats);
  if (0 != igraph_vector_init(&c_logprobs, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_logprobs); 
  logprobs=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_logcited, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_logcited); 
  logcited=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_logciting, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_logciting); 
  logciting=NEW_NUMERIC(0); /* hack to have a non-NULL value */
                                        /* Call igraph */
  c_result=igraph_revolver_probs_ade(&c_graph, &c_kernel, &c_cats, (isNull(logprobs) ? 0 : &c_logprobs), (isNull(logcited) ? 0 : &c_logcited), (isNull(logciting) ? 0 : &c_logciting));

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(logprobs=R_igraph_0orvector_to_SEXP(&c_logprobs)); 
  igraph_vector_destroy(&c_logprobs); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logcited=R_igraph_0orvector_to_SEXP(&c_logcited)); 
  igraph_vector_destroy(&c_logcited); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logciting=R_igraph_0orvector_to_SEXP(&c_logciting)); 
  igraph_vector_destroy(&c_logciting); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, logprobs);
  SET_VECTOR_ELT(result, 1, logcited);
  SET_VECTOR_ELT(result, 2, logciting);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("logprobs"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("logcited"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("logciting"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_f                       /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_f(SEXP graph, SEXP niter, SEXP delta, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_niter;
  igraph_vector_t c_kernel;
  igraph_vector_t c_cites;
  igraph_real_t c_delta;
  igraph_real_t c_logprob;
  igraph_real_t c_logmax;
  SEXP kernel;
  SEXP cites;
  SEXP logprob;
  SEXP logmax;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_niter=REAL(niter)[0];
  if (0 != igraph_vector_init(&c_kernel, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_kernel);
  if (0 != igraph_vector_init(&c_cites, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_cites); 
  cites=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  c_delta=REAL(delta)[0];
                                        /* Call igraph */
  c_result=igraph_revolver_ml_f(&c_graph, c_niter, &c_kernel, (isNull(cites) ? 0 : &c_cites), c_delta, &c_logprob, &c_logmax);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(4));
  PROTECT(names=NEW_CHARACTER(4));
  PROTECT(kernel=R_igraph_vector_to_SEXP(&c_kernel)); 
  igraph_vector_destroy(&c_kernel); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(cites=R_igraph_0orvector_to_SEXP(&c_cites)); 
  igraph_vector_destroy(&c_cites); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logprob=NEW_NUMERIC(1)); 
  REAL(logprob)[0]=c_logprob;
  PROTECT(logmax=NEW_NUMERIC(1)); 
  REAL(logmax)[0]=c_logmax;
  SET_VECTOR_ELT(result, 0, kernel);
  SET_VECTOR_ELT(result, 1, cites);
  SET_VECTOR_ELT(result, 2, logprob);
  SET_VECTOR_ELT(result, 3, logmax);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("logprob"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("logmax"));
  SET_NAMES(result, names);
  UNPROTECT(5);

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_df                      /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_df(SEXP graph, SEXP niter, SEXP delta, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_niter;
  igraph_matrix_t c_kernel;
  igraph_matrix_t c_cites;
  igraph_real_t c_delta;
  igraph_real_t c_logprob;
  igraph_real_t c_logmax;
  SEXP kernel;
  SEXP cites;
  SEXP logprob;
  SEXP logmax;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_niter=REAL(niter)[0];
  if (0 != igraph_matrix_init(&c_kernel, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_kernel);
  if (0 != igraph_matrix_init(&c_cites, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_cites); 
  cites=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  c_delta=REAL(delta)[0];
                                        /* Call igraph */
  c_result=igraph_revolver_ml_df(&c_graph, c_niter, &c_kernel, (isNull(cites) ? 0 : &c_cites), c_delta, &c_logprob, &c_logmax);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(4));
  PROTECT(names=NEW_CHARACTER(4));
  PROTECT(kernel=R_igraph_matrix_to_SEXP(&c_kernel)); 
  igraph_matrix_destroy(&c_kernel); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(cites=R_igraph_0ormatrix_to_SEXP(&c_cites)); 
  igraph_matrix_destroy(&c_cites); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logprob=NEW_NUMERIC(1)); 
  REAL(logprob)[0]=c_logprob;
  PROTECT(logmax=NEW_NUMERIC(1)); 
  REAL(logmax)[0]=c_logmax;
  SET_VECTOR_ELT(result, 0, kernel);
  SET_VECTOR_ELT(result, 1, cites);
  SET_VECTOR_ELT(result, 2, logprob);
  SET_VECTOR_ELT(result, 3, logmax);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("logprob"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("logmax"));
  SET_NAMES(result, names);
  UNPROTECT(5);

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_l                       /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_l(SEXP graph, SEXP niter, SEXP agebins, SEXP delta) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_niter;
  igraph_vector_t c_kernel;
  igraph_vector_t c_cites;
  igraph_integer_t c_agebins;
  igraph_real_t c_delta;
  igraph_real_t c_logprob;
  igraph_real_t c_logmax;
  SEXP kernel;
  SEXP cites;
  SEXP logprob;
  SEXP logmax;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_niter=REAL(niter)[0];
  if (0 != igraph_vector_init(&c_kernel, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_kernel);
  if (0 != igraph_vector_init(&c_cites, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_cites); 
  cites=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  c_agebins=REAL(agebins)[0];
  c_delta=REAL(delta)[0];
                                        /* Call igraph */
  c_result=igraph_revolver_ml_l(&c_graph, c_niter, &c_kernel, (isNull(cites) ? 0 : &c_cites), c_agebins, c_delta, &c_logprob, &c_logmax);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(4));
  PROTECT(names=NEW_CHARACTER(4));
  PROTECT(kernel=R_igraph_vector_to_SEXP(&c_kernel)); 
  igraph_vector_destroy(&c_kernel); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(cites=R_igraph_0orvector_to_SEXP(&c_cites)); 
  igraph_vector_destroy(&c_cites); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logprob=NEW_NUMERIC(1)); 
  REAL(logprob)[0]=c_logprob;
  PROTECT(logmax=NEW_NUMERIC(1)); 
  REAL(logmax)[0]=c_logmax;
  SET_VECTOR_ELT(result, 0, kernel);
  SET_VECTOR_ELT(result, 1, cites);
  SET_VECTOR_ELT(result, 2, logprob);
  SET_VECTOR_ELT(result, 3, logmax);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("logprob"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("logmax"));
  SET_NAMES(result, names);
  UNPROTECT(5);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_ad                      /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_ad(SEXP graph, SEXP niter, SEXP agebins, SEXP delta, SEXP filter, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_integer_t c_niter;
  igraph_matrix_t c_kernel;
  igraph_matrix_t c_cites;
  igraph_integer_t c_agebins;
  igraph_real_t c_delta;
  igraph_vector_t c_filter;
  igraph_real_t c_logprob;
  igraph_real_t c_logmax;
  SEXP kernel;
  SEXP cites;
  SEXP logprob;
  SEXP logmax;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_niter=REAL(niter)[0];
  if (0 != igraph_matrix_init(&c_kernel, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_kernel);
  if (0 != igraph_matrix_init(&c_cites, 0, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_matrix_destroy, &c_cites); 
  cites=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  c_agebins=REAL(agebins)[0];
  c_delta=REAL(delta)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_ad(&c_graph, c_niter, &c_kernel, (isNull(cites) ? 0 : &c_cites), c_agebins, c_delta, (isNull(filter) ? 0 : &c_filter), &c_logprob, &c_logmax);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(4));
  PROTECT(names=NEW_CHARACTER(4));
  PROTECT(kernel=R_igraph_matrix_to_SEXP(&c_kernel)); 
  igraph_matrix_destroy(&c_kernel); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(cites=R_igraph_0ormatrix_to_SEXP(&c_cites)); 
  igraph_matrix_destroy(&c_cites); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logprob=NEW_NUMERIC(1)); 
  REAL(logprob)[0]=c_logprob;
  PROTECT(logmax=NEW_NUMERIC(1)); 
  REAL(logmax)[0]=c_logmax;
  SET_VECTOR_ELT(result, 0, kernel);
  SET_VECTOR_ELT(result, 1, cites);
  SET_VECTOR_ELT(result, 2, logprob);
  SET_VECTOR_ELT(result, 3, logmax);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("kernel"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("cites"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("logprob"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("logmax"));
  SET_NAMES(result, names);
  UNPROTECT(5);

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_probs_ad                   /
/-------------------------------------------*/
SEXP R_igraph_revolver_probs_ad(SEXP graph, SEXP kernel, SEXP ntk) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_matrix_t c_kernel;
  igraph_vector_t c_probs;
  igraph_vector_t c_citedprobs;
  igraph_vector_t c_citingprobs;
  igraph_bool_t c_ntk;
  SEXP probs;
  SEXP citedprobs;
  SEXP citingprobs;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_matrix(kernel, &c_kernel);
  if (0 != igraph_vector_init(&c_probs, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_probs); 
  probs=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_citedprobs, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_citedprobs); 
  citedprobs=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_citingprobs, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_citingprobs); 
  citingprobs=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  c_ntk=LOGICAL(ntk)[0];
                                        /* Call igraph */
  c_result=igraph_revolver_probs_ad(&c_graph, &c_kernel, (isNull(probs) ? 0 : &c_probs), (isNull(citedprobs) ? 0 : &c_citedprobs), (isNull(citingprobs) ? 0 : &c_citingprobs), c_ntk);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(probs=R_igraph_0orvector_to_SEXP(&c_probs)); 
  igraph_vector_destroy(&c_probs); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(citedprobs=R_igraph_0orvector_to_SEXP(&c_citedprobs)); 
  igraph_vector_destroy(&c_citedprobs); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(citingprobs=R_igraph_0orvector_to_SEXP(&c_citingprobs)); 
  igraph_vector_destroy(&c_citingprobs); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, probs);
  SET_VECTOR_ELT(result, 1, citedprobs);
  SET_VECTOR_ELT(result, 2, citingprobs);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("probs"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("citedprobs"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("citingprobs"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_D_alpha                 /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_D_alpha(SEXP graph, SEXP alpha, SEXP abstol, SEXP reltol, SEXP maxit, SEXP filter, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_real_t c_alpha;
  igraph_real_t c_Fmin;
  igraph_real_t c_abstol;
  igraph_real_t c_reltol;
  int c_maxit;
  igraph_vector_t c_filter;
  igraph_integer_t c_fncount;
  igraph_integer_t c_grcount;
  SEXP Fmin;
  SEXP fncount;
  SEXP grcount;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_alpha=REAL(alpha)[0];
  c_abstol=REAL(abstol)[0];
  c_reltol=REAL(reltol)[0];
  c_maxit=INTEGER(maxit)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_D_alpha(&c_graph, &c_alpha, &c_Fmin, c_abstol, c_reltol, c_maxit, (isNull(filter) ? 0 : &c_filter), &c_fncount, &c_grcount);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(4));
  PROTECT(names=NEW_CHARACTER(4));
  PROTECT(alpha=NEW_NUMERIC(1)); 
  REAL(alpha)[0]=c_alpha;
  PROTECT(Fmin=NEW_NUMERIC(1)); 
  REAL(Fmin)[0]=c_Fmin;
  PROTECT(fncount=NEW_NUMERIC(1)); 
  REAL(fncount)[0]=c_fncount;
  PROTECT(grcount=NEW_NUMERIC(1)); 
  REAL(grcount)[0]=c_grcount;
  SET_VECTOR_ELT(result, 0, alpha);
  SET_VECTOR_ELT(result, 1, Fmin);
  SET_VECTOR_ELT(result, 2, fncount);
  SET_VECTOR_ELT(result, 3, grcount);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("alpha"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("Fmin"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("fncount"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("grcount"));
  SET_NAMES(result, names);
  UNPROTECT(5);

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_D_alpha_a               /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_D_alpha_a(SEXP graph, SEXP alpha, SEXP a, SEXP abstol, SEXP reltol, SEXP maxit, SEXP filter, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_real_t c_alpha;
  igraph_real_t c_a;
  igraph_real_t c_Fmin;
  igraph_real_t c_abstol;
  igraph_real_t c_reltol;
  int c_maxit;
  igraph_vector_t c_filter;
  igraph_integer_t c_fncount;
  igraph_integer_t c_grcount;
  SEXP Fmin;
  SEXP fncount;
  SEXP grcount;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_alpha=REAL(alpha)[0];
  c_a=REAL(a)[0];
  c_abstol=REAL(abstol)[0];
  c_reltol=REAL(reltol)[0];
  c_maxit=INTEGER(maxit)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_D_alpha_a(&c_graph, &c_alpha, &c_a, &c_Fmin, c_abstol, c_reltol, c_maxit, (isNull(filter) ? 0 : &c_filter), &c_fncount, &c_grcount);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(5));
  PROTECT(names=NEW_CHARACTER(5));
  PROTECT(alpha=NEW_NUMERIC(1)); 
  REAL(alpha)[0]=c_alpha;
  PROTECT(a=NEW_NUMERIC(1)); 
  REAL(a)[0]=c_a;
  PROTECT(Fmin=NEW_NUMERIC(1)); 
  REAL(Fmin)[0]=c_Fmin;
  PROTECT(fncount=NEW_NUMERIC(1)); 
  REAL(fncount)[0]=c_fncount;
  PROTECT(grcount=NEW_NUMERIC(1)); 
  REAL(grcount)[0]=c_grcount;
  SET_VECTOR_ELT(result, 0, alpha);
  SET_VECTOR_ELT(result, 1, a);
  SET_VECTOR_ELT(result, 2, Fmin);
  SET_VECTOR_ELT(result, 3, fncount);
  SET_VECTOR_ELT(result, 4, grcount);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("alpha"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("a"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("Fmin"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("fncount"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("grcount"));
  SET_NAMES(result, names);
  UNPROTECT(6);

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_DE_alpha_a              /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_DE_alpha_a(SEXP graph, SEXP cats, SEXP alpha, SEXP a, SEXP coeffs, SEXP abstol, SEXP reltol, SEXP maxit, SEXP filter) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_cats;
  igraph_real_t c_alpha;
  igraph_real_t c_a;
  igraph_vector_t c_coeffs;
  igraph_real_t c_Fmin;
  igraph_real_t c_abstol;
  igraph_real_t c_reltol;
  int c_maxit;
  igraph_vector_t c_filter;
  igraph_integer_t c_fncount;
  igraph_integer_t c_grcount;
  SEXP Fmin;
  SEXP fncount;
  SEXP grcount;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_vector(cats, &c_cats);
  c_alpha=REAL(alpha)[0];
  c_a=REAL(a)[0];
  if (0 != R_SEXP_to_vector_copy(coeffs, &c_coeffs)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_coeffs);
  c_abstol=REAL(abstol)[0];
  c_reltol=REAL(reltol)[0];
  c_maxit=INTEGER(maxit)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_DE_alpha_a(&c_graph, &c_cats, &c_alpha, &c_a, &c_coeffs, &c_Fmin, c_abstol, c_reltol, c_maxit, (isNull(filter) ? 0 : &c_filter), &c_fncount, &c_grcount);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(6));
  PROTECT(names=NEW_CHARACTER(6));
  PROTECT(alpha=NEW_NUMERIC(1)); 
  REAL(alpha)[0]=c_alpha;
  PROTECT(a=NEW_NUMERIC(1)); 
  REAL(a)[0]=c_a;
  PROTECT(coeffs=R_igraph_vector_to_SEXP(&c_coeffs)); 
  igraph_vector_destroy(&c_coeffs); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(Fmin=NEW_NUMERIC(1)); 
  REAL(Fmin)[0]=c_Fmin;
  PROTECT(fncount=NEW_NUMERIC(1)); 
  REAL(fncount)[0]=c_fncount;
  PROTECT(grcount=NEW_NUMERIC(1)); 
  REAL(grcount)[0]=c_grcount;
  SET_VECTOR_ELT(result, 0, alpha);
  SET_VECTOR_ELT(result, 1, a);
  SET_VECTOR_ELT(result, 2, coeffs);
  SET_VECTOR_ELT(result, 3, Fmin);
  SET_VECTOR_ELT(result, 4, fncount);
  SET_VECTOR_ELT(result, 5, grcount);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("alpha"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("a"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("coeffs"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("Fmin"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("fncount"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("grcount"));
  SET_NAMES(result, names);
  UNPROTECT(7);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_AD_alpha_a_beta         /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_AD_alpha_a_beta(SEXP graph, SEXP alpha, SEXP a, SEXP beta, SEXP abstol, SEXP reltol, SEXP maxit, SEXP agebins, SEXP filter, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_real_t c_alpha;
  igraph_real_t c_a;
  igraph_real_t c_beta;
  igraph_real_t c_Fmin;
  igraph_real_t c_abstol;
  igraph_real_t c_reltol;
  int c_maxit;
  int c_agebins;
  igraph_vector_t c_filter;
  igraph_integer_t c_fncount;
  igraph_integer_t c_grcount;
  SEXP Fmin;
  SEXP fncount;
  SEXP grcount;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_alpha=REAL(alpha)[0];
  c_a=REAL(a)[0];
  c_beta=REAL(beta)[0];
  c_abstol=REAL(abstol)[0];
  c_reltol=REAL(reltol)[0];
  c_maxit=INTEGER(maxit)[0];
  c_agebins=INTEGER(agebins)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_AD_alpha_a_beta(&c_graph, &c_alpha, &c_a, &c_beta, &c_Fmin, c_abstol, c_reltol, c_maxit, c_agebins, (isNull(filter) ? 0 : &c_filter), &c_fncount, &c_grcount);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(6));
  PROTECT(names=NEW_CHARACTER(6));
  PROTECT(alpha=NEW_NUMERIC(1)); 
  REAL(alpha)[0]=c_alpha;
  PROTECT(a=NEW_NUMERIC(1)); 
  REAL(a)[0]=c_a;
  PROTECT(beta=NEW_NUMERIC(1)); 
  REAL(beta)[0]=c_beta;
  PROTECT(Fmin=NEW_NUMERIC(1)); 
  REAL(Fmin)[0]=c_Fmin;
  PROTECT(fncount=NEW_NUMERIC(1)); 
  REAL(fncount)[0]=c_fncount;
  PROTECT(grcount=NEW_NUMERIC(1)); 
  REAL(grcount)[0]=c_grcount;
  SET_VECTOR_ELT(result, 0, alpha);
  SET_VECTOR_ELT(result, 1, a);
  SET_VECTOR_ELT(result, 2, beta);
  SET_VECTOR_ELT(result, 3, Fmin);
  SET_VECTOR_ELT(result, 4, fncount);
  SET_VECTOR_ELT(result, 5, grcount);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("alpha"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("a"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("beta"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("Fmin"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("fncount"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("grcount"));
  SET_NAMES(result, names);
  UNPROTECT(7);

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_AD_dpareto              /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_AD_dpareto(SEXP graph, SEXP alpha, SEXP a, SEXP paralpha, SEXP parbeta, SEXP parscale, SEXP abstol, SEXP reltol, SEXP maxit, SEXP agebins, SEXP filter, SEXP verbose) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_real_t c_alpha;
  igraph_real_t c_a;
  igraph_real_t c_paralpha;
  igraph_real_t c_parbeta;
  igraph_real_t c_parscale;
  igraph_real_t c_Fmin;
  igraph_real_t c_abstol;
  igraph_real_t c_reltol;
  int c_maxit;
  int c_agebins;
  igraph_vector_t c_filter;
  igraph_integer_t c_fncount;
  igraph_integer_t c_grcount;
  SEXP Fmin;
  SEXP fncount;
  SEXP grcount;
  int c_result;
  SEXP result, names;

  R_igraph_before2(verbose, "");
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_alpha=REAL(alpha)[0];
  c_a=REAL(a)[0];
  c_paralpha=REAL(paralpha)[0];
  c_parbeta=REAL(parbeta)[0];
  c_parscale=REAL(parscale)[0];
  c_abstol=REAL(abstol)[0];
  c_reltol=REAL(reltol)[0];
  c_maxit=INTEGER(maxit)[0];
  c_agebins=INTEGER(agebins)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_AD_dpareto(&c_graph, &c_alpha, &c_a, &c_paralpha, &c_parbeta, &c_parscale, &c_Fmin, c_abstol, c_reltol, c_maxit, c_agebins, (isNull(filter) ? 0 : &c_filter), &c_fncount, &c_grcount);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(8));
  PROTECT(names=NEW_CHARACTER(8));
  PROTECT(alpha=NEW_NUMERIC(1)); 
  REAL(alpha)[0]=c_alpha;
  PROTECT(a=NEW_NUMERIC(1)); 
  REAL(a)[0]=c_a;
  PROTECT(paralpha=NEW_NUMERIC(1)); 
  REAL(paralpha)[0]=c_paralpha;
  PROTECT(parbeta=NEW_NUMERIC(1)); 
  REAL(parbeta)[0]=c_parbeta;
  PROTECT(parscale=NEW_NUMERIC(1)); 
  REAL(parscale)[0]=c_parscale;
  PROTECT(Fmin=NEW_NUMERIC(1)); 
  REAL(Fmin)[0]=c_Fmin;
  PROTECT(fncount=NEW_NUMERIC(1)); 
  REAL(fncount)[0]=c_fncount;
  PROTECT(grcount=NEW_NUMERIC(1)); 
  REAL(grcount)[0]=c_grcount;
  SET_VECTOR_ELT(result, 0, alpha);
  SET_VECTOR_ELT(result, 1, a);
  SET_VECTOR_ELT(result, 2, paralpha);
  SET_VECTOR_ELT(result, 3, parbeta);
  SET_VECTOR_ELT(result, 4, parscale);
  SET_VECTOR_ELT(result, 5, Fmin);
  SET_VECTOR_ELT(result, 6, fncount);
  SET_VECTOR_ELT(result, 7, grcount);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("alpha"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("a"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("paralpha"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("parbeta"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("parscale"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("Fmin"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("fncount"));
  SET_STRING_ELT(names, 7, CREATE_STRING_VECTOR("grcount"));
  SET_NAMES(result, names);
  UNPROTECT(9);

  R_igraph_after2(verbose);

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_AD_dpareto_eval         /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_AD_dpareto_eval(SEXP graph, SEXP alpha, SEXP a, SEXP paralpha, SEXP parbeta, SEXP parscale, SEXP agebins, SEXP filter) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_real_t c_alpha;
  igraph_real_t c_a;
  igraph_real_t c_paralpha;
  igraph_real_t c_parbeta;
  igraph_real_t c_parscale;
  igraph_real_t c_value;
  igraph_vector_t c_deriv;
  int c_agebins;
  igraph_vector_t c_filter;
  SEXP value;
  SEXP deriv;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  c_alpha=REAL(alpha)[0];
  c_a=REAL(a)[0];
  c_paralpha=REAL(paralpha)[0];
  c_parbeta=REAL(parbeta)[0];
  c_parscale=REAL(parscale)[0];
  if (0 != igraph_vector_init(&c_deriv, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_deriv);
  c_agebins=INTEGER(agebins)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_AD_dpareto_eval(&c_graph, c_alpha, c_a, c_paralpha, c_parbeta, c_parscale, &c_value, &c_deriv, c_agebins, (isNull(filter) ? 0 : &c_filter));

                                        /* Convert output */
  PROTECT(result=NEW_LIST(2));
  PROTECT(names=NEW_CHARACTER(2));
  PROTECT(value=NEW_NUMERIC(1)); 
  REAL(value)[0]=c_value;
  PROTECT(deriv=R_igraph_vector_to_SEXP(&c_deriv)); 
  igraph_vector_destroy(&c_deriv); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, value);
  SET_VECTOR_ELT(result, 1, deriv);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("value"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("deriv"));
  SET_NAMES(result, names);
  UNPROTECT(3);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_ADE_alpha_a_beta        /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_ADE_alpha_a_beta(SEXP graph, SEXP cats, SEXP alpha, SEXP a, SEXP beta, SEXP coeffs, SEXP abstol, SEXP reltol, SEXP maxit, SEXP agebins, SEXP filter) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_cats;
  igraph_real_t c_alpha;
  igraph_real_t c_a;
  igraph_real_t c_beta;
  igraph_vector_t c_coeffs;
  igraph_real_t c_Fmin;
  igraph_real_t c_abstol;
  igraph_real_t c_reltol;
  int c_maxit;
  int c_agebins;
  igraph_vector_t c_filter;
  igraph_integer_t c_fncount;
  igraph_integer_t c_grcount;
  SEXP Fmin;
  SEXP fncount;
  SEXP grcount;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_vector(cats, &c_cats);
  c_alpha=REAL(alpha)[0];
  c_a=REAL(a)[0];
  c_beta=REAL(beta)[0];
  if (0 != R_SEXP_to_vector_copy(coeffs, &c_coeffs)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_coeffs);
  c_abstol=REAL(abstol)[0];
  c_reltol=REAL(reltol)[0];
  c_maxit=INTEGER(maxit)[0];
  c_agebins=INTEGER(agebins)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_ADE_alpha_a_beta(&c_graph, &c_cats, &c_alpha, &c_a, &c_beta, &c_coeffs, &c_Fmin, c_abstol, c_reltol, c_maxit, c_agebins, (isNull(filter) ? 0 : &c_filter), &c_fncount, &c_grcount);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(7));
  PROTECT(names=NEW_CHARACTER(7));
  PROTECT(alpha=NEW_NUMERIC(1)); 
  REAL(alpha)[0]=c_alpha;
  PROTECT(a=NEW_NUMERIC(1)); 
  REAL(a)[0]=c_a;
  PROTECT(beta=NEW_NUMERIC(1)); 
  REAL(beta)[0]=c_beta;
  PROTECT(coeffs=R_igraph_vector_to_SEXP(&c_coeffs)); 
  igraph_vector_destroy(&c_coeffs); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(Fmin=NEW_NUMERIC(1)); 
  REAL(Fmin)[0]=c_Fmin;
  PROTECT(fncount=NEW_NUMERIC(1)); 
  REAL(fncount)[0]=c_fncount;
  PROTECT(grcount=NEW_NUMERIC(1)); 
  REAL(grcount)[0]=c_grcount;
  SET_VECTOR_ELT(result, 0, alpha);
  SET_VECTOR_ELT(result, 1, a);
  SET_VECTOR_ELT(result, 2, beta);
  SET_VECTOR_ELT(result, 3, coeffs);
  SET_VECTOR_ELT(result, 4, Fmin);
  SET_VECTOR_ELT(result, 5, fncount);
  SET_VECTOR_ELT(result, 6, grcount);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("alpha"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("a"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("beta"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("coeffs"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("Fmin"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("fncount"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("grcount"));
  SET_NAMES(result, names);
  UNPROTECT(8);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_ADE_dpareto             /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_ADE_dpareto(SEXP graph, SEXP cats, SEXP alpha, SEXP a, SEXP paralpha, SEXP parbeta, SEXP parscale, SEXP coeffs, SEXP abstol, SEXP reltol, SEXP maxit, SEXP agebins, SEXP filter) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_cats;
  igraph_real_t c_alpha;
  igraph_real_t c_a;
  igraph_real_t c_paralpha;
  igraph_real_t c_parbeta;
  igraph_real_t c_parscale;
  igraph_vector_t c_coeffs;
  igraph_real_t c_Fmin;
  igraph_real_t c_abstol;
  igraph_real_t c_reltol;
  int c_maxit;
  int c_agebins;
  igraph_vector_t c_filter;
  igraph_integer_t c_fncount;
  igraph_integer_t c_grcount;
  SEXP Fmin;
  SEXP fncount;
  SEXP grcount;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_vector(cats, &c_cats);
  c_alpha=REAL(alpha)[0];
  c_a=REAL(a)[0];
  c_paralpha=REAL(paralpha)[0];
  c_parbeta=REAL(parbeta)[0];
  c_parscale=REAL(parscale)[0];
  if (0 != R_SEXP_to_vector_copy(coeffs, &c_coeffs)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_coeffs);
  c_abstol=REAL(abstol)[0];
  c_reltol=REAL(reltol)[0];
  c_maxit=INTEGER(maxit)[0];
  c_agebins=INTEGER(agebins)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_ADE_dpareto(&c_graph, &c_cats, &c_alpha, &c_a, &c_paralpha, &c_parbeta, &c_parscale, &c_coeffs, &c_Fmin, c_abstol, c_reltol, c_maxit, c_agebins, (isNull(filter) ? 0 : &c_filter), &c_fncount, &c_grcount);

                                        /* Convert output */
  PROTECT(result=NEW_LIST(9));
  PROTECT(names=NEW_CHARACTER(9));
  PROTECT(alpha=NEW_NUMERIC(1)); 
  REAL(alpha)[0]=c_alpha;
  PROTECT(a=NEW_NUMERIC(1)); 
  REAL(a)[0]=c_a;
  PROTECT(paralpha=NEW_NUMERIC(1)); 
  REAL(paralpha)[0]=c_paralpha;
  PROTECT(parbeta=NEW_NUMERIC(1)); 
  REAL(parbeta)[0]=c_parbeta;
  PROTECT(parscale=NEW_NUMERIC(1)); 
  REAL(parscale)[0]=c_parscale;
  PROTECT(coeffs=R_igraph_vector_to_SEXP(&c_coeffs)); 
  igraph_vector_destroy(&c_coeffs); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(Fmin=NEW_NUMERIC(1)); 
  REAL(Fmin)[0]=c_Fmin;
  PROTECT(fncount=NEW_NUMERIC(1)); 
  REAL(fncount)[0]=c_fncount;
  PROTECT(grcount=NEW_NUMERIC(1)); 
  REAL(grcount)[0]=c_grcount;
  SET_VECTOR_ELT(result, 0, alpha);
  SET_VECTOR_ELT(result, 1, a);
  SET_VECTOR_ELT(result, 2, paralpha);
  SET_VECTOR_ELT(result, 3, parbeta);
  SET_VECTOR_ELT(result, 4, parscale);
  SET_VECTOR_ELT(result, 5, coeffs);
  SET_VECTOR_ELT(result, 6, Fmin);
  SET_VECTOR_ELT(result, 7, fncount);
  SET_VECTOR_ELT(result, 8, grcount);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("alpha"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("a"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("paralpha"));
  SET_STRING_ELT(names, 3, CREATE_STRING_VECTOR("parbeta"));
  SET_STRING_ELT(names, 4, CREATE_STRING_VECTOR("parscale"));
  SET_STRING_ELT(names, 5, CREATE_STRING_VECTOR("coeffs"));
  SET_STRING_ELT(names, 6, CREATE_STRING_VECTOR("Fmin"));
  SET_STRING_ELT(names, 7, CREATE_STRING_VECTOR("fncount"));
  SET_STRING_ELT(names, 8, CREATE_STRING_VECTOR("grcount"));
  SET_NAMES(result, names);
  UNPROTECT(10);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_ADE_dpareto_eval        /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_ADE_dpareto_eval(SEXP graph, SEXP cats, SEXP alpha, SEXP a, SEXP paralpha, SEXP parbeta, SEXP parscale, SEXP coeffs, SEXP agebins, SEXP filter) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_cats;
  igraph_real_t c_alpha;
  igraph_real_t c_a;
  igraph_real_t c_paralpha;
  igraph_real_t c_parbeta;
  igraph_real_t c_parscale;
  igraph_vector_t c_coeffs;
  igraph_real_t c_value;
  igraph_vector_t c_deriv;
  int c_agebins;
  igraph_vector_t c_filter;
  SEXP value;
  SEXP deriv;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_vector(cats, &c_cats);
  c_alpha=REAL(alpha)[0];
  c_a=REAL(a)[0];
  c_paralpha=REAL(paralpha)[0];
  c_parbeta=REAL(parbeta)[0];
  c_parscale=REAL(parscale)[0];
  R_SEXP_to_vector(coeffs, &c_coeffs);
  if (0 != igraph_vector_init(&c_deriv, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_deriv);
  c_agebins=INTEGER(agebins)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_ADE_dpareto_eval(&c_graph, &c_cats, c_alpha, c_a, c_paralpha, c_parbeta, c_parscale, &c_coeffs, &c_value, &c_deriv, c_agebins, (isNull(filter) ? 0 : &c_filter));

                                        /* Convert output */
  PROTECT(result=NEW_LIST(2));
  PROTECT(names=NEW_CHARACTER(2));
  PROTECT(value=NEW_NUMERIC(1)); 
  REAL(value)[0]=c_value;
  PROTECT(deriv=R_igraph_vector_to_SEXP(&c_deriv)); 
  igraph_vector_destroy(&c_deriv); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, value);
  SET_VECTOR_ELT(result, 1, deriv);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("value"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("deriv"));
  SET_NAMES(result, names);
  UNPROTECT(3);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_ml_ADE_dpareto_evalf       /
/-------------------------------------------*/
SEXP R_igraph_revolver_ml_ADE_dpareto_evalf(SEXP graph, SEXP cats, SEXP par, SEXP agebins, SEXP filter) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_vector_t c_cats;
  igraph_matrix_t c_par;
  igraph_vector_t c_value;
  int c_agebins;
  igraph_vector_t c_filter;
  SEXP value;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_vector(cats, &c_cats);
  R_SEXP_to_matrix(par, &c_par);
  if (0 != igraph_vector_init(&c_value, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_value);
  c_agebins=INTEGER(agebins)[0];
  if (!isNull(filter)) { R_SEXP_to_vector(filter, &c_filter); }
                                        /* Call igraph */
  c_result=igraph_revolver_ml_ADE_dpareto_evalf(&c_graph, &c_cats, &c_par, &c_value, c_agebins, (isNull(filter) ? 0 : &c_filter));

                                        /* Convert output */
  PROTECT(value=R_igraph_vector_to_SEXP(&c_value)); 
  igraph_vector_destroy(&c_value); 
  IGRAPH_FINALLY_CLEAN(1);
  result=value;

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}

/*-------------------------------------------/
/ igraph_revolver_probs_ADE_dpareto          /
/-------------------------------------------*/
SEXP R_igraph_revolver_probs_ADE_dpareto(SEXP graph, SEXP par, SEXP cats, SEXP gcats, SEXP agebins) {
                                        /* Declarations */
  igraph_t c_graph;
  igraph_matrix_t c_par;
  igraph_vector_t c_cats;
  igraph_vector_t c_gcats;
  int c_agebins;
  igraph_vector_t c_logprobs;
  igraph_vector_t c_logcited;
  igraph_vector_t c_logciting;
  SEXP logprobs;
  SEXP logcited;
  SEXP logciting;
  int c_result;
  SEXP result, names;

  R_igraph_before();
                                        /* Convert input */
  R_SEXP_to_igraph(graph, &c_graph);
  R_SEXP_to_matrix(par, &c_par);
  R_SEXP_to_vector(cats, &c_cats);
  R_SEXP_to_vector(gcats, &c_gcats);
  c_agebins=INTEGER(agebins)[0];
  if (0 != igraph_vector_init(&c_logprobs, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_logprobs); 
  logprobs=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_logcited, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_logcited); 
  logcited=NEW_NUMERIC(0); /* hack to have a non-NULL value */
  if (0 != igraph_vector_init(&c_logciting, 0)) { 
  igraph_error("", __FILE__, __LINE__, IGRAPH_ENOMEM); 
  } 
  IGRAPH_FINALLY(igraph_vector_destroy, &c_logciting); 
  logciting=NEW_NUMERIC(0); /* hack to have a non-NULL value */
                                        /* Call igraph */
  c_result=igraph_revolver_probs_ADE_dpareto(&c_graph, &c_par, &c_cats, &c_gcats, c_agebins, (isNull(logprobs) ? 0 : &c_logprobs), (isNull(logcited) ? 0 : &c_logcited), (isNull(logciting) ? 0 : &c_logciting));

                                        /* Convert output */
  PROTECT(result=NEW_LIST(3));
  PROTECT(names=NEW_CHARACTER(3));
  PROTECT(logprobs=R_igraph_0orvector_to_SEXP(&c_logprobs)); 
  igraph_vector_destroy(&c_logprobs); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logcited=R_igraph_0orvector_to_SEXP(&c_logcited)); 
  igraph_vector_destroy(&c_logcited); 
  IGRAPH_FINALLY_CLEAN(1);
  PROTECT(logciting=R_igraph_0orvector_to_SEXP(&c_logciting)); 
  igraph_vector_destroy(&c_logciting); 
  IGRAPH_FINALLY_CLEAN(1);
  SET_VECTOR_ELT(result, 0, logprobs);
  SET_VECTOR_ELT(result, 1, logcited);
  SET_VECTOR_ELT(result, 2, logciting);
  SET_STRING_ELT(names, 0, CREATE_STRING_VECTOR("logprobs"));
  SET_STRING_ELT(names, 1, CREATE_STRING_VECTOR("logcited"));
  SET_STRING_ELT(names, 2, CREATE_STRING_VECTOR("logciting"));
  SET_NAMES(result, names);
  UNPROTECT(4);

  R_igraph_after();

  UNPROTECT(1);
  return(result);
}
