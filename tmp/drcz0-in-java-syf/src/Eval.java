/* naiwny ewaluator dla drcz0, wraz z interpreterem REPL.

   Dedykowany pamięci John McCarthy'ego (1927-2011). */

import java.util.*;
import java.io.*; // ...

public class Eval {

    /* 'symbole stałe': */
    private SExp tSym = new Sym("T");
    private SExp quoteSym = new Sym("quote");
    private SExp lambdaSym = new Sym("^");
    private SExp ifSym = new Sym("if");
    private SExp defSym = new Sym("!");
    private SExp undefSym = new Sym("#UNDEFINED");
    /* słownik dla operatorów pierwotnych: */
    private HashMap<String,PrimitiveProcedure> primOps = new HashMap<String,PrimitiveProcedure>();

    /* a tak, konstruktor: */
    public Eval() {	
	addDefaultPrimOps();
	addArithmPrimOps();       
    }

    /* dodawanie nowych operatorów pierwotnych: */
    public void addPrimOp(String name,PrimitiveProcedure impl) { primOps.put(name,impl); }

    /* ewaluator: */
    public SExp eval(SExp expr,HashMap<String,SExp> env) {
	try {
	    if(isConstant(expr)) return expr;

	    else if(isVariable(expr)) return lookup(expr,env);

	    else if(isQuoteForm(expr)) return expr.getCdr().getCar();

	    else if(isLambdaForm(expr)) return expr;

	    else if(isIfForm(expr)) {
		SExp cond = expr.getCdr().getCar();
		SExp tbranch = expr.getCdr().getCdr().getCar();
		SExp fbranch = expr.getCdr().getCdr().getCdr().getCar();
		if(eval(cond,env)==null) return eval(fbranch,env);
		else return eval(tbranch,env);

	    } else return apply(expr.getCar(),evlis(expr.getCdr(),env),env);	    

	} catch(Exception e) {
	    e.printStackTrace();
	    return undefSym;
	}
    }

    /* interpreter REPL: wczytuje wyrażenie, ewaluuje w bieżącym środowisku i wyświetla na stdin, a
       dodatkowo obsługuje formy definicyjne - tj rządania zmiany środowiska (np forma "(! a 5)"
       jest rządaniem o dodanie w środowisku symbolu "a" i przyporządkowanie mu wyrażenia będącego
       wartością wyrażenia "5").

       dodatkową specjalną formą, doklejoną praktycznie pozajęzykowo, jest forma "(load <plik>)"- otwiera
       ona plik o zadanej nazwie i realizuje na nim pętlę REPL; po zakończeniu pamiętając definicje
       wyczytane z pliku. taki myk.

       < dla wygody debugowania obsługuje też -komendę- "_showenv_" wypisującą bieżący stan środowiska >
     */
    public void runREPL(Parser parser, HashMap<String,SExp> initialEnv) {
	HashMap<String,SExp> env = initialEnv;
	while(true) {
	    try {
		SExp inputExpr = parser.read();

		if(inputExpr!=null && inputExpr.isSymbol() && inputExpr.getSym().equals("_showenv_")) {
		    System.out.println("---------------------------------------------------------------------------");
		    for(String k : env.keySet()) System.out.println(" "+k+" -> "+env.get(k));
		    System.out.println("---------------------------------------------------------------------------");

		} else if(inputExpr!=null && inputExpr.isCons() && inputExpr.getCar().isSymbol()
			  && inputExpr.getCar().getSym().equals("load")) {
		    String plik=inputExpr.getCdr().getCar().getSym();
		    // teraz wykorzystamy (niemalże bezczelnie) dżawoski kol-baj-nejm:
		    runREPLplik(plik,parser,env);
		    // no, i tu env zawiera wszystkie nowe symbole z pliku (to jest magia kina!)

		} else if(isDefineForm(inputExpr)) {
		    String definiens = inputExpr.getCdr().getCar().getSym();
		    SExp definiendum = eval( inputExpr.getCdr().getCdr().getCar() ,env);
		    env=updateEnv(definiens,definiendum,env);
		    System.out.println("Dodano nowy symbol do środowiska ("+definiens+").");

		} else {
		    System.out.println(parser.toString(eval(inputExpr,env)));
		}

	    } catch(Exception e) {		
		e.printStackTrace();
	    }
	}	

    }


    /* duplikowanie się kodu rzadko cieszy oko, jednak takie rozgraniczenie jawi się zasadnym - czytając
       plik możemy chcieć w przyszłości inaczej traktować pewne dyrektywy. obecnie jedynie przyjmujemy,
       że z poziomu pliku nie możemy ładować innych plików i nie możemy odpytywać o stan środowiska. */

    public void runREPLplik(String plikName, Parser parser, HashMap<String,SExp> initialEnv) throws Exception {
	HashMap<String,SExp> env = initialEnv;
	File file = new File(plikName);
	StringBuffer sexpAsString = new StringBuffer();
	BufferedReader reader = new BufferedReader(new FileReader(file));
	String line = null;
	
	while(true) {
	    try {
		line = reader.readLine();
		if(line==null) break; // <--- exit :)
		SExp inputExpr = parser.parse(line);

		if(isDefineForm(inputExpr)) {
		    String definiens = inputExpr.getCdr().getCar().getSym();
		    SExp definiendum = eval( inputExpr.getCdr().getCdr().getCar() ,env);
		    env=updateEnv(definiens,definiendum,env);
		    System.out.println("Dodano nowy symbol do środowiska ("+definiens+").");
		} else {
		    System.out.println(parser.toString(eval(inputExpr,env)));
		}

	    } catch(Exception e) {		
		e.printStackTrace();
	    }
	}	

    }



    /**********************************************************************************************/

    /* klasyfikowanie wyrażeń do kategorii semantycznych */
    private boolean isConstant(SExp e) throws Exception {
	if(e==null) return true; // tak, nil czyli () to przecież też stała.
	if(!e.isAtom() || e.isSymbol()) return false;
	return true;
    }

    private boolean isVariable(SExp e) throws Exception {
	if(e!=null && e.isSymbol()) return true;
	return false;
    }

    private boolean isQuoteForm(SExp e) throws Exception {
	if(e==null || !e.isCons()) return false;
	SExp hd=e.getCar();
	if(quoteSym.equals(hd)) return true;
	return false;
    }

    private boolean isLambdaForm(SExp e) throws Exception {
	if(e==null || !e.isCons()) return false;
	SExp hd=e.getCar();
	if(lambdaSym.equals(hd)) return true;
	return false;
    }

    private boolean isIfForm(SExp e) throws Exception {
	if(e==null || !e.isCons()) return false;
	SExp hd=e.getCar();
	if(ifSym.equals(hd)) return true;
	return false;
    }

    private boolean isDefineForm(SExp e) throws Exception {
	if(e==null || !e.isCons()) return false;
	SExp hd=e.getCar();
	if(defSym.equals(hd)) return true;
	return false;
    }

    private boolean isPrimOp(SExp e) throws Exception {
	if(e!=null && e.isSymbol() && primOps.get(e.getSym())!=null) return true;
	return false;
    }

    /* manipulacje środowiskiem: */
    private SExp lookup(SExp expr,HashMap<String,SExp> env) throws Exception {
	if(expr==null || !expr.isSymbol()) return null;
	// return env.get(expr.getSym()); // <--- zaczarowany na ,,Case Insensytywny'' :)
	return env.get(expr.getSym().toUpperCase());
    }

    private HashMap<String,SExp> updateEnv(String sym,SExp val,HashMap<String,SExp> env) {
	// env.put(sym,val); // <--- ,,Case Insensytywność'' zabieg 2.
	env.put(sym.toUpperCase(),val);
	return env;
    }

    /* metody pomocnicze dla eval: */
    private SExp evlis(SExp list,HashMap<String,SExp> env) throws Exception {
	if(list==null) return null;
	if(!list.isCons()) return undefSym; /* #UNDEFINED */
	return new Cons(eval(list.getCar(),env),evlis(list.getCdr(),env));
    }

    private SExp apply(SExp rator,SExp rands, HashMap<String,SExp> env) throws Exception {
	if(rator==null || isConstant(rator)) return undefSym; /* #UNDEFINED */
	if(isPrimOp(rator)) return applyPrimOp(rator,rands);
	if(isLambdaForm(rator)) return evLambda(rator,rands,env);
	return apply(eval(rator,env),rands,env);
    }

    @SuppressWarnings("unchecked") /* bo rzutując Object do HashMap wiem co robię. */
    SExp evLambda(SExp lambdaForm,SExp args,HashMap<String,SExp> env) throws Exception {
	SExp argList = lambdaForm.getCdr().getCar();
	SExp body = lambdaForm.getCdr().getCdr().getCar();
	HashMap<String,SExp> newEnv = (HashMap<String,SExp>)env.clone();
	try {
	    while(argList!=null) {
		String var = argList.getCar().getSym();
		SExp val = args.getCar();
		argList = argList.getCdr();
		args = args.getCdr();
		newEnv = updateEnv(var,val,newEnv);
	    }
	return eval(body,newEnv);
	} catch(Exception e) {
	    return undefSym; /* #UNDEFINED */
	}
    }

    private SExp applyPrimOp(SExp rator,SExp rands) throws Exception {
	PrimitiveProcedure p=primOps.get(rator.getSym());
	if(p==null) return undefSym; /* #UNDEFINED */	
        return p.execute(rands);
    }


    /* standardowe operatory lispowe, uniwersalne dla wszystkich SExpów: */
    public void addDefaultPrimOps() {

	primOps.put("#", new PrimitiveProcedure() { // num
		public SExp execute(SExp args) throws Exception{
		    if(args.getCar()!=null && args.getCar().isNumber()) return tSym;
		    else return null;
		}
	    });

	primOps.put("@", new PrimitiveProcedure() { // atom
		public SExp execute(SExp args) throws Exception{
		    if(args.getCar()==null || args.getCar().isAtom()) return tSym;
		    else return null;
		}
	    });

	primOps.put(".", new PrimitiveProcedure() { // car
		public SExp execute(SExp args) throws Exception{
		    return args.getCar().getCar();
		}
	    });

	primOps.put(",", new PrimitiveProcedure() { // cdr
		public SExp execute(SExp args) throws Exception{
		    return args.getCar().getCdr();
		}
	    });

	primOps.put(";", new PrimitiveProcedure() { // cons
		public SExp execute(SExp args) throws Exception{
		    return new Cons(args.getCar(),args.getCdr().getCar());
		}
	    });

	primOps.put("=", new PrimitiveProcedure() { // eq
		public SExp execute(SExp args) throws Exception{
		    if(SExp.areEqual(args.getCar(),args.getCdr().getCar())) return tSym;
 		     else return null;
		}
	    });
    }

    /* plus operatory arytmetyczne... */
    public void addArithmPrimOps() {

	primOps.put("+", new PrimitiveProcedure() {
		public SExp execute(SExp args) throws Exception{
		    return new Num(args.getCar().getNum() + args.getCdr().getCar().getNum());
		}
	    });

	primOps.put("-", new PrimitiveProcedure() {
		public SExp execute(SExp args) throws Exception{
		    return new Num(args.getCar().getNum() - args.getCdr().getCar().getNum());
		}
	    });

	primOps.put("*", new PrimitiveProcedure() {
		public SExp execute(SExp args) throws Exception{
		    return new Num(args.getCar().getNum() * args.getCdr().getCar().getNum());
		}
	    });

	primOps.put("/", new PrimitiveProcedure() {
		public SExp execute(SExp args) throws Exception{
		    return new Num(args.getCar().getNum() / args.getCdr().getCar().getNum());
		}
	    });
	primOps.put("%", new PrimitiveProcedure() {
		public SExp execute(SExp args) throws Exception{
		    return new Num((int)args.getCar().getNum() % (int)args.getCdr().getCar().getNum());
		                   /* uwaga, rzutowanie do int! */
		}
	    });

	primOps.put("<", new PrimitiveProcedure() {
		public SExp execute(SExp args) throws Exception{
		    if(args.getCar().getNum() < args.getCdr().getCar().getNum()) return tSym;
		    else return null;
		}
	    });

	primOps.put(">", new PrimitiveProcedure() {
		public SExp execute(SExp args) throws Exception{
		    if(args.getCar().getNum() > args.getCdr().getCar().getNum()) return tSym;
		    else return null;
		}
	    });
    }
}
