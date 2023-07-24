package main.system;

import java.util.ArrayList;
import java.util.List;

import main.utils.ErrorHandler;

public class GNULinker implements Linker{
    // 32bit Linux dependent
       private  static final  String LINKER = "/usr/bin/ld";
       private   static final  String DYNAMIC_LINKER      = "/lib/ld-linux.so.2";
      private  static final  String C_RUNTIME_INIT      = "/usr/lib/crti.o";
      private  static final  String C_RUNTIME_START     = "/usr/lib/crt1.o";
       private static final  String C_RUNTIME_START_PIE = "/usr/lib/Scrt1.o";
        private static final  String C_RUNTIME_FINI      = "/usr/lib/crtn.o";
        
    
        ErrorHandler errorHandler;
    
        GNULinker(ErrorHandler errorHandler) {
            this.errorHandler = errorHandler;
        }
    
        
        public void generateExecutable(List<String> args,
                String destPath, LinkerOptions opts) throws IPCException {
            List<String> cmd = new ArrayList<String>();
            cmd.add(LINKER);
            cmd.add("-dynamic-linker");
            cmd.add(DYNAMIC_LINKER);
            if (opts.generatingPIE) {
                cmd.add("-pie");
            }
            if (! opts.noStartFiles) {
                cmd.add(opts.generatingPIE
                            ? C_RUNTIME_START_PIE
                            : C_RUNTIME_START);
                cmd.add(C_RUNTIME_INIT);
            }
            cmd.addAll(args);
            if (! opts.noDefaultLibs) {
                cmd.add("-lc");
                cmd.add("-lcbc");
            }
            if (! opts.noStartFiles) {
                cmd.add(C_RUNTIME_FINI);
            }
            cmd.add("-o");
            cmd.add(destPath);
            CommandUtils.invoke(cmd, errorHandler, opts.verbose);
        }
       
        public void generateSharedLibrary(List<String> args,
                String destPath, LinkerOptions opts) throws IPCException {
            List<String> cmd = new ArrayList<String>();
            cmd.add(LINKER);
            cmd.add("-shared");
            if (! opts.noStartFiles) {
                cmd.add(C_RUNTIME_INIT);
            }
            cmd.addAll(args);
            if (! opts.noDefaultLibs) {
                cmd.add("-lc");
                cmd.add("-lcbc");
            }
            if (! opts.noStartFiles) {
                cmd.add(C_RUNTIME_FINI);
            }
            cmd.add("-o");
            cmd.add(destPath);
            CommandUtils.invoke(cmd, errorHandler, opts.verbose);
        }
    
}
