#include <torch_points/dummy/dummy.h>
#include <torch_points/common/check.h>

namespace torch_points {

torch::Tensor dummy_cpu(torch::Tensor x)
{
    CHECK_CPU(x);
    TORCH_CHECK(x.dim() == 2);
    TORCH_CHECK(x.dim() == 2);
    auto x_acc = x.accessor<float,2>();
    // print
    for(int i = 0; i < x.size(0); ++i)
        for(int j = 0; j < x.size(1); ++j)
            std::cout << "CPU x[" << i << ',' << j << "] = " << x_acc[i][j] << std::endl;
    // x10
    auto y = torch::Tensor(x);
    y *= 10;
    return y;
}

} // namespace torch_points
