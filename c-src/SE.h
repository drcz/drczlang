/*
   Reprezentacja Wyrażeń Symbolicznych, subtelna jak granat w kościele.
   Filozoficznie to jedna wielka katastrofa. Czynią zadość poniższej gramatyce:

     SE := Num | Sym | Cons,
   Cons := [ SE . SE ],

   Gdzie przez Num rozumiemy dziedzinę reprezentacji liczb naturalnych,
   przez Sym - dziedzinę skończonych ciągów znaków alfanumerycznych i niektórych
   symboli specjalnych ("+", "-", "*", ";", etc - z wyłączeniem znaków białych).

 */

#ifndef _SE_H_
#define _SE_H_

// #define new(TYPE) (TYPE *)malloc(sizeof(TYPE)) // na pamiątkę...

typedef enum {NUM,SYM,CONS} SEtype;
typedef struct SE SE;

struct SE {
  SEtype type;
  union {
    int num;
    char *str;
    SE *cons[2];
  };
};

#define NIL ((void *)0)

#define type(E) (E)->type
#define numval(E) (E)->num
#define symval(E) (E)->str
#define car(E) (E)->cons[0]
#define cdr(E) (E)->cons[1]

SE *new_num(int);
SE *new_sym(char *);
SE *new_cons(SE *,SE *);

void destroy(SE *);
void destroy_rec(SE *);

SE *clone(SE *);
SE *clone_rec(SE *);

int are_equal(SE *,SE *);

#endif
