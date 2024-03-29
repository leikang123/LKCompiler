#ifndef ABSTRACT_KERNEL_INTERFACES
#define ABSTRACT_KERNEL_INTERFACES

include "mlir/IR/OpBase.td"
include "mlir/IR/BuiltinAttributes.td"

def ElemwiseKernelInterface : OpInterface<"ElemwiseKernelInterface"> {
    let description = "an elemwise-like operation interface";
    let cppNamespace = "::mlir::Kernel";

    let methods = [
        InterfaceMethod<
            /*desc=*/"return elemwise mode",
            /*retTy=*/"llvm::StringRef",
            /*methodName=*/"getMode",
            /*args=*/(ins),
            /*methodBody=*/"return $_op->getName().stripDialect();"
        >,
    ];
}

def MemFwdInterface : OpInterface<"MemFwdInterface"> {
    let description = "readonly memory forward operation interface";
    let cppNamespace = "::mlir::Kernel";

    let methods = [
        InterfaceMethod<
            /*desc=*/"try to forward input memref to output",
            /*retTy=*/"MemRefType",
            /*methodName*/"memoryForward",
            /*args=*/(ins "MemRefType":$input)
        >,
    ];
}

def LayoutConstraintInterface : OpInterface<"LayoutConstraintInterface"> {
    let description = "add layout constraint for operands";
    let cppNamespace = "::mlir::Kernel";

    let methods = [
        InterfaceMethod<
            /*desc=*/"return whether given input memref has valid layout",
            /*retTy=*/"bool",
            /*methodName*/"checkInputLayout",
            /*args=*/(ins "MemRefType":$memref, "size_t":$index),
            /*methodBody=*/[{}],
            /*defaultImplementation=*/[{
                return ::mlir::Kernel::isContiguous(memref);
            }]
        >,
    ];
}

#endif // ABSTRACT_KERNEL_INTERFACES
