#include <torch_points/io/ply.h>
#include <torch_points/common/check.h>

namespace torch_points {

torch::optional<torch::Tensor> read_ply(const std::string& path)
{
    plyio::PLYReader reader;
    std::ifstream fs(path);
    if(not fs.is_open()) {
        TORCH_WARN("Failed to open input PLY file '", path, "'");
        return {};
    }
    reader.read_header(fs);
    if(reader.has_error()) {
        for(const std::string& err : reader.errors())
            TORCH_WARN(err);
        return {};
    }
    if(reader.has_warning()) {
        for(const std::string& w : reader.warnings())
            TORCH_WARN(w);
    }
    if(not reader.has_element("vertex")) {
        TORCH_WARN("PLY element 'vertex' not found");
        return {};
    }
    if(not reader.has_property("vertex", "x")) {
        TORCH_WARN("PLY property 'x' not found");
        return {};
    }
    if(not reader.has_property("vertex", "y")) {
        TORCH_WARN("PLY property 'y' not found");
        return {};
    }
    if(not reader.has_property("vertex", "z")) {
        TORCH_WARN("PLY property 'z' not found");
        return {};
    }
    const auto ply_dtype_x = reader.property("vertex", "x").dtype();
    const auto ply_dtype_y = reader.property("vertex", "y").dtype();
    const auto ply_dtype_z = reader.property("vertex", "z").dtype();
    if(ply_dtype_x != ply_dtype_y or ply_dtype_x != ply_dtype_z or ply_dtype_y != ply_dtype_z) {
        TORCH_WARN("PLY properties 'x', 'y' and 'z' dtype mismatched: ", ply_dtype_x, " ", ply_dtype_y, " ", ply_dtype_z);
        return {};
    }
    const int vertex_count = reader.element_count("vertex");
    const auto torch_dtype0 = internal::get_torch_dtype(ply_dtype_x);
    if(not torch_dtype0.has_value()) {
        TORCH_WARN("dtype ", plyio::internal::to_string(ply_dtype_x), " not supported");
        return {};
    }
    const auto torch_dtype = torch_dtype0.value();
    const int size = plyio::internal::size_of(ply_dtype_x);
    const int stride = 3 * size;
    TORCH_INTERNAL_ASSERT(size > 0);
    TORCH_INTERNAL_ASSERT(stride > 0);
    const int offset_x = 0 * size;
    const int offset_y = 1 * size;
    const int offset_z = 2 * size;
    const auto options = torch::TensorOptions().dtype(torch_dtype);
    torch::Tensor points = torch::zeros({vertex_count,3}, options);
    void* data_ptr = points.data_ptr();
    reader.property("vertex", "x").read(data_ptr, offset_x, stride);
    reader.property("vertex", "y").read(data_ptr, offset_y, stride);
    reader.property("vertex", "z").read(data_ptr, offset_z, stride);
    reader.read_body(fs);
    if(reader.has_error()) {
        for(const std::string& err : reader.errors())
            TORCH_WARN(err);
        return {};
    }
    if(reader.has_warning()) {
        for(const std::string& w : reader.warnings())
            TORCH_WARN(w);
    }
    return points;
}

} // namespace torch_points