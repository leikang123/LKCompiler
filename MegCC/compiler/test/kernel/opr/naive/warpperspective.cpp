#include "test/kernel/common/checker.h"
#include "test/kernel/common/rng.h"
using namespace megdnn;
using namespace megcc::test;
using namespace megcc::KernelGen;
using Format = WarpPerspective::Param::Format;
using BorderMode = WarpPerspective::Param::BorderMode;
using InterpolationMode = WarpPerspective::Param::InterpolationMode;

TEST(NAIVE, WarpPerspective) {
    Checker<WarpPerspectiveForward> checker(Arch::BAREMETAL);
    checker.set_kernel_symbol("kernel_.*");
    checker.set_epsilon(1 + 1e-4);
    WarpPerspectiveMatRNG rng;
    checker.set_rng(1, &rng);
    WarpPerspectiveForward::Param param;
    param.imode = InterpolationMode::LINEAR;
    for (auto fmt : {Format::NCHW})
        for (auto bmode :
             {WarpPerspective::BorderMode::WRAP, WarpPerspective::BorderMode::REFLECT,
              WarpPerspective::BorderMode::REPLICATE,
              WarpPerspective::BorderMode::CONSTANT}) {
            param.format = fmt;
            param.border_val = 5;
            param.bmode = bmode;
            checker.set_param(param);
            checker.set_dtype(0, dtype::Uint8());
            checker.set_dtype(2, dtype::Uint8());
            checker.execs({{1, 13, 13, 17}, {1, 3, 3}, {1, 13, 7, 17}});
            checker.execs({{2, 13, 33, 17}, {2, 3, 3}, {2, 13, 7, 17}});
            checker.execs({{5, 7, 22, 33}, {5, 3, 3}, {5, 7, 7, 5}});
        }

    for (auto fmt : {Format::NCHW, Format::NHWC})
        for (auto bmode :
             {WarpPerspective::BorderMode::WRAP, WarpPerspective::BorderMode::REFLECT,
              WarpPerspective::BorderMode::REPLICATE,
              WarpPerspective::BorderMode::CONSTANT}) {
            param.format = fmt;
            param.border_val = 1.25;
            param.bmode = bmode;
            checker.set_param(param);
#if ENABLE_KERNEL_FP16
            checker.set_epsilon(2e-3);
            for (auto dtype : {(DType)dtype::Float16(), (DType)dtype::Float32()})
#else
            checker.set_epsilon(1e-4);
            for (auto dtype : {(DType)dtype::Float32()})
#endif
            {
                checker.set_dtype(0, dtype);
                checker.set_dtype(2, dtype);
                checker.execs({{1, 13, 13, 17}, {1, 3, 3}, {1, 13, 7, 17}});
                checker.execs({{1000, 13, 13, 17}, {1000, 3, 3}, {1000, 13, 7, 17}});
                checker.execs({{2, 13, 22, 17}, {2, 3, 3}, {2, 13, 7, 17}});
                checker.execs({{5, 13, 33, 17}, {5, 3, 3}, {5, 13, 7, 17}});
            }
        }
}
TEST(NAIVE, WarpPerspectiveCv) {
    Checker<WarpPerspectiveForward> checker(Arch::BAREMETAL);
    checker.set_kernel_symbol("kernel_.*");
    checker.set_epsilon(2e-3);
    checker.set_dtype(2, dtype::Int32());
    WarpPerspectiveMatRNG rng;
    checker.set_rng(1, &rng);
    UniformIntRNG mid_rng_b2(0, 2);
    checker.set_rng(2, &mid_rng_b2);
    WarpPerspectiveForward::Param param;
    param.imode = InterpolationMode::LINEAR;
    for (auto fmt : {Format::NCHW})
        for (auto bmode :
             {WarpPerspective::BorderMode::WRAP, WarpPerspective::BorderMode::REFLECT,
              WarpPerspective::BorderMode::REPLICATE,
              WarpPerspective::BorderMode::CONSTANT}) {
            param.format = fmt;
            param.border_val = 5;
            param.bmode = bmode;
            checker.set_param(param);
            checker.set_dtype(0, dtype::Uint8());
            checker.set_dtype(3, dtype::Uint8());
            checker.execs({{3, 13, 13, 17}, {3, 3, 3}, {3}, {3, 13, 7, 17}});
            checker.execs({{3, 7, 22, 17}, {5, 3, 3}, {5}, {5, 7, 7, 33}});
        }
    for (auto fmt : {Format::NCHW, Format::NHWC})
        for (auto bmode :
             {WarpPerspective::BorderMode::WRAP, WarpPerspective::BorderMode::REFLECT,
              WarpPerspective::BorderMode::REPLICATE,
              WarpPerspective::BorderMode::CONSTANT}) {
            param.format = fmt;
            param.border_val = 1.25;
            param.bmode = bmode;
            checker.set_param(param);
            checker.set_dtype(0, dtype::Float32());
            checker.set_dtype(3, dtype::Float32());
            checker.execs({{3, 13, 13, 17}, {3, 3, 3}, {3}, {3, 13, 7, 17}});
        }
}

TEST(NAIVE, WarpPerspectiveResize) {
    Checker<WarpPerspectiveForward> checker(Arch::BAREMETAL);
    checker.set_kernel_symbol("kernel_.*");
    WarpPerspectiveForward::Param param;
    ResizeMatRNG rng;

    checker.set_rng(1, &rng);
    using BMode = param::WarpPerspective::BorderMode;
    param.imode = param::WarpPerspective::InterpolationMode::LINEAR;

#if ENABLE_KERNEL_FP16
    checker.set_epsilon(2e-3);
    for (auto dtype : {(DType)dtype::Float16(), (DType)dtype::Float32()})
#else
    checker.set_epsilon(1e-4);
    for (auto dtype : {(DType)dtype::Float32()})
#endif
    {
        checker.set_dtype(0, dtype);
        checker.set_dtype(2, dtype);

        param.format = Format::NCHW;
        for (auto mode :
             {BMode::REFLECT_101, BMode::REPLICATE, BMode::REFLECT, BMode::WRAP,
              BMode::CONSTANT}) {
            param.bmode = mode;
            param.border_val = 1.737;
            checker.set_param(param);
            checker.exec({{1000, 2, 10, 11}, {1000, 3, 3}, {1000, 2, 12, 13}});
        }

        param.format = Format::NHWC;
        for (auto mode :
             {BMode::REFLECT_101, BMode::REPLICATE, BMode::REFLECT, BMode::WRAP,
              BMode::CONSTANT}) {
            param.bmode = mode;
            param.border_val = 0.439;
            checker.set_param(param);
            checker.exec({{1000, 10, 11, 4}, {1000, 3, 3}, {1000, 12, 15, 4}});
        }
    }
}