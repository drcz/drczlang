#include<string.h> /* bo strdup... */
#include<assert.h>
#include<stdlib.h> /* bo free (dla stringów, na razie...) */
#include"mempool.h" /* novum! */
#include "SE.h"

/* gdyby kiedy chcial tu debugowe te, wiesz, proby wstawiac.
#include"parser.h"
*/

//#define _MEM_STATS 997 // sieg heil szatan

#ifdef _MEM_STATS
#include<stdio.h> // fprintf, stderr...
#endif

// new_cons i destroy są moimi kandydatami na inline...

SE *new_num(int n) {
  SE *e=alloc_se();
  assert(e);
#ifdef _MEM_STATS
  fprintf(stderr,"new num [%d]\n",n);
#endif
  type(e)=NUM;
  numval(e)=n;
  return e;
}

SE *new_sym(char *str) {
  SE *e=alloc_se();
  assert(e);
#ifdef _MEM_STATS
  fprintf(stderr,"new sym [%s]\n",str);
#endif
  type(e)=SYM;
  symval(e)=str;
  return e;
}

SE *new_cons(SE *hd,SE *tl) {
  SE *e=alloc_se();
  assert(e);
#ifdef _MEM_STATS
  fprintf(stderr,"new cons\n");
#endif
  type(e)=CONS;
  car(e)=hd;
  cdr(e)=tl;
  return e;
}

void destroy(SE *e) {
  if(e) {
    /* nowość: nie dealokujemy i nie klonujemy ciągów znaków, jedynie wskaźniki do nich;
       przy wykonywaniu programu wspomnianego w nagłówku mempool.c 8.6mln alokowań/dealokowań
       symboli dotyczyło cały czas tych samych 66 ciągów znaków.
       zauważmy, że w programie w którym nie jest używany op.pierwotny READ jedyne ciągi znaków
       jakie mogą być noszone przez symbole to T, oraz te które występują bezpośrednio w kodzie...
       zatem - przynajmniej na razie - olewamy kwestię zarządzania pamięcią dla ciągów znaków.

    if(type(e)==SYM) {
#ifdef _MEM_STATS
    fprintf(stderr,"destroy sym=%s ",symval(e));
#endif
      free(symval(e));
    }
    */
#ifdef _MEM_STATS
    fprintf(stderr,"destroy [t=%d]\n",type(e));
#endif
    free_se(e);
  }
#ifdef _MEM_STATS
  else fprintf(stderr,"destroy [NIL]\n");
#endif

}

void destroy_rec(SE *e) {
  if(e) {
    /* jw.
    if(type(e)==SYM) {
#ifdef _MEM_STATS
      fprintf(stderr,"destroy sym=%s ",symval(e));
#endif
      free(symval(e));
      } else*/ if(type(e)==CONS) {
      destroy_rec(car(e));
      destroy_rec(cdr(e));
    }
#ifdef _MEM_STATS
    fprintf(stderr,"destroy_rec [t=%d]\n",type(e));
#endif
    free_se(e);
  }
#ifdef _MEM_STATS
    else fprintf(stderr,"destroy_rec [NIL]\n");
#endif

}

SE *clone(SE *e) {
  if(e) {
    switch(type(e)) {
    case NUM: return new_num(numval(e));
//    case SYM: return new_sym(strdup(symval(e)));
    case SYM: return new_sym(symval(e)); // cf. destroy(SE *)...
    case CONS: return new_cons(car(e),cdr(e));
    }
  } else return NIL;
  return NIL; /*notreached*/
}

SE *clone_rec(SE *e) {
  if(e) {
    switch(type(e)) {
    case NUM: return new_num(numval(e));
//    case SYM: return new_sym(strdup(symval(e)));
    case SYM: return new_sym(symval(e)); // cf. destroy(SE *)
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

