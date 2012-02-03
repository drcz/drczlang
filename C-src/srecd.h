/* Stan (ale nie Borys) */
SE *Sreg;
SE **Ereg;
SE *Creg;
SE *Dreg;

/* wprawia maszynę w ruch, a po jej zatrzymaniu zwraca główkę jej stosu (Sreg) */
SE *run();

enum opcodes {
  O_LDC,

  O_LU,
  O_BIND,
  O_UNBIND,

  O_SEL,
  O_SER,
  O_AP,
  O_TRAP,
  O_RTN,

  O_EQ,
  O_NUM,
  O_ATOM,
  O_CAR,
  O_CDR,
  O_CONS,

  O_DISP,
  O_READ,

  O_ADD,
  O_SUB,
  O_MUL,
  O_DIV,
  O_MOD,
  O_GT,
  O_LT
};

/* tak w sumie to chyba tylko push winien być 'publiczny'... */

/* operacje na Sreg */
void push(SE *);
SE *pop();
/* na Ereg */
void env_add(int,SE *);
void env_rem(int);
SE *env_lookup(int);
/* na Creg */
void set_code(SE *);
/* na Dreg (i Creg, siłą rzeczy) */
void dump();
void redump();
