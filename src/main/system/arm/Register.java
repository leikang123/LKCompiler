package main.system.x86;

import main.assembly.SymbolTable;
import main.type.Type;

public class Register extends main.assembly.Register{
    public RegisterClass class;
    public Type type;
    
    //构造方法
    public Register(RegisterClass class,Type type){
        this.class = class;
        this.type = type;

    }
    public Register forType(Type t) {
        return new Register(class, t);
    }

    public boolean isRegister() { 
        return true;
     }

    public boolean equals(Object other) {
        return (other instanceof Register) && equals((Register)other);
    }

    /** 大小不同的匹配. */
    public boolean equals(Register reg) {
        return class.equals(reg.class);
    }

    public int hashCode() {
        return class.hashCode();
    }

    RegisterClass registerClass() {
        return class;
    }

    String baseName() {
        return class.toString().toLowerCase();
    }

    private String typedName() {
        switch (type) {
        case INT8: return lowerByteRegister();
        case INT16: return baseName();
        case INT32: return "e" + baseName();
        case INT64: return "r" + baseName();
        default:
            throw new Error("unknown register Type: " + type);
        }
    }

    /*private String lowerByteRegister() {
        switch (class) {
        case AX:
        case BX:
        case CX:
        case DX:
            return baseName().substring(0, 1) + "l";
        default:
            throw new Error("does not have lower-byte register: " + _class);
        }
    }*/

    public String dump() {
        return "(Register " +class.toString() + " " + type.toString() + ")";
    }
    @Override
    public String toSource(SymbolTable syms) {
        return null;
    }
}
