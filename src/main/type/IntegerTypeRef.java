package main.type;
import main.AST.Location;

public class IntegerTypeRef {
    static public IntegerTypeRef charRef(Location loc) {
        return new IntegerTypeRef("char", loc);
    }

  public  static  IntegerTypeRef charRef() {
        return new IntegerTypeRef("char");
    }

    public static  IntegerTypeRef shortRef(Location loc) {
        return new IntegerTypeRef("short", loc);
    }

    public static  IntegerTypeRef shortRef() {
        return new IntegerTypeRef("short");
    }

    public static IntegerTypeRef intRef(Location loc) {
        return new IntegerTypeRef("int", loc);
    }

    public static  IntegerTypeRef intRef() {
        return new IntegerTypeRef("int");
    }

    public static  IntegerTypeRef longRef(Location loc) {
        return new IntegerTypeRef("long", loc);
    }

    public static IntegerTypeRef longRef() {
        return new IntegerTypeRef("long");
    }

    public static  IntegerTypeRef ucharRef(Location loc) {
        return new IntegerTypeRef("unsigned char", loc);
    }

    public static  IntegerTypeRef ucharRef() {
        return new IntegerTypeRef("unsigned char");
    }

    public static IntegerTypeRef ushortRef(Location loc) {
        return new IntegerTypeRef("unsigned short", loc);
    }

    public static  IntegerTypeRef ushortRef() {
        return new IntegerTypeRef("unsigned short");
    }

    public static IntegerTypeRef uintRef(Location loc) {
        return new IntegerTypeRef("unsigned int", loc);
    }

    public static IntegerTypeRef uintRef() {
        return new IntegerTypeRef("unsigned int");
    }

    public static  IntegerTypeRef ulongRef(Location loc) {
        return new IntegerTypeRef("unsigned long", loc);
    }

    public static IntegerTypeRef ulongRef() {
        return new IntegerTypeRef("unsigned long");
    }

    protected String name;

    public IntegerTypeRef(String name) {
        this(name, null);
    }

    public IntegerTypeRef(String name, Location loc) {
        super(loc);
        this.name = name;
    }

    public String name() {
        return name;
    }

    public boolean equals(Object other) {
        if (! (other instanceof IntegerTypeRef)) return false;
        IntegerTypeRef ref = (IntegerTypeRef)other;
        return name.equals(ref.name);
    }

    public String toString() {
        return name;
    }
}
