package main.IR;
import main.AST.*;

public class ExprStmt extends Stmt {
    protected Expr expr;

    public ExprStmt(Location loc, Expr expr) {
        super(loc);
        this.expr = expr;
    }

    public Expr expr() {
        return expr;
    }

    public <S,E> S accept(IRVisitor<S,E> visitor) {
        return visitor.visit(this);
    }

    protected void _dump(Dumper d) {
        d.printMember("expr", expr);
    }
}
