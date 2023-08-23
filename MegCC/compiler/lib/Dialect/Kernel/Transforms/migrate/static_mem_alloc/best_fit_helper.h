#pragma once

#include <functional>
#include "./impl.h"

namespace mlir {
namespace Kernel {
namespace migrate {

struct BestFitHelper {
    using Interval = StaticMemAllocImplHelper::Interval;
    std::function<void(Interval*)> alloc;
    std::function<void(Interval* dest, size_t offset, Interval*)> alloc_overwrite;
    std::function<void(Interval*)> free;

    /*!
     * \brief run on intervals and call corresponding methods
     */
    void run(const StaticMemAllocImplHelper::IntervalPtrArray& intervals);
};

}  // namespace migrate
}  // namespace Kernel
}  // namespace mlir

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}
