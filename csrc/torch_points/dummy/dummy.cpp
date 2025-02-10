#include <torch_points/dummy/dummy.h>
#include <torch_points/common/dispatch.h>

namespace torch_points {

torch::Tensor dummy(torch::Tensor x)
{
    DISPATCH(x.device(), dummy, x);
}   

} // namespace torch_points
