#include <unistd.h>
#include <sstream>
#include <vector>

#include "Elemwise.h"
#include "Matmul.h"
#include "compiler/CodeGen/CodeGen.h"

using namespace megcc;
using namespace KernelGen;
namespace {
struct AllMLIRKernel {
    AllMLIRKernel() {
        inner_map[KernelPack::KernType::ElemwiseKernel] =
                std::make_shared<codegen::ElemwiseKernel>();
        inner_map[KernelPack::KernType::MatrixMulKernel] =
                std::make_shared<codegen::MatmulKernel>();
    }

    std::unordered_map<KernelPack::KernType, std::shared_ptr<codegen::AutoKernelFunc>>
            inner_map;
};
}  // namespace

codegen::AutoKernelFunc* codegen::GenCode(KernelPack::KernType kern_type) {
    static AllMLIRKernel all_kernel;
    if (all_kernel.inner_map.find(kern_type) != all_kernel.inner_map.end()) {
        return all_kernel.inner_map[kern_type].get();
    } else {
        return nullptr;
    }
}
