package main.IR;
import main.type.*;
import main.type.Type;
import main.assembly.*;
import main.entity.Entity;

public abstract class Expr implements Dumpable {
    final Type type;

    Expr(Type type) {
        this.type = type;
    }

    public Type type() { return type; }

    public boolean isVar() { return false; }
    public boolean isAddr() { return false; }

    public boolean isConstant() { return false; }

    public ImmediateValue asmValue() {
        throw new Error("Expr#asmValue called");
    }

    public Operand address() {
        throw new Error("Expr#address called");
    }

    public MemoryReference memref() {
        throw new Error("Expr#memref called");
    }

    public Expr addressNode(Type type) {
        throw new Error("unexpected node for LHS: " + getClass());
    }
    
    public Entity getEntityForce() {
        return null;
    }

   public abstract <S,E> E accept(IRVisitor<S,E> visitor);

    public void dump(Dumper d) {
        d.printClass(this);
        d.printMember("type", type);
        _dump(d);
    }

 protected abstract void _dump(Dumper d);
}
