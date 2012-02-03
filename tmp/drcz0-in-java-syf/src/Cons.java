/* Pary kropkowane (,wagoniki') */

public class Cons extends SExp {
    private SExp car,cdr;    
    public Cons(SExp hd,SExp tl) { type=CONS; car=hd; cdr=tl; }
    public boolean isCons() { return true; }
    public SExp getCar() { return car; }
    public SExp getCdr() { return cdr; }

    public String toString() {
	return "["+(car==null?"()":car.toString())+" . "+(cdr==null?"()":cdr.toString())+"]";
    }
}