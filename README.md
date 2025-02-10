# pytorch_points

## Compiling the C++/CUDA extension

### Using a Python virtual environment `venv`

Install the following dependencies (on Ubuntu)
```bash
sudo apt-get install ninja-build
```

Setup the virtual environment and install the requirements
- `torch`
- `numpy`
- `pytest`
```bash
python3 -m venv venv
source venv/bin/activate
pip install -r requirements
```

Compile `pytorch_points`
```bash
python3 setup.py develop
```

Once `pytorch_points` has been built, if a C++/CUDA file is modified and  no file has been created/deleted, a quicker compilation can be achieved by directly running `ninja`
```bash
ninja -C build/temp.linux-x86_64-cpython-312/
```

Run Python tests 
```bash
pytest
```
