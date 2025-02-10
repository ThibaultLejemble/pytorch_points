import torch
from setuptools import find_packages, setup
from torch.__config__ import parallel_info
from torch.utils.cpp_extension import BuildExtension, CppExtension, CUDAExtension
from pathlib import Path
import os 

WITH_CUDA = torch.cuda.is_available()

def get_extensions():
    cpp_files = [str(f) for f in Path('csrc').rglob('*.cpp')]
    cu_files = [str(f) for f in Path('csrc').rglob('*.cu')]
    all_sources = cpp_files + cu_files if WITH_CUDA else cpp_files

    define_macros = [('WITH_PYTHON', None)]
    undef_macros = []
    
    extra_compile_args = {'cxx': ['-O3']}
    extra_link_args = ['-s']

    info = parallel_info()
    if ('backend: OpenMP' in info and 'OpenMP not found' not in info):
        extra_compile_args['cxx'] += ['-DAT_PARALLEL_OPENMP']
        extra_compile_args['cxx'] += ['-fopenmp']
    else:
        print('Compiling without OpenMP...')

    if WITH_CUDA:
        define_macros += [('WITH_CUDA', None)]
        nvcc_flags = os.getenv('NVCC_FLAGS', '')
        nvcc_flags = [] if nvcc_flags == '' else nvcc_flags.split(' ')
        nvcc_flags += ['-O2']
        extra_compile_args['nvcc'] = nvcc_flags

    Extension = CUDAExtension if WITH_CUDA else CppExtension 

    extension = Extension(
        'torch_points.torch_points_csrc',
        all_sources,
        include_dirs=[Path('csrc').absolute()],
        define_macros=define_macros,
        undef_macros=undef_macros,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
    )
    return [extension]
    
install_requires = [
]

test_requires = [
    'pytest',
]

__version__ = '0.0.1'
URL = 'https://github.com/ThibaultLejemble/pytorch_points'

setup(
    name='torch_points',
    version=__version__,
    description=('PyTorch Extension Library for Optimized Point Cloud Processing'),
    author='Thibault Lejemble',
    author_email='tlejemble@gmail.com',
    url=URL,
    # download_url=f'{URL}/archive/{__version__}.tar.gz',
    keywords=[
        'pytorch',
        'geometric-deep-learning',
        'point-cloud',
    ],
    python_requires='>=3.8',
    install_requires=install_requires,
    extras_require={
        'test': test_requires,
    },
    ext_modules=get_extensions(),
    cmdclass={
        'build_ext':
        BuildExtension.with_options(no_python_abi_suffix=True, use_ninja=True)
    },
    packages=find_packages(),
    include_package_data=True,
)
