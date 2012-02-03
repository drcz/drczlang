/* drcz0uproszcz - coś w guście preparsera; pozbawia wyrażenia drczowe
   niepotrzebnych białych znaków, omija komentarze {...} i zamienia
   'Wyrażenie na (quote Wyrażenie).
   otrzymany z innego narzędzia (patrz.niżej) poprzez wykomentowanie
   znakomitej części treści procedury drcz0sym_to_tdrcz0sym(char *),
   specjalnie na potrzeby implementacji drcz0 w javie.
   2011-12-06  */

/***************************************************************************
  narzędzie do pracy z drcz0 w scm (guile) - np. do kompilacji spod scm.

  zamienia reprezentację kodu w drcz0 na reprezentację tekstową, tj symbole
  primopów drcz'owych na ich alfanumeryczne odpowiedniki, np "." na "car";
  pełna lista zamian [prócz zamiany wyrażeń postaci 'X na (quote X), która
  była już wcześniej częścią parsera] znajduje się w wierszach 236-251 tego
  pliku.
  uwaga: program zamienia te symbole również w wyquotowanych listach - dla
  wygody przy kompilowaniu interpretera np.

                                         -- 2011-10-08 gdańsk drcz@o2.pl -- 
****************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

#define new(TYPE) (TYPE *)malloc(sizeof(TYPE))

void err(char *);

/** Symbolic Expressions: ***************************************************/
typedef struct SE SE;
typedef enum{NUM,SYM,STR,CONS}setype;

struct SE {
  setype type;
  union {
    double num;
    char *str;
    SE *cons[2];
  };
};

#define NIL NULL
SE *T;

#define type(S) (S)->type
#define numval(S) (S)->num
#define symval(S) (S)->str
#define strval(S) (S)->str
#define head(S) (S)->cons[0]
#define tail(S) (S)->cons[1]
#define isatom(S) ((S)==NIL||(S)->type!=CONS)
#define isnum(S) ((S)->type==NUM)
#define isstr(S) ((S)->type==STR)

SE *new_num(double);
SE *new_sym(char *);
SE *new_str(char *);
SE *new_cons(SE *,SE *); // inline?!

/** Parser: *****************************************************************/
void display(SE *);
SE *read();

void savese(FILE *,SE *);
SE *loadse(FILE *);

char *se2str(SE *);
SE *parse(char *);

/* ======================== IMPLEMENTACJA =================================== */

void err(char *msg) {
  fprintf(stderr,"ERR:%s\n",msg);
  exit(-1);
}

/** Symbolic Expressions: ***************************************************/
SE *new_num(double n) {
  SE *e=new(SE);
  if(e) {
      type(e)=NUM;
      numval(e)=n;
    }
   else err("new_num failed");
  return e;
}

SE *new_sym(char *s) {
  SE *e=new(SE);
  if(e) {
      type(e)=SYM;
      symval(e)=s;
    }
   else err("new_sym failed");
  return e;
}

SE *new_str(char *s) {
  SE *e=new(SE);
  if(e) {
      type(e)=STR;
      strval(e)=s;
    }
   else err("new_str failed");
  return e;
}

SE *new_cons(SE *h,SE *t) {
  SE *e=new(SE);
  if(e) {
      type(e)=CONS;
      head(e)=h;
      tail(e)=t;
    }
   else err("new_cons failed");
  return e;
}

/** Parser: *****************************************************************/
/* -- scanner: ----------------------------------------------- */
#define MAX_TOKEN 255

typedef enum{TNUM,TSYM,TSTR,TLPAR,TRPAR,TQUOTE,TEOF} toktype;

struct {
  toktype type;
  char buf[MAX_TOKEN+1];
}token;

int (*c_in)(void);
void (*c_putback)(int);

/* -- stream input ------------------------------------------- */
FILE *f_in;
int fc_in() {return fgetc(f_in);} // getc????
void fc_putback(int c) {ungetc(c,f_in);}

/* -- string input ------------------------------------------- */
char *s_in;

int sc_in() {
  int c=*s_in;
  s_in++;
  if(c=='\0') return EOF;
  return c;
}

void sc_putback(int c) {s_in--;}

/* -- scanner itself ----------------------------------------- */
void get_token() {
  int c,i=0;
 get_token_poczatek:
  while((c=c_in())==' ' || c=='\n' || c=='\t') ;
  // omijamy komentarze, czyli bloki { ... } :
  if(c=='{') {
      while((c = c_in()) != '}' && c!=EOF) ;
      goto get_token_poczatek;
    }
 
  switch(c) {
    case EOF: token.type=TEOF; return;
    case '(': token.type=TLPAR; return;
    case ')': token.type=TRPAR; return;
    case '\'': token.type=TQUOTE; return;
    case '"':
      token.type=TSTR;
      while(i<MAX_TOKEN && (c=c_in())!=EOF && c!='"') token.buf[i++]=c;
      token.buf[i]='\0';
      return;
    default:
      do {
	  token.buf[i++]=tolower(c);
      } while(i<MAX_TOKEN
	      && (c=c_in())!=EOF
	      && c!=' ' && c!='\n' && c!='\t'
	      && c!='(' && c!=')' && c!='\'');
      c_putback(c);
      token.buf[i]='\0';
      if(isdigit(token.buf[0])
	 || (i>1 && token.buf[0]=='-' && isdigit(token.buf[1])))
	 token.type=TNUM;
        else
	 token.type=TSYM;
    }
}

/* -- parser ------------------------------------------------- */
SE *get_tail();

SE *get_se() {
  switch(token.type) {
  case TEOF: exit(0); //return new_str(strdup("#eof")); <--- o!
    case TNUM: return new_num(atof(token.buf));
    case TSYM: return new_sym(strdup(token.buf));
    case TSTR: return new_str(strdup(token.buf));
    case TQUOTE: get_token(); /* ! */
      return new_cons(new_sym(strdup("quote")),
			      new_cons(get_se(),NIL));
    case TLPAR: return get_tail();
    default: break; /* err! */
    }
 return NIL; /* notreached */
}

SE *get_tail() {
  SE *hd,*tl;
  get_token();
  if(token.type==TRPAR) return NIL;
  hd=get_se();
  tl=get_tail();
  return new_cons(hd,tl);
}

SE *read_se() {
  get_token();
  return get_se();
}

/* -- output ------------------------------------------------- */
void (*c_out)(char *);

/* -- stream output ------------------------------------------ */
FILE *f_out;
void fc_out(char *s) {fprintf(f_out,"%s",s);}

/* -- string output ------------------------------------------ */
#define MAX_BUF 1023
char s_buf[MAX_BUF+1];
int _sbufc;
void sc_out(char *s) {
  while(_sbufc<MAX_BUF && *s!='\0') s_buf[_sbufc++]=*(s++);
  s_buf[_sbufc]='\0';
}

/* -- still output ------------------------------------------- */

char *drcz0sym_to_tdrcz0sym(char *symval) {
  return symval;
  /*
  if(symval[1]==0) { // to 1-znakowy symbol, może być że prymop drcz'owy!
    switch(symval[0]) {
    case '^': return strdup("lambda");
    case '.': return strdup("car");
    case ',': return strdup("cdr");
    case ';': return strdup("cons");
    case '@': return strdup("atom?");
    case '#': return strdup("number?");
    case '=': return strdup("eq?");
    case '>': return strdup("greater?");
    case '<': return strdup("less?");
    case '$': return strdup("eval");
    case '!': return strdup("label");
    case '+': return strdup("add");
    case '-': return strdup("sub");
    case '*': return strdup("mul");
    case '/': return strdup("div");
    case '%': return strdup("mod");
    default: return symval;
    }
  } else return symval;
  */
}

void write_tail(SE *);

void write_se(SE *e) {
  char numbuf[23];
  if(e==NIL)  {
    c_out("()");
    return;
  }
  switch(type(e)) {
    case NUM: sprintf(numbuf,"%f",numval(e));
              c_out(numbuf); break;
    case SYM: c_out(drcz0sym_to_tdrcz0sym(symval(e))); break; // !
    case STR: c_out("\""); c_out(strval(e));c_out("\""); break;
    case CONS: c_out("("); write_tail(e); break;
  }
}

void write_tail(SE *e) {
  if(e==NIL){c_out(")"); return;}
  write_se(head(e));
  if(tail(e)==NIL){c_out(")"); return;}
  if(isatom(tail(e))) {
      c_out(" . ");
      write_se(tail(e));
      c_out(")");
      return;
    }
  c_out(" ");
  write_tail(tail(e));
}

/* -- TADAAM ------------------------------------------------- */
void display(SE *e) {
  f_out=stdout;
  c_out=fc_out;
  write_se(e);
}

SE *read() {
  f_in=stdin;
  c_in=fc_in;
  c_putback=fc_putback;
  return read_se();
}

void savese(FILE *f,SE *e) {
  f_out=f;
  c_out=fc_out;
  write_se(e);
}

SE *loadse(FILE *f) {
  f_in=f;
  c_in=fc_in;
  c_putback=fc_putback;
  return read_se();
}

char *se2str(SE *e) {
  c_out=sc_out;
  _sbufc=0;
  write_se(e);
  return strdup(s_buf);
}

SE *parse(char *s) {
  s_in=s;
  c_in=sc_in;
  c_putback=sc_putback;
  return read_se();
}


/* read-print loop, wiesz. */
int main() {
  while(1) {  
    display(read());
    printf("\n");
  }
  return 0; /* notreached */
}

