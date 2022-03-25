package main.type;
import java.util.*;
import main.AST.Location;
import main.exception.*;


public abstract class NamedType extends Type{
    protected String name;
    protected Location location;

    public NamedType(String name, Location loc) {
        this.name = name;
        this.location = loc;
    }

    public String name() {
        return name;
    }

    public Location location() {
        return location;
    }
    
}
