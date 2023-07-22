package main.IR;
import main.AST.*;

public abstract  class Stmt implements Dumpable {
    protected Location location;

    public Stmt(Location loc) {
        this.location = loc;
    }

    abstract public <S,E> S accept(IRVisitor<S,E> visitor);

    public Location location() {
        return location;
    }

    public void dump(Dumper d) {
        d.printClass(this, location);
        _dump(d);
    }

    public abstract void _dump(Dumper d);
}
