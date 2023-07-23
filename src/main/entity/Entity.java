package main.entity;
import main.type.*;
import main.AST.TypeNode;
import main.AST.Location;
import main.AST.ExprNode;
import main.Assembly.Symbol;
import main.Assembly.Operand;
import main.Assembly.MemoryReference;
import main.Assembly.ImmediateValue;

public  abstract class Entity
        implements net.loveruby.cflat.ast.Dumpable {
    protected String name;
    protected boolean isPrivate;
    protected TypeNode typeNode;
    protected long nRefered;
    protected MemoryReference memref;
    protected Operand address;

    public Entity(boolean priv, TypeNode type, String name) {
        this.name = name;
        this.isPrivate = priv;
        this.typeNode = type;
        this.nRefered = 0;
    }

    public String name() {
        return name;
    }

    public String symbolString() {
        return name();
    }

 public   abstract  boolean isDefined();
  public   abstract  boolean isInitialized();

    public boolean isConstant() { return false; }

    public ExprNode value() {
        throw new Error("Entity#value");
    }

    public boolean isParameter() { return false; }

    public boolean isPrivate() {
        return isPrivate;
    }

    public TypeNode typeNode() {
        return typeNode;
    }

    public Type type() {
        return typeNode.type();
    }

    public long allocSize() {
        return type().allocSize();
    }

    public long alignment() {
        return type().alignment();
    }

    public void refered() {
        nRefered++;
    }

    public boolean isRefered() {
        return (nRefered > 0);
    }

    public void setMemref(MemoryReference mem) {
        this.memref = mem;
    }

    public MemoryReference memref() {
        checkAddress();
        return memref;
    }

    public void setAddress(MemoryReference mem) {
        this.address = mem;
    }

    public void setAddress(ImmediateValue imm) {
        this.address = imm;
    }

    public Operand address() {
        checkAddress();
        return address;
    }

    protected void checkAddress() {
        if (memref == null && address == null) {
            throw new Error("address did not resolved: " + name);
        }
    }

    public Location location() {
        return typeNode.location();
    }

    public abstract <T> T accept(EntityVisitor<T> visitor);

    public void dump(net.loveruby.cflat.ast.Dumper d) {
        d.printClass(this, location());
        _dump(d);
    }

    protected abstract void _dump(net.loveruby.cflat.ast.Dumper d);
                
}
