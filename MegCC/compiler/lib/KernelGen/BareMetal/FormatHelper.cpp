#include "FormatHelper.h"
#include <sstream>

using namespace megcc;
using namespace KernelGen;
using namespace BareMetal;

std::string GenFormatIter::gen_inline_format_iter_symbol(std::string format_str) {
    return "get_linear_addr_" + format_str;
}

std::string GenFormatIter::gen_inline_format_iter_body(std::string format_str) {
    std::stringstream ss;
    ss << R"(static inline size_t )" << gen_inline_format_iter_symbol(format_str);
    ss << R"((const int n, const int c, const int h,
                                     const int w, const int* stride,
                                     const bool is_output) {)";
    if (format_str == "NCHW") {
        ss << R"(
            size_t offset = (size_t)n * stride[0] + c * stride[1] + h * stride[2] + w * stride[3];
            return offset;
        )";
    } else if (format_str == "NCHW44") {
        ss << R"(
            return (size_t)n * stride[0] + c / 4 * stride[1] + h * stride[2] + w * stride[3] + (c % 4) * stride[4];
        )";
    } else {
        CC_ASSERT(format_str == "NCHW88") << "format not support\n";
        ss << R"(
            return (size_t)n * stride[0] + c / 8 * stride[1] + h * stride[2] + w * stride[3] + (c % 8) * stride[4];
        )";
    }
    ss << "}\n";
    return ss.str();
}

// vim: syntax=cpp.doxygen
