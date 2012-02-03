/* byćmoże kiedyś ktoś (na pewno nie ja) napisze porządny parser, np
   w oparciu o java.util.Scanner, ,,albo co jeszcze gorszego'', taki który
   nie boi się znaków nowej linii i bimba sobie na preparser w C...

   a oficjalnie to dla szaleńców którzy woleliby np nawiasy kwadratowe
   miast okrągłych, albo operatory infiksowe, albo inne zdrożności. */

public interface Parser {
    public SExp parse(String s) throws Exception;
    public String toString(SExp e) throws Exception;
    public SExp loadSExp(String filename) throws Exception;
    public void saveSExp(String filename, SExp e) throws Exception;
    public SExp read() throws Exception;
}