#include <torch_points/io/ply.h>
#include <torch_points/common/check.h>

namespace torch_points {

std::tuple<
    torch::Tensor, // points
    torch::optional<torch::Tensor>, // normals
    torch::optional<torch::Tensor>, // colors
    torch::optional<std::map<std::string,torch::Tensor>>> // properties
read_ply_data(const std::string& path)
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
    const int vertex_count = reader.element_count("vertex");
    const bool has_normals = 
        reader.has_property("vertex", "nx") and
        reader.has_property("vertex", "ny") and
        reader.has_property("vertex", "nz");
    const bool has_colors = 
        reader.has_property("vertex", "red") and
        reader.has_property("vertex", "green") and
        reader.has_property("vertex", "blue");
    const bool has_alpha = has_colors and reader.has_property("vertex", "alpha");
    torch::Tensor points;
    torch::optional<torch::Tensor> normals = {};
    torch::optional<torch::Tensor> colors = {};
    torch::optional<std::map<std::string,torch::Tensor>> properties = {};
    {
        const auto ply_dtype_x = reader.property("vertex", "x").dtype();
        const auto ply_dtype_y = reader.property("vertex", "y").dtype();
        const auto ply_dtype_z = reader.property("vertex", "z").dtype();
        if(ply_dtype_x != ply_dtype_y or ply_dtype_x != ply_dtype_z or ply_dtype_y != ply_dtype_z) {
            TORCH_WARN("PLY properties 'x', 'y' and 'z' dtype mismatched: ", ply_dtype_x, " ", ply_dtype_y, " ", ply_dtype_z);
            return {};
        }
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
        points = torch::zeros({vertex_count,3}, options);
        void* data_ptr = points.data_ptr();
        reader.property("vertex", "x").read(data_ptr, offset_x, stride);
        reader.property("vertex", "y").read(data_ptr, offset_y, stride);
        reader.property("vertex", "z").read(data_ptr, offset_z, stride);
    }
    if(has_normals)
    {
        const auto ply_dtype_x = reader.property("vertex", "nx").dtype();
        const auto ply_dtype_y = reader.property("vertex", "ny").dtype();
        const auto ply_dtype_z = reader.property("vertex", "nz").dtype();
        if(ply_dtype_x != ply_dtype_y or ply_dtype_x != ply_dtype_z or ply_dtype_y != ply_dtype_z) {
            TORCH_WARN("PLY properties 'nx', 'ny' and 'nz' dtype mismatched: ", ply_dtype_x, " ", ply_dtype_y, " ", ply_dtype_z);
            return {};
        }
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
        normals = torch::zeros({vertex_count,3}, options);
        void* data_ptr = normals->data_ptr();
        reader.property("vertex", "x").read(data_ptr, offset_x, stride);
        reader.property("vertex", "y").read(data_ptr, offset_y, stride);
        reader.property("vertex", "z").read(data_ptr, offset_z, stride);
    }
    if(has_colors)
    {
        const auto ply_dtype_x = reader.property("vertex", "red").dtype();
        const auto ply_dtype_y = reader.property("vertex", "green").dtype();
        const auto ply_dtype_z = reader.property("vertex", "blue").dtype();
        if(has_alpha) {
            const auto ply_dtype_w = reader.property("vertex", "alpha").dtype();
            if(ply_dtype_x != ply_dtype_y or ply_dtype_x != ply_dtype_z or ply_dtype_y != ply_dtype_z or ply_dtype_x != ply_dtype_w) {
                TORCH_WARN("PLY properties 'red', 'green', 'blue' and 'alpha' dtype mismatched: ", ply_dtype_x, " ", ply_dtype_y, " ", ply_dtype_z, " ", ply_dtype_w);
                return {};
            }
        } else {
            if(ply_dtype_x != ply_dtype_y or ply_dtype_x != ply_dtype_z or ply_dtype_y != ply_dtype_z) {
                TORCH_WARN("PLY properties 'red', 'green' and 'blue' dtype mismatched: ", ply_dtype_x, " ", ply_dtype_y, " ", ply_dtype_z);
                return {};
            }
        }
        const auto torch_dtype0 = internal::get_torch_dtype(ply_dtype_x);
        if(not torch_dtype0.has_value()) {
            TORCH_WARN("dtype ", plyio::internal::to_string(ply_dtype_x), " not supported");
            return {};
        }
        const auto torch_dtype = torch_dtype0.value();
        const int size = plyio::internal::size_of(ply_dtype_x);
        const int C = has_alpha ? 4 : 3;
        const int stride = C * size;
        TORCH_INTERNAL_ASSERT(size > 0);
        TORCH_INTERNAL_ASSERT(stride > 0);
        const int offset_x = 0 * size;
        const int offset_y = 1 * size;
        const int offset_z = 2 * size;
        const int offset_w = 3 * size;
        const auto options = torch::TensorOptions().dtype(torch_dtype);
        colors = torch::zeros({vertex_count,C}, options);
        void* data_ptr = colors->data_ptr();
        reader.property("vertex", "red").read(data_ptr, offset_x, stride);
        reader.property("vertex", "green").read(data_ptr, offset_y, stride);
        reader.property("vertex", "blue").read(data_ptr, offset_z, stride);
        if(has_alpha)
            reader.property("vertex", "alpha").read(data_ptr, offset_w, stride);
    }
    {
        // properties
        const std::vector<std::string> predefined = {
            "x", "y", "z",
            "nx", "ny", "nz",
            "red", "green", "blue", "alpha"
        };
        for(auto& v_prop : reader.properties("vertex"))
        {
            if(std::find(predefined.begin(), predefined.end(), v_prop.name()) == predefined.end())
            {
                const auto ply_dtype = v_prop.dtype();
                const auto torch_dtype = internal::get_torch_dtype(ply_dtype);
                const int size = plyio::internal::size_of(ply_dtype);
                const auto options = torch::TensorOptions().dtype(torch_dtype.value());
                auto prop_tensor = torch::zeros({vertex_count}, options);
                void* data_ptr = prop_tensor.data_ptr();
                const int offset = 0;
                const int stride = size;
                v_prop.read(data_ptr, offset, stride);
                if(not properties.has_value())
                    properties = std::map<std::string,torch::Tensor>();
                properties->emplace(v_prop.name(), prop_tensor);
            }
        }
    }
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
    return {points, normals, colors, properties};
}

} // namespace torch_points