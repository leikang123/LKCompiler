#include "ElemwiseHelper.h"
#include "GeneralIntrinsic/GISimdHelper.h"
#include "Utils/SymbolHelper.h"
#include "Utils/Utils.h"
#include "compiler/Common/Logger.h"

using namespace megcc;
using namespace KernelGen;
using namespace GeneralIntrinsic;

std::string ElemwiseGenUnary::GenCodeBody(std::vector<std::string> strs) const {
    std::stringstream body_ss;
    if (m_inline_mode) {
        body_ss << R"(static inline void ${inline_func_name}(const ${src_specifier}* src, ${dst_specifier}* dst, size_t nr_elem)";
        if (m_i32_to_qs8) {
            body_ss << ", float src_scale, float dst_scale";
        }
        body_ss << "){";
    } else {
        body_ss << R"(
            Layout in_layout = inputs[0]->layout;
            size_t nr_elem = 1;
            for (int i = 0; i < in_layout.nr_dim; ++i) {
                nr_elem *= in_layout.dims[i];
            }
            const ${src_specifier} * src = ${source};
            ${dst_specifier}* dst = ${dst};
        )";
    }
    body_ss << R"(
        ${kernel_init()}

        size_t index = 0;
        for(; index + 2*${simd_len}-1 < nr_elem; index += 2*${simd_len}) {
            ${src_simd_specifier} vsrc0 = ${src_ld1q}(src);
            ${src_simd_specifier} vsrc1 = ${src_ld1q}(src + ${simd_len});
            ${kernel_simd_unroll(2, vsrc0, vdst0, vsrc1, vdst1)}
            ${dst_store(dst, vdst0)};
            dst += ${simd_len};
            ${dst_store(dst, vdst1)};
            src += 2*${simd_len};
            dst += ${simd_len};
        }
        for(; index + ${simd_len}-1 < nr_elem; index += ${simd_len}) {
            ${src_simd_specifier} vsrc0 = ${src_ld1q}(src);
            ${kernel_simd_unroll(1, vsrc0, vdst0)}
            ${dst_store(dst, vdst0)};
            src += ${simd_len};
            dst += ${simd_len};
        }
        for(; index < nr_elem; index++) {
            ${kernel_naive_unroll(1, src, dst)}
            src += 1;
            dst += 1;
        })";
    if (m_inline_mode) {
        body_ss << "}";
    }
    auto kernel_init = [this](std::vector<std::string> strs) {
        return GenKernelSimdInit(strs);
    };
    auto kernel_simd_unroll = [this](std::vector<std::string> strs) {
        return GenKernelSimdUnroll(strs);
    };
    auto kernel_naive_unroll = [this](std::vector<std::string> strs) {
        return GenKernelNaiveUnroll(strs);
    };
    std::stringstream ss;
    auto body_render =
            StringTemplate::StringTemplateArgs()
                    .add("kernel_init", kernel_init)
                    .add("kernel_simd_unroll", kernel_simd_unroll)
                    .add("kernel_naive_unroll", kernel_naive_unroll)
                    .add("src_specifier", Utils::cvt_dtype_specifier(m_src_dtype))
                    .add("dst_specifier", Utils::cvt_dtype_specifier(m_dst_dtype))
                    .add("src_ld1q", m_src_simd->get_ld1q_symbol())
                    .add("dst_store",
                         [=](std::string ptr, std::string dst_reg) {
                             if (m_i32_to_qs8) {
                                 return "GiStoreLane0Int32((int32_t*)(" + ptr + ")," +
                                        dst_reg + ", 0)\n";
                             } else {
                                 return m_dst_simd->get_st1q_symbol() + "(" + ptr +
                                        "," + dst_reg + ")\n";
                             }
                         })
                    .add("dst_st1q", m_dst_simd->get_st1q_symbol())
                    .add("src_simd_specifier", m_src_simd->get_specifier_q_symbol())
                    .add("simd_len", m_src_simd->get_nr_elem_q());

    if (m_inline_mode) {
        body_render.add("inline_func_name", GenInlineName());
    } else {
        auto input = strs[0];
        auto output = strs[1];
        body_render.add("source", input).add("dst", output);
    }
    ss << body_render.render(body_ss.str());

    return ss.str();
}

//! Relu
std::string ElemwiseGenUnaryRelu::GenInlineName() const {
    return "ElemwiseGenUnaryRelu";
}
std::string ElemwiseGenUnaryRelu::GenKernelSimdInit(std::vector<std::string>) const {
    if (m_src_dtype == "f32") {
        return "GI_FLOAT32_t vzero = GiBroadcastFloat32(0.0f);";
    } else if (m_src_dtype == "f16") {
        return "GI_FLOAT16_t vzero = GiZeroFloat16();";
    } else {
        CC_ABORT << " ElemwiseGenUnaryRelu is not supported  dtype: " << m_src_dtype
                 << "\n";
        return "";
    }
}

std::string ElemwiseGenUnaryRelu::GenKernelSimdUnroll(
        std::vector<std::string> strs) const {
    int unroll = std::stoi(strs[0]);
    std::stringstream writer;
    for (int i = 0; i < unroll; i++) {
        if (m_src_dtype == "f32") {
            writer << "\n GI_FLOAT32_t " << strs[2 * i + 2] << " = GiMaximumFloat32(("
                   << strs[2 * i + 1] << "), vzero);";
        } else if (m_src_dtype == "f16") {
            writer << "\n GI_FLOAT16_t " << strs[2 * i + 2] << " = GiMaximumFloat16(("
                   << strs[2 * i + 1] << "), vzero);";
        } else {
            CC_ABORT << " ElemwiseGenUnaryRelu is not supported  dtype: " << m_src_dtype
                     << "\n";
        }
    }
    return writer.str();
}

std::string ElemwiseGenUnaryRelu::GenKernelNaiveUnroll(
        std::vector<std::string> strs) const {
    int unroll = std::stoi(strs[0]);
    auto input_ptr = strs[1];
    auto output_ptr = strs[2];
    std::stringstream writer;
    for (int i = 0; i < unroll; i++) {
        if (m_src_dtype == "f32") {
            writer << "\n(" << output_ptr << ")[" << i << "] =  fmax((" << input_ptr
                   << ")[" << i << "], 0.0f);";
        } else if (m_src_dtype == "f16") {
            writer << "\n(" << output_ptr << ")[" << i << "] =  (" << input_ptr << ")["
                   << i << "] > 0.0 ? (" << input_ptr << ")[" << i << "] : 0.0;";
        } else {
            CC_ABORT << " ElemwiseGenUnaryRelu is not supported  dtype: " << m_src_dtype
                     << "\n";
        }
    }
    return writer.str();
}

//! Exp
std::string ElemwiseGenUnaryExp::GenInlineName() const {
    return "ElemwiseGenUnaryExp";
}
std::string ElemwiseGenUnaryExp::GenKernelSimdInit(std::vector<std::string>) const {
    return " ";
}

std::string ElemwiseGenUnaryExp::GenKernelSimdUnroll(
        std::vector<std::string> strs) const {
    int unroll = std::stoi(strs[0]);
    std::stringstream writer;
    for (int i = 0; i < unroll; i++) {
        if (m_src_dtype == "f16") {
            auto input_render = StringTemplate::StringTemplateArgs()
                                        .add("idx", i)
                                        .add("input_reg", strs[2 * i + 1])
                                        .add("dst_reg", strs[2 * i + 2]);

            writer << input_render.render(R"(
                GI_FLOAT16_t input_reg_${idx} = ${input_reg};
                GI_FLOAT16_t dst_reg_${idx}; 
                {
                    GI_FLOAT32_V2_t fp32 = GiCastFloat16ToFloat32(input_reg_${idx});
                    GI_FLOAT32_t low = GiGetSubVectorFloat32V2(fp32, 0);
                    GI_FLOAT32_t high = GiGetSubVectorFloat32V2(fp32, 1);
                    low = GiExpPsFloat32(low);
                    high = GiExpPsFloat32(high);
                    dst_reg_${idx} = GiCastFloat32ToFloat16(low, high);
                }

            )");
            writer << "\n GI_FLOAT16_t " << strs[2 * i + 2] << " = dst_reg_" << i
                   << ";";
        } else {
            CC_ASSERT(Utils::is_float_dtype(m_dst_dtype, 32));
            writer << "\n GI_FLOAT32_t " << strs[2 * i + 2] << " = GiExpPsFloat32("
                   << strs[2 * i + 1] << ");";
        }
    }
    return writer.str();
}

std::string ElemwiseGenUnaryExp::GenKernelNaiveUnroll(
        std::vector<std::string> strs) const {
    int unroll = std::stoi(strs[0]);
    auto input_ptr = strs[1];
    auto output_ptr = strs[2];
    std::stringstream writer;
    for (int i = 0; i < unroll; i++) {
        if (m_src_dtype == "f16") {
            auto body_render = StringTemplate::StringTemplateArgs()
                                       .add("idx", i)
                                       .add("src", input_ptr)
                                       .add("dst", output_ptr);
            writer << body_render.render(R"(
                float src_${idx} = FastFp16toFp32(${src}[${idx}]);
                float dst_${idx} = exp(src_${idx});
                (${dst})[${idx}] = FastFp32toFp16(dst_${idx}); 
            )");
        } else {
            CC_ASSERT(Utils::is_float_dtype(m_dst_dtype, 32));
            writer << "\n(" << output_ptr << ")[" << i << "] =  exp((" << input_ptr
                   << ")[" << i << "]);";
        }
    }
    return writer.str();
}

//! Sigmoid
std::string ElemwiseGenUnarySigmoid::GenInlineName() const {
    return "ElemwiseGenUnarySigmoid";
}

std::string ElemwiseGenUnarySigmoid::GenKernelSimdInit(std::vector<std::string>) const {
    std::stringstream writer;
    if (m_src_dtype == "f32") {
        return "GI_FLOAT32_t ones = GiBroadcastFloat32(1.0f);";
    } else if (m_src_dtype == "f16") {
        return "GI_FLOAT16_t ones = GiBroadcastFloat16(1.0f);";
    } else {
        CC_ABORT << " ElemwiseGenUnaryRelu is not supported  dtype: " << m_src_dtype
                 << "\n";
        return "";
    }
    return writer.str();
}

std::string ElemwiseGenUnarySigmoid::GenKernelSimdUnroll(
        std::vector<std::string> strs) const {
    int unroll = std::stoi(strs[0]);
    auto input_ptr = strs[1];
    auto output_ptr = strs[2];
    std::stringstream writer;
    for (int i = 0; i < unroll; i++) {
        std::string input_reg_str = "";
        auto input_render = StringTemplate::StringTemplateArgs().add("idx", i).add(
                "input_reg", strs[2 * i + 1]);
        if (m_i32_to_qs8) {
            input_reg_str = input_render.render(R"(
                GI_FLOAT32_t input_reg_${idx} = GiMultiplyScalerFloat32(GiCastToFloat32(${input_reg}), src_scale);
            )");
        } else if (m_src_dtype == "f16") {
            input_reg_str = input_render.render(R"(
                GI_FLOAT16_t input_reg_${idx} = ${input_reg};
            )");
        } else {
            CC_ASSERT(Utils::is_float_dtype(m_dst_dtype, 32))
                    << "not support dst_type " << m_dst_dtype;
            input_reg_str = input_render.render(R"(
                GI_FLOAT32_t input_reg_${idx} = ${input_reg};
            )");
        }
        writer << input_reg_str;
        if (m_src_dtype == "f16") {
            writer << "\n GI_FLOAT16_t dst_temp_" << i
                   << " = GiSigmoidPsFloat16(input_reg_" << i << ");";
        } else {
            writer << "\n GI_FLOAT32_t dst_temp_" << i
                   << " = GiSigmoidPsFloat32(input_reg_" << i << ");";
        }
        if (m_i32_to_qs8) {
            std::string quant_temp = R"(
                GI_INT8_t ${dst_reg_name} = GiCvtFromFloat32ToInt8(GiMultiplyScalerFloat32(dst_temp_${idx}, dst_scale));
            )";
            writer << StringTemplate::StringTemplateArgs()
                              .add("dst_reg_name", strs[2 * i + 2])
                              .add("idx", i)
                              .render(quant_temp);
        } else if (m_src_dtype == "f16") {
            writer << "\n GI_FLOAT16_t " << strs[2 * i + 2] << " = dst_temp_" << i
                   << ";";
        } else {
            CC_ASSERT(Utils::is_float_dtype(m_dst_dtype, 32));
            writer << "\n GI_FLOAT32_t " << strs[2 * i + 2] << " = dst_temp_" << i
                   << ";";
        }
    }
    return writer.str();
}

std::string ElemwiseGenUnarySigmoid::GenKernelNaiveUnroll(
        std::vector<std::string> strs) const {
    int unroll = std::stoi(strs[0]);
    auto input_ptr = strs[1];
    auto output_ptr = strs[2];
    std::stringstream writer;
    auto body_render = StringTemplate::StringTemplateArgs()
                               .add("dst_ptr", output_ptr)
                               .add("src_ptr", input_ptr);
    for (int i = 0; i < unroll; i++) {
        if (m_i32_to_qs8) {
            writer << body_render.render(R"(
                float res = 1.f / ( 1.f + expf(-(${src_ptr}[0] * src_scale)));
                int res_i32 = (int)roundf(res * dst_scale);
                res_i32 = res_i32 > 127 ? 127 : res_i32;
                res_i32 = res_i32 < -128 ? -128 : res_i32;
                (${dst_ptr})[0] = res_i32;
            )");
        } else if (m_src_dtype == "f16") {
            writer << body_render.render(R"(
                float input = FastFp16toFp32((${src_ptr})[0]);
                float output = 1.f / ( 1.f + expf(-input));
                (${dst_ptr})[0] = FastFp32toFp16(output);
            )");
        } else {
            CC_ASSERT(Utils::is_float_dtype(m_dst_dtype, 32));
            writer << body_render.render(R"(
                (${dst_ptr})[0] =  1.f / ( 1.f + expf(-(${src_ptr})[0]));
            )");
        }
    }
    return writer.str();
}

//! HSWISH
std::string ElemwiseGenUnaryHswish::GenInlineName() const {
    return "ElemwiseGenUnaryHswish";
}
std::string ElemwiseGenUnaryHswish::GenKernelSimdInit(std::vector<std::string>) const {
    return "";
}

std::string ElemwiseGenUnaryHswish::GenKernelSimdUnroll(
        std::vector<std::string> strs) const {
    int unroll = std::stoi(strs[0]);
    auto input_ptr = strs[1];
    auto output_ptr = strs[2];
    std::stringstream writer;
    for (int i = 0; i < unroll; i++) {
        std::string input_reg_str = "";
        auto input_render = StringTemplate::StringTemplateArgs().add("idx", i).add(
                "input_reg", strs[2 * i + 1]);
        if (m_i32_to_qs8) {
            input_reg_str = input_render.render(R"(
                GI_FLOAT32_t input_reg_${idx} = GiMultiplyScalerFloat32(GiCastToFloat32(${input_reg}), src_scale);
            )");
        } else if (m_src_dtype == "f16") {
            input_reg_str = input_render.render(R"(
                GI_FLOAT16_t input_reg_${idx} = ${input_reg};
            )");
        } else {
            CC_ASSERT(Utils::is_float_dtype(m_dst_dtype, 32))
                    << "not support dst_type " << m_dst_dtype;
            input_reg_str = input_render.render(R"(
                GI_FLOAT32_t input_reg_${idx} = ${input_reg};
            )");
        }
        writer << input_reg_str;
        std::string input_temp;
        if (m_src_dtype == "f16") {
            input_temp = R"(
                 GI_FLOAT16_t dst_temp_${idx}; 
                {
                    GI_FLOAT32_V2_t fp32 = GiCastFloat16ToFloat32(input_reg_${idx});
                    GI_FLOAT32_t low = GiGetSubVectorFloat32V2(fp32, 0);
                    GI_FLOAT32_t high = GiGetSubVectorFloat32V2(fp32, 1);
                    low = GiHSwishFloat32(low);
                    high = GiHSwishFloat32(high);
                    dst_temp_${idx} = GiCastFloat32ToFloat16(low, high);
                } 
            )";

        } else {
            input_temp = R"(
                GI_FLOAT32_t dst_temp_${idx} = GiHSwishFloat32(input_reg_${idx});
            )";
        }
        writer << input_render.render(input_temp);

        if (m_i32_to_qs8) {
            std::string quant_temp = R"(
                GI_INT8_t ${dst_reg_name}= GiCvtFromFloat32ToInt8(GiMultiplyScalerFloat32(dst_temp_${idx}, dst_scale));
            )";
            writer << StringTemplate::StringTemplateArgs()
                              .add("dst_reg_name", strs[2 * i + 2])
                              .add("idx", i)
                              .render(quant_temp);
        } else if (m_src_dtype == "f16") {
            writer << "\n GI_FLOAT16_t " << strs[2 * i + 2] << " = dst_temp_" << i
                   << ";";
        } else {
            CC_ASSERT(Utils::is_float_dtype(m_dst_dtype, 32));
            writer << "\n GI_FLOAT32_t " << strs[2 * i + 2] << " = dst_temp_" << i
                   << ";";
        }
    }
    return writer.str();
}

std::string ElemwiseGenUnaryHswish::GenKernelNaiveUnroll(
        std::vector<std::string> strs) const {
    int unroll = std::stoi(strs[0]);
    auto input_ptr = strs[1];
    auto output_ptr = strs[2];
    std::stringstream writer;
    auto body_render = StringTemplate::StringTemplateArgs()
                               .add("dst_ptr", output_ptr)
                               .add("src_ptr", input_ptr);
    for (int i = 0; i < unroll; i++) {
        if (m_i32_to_qs8) {
            writer << body_render.render(R"(
                float temp = ${src_ptr}[0] + 3;
                temp = temp > 6? 6 : temp;
                temp = temp < 0? 0 : temp;
                float res = ${src_ptr}[0] * temp / 6.f;
                int res_i32 = (int)roundf(res * dst_scale);
                res_i32 = res_i32 > 127 ? 127 : res_i32;
                res_i32 = res_i32 < -128 ? -128 : res_i32;
                (${dst_ptr})[0] = res_i32;
            )");
        } else if (m_src_dtype == "f16") {
            writer << body_render.render(R"(

                float temp = FastFp16toFp32(${src_ptr}[0]) + 3;
                temp = temp > 6? 6 : temp;
                temp = temp < 0? 0 : temp;
                (${dst_ptr})[0] =  FastFp32toFp16((temp-3) * temp / 6.f);
            )");

        } else {
            CC_ASSERT(Utils::is_float_dtype(m_dst_dtype, 32));
            writer << body_render.render(R"(
                float temp = ${src_ptr}[0] + 3;
                temp = temp > 6? 6 : temp;
                temp = temp < 0? 0 : temp;
                (${dst_ptr})[0] =  ${src_ptr}[0] * temp / 6.f;
            )");
        }
    }
    return writer.str();
}

// vim: syntax=cpp.doxygen
