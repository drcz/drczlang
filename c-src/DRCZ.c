#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include "SE.h"
#include "DRCZ.h"
#include "parser.h"

/* krótko o GC: przyjmujemy 'bezpieczną' taktykę bliską copy'n'sweep; opkody wiedzą, co mogą
   usunąć, a czym tylko żąglują, nastomiast sam lookup nie zwraca wyrażenia ze środowiska,
   tylko jego kopię; korzystamy z destroy(SE *) który jest 'płytki' (usuwa consa nie naruszając
   jego główki ani ogonka) do zmian strukturalnych (przy żąglowaniu), oraz destroy_rec(SE *)
   (który usuwa całe zadane wyrażenie) przy pozbywaniu się wyników pośrednich. */

#define T_ATOM D_lookup(0)
/* A tak, bo kompilator dba by kod rozpoczynało sakramentalne "CONST T NAME T" - wtedy
   asembler w czasie zliczania symboli użytych w kodzie natrafiając na "T" jako pierwsze
   nadaje mu ,,adres'' 0. */

void code_next_1();
void code_next_2();
void code_next_3();

#ifdef COUNT_NUMBER_OF_STEPS
/* Czasem się może przydać. Czasem nie. */
unsigned long no_steps=0;
unsigned long get_number_of_steps() { return no_steps; }
#endif


SE *run() {
  FILE *f;
  SE *arg1,*arg2;
  SE *current_frame;

 top:
#ifdef COUNT_NUMBER_OF_STEPS
  no_steps++;
#endif
  if((current_frame=car(Creg))==NIL) {
    C_pop();
    if(Creg==NIL) return R_pop();
    goto top;
  }

  //printf("==>%d\n",numval(car(current_frame)));///dbg

  switch(numval(car(current_frame))) {

  case O_CONST: ///////////////////////////////////////////////////////////////////////////////////////////////
    arg1=car(cdr(current_frame));
    R_push(arg1);
    code_next_2();    
    goto top;

  case O_LOOKUP: //////////////////////////////////////////////////////////////////////////////////////////////
    arg1=car(cdr(current_frame));
    R_push(D_lookup(numval(arg1)));
    code_next_2();
    destroy(arg1);
    goto top;

  case O_NAME: /////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=car(cdr(current_frame));
    D_push(numval(arg1),R_pop());
    code_next_2();
    destroy(arg1);
    goto top;

  case O_FORGET: ///////////////////////////////////////////////////////////////////////////////////////////////
    arg1=car(cdr(current_frame));
    D_pop(numval(arg1));
    code_next_2();
    destroy(arg1);
    goto top;

  case O_SELECT: ///////////////////////////////////////////////////////////////////////////////////////////////
    arg1=car(cdr(current_frame));
    arg2=car(cdr(cdr(current_frame)));
    code_next_3(); // !
    if(R_pop()==NIL) {
      C_push(arg2);
      destroy_rec(arg1);
    } else {
      C_push(arg1);
      destroy_rec(arg2);
    }
    goto top;

  case O_SRELECT: //////////////////////////////////////////////////////////////////////////////////////////////
    arg1=car(cdr(current_frame));
    arg2=car(cdr(cdr(current_frame)));
    code_next_3();
    C_pop(); /* ! */
    if(R_pop()==NIL) {
      C_push(arg2);
      destroy_rec(arg1);
    } else {
      C_push(arg1);
      destroy_rec(arg2);
    }
    goto top;

  case O_APPLY:  ////////////////////////////////////////////////////////////////////////////////////////////////
    code_next_1();
    C_push(R_pop());
    goto top;

  case O_TRAPPLY:  //////////////////////////////////////////////////////////////////////////////////////////////
    code_next_1(); //<- czy nie trzeba, C_pop() go zniszczy?
    C_pop(); /* ! */
    C_push(R_pop());
    goto top;

  case O_EQ: ////////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop();
    arg2=R_pop();
    if(are_equal(arg1,arg2)) R_push(T_ATOM);
      else R_push(NIL);
    code_next_1();
    destroy_rec(arg1);
    destroy_rec(arg2);
    goto top;

  case O_NUM: ///////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop();
    if(arg1 && type(arg1)==NUM) R_push(T_ATOM);
    else R_push(NIL);
    code_next_1();
    destroy_rec(arg1);
    goto top;

  case O_ATOM: //////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop();
    if(!arg1 || type(arg1)!=CONS) R_push(T_ATOM);
    else R_push(NIL);
    code_next_1();
    destroy_rec(arg1);
    goto top;
      
  case O_CAR: ///////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop();
    assert(arg1);
    assert(type(arg1)==CONS);
    R_push(car(arg1));
    // przy CAR i CDR nie niszczymy całego argumentu, jedynie jego ramę cons i niepotrzebny elem.pary
    destroy_rec(cdr(arg1));
    destroy(arg1);
    code_next_1();
    goto top;
      
  case O_CDR: ///////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop();
    assert(arg1);
    assert(type(arg1)==CONS);
    R_push(cdr(arg1));
    destroy_rec(car(arg1));
    destroy(arg1);
    code_next_1();
    goto top;

  case O_CONS: //////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop();
    arg2=R_pop();
    R_push(new_cons(arg1,arg2));
    code_next_1();
    goto top;


  case O_ADD: ///////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop(); arg2=R_pop();
    assert(arg1); assert(arg2);
    assert(type(arg1)==NUM && type(arg2)==NUM);
    R_push(new_num(numval(arg1)+numval(arg2)));      
    code_next_1();
    destroy(arg1);
    destroy(arg2);
    goto top;

  case O_SUB: ///////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop(); arg2=R_pop();
    assert(arg1); assert(arg2);
    assert(type(arg1)==NUM && type(arg2)==NUM);
    R_push(new_num(numval(arg1)-numval(arg2)));      
    code_next_1();
    destroy(arg1);
    destroy(arg2);
    goto top;

  case O_MUL: ///////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop(); arg2=R_pop();
    assert(arg1); assert(arg2);
    assert(type(arg1)==NUM && type(arg2)==NUM);
    R_push(new_num(numval(arg1)*numval(arg2)));      
    code_next_1();
    destroy(arg1);
    destroy(arg2);
    goto top;

  case O_DIV: ///////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop(); arg2=R_pop();
    assert(arg1); assert(arg2);
    assert(type(arg1)==NUM && type(arg2)==NUM);
    assert(numval(arg2)!=0);
    R_push(new_num(numval(arg1)/numval(arg2)));      
    code_next_1();
    destroy(arg1);
    destroy(arg2);
    goto top;

  case O_MOD: ///////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop(); arg2=R_pop();
    assert(arg1); assert(arg2);
    assert(type(arg1)==NUM && type(arg2)==NUM);
    R_push(new_num(numval(arg1)%numval(arg2)));      
    code_next_1();
    destroy(arg1);
    destroy(arg2);
    goto top;

  case O_GT: ////////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop(); arg2=R_pop();
    assert(arg1); assert(arg2);
    assert(type(arg1)==NUM && type(arg2)==NUM);
    if(numval(arg1)>numval(arg2)) R_push(T_ATOM);
    else R_push(NIL);
    code_next_1();
    destroy(arg1);
    destroy(arg2);
    goto top;

  case O_LT: ////////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop(); arg2=R_pop();
    assert(arg1); assert(arg2);
    assert(type(arg1)==NUM && type(arg2)==NUM);
    if(numval(arg1)<numval(arg2)) R_push(T_ATOM);
    else R_push(NIL);
    code_next_1();
    destroy(arg1);
    destroy(arg2);
    goto top;


  case O_DISP: //////////////////////////////////////////////////////////////////////////////////////////////////
    write_se(car(Rreg),stdout); printf("\n");
    code_next_1();
    goto top;

  case O_READ: //////////////////////////////////////////////////////////////////////////////////////////////////
    if(!silent) printf(">");
    R_push(read_se(stdin));
    code_next_1();
    goto top;

  case O_SAVE: //////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop();
    f=fopen(symval(arg1),"w");
    write_se(car(Rreg),f);
    fclose(f);
    code_next_1();
    goto top;

  case O_LOAD: //////////////////////////////////////////////////////////////////////////////////////////////////
    arg1=R_pop();
    f=fopen(symval(arg1),"r");
    R_push(read_se(f));
    fclose(f);
    code_next_1();
    goto top;

  case O_HALT: //////////////////////////////////////////////////////////////////////////////////////////////////
    if(!silent) printf("<HALT!>\n");
    exit(1);

  default: assert(0==1); /* ;) */
  }
}

/**************************************************************************************/

/* acha, one - tj te operacje na 'rejestrach' - zasadniczo mają osadzone w sobie odśmiecanie;
   choć pewne rzeczy są czyszczone po stronie operatorów; nie jest to jakaś asymetria, a jedynie
   ,,dzielenie się obowiązkami'' - pewne SExpy usuwa się z pewnością, inne tylko ,,okazjonalnie''. */

void R_push(SE *e) {
  Rreg=new_cons(e,Rreg);
}

SE *R_pop() {
  SE *tmp=Rreg,*top; // register??
  assert(tmp);
  top=car(tmp);
  Rreg=cdr(tmp);
  destroy(tmp);
  return top;
}

void D_push(int key,SE *e) {
  Dreg[key]=new_cons(e,Dreg[key]);
}

void D_pop(int key) {
  SE *tmp=Dreg[key]; // register?
  assert(tmp);
  Dreg[key]=cdr(tmp);
  destroy_rec(car(tmp));
  destroy(tmp);
}

SE *D_lookup(int key) {
  assert(Dreg[key]);
  return clone_rec(car(Dreg[key]));
}

void C_push(SE *proc) {
  Creg=new_cons(proc,Creg);
}

void C_pop() {
  SE *tmp=Creg; // register??
  assert(tmp);
  destroy_rec(car(tmp));
  Creg=cdr(tmp);
  destroy(tmp);
}

/* A te cuda ,,przesuwają taśmę bieżącej ramki'', usuwając przy okazji zbyteczny szkielet wagoników;
   proszę zwrócić uwagę że code_next_2 i code_next_3 _nie usuwają_ arugmentów występujących w kodzie
   - bo prócz NAME, FORGET i LOOKUP są one potrzebne. A trzech wspomnianych dżentelmenów radzi sobie
   dzielnie po swojej stronie [poprzez destroy'e]. */

#define CURRENT_FRAME car(Creg)
#define CURRENT_COMMAND car(car(Creg))

void code_next_1() {
  SE *tmp=CURRENT_FRAME; /* [op . hd(C)] */
  CURRENT_FRAME=cdr(CURRENT_FRAME); /* hd(C) */
  destroy(car(tmp)); /* op, tj CURRENT_COMMAND */
  destroy(tmp);      /* {op . hd(C)} */
}

void code_next_2() {
  SE *tmp=CURRENT_FRAME; /* [op . [arg1 . hd(C)]] */
  CURRENT_FRAME=cdr(cdr(CURRENT_FRAME)); /* hd(C) */
  destroy(car(tmp)); /* op */
  destroy(cdr(tmp)); /* {arg1 . hd(C)} */
  destroy(tmp); /* {op . [arg1 . hd(C)]} */
}

void code_next_3() {
  SE *tmp=CURRENT_FRAME; /* [op . [arg1 . [arg2 . hd(C)]]] */
  CURRENT_FRAME=cdr(cdr(cdr(CURRENT_FRAME))); /* hd(C) */
  destroy(car(tmp)); /* op */
  destroy(cdr(cdr(tmp))); /* {arg2 . hd(C)} */
  destroy(cdr(tmp)); /* {arg1 . [arg2 . hd(C)]} */
  destroy(tmp); /* {op . [arg1 . [arg2 . hd(C)]]} */
}
