package main.AST;
import main.type.*;

// 二元表达式运算 x+y
public class BinaryOpNode extends ExprNode {
    // 操作符类型
    protected String operator;
    // 表达式节点左边和右边
    protected ExprNode left, right;
    // 类型
    protected Type type;
   // 构造函数带参数
    public BinaryOpNode(ExprNode left, String op, ExprNode right) {
        super();
        this.operator = op;
        this.left = left;
        this.right = right;
    }
    // 构造函数带参数四个
    public BinaryOpNode(Type t, ExprNode left, String op, ExprNode right) {
        super();
        this.operator = op;
        this.left = left;
        this.right = right;
        this.type = t;
    }
   // 操作方法
    public String operator() {
        return operator;
    }
   // 三木表达式
    public Type type() {
        return (type != null) ? type : left.type();
    }
    //setter
    public void setType(Type type) {
        if (this.type != null)
            throw new Error("BinaryOp#setType called twice");
        this.type = type;
    }

    public ExprNode left() {
        return left;
    }

    public void setLeft(ExprNode left) {
        this.left = left;
    }

    public ExprNode right() {
        return right;
    }

    public void setRight(ExprNode right) {
        this.right = right;
    }
    // 返回节点位置对象
    public Location location() {
        return left.location();
    }
    // --dump-ast来输出该节点的dump语法树
    protected void _dump(Dumper d) {
        d.printMember("operator", operator);
        d.printMember("left", left);
        d.printMember("right", right);
    }

    public <S,E> E accept(ASTVisitor<S,E> visitor) {
        return visitor.visit(this);
    }
}
