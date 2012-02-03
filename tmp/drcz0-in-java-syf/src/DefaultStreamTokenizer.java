import java.io.*;

public class DefaultStreamTokenizer implements Tokenizer {

    private StreamTokenizer tokenizer;

    public DefaultStreamTokenizer() {
	tokenizer = new StreamTokenizer(System.in);
    }

    public String nextToken() throws Exception {
	switch(tokenizer.nextToken()) {
	case StreamTokenizer.TT_NUMBER: return Double.toString(tokenizer.nval);
	case StreamTokenizer.TT_WORD: return tokenizer.sval;
	case StreamTokenizer.TT_EOF: throw new TokenizerEOFException();
	}
	return ""; /* "not reached" jak to mówią */
    }
}