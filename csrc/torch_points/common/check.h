#pragma once

#define CHECK_CPU(x) TORCH_CHECK((x).device().is_cpu(), #x " must be a CPU tensor")
#define CHECK_CUDA(x) TORCH_CHECK((x).device().is_cuda(), #x " must be a CUDA tensor")
#define CHECK_CONTIGUOUS(x) TORCH_CHECK((x).is_contiguous(), #x " must be contiguous")
#define CHECK_POINTS(x) TORCH_CHECK((x).dim() == 2 and (x).size(1) == 3, #x " must have size [N,3]")