#pragma once

#include <limits>
#include "compiler/Common/Logger.h"
#include "compiler/Common/TContext.h"
namespace megcc {
namespace KernelGen {
namespace Utils {

enum DtypeEnum {
    unknown = 0,
    float32 = 1,
    int32 = 2,
    int8 = 3,
    uint8 = 4,
    qsi8 = 5,
    qsi32 = 6,
    float16 = 7,
};

static inline bool is_shape_dynamic(const std::vector<size_t>& shape) {
    for (auto x : shape) {
        if (x >= std::numeric_limits<size_t>::max()) {
            return true;
        }
    }
    return false;
}

static inline bool is_any_op_dynamic(TContext* ctx) {
    int nr_operand = ctx->getAttrInt("nr_operands");
    for (int i = 0; i < nr_operand; ++i) {
        auto operand = ctx->getAttrOprand("operand:" + std::to_string(i));
        if (is_shape_dynamic(operand.shape)) {
            return true;
        }
    }
    return false;
}

static inline bool is_float_dtype(const std::string& dtype, int bit_width = -1) {
    if (bit_width == 32 && dtype == "f32") {
        return true;
    } else if (bit_width == 16 && dtype == "f16") {
        return true;
    } else if (bit_width != -1) {
        return false;
    } else {
        if (dtype == "f32") {
            return true;
        } else {
            return false;
        }
    }
}

static inline bool is_int_dtype(const std::string& dtype, int bit_width = -1) {
    if (bit_width == 8 && (dtype == "i8" || dtype == "si8" || dtype == "ui8")) {
        return true;
    } else if (
            bit_width == 32 &&
            (dtype == "i32" || dtype == "si32" || dtype == "qsi32")) {
        return true;
    } else if (bit_width == 16 && (dtype == "i16" || dtype == "ui16")) {
        return true;
    } else if (bit_width != -1) {
        return false;
    } else {
        if (dtype == "i32" || dtype == "i8" || dtype == "si8" || dtype == "ui8") {
            return true;
        } else {
            return false;
        }
    }
}

static inline bool is_quant_dtype(const std::string& dtype, int bit_width = -1) {
    const std::string q_prefix = "qsi";
    if (dtype.size() > 3 && dtype.substr(0, 3) == q_prefix) {
        if (bit_width > 0) {
            std::string prefix = q_prefix + std::to_string(bit_width);
            return dtype.substr(0, prefix.size()) == prefix;
        } else {
            return true;
        }
    }
    return false;
}

static inline DtypeEnum get_dtype_enum(const std::string& dtype) {
    if (is_float_dtype(dtype, 32)) {
        return DtypeEnum::float32;
    } else if (is_int_dtype(dtype, 32)) {
        return DtypeEnum::int32;
    } else if (dtype == "ui8") {
        return DtypeEnum::uint8;
    } else if (is_quant_dtype(dtype, 8)) {
        return DtypeEnum::qsi8;
    } else if (is_quant_dtype(dtype, 32)) {
        return DtypeEnum::qsi32;
    } else if (is_float_dtype(dtype, 16)) {
        return DtypeEnum::float16;
    } else {
        CC_ASSERT(is_int_dtype(dtype, 8) || is_quant_dtype(dtype, 8))
                << "not support " << dtype;
        return DtypeEnum::int8;
    }
}

static inline std::string get_tinynn_dtype_string(const std::string& dtype) {
    if (is_float_dtype(dtype, 32)) {
        return "TinyNN_FLOAT";
    } else if (is_int_dtype(dtype, 32)) {
        return "TinyNN_INT";
    } else if (dtype == "ui8") {
        return "TinyNN_UINT8";
    } else if (is_quant_dtype(dtype, 8)) {
        return "TinyNN_QINT8";
    } else if (is_quant_dtype(dtype, 32)) {
        return "TinyNN_QINT32";
    } else if (is_float_dtype(dtype, 16)) {
        return "TinyNN_FLOAT16";
    } else {
        CC_ASSERT(is_int_dtype(dtype, 8) || is_quant_dtype(dtype, 8))
                << "not support " << dtype;
        return "TinyNN_INT8";
    }
}
static inline size_t get_dtype_size(const std::string& dtype) {
    if (is_float_dtype(dtype, 32) || is_int_dtype(dtype, 32) ||
        is_quant_dtype(dtype, 32)) {
        return 4;
    } else if (is_int_dtype(dtype, 16) || is_float_dtype(dtype, 16)) {
        return 2;
    } else {
        CC_ASSERT(is_int_dtype(dtype, 8) || is_quant_dtype(dtype, 8))
                << "not support " << dtype;
        return 1;
    }
}

template <typename T>
T round_up(T dividend, T divisor) {
    static_assert(std::is_integral<T>::value, "must be integers");
    CC_ASSERT(dividend >= 0);
    CC_ASSERT(divisor > 0);
    return ((dividend + divisor - 1) / divisor) * divisor;
}

static inline std::string cvt_dtype_specifier(const std::string& dtype_str) {
    std::string dtype_c_str;
    if (dtype_str == "f32") {
        dtype_c_str = "float";
    } else if (dtype_str == "ui8") {
        dtype_c_str = "uint8_t";
    } else if (dtype_str == "i8") {
        dtype_c_str = "int8_t";
    } else if (dtype_str == "si8") {
        dtype_c_str = "int8_t";
    } else if (dtype_str == "si32") {
        dtype_c_str = "int";
    } else if (dtype_str == "i32") {
        dtype_c_str = "int";
    } else if (dtype_str.substr(0, 4) == "qsi8") {
        dtype_c_str = "int8_t";
    } else if (dtype_str.substr(0, 5) == "qsi32") {
        dtype_c_str = "int";
    } else if (dtype_str == "f16") {
        dtype_c_str = "gi_float16_t";
    } else {
        CC_ABORT << "not support dtype " << dtype_str << "\n";
    }
    return dtype_c_str;
}

static inline std::string get_common_dtype_specifier(int dtype_size) {
    std::string dtype_c_str;
    switch (dtype_size) {
        case 4:
            return "float";
            break;
        case 2:
            return "uint16_t";
            break;
        case 1:
            return "uint8_t";
            break;
        default:
            CC_ABORT << "not support dtype size " << dtype_size << "\n";
            break;
    }
    return "";
}

static inline CCOperand get_last_operand(TContext* ctx) {
    int last_idx = ctx->getAttrInt("nr_operands") - 1;
    return ctx->getAttrOprand("operand:" + std::to_string(last_idx));
}

static inline bool is_test_mode(TContext* ctx) {
    if (ctx->haveAttr("unitest_mode")) {
        return ctx->getAttrBool("unitest_mode");
    }
    return false;
}

static inline void cv_kern_sym_add_prefix(
        TContext* ctx, std::string prefix, std::stringstream& ss) {
    if (is_test_mode(ctx)) {
        ss << prefix << "_";
    }
}

static inline size_t get_dtype_simd_length(DtypeEnum dtype) {
    switch (dtype) {
        case DtypeEnum::float32:
        case DtypeEnum::int32:
        case DtypeEnum::qsi32:
            return 4;
        case DtypeEnum::float16:
            return 8;
        case DtypeEnum::int8:
        case DtypeEnum::qsi8:
        case DtypeEnum::uint8:
            return 16;
        default:
            CC_ABORT << "unknown dtype for get_dtype_simd_length\n";
            return 4;
    }
}

static inline std::string cvt_dtype_specifier(DtypeEnum dtype) {
    switch (dtype) {
        case DtypeEnum::float32:
            return "float";
        case DtypeEnum::int32:
        case DtypeEnum::qsi32:
            return "int";
        case DtypeEnum::float16:
            return "gi_float16_t";
        case DtypeEnum::int8:
        case DtypeEnum::qsi8:
            return "int8_t";
        case DtypeEnum::uint8:
            return "uint8_t";
        default:
            CC_ABORT << "unknown dtype for cvt_dtype_specifier\n";
            return "";
    }
}

static inline std::string get_dtype_gi_simd_type(DtypeEnum dtype) {
    switch (dtype) {
        case DtypeEnum::float32:
            return "GI_FLOAT32_t";
        case DtypeEnum::int32:
        case DtypeEnum::qsi32:
            return "GI_INT32_t";
        case DtypeEnum::float16:
            return "GI_FLOAT16_t";
        case DtypeEnum::int8:
        case DtypeEnum::qsi8:
            return "GI_INT8_t";
        case DtypeEnum::uint8:
            return "GI_UINT8_t";
        default:
            CC_ABORT << "unknown dtype for get_dtype_gi_simd_type\n";
            return "";
    }
}

static inline std::string get_dtype_gi_type_str(DtypeEnum dtype) {
    switch (dtype) {
        case DtypeEnum::float32:
            return "Float32";
        case DtypeEnum::int32:
        case DtypeEnum::qsi32:
            return "Int32";
        case DtypeEnum::float16:
            return "Float16";
        case DtypeEnum::int8:
        case DtypeEnum::qsi8:
            return "Int8";
        case DtypeEnum::uint8:
            return "Uint8";
        default:
            CC_ABORT << "unknown dtype for get_dtype_gi_simd_type\n";
            return "";
    }
}

std::string ssprintf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

class DtypeHelper {
public:
    DtypeHelper(const std::string& dtype) : m_dtype(get_dtype_enum(dtype)){};
    std::string max();
    std::string min();
    std::string one();
    std::string zero();
    std::string inline_max_func();
    std::string inline_min_func();

private:
    DtypeEnum m_dtype;
};

template <typename T>
std::string to_string(const std::vector<T>& vec) {
    std::stringstream ss;
    for (auto x : vec) {
        ss << std::to_string(x) << ",";
    }
    return ss.str();
}

static inline std::string gen_operande_string(TContext* ctx) {
    int nr_operand = ctx->getAttrInt("nr_operands");
    std::stringstream ss;
    for (int i = 0; i < nr_operand; ++i) {
        auto operand = ctx->getAttrOprand("operand:" + std::to_string(i));
        ss << operand.to_string() << "; ";
    }
    return ss.str();
}

std::vector<std::string> split_string(const std::string& s, const char delim);

}  // namespace Utils
}  // namespace KernelGen
}  // namespace megcc
