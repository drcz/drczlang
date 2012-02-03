/* demo maszyny SrECD, owocu 7 miesięcy kontemplacji nad ,,Mechanical Evaluation of Expressions'',
   zaimplementowanej w tej biednej, zaściankowej Javie. Koncepcyjnie jest o tyle jasna, że
   w ogóle się nie przejmujemy  */

import java.io.*;

public class DemoSRECD {

    public static void main(String[] args) throws Exception {

	if(args.length!=1) {
	    System.out.println("Uzywaj: java DemoSRECD <skompilowany program>");
	    return;
	}

	final Parser parser=new DefaultParser();
	final InputStreamReader converter=new InputStreamReader(System.in);
        final BufferedReader in=new BufferedReader(converter);
	SExp kod=parser.loadSExp(args[0]);
	System.out.println("Ladujemy plik "+args[0]+" do maszyny i ruszamy!");
        SRECD masz1=new SRECD(null,kod);

	masz1.addPrimOp("READ",new PrimitiveProcedureSRECD() {
		public int getArity() { return 0; }
		public SExp execute(SExp[] args) throws Exception {
		    		    StringBuffer inp = new StringBuffer("");
		    		    //String line = "";
		    		    //while((line = in.readLine()) != null && line.trim().length()>0) inp.append(line);
		    	            //return parser.parse(inp.toString());
				    return parser.parse(in.readLine());
		}
	    });
	masz1.addPrimOp("DISP",new PrimitiveProcedureSRECD() {
		public int getArity() { return 1; }
		public SExp execute(SExp[] args) throws Exception { 
		    System.out.println(parser.toString(args[0]));
		    return args[0];
		}
	    });
	System.out.println("Koniec-bomba, na stosie zostało: "+masz1.run());
    }
}