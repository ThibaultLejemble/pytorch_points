#include <torch_points/spatial/grid2D.h>
#include <torch_points/common/dispatch.h>
#include <torch_points/common/check.h>
#include <torch_points/common/parallel.h>

namespace torch_points {

std::pair<torch::Tensor,torch::Tensor> 
build_grid2d(
    torch::Tensor points,
    float xmin, 
    float xmax,
    float ymin, 
    float ymax,
    int Nx, 
    int Ny,
    bool sort_z)
{
    CHECK_POINTS(points);
    CHECK_CONTIGUOUS(points);
    TORCH_CHECK(0 < Nx);
    TORCH_CHECK(0 < Ny);
    TORCH_CHECK(xmin < xmax);
    TORCH_CHECK(ymin < ymax);
    DISPATCH(points.device(), build_grid2d, 
        points, xmin, xmax, ymin, ymax, Nx, Ny, sort_z);
}

std::pair<torch::Tensor,torch::Tensor> 
build_grid2d_cpu(
    torch::Tensor points,
    float xmin,
    float xmax,
    float ymin,
    float ymax,
    int Nx,
    int Ny,
    bool sort_z)
{
    CHECK_CPU(points);
    const int N = points.size(0);

    auto indices = torch::arange(N, torch::kInt32);
    auto cells = torch::empty({Nx,Ny,2}, torch::kInt32);

    const auto points_acc = points.accessor<float,2>();
    auto cells_acc = cells.accessor<int,3>();
    int* indices_ptr = indices.data_ptr<int>();

    const auto OrderX = [&points_acc](int i, int j) -> bool {
        return points_acc[i][0] < points_acc[j][0];
    };
    const auto OrderY = [&points_acc](int i, int j) -> bool {
        return points_acc[i][1] < points_acc[j][1];
    };
    const auto OrderZ = [&points_acc](int i, int j) -> bool {
        return points_acc[i][2] < points_acc[j][2];
    };

    const float dx = (xmax - xmin) / Nx;
    const float dy = (ymax - ymin) / Ny;

    // 0. remove points 
    const auto end = std::partition(indices_ptr, indices_ptr + N, [&](int i) {
        return xmin <= points_acc[i][0] and points_acc[i][0] < xmax and
               ymin <= points_acc[i][1] and points_acc[i][1] < ymax;
    });
    const int M = std::distance(indices_ptr, end);
    
    // 1. partition along y
    {
        // TODO call std::partition in parallel by iteratively splitting two partitions
        std::sort(indices_ptr, indices_ptr + M, OrderY);
        cells_acc[0][   0][0] = 0;
        cells_acc[0][Ny-1][1] = M;
        for(int iy = 0; iy < Ny-1; ++iy)
        {
            const float sup = ymin + (iy+1) * dy;
            const auto it = std::upper_bound(
                indices_ptr + cells_acc[0][iy][0],
                indices_ptr + M,
                sup,
                [&](float sup_value, int i){return sup_value <= points_acc[i][1];});
            const int idx_sup = std::distance(indices_ptr, it);
            cells_acc[0][iy  ][1] = idx_sup;
            cells_acc[0][iy+1][0] = idx_sup;
        }
    }

    // for(int iy = 0; iy < Ny; ++iy)
    // {
    //     const int begin = cells_acc[0][iy][0];
    //     const int end   = cells_acc[0][iy][1];
    //     const float a = ymin + (iy+0) * dy;
    //     const float b = ymin + (iy+1) * dy;
    //     for(int i = begin; i < end; ++i) {
    //         const int idx = indices_ptr[i];
    //         const float y = points_acc[idx][1];
    //         TORCH_CHECK(a <= y);
    //         TORCH_CHECK(y < b);
    //     }
    // }

    // 2. partition along x
    {
        parallel_for(Ny, [&](int iy) 
        {
            const int begin = cells_acc[0][iy][0];
            const int end   = cells_acc[0][iy][1];
            std::sort(
                indices_ptr + begin,
                indices_ptr + end, 
                OrderX);
            cells_acc[   0][iy][0] = begin;
            cells_acc[Nx-1][iy][1] = end;
            for(int ix = 0; ix < Nx-1; ++ix)
            {
                const float sup = xmin + (ix+1) * dx;
                const auto it = std::upper_bound(
                    indices_ptr + cells_acc[ix][iy][0],
                    indices_ptr + end,
                    sup,
                    [&](float sup_value, int i){return sup_value <= points_acc[i][0];});
                const int idx_sup = std::distance(indices_ptr, it);
                cells_acc[ix  ][iy][1] = idx_sup;
                cells_acc[ix+1][iy][0] = idx_sup;
            }
        }); // parallel_for
    }

    // 3. sort along z
    if(sort_z)
    {
        parallel_for(Ny, [&](int iy)
        {
            for(int ix = 0; ix < Nx; ++ix)
            {
                const int begin = cells_acc[ix][iy][0];
                const int end =   cells_acc[ix][iy][1];
                std::sort(indices_ptr + begin, indices_ptr + end, OrderZ);
            }
        }); // parallel_for
    }

    return std::make_pair(cells, indices);
}

} // namespace torch_points 