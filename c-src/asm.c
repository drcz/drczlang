/* absmak. */

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<assert.h>
#include"mempool.h"
#include"parser.h"
#include"DRCZ.h"

void versioninfo(FILE *out,char *filename) {
  fprintf(out,"DRCZ* machine assembler v 1.0, 2012.08.05 M.J.Stańczyk (drcz@o2.pl)\n");
}

void helpinfo(FILE *out,char *filename) {
  fprintf(out,"usage:\n");
  fprintf(out,"cat <DRCZ* assembly file> | %s [options]\n",filename);
  fprintf(out,"with available options :\n");
  fprintf(out,"-p <size> - run with pool size <size> (in SE cells, not bytes, default 1024*1024, which takes ~12MB).\n");

}

void assemble(SE *,FILE *,char);
void gather_symbols(SE *);
void init_symbols_list();

int main(int ac,char **av) {
  int mempoolsize=1024*1024;
  SE *code;
  int c;

  if(ac>1) {
    while((c=getopt(ac,av,"vh?p:"))!=-1) {
      switch(c) {
      case 'v': versioninfo(stdout,av[0]); return 0;
      case '?': case 'h': versioninfo(stdout,av[0]); helpinfo(stdout,av[0]); return 0;
      case 'p':	mempoolsize=atoi(optarg); break;
      default: helpinfo(stdout,av[0]); return 1;
      }
    }
  }

  init_mempool(mempoolsize);
  init_symbols_list();
  gather_symbols((code=read_se(stdin)));
  assemble(code,stdout,1);
  printf("\n");
  return 0;
}

/***************************************************************************************************/
#define MAX_NO_SYMBOLS 1024
char *symbols[MAX_NO_SYMBOLS];
int symbols_p;
void push_symbol(char *sym) {
  int i; for(i=0;i<symbols_p;i++) if(strcmp(sym,symbols[i])==0) return;
  symbols[symbols_p++]=sym;
//printf("szatan symbol pcha: %d -> %s\n",symbols_p-1,sym);////dbg
}
void init_symbols_list() { symbols_p=0; push_symbol("t"); }

int sym2num(char *sym) {
  //  printf("sym2num: %s\n",sym);///dbg
  int i; for(i=0;i<symbols_p;i++) if(strcmp(sym,symbols[i])==0) return i;
  assert(0==1); /* :) */
}

void gather_symbols(SE *code) {
  char *opsym;
  while(code!=NULL) {
    /* unarne CONST, LOOKUP i FORGET przeskakujemy od razu */
    opsym=symval(car(code));
    if(strcmp(opsym,"const")==0 || strcmp(opsym,"lookup")==0 || strcmp(opsym,"forget")==0) {
      code=cdr(cdr(code)); continue;      
    } else /* NAME to to czego szukamy */ if(strcmp(opsym,"name")==0) {	
      push_symbol(symval(car(cdr(code))));
      code=cdr(cdr(code));
      continue;
    } else /* może S(R)ELECT? */ if(strcmp(opsym,"select")==0 || strcmp(opsym,"srelect")==0) {
      /* teraz czytelnik może się faktycznie wzruszyć */
      gather_symbols(car(cdr(code)));
      gather_symbols(car(cdr(cdr(code))));
      code=cdr(cdr(cdr(code)));
      continue;
    } else /* może chociaż PROC? */ if(strcmp(opsym,"proc")==0) {
      gather_symbols(car(cdr(code)));
      code=cdr(cdr(code));
      continue;
    } else /* a więc nic ciekawego */ {
      code=cdr(code);
    }
  } /* while... */
}

int opcode(char *op) {
  if(strcmp(op,"proc")==0) return O_CONST; /* sic! */
  else if(strcmp(op,"const")==0) return O_CONST;
  else if(strcmp(op,"lookup")==0) return O_LOOKUP;
  else if(strcmp(op,"name")==0) return O_NAME;
  else if(strcmp(op,"forget")==0) return O_FORGET;
  else if(strcmp(op,"select")==0) return O_SELECT;
  else if(strcmp(op,"srelect")==0) return O_SRELECT;
  else if(strcmp(op,"apply")==0) return O_APPLY;
  else if(strcmp(op,"trapply")==0) return O_TRAPPLY;
  else if(strcmp(op,"eq")==0) return O_EQ;
  else if(strcmp(op,"num")==0) return O_NUM;
  else if(strcmp(op,"atom")==0) return O_ATOM;
  else if(strcmp(op,"car")==0) return O_CAR;
  else if(strcmp(op,"cdr")==0) return O_CDR;
  else if(strcmp(op,"cons")==0) return O_CONS;
  else if(strcmp(op,"gt")==0) return O_GT;
  else if(strcmp(op,"lt")==0) return O_LT;
  else if(strcmp(op,"add")==0) return O_ADD;
  else if(strcmp(op,"sub")==0) return O_SUB;
  else if(strcmp(op,"mul")==0) return O_MUL;
  else if(strcmp(op,"div")==0) return O_DIV;
  else if(strcmp(op,"mod")==0) return O_MOD;
  else if(strcmp(op,"disp")==0) return O_DISP;
  else if(strcmp(op,"read")==0) return O_READ;
  else if(strcmp(op,"save")==0) return O_SAVE;
  else if(strcmp(op,"load")==0) return O_LOAD;
  else if(strcmp(op,"halt")==0) return O_HALT;
  else assert(0==1);
} /* `cat ops`split("\n").each{|a| a=a.strip;puts "  else if(strcmp(op,\"#{a.downcase}\")==0) return O_#{a};"} */


void assemble(SE *code,FILE *out,char first) {
  char *opsym;
  /* 1) machine code file should always start with number of used env-slots (symbols),
     2) machine code should at first add T atom to its environment, like this: (CONST t NAME t). */
  if(first==1) fprintf(out,"(%d %d t %d 0 ",symbols_p,O_CONST,O_NAME);
  else fprintf(out,"( ");
  while(code!=NULL) {
    opsym=symval(car(code));
    fprintf(out,"%d ",opcode(opsym));
    if(strcmp(opsym,"const")==0) {      
      write_se(car(cdr(code)),out); fprintf(out," ");
      code=cdr(cdr(code)); continue;

    } else if(strcmp(opsym,"lookup")==0 || strcmp(opsym,"name")==0 || strcmp(opsym,"forget")==0) {
      fprintf(out,"%d ",sym2num(symval(car(cdr(code)))));
      code=cdr(cdr(code)); continue;

    } else if(strcmp(opsym,"select")==0 || strcmp(opsym,"srelect")==0) {
      assemble(car(cdr(code)),out,0);
      assemble(car(cdr(cdr(code))),out,0);
      code=cdr(cdr(cdr(code)));
      continue;

    } else if(strcmp(opsym,"proc")==0) {
      assemble(car(cdr(code)),out,0);
      code=cdr(cdr(code));
      continue;

    } else {
      code=cdr(code);
    }
  } /* while... */
  fprintf(out,") ");
}
