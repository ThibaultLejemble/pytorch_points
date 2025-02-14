from typing import Tuple
import torch
import torch_points.torch_points_csrc as csrc

def build_grid2d(
        points: torch.Tensor,
        xmin: float,
        xmax: float,
        ymin: float,
        ymax: float,
        Nx: int,
        Ny: int,
        sort_z: bool=False) -> Tuple[torch.Tensor,torch.Tensor]:
    return csrc.build_grid2d(points, xmin, xmax, ymin, ymax, Nx, Ny, sort_z)
