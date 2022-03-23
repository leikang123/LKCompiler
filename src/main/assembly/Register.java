package main.assembly;

// 寄存器抽象类继承操作数类
public abstract class Register extends Operand {
    // 判断是否是寄存器
    public boolean isRegister() {
        return true;
    }

    public void collectStatistics(Statistics stats) {
        stats.registerUsed(this);
    }
   // 自定义的抽象方法功能
   public abstract String toSource(SymbolTable syms);
   public abstract  String dump();
    
}
