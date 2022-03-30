package main.AST;
import main.type.TypeRef;
import main.*;

public abstract class LiteralNode extends ExprNode {
    // 对象位置
    protected Location location;
    // 类型节点
    protected TypeNode typeNode;

    public LiteralNode(Location loc, TypeRef ref) {
        super();
        this.location = loc;
        this.typeNode = new TypeNode(ref);
    }

    public Location location() {
        return location;
    }

    public Type type() {
        return typeNode.type();
    }

    public TypeNode typeNode() {
        return typeNode;
    }

    public boolean isConstant() {
        return true;
    }
}
