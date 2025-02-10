#include <torch_points/io/txt.h>

#include <fstream>

namespace torch_points {

torch::optional<torch::Tensor> read_txt(
    const std::string& path, int rows, int cols)
{
    std::ifstream fs(path);
    if(not fs.is_open()) {
        return {};
    }

    auto x = torch::empty({rows,cols});
    auto acc = x.accessor<float,2>();
    for(int i = 0; i < rows; ++i)
    {
        for(int j = 0; j < cols; ++j)
        {
            fs >> acc[i][j];
        }
    }
    return x;
}

} // namespace torch_points