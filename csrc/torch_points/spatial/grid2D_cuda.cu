#include <torch_points/spatial/grid2D.h>
#include <torch_points/common/dispatch.h>
#include <torch_points/common/check.h>

#include <thrust/transform.h>

namespace torch_points {

namespace kernel {

} // namespace kernel

std::pair<torch::Tensor,torch::Tensor> 
build_grid2d_cuda(
    torch::Tensor points,
    float xmin, 
    float xmax,
    float ymin, 
    float ymax,
    int nx, 
    int ny)
{
}

} // namespace torch_points 