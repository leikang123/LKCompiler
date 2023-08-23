#pragma once

#include "../helper.h"
#include "../static_mem_alloc.h"
#include "compiler/Common/Logger.h"

#include <unordered_map>
#include <vector>

namespace mlir {
namespace Kernel {
namespace migrate {
class StaticMemAllocImplHelper : public StaticMemAlloc {
public:
    class Interval;
    using IntervalPtrArray = std::vector<Interval*>;

    ~StaticMemAllocImplHelper() noexcept;

    size_t add(size_t begin, size_t end, size_t size, UserKeyType key) override final;

    StaticMemAlloc& add_overwrite_spec(
            size_t iid_src, size_t iid_dest, size_t offset) override final;

    size_t get_start_addr(UserKeyType key) const override final;

    StaticMemAlloc& solve() override final;

    StaticMemAlloc& alignment(size_t alignment) override final {
        CC_ASSERT(!(alignment & (alignment - 1)));
        m_alignment = alignment;
        return *this;
    }

    StaticMemAlloc& padding(size_t padding) override final {
        m_padding = padding;
        return *this;
    }

    size_t tot_alloc_lower_bound() const override final { return m_peak_lower_bound; }

protected:
    static constexpr size_t INVALID = -1;

    //! sorted intervals
    IntervalPtrArray m_interval;

    /*!
     * \brief implement solve(); subclasses should work on m_interval, and
     *      write results to Interval::addr_begin
     */
    virtual void do_solve() = 0;

    /*!
     * \brief get aligned address
     */
    size_t align(size_t addr) { return get_aligned_power2(addr, m_alignment); }

private:
    size_t m_alignment = 1, m_padding = 0, m_peak_lower_bound = 0;

    //! original interval storage
    std::vector<Interval> m_interval_storage;

    //! tuple of (src, dest, offset)
    std::vector<std::tuple<size_t, size_t, size_t>> m_overwrite_spec;

    std::unordered_map<UserKeyType, Interval*> m_userkey2itrv;

    /*!
     * \brief copy m_overwrite_spec to Interval::overwrite_dest
     */
    void init_overwrite_dest();

    void check_result_and_calc_lower_bound();
};

class StaticMemAllocImplHelper::Interval {
    Interval *m_overwrite_dest = nullptr, *m_overwrite_src = nullptr,
             *m_overwrite_dest_root = nullptr;
    size_t m_offset_in_overwrite_dest = 0, m_offset_in_overwrite_dest_root = 0;

    Interval* overwrite_dest_root_path_compression();

    Interval(size_t b, size_t e, size_t size, UserKeyType k, size_t id)
            : key(k),
              time_begin_orig(b),
              time_end_orig(e),
              size_orig(size),
              id(id),
              time_begin(b),
              time_end(e),
              size(size) {}

    friend class StaticMemAllocImplHelper;

public:
    Interval() = default;

    UserKeyType const key = nullptr;
    size_t const time_begin_orig = INVALID, time_end_orig = INVALID,
                 size_orig = INVALID, id = INVALID;

    //! time_begin, time_end and size could be modified to ease
    //! implementation
    //! addr_begin stores final result
    size_t time_begin = INVALID, time_end = INVALID, size = INVALID,
           addr_begin = INVALID;

    /*!
     * \brief the interval that is overwritten by this one
     *
     * Note that overwrite dest must be respected by allocators; and it is
     * guaranteed that there would be no conflict for overwritting.
     */
    Interval* overwrite_dest() const { return m_overwrite_dest; }

    /*!
     * \brief root overwrite dest with chain coalesced
     *
     * Initialized by init_overwrite_dest() before do_solve(); nullptr if no
     * overwrite spec.
     */
    Interval* overwrite_dest_root() const { return m_overwrite_dest_root; }

    /*!
     * \brief get offset of this interval in overwrite dest
     */
    size_t offset_in_overwrite_dest() const { return m_offset_in_overwrite_dest; }

    /*!
     * \brief get offset of this interval in overwrite dest root
     */
    size_t offset_in_overwrite_dest_root() const {
        return m_offset_in_overwrite_dest_root;
    }

    /*!
     * \brief the interval that overwrites this one
     */
    Interval* overwrite_src() const { return m_overwrite_src; }

    /*!
     * \brief whether this interval does not override any other interval
     */
    bool is_overwrite_root() const { return !m_overwrite_dest; }

    size_t time_length() const { return time_end - time_begin; }

    bool time_overlap(const Interval& rhs) const {
        return time_begin < rhs.time_end && rhs.time_begin < time_end;
    }

    bool addr_overlap(const Interval& rhs) const {
        return addr_begin < rhs.addr_end() && rhs.addr_begin < addr_end();
    }

    size_t addr_end() const { return addr_begin + size; }
};

}  // namespace migrate
}  // namespace Kernel
}  // namespace mlir

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}
