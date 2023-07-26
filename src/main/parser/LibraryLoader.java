package main.parser;
import main.AST.Declarations;
import main.utils.ErrorHandler;
import main.exception.*;
import java.util.*;
import java.io.*;
// 加载库类
public class LibraryLoader {
    // 加载路径
    protected List<String> loadPath;
    // 加载的库列表
    protected LinkedList<String> loadingLibraries;
    // 加载库声明
    protected Map<String, Declarations> loadedLibraries;
    //默认加载字符列表
     public static List<String> defaultLoadPath() {
        List<String> pathes = new ArrayList<String>();
        pathes.add(".");
        return pathes;
    }

    public LibraryLoader() {
        this(defaultLoadPath());
    }

    public LibraryLoader(List<String> loadPath) {
        this.loadPath = loadPath;
        this.loadingLibraries = new LinkedList<String>();
        this.loadedLibraries = new HashMap<String, Declarations>();
    }
 //添加家在路径方法
    public void addLoadPath(String path) {
        loadPath.add(path);
    }

    public Declarations loadLibrary(String libid, ErrorHandler handler)
            throws CompileException {
        if (loadingLibraries.contains(libid)) {
            throw new SemanticException("recursive import from "
                                        + loadingLibraries.getLast()
                                        + ": " + libid);
        }
        loadingLibraries.addLast(libid);   // stop recursive import
        Declarations decls = loadedLibraries.get(libid);
        if (decls != null) {
            // Already loaded import file.  Returns cached declarations.
            return decls;
        }
        decls = Parser.parseDeclFile(searchLibrary(libid), this, handler);
        loadedLibraries.put(libid, decls);
        loadingLibraries.removeLast();
        return decls;
    }

    public File searchLibrary(String libid) throws FileException {
        try {
            for (String path : loadPath) {
                File file = new File(path + "/" + libPath(libid) + ".hb");
                if (file.exists()) {
                    return file;
                }
            }
            throw new FileException(
                "no such library header file: " + libid);
        }
        catch (SecurityException ex) {
            throw new FileException(ex.getMessage());
        }
    }

    protected String libPath(String id) {
        return id.replace('.', '/');
    }
}
