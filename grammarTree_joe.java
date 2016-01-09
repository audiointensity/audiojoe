import java.util.Arrays;
import javax.swing.JFrame;
import javax.swing.JPanel;
import java.io.*;

import org.antlr.v4.runtime.ANTLRInputStream;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.tree.ParseTree;
import org.antlr.v4.gui.TreeViewer;

/**
 * A simple demo to show AST GUI with ANTLR
 * @see http://www.antlr.org/api/Java/org/antlr/v4/runtime/tree/gui/TreeViewer.html
 * 
 * @author wangdq
 * 2014-5-24
 *
 */
public class grammarTree_joe {
    public static void main(String[] args) {
        //prepare token stream
	System.out.println(args[0]);
	try {
	    BufferedReader br = new BufferedReader(new FileReader(args[0]));
	    CharStream stream = new ANTLRInputStream(br);
	    joeLexer lexer  = new joeLexer(stream);   
	    TokenStream tokenStream = new CommonTokenStream(lexer);
	    joeParser parser = new joeParser(tokenStream);
	    ParseTree tree = parser.joefile(); 

	    //show AST in console
	    System.out.println(tree.toStringTree(parser));

	    //show AST in GUI
	    JFrame frame = new JFrame("Antlr AST");
	    JPanel panel = new JPanel();
	    TreeViewer viewr = new TreeViewer(Arrays.asList(
							    parser.getRuleNames()),tree);
	    viewr.setScale(1.5);//scale a little
	    panel.add(viewr);
	    frame.add(panel);
	    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	    frame.setSize(200,200);
	    frame.setVisible(true);
	} catch(Exception e) {}
    }
}
