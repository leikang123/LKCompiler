package main.system;
import java.util.List;

import main.exception.IPCException;
import main.utils.CommandUtils;
import main.utils.ErrorHandler;

import java.util.ArrayList;

public  class GNUAssembler implements Assembler {
    ErrorHandler errorHandler;

    GNUAssembler(ErrorHandler h) {
        this.errorHandler = h;
    }

    public void assemble(String srcPath, String destPath,
                            AssemblerOptions opts) throws IPCException {
        List<String> cmd = new ArrayList<String>();
        cmd.add("as");
        cmd.addAll(opts.args);
        cmd.add("-o");
        cmd.add(destPath);
        cmd.add(srcPath);
        CommandUtils.invoke(cmd, errorHandler, opts.verbose);
    }
}
