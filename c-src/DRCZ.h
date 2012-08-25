  /************************************
   **  -= Emulator maszyny DRCZ* =-  **
   ** v1.1 2012-07-24/29, drcz@o2.pl **
   ************************************/

//#define COUNT_NUMBER_OF_STEPS

/* Stan, ale nie Borys. */
SE **Dreg, *Rreg, *Creg;
char silent; /* cf main.c, opcja -s */

/* Wprawia maszynę w ruch, a po jej [ewentualnym] zatrzymaniu zwraca główkę jej stosu (car(Rreg)). */
SE *run();

/* Tak. */
enum opcodes {
  O_CONST, /* ,,numerals&quoted&procs'' */

  O_LOOKUP, O_NAME, O_FORGET, /* ,,variables'' */

  O_SELECT, O_SRELECT, /* ,,conditionals'' */
  O_APPLY, O_TRAPPLY, /* ,,procedure calls'' */

  O_EQ, O_NUM, O_ATOM, O_CAR, O_CDR, O_CONS, /* ,,lisp'' */
  O_GT, O_LT, O_ADD, O_SUB, O_MUL, O_DIV, O_MOD, /* ,,<Z,+,*>'' */
  O_DISP, O_READ, O_LOAD, O_SAVE /* i/o */
};

/* Operacje na Rreg... */
void R_push(SE *);
SE *R_pop();
/* ...na Dreg... */
void D_push(int,SE *);
void D_pop(int);
SE *D_lookup(int);
/* ...i na Creg. */
void C_push(SE *);
void C_pop();
void C_next();

/* mhm. */
#ifdef COUNT_NUMBER_OF_STEPS
unsigned long get_number_of_steps();
#endif

/* Bomba w kwiatki! */
