/* trzaskanie malloc i free po 20-30mln razy w ciągu wykonania porgramu* sprawia że całość
   chodzi jak gówno (a do tego pamięć się fragmentuje) - potrzebny będzie memory pool elegancki.
   [na razie trzeba zignorować alokowanie pod stringi; na 8mln stringów przypadło 13mln consów]
   pomysł jest taki, żeby wpierw alokować wielką stertę, interpretować jako tablicę SExpów,
   a ich adresy trzymać na stosie (klasycznym, zainfekowanym chorobą programistów C - wg
   moich podliczeń najwyższe obłożenie we wspomnianym programie to było niecałe 500 000
   zaalokowanych komórkuf, więc pewnie stos długi na (2^20)*sizeof(SE *) to aż nadto....).
  --------------------------------------------------------------------------------------
  *) tym programem był interpreter drcz0 odpalony w interpreterze drcz1 i wykonujący
  proste podstawienie ((^ (x) (* x x)) (+ 2 3)). */

#include<stdlib.h> /* tak, dla malloc i free */
#include<assert.h> /* :) */
#include"SE.h"

//#include<stdio.h> // do testowania...

#define POOL_SIZE 2*1024*1024 /* to jest rozmiar w sizeof(SE) a nie w bajtach */

SE *mempool; /* cf. http://www.youtube.com/watch?v=7BEsP2X1JD8 */

/* memstak trzyma wskaźniki do wolnych komórek z mempóli */
SE **memstack;
unsigned int memstack_p;

SE *alloc_se() { /* czyli "memstack_pop()" */
  assert(memstack_p>0);
  return memstack[--memstack_p];
}

void free_se(SE *e) { /* "memstack_push(e)" */
  assert(memstack_p<POOL_SIZE);
  memstack[memstack_p++]=e;
}

void init_mempool() {
  int i;
  mempool=(SE *)malloc(POOL_SIZE*sizeof(SE));
  assert(mempool);
  memstack=(SE **)malloc(sizeof(SE *)*POOL_SIZE);
  assert(memstack);
  for(i=0;i<POOL_SIZE;i++) free_se(&mempool[i]); /* błyskotliwe! */
}

void destroy_mempool() {
  free(mempool);
  free(memstack);
}

/*
// testujemi...
main() {
  int i;
  SE *abc[23];
  init_mempool();
  for(i=0;i<10;i++) {
    abc[i]=alloc_se();
    printf("alok; abc=%p; memstack_p=%d\n",abc[i],memstack_p);
  }
  for(i=0;i<4;i++) {
    printf("fri; abc=%p;",abc[i]);
    free_se(abc[i]);
    printf("memstack_p=%d\n",memstack_p);
  }
  for(i=12;i<16;i++) {
    abc[i]=alloc_se();
    printf("alok; abc=%p; memstack_p=%d\n",abc[i],memstack_p);
  }
  destroy_mempool();
  return 0;
}
*/
