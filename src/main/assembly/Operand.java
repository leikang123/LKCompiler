package main.assembly;
// 操作指令的抽象类实现指令平台接口
public abstract class Operand implements OperandPattern {
    abstract public String toSource(SymbolTable table);
    abstract public String dump();
   // 寄存器
    public boolean isRegister() {
        return false;
    }
   // 内存引用
    public boolean isMemoryReference() {
        return false;
    }
    // 立即数功能
    public IntegerLiteral integerLiteral() {
        return null;
    }

    abstract public void collectStatistics(Statistics stats);

    // 匹配操作数
    public boolean match(Operand operand) {
        return equals(operand);
    }
}
    
