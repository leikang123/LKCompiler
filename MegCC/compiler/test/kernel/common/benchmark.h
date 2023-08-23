#pragma once

// clang-format off
#include <string>

#include "test/kernel/common/performance.h"
#include "test/kernel/common/dnn_proxy.h"
#include "test/kernel/common/runner.h"
#include "dnn_algo_checker.h"
#include <functional>
// clang-format on

namespace megcc {
namespace test {

using TensorNDArray = megdnn::SmallVector<megdnn::TensorND>;
using TensorLayoutArray = megdnn::SmallVector<megdnn::TensorLayout>;
using TensorShapeArray = megdnn::SmallVector<megdnn::TensorShape>;

template <typename Opr>
class Benchmarker : public Runner<Opr> {
public:
    using Param = typename Opr::Param;
    using BeforeExecCallback = std::function<void(Opr*, const TensorNDArray&)>;
    Benchmarker(
            KernelGen::Arch arch = KernelGen::Arch::BAREMETAL, const int dnn_level = 0)
            : Runner<Opr>(arch, dnn_level), m_arch(arch), m_kernel_symbol(".*") {
        m_benchmark_option.disable_check = true;
        m_benchmark_option.valid_megcc_performance = true;
        m_benchmark_option.valid_dnn_performance = true;
    };

    TensorLayoutArray make_layouts(const TensorShapeArray& shapes);
    PerformanceResultPair exec(const TensorShapeArray& shapes) {
        return exec(Runner<Opr>::make_layouts(shapes, m_dtype, m_fmt));
    }

    //! set a callback to be invoked before executing the operator
    Benchmarker& set_before_exec_callback(const BeforeExecCallback& cb) {
        m_before_exec_callback = cb;
        return *this;
    }

    PerformanceResultPair exec(TensorLayoutArray layouts);

    //! explicitly require argument to be TensorShape
    PerformanceResultPair execs(const TensorShapeArray& shapes) { return exec(shapes); }

    //! explicitly require argument to be TensorLayout
    PerformanceResultPair execl(const TensorLayoutArray& layouts) {
        return exec(layouts);
    }
    Benchmarker& set_param(Param param) {
        m_param = param;
        return *this;
    }
    Benchmarker& set_dtype(size_t idx, megdnn::DType dtype) {
        m_dtype[idx] = dtype;
        return *this;
    }
    Benchmarker& set_kernel_symbol(std::string kernel_symbol) {
        m_kernel_symbol = kernel_symbol;
        return *this;
    }
    Benchmarker& set_benchmark_option(BenchmarkOption benchmark_option) {
        m_benchmark_option = benchmark_option;
        return *this;
    }
    Benchmarker& set_rng(size_t idx, RNG* rng) {
        m_rng[idx] = rng;
        return *this;
    }
    Benchmarker& set_proxy(const megdnn::test::DnnOprProxy<Opr>& proxy) {
        m_dnn_proxy = proxy;
        return *this;
    }
    static std::string format_result(const PerformanceResultPair& perf_result);

private:
    Param m_param;
    BenchmarkOption m_benchmark_option;
    std::unique_ptr<Opr> m_dnn_opr;
    std::unordered_map<size_t, RNG*> m_rng;
    std::unordered_map<size_t, megdnn::DType> m_dtype;
    std::unordered_map<size_t, megdnn::TensorFormat> m_fmt;
    std::unique_ptr<RNG> m_default_rng;
    KernelGen::Arch m_arch;
    std::string m_kernel_symbol;

    BeforeExecCallback m_before_exec_callback = nullptr;
    megdnn::test::DnnOprProxy<Opr> m_dnn_proxy;
};
}  // namespace test

}  // namespace megcc
