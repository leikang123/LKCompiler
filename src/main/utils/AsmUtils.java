package main.utils;

public final class AsmUtils {
        static public long align(long n, long alignment) {
            return (n + alignment - 1) / alignment * alignment;
        }
}
