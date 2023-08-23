#include "test/kernel/common/benchmark.h"
using namespace megdnn;
using namespace megcc::test;
using namespace megcc::KernelGen;

#ifdef ENABLE_KERNEL_BENCHMARK
TEST(ARMV7, BenchmarkFP32GEMM) {
    Benchmarker<MatrixMulForward> benchmarker(Arch::ARMV7);
    benchmarker.set_kernel_symbol("Armv7_kernel_fp32_matmul_4x12_.*");
    size_t m = 1103, k = 151, n = 179;
    size_t a0 = m, a1 = k, b0 = k, b1 = n;
    MatrixMulForward::Param param;
    param.transposeA = false;
    param.transposeB = false;
    benchmarker.set_param(param);
    if (param.transposeA) {
        a0 = k, a1 = m;
    }
    if (param.transposeB) {
        b0 = n, b1 = k;
    }
    benchmarker.execs({{a0, a1}, {b0, b1}, {}}).print();
}

TEST(ARMV7, BenchmarkFP32GEMMNCHW44) {
    Benchmarker<MatrixMulForward> benchmarker(Arch::ARMV7);
    benchmarker.set_kernel_symbol("Armv7_kernel_fp32_matmul_4x12mk4_.*");
    size_t m = 1120, k = 152, n = 180;
    size_t a0 = m, a1 = k, b0 = k, b1 = n;
    MatrixMulForward::Param param;
    param.transposeA = false;
    param.transposeB = false;
    param.format = param::MatrixMul::Format::MK4;
    benchmarker.set_param(param);
    if (param.transposeA) {
        a0 = k, a1 = m;
    }
    if (param.transposeB) {
        b0 = n, b1 = k;
    }
    benchmarker.execs({{a0 / 4, a1 / 4, 4, 4}, {b0 / 4, b1, 4}, {}}).print();
}

TEST(ARMV7, BenchmarkFP32M4N8K4) {
    Benchmarker<MatrixMulForward> benchmarker(Arch::ARMV7);
    benchmarker.set_kernel_symbol("Armv7_kernel_fp32_matmul_4x8mk4_.*");
    benchmarker.set_before_exec_callback(
            megdnn::test::AlgoChecker<MatrixMulForward>("ARMV7_F32_MK4_4x8"));
    for (size_t m : {64, 128})
        for (size_t n : {256, 384})
            for (size_t k : {128, 256}) {
                MatrixMulForward::Param param;
                param.transposeA = false;
                param.transposeB = false;
                param.format = param::MatrixMul::Format::MK4;
                benchmarker.set_param(param);
                benchmarker.execs({{m / 4, k / 4, 4, 4}, {k / 4, n, 4}, {}}).print();
            }
}

TEST(ARMV7, BenchmarkInt8x8x32MatMulMK4) {
    Benchmarker<MatrixMulForward> benchmarker(Arch::ARMV7, 0);
    benchmarker.set_before_exec_callback(
            megdnn::test::AlgoChecker<MatrixMulForward>("ARMV7_INT8X8X32_MK4_4X2X16"));
    MatrixMulForward::Param param;
    UniformIntRNG rng(-127, 127);
    benchmarker.set_rng(0, &rng);
    benchmarker.set_rng(1, &rng);
    benchmarker.set_dtype(0, dtype::Int8())
            .set_dtype(1, dtype::Int8())
            .set_dtype(2, dtype::Int32());
    benchmarker.set_kernel_symbol("Armv7_kernel_int8x8x32_matmul_mk4_.*");
    param.transposeA = false;
    param.transposeB = false;
    param.format = param::MatrixMul::Format::MK4;
    benchmarker.set_param(param);
    for (size_t m : {64, 128})
        for (size_t n : {256, 384})
            for (size_t k : {128, 256}) {
                auto result =
                        benchmarker.execs({{m / 4, k / 4, 4, 4}, {k / 4, n, 4}, {}});
                printf("m=%zu, n=%zu, k=%zu\n", m, n, k);
                result.print();
            }
}

#endif
