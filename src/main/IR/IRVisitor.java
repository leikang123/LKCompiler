package main.IR;

import main.AST.ReturnNode;
import main.AST.SwitchNode;
import main.assembly.Label;

public interface IRVisitor<S,E> {
    public S visit(ExprStmt s);
    public S visit(Assign s);
    public S visit(CJump s);
    public S visit(Jump s);
    public S visit(SwitchNode s);
    public S visit(Label s);
    public S visit(ReturnNode s);

    public E visit(Uni s);
    public E visit(Bin s);
    public E visit(Call s);
    public E visit(Addr s);
    public E visit(Mem s);
    public E visit(Var s);
    public E visit(Int s);
    public E visit(Str s);
}
