package main.AST;

 public abstract class StmtNode extends Node {
    protected Location location;

    public StmtNode(Location loc) {
        this.location = loc;
    }

    public Location location() {
        return location;
    }

     public abstract <S,E> S accept(ASTVisitor<S,E> visitor);
}
