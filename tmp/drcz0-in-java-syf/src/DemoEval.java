/* demonstracja samego naiwnego interpretera REPL...
   nie ogarnia wywołań ogonowych, ale poza tym jest niczego sobie;
   a ładne jest w nim to jak dosłownie realizuje podstawieniowy model obliczeń.
   ----------------------------------------------------------------------------
   2011-12-23 M.J.Stańczyk, drcz@o2.pl */

import java.io.*;
import java.util.*;

public class DemoEval {

    public static void main(String[] args) throws Exception {
 
	final Parser parser=new DefaultParser();
        Eval ewaluator=new Eval();
	
	ewaluator.addPrimOp("read",new PrimitiveProcedure() {
		public SExp execute(SExp args) throws Exception { return parser.read(); }
	    });

	ewaluator.addPrimOp("disp",new PrimitiveProcedure() {
		public SExp execute(SExp args) throws Exception { 
		    System.out.println(parser.toString(args.getCar()));
		    return args.getCar();
		}
	    });

	ewaluator.addPrimOp("quit",new PrimitiveProcedure() {
		public SExp execute(SExp args) throws Exception {
		    System.exit(0);
		    return null; /* notreached :) */
		}
	    });


	System.out.println("(drcz0 w Javie słucha)\n");
	HashMap<String,SExp> env = new HashMap<String,SExp>();
	env.put("T",new Sym("T"));
	ewaluator.runREPL(parser,env);
    }
}