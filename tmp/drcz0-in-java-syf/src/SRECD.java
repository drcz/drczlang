/* maszyna abstrakcyjna SrECD 

   jak każda implementacja SrECD dedykowana pamięci Petera Landina (1930-2009). */

import java.util.*;

public class SRECD {

    /*** stan maszyny ('rejestry' S, E, C i D) ***/

    private SExp Stack = null;
    private SExp Code = null;
    private SExp Dump = null;
    private HashMap<String,SExp> Env = new HashMap<String,SExp>();

    /*** słownik pierwotnych operatorów (PrimOpów) maszyny: ***/

    private HashMap<String,PrimitiveProcedureSRECD> primOps = new HashMap<String,PrimitiveProcedureSRECD>();


    /*** operacje pomocniczne na 'rejestrach' S, E, C i D: ***/

    private void push(SExp e) throws Exception { Stack=new Cons(e,Stack); }
    private SExp pop() throws Exception { SExp hd=Stack.getCar(); Stack=Stack.getCdr(); return hd; }

    private void dump() throws Exception { Dump=new Cons(Code,Dump); }
    private void redump() throws Exception { Code=Dump.getCar(); Dump=Dump.getCdr(); }

    private void envAdd(String sym,SExp e) throws Exception {
	sym=sym.toLowerCase();
	Env.put(sym,new Cons(e,Env.get(sym)));
    }

    private void envRem(String sym) throws Exception {
	sym=sym.toLowerCase();
	if(Env.get(sym)==null) return;
	Env.put(sym,Env.get(sym).getCdr());
    }
    private SExp envGet(String sym) throws Exception {
	sym=sym.toLowerCase();
	return Env.get(sym).getCar();
    }

    private void setCode(SExp c) throws Exception { Code=c; }


    /*** konstruktor i jego pomocnicy: ***/

    public SRECD(SExp s,SExp c) throws Exception { /* konstruktor zwracający wyjątki mmm, miodzio :) */

	Stack=s;
	Code=c;
	envAdd("T",new Sym("T")); /* nie jest to jakiś skandal, ale uczulam... */
	addDefaultPrimOps();
	addArithmPrimOps();
    }

    /* standardowe operatory lispowe, uniwersalne dla wszystkich SExpów: */
    public void addDefaultPrimOps() {

	primOps.put("NUM", new PrimitiveProcedureSRECD() {
		public int getArity() { return 1; }
		public SExp execute(SExp[] args) throws Exception{
		    if(args[0]!=null && args[0].isNumber()) return new Sym("T");
		    else return null;
		}
	    });

	primOps.put("ATOM", new PrimitiveProcedureSRECD() {
		public int getArity() { return 1; }
		public SExp execute(SExp[] args) throws Exception{
		    if(args[0]==null || args[0].isAtom()) return new Sym("T");
		    else return null;
		}
	    });

	primOps.put("CAR", new PrimitiveProcedureSRECD() {
		public int getArity() { return 1; }
		public SExp execute(SExp[] args) throws Exception{
		    return args[0].getCar();
		}
	    });

	primOps.put("CDR", new PrimitiveProcedureSRECD() {
		public int getArity() { return 1; }
		public SExp execute(SExp[] args) throws Exception{
		    return args[0].getCdr();
		}
	    });

	primOps.put("CONS", new PrimitiveProcedureSRECD() {
		public int getArity() { return 2; }
		public SExp execute(SExp[] args) throws Exception{
		    return new Cons(args[0],args[1]);
		}
	    });

	primOps.put("EQ", new PrimitiveProcedureSRECD() {
		public int getArity() { return 2; }
		public SExp execute(SExp[] args) throws Exception{
		    if(SExp.areEqual(args[0],args[1])) return new Sym("T");
 		     else return null;
		}
	    });
    }

    /* plus operatory arytmetyczne... */
    public void addArithmPrimOps() {

	primOps.put("ADD", new PrimitiveProcedureSRECD() {
		public int getArity() { return 2; }
		public SExp execute(SExp[] args) throws Exception{
		    return new Num(args[0].getNum() + args[1].getNum());
		}
	    });

	primOps.put("SUB", new PrimitiveProcedureSRECD() {
		public int getArity() { return 2; }
		public SExp execute(SExp[] args) throws Exception{
		    return new Num(args[0].getNum() - args[1].getNum());
		}
	    });

	primOps.put("MUL", new PrimitiveProcedureSRECD() {
		public int getArity() { return 2; }
		public SExp execute(SExp[] args) throws Exception{
		    return new Num(args[0].getNum() * args[1].getNum());
		}
	    });

	primOps.put("DIV", new PrimitiveProcedureSRECD() {
		public int getArity() { return 2; }
		public SExp execute(SExp[] args) throws Exception{
		    return new Num(args[0].getNum() / args[1].getNum());
		}
	    });
	primOps.put("MOD", new PrimitiveProcedureSRECD() {
		public int getArity() { return 2; }
		public SExp execute(SExp[] args) throws Exception{
		    return new Num((int)args[0].getNum() % (int)args[1].getNum()); /* uwaga, rzutowanie do int! */
		    /* tak w zasadzie to można było sobie ten operator podarować,
		       bardzo pięknie można go wyrazić za pomocą SUB i GT, ale jak już
		       w historycznej implementacji języka był, to trudno... */
		}
	    });

	primOps.put("LT", new PrimitiveProcedureSRECD() {
		public int getArity() { return 2; }
		public SExp execute(SExp[] args) throws Exception{
		    if(args[0].getNum() < args[1].getNum()) return new Sym("T");
		    else return null;
		}
	    });

	primOps.put("GT", new PrimitiveProcedureSRECD() {
		public int getArity() { return 2; }
		public SExp execute(SExp[] args) throws Exception{
		    if(args[0].getNum() > args[1].getNum()) return new Sym("T");
		    else return null;
		}
	    });
    }

    public void addPrimOp(String name, PrimitiveProcedureSRECD impl) { primOps.put(name,impl); }

    /*** serce maszyny: transformacja <S,E,C,D> -> <S',E',C',D'> : ***/
    /* Obok każdego warunku rozpoznania opkodu zapisano regułę transformacji która ma zostać
       zrealizowana; wielki litery oznaczają stany rejestrów przed transformacją (i konstytuują
       symbole tj. c.liter poprzedzone pojedynczym cudzysłowem), małe zaś to 'zmienne syntaktyczne'
       (ang. placeholders); dwa [infiksowe] operatory kropki "( . )" i dwukropka  ":"  to konstruktor
       wagonika (CONS) i konkatenacja list (APPEND) odpowiednio.

       [ nb. APPEND nie jest u nas op.pierwotnym, ze wzgledu na prostotę jego implementacji
       [     za pomocą SEL, EQ, LDC i CONS, wg. reguły:
       [     
       [     APPEND(a,b) = b, gdy a jest lista pusta, 
       [                 = CONS(x,APPEND(y,b)) gdy a jest wagonikiem (x.y)
       
       Notacja E{x->a} oznacza tu rozszerzenie odwzorowania E, tj.A

              E{x->a}(s) = a jeśli x=s [chodzi o 'identyczność syntaktyczną']
                         = E(s) w przeciwnym razie.                                                  */

    public SExp run() throws Exception {

	while(Code!=null) {

	    String code=Code.getCar().getSym();
	    if(code.equals("LDC")) {  /* <S,E,(LDC x)::C,D> -> <(x.S),E,C,D> */
		push(Code.getCdr().getCar()); 
		Code=Code.getCdr().getCdr();

	    } else if(code.equals("LU")) {  /* <S,E{x->a},('LU x)::C,D> -> <(a.S),E{x->a},C,D> */
		push(envGet(Code.getCdr().getCar().getSym()));
		Code=Code.getCdr().getCdr();

	    } else if(code.equals("BIND")) {  /* <(a.S),E,('BIND x)::C,D> -> <S,E{x->a},C,D> */
		envAdd(Code.getCdr().getCar().getSym(),pop()); 
		Code=Code.getCdr().getCdr();

	    } else if(code.equals("UNBIND")) {  /* <S,E{x->a},('UNBIND x)::C,D> -> <S,E,C,D> */
		envRem(Code.getCdr().getCar().getSym()); 
		Code=Code.getCdr().getCdr();

	    } else if(code.equals("SEL")) {
		/* <(x.S),E,('SEL tb fb):C,D> -> <S,E,tb,(C.D)> jesli x jest różny od listy pustej,
		                         lub  -> <S,E,fb,(C.D)> w przeciwnym razie. */
		SExp tb=Code.getCdr().getCar();
		SExp fb=Code.getCdr().getCdr().getCar();
		Code=Code.getCdr().getCdr().getCdr();
		dump();
		if(pop()==null) setCode(fb); else setCode(tb);

	    } else if(code.equals("SER")) {
		/* <(x.S),E,('SER tb fb):C,D> -> <S,E,tb,D> jesli x jest różny od listy pustej,
		                          lub -> <S,E,fb,D> w przec.razie. */
		if(pop()==null) setCode(Code.getCdr().getCdr().getCar());
		else setCode(Code.getCdr().getCar());

	    } else if(code.equals("AP")) {  /* <(f.S),E,('AP.C),D> -> <S,E,f,(C.D)> */
		Code=Code.getCdr();
		dump();
		setCode(pop());

	    } else if(code.equals("TRAP")) {  /* <(f.S),E,('TRAP.C),D> -> <S,E,f,D> */
		setCode(pop());

	    } else if(code.equals("RTN")) {  /* <S,E,('RTN),(C.D)> -> <S,E,C,D> */
		redump();

	    } else {
		PrimitiveProcedureSRECD p=primOps.get(code);
		if(p==null) throw new Exception("unknown opcode  '"+code+"'");
		int arity=p.getArity();
		SExp[] args=new SExp[arity];
		for(int i=0;i<arity;i++) args[i]=pop();
		push(p.execute(args));
		Code=Code.getCdr();
	    }
	} /* while(Code!=null... */

	return Stack.getCar();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* alternatywny trzon maszyny ("silnik trybu odrobaczania"):
       czasem może się okazać, że chcemy logować wszystkie kroki maszyny - tu idą na stderr.
       działa niemiłosiernie wolno, nawet już mniejsza o konkatenacje Stringów, ale każdorazowy
       przelot przez env i parsowanie, no... trochę waży. ale w trakcie pisania tej maszyny był
       nieoceniony.
       duplikacja kodu z run() jest tu uzasadniona, czasem chce się dodać jakieś dodatkowe
       wyjście przy określonym opkodzie np. - żeby nie zabrudzać run() możemy to śmiało robić tu. */
    public SExp dbgRun() throws Exception {

	Parser pr=new DefaultParser();
	int krok=0;

	while(Code!=null) {
	    ////////////////////////////////////////////////////////////////////////////////
	    krok++;
	    System.err.println("--krok "+krok+"--------------------------------------------------------------------");
	    System.err.println("--S:"+pr.toString(Stack));
	    System.err.println("--E:{");
	    for(String klucz : Env.keySet()) System.err.println("   "+klucz+" -> "+pr.toString(Env.get(klucz)));
	    System.err.println("}");
	    System.err.println("--C:"+pr.toString(Code));
	    System.err.println("--D:"+pr.toString(Dump)+"\n");
	    ////////////////////////////////////////////////////////////////////////////////
	    String code=Code.getCar().getSym();
	    if(code.equals("LDC")) {  /* <S,E,(LDC x)::C,D> -> <(x.S),E,C,D> */
		push(Code.getCdr().getCar()); 
		Code=Code.getCdr().getCdr();

	    } else if(code.equals("LU")) {  /* <S,E{x->a},('LU x)::C,D> -> <(a.S),E{x->a},C,D> */
		push(envGet(Code.getCdr().getCar().getSym()));
		Code=Code.getCdr().getCdr();

	    } else if(code.equals("BIND")) {  /* <(a.S),E,('BIND x)::C,D> -> <S,E{x->a},C,D> */
		envAdd(Code.getCdr().getCar().getSym(),pop()); 
		Code=Code.getCdr().getCdr();

	    } else if(code.equals("UNBIND")) {  /* <S,E{x->a},('UNBIND x)::C,D> -> <S,E,C,D> */
		envRem(Code.getCdr().getCar().getSym()); 
		Code=Code.getCdr().getCdr();

	    } else if(code.equals("SEL")) {
		/* <(x.S),E,('SEL tb fb):C,D> -> <S,E,tb,(C.D)> jesli x jest różny od listy pustej,
		                         lub  -> <S,E,fb,(C.D)> w przeciwnym razie. */
		SExp tb=Code.getCdr().getCar();
		SExp fb=Code.getCdr().getCdr().getCar();
		Code=Code.getCdr().getCdr().getCdr();
		dump();
		if(pop()==null) setCode(fb); else setCode(tb);

	    } else if(code.equals("SER")) {
		/* <(x.S),E,('SER tb fb):C,D> -> <S,E,tb,D> jesli x jest różny od listy pustej,
		                          lub -> <S,E,fb,D> w przec.razie. */
		if(pop()==null) setCode(Code.getCdr().getCdr().getCar());
		else setCode(Code.getCdr().getCar());

	    } else if(code.equals("AP")) {  /* <(f.S),E,('AP.C),D> -> <S,E,f,(C.D)> */
		Code=Code.getCdr();
		dump();
		setCode(pop());

	    } else if(code.equals("TRAP")) {  /* <(f.S),E,('AP.C),D> -> <S,E,f,D> */
		setCode(pop());

	    } else if(code.equals("RTN")) {  /* <S,E,('RTN),(C.D)> -> <S,E,C,D> */
		redump();

	    } else {
		PrimitiveProcedureSRECD p=primOps.get(code);
		if(p==null) throw new Exception("unknown opcode  '"+code+"'");
		int arity=p.getArity();
		SExp[] args=new SExp[arity];
		for(int i=0;i<arity;i++) args[i]=pop();
		push(p.execute(args));
		Code=Code.getCdr();
	    }
	} /* while(Code!=null... */

	return Stack.getCar();
    }
}
