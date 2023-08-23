#include "test/kernel/common/benchmark.h"
#include "test/kernel/common/checker.h"
using namespace megdnn;
using namespace megcc::test;
using namespace megcc::KernelGen;
using Format = WarpPerspective::Param::Format;
using BorderMode = WarpPerspective::Param::BorderMode;
using InterpolationMode = WarpPerspective::Param::InterpolationMode;

TEST(GI, WarpAffine) {
    Checker<WarpAffineForward> checker(Arch::BAREMETAL);
    NormalRNG rng;
    checker.set_rng(1, &rng);
    checker.set_epsilon(1e-3);
    checker.set_dtype(0, dtype::Float32());
    WarpAffineForward::Param param;
    param.imode = InterpolationMode::LINEAR;
    for (auto fmt : {Format::NCHW, Format::NHWC}) {
        for (auto bmode :
             {WarpPerspective::BorderMode::WRAP, WarpPerspective::BorderMode::REFLECT,
              WarpPerspective::BorderMode::REPLICATE,
              WarpPerspective::BorderMode::CONSTANT}) {
            printf("format=%d border_mode=%d\n", fmt, bmode);
            param.format = fmt;
            param.border_val = 1.25;
            param.border_mode = bmode;
            checker.set_param(param);

            checker.execs({{1, 13, 13, 17}, {1, 2, 3}, {1, 13, 7, 17}});
            checker.execs({{2, 13, 22, 17}, {2, 2, 3}, {2, 13, 7, 17}});
            checker.execs({{5, 13, 33, 17}, {5, 2, 3}, {5, 13, 7, 17}});
            if (fmt == Format::NHWC) {
                checker.execs({{1, 13, 22, 5}, {1, 2, 3}, {1, 5, 7, 5}});
            }
        }
    }
}

TEST(GI, WarpAffineU8) {
    Checker<WarpAffineForward> checker(Arch::BAREMETAL);
    UniformIntRNG rng(0, 255);
    checker.set_rng(0, &rng);
    checker.set_rng(1, &rng);
    checker.set_dtype(0, dtype::Uint8());
    checker.set_dtype(2, dtype::Uint8());
    WarpAffineForward::Param param;
    param.imode = InterpolationMode::LINEAR;
    for (auto fmt : {Format::NHWC, Format::NCHW})
        for (auto bmode :
             {WarpPerspective::BorderMode::WRAP, WarpPerspective::BorderMode::REFLECT,
              WarpPerspective::BorderMode::REPLICATE,
              WarpPerspective::BorderMode::CONSTANT}) {
            printf("format=%d border_mode=%d\n", fmt, bmode);
            param.format = fmt;
            param.border_val = 3;
            param.border_mode = bmode;
            checker.set_param(param);

            checker.execs({{1, 13, 13, 1}, {1, 2, 3}, {1, 13, 7, 1}});
            checker.execs({{2, 13, 22, 2}, {2, 2, 3}, {2, 13, 7, 2}});
            if (fmt == Format::NHWC) {
                checker.execs({{1, 8, 3, 4}, {1, 2, 3}, {1, 3, 8, 4}});
                checker.execs({{2, 28, 15, 3}, {2, 2, 3}, {2, 5, 7, 3}});
            }
            checker.execs({{5, 13, 33, 3}, {5, 2, 3}, {5, 13, 7, 3}});
        }
}

#ifdef ENABLE_KERNEL_BENCHMARK
TEST(GI, BenchmarkWarpAffine) {
    Benchmarker<WarpAffineForward> benchmarker(Arch::BAREMETAL);
    WarpAffineForward::Param param;
    param.format = WarpAffineForward::Param::Format::NHWC;
    param.imode = WarpAffineForward::Param::InterpolationMode::LINEAR;
    param.border_mode = WarpAffineForward::BorderMode::REPLICATE;
    param.border_val = 5;
    benchmarker.set_param(param);
    benchmarker.set_dtype(0, dtype::Uint8());
    benchmarker.set_dtype(2, dtype::Uint8());

    benchmarker.execs({{1, 1080, 1920, 3}, {1, 2, 3}, {1, 720, 1280, 3}}).print();

    benchmarker.execs({{1, 1080, 1920, 1}, {1, 2, 3}, {1, 720, 1280, 1}}).print();

    benchmarker.execs({{1, 1080, 1920, 2}, {1, 2, 3}, {1, 720, 1280, 2}}).print();
}
#endif
