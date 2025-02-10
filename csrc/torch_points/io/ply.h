#pragma once

#include <torch/extension.h>
#include <optional>

#define PLYIO_ASSERT(x) TORCH_INTERNAL_ASSERT(x, "plyio error")
#include "internal/plyio.h"

namespace torch_points {

torch::optional<torch::Tensor> read_ply(const std::string& path);

void write_ply(const std::string& path, torch::Tensor points);

std::tuple<
    torch::Tensor, // points
    torch::optional<torch::Tensor>, // normals
    torch::optional<torch::Tensor>, // colors
    torch::optional<std::map<std::string,torch::Tensor>>> // properties
read_ply_data(const std::string& path);

void write_ply_data(
    const std::string& path, 
    torch::Tensor points,
    torch::optional<torch::Tensor> normals,
    torch::optional<torch::Tensor> colors,
    torch::optional<std::map<std::string,torch::Tensor>> properties);

namespace internal {
std::optional<torch::ScalarType> get_torch_dtype(plyio::Type ply_dtype);
plyio::Type get_ply_type(caffe2::TypeMeta torch_dtype);
} // namespace internal

} // namespace torch_points