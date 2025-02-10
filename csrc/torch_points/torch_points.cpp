#include <torch_points/dummy/dummy.h>

using namespace torch_points;

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) 
{
    m.def("dummy", &dummy);
}
