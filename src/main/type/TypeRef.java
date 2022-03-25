package main.type;

public abstract class TypeRef {

    protected Location location;

    public TypeRef(Location loc) {
        this.location = loc;
    }

    public Location location() {
        return location;
    }

    public int hashCode() {
        return toString().hashCode();
    }
}
