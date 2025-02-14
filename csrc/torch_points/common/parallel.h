#pragma once

#include <torch/extension.h>

namespace torch_points {

template<typename FuncT>
void parallel_for(int end, const FuncT& f, int grain_size = 0)
{
    at::parallel_for(0, end, grain_size, [&](int64_t for_begin, int64_t for_end)
    {
        for(int i = for_begin; i < for_end; ++i) {
            f(i);
        }
    });
}

} // namespace torch_points