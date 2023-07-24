package main.utils;

public final class AsmUtils {
      public  static  long align(long n, long alignment) {
            return (n + alignment - 1) / alignment * alignment;
        }
}
