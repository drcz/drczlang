import java.util.*;

public class DefaultStringTokenizer implements Tokenizer {

    private StringTokenizer tokenizer;

    public DefaultStringTokenizer(String s) {
	tokenizer = new StringTokenizer(s," \n\t()",true);
    }

    public String nextToken() throws Exception {
	String nt = tokenizer.nextToken();
	if(" ".equals(nt)) return nextToken();
	return nt;
    }
}