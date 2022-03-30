package main.AST;

public class PrefixOpNode extends UnaryArithmeticOpNode {
    // 构造方法
    public PrefixOpNode(String op, ExprNode expr) {
        super(op, expr);
    }

    public <S,E> E accept(ASTVisitor<S,E> visitor) {
        return visitor.visit(this);
    }
}
