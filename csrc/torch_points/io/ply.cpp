#include <torch_points/io/ply.h>

namespace torch_points {
namespace internal {

std::optional<torch::ScalarType> get_torch_dtype(plyio::Type ply_dtype)
{
    switch(ply_dtype)
    {
    case plyio::Type::type_char:    return torch::kInt8;
    case plyio::Type::type_uchar:   return torch::kUInt8;
    case plyio::Type::type_short:   return torch::kInt16;
//  case plyio::Type::type_ushort:  return ...
    case plyio::Type::type_int:     return torch::kInt32;
//  case plyio::Type::type_uint:    return ...
    case plyio::Type::type_float:   return torch::kFloat32;
    case plyio::Type::type_double:  return torch::kFloat64;
    case plyio::Type::type_unkown:  return torch::kFloat32;
    default:                        return {};
    }
    return {};
}

plyio::Type get_ply_type(caffe2::TypeMeta torch_dtype)
{
         if(torch_dtype == torch::kInt8)    return plyio::Type::type_char;
    else if(torch_dtype == torch::kUInt8)   return plyio::Type::type_uchar;
    else if(torch_dtype == torch::kInt16)   return plyio::Type::type_short;
//  else if(torch_dtype == ...)             return plyio::Type::type_ushort;
    else if(torch_dtype == torch::kInt32)   return plyio::Type::type_int;
//  else if(torch_dtype == ...)             return plyio::Type::type_uint;
    else if(torch_dtype == torch::kFloat32) return plyio::Type::type_float;
    else if(torch_dtype == torch::kFloat64) return plyio::Type::type_double;
    return plyio::Type::type_unkown;
}

} // namespace internal
} // namespace torch_points
