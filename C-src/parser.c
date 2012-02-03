#include<stdio.h>
#include<string.h>
#include<ctype.h>

#include "parser.h"

/* this code comes from older implementations of drcz0, laziness prevents me from rewriting it. */

/* -- scanner: ----------------------------------------------- */
#define MAX_TOKEN 255

typedef enum {TNUM,TSYM,TLPAR,TRPAR,TQUOTE,TEOF} toktype;

struct {
  toktype type;
  char buf[MAX_TOKEN+1];
} token;

int (*c_in)(void);
void (*c_putback)(int);

/* -- stream input ------------------------------------------- */
FILE *f_in;
int fc_in(){return fgetc(f_in);} // getc????
void fc_putback(int c){ungetc(c,f_in);}

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
 get_token_start:
  while((c=c_in())==' ' || c=='\n' || c=='\t') ;
  if(c=='{') {   /* ommit comment blocks (enclosed in curly braces) : */
      while((c = c_in()) != '}' && c!=EOF) ;
      goto get_token_start;
    }
 
  switch(c) {
    case EOF: token.type=TEOF; return;
    case '(': token.type=TLPAR; return;
    case ')': token.type=TRPAR; return;
    case '\'': token.type=TQUOTE; return;
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
SE *get_cdr();

SE *get_se()
{
  switch(token.type)
    {
    case TEOF: return new_sym(strdup("#eof"));
    case TNUM: return new_num(atoi(token.buf));
    case TSYM: return new_sym(strdup(token.buf));
    case TQUOTE: get_token(); /* ! */
                 return new_cons(new_sym(strdup("quote")),
                                 new_cons(get_se(),NIL));
    case TLPAR: return get_cdr();
    default: break; /* err! */
    }
 return NIL; /* notreached */
}

SE *get_cdr() {
  SE *hd,*tl;
  get_token();
  if(token.type==TRPAR) return NIL;
  hd=get_se();
  tl=get_cdr();
  return new_cons(hd,tl);
}

SE *_read_se() {
  get_token();
  return get_se();
}

/* -- output ------------------------------------------------- */
void (*c_out)(char *);

/* -- stream output ------------------------------------------ */
FILE *f_out;
void fc_out(char *s) { fprintf(f_out,"%s",s); }

/* -- string output ------------------------------------------ */
#define MAX_BUF 1023
char s_buf[MAX_BUF+1];
int _sbufc;

void sc_out(char *s) {
  while(_sbufc<MAX_BUF && *s!='\0') s_buf[_sbufc++]=*(s++);
  s_buf[_sbufc]='\0';
}

/* -- more output -------------------------------------------- */
void write_cdr(SE *);

void _write_se(SE *e) {
  char numbuf[23];
  if(e==NIL) {
      c_out("()");
      return;
    }
  switch(type(e)) {
    case NUM: sprintf(numbuf,"%d",numval(e));
              c_out(numbuf); break;
    case SYM: c_out(symval(e)); break;
    case CONS: c_out("("); write_cdr(e); break;
    }
}

void write_cdr(SE *e) {
  if(e==NIL){c_out(")"); return;}
  _write_se(car(e));
  if(cdr(e)==NIL){c_out(")"); return;}
  if(type(cdr(e))!=CONS) {
      c_out(" . ");
      _write_se(cdr(e));
      c_out(")");
      return;
    }
  c_out(" ");
  write_cdr(cdr(e));
}

/* -- TADAAM ------------------------------------------------- */
/*
  uee tam, było minęło.

void display(SE *e) {
  f_out=stdout;
  c_out=fc_out;
  _write_se(e);
}

SE *read() {
  f_in=stdin;
  c_in=fc_in;
  c_putback=fc_putback;
  return _read_se();
}

void savese(FILE *f,SE *e)
{
  f_out=f;
  c_out=fc_out;
  _write_se(e);
}

SE *loadse(FILE *f)
{
  f_in=f;
  c_in=fc_in;
  c_putback=fc_putback;
  return _read_se();
}

char *se2str(SE *e)
{
  c_out=sc_out;
  _sbufc=0;
  _write_se(e);
  return strdup(s_buf);
}

SE *parse(char *s)
{
  s_in=s;
  c_in=sc_in;
  c_putback=sc_putback;
  return _read_se();
}
*/

//// o, tutej: ////

SE *read_se(FILE *in) {
  f_in=in;
  c_in=fc_in;
  c_putback=fc_putback;
  return _read_se();

}

void write_se(SE *e,FILE *out) {
  f_out=out;
  c_out=fc_out;
  _write_se(e);  
}
