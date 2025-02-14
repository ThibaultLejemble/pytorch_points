#pragma once

#include <torch/extension.h>

namespace torch_points {

//
// points are not modified
//
// returns
//      cells:   int (Nx,Ny,2): begin/end indices
//      indices: int (N):       indices in points
//
// example
//      Nx = 6
//      Ny = 4
//      
//         Y
//         ^
//         |
// ymax ---+----+----+----+----+----+----+
//       3 | 18 | 19 | 20 | 21 | 22 | 13 |
//       2 | 12 | 13 | 14 | 15 | 16 | 17 |
//       1 |  6 |  7 |  8 |  9 | 10 | 11 |
//       0 |  0 |  1 |  2 |  3 |  4 |  5 |
// ymin ---+----+----+----+----+----+----+---> X
//         |  0    1    2    3    4    5 |
//        xmin                          xmax                            
//
std::pair<torch::Tensor,torch::Tensor> 
build_grid2d(
    torch::Tensor points,
    float xmin,
    float xmax,
    float ymin,
    float ymax,
    int Nx,
    int Ny,
    bool sort_z = false);

std::pair<torch::Tensor,torch::Tensor> 
build_grid2d_cpu(
    torch::Tensor points,
    float xmin,
    float xmax,
    float ymin,
    float ymax,
    int Nx,
    int Ny,
    bool sort_z = false);

std::pair<torch::Tensor,torch::Tensor> 
build_grid2d_cuda(
    torch::Tensor points,
    float xmin,
    float xmax,
    float ymin,
    float ymax,
    int Nx,
    int Ny,
    bool sort_z = false);

} // namespace torch_points