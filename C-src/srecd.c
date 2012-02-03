#include "SE.h"
#include "srecd.h"
#include "parser.h"

/* krótko o GC: przyjmujemy 'bezpieczną' taktykę bliską copy'n'sweep; opkody wiedzą, co mogą
   usunąć, a czym tylko żąglują, nastomiast sam lookup nie zwraca wyrażenia ze środowiska,
   tylko jego kopię; korzystamy z destroy(SE *) który jest 'płytki' (usuwa consa nie naruszając
   jego główki ani ogonka) do zmian strukturalnych (przy żąglowaniu), oraz destroy_rec(SE *)
   (który usuwa całe zadane wyrażenie) przy pozbywaniu się wyników pośrednich. */

#define T_ATOM env_lookup(0) /// new_sym("T") ma ten problem, że to T jest statyczne...
/// tj robienie free na takim cacku to SŁABY pomysł.

void code_next_1();
void code_next_2();
void code_next_3();

SE *run() {
  SE *arg1,*arg2,*tmp;

 top:
  if(Creg) {
    //    printf("op goes=%d\n",numval(car(Creg)));///dbg
    switch(numval(car(Creg))) {

    case O_LDC:
      arg1=car(cdr(Creg));
      push(arg1);
      code_next_2();
      goto top;

    case O_LU:
      arg1=car(cdr(Creg));
      push(env_lookup(numval(arg1)));
      code_next_2();
      destroy(arg1);
      goto top;

    case O_BIND:
      arg1=car(cdr(Creg));
      tmp=pop();
      env_add(numval(arg1),tmp);
      code_next_2();
      destroy(arg1);
      goto top;
      
    case O_UNBIND:
      arg1=car(cdr(Creg));
      env_rem(numval(arg1));
      code_next_2();
      destroy(arg1);
      goto top;

    case O_SEL:
      arg1=car(cdr(Creg));
      arg2=car(cdr(cdr(Creg)));
      tmp=pop();
      code_next_3();
      dump();
      if(tmp) {
	set_code(arg1);
	destroy_rec(arg2);
	destroy_rec(tmp);
      } else {
	set_code(arg2);
	destroy_rec(arg1);
      }
      goto top;

    case O_SER:
      arg1=car(cdr(Creg));
      arg2=car(cdr(cdr(Creg)));
      tmp=pop();
      code_next_3();
      if(tmp) {
	set_code(arg1);
	destroy_rec(arg2);
      } else {
	set_code(arg2);
	destroy_rec(arg1);
      }
      destroy_rec(tmp);
      goto top;

    case O_AP:
      code_next_1();
      dump();
      set_code(pop());
      goto top;

    case O_TRAP:
      code_next_1();
      set_code(pop());
      goto top;

    case O_RTN:
      code_next_1();
      redump();
      goto top;

    case O_EQ:
      arg1=pop();
      arg2=pop();
      if(are_equal(arg1,arg2)) push(T_ATOM);
      else push(NIL);
      code_next_1();
      destroy_rec(arg1);
      destroy_rec(arg2);
      goto top;
      
    case O_NUM:
      arg1=pop();
      if(arg1 && type(arg1)==NUM) push(T_ATOM);
      else push(NIL);
      code_next_1();
      destroy_rec(arg1);
      goto top;

    case O_ATOM:
      arg1=pop();
      if(!arg1 || type(arg1)!=CONS) push(T_ATOM);
      else push(NIL);
      code_next_1();
      destroy_rec(arg1);
      goto top;
      
    case O_CAR: /* przy CAR i CDR nie niszczymy całego argumentu, jedynie jego ramę cons i niepotrzebny elem.pary */
      arg1=pop();
      assert(arg1);
      assert(type(arg1)==CONS);
      push(car(arg1));
      destroy_rec(cdr(arg1));
      destroy(arg1);
      code_next_1();
      goto top;
      
    case O_CDR: /* jw. */
      arg1=pop();
      assert(arg1);
      assert(type(arg1)==CONS);
      push(cdr(arg1));
      destroy_rec(car(arg1));
      destroy(arg1);
      code_next_1();
      goto top;

    case O_CONS: /* a przy CONS w ogóle nic nie niszczymy */
      arg1=pop();
      arg2=pop();
      push(new_cons(arg1,arg2));
      code_next_1();
      goto top;


    case O_DISP:
      write_se(car(Sreg),stdout); printf("\n");
      code_next_1();
      goto top;

    case O_READ:
      printf(">"); push(read_se(stdin));
      code_next_1();
      goto top;


    case O_ADD:
      arg1=pop(); arg2=pop();
      assert(arg1); assert(arg2);
      assert(type(arg1)==NUM && type(arg2)==NUM);
      push(new_num(numval(arg1)+numval(arg2)));      
      code_next_1();
      destroy(arg1);
      destroy(arg2);
      goto top;

    case O_SUB:
      arg1=pop(); arg2=pop();
      assert(arg1); assert(arg2);
      assert(type(arg1)==NUM && type(arg2)==NUM);
      push(new_num(numval(arg1)-numval(arg2)));      
      code_next_1();
      destroy(arg1);
      destroy(arg2);
      goto top;

    case O_MUL:
      arg1=pop(); arg2=pop();
      assert(arg1); assert(arg2);
      assert(type(arg1)==NUM && type(arg2)==NUM);
      push(new_num(numval(arg1)*numval(arg2)));      
      code_next_1();
      destroy(arg1);
      destroy(arg2);
      goto top;

    case O_DIV:
      arg1=pop(); arg2=pop();
      assert(arg1); assert(arg2);
      assert(type(arg1)==NUM && type(arg2)==NUM);
      assert(numval(arg2)!=0); /* :) */
      push(new_num(numval(arg1)/numval(arg2)));
      code_next_1();
      destroy(arg1);
      destroy(arg2);
      goto top;

    case O_MOD:
      arg1=pop(); arg2=pop();
      assert(arg1); assert(arg2);
      assert(type(arg1)==NUM && type(arg2)==NUM);
      assert(numval(arg2)!=0); /* (: */
      push(new_num(numval(arg1)+numval(arg2)));      
      code_next_1();
      destroy(arg1);
      destroy(arg2);
      goto top;

    case O_GT:
      arg1=pop(); arg2=pop();
      assert(arg1); assert(arg2);
      assert(type(arg1)==NUM && type(arg2)==NUM);
      if(numval(arg1)>numval(arg2)) return T_ATOM;
      else return NIL;
      code_next_1();
      destroy(arg1);
      destroy(arg2);
      goto top;

    case O_LT:    
      arg1=pop(); arg2=pop();
      assert(arg1); assert(arg2);
      assert(type(arg1)==NUM && type(arg2)==NUM);
      if(numval(arg1)<numval(arg2)) return T_ATOM;
      else return NIL;
      code_next_1();
      destroy(arg1);
      destroy(arg2);
      goto top;

    default: assert(0==1); /* się nie zesraj... */
    }
  }
  return pop();
}


/**************************************************************************************/

/* acha, one - tj te operacje na 'rejestrach' - zasadniczo mają osadzone w sobie odśmiecanie;
   choć pewne rzeczy są czyszczone po stronie opkodów. nie jest to jakaś asymetria, a jedynie
   dzielenie się obowiązkami - pewne rzeczy usuwa się z pewnością, inne tylko okazjonalnie... */

/* 'defensywne' asserty warto mieć... i jeszcze taka mała uwaga: na razie się nie sugeruj tym,
   że te procedury będą inline'owane - jak wszystko zabanlga to się to wygładzi. Zresztą narzut
   związany z wywołaniami teraz chuj mję boly, takie smaczki to na jakiś gorszy dzień sobie można... */

void push(SE *e) {
  Sreg=new_cons(e,Sreg);
}

SE *pop() {
  SE *tmp=Sreg,*top; // register??
  assert(tmp);
  top=car(tmp);
  Sreg=cdr(tmp);
  destroy(tmp);
  return top;
}

/* przy env_* pewnie by warto obczajać czy key nie wykracza poza obsza zaalokowany ale no, w sumie
 jeśli kompilacja przebiega poprawnie to się nigdy nie stanie. w zasadzie z tego samego powodu można
 by sobie darować te asserty, ale to na razie tak, do debugu... */

void env_add(int key,SE *e) {
  Ereg[key]=new_cons(e,Ereg[key]);
}

void env_rem(int key) {
  SE *tmp=Ereg[key]; // register?
  assert(tmp);
  Ereg[key]=cdr(tmp);
  destroy_rec(car(tmp));
  destroy(tmp);
}

SE *env_lookup(int key) {
  assert(Ereg[key]); /* a nie trzeba sprawdzać czy jest consem - jeśli nie jest nilem to wyboru nie ma (zob. env_add). */
  return clone_rec(car(Ereg[key])); /* patrzcie Państwo, oto słynne "copy" z "copy'n'swep"! :) */
}

void dump() {
  Dreg=new_cons(Creg,Dreg);
}

void redump() {
  SE *tmp=Dreg; // register?
  assert(tmp);
  Dreg=cdr(tmp);
  Creg=car(tmp);
  destroy(tmp);
}

void set_code(SE *newcode) { // czy w ogóle ktoś go jeszcze używa? ;) sel może...
  Creg=newcode;
}

/* te cuda "przesuwają taśmę z kodem" usuwając przy okazji zbyteczną strukturę wagoników;
   proszę zwrócić uwagę że code_next_2 i code_next_3 nie usuwają arugmentów występujących
   w kodzie - bo prócz BIND, UNBIND i LU są one potrzebne. a trzech wspomnianych dżentelmenów
   radzi sobie dzielnie po swojej stronie poprzez destroy'e. */

void code_next_1() {
  SE *tmp=Creg; /* [op . C] */
  Creg=cdr(tmp); /* C */
  destroy(car(tmp)); /* op */
  destroy(tmp);      /* {op . C} */
}

void code_next_2() {
  SE *tmp=Creg; /* [op . [arg1 . C]] */
  Creg=cdr(cdr(tmp)); /* C */
  destroy(car(tmp)); /* op */
  destroy(cdr(tmp)); /* {arg1 . C} */
  destroy(tmp); /* {op . [arg1 . C]} */
}

void code_next_3() {
  SE *tmp=Creg; /* [op . [arg1 . [arg2 . C]]] */
  Creg=cdr(cdr(cdr(tmp))); /* C */
  destroy(car(tmp)); /* op */
  destroy(cdr(cdr(tmp))); /* {arg2 . C} */
  destroy(cdr(tmp)); /* {arg1 . [arg2 . C]} */
  destroy(tmp); /* {op . [arg1 . [arg2 . C]]} */
}
