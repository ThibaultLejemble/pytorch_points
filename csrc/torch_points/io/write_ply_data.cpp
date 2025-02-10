#include <torch_points/io/ply.h>
#include <torch_points/common/check.h>

namespace torch_points {

void write_ply_data(
    const std::string& path, 
    torch::Tensor points,
    torch::optional<torch::Tensor> normals,
    torch::optional<torch::Tensor> colors,
    torch::optional<std::map<std::string,torch::Tensor>> properties)
{
    CHECK_CPU(points);
    CHECK_CONTIGUOUS(points);
    TORCH_CHECK(points.dim() == 2, "points tensor size must be Nx3");
    TORCH_CHECK(points.size(1) == 3, "points tensor size must be Nx3");
    if(normals) {
        CHECK_CPU(*normals);
        CHECK_CONTIGUOUS(*normals);
        TORCH_CHECK(normals->dim() == 2, "normals tensor size must be Nx3");
        TORCH_CHECK(normals->size(1) == 3, "normals tensor size must be Nx3");
    }
    if(colors) {
        CHECK_CPU(*colors);
        CHECK_CONTIGUOUS(*colors);
        TORCH_CHECK(colors->dim() == 2, "colors tensor size must be NxC");
        TORCH_CHECK(colors->size(1) == 3 or colors->size(1) == 4, "colors tensor size must be NxC, with C=[3,4]");
    }
    if(properties) {
        TORCH_WARN("PLY properties not yet implemented");
    }
    std::ofstream fs(path);
    if(not fs.is_open()) {
        TORCH_WARN("Failed to open output PLY file '", path, "'");
        return;
    }
    plyio::PLYWriter writer;
    writer.set_binary();
    writer.add_comment("torch_points");
    writer.add_element("vertex", points.size(0));
    {
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
    }
    if(normals)
    {
        const void* data_ptr = normals->data_ptr();
        const plyio::Type ply_dtype = internal::get_ply_type(normals->dtype());
        if(ply_dtype == plyio::Type::type_unkown) {
            TORCH_WARN(normals->dtype(), " not supported");
            return;
        }
        const int size = plyio::internal::size_of(ply_dtype);
        const int stride = 3 * size;
        TORCH_INTERNAL_ASSERT(size > 0);
        TORCH_INTERNAL_ASSERT(stride > 0);
        const int offset_x = 0 * size;
        const int offset_y = 1 * size;
        const int offset_z = 2 * size;
        writer.add_property("vertex", "nx", data_ptr, ply_dtype, offset_x, stride);
        writer.add_property("vertex", "ny", data_ptr, ply_dtype, offset_y, stride);
        writer.add_property("vertex", "nz", data_ptr, ply_dtype, offset_z, stride);
    }
    if(colors)
    {
        const void* data_ptr = colors->data_ptr();
        const plyio::Type ply_dtype = internal::get_ply_type(colors->dtype());
        if(ply_dtype == plyio::Type::type_unkown) {
            TORCH_WARN(colors->dtype(), " not supported");
            return;
        }
        const int size = plyio::internal::size_of(ply_dtype);
        const int C = colors->size(1);
        TORCH_INTERNAL_ASSERT(C == 3 or C == 4);
        const int stride = C * size;
        TORCH_INTERNAL_ASSERT(size > 0);
        TORCH_INTERNAL_ASSERT(stride > 0);
        const int offset_x = 0 * size;
        const int offset_y = 1 * size;
        const int offset_z = 2 * size;
        const int offset_w = 3 * size;
        if(C == 3) {
            writer.add_property("vertex", "red",   data_ptr, ply_dtype, offset_x, stride);
            writer.add_property("vertex", "green", data_ptr, ply_dtype, offset_y, stride);
            writer.add_property("vertex", "blue",  data_ptr, ply_dtype, offset_z, stride);
        } else { // C == 4
            writer.add_property("vertex", "red",   data_ptr, ply_dtype, offset_x, stride);
            writer.add_property("vertex", "green", data_ptr, ply_dtype, offset_y, stride);
            writer.add_property("vertex", "blue",  data_ptr, ply_dtype, offset_z, stride);
            writer.add_property("vertex", "alpha", data_ptr, ply_dtype, offset_w, stride);
        }
    }
    writer.write(fs);
}

} // namespace torch_points