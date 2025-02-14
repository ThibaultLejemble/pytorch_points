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
    '''
    Build a 2D grid from a set of points.

    The input points are unchanged.

    To access the points in a cell `(i,j)`:

    .. code-block:: python

        cells, indices = build_grid2d(points, xmin, xmax, ymin, ymax, Nx, Ny)
        begin = cells[i,j,0].item()
        end = cells[i,j,1].item()
        points_in_cell = points[indices[begin:end]]

    Args:
        points (torch.Tensor): 3D points of shape `(N,3)`.
        xmin (float): The minimum x value of the grid.
        xmax (float): The maximum x value of the grid.
        ymin (float): The minimum y value of the grid.
        ymax (float): The maximum y value of the grid.
        Nx (int): The number of cells in the x direction.
        Ny (int): The number of cells in the y direction.
        sort_z (bool): If True, the points are sorted by their z coordinate in each cell.

    Returns:
        tuple:
            A tuple containing:

            - `cells` of shape `(Nx,Ny,2)` containing the begin/end of points indices in each cell.
            - `indices` of shape `(N,)` refering to the original points.
    '''
    return csrc.build_grid2d(points, xmin, xmax, ymin, ymax, Nx, Ny, sort_z)
