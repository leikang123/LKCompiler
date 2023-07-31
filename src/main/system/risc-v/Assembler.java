package main.system.x86;
// 汇编接口
public interface Assembler {
    void assembly(String strPath,String destPath,
    AssemblerOptions opts)throws IPCException;
    
}
