#include "parser.h"
#include "srecd.h"

int main(int ac, char **av) {
  FILE *codefile;
  SE *prog,*result;

  printf("-= SrECD machine v 1.2 =-\n2012.02.01 M.J.Stanczyk, drcz@o2.pl\n\n");
  if(ac!=2) {
    printf("usage: %s <srecd machine code file>\n\n",av[0]);
  } else if((codefile=fopen(av[1],"r"))) {
      printf("loading %s...\n",av[1]);
      prog=read_se(codefile);
      Sreg=NIL;
      printf("allocating %d slots for environments...\n",numval(car(prog)));
      Ereg=(SE **)malloc(sizeof(SE)*numval(car(prog)));
      Creg=cdr(prog);
      Dreg=NIL;
      printf("go!\n\n");
      result=run();
      printf("result: ");
      write_se(result,stdout);
      printf("\n\ngoood nightdingdingdingding!\n");
      return 0;
  } else printf("holy crap, could not open file %s.\n",av[1]);
  return -1;
}
 
