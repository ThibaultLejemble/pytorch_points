import torch
from torch_points import dummy

def test_dummy():
    x = torch.Tensor([
        [1.0, 2.0],
        [3.0, 4.0],
        [5.0, 6.0],
    ])
    y_ref = torch.Tensor([
        [10.0, 20.0],
        [30.0, 40.0],
        [50.0, 60.0],
    ])
    y = dummy(x)
    assert torch.equal(y, y_ref)

    


