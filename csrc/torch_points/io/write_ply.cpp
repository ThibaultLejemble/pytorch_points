#include <torch_points/io/ply.h>
#include <torch_points/common/check.h>

namespace torch_points {

void write_ply(const std::string& path, torch::Tensor points)
{
    CHECK_CPU(points);
    CHECK_CONTIGUOUS(points);
    TORCH_CHECK(points.dim() == 2, "points tensor size must be Nx3");
    TORCH_CHECK(points.size(1) == 3, "points tensor size must be Nx3");
    std::ofstream fs(path);
    if(not fs.is_open()) {
        TORCH_WARN("Failed to open output PLY file '", path, "'");
        return;
    }
    plyio::PLYWriter writer;
    writer.set_binary();
    writer.add_comment("torch_points");
    writer.add_element("vertex", points.size(0));
    const void* data_ptr = points.data_ptr();
    const plyio::Type ply_dtype = internal::get_ply_type(points.dtype());
    if(ply_dtype == plyio::Type::type_unkown) {
        TORCH_WARN(points.dtype(), " not supported");
        return;
    }
    const int size = plyio::internal::size_of(ply_dtype);
    const int stride = 3 * size;
    TORCH_INTERNAL_ASSERT(size > 0);
    TORCH_INTERNAL_ASSERT(stride > 0);
    const int offset_x = 0 * size;
    const int offset_y = 1 * size;
    const int offset_z = 2 * size;
    writer.add_property("vertex", "x", data_ptr, ply_dtype, offset_x, stride);
    writer.add_property("vertex", "y", data_ptr, ply_dtype, offset_y, stride);
    writer.add_property("vertex", "z", data_ptr, ply_dtype, offset_z, stride);
    writer.write(fs);
}

} // namespace torch_points