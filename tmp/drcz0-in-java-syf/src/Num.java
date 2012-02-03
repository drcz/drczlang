/* Numerały */

public class Num extends SExp {
    private double num;
    public Num(double n) { type=NUM; num=n; }
    public boolean isNumber() { return true; }
    public boolean isAtom() { return true; }
    public double getNum() { return num; }

    public String toString() { return Double.toString(num); }
}