#include <torch_points/io/ply.h>
#include <torch_points/io/txt.h>
#include <torch_points/spatial/grid2D.h>
#include <torch_points/dummy/dummy.h>

using namespace torch_points;

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) 
{
    // ----------------------------------------------------
    m.def("read_ply",         &read_ply);
    m.def("read_ply_data",    &read_ply_data);
    m.def("write_ply",        &write_ply);
    m.def("write_ply_data",   &write_ply_data);
    m.def("read_txt",         &read_txt);
    // ----------------------------------------------------
    m.def("build_grid2d",     &build_grid2d);
    // ----------------------------------------------------
    m.def("dummy",            &dummy);
    // ----------------------------------------------------
}
