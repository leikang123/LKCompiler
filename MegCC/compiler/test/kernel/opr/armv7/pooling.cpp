#include "test/kernel/common/checker.h"
using namespace megdnn;
using namespace megcc::test;
using namespace megcc::KernelGen;
using Mode = PoolingForward::Param::Mode;

TEST(ARMV7, PoolingNCHW44) {
    Checker<Pooling> checker(Arch::ARMV7);
    PoolingForward::Param param;
    param.format = PoolingForward::Param::Format::NCHW44;
    checker.set_param(param);
    checker.set_epsilon(1e-4);

    for (auto mode : {Mode::MAX, Mode::AVERAGE, Mode::AVERAGE_COUNT_EXCLUDE_PADDING})
        for (size_t window : {2, 3, 5})
            for (size_t stride : {(size_t)1, window})
                for (size_t pad : {(size_t)0, window / 2})
                    for (size_t n : {1, 4})
                        for (size_t c : {4, 12})
                            for (size_t hw : {5, 23}) {
                                param.mode = mode;
                                param.pad_h = pad;
                                param.pad_w = pad;
                                param.window_h = window;
                                param.window_w = window;
                                param.stride_h = stride;
                                param.stride_w = stride;
                                checker.set_param(param);
                                checker.execs({{n, c / 4, hw, hw, 4}, {}});
                            }
}

TEST(ARMV7, PoolingNCHW44QInt8) {
    Checker<Pooling> checker(Arch::ARMV7);
    PoolingForward::Param param;
    param.format = PoolingForward::Param::Format::NCHW44;
    UniformIntRNG rng(-127, 127);
    checker.set_rng(0, &rng);
    checker.set_param(param);
    for (auto scale : {0.35f, 0.7f, 1.6f})
        for (auto mode :
             {Mode::MAX, Mode::AVERAGE, Mode::AVERAGE_COUNT_EXCLUDE_PADDING})
            for (size_t window : {2, 3, 5})
                for (size_t stride : {(size_t)1, window})
                    for (size_t pad : {(size_t)0, window / 2})
                        for (size_t n : {1, 3})
                            for (size_t c : {4, 12})
                                for (size_t hw : {5, 23}) {
                                    checker.set_dtype(0, dtype::QuantizedS8(scale));
                                    checker.set_dtype(1, dtype::QuantizedS8(scale));
                                    param.mode = mode;
                                    param.pad_h = pad;
                                    param.pad_w = pad;
                                    param.window_h = window;
                                    param.window_w = window;
                                    param.stride_h = stride;
                                    param.stride_w = stride;
                                    checker.set_param(param);
                                    checker.execs({{n, c / 4, hw, hw, 4}, {}});
                                }
}
