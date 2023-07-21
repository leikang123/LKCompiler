package main.AST;
import main.type.Type;

 public class  abstract LHSNode extends ExprNode {
    protected Type type, origType;

    public Type type() {
        return type != null ? type : origType();
    }

    public void setType(Type t) {
        this.type = t;
    }

protected abstract  Type origType();

    public long allocSize() { return origType().allocSize(); }

    public boolean isLvalue() { return true; }
    public boolean isAssignable() { return isLoadable(); }

    public boolean isLoadable() {
        Type t = origType();
        return !t.isArray() && !t.isFunction();
    }
}
