package main.type;
import  main.AST.Location;

public class UserTypeRef extends TypeRef {
    protected String name;

    public UserTypeRef(String name) {
        this(null, name);
    }

    public UserTypeRef(Location loc, String name) {
        super(loc);
        this.name = name;
    }

    public boolean isUserType() {
        return true;
    }

    public String name() {
        return name;
    }

    public boolean equals(Object other) {
        if (!(other instanceof UserTypeRef)) return false;
        return name.equals(((UserTypeRef)other).name);
    }

    public String toString() {
        return name;
    }
}
