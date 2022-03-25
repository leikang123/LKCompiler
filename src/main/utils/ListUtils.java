package main.utils;
import java.io.*;
import java.util.*;

public abstract class ListUtils {
    static public <T> List<T> reverse(List<T> list) {
        List<T> result = new ArrayList<T>(list.size());
        ListIterator<T> it = list.listIterator(list.size());
        while (it.hasPrevious()) {
            result.add(it.previous());
        }
        return result;
    }
}
