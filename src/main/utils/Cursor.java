package main.utils;

import java.util.Iterator;
import java.io.*;
import java.util.*;
// 继承遍历类
public class Cursor<T> implements Iterator {
    // 列表
    protected List<T> list;
    // 索引
    protected int index;

    public Cursor(List<T> list) {
        this(list, 0);
    }

    protected Cursor(List<T> list, int index) {
        this.list = list;
        this.index = index;
    }

    public Cursor<T> clone() {
        return new Cursor<T>(list, index);
    }

    public boolean hasNext() {
        return index < list.size();
    }

    public T next() {
        return list.get(index++);
    }

    public T current() {
        if (index == 0) {
            throw new Error("must not happen: Cursor#current");
        }
        return list.get(index - 1);
    }

    public void remove() {
        list.remove(index);
    }

    public String toString() {
        return "#<Cursor list=" + list + " index=" + index + ">";
    }
    
}
