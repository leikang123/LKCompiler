package main.utils;
import java .io.*;

// 错误类
public class ErrorHandler {
    // 错误类的各种显示信息
    protected String programId;
    protected PrintStream stream;
    protected long nError;
    protected long nWarning;

    public ErrorHandler(String progid) {
        programId = progid;
        stream = System.err;
    }

    public ErrorHandler(String progid, OutputStream stream) {
        programId = progid;
        this.stream = new PrintStream(stream);
    }

    public void error(Location loc, String msg) {
        error(loc.toString() + ": " + msg);
    }

    public void error(String msg) {
        stream.println(programId + ": error: " + msg);
        nError++;
    }

    public void warn(Location loc, String msg) {
        warn(loc.toString() + ": " + msg);
    }

    public void warn(String msg) {
        stream.println(programId + ": warning: " + msg);
        nWarning++;
    }

    public boolean errorOccured() {
        return (nError > 0);
    }
    
}
