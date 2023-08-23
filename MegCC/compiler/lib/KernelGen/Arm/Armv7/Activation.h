#pragma once
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "Arm/ArmCommon/Activation.h"
#include "Utils/StringTemplate.h"
#include "Utils/Utils.h"
#include "compiler/Common/Logger.h"
namespace megcc {
namespace KernelGen {
namespace Armv7 {

enum NonlineMode { IDENTITY, H_SWISH, RELU, SIGMOID };

struct ActivationGenAsmBase {
    virtual std::string GenAsmFloat(
            std::vector<std::string> regs, std::vector<std::string> help_regs) = 0;
    virtual std::string GenAsmInt8(
            std::vector<std::string> regs, std::vector<std::string> help_regs) = 0;
    virtual std::string GenAsmFloat(std::vector<std::string> regs) {
        CC_ASSERT(regs.size() >= 2);
        auto helper_reg = regs.back();
        regs.pop_back();
        return GenAsmFloat(regs, {helper_reg});
    }
    virtual std::string GenAsmQuantInit(
            const std::vector<std::string> args_reg, const std::string& mode,
            const std::vector<std::string> args_ptr);
    virtual std::string GenAsmQuantStore(
            std::vector<std::string> int_regs, std::string scale_reg,
            const std::string& output_sym, const int elem_offset,
            const std::string dst_specifier, const std::vector<std::string> args_reg,
            const std::string& mode, bool with_store = true);
};

template <NonlineMode mode>
struct ActivationGenAsm : public ActivationGenAsmBase {
public:
    std::string GenAsmFloat(
            std::vector<std::string>, std::vector<std::string> help_regs) override {
        return "";
    }
    std::string GenAsmInt8(
            std::vector<std::string>, std::vector<std::string> help_regs) override {
        return "";
    }
};

template <>
struct ActivationGenAsm<NonlineMode::RELU> : public ActivationGenAsmBase {
public:
    //! the first register is the zero register
    std::string GenAsmFloat(
            std::vector<std::string> registers,
            std::vector<std::string> help_regs) override {
        std::string r_zero = help_regs[0] + "\\n\"\n";
        std::stringstream writer;
        writer << "\n";
        for (size_t i = 0; i < registers.size(); i++) {
            writer << "\"vmax.f32 " << registers[i] << ", " << registers[i] << ", "
                   << r_zero;
        }
        return writer.str();
    }
    std::string GenAsmInt8(
            std::vector<std::string> registers,
            std::vector<std::string> help_regs) override {
        // TODO: to add the activation
        return "";
    }
};

std::shared_ptr<ActivationGenAsmBase> create_activation_gener(std::string mode);

}  // namespace Armv7
}  // namespace KernelGen
}  // namespace megcc

// vim: syntax=cpp.doxygen
