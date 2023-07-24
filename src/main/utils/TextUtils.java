package main.utils;
import java.io.*;

public abstract class TextUtils {
   private static final byte vtab = 013;

    public static  String dumpString(String str) {
        try {
            return dumpString(str, Parser.SOURCE_ENCODING);
        }
        catch (UnsupportedEncodingException ex) {
            throw new Error("UTF-8 is not supported??: " + ex.getMessage());
        }
    }

    public static String dumpString(String string, String encoding)
            throws UnsupportedEncodingException {
        byte[] src = string.getBytes(encoding);
        StringBuffer buf = new StringBuffer();
        buf.append("\"");
        for (int n = 0; n < src.length; n++) {
            int c = toUnsigned(src[n]);
            if (c == '"') buf.append("\\\"");
            else if (isPrintable(c)) buf.append((char)c);
            else if (c == '\b') buf.append("\\b");
            else if (c == '\t') buf.append("\\t");
            else if (c == '\n') buf.append("\\n");
            else if (c == vtab) buf.append("\\v");
            else if (c == '\f') buf.append("\\f");
            else if (c == '\r') buf.append("\\r");
            else {
                buf.append("\\" + Integer.toOctalString(c));
            }
        }
        buf.append("\"");
        return buf.toString();
    }

    private static  int toUnsigned(byte b) {
        return b >= 0 ? b : 256 + b;
    }

    public static  boolean isPrintable(int c) {
        return (' ' <= c) && (c <= '~');
    }
}
