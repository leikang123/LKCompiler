package main.system;

public interface Assembler {
    void assemble(String srcPath, String destPath,
            AssemblerOptions opts) throws IPCException;
}

