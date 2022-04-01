package main.entity;
import main.AST.TypeNode;
import main.type.*;

public abstract  class Variable extends Entity {
    public Variable(boolean priv, TypeNode type, String name) {
        super(priv, type, name);
    }
}
