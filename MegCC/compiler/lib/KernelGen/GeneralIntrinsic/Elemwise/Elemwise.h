#pragma once
#include <sstream>
#include <string>
#include "compiler/KernelGen/KernelGen.h"

namespace megcc {
namespace KernelGen {
namespace GeneralIntrinsic {

class ElemwiseKernel : public KernelFunc {
public:
    virtual ~ElemwiseKernel(){};
    bool IsAvailable(TContext* context) const override;
    std::string GetKernelSymbol(TContext* context) const override;
    std::string GetKernelBody(TContext* context) const override;

    std::vector<KernelObj> GetDependInternalSymbol(TContext* context) const override;
};

}  // namespace GeneralIntrinsic
}  // namespace KernelGen
}  // namespace megcc

// vim: syntax=cpp.doxygen
