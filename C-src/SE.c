#include "SE.h"
#include<string.h> /* strdup */

/* gdyby kiedy chcial tu debugowe te, wiesz, proby wstawiac.
#include<stdio.h>
#include"parser.h"
*/

/* new_cons i destroy są moimi kandydatami na inline... */

SE *new_num(int n) {
  SE *e=new(SE);
  assert(e);
  type(e)=NUM;
  numval(e)=n;
  return e;
}

SE *new_sym(char *str) {
  SE *e=new(SE);
  assert(e);
  type(e)=SYM;
  symval(e)=str;
  return e;
}

SE *new_cons(SE *hd,SE *tl) {
  SE *e=new(SE);
  assert(e);
  type(e)=CONS;
  car(e)=hd;
  cdr(e)=tl;
  return e;
}

void destroy(SE *e) {
  if(e) {
    if(type(e)==SYM) free(symval(e));
    free(e);
  }
}

void destroy_rec(SE *e) {
  if(e) {
    if(type(e)==SYM) {
      free(symval(e));
    } else if(type(e)==CONS) {
      destroy_rec(car(e));
      destroy_rec(cdr(e));
    }
    free(e);
  }
}

SE *clone(SE *e) {
  if(e) {
    switch(type(e)) {
    case NUM: return new_num(numval(e));
    case SYM: return new_sym(strdup(symval(e)));
    case CONS: return new_cons(car(e),cdr(e));
    }
  } else return NIL;
  return NIL; /*notreached*/
}

SE *clone_rec(SE *e) {
  if(e) {
    switch(type(e)) {
    case NUM: return new_num(numval(e));
    case SYM: return new_sym(strdup(symval(e)));
    case CONS: return new_cons(clone_rec(car(e)),clone_rec(cdr(e)));
    }
  } else return NIL;
  return NIL; /*notreached*/
}

int are_equal(SE *e1,SE *e2) {
  if(e1==NIL && e2==NIL) return 1;
  else if(e1==NIL || e2==NIL) return 0;
  else if(type(e1)!=type(e2)) return 0;
  else if(type(e1)==NUM && numval(e1)==numval(e2)) return 1;
  else if(type(e1)==SYM && strcmp(symval(e1),symval(e2))==0) return 1;
  else return 0;
}

