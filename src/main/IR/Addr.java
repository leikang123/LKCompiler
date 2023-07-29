package main.IR;


import main.assembly.MemoryReference;
import main.assembly.Operand;
import main.assembly.Type;
import main.entity.Entity;

// 取地址类 &
public class Addr extends Expr {
    // 实体类属性定义
    Entity entity;

    public Addr(Type type, Entity entity) {
        super(type);
        this.entity = entity;
    }

    public boolean isAddr() { 
        return true; 
    }

    public Entity entity() { 
        return entity; 
    }

    public Operand address() {
        return entity.address();
    }
    //内存引用
    public MemoryReference memref() {
        return entity.memref();
    }

    public Entity getEntityForce() {
        return entity;
    }

    public <S,E> E accept(IRVisitor<S,E> visitor) {
        return visitor.visit(this);
    }
  // 打印
    protected void _dump(Dumper d) {
        d.printMember("entity", entity.name());
    }
}
