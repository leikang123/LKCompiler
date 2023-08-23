#include "WinogradCommon.h"
#include <memory>
#include "Common/ConvKernel.h"
#include "GeneralIntrinsic/Activation.h"
#include "Utils/StringTemplate.h"
#include "Utils/Utils.h"
#include "compiler/KernelGen/KernelGen.h"

using namespace megcc;
using namespace KernelGen;
using namespace GeneralIntrinsic;
namespace {
std::string workspace_template(
        TContext* ctx, WinogradStrategyBase* strategy, uint32_t pack_c_size) {
    int nr_operands = ctx->getAttrInt("nr_operands");
    std::vector<CCOperand> operands;
    for (int i = 0; i < nr_operands; i++) {
        operands.push_back(ctx->getAttrOprand("operand:" + std::to_string(i)));
    }
    uint32_t src_specifier_size = Utils::get_dtype_size(operands[0].dtype);
    uint32_t dst_specifier_size =
            Utils::get_dtype_size(operands[operands.size() - 1].dtype);
    return StringTemplate::StringTemplateArgs()
            .add("pack_c_size", pack_c_size)
            .add("KernelSize", strategy->GetKernelSize())
            .add("OutputBlockSize", strategy->GetOutputBlockSize())
            .add("tile_per_loop", strategy->GetTileSize())
            .add("src_specifier_size", src_specifier_size)
            .add("dst_specifier_size", dst_specifier_size)
            .render(R"({
        TINYNN_ASSERT(workspace);
        uint32_t PACK_C_SIZE = ${pack_c_size};
        uint32_t Align = 64;
        uint32_t KernelSize = ${KernelSize};
        uint32_t OutputBlockSize = ${OutputBlockSize};
        uint32_t Alpha = OutputBlockSize + KernelSize - 1;

        const Layout in_layout = inputs[1]->layout;

        size_t OC = in_layout.dims[0] * PACK_C_SIZE;
        size_t IC = in_layout.dims[1] * PACK_C_SIZE;
        if (in_layout.nr_dim == 7) {
            OC = in_layout.dims[1] * PACK_C_SIZE;
            IC = in_layout.dims[2] * PACK_C_SIZE;
        }

        //! input : (alpha, alpha, unit_tile_size, IC) or (alpha, alpha,
        //! ICB, unit_tile_size, IC_BLOCK_SIZE)
        size_t input_transform_buf_size =
                Alpha * Alpha * IC * ${tile_per_loop} * ${src_specifier_size};
        input_transform_buf_size = 
                (input_transform_buf_size + Align -1) / Align * Align;

        //! output : (alpha, alpha, unit_tile_size, OC) or
        //! (alpha, alpha, OCB, unit_tile_size, OC_BLOCK_SIZE)
        size_t output_transform_buf_size =
                Alpha * Alpha * OC * ${tile_per_loop} * ${dst_specifier_size};
        output_transform_buf_size = 
                (output_transform_buf_size + Align -1) / Align * Align;

        size_t transform_mid_buf_size = 2 * Alpha * Alpha * ${src_specifier_size} *
                PACK_C_SIZE;
        transform_mid_buf_size = (transform_mid_buf_size + Align -1) / Align * Align; 
        *workspace = input_transform_buf_size + output_transform_buf_size
        + transform_mid_buf_size;
        return TinyNN_SUCCESS;
    }

                    )");
}

std::string init_template(
        TContext* ctx, WinogradStrategyBase* strategy, uint32_t pack_c_size) {
    int nr_operands = ctx->getAttrInt("nr_operands");
    std::vector<CCOperand> operands;
    for (int i = 0; i < nr_operands; i++) {
        operands.push_back(ctx->getAttrOprand("operand:" + std::to_string(i)));
    }
    auto src_specifier = Utils::cvt_dtype_specifier(operands[0].dtype);
    auto dst_specifier =
            Utils::cvt_dtype_specifier(operands[operands.size() - 1].dtype);
    uint32_t nr_out_weight = 1;
    std::string dtype_enum = Utils::get_tinynn_dtype_string(operands[0].dtype);
    std::string common_def = R"(
    int PACK_C_SIZE = ${pack_c_size};
    Tensor* in_weights = inputs[1];
    Layout in_layout = inputs[1]->layout;

    uint32_t OC, IC, Group;
    if(in_layout.nr_dim == 7){
        OC = in_layout.dims[1] * PACK_C_SIZE;
        IC = in_layout.dims[2] * PACK_C_SIZE;
        Group = in_layout.dims[0];
    } else {
        OC = in_layout.dims[0] * PACK_C_SIZE;
        IC = in_layout.dims[1] * PACK_C_SIZE;
        Group = 1;
    }
    uint32_t KernelSize = ${KernelSize};
    uint32_t OutputBlockSize = ${OutputBlockSize};
    uint32_t Alpha = OutputBlockSize + KernelSize - 1;)";

    std::stringstream common_writer;
    common_writer << StringTemplate::StringTemplateArgs()
                             .add("KernelSize", strategy->GetKernelSize())
                             .add("OutputBlockSize", strategy->GetOutputBlockSize())
                             .add("pack_c_size", pack_c_size)
                             .render(common_def);
    common_def = common_writer.str();

    std::string fill_weight_attr = StringTemplate::StringTemplateArgs()
                                           .add("dtype_enum", dtype_enum)
                                           .render(R"(
    out_weights->layout.nr_dim = 4;
    out_weights->layout.dims[0] = Group;
    out_weights->layout.dims[1] = Alpha * Alpha;
    out_weights->layout.dims[2] = OC;
    out_weights->layout.dims[3] = IC;
    out_weights->layout.stride[0] = out_weights->layout.dims[1] * out_weights->layout.dims[2] * out_weights->layout.dims[3];
    out_weights->layout.stride[1] = out_weights->layout.dims[2] * out_weights->layout.dims[3];
    out_weights->layout.stride[2] = out_weights->layout.dims[3];
    out_weights->layout.stride[3] = 1;
    out_weights->dtype.type_enum=${dtype_enum};
    out_weights->name = in_weights->name;)");

    std::string fill_weight_transform = R"(
    ${dst_specifier}* outptr = out_weights->ptr;
    ${src_specifier}* inptr = in_weights->ptr;
    {
    ${FilterTransform(inptr, outptr, OC, IC)}
    }
    )";
    std::stringstream transform_writer;
    transform_writer << StringTemplate::StringTemplateArgs()
                                .add("FilterTransform",
                                     [&](std::vector<std::string> strs) {
                                         return strategy->WeightTrans(strs);
                                     })
                                .add("src_specifier", src_specifier)
                                .add("dst_specifier", dst_specifier)
                                .render(fill_weight_transform);

    fill_weight_transform = transform_writer.str();

    std::stringstream ss;
    ss << StringTemplate::render_init_body(
            nr_out_weight, fill_weight_attr, fill_weight_transform, common_def);
    return ss.str();
}

std::string kernbody_template(
        TContext* ctx, WinogradStrategyBase* strategy, uint32_t pack_c_size) {
    std::stringstream writer;
    int nr_operands = ctx->getAttrInt("nr_operands");
    std::vector<CCOperand> operands;
    for (int i = 0; i < nr_operands; i++) {
        operands.push_back(ctx->getAttrOprand("operand:" + std::to_string(i)));
    }
    auto src_specifier = Utils::cvt_dtype_specifier(operands[0].dtype);
    auto dst_specifier =
            Utils::cvt_dtype_specifier(operands[operands.size() - 1].dtype);
    std::string framework = R"(
    //! weights is transformed
    Tensor* weight = inputs[1];
    Layout weight_layout = inputs[1]->layout;

    Tensor* input = inputs[0];
    Layout in_layout = inputs[0]->layout;

    Tensor* output = outputs[0];
    Layout out_layout = outputs[0]->layout;

    const uint32_t PACK_C_SIZE = ${pack_c_size};
    const uint32_t Align = 64;

    size_t N = out_layout.dims[0];
    size_t OC = out_layout.dims[1] * PACK_C_SIZE;
    size_t IC = in_layout.dims[1] * PACK_C_SIZE;
    size_t IH = in_layout.dims[2];
    size_t IW = in_layout.dims[3];
    size_t OH = out_layout.dims[2];
    size_t OW = out_layout.dims[3];
    size_t PH = ${pad_h};
    size_t PW = ${pad_w};

    uint32_t Group = 1;
    if(in_layout.nr_dim == 7){
        Group = weight_layout.dims[0];
    }
    uint32_t KernelSize = ${KernelSize};
    uint32_t OutputBlockSize = ${OutputBlockSize};
    uint32_t Alpha = OutputBlockSize + KernelSize - 1;

    uint32_t tiles_h = (OH + OutputBlockSize -1) / OutputBlockSize;
    uint32_t tiles_w = (OW + OutputBlockSize -1) / OutputBlockSize;
    uint32_t nr_tiles = tiles_h * tiles_w;
    uint32_t nr_tiles_per_loop = ${nr_tiles_per_loop};

    size_t input_transform_buf_size =
                Alpha * Alpha * IC * nr_tiles_per_loop * sizeof(${src_specifier});
    input_transform_buf_size = 
                (input_transform_buf_size + Align -1) / Align * Align;
    
    size_t output_transform_buf_size =
                Alpha * Alpha * OC * nr_tiles_per_loop * sizeof(${dst_specifier});
    output_transform_buf_size = 
                (output_transform_buf_size + Align -1) / Align * Align;

    ${src_specifier}* transform_input_ptr = workspace->ptr;
    ${src_specifier}* transform_output_ptr = transform_input_ptr +
                        input_transform_buf_size / sizeof(${dst_specifier});
    
    ${src_specifier}* transform_mid_ptr = transform_output_ptr +
                        output_transform_buf_size / sizeof(${dst_specifier});

    const ${src_specifier}* input_ptr = input->ptr;
    const ${src_specifier}* weight_ptr = weight->ptr;
    ${dst_specifier}* output_ptr = output->ptr;
    const ${src_specifier}* bias_ptr = ${BiasPtr};

    size_t group_input_offset = IC * IH * IW;
    size_t group_weight_offset = Alpha * Alpha * OC * IC;
    size_t group_output_offset = OC * OH * OW;

    for(uint32_t n = 0; n < N; n++){
        for (uint32_t group = 0; group < Group; group++){
            const ${src_specifier}* wptr = weight_ptr + group * group_weight_offset;
            const ${src_specifier}* inptr = input_ptr + (n * Group + group) *
                                 group_input_offset;
            ${dst_specifier}* outptr = output_ptr + (n * Group + group)* group_output_offset;
            const ${src_specifier}* bptr = NULL;
            if(bias_ptr) bptr = bias_ptr + group * OC;

            for(uint32_t tile_id = 0; tile_id < nr_tiles; tile_id += nr_tiles_per_loop) {
                    uint32_t nr_tiles_in_loop = nr_tiles_per_loop > nr_tiles -
                                tile_id? nr_tiles - tile_id : nr_tiles_per_loop;
                    //! input transform BTdB
                    {
                    ${InputTransform(inptr, transform_input_ptr, IH, IW, IC, PH, PW, tile_id, nr_tiles_in_loop)}
                    }

                    //! batched Matmul
                    const ${src_specifier}* A_ptr = wptr;
                    ${src_specifier}* B_ptr = transform_input_ptr;
                    ${dst_specifier}* C_ptr = transform_output_ptr;
                    uint32_t LDA = IC * PACK_C_SIZE;
                    uint32_t LDB = nr_tiles_in_loop * PACK_C_SIZE;
                    uint32_t LDC = nr_tiles_in_loop * PACK_C_SIZE;

                
                    {
                    ${BatchedMatmul(A_ptr, LDA, B_ptr, LDB, C_ptr, LDC, OC, IC, nr_tiles_in_loop)}
                    }

                    //! output transform: ATmA

                    {
                    ${OutputTransform(transform_output_ptr, outptr, bptr, OH, OW, OC, tile_id, nr_tiles_in_loop)}
                    }

                }
        }
    })";
    std::string bias_ptr = ConvImpl::is_bias(ctx) ? "inputs[2]->ptr" : "NULL";
    writer << StringTemplate::StringTemplateArgs(ctx)
                      .add("KernelSize", strategy->GetKernelSize())
                      .add("OutputBlockSize", strategy->GetOutputBlockSize())
                      .add("nr_tiles_per_loop", strategy->GetTileSize())
                      .add("BiasPtr", bias_ptr)
                      .add_ctx_int("pad_h")
                      .add_ctx_int("pad_w")
                      .add("InputTransform",
                           [&](std::vector<std::string> strs) {
                               return strategy->InputFeatureTrans(strs);
                           })
                      .add("BatchedMatmul",
                           [&](std::vector<std::string> strs) {
                               return strategy->BatchedMatMul(strs, dst_specifier);
                           })
                      .add("OutputTransform",
                           [&](std::vector<std::string> strs) {
                               return strategy->OutputFeatureTrans(strs, ctx);
                           })
                      .add("pack_c_size", pack_c_size)
                      .add("src_specifier", src_specifier)
                      .add("dst_specifier", dst_specifier)
                      .render(framework);
    return writer.str();
}

}  // namespace

std::string WinogradStrategyBase::BatchedMatMul(
        const std::vector<std::string>& strs, const std::string& ctype_specifier) {
    std::string matmul_compute = R"(
    for(uint32_t i =0; i< Alpha; i++){
        for(uint32_t j=0; j<Alpha; j++){
            const ${ctype_specifier}* a_ptr = ${A_ptr} +
                (i * Alpha + j) * ${OC} * ${IC};
            ${ctype_specifier}* b_ptr = ${B_ptr} +
                (i * Alpha + j) * ${nr_tiles_in_loop} * ${IC};
            ${ctype_specifier}* c_ptr = ${C_ptr} +
                (i * Alpha + j) * ${nr_tiles_in_loop} * ${OC};
            ${MatMul}(a_ptr, ${LDA}, b_ptr, ${LDB}, c_ptr, ${LDC}, ${OC}, 
                    ${nr_tiles_in_loop}, ${IC});
        }
    })";

    std::stringstream ss;
    ss << StringTemplate::StringTemplateArgs()
                    .add("MatMul", DependMatmulSymbol())
                    .add("A_ptr", strs[0])
                    .add("LDA", strs[1])
                    .add("B_ptr", strs[2])
                    .add("LDB", strs[3])
                    .add("C_ptr", strs[4])
                    .add("LDC", strs[5])
                    .add("OC", strs[6])
                    .add("IC", strs[7])
                    .add("nr_tiles_in_loop", strs[8])
                    .add("ctype_specifier", ctype_specifier)
                    .render(matmul_compute);
    return ss.str();
}

std::string WinogradFrameNchw44::GenGetWorkSpaceCode(
        TContext* context, WinogradStrategyBase* strategy) {
    CC_ASSERT(context->getAttrStr("format") == "NCHW44")
            << "format mismatch  now: " << context->getAttrStr("format")
            << ", expect: NCHW44\n";
    auto WeightShape = context->getAttrOprand("operand:1").shape;
    return workspace_template(context, strategy, 4);
}

std::string WinogradFrameNchw44::GenInitCode(
        TContext* ctx, WinogradStrategyBase* strategy) {
    return init_template(ctx, strategy, 4);
}

std::string WinogradFrameNchw44::GenKernelBodyCode(
        TContext* ctx, WinogradStrategyBase* strategy) {
    return kernbody_template(ctx, strategy, 4);
}

std::string WinogradFrameNchw88::GenGetWorkSpaceCode(
        TContext* context, WinogradStrategyBase* strategy) {
    CC_ASSERT(context->getAttrStr("format") == "NCHW88")
            << "format mismatch  now: " << context->getAttrStr("format")
            << ", expect: NCHW88\n";
    auto WeightShape = context->getAttrOprand("operand:1").shape;
    return workspace_template(context, strategy, 8);
}

std::string WinogradFrameNchw88::GenInitCode(
        TContext* ctx, WinogradStrategyBase* strategy) {
    return init_template(ctx, strategy, 8);
}

std::string WinogradFrameNchw88::GenKernelBodyCode(
        TContext* ctx, WinogradStrategyBase* strategy) {
    return kernbody_template(ctx, strategy, 8);
}

// vim: syntax=cpp.doxygen
