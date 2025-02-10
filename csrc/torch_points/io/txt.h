#pragma once

#include <torch/extension.h>
#include <optional>

namespace torch_points {

//
// read a 2D tensor of size (rows,cols)
// float dtype
// cols values per line
// space-separated
//
torch::optional<torch::Tensor> read_txt(
    const std::string& path, int rows, int cols);

// void write_txt(const std::string& path, torch::Tensor points);

} // namespace torch_points