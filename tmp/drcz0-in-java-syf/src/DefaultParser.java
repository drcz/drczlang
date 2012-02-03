import java.util.*;
import java.io.*;

/* Ooo, jaki ładny! */

public class DefaultParser implements Parser {

    public String toString(SExp e) throws Exception {
	if(e==null) return "()";
	if(e.isNumber()) return Double.toString(e.getNum());
	if(e.isSymbol()) return e.getSym();
	if(e.isCons()) return "(" + tailToString(e);
	return "[err]";
    }

    public SExp parse(String s) throws Exception {
	Tokenizer stringTokenizer = new DefaultStringTokenizer(s);
	return getExpression(stringTokenizer);
    }

    public SExp read() throws Exception {
	//	Tokenizer streamTokenizer = new DefaultStreamTokenizer();
	final InputStreamReader converter=new InputStreamReader(System.in);
        final BufferedReader in=new BufferedReader(converter);
	Tokenizer streamTokenizer = new DefaultStringTokenizer(in.readLine());
	return getExpression(streamTokenizer);
    }

    // ten load i save to jakiś idiotyzm ziomek, to nie zadziała...

    public SExp loadSExp(String filename) throws Exception {
	File file = new File(filename);
	StringBuffer sexpAsString = new StringBuffer();
	BufferedReader reader = new BufferedReader(new FileReader(file));
	String line = null;
	while((line = reader.readLine()) != null) {
	    sexpAsString.append(line);
	    sexpAsString.append(" ");
	}
	reader.close();
	return parse(sexpAsString.toString());
    }

    public void saveSExp(String filename,SExp e) throws Exception {
	String sexprAsString = toString(e);
	File file = new File(filename);
	Writer output = new BufferedWriter(new FileWriter(file));
	output.write(sexprAsString);
	output.close();
    }    

    /**********************************************************************************************/

    private String tailToString(SExp t) throws Exception {
	if(t==null) return ")";
	if(t.isAtom()) return ". " + toString(t) + ")";		
	return toString(t.getCar()) + ((t.getCdr()==null)?"":" ") + tailToString(t.getCdr());
    }

    private SExp getExpression(Tokenizer st) throws Exception {
        return getExpr(st.nextToken(),st);
    }
    
    private SExp getExpr(String tok,Tokenizer st) throws Exception {
        if(tok.equals("(")) return getTail(st);
        try /* taki maly trajkacz: moze to liczba? */
        {
            double n=Double.parseDouble(tok);
            return new Num(n);
        }catch(NumberFormatException e) { /* ...a wiec symbol! */
             return new Sym(tok);
        }
    }
    
    private SExp getTail(Tokenizer st) throws Exception { 
       SExp hd,tl;
       String tok;
       
       tok=st.nextToken();
       if(tok.equals(")")) return null;
       hd=getExpr(tok,st);
       tl=getTail(st);
       return new Cons(hd,tl);
    }
}
