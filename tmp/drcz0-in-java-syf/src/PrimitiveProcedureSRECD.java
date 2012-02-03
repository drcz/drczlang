/* takie rzeczy to chyba tylko w OOP mogą przejść... jakkolwiek to nie wygląda,
   służy jedynie udawaniu w Javie wskaźnika na procedurę. */

public interface PrimitiveProcedureSRECD {
    public int getArity();
    public SExp execute(SExp[] args) throws Exception;
}
