package main.system;

import java.util.List;

import main.exception.IPCException;

public interface Linker {
    void generateExecutable(List<String> args, String destPath,
            LinkerOptions opts) throws IPCException;
    void generateSharedLibrary(List<String> args, String destPath,
            LinkerOptions opts) throws IPCException;
}
