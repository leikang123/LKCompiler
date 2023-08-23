#ifndef TENSOR_UTIL_H
#define TENSOR_UTIL_H
#include <stdbool.h>
#include "data_struct.h"
#include "utils.h"

static inline bool force_layout_contiguous(Layout* layout) {
    int stride = 1;
    for (int i = layout->nr_dim - 1; i >= 0; --i) {
        layout->stride[i] = stride;
        stride *= layout->dims[i];
    }
    return stride != 0;
}

typedef struct {
    int inc_dims[MAX_DIM];
    int reset_stride[MAX_DIM];
    int offset;
} NoconIter;

static inline NoconIter init_iter(const Layout layout) {
    NoconIter iter;
    for (int i = layout.nr_dim - 1; i >= 0; --i) {
        iter.inc_dims[i] = 0;
        iter.reset_stride[i] = (layout.dims[i] - 1) * layout.stride[i];
    }
    iter.offset = 0;
    return iter;
}

static inline void inc_iter(const Layout layout, NoconIter* iter) {
    for (int i = layout.nr_dim - 1; i >= 0; --i) {
        iter->inc_dims[i] += 1;
        if (iter->inc_dims[i] >= layout.dims[i]) {
            iter->inc_dims[i] = 0;
            iter->offset = iter->offset - iter->reset_stride[i];
        } else {
            iter->offset = iter->offset + layout.stride[i];
            break;
        }
    }
}

static inline bool is_contiguous(Layout layout) {
    int stride = 1;
    for (int i = layout.nr_dim - 1; i >= 0; --i) {
        if (layout.stride[i] < 0 || layout.stride[i] != stride) {
            return 0;
        } else {
            stride *= layout.dims[i];
        }
    }
    return stride != 0;
}

static inline bool is_layout_scalar(const Layout* layout) {
    return layout->nr_dim == 1 && layout->dims[0] == 1;
}

static inline void broadcast_layout(Layout* layout_in, const Layout layout_dst) {
    TINYNN_ASSERT_MSG(
            layout_in->nr_dim && layout_dst.nr_dim, "broadcast involves empty layout");
    if (is_layout_scalar(layout_in)) {
        layout_in->nr_dim = layout_dst.nr_dim;
        for (size_t j = 0; j < layout_dst.nr_dim; ++j) {
            if (j) {
                layout_in->dims[j - 1] = layout_dst.dims[j];
            }
            layout_in->stride[j] = (layout_dst.dims[j] == 1);
        }
    }
    TINYNN_ASSERT_MSG(
            layout_dst.nr_dim >= layout_in->nr_dim,
            "dimension for broadcast must less than layout_dst");
    for (size_t j = 0; j < layout_dst.nr_dim; ++j) {
        int target_idx = layout_dst.nr_dim - j - 1;
        int cur_idx = layout_in->nr_dim - j - 1;
        size_t cur_shape = (cur_idx >= 0 ? layout_in->dims[cur_idx] : 1),
               cur_stride = (cur_idx >= 0 ? layout_in->stride[cur_idx] : 0);
        if (layout_dst.dims[target_idx] != cur_shape) {
            TINYNN_ASSERT_MSG(
                    cur_shape == 1 || cur_stride == 0,
                    "broadcast on dim with shape not equal to 1");
            layout_in->dims[target_idx] = layout_dst.dims[target_idx];
            layout_in->stride[target_idx] = 0;
        } else {
            layout_in->dims[target_idx] = cur_shape;
            layout_in->stride[target_idx] = cur_stride;
        }
    }
    layout_in->nr_dim = layout_dst.nr_dim;
}

static inline int32_t get_tensor_value(const Tensor* tensor, int index) {
    if (tensor->dtype.type_enum == TinyNN_FLOAT) {
        return *((float*)(tensor->ptr) + index);
    } else if (tensor->dtype.type_enum == TinyNN_INT) {
        return *((int32_t*)(tensor->ptr) + index);
    } else if (tensor->dtype.type_enum == TinyNN_INT8) {
        return *((int8_t*)(tensor->ptr) + index);
    } else {
        TINYNN_ASSERT_MSG(0, "unsupport dtype.\n");
    }
    return 0;
}
#endif
