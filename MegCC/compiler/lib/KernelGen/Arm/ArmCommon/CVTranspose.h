#pragma once
#include "CvCommon.h"
#include "compiler/KernelGen/KernelGen.h"
namespace megcc {
namespace KernelGen {
namespace ArmCommon {

class CvTransposeKernel : public CVKernelImpl {
public:
    bool IsCVAvailable(TContext* context) const override;
    std::string GetCVKernelBody(TContext* context) const override;
    std::string GetCVKernelSubSymbol(TContext* context) const override;
    std::string GetCVKernelSignature(TContext* context) const override;
};

}  // namespace ArmCommon
}  // namespace KernelGen
}  // namespace megcc