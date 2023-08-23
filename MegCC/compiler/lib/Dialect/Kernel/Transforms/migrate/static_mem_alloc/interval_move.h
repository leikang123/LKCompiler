#pragma once
#include "./impl.h"

namespace mlir {
namespace Kernel {
namespace migrate {

class StaticMemAllocIntervalMove final : public StaticMemAllocImplHelper {
    struct MergedInterval {
        size_t begin, end;
    };

    size_t m_peak = 0, m_move_space_size_version = 0;

    struct IntervalExtraInfo {
        //! conflicting intervals if trying to move this interval to higher
        //! address
        IntervalPtrArray move_conflict;

        //! max dist to move without increasing peak
        struct MoveSpaceSizeRecord {
            size_t version = 0, size;
        };
        MoveSpaceSizeRecord move_space_size;
    };

    //! extra info for each interval, indexed by interval id
    std::vector<IntervalExtraInfo> m_interval_extra;

    void sort_intervals();

    /*!
     * \brief get max move distance without increasing peak usage
     * \param from the interval that initiates this query, to avoid infinite
     *      recursion
     */
    size_t get_move_space_size(Interval* interval);

    /*!
     * \brief move interval higher so addr_begin >= prev_end
     * \param from the interval that initiates this action, to avoid infinite
     *      recursion
     */
    void move_interval_higher(Interval* interval, size_t prev_end);

    void insert_interval(Interval& dest, const IntervalPtrArray& conflict);

    std::vector<MergedInterval> merge_interval_by_addr(
            const IntervalPtrArray& intervals);

    /*!
     * \brief find best fit
     *
     * minimize peak_add
     * 1. if dest.size < free_space_size, then minimize remaining space
     * 2. otherwise, minimize move distance
     *
     * \return start address, peak_incr
     */
    std::pair<size_t, size_t> find_best_fit(
            const IntervalPtrArray& conflict, size_t dest_size);

    void do_solve() override;

public:
    size_t tot_alloc() const override {
        CC_ASSERT(m_peak);
        return m_peak;
    }
};

}  // namespace migrate
}  // namespace Kernel
}  // namespace mlir

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}
