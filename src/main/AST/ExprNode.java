package main.AST;

import main.exception.SemanticError;
import main.type.Type;
// 表达式的节点的表示
public abstract class ExprNode extends Node{
    public ExprNode() {
        super();
    }
   // 抽象表达式的类型
    public abstract Type type();
    // 
    protected Type origType() {
    	return type(); 
    	}
    // 
    public long allocSize() { 
    	return type().allocSize(); }

    public boolean isConstant() { 
        return false;
     }
    public boolean isParameter() { 
        return false; 
    }

    public boolean isLvalue() { 
        return false; 
    }
    public boolean isAssignable() {
         return false; 
        }
    public boolean isLoadable() {
         return false; 
        }

    public boolean isCallable() {
        try {
            return type().isCallable();
        }
        catch (SemanticError err) {
            return false;
        }
    }


    public boolean isPointer() {
        try {
            return type().isPointer();
        }
        catch (SemanticError err) {
            return false;
        }
    }

   public  abstract <S,E> E accept(ASTVisitor<S,E> visitor);
}
