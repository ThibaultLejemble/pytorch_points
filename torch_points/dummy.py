import torch
import torch_points.torch_points_csrc as csrc

def dummy(x: torch.Tensor) -> torch.Tensor:
    '''
    Print into the terminal the content of x, and multiply it by 10.
    Input tensor x must be a 2-dimensional tensor of 32-bits float.
    '''
    return csrc.dummy(x)
