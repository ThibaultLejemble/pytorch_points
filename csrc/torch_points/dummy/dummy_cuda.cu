#include <torch_points/dummy/dummy.h>
#include <torch_points/common/check.h>

namespace torch_points {

namespace kernel {
    
// 1x1 threads per bloc
__global__ void dummy_print_and_mult10(
    const float* __restrict__ x, 
    float* __restrict__ y,
    int nrows, 
    int ncols)
{
    const int i = blockIdx.x;
    const int j = blockIdx.y;
    if(i < nrows and j < ncols)
    {
        const int k = i * ncols + j; // row-major
        printf("CUDA x[%d,%d] = %f\n", i, j, x[k]);
        y[k] = 10 * x[k];
    }
}

} // namespace kernel


torch::Tensor dummy_cuda(torch::Tensor x)
{
    CHECK_CUDA(x);
    const int nrows = x.size(0);
    const int ncols = x.size(1);
    auto y = torch::Tensor(x);
    // 1x1 threads per bloc
    const dim3 blocs = {uint(nrows), uint(ncols), 1};
    const dim3 threads = {1, 1, 1};
    kernel::dummy_print_and_mult10<<<blocs,threads>>>(
        x.data_ptr<float>(), 
        y.data_ptr<float>(),
        nrows, 
        ncols);
    return y;
}

} // namespace torch_points
