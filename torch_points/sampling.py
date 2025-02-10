import torch

def sample_points_random(
        points: torch.Tensor, 
        M: int):
    """
    Randomly sample `M` 3D points.

    Args:
        points (torch.Tensor): 3D points of shape `(N,3)`.
        M (int): number of samples

    Returns:
        torch.Tensor: sampled 3D points of shape `(M,3)`.
    """
    N = len(points)
    assert 1 <= M and M <= N
    return points[torch.randperm(N, dtype=torch.int32, device=points.device)[:M]]
