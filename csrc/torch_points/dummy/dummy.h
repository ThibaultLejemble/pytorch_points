#pragma once

#include <torch/extension.h>

namespace torch_points {
    
torch::Tensor dummy(torch::Tensor x);
torch::Tensor dummy_cpu(torch::Tensor x);
torch::Tensor dummy_cuda(torch::Tensor x);

} // namespace torch_points
