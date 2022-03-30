package main.AST;
import java.io.PrintStream;

 public abstract class Node implements Dumpable {
    public Node() {
    }
   // 返回在某节点所对应的语法在代码中的位置方法
    public abstract Location location();
   // 文本形式表示抽象语法树的方法
    public void dump() {
        dump(System.out);
    }
  //输出语法树的方法，调用这个方法打印语法树
    public void dump(PrintStream s) {
        dump(new Dumper(s));
    }

    public void dump(Dumper d) {
        d.printClass(this, location());
        _dump(d);
    }

     protected abstract void _dump(Dumper d);
}
