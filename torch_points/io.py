from typing import Optional, Dict
import torch
import torch_points.torch_points_csrc as csrc

def read_ply(path: str) -> torch.Tensor:
    """
    Read 3D points from a PLY file.

    Args:
        path (str): The path to the PLY file.

    Returns:
        torch.Tensor: 3D points of shape `(N,3)`.
    """
    return csrc.read_ply(path)

def write_ply(path: str, points: torch.Tensor) -> None:
    """
    Write 3D points to a PLY file.

    Args:
        path (str): The path to the PLY file.
        points (torch.Tensor): 3D points of shape `(N,3)`.
    """
    csrc.write_ply(path, points)

def read_ply_data(path: str) -> tuple[
    torch.Tensor,                     # points
    Optional[torch.Tensor],           # normals
    Optional[torch.Tensor],           # colors
    Optional[Dict[str,torch.Tensor]], # properties
]:
    """
    Read all the data from a PLY file.

    Args:
        path (str): The path to the PLY file.

    Returns:
        a tuple of `points`, `normals`, `colors` and `properties`

        0. `points`: 3D points of shape `(N,3)`
        1. `normals`: optional normals of shape `(N,3)`
        2. `colors`: optional colors of shape `(N,C)`
        3. `properties`: optional dictionnary of named tensors
    """
    return csrc.read_ply_data(path)

def write_ply_data(
        path: str,
        points: torch.Tensor,
        normals: torch.Tensor=None,
        colors: torch.Tensor=None,
        properties: Dict[str,torch.Tensor]=None) -> None:
    """
    Write all the data to a PLY file.

    Args:
        path (str): The path to the PLY file.
        points (torch.Tensor): 3D points of shape `(N,3)`.
        normals (torch.Tensor): optional normals of shape `(N,3)`.
        colors (torch.Tensor): optional colors of shape `(N,C)`.
        properties (torch.Tensor): optional dictionnary of named tensors.
    """
    csrc.write_ply_data(path, points, normals, colors, properties)




def read_txt(path: str, rows: int, cols: int) -> torch.Tensor:
    return csrc.read_txt(path, rows, cols)

def read_xyz(path: str, cols: int=3, rows: int=100000):
    data = torch.empty((rows,cols), dtype=torch.float32)
    f = open(path, 'r')
    i = 0
    for line in f.readlines():
        values_str = line.split()
        for j in range(cols):
            data[i,j] = float(values_str[j])
        i += 1
    assert i == rows
    return data
