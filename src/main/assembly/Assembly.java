  package main.assembly;
  // 汇编抽象类
  public abstract class Assembly {
      // 抽象源符号表
    abstract public String toSource(SymbolTable table);
    // 抽象转储方法
    abstract public String dump();

    public boolean isInstruction() {
        return false;
    }

    public boolean isLabel() {
        return false;
    }

    public boolean isDirective() {
        return false;
    }
    
    public boolean isCommon() {
        return false;
    }

    public void collectStatistics(Statistics stats) {
        
    }
}
