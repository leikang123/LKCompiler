package main.entity;
import main.exception.*;
import java.util.List;
import java.util.ArrayList;
//作用域的抽象类
public abstract class Scope {
    //临时作用域的列表对象
    protected List<LocalScope> children;
    // 构造方法
    public Scope() {
        children = new ArrayList<LocalScope>();
    }
   // 抽象功能方法
   // 判断是否是顶部作用域方法
    abstract  boolean isToplevel();
    // 
    abstract  ToplevelScope toplevel();
    abstract  Scope parent();

    protected void addChild(LocalScope s) {
        children.add(s);
    }

    public abstract  Entity get(String name) throws SemanticException;
}
