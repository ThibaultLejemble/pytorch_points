
import torch
from torch_points import read_ply, write_ply, read_ply_data, write_ply_data
from pathlib import Path

def test_ply():
    x = torch.zeros([64,3], dtype=torch.float32)
    n = 0
    for i in range(64):
        x[i,0] = n
        x[i,1] = n+1
        x[i,2] = n+2
        n += 3
    write_ply('tensor.ply', x)
    y = read_ply('tensor.ply')
    assert y.device == torch.device('cpu')
    assert y.shape == (64,3)
    assert y.dtype == torch.float32
    for i in range(64):
        for j in range(3):
            assert int(x[i,j]) == int(y[i,j])
    # remove file
    f = Path('tensor.ply')
    assert f.exists()
    f.unlink()


def test_ply_data():
    x_points = torch.zeros([64,3], dtype=torch.float32)
    x_normals = None
    x_colors = torch.zeros([64,4], dtype=torch.uint8)
    x_prop = {
        'value': torch.zeros([64,1], dtype=torch.float64),
        'coord': torch.zeros([64,2], dtype=torch.int32),
        'feature': torch.zeros([64,1], dtype=torch.float32),
    }
    n = 0
    for i in range(64):
        x_points[i,0] = n
        x_points[i,1] = n+1
        x_points[i,2] = n+2
        x_colors[i,0] = 4*i
        x_colors[i,1] = 4*i
        x_colors[i,2] = 4*i
        x_colors[i,3] = 4*i
        x_prop['value'][i,0] = i/64
        x_prop['coord'][i,0] = n
        x_prop['coord'][i,1] = 63-n
        x_prop['feature'][i,0] = i/64
        n += 3
    write_ply_data(
        'tensor.ply',
        points=x_points,
        normals=x_normals,
        colors=x_colors,
        properties=x_prop)
    y_points, y_normals, y_colors, y_prop = read_ply_data('tensor.ply')
    assert y_points.device == torch.device('cpu')
    assert y_points.shape == (64,3)
    assert y_points.dtype == torch.float32
    for i in range(64):
        for j in range(3):
            assert int(x_points[i,j]) == int(y_points[i,j])
    assert y_normals is None
    assert y_colors.device == torch.device('cpu')
    assert y_colors.dtype == torch.uint8
    assert y_colors.shape == (64,4)
    # TODO: properties not yet handled
    # remove file
    f = Path('tensor.ply')
    assert f.exists()
    f.unlink()