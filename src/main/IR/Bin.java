package main.IR;
import main.assembly.*;
//二元运算符类继承表达式类 x+y =z
public class Bin extends Expr {
    // 操作符属性
    protected Op op;
    // 左边表达式，右边表达式属性
    protected Expr left, right;

    public Bin(Type type, Op op, Expr left, Expr right) {
        super(type);
        this.op = op;
        this.left = left;
        this.right = right;
    }

    public Expr left() { 
        return left; 
    }
    public Expr right() { 
        return right; 
    }
    public Op op() { 
        return op;
     }

    public <S,E> E accept(IRVisitor<S,E> visitor) {
        return visitor.visit(this);
    }

    protected void _dump(Dumper d) {
        d.printMember("op", op.toString());
        d.printMember("left", left);
        d.printMember("right", right);
    }
}
