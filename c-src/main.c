#include<stdlib.h> /* malloc */
#include<unistd.h> /* getopt etc. */

#include"mempool.h"
#include"parser.h"
#include"DRCZ.h"

void versioninfo(FILE *out,char *filename) {
  fprintf(out,"DRCZ* machine v 1.1, 2012.07.29 M.J.Stańczyk (drcz@o2.pl)\n");
}

void helpinfo(FILE *out,char *filename) {
  fprintf(out,"usage:\n");
  fprintf(out,"%s [options] <srecd machine code file>\n",filename);
  fprintf(out,"with available options :\n");
  fprintf(out,"-h or -? - this information,\n");
  fprintf(out,"-c - tells the size of one CONS cell on current system,\n");
  fprintf(out,"-s - silent mode, displays only the resulting SE (on stdout, followed by newline character),\n");
  fprintf(out,"-p <size> - run with pool size <size> (in SE cells, not bytes, default 1024*1024, which takes ~12MB).\n");
}

void conscellinfo(FILE *out,char *filename) {
  fprintf(out,"On this system one CONS cell weights %d bytes.\n",sizeof(SE));
}

int main(int ac, char **av) {
  FILE *codefile=stdin;
  int  mempoolsize=1024*1024;
  int c;
  SE *prog,*result;

  silent=0;

  if(ac<2) {
    versioninfo(stderr,av[0]);
    helpinfo(stderr,av[0]);
    return 1;
  }

  while((c=getopt(ac,av,"svch?p:"))!=-1) {
    switch(c) {

    case 's':
      silent=1;
      break;

    case 'c':
      conscellinfo(stdout,av[0]);
      return 0;

    case 'v':
      versioninfo(stdout,av[0]);
      return 0;

    case '?':
    case 'h':
      versioninfo(stdout,av[0]);
      helpinfo(stdout,av[0]);
      return 0;

    case 'p':
      mempoolsize=atoi(optarg);
      break;

    default:      
      // versioninfo(stderr,av[0]);
      // helpinfo(stdout,av[0]);
      return 1;      
    }
  }

  if((codefile=fopen(av[optind],"r"))) {
    if(silent!=1) printf("Initializing mempool (%d cells)...\n",mempoolsize);
    init_mempool(mempoolsize);

    if(silent!=1) printf("Loading code from %s...\n",av[optind]);
    prog=read_se(codefile);

    if(silent!=1) printf("Allocating %d env-slot(s) for environments...\n",numval(car(prog)));
    Dreg=(SE **)malloc(sizeof(SE *)*numval(car(prog)));

    Rreg=NIL;
    Creg=NIL;

    C_push(cdr(prog));

    if(silent!=1) printf("Running the machine.\n");
    result=run();

    if(silent!=1) printf("Result: ");
    write_se(result,stdout);
    printf("\n"); // ?

    if(silent!=1) printf("\nAuf wiedersehen!\n");
    return 0;

  } else fprintf(stderr,"Could not open file %s.\n",av[optind]);

  return 1;
}
 
