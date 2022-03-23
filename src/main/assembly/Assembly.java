    package main.assembly;
  // 汇编抽象类
  public abstract class Assembly {
      // 抽象源符号表
    public abstract String toSource(SymbolTable table);
    // 抽象转储方法
    public abstract String dump();
    // 判断指令
    public boolean isInstruction() {
        return false;
    }
    // 标签
    public boolean isLabel() {
        return false;
    }
     // 汇编伪操作
     public boolean isDirective() {
        return false;
    }
    // 注释
    public boolean isCommon() {
        return false;
    }

    public void collectStatistics(Statistics stats) {
        
    }
}
