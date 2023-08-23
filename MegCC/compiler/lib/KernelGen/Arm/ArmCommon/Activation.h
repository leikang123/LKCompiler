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
namespace ArmCommon {

enum NonlineMode { IDENTITY, H_SWISH, RELU, SIGMOID };

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
            const std::string& src_scale, const std::string& dst_scale) const = 0;
};

template <NonlineMode mode>
struct ActivationGenIntrinsic : public ActivationGenIntrinsicBase {
public:
    //! gen the const neon data, such as zero in relu
    virtual std::string GenIntrinsicInitFloat() const override { return ""; }

    //! compute the input neon data and write to the output neon
    virtual std::string GenIntrinsicFloat(
            const std::string&, const std::string&) const override {
        return "";
    };
    std::string GenIntrinsicFloatStore(
            const std::string& input, const std::string& outptr) const override {
        std::stringstream writer;
        writer << "\n vst1q_f32(" << outptr << ", " << input << ");";
        return writer.str();
    }
    std::string GenIntrinsicQuantStore(
            const std::string& input, const std::string& outptr,
            const std::string& src_scale, const std::string& dst_scale) const override {
        std::string store_temp = R"(
            {
                float32x4_t f32_res = vcvtq_f32_s32(${input_reg});
                float32x4_t res = vmulq_n_f32(f32_res, ${src_scale});
                res = vmulq_n_f32(res, ${dst_scale}); 
                int32x4_t s32_res = vcvtaq_s32_f32(res);
                int16x4_t s16_res = vqmovn_s32(s32_res);
                int8x8_t s8_res = vqmovn_s16(vcombine_s16(s16_res, s16_res));
                vst1_lane_s32((${output_ptr}), vreinterpret_s32_s8(s8_res), 0);
            }        
        )";
        return StringTemplate::StringTemplateArgs()
                .add("input_reg", input)
                .add("src_scale", src_scale)
                .add("dst_scale", dst_scale)
                .add("output_ptr", outptr)
                .render(store_temp);
    }
};

template <>
struct ActivationGenIntrinsic<NonlineMode::RELU> : public ActivationGenIntrinsicBase {
public:
    std::string GenIntrinsicInitFloat() const override {
        std::stringstream writer;
        writer << "\nfloat32x4_t vzero = vdupq_n_f32(0.f);";
        return writer.str();
    }
    std::string GenIntrinsicFloat(
            const std::string& input, const std::string& output) const override {
        std::stringstream writer;
        writer << "\n" << output << " = vmaxq_f32(" << input << ", vzero);";
        return writer.str();
    }
    std::string GenIntrinsicFloatStore(
            const std::string& input, const std::string& outptr) const override {
        std::stringstream writer;
        writer << "\n vst1q_f32(" << outptr << ", vmaxq_f32(" << input << ", vzero));";
        return writer.str();
    }
    std::string GenIntrinsicQuantStore(
            const std::string& input, const std::string& outptr,
            const std::string& src_scale, const std::string& dst_scale) const override {
        std::string store_temp = R"(
            {
                float32x4_t f32_res = vcvtq_f32_s32(${input_reg});
                float32x4_t res = vmaxq_f32(vmulq_n_f32(f32_res, ${src_scale}), vzero);
                res = vmulq_n_f32(res, ${dst_scale}); 
                int32x4_t s32_res = vcvtaq_s32_f32(res);
                int16x4_t s16_res = vqmovn_s32(s32_res);
                int8x8_t s8_res = vqmovn_s16(vcombine_s16(s16_res, s16_res));
                vst1_lane_s32((${output_ptr}), vreinterpret_s32_s8(s8_res), 0);
            }        
        )";
        return StringTemplate::StringTemplateArgs()
                .add("input_reg", input)
                .add("src_scale", src_scale)
                .add("dst_scale", dst_scale)
                .add("output_ptr", outptr)
                .render(store_temp);
    }
};

template <>
struct ActivationGenIntrinsic<NonlineMode::H_SWISH>
        : public ActivationGenIntrinsicBase {
public:
    std::string GenIntrinsicInitFloat() const override {
        std::stringstream writer;
        writer << "\nfloat32x4_t vzero = vdupq_n_f32(0.f);";
        writer << "\nfloat32x4_t f6_v = vdupq_n_f32(6.f);";
        writer << "\nfloat32x4_t f3_v = vdupq_n_f32(3.f);";
        writer << "\nfloat32x4_t inv6_v = vdupq_n_f32(1/6.f);";
        return writer.str();
    }
    std::string GenIntrinsicFloat(
            const std::string& input, const std::string& output) const override {
        std::stringstream writer;
        auto input_temp = "hswish_temp";
        writer << "\n{";
        writer << "float32x4_t " << input_temp << " = vaddq_f32(" << input
               << ", f3_v);\n";
        writer << input_temp << " = vmaxq_f32(" << input_temp << ", vzero);\n";
        writer << input_temp << " = vminq_f32(" << input_temp << ", f6_v);\n";
        writer << input_temp << " = vmulq_f32(" << input << ", " << input_temp
               << ");\n";
        writer << "\n" << output << " = vmulq_f32(" << input_temp << ", inv6_v);";
        writer << "\n}";
        return writer.str();
    }
    std::string GenIntrinsicFloatStore(
            const std::string& input, const std::string& outptr) const override {
        std::stringstream writer;

        auto input_temp = "hswish_temp";
        writer << "\n{";
        writer << "float32x4_t " << input_temp << " = vaddq_f32(" << input
               << ", f3_v);\n";
        writer << input_temp << " = vmaxq_f32(" << input_temp << ", vzero);\n";
        writer << input_temp << " = vminq_f32(" << input_temp << ", f6_v);\n";
        writer << input_temp << " = vmulq_f32(" << input << ", " << input_temp
               << ");\n";
        writer << "\n vst1q_f32(" << outptr << ", vmulq_f32(" << input_temp
               << ", inv6_v));";
        writer << "\n}";
        return writer.str();
    }
    std::string GenIntrinsicQuantStore(
            const std::string& input, const std::string& outptr,
            const std::string& src_scale, const std::string& dst_scale) const override {
        std::string store_temp = R"(
            {
                float32x4_t f32_res = vcvtq_f32_s32(${input_reg});
                f32_res = vmulq_n_f32(f32_res, ${src_scale});

                float32x4_t relu6 = vaddq_f32(f32_res, f3_v);
                relu6 = vminq_f32(relu6, f6_v);
                relu6 = vmaxq_f32(vzero, relu6);
                f32_res = vmulq_f32(f32_res, relu6);
                f32_res = vmulq_f32(f32_res, inv6_v);
                f32_res = vmulq_n_f32(f32_res, ${dst_scale}); 
                int32x4_t s32_res = vcvtaq_s32_f32(f32_res);
                int16x4_t s16_res = vqmovn_s32(s32_res);
                int8x8_t s8_res = vqmovn_s16(vcombine_s16(s16_res, s16_res));
                vst1_lane_s32((${output_ptr}), vreinterpret_s32_s8(s8_res), 0);
            }        
        )";
        return StringTemplate::StringTemplateArgs()
                .add("input_reg", input)
                .add("src_scale", src_scale)
                .add("dst_scale", dst_scale)
                .add("output_ptr", outptr)
                .render(store_temp);
    }
};

std::shared_ptr<ActivationGenIntrinsicBase> create_activation_gener_instrinsic(
        std::string mode);

}  // namespace ArmCommon
}  // namespace KernelGen
}  // namespace megcc

// vim: syntax=cpp.doxygen
