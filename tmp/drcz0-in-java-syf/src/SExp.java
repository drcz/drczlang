/* -====== klasa wyrażeń symbolicznych SExp : ======-

           SExp := () | Num | Sym | Cons
            Num := <double>
            Sym := <String bez znaków białych>
           Cons := (SExp . SExp)

   -================================================- */

public class SExp {

    public static final int  NUM=1;
    public static final int  SYM=2;
    public static final int CONS=4;
    public int type;

    public int getType() { return type; }
    public void setType(int t) { type=t; }
    public boolean isAtom() { return false; }
    public boolean isNumber() { return false; }
    public boolean isSymbol() { return false; }
    public boolean isCons() { return false; }
    public double getNum() throws Exception { throw new Exception("to wyrazenie nie jest numeralem!"); }
    public String getSym() throws Exception { throw new Exception("to wyrazenie nie jest symbolem!"); }
    public SExp getCar() throws Exception { throw new Exception("to wyrazenie nie jest wagonikiem!"); }
    public SExp getCdr() throws Exception { throw new Exception("to wyrazenie nie jest wagonikiem!"); }

    public boolean equals(SExp x) {
	try {
	    return areEqual(this,x);
	} catch(Exception ex) {}
	return false;
    }
    
    public static boolean areEqual(SExp a,SExp b) throws Exception {
	if(a==null) {
	    if(b==null) return true;
	    else return false;
	} else {
	    if(b==null) return false;
	    if(a.getType()!=b.getType()) {
		return false;
	    } else {
		switch(a.getType()) {
		 case NUM: return a.getNum()==b.getNum();
		 case SYM: return a.getSym().equalsIgnoreCase(b.getSym());
		 case CONS: return false;
	        }
	    }
	}
	return false; /* notreached? */
    }

    /* te toString to są do debugowania jedynie, do 'przyzwoitego' i/o proszę używać Parserów */
    public String toString() { return "nieznany-typ-wyrazenia"; }

}
