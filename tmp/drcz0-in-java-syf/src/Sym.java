/* Symbole */

public class Sym extends SExp {
    private String sym;
    public Sym(String s) { type=SYM; sym=s; }
    public boolean isSymbol() { return true; }
    public boolean isAtom() { return true; }
    public String getSym() { return sym; }

    public String toString() { return sym; }
}