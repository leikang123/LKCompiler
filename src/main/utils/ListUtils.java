package main.utils;
import java.io.*;
import java.util.*;
// 抽象列表工具类
public abstract class ListUtils {
    // 列表任何类型都可以通过
     public static <T> List<T> reverse(List<T> list) {
        List<T> result = new ArrayList<T>(list.size());
        ListIterator<T> it = list.listIterator(list.size());
        while (it.hasPrevious()) {
            result.add(it.previous());
        }
        return result;
    }
}
