#pragma once

namespace torch_points {

#ifdef WITH_GPU
#define DISPATCH(device, func, ...)           \
    if(device.is_cuda()) {                    \
        return func ## _cuda(__VA_ARGS__);    \
    } else if(device.is_cpu()) {              \
        return func ## _cpu(__VA_ARGS__);     \
    } else {                                  \
        TORCH_INTERNAL_ASSERT(false);         \
    }
#else // no gpu
#define DISPATCH(device, func, ...)           \
    if(device.is_cuda()) {                    \
        TORCH_INTERNAL_ASSERT(false);         \
    } else if(device.is_cpu()) {              \
        return func ## _cpu(__VA_ARGS__);     \
    } else {                                  \
        TORCH_INTERNAL_ASSERT(false);         \
    }
#endif

} // namespace torch_points
