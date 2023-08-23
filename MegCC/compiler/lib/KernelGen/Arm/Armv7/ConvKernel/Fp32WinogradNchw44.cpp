#include <memory>
#include "Arm/Armv7/Activation.h"
#include "Arm/Armv7/ConvKernel/ConvKernel.h"
#include "Arm/Armv7/InternalKernel/InternalKernel.h"
#include "Utils/StringTemplate.h"
#include "compiler/KernelGen/KernelGen.h"

using namespace megcc;
using namespace KernelGen;
using namespace Armv7;
using namespace ArmCommon;

bool WinogradFloatF23NCHW44::IsAvailable(TContext* ctx) const {
    bool param_value_ok =
            ctx->getAttrUInt("kernel_h") == 3 && ctx->getAttrUInt("kernel_w") == 3 &&
            ctx->getAttrUInt("stride_h") == ctx->getAttrUInt("stride_w") &&
            ctx->getAttrUInt("stride_h") == 1 && ctx->getAttrUInt("dilate_h") == 1 &&
            ctx->getAttrUInt("dilate_w") == 1;

    bool param_mode_ok = ctx->getAttrStr("sparse") == "DENSE" &&
                         ctx->getAttrStr("format") == "NCHW44" &&
                         ctx->getAttrStr("mode") == "CROSS_CORRELATION";

    bool noline_ok = !ctx->haveAttr("nonlineMode") ||
                     ctx->getAttrStr("nonlineMode") == "IDENTITY" ||
                     ctx->getAttrStr("nonlineMode") == "RELU";

    bool type_ok = ctx->getAttrInt("nr_operands") >= 3 &&
                   ctx->getAttrOprand("operand:0").dtype == "f32" &&
                   ctx->getAttrOprand("operand:1").dtype == "f32" &&
                   ctx->getAttrOprand("operand:2").dtype == "f32";

    bool layout_ok = ctx->getAttrOprand("operand:0").shape.size() == 5 &&
                     ctx->getAttrOprand("operand:0").shape[4] == 4;

    return param_value_ok && param_mode_ok && type_ok && noline_ok && layout_ok;
}

std::string WinogradFloatF23NCHW44::GetInitBody(TContext* ctx) const {
    std::stringstream writer;
    writer << "#include<arm_neon.h>\n";
    writer << "#include<math.h>\n";
    writer << "\n\n";
    writer << GenCommonRet() << " " << GetInitSignature(ctx) << "{\n";
    writer << m_framework.GenInitCode(ctx, &m_winograd_strategy);
    writer << "\n}";
    return writer.str();
}

std::string WinogradFloatF23NCHW44::GetWorkspaceBody(TContext* ctx) const {
    std::stringstream writer;
    writer << GenCommonRet() << " " << GetWorkspaceSignature(ctx) << "{\n";
    writer << m_framework.GenGetWorkSpaceCode(ctx, &m_winograd_strategy);
    writer << "\n}";
    return writer.str();
}

std::string WinogradFloatF23NCHW44::GetKernelBody(TContext* ctx) const {
    std::stringstream writer;
    writer << "#include<arm_neon.h>";
    writer << "\n\n";
    writer << "extern " << MatmulM4N8MK4Kernel().GetKernelSignature(ctx) << ";\n";
    writer << GenCommonRet() << " " << GetKernelSignature(ctx) << "{\n";
    writer << m_framework.GenKernelBodyCode(ctx, &m_winograd_strategy);
    writer << "return TinyNN_SUCCESS;\n}";
    return writer.str();
}

std::vector<KernelObj> WinogradFloatF23NCHW44::GetDependInternalSymbol(
        TContext*) const {
    auto matmul = MatmulM4N8MK4Kernel();
    return {
            {matmul.GetKernelSymbol(nullptr), matmul.GetKernelBody(nullptr),
             matmul.GetBodyGuardBegin(nullptr), matmul.GetBodyGuardEnd(nullptr),
             matmul.GetDependInternalSymbol(nullptr)}};
}

std::string WinogradFloatF23NCHW44::GetKernelSymbol(TContext* context) const {
    auto symbol = Armv7ConvImpl::GetKernelSymbol(context);
    return symbol + "_winograd_f23";
}
// vim: syntax=cpp.doxygen
