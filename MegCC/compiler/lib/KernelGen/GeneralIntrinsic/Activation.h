#pragma once
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "Utils/StringTemplate.h"
#include "Utils/Utils.h"
#include "compiler/Common/Logger.h"

namespace megcc {
namespace KernelGen {
namespace GeneralIntrinsic {

enum NonlineMode { IDENTITY, H_SWISH, RELU, SIGMOID };
enum Dtype { FLOAT32, FLOAT16 };

struct ActivationGenIntrinsicBase {
    //! gen the const neon data, such as zero in relu
    virtual std::string GenIntrinsicInitFloat() const = 0;

    //! compute the input neon data and write to the output neon
    virtual std::string GenIntrinsicFloat(
            const std::string& input, const std::string& output) const = 0;
    //! compute the input neon data and write to the output ptr
    virtual std::string GenIntrinsicFloatStore(
            const std::string& input, const std::string& outptr) const = 0;

    //! compute the input neon data and write to the output ptr
    virtual std::string GenIntrinsicQuantStore(
            const std::string& input, const std::string& outptr,
            const std::string& scale_sym) const = 0;
};

template <NonlineMode mode, Dtype ctype>
struct ActivationGenIntrinsic : public ActivationGenIntrinsicBase {
public:
    //! gen the const neon data, such as zero in relu
    virtual std::string GenIntrinsicInitFloat() const override { return ""; }

    //! compute the input neon data and write to the output neon
    virtual std::string GenIntrinsicFloat(
            const std::string& input, const std::string& output) const override {
        std::stringstream writer;
        writer << "\n" << output << " = " << input << ";";
        return writer.str();
    };
    std::string GenIntrinsicFloatStore(
            const std::string& input, const std::string& outptr) const override {
        std::stringstream writer;
        writer << "\n GiStoreFloat32(" << outptr << ", " << input << ");";
        return writer.str();
    }
    std::string GenIntrinsicQuantStore(
            const std::string& input, const std::string& outptr,
            const std::string& scale_sym) const override {
        std::string store_temp = R"(
            {
                GI_FLOAT32_t f32_res = vcvtq_f32_s32(${input_reg});
                GI_INT8_t s8_res =  GiCvtFromFloat32ToInt8(GiMultiplyScalerFloat32(f32_res, ${scale_sym}));
                GiStoreLane0Int32((int32_t*)(${output_ptr}), GiReinterpretInt8AsInt32(s8_res));
            }        
        )";
        return StringTemplate::StringTemplateArgs()
                .add("input_reg", input)
                .add("scale_sym", scale_sym)
                .add("output_ptr", outptr)
                .render(store_temp);
    }
};

template <NonlineMode mode>
struct ActivationGenIntrinsic<mode, Dtype::FLOAT16>
        : public ActivationGenIntrinsicBase {
public:
    //! gen the const neon data, such as zero in relu
    virtual std::string GenIntrinsicInitFloat() const override { return ""; }

    //! compute the input neon data and write to the output neon
    virtual std::string GenIntrinsicFloat(
            const std::string& input, const std::string& output) const override {
        std::stringstream writer;
        writer << "\n" << output << " = " << input << ";";
        return writer.str();
    };
    std::string GenIntrinsicFloatStore(
            const std::string& input, const std::string& outptr) const override {
        std::stringstream writer;
        writer << "\n GiStoreFloat16(" << outptr << ", " << input << ");";
        return writer.str();
    }
    std::string GenIntrinsicQuantStore(
            const std::string& input, const std::string& outptr,
            const std::string& scale_sym) const override {
        CC_ASSERT(0) << "not impl GI quant relu fp16 compute act\n";
        return "";
    }
};

template <>
struct ActivationGenIntrinsic<NonlineMode::RELU, Dtype::FLOAT32>
        : public ActivationGenIntrinsicBase {
public:
    std::string GenIntrinsicInitFloat() const override {
        std::stringstream writer;
        writer << "\nGI_FLOAT32_t vzero = GiBroadcastFloat32(0.f);";
        return writer.str();
    }
    std::string GenIntrinsicFloat(
            const std::string& input, const std::string& output) const override {
        std::stringstream writer;
        writer << "\n" << output << " = GiMaximumFloat32(" << input << ", vzero);";
        return writer.str();
    }
    std::string GenIntrinsicFloatStore(
            const std::string& input, const std::string& outptr) const override {
        std::stringstream writer;
        writer << "\n GiStoreFloat32(" << outptr << ", GiMaximumFloat32(" << input
               << ", vzero));";
        return writer.str();
    }
    std::string GenIntrinsicQuantStore(
            const std::string& input, const std::string& outptr,
            const std::string& scale_sym) const override {
        std::string store_temp = R"(
            {
                GI_FLOAT32_t f32_res = vcvtq_f32_s32(${input_reg});
                GI_INT8_t s8_res =  GiCvtFromFloat32ToInt8(GiMultiplyScalerFloat32(f32_res, ${scale_sym}));
                GiStoreLane0Int32((int32_t*)(${output_ptr}), GiReinterpretInt8AsInt32(s8_res));
            }        
        )";
        return StringTemplate::StringTemplateArgs()
                .add("input_reg", input)
                .add("scale_sym", scale_sym)
                .add("output_ptr", outptr)
                .render(store_temp);
    }
};

template <>
struct ActivationGenIntrinsic<NonlineMode::H_SWISH, Dtype::FLOAT32>
        : public ActivationGenIntrinsicBase {
public:
    std::string GenIntrinsicInitFloat() const override {
        std::stringstream writer;
        writer << "\n GI_FLOAT32_t vzero =  GiBroadcastFloat32(0.f);";
        writer << "\n GI_FLOAT32_t f6_v =   GiBroadcastFloat32(6.f);";
        writer << "\n GI_FLOAT32_t f3_v =   GiBroadcastFloat32(3.f);";
        writer << "\n GI_FLOAT32_t inv6_v = GiBroadcastFloat32(1/6.f);";
        return writer.str();
    }
    std::string GenIntrinsicFloat(
            const std::string& input, const std::string& output) const override {
        std::stringstream writer;
        auto input_temp = "hswish_temp";
        writer << "\n{";
        writer << "GI_FLOAT32_t " << input_temp << " = GiAddFloat32(" << input
               << ", f3_v);\n";
        writer << input_temp << " = GiMaximumFloat32(" << input_temp << ", vzero);\n";
        writer << input_temp << " = GiMinimumFloat32(" << input_temp << ", f6_v);\n";
        writer << input_temp << " = GiMultiplyFloat32(" << input << ", " << input_temp
               << ");\n";
        writer << "\n"
               << output << " = GiMultiplyFloat32(" << input_temp << ", inv6_v);";
        writer << "\n}";
        return writer.str();
    }
    std::string GenIntrinsicFloatStore(
            const std::string& input, const std::string& outptr) const override {
        std::stringstream writer;

        auto input_temp = "hswish_temp";
        writer << "\n{";
        writer << "GI_FLOAT32_t " << input_temp << " = GiAddFloat32(" << input
               << ", f3_v);\n";
        writer << input_temp << " = GiMaximumFloat32(" << input_temp << ", vzero);\n";
        writer << input_temp << " = GiMinimumFloat32(" << input_temp << ", f6_v);\n";
        writer << input_temp << " = GiMultiplyFloat32(" << input << ", " << input_temp
               << ");\n";
        writer << "\n GiStoreFloat32(" << outptr << ", GiMultiplyFloat32(" << input_temp
               << ", inv6_v));";
        writer << "\n}";
        return writer.str();
    }
    std::string GenIntrinsicQuantStore(
            const std::string& input, const std::string& outptr,
            const std::string& scale_sym) const override {
        CC_ASSERT(0) << "not impl GI quant hswish act\n";
        return "";
    }
};

template <>
struct ActivationGenIntrinsic<NonlineMode::RELU, Dtype::FLOAT16>
        : public ActivationGenIntrinsicBase {
public:
    std::string GenIntrinsicInitFloat() const override {
        std::stringstream writer;
        writer << "\nGI_FLOAT16_t vzero = GiBroadcastFloat16(0.f);";
        return writer.str();
    }
    std::string GenIntrinsicFloat(
            const std::string& input, const std::string& output) const override {
        std::stringstream writer;
        writer << "\n" << output << " = GiMaximumFloat16(" << input << ", vzero);";
        return writer.str();
    }
    std::string GenIntrinsicFloatStore(
            const std::string& input, const std::string& outptr) const override {
        std::stringstream writer;
        writer << "\n GiStoreFloat16(" << outptr << ", GiMaximumFloat16(" << input
               << ", vzero));";
        return writer.str();
    }
    std::string GenIntrinsicQuantStore(
            const std::string& input, const std::string& outptr,
            const std::string& scale_sym) const override {
        CC_ASSERT(0) << "not impl GI quant relu fp16 compute act\n";
        return "";
    }
};

template <>
struct ActivationGenIntrinsic<NonlineMode::H_SWISH, Dtype::FLOAT16>
        : public ActivationGenIntrinsicBase {
public:
    std::string GenIntrinsicInitFloat() const override {
        std::stringstream writer;
        writer << "\n GI_FLOAT16_t vzero =  GiBroadcastFloat16(0.f);";
        writer << "\n GI_FLOAT16_t f6_v =   GiBroadcastFloat16(6.f);";
        writer << "\n GI_FLOAT16_t f3_v =   GiBroadcastFloat16(3.f);";
        writer << "\n GI_FLOAT16_t inv6_v = GiBroadcastFloat16(1/6.f);";
        return writer.str();
    }
    std::string GenIntrinsicFloat(
            const std::string& input, const std::string& output) const override {
        std::stringstream writer;
        auto input_temp = "hswish_temp";
        writer << "\n{";
        writer << "GI_FLOAT16_t " << input_temp << " = GiAddFloat16(" << input
               << ", f3_v);\n";
        writer << input_temp << " = GiMaximumFloat16(" << input_temp << ", vzero);\n";
        writer << input_temp << " = GiMinimumFloat16(" << input_temp << ", f6_v);\n";
        writer << input_temp << " = GiMultiplyFloat16(" << input << ", " << input_temp
               << ");\n";
        writer << "\n"
               << output << " = GiMultiplyFloat16(" << input_temp << ", inv6_v);";
        writer << "\n}";
        return writer.str();
    }
    std::string GenIntrinsicFloatStore(
            const std::string& input, const std::string& outptr) const override {
        std::stringstream writer;

        auto input_temp = "hswish_temp";
        writer << "\n{";
        writer << "GI_FLOAT16_t " << input_temp << " = GiAddFloat16(" << input
               << ", f3_v);\n";
        writer << input_temp << " = GiMaximumFloat16(" << input_temp << ", vzero);\n";
        writer << input_temp << " = GiMinimumFloat16(" << input_temp << ", f6_v);\n";
        writer << input_temp << " = GiMultiplyFloat16(" << input << ", " << input_temp
               << ");\n";
        writer << "\n GiStoreFloat16(" << outptr << ", GiMultiplyFloat16(" << input_temp
               << ", inv6_v));";
        writer << "\n}";
        return writer.str();
    }
    std::string GenIntrinsicQuantStore(
            const std::string& input, const std::string& outptr,
            const std::string& scale_sym) const override {
        CC_ASSERT(0) << "not impl GI quant hswish act\n";
        return "";
    }
};

std::shared_ptr<ActivationGenIntrinsicBase> create_activation_gener_instrinsic(
        std::string mode, std::string ctype = "f32");

}  // namespace GeneralIntrinsic
}  // namespace KernelGen
}  // namespace megcc

// vim: syntax=cpp.doxygen
