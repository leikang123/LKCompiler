package main.assembly;
// 汇编语言字面量的接口功能继承Comparable
public interface Literal extends Comparable<Literal> {
    public String toSource();
    public String toSource(SymbolTable table);
    public String dump();
    public void collectStatistics(Statistics stats);
    public boolean isZero();
    public Literal plus(long diff);
    // cmp数据比较指令
    public int cmp(IntegerLiteral i);
    public int cmp(NamedSymbol sym);
    public int cmp(UnnamedSymbol sym);
    public int cmp(SuffixedSymbol sym);
}
