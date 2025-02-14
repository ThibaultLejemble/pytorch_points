
import torch
from torch_points import build_grid2d


def test_grid2d():
    N = 256
    Nx = 10
    Ny = 10
    xmin = -100
    xmax = 100
    ymin = -100
    ymax = 100
    points = torch.rand([N,3])
    points[:,0:2] = points[:,0:2]*200 -100 # scale x and y coord to (-100,100)
    cells, indices = build_grid2d(points=points, xmin=xmin, xmax=xmax, ymin=ymin, ymax=ymax, Nx=Nx, Ny=Ny)
    dx = (xmax - xmin) / Nx
    dy = (ymax - ymin) / Ny 
    assert cells.shape == (Nx,Ny,2)
    assert indices.shape == (N,)
    for i in range(10):
        for j in range(10):
            cell_xmin = xmin + i*dx
            cell_xmax = xmin + i*dx + dx
            cell_ymin = ymin + j*dy
            cell_ymax = ymin + j*dy + dy
            cell_begin,cell_end = cells[i,j]
            cell_begin,cell_end = cell_begin.item(),cell_end.item()
            assert cell_begin <= cell_end
            assert 0 <= cell_begin and cell_begin < N # excluding N
            assert 0 <= cell_end and cell_end <= N # including N
            for k in range(cell_begin,cell_end):
                idx = indices[k]
                assert 0 <= idx and idx < N
                x,y,z = points[idx]
                x,y,z = x.item(),y.item(),z.item()
                assert cell_xmin <= x and x < cell_xmax
                assert cell_ymin <= y and y < cell_ymax
