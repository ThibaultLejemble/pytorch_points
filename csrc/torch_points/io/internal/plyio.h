#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

#ifndef PLYIO_ASSERT
    #include <assert.h>
    #define PLYIO_ASSERT(expr) assert(expr)
#endif

//
// PLY reader and writer
// ---------------------
//
// Notes
// - property list
//      - property list with varying size is not supported
//      - a constant size is required by the user (like 3 to read triangle indices)
//      - if the list size in the file is 
//           - greater: additional values are ignored
//           - lower: extra allocated memory is untouched
// - offset, outter stride, inner stride are always in bytes

namespace plyio {

// Types -----------------------------------------------------------------------

using char_t   = signed char;   // 1 bytes
using uchar_t  = unsigned char; // 1 bytes
using short_t  = int16_t;       // 2 bytes
using ushort_t = uint16_t;      // 2 bytes
using int_t    = int32_t;       // 4 bytes
using uint_t   = uint32_t;      // 4 bytes
using float_t  = float;         // 4 bytes
using double_t = double;        // 8 bytes

enum Type
{
    type_char,
    type_uchar,
    type_short,
    type_ushort,
    type_int,
    type_uint,
    type_float,
    type_double,
    type_unkown
};

namespace internal {

inline Type to_type(const std::string& str);
inline std::string to_string(Type type);
inline std::size_t size_of(Type type);

// String ----------------------------------------------------------------------

//! \brief to_tokens extracts tokens delimited by space in a string
inline std::vector<std::string> to_tokens(const std::string& str);

} // namespace internal

// Error ----------------------------------------------------------------------

namespace internal {
class ErrorManager
{
public:
    inline bool has_error() const;
    inline const std::vector<std::string>& errors() const;

    inline bool has_warning() const;
    inline const std::vector<std::string>& warnings() const;

protected:
    std::vector<std::string> m_errors;
    std::vector<std::string> m_warnings;
};

// Memory ---------------------------------------------------------------------

// get address of the i-th element from ptr using offset and stride (in bytes)
inline const void* get_addr(int i, const void* ptr, int offset, int stride);
inline       void* get_addr(int i,       void* ptr, int offset, int stride);

// get address of the j-th element in the i-th list from ptr using offset and inner/outter stride (in bytes)
inline const void* get_addr(int i, int j, const void* ptr, int offset, int outter_stride, int inner_stride);
inline       void* get_addr(int i, int j,       void* ptr, int offset, int outter_stride, int inner_stride);

} // namespace internal

// Reading --------------------------------------------------------------------

class RProperty
{
public:
    inline RProperty(const std::string name, Type dtype, Type stype = Type::type_unkown) :
        m_name(name),
        m_dtype(dtype),
        m_stype(stype),
        m_data_ptr(nullptr),
        m_list_size(0),
        m_offset(0),
        m_stride(0),
        m_inner_stride(0) {}

public:
    // do not use m_list_size as it is a user-side setting not read in header
    inline bool is_list() const {return m_stype != Type::type_unkown;}
    inline bool ignore() const {return m_data_ptr == nullptr;}

    inline void read(void* data_ptr, int offset, int stride);
    inline void read_list(void* data_ptr, int list_size, int offset, int stride, int outter_stride);

    inline const std::string& name() const {return m_name;}
    inline Type dtype() const {return m_dtype;}
    inline Type stype() const {return m_stype;}
    inline int list_size() const {return m_list_size;}

public:
    template<typename T> void set_value(int i, T val) 
    {
        void* addr = internal::get_addr(i, m_data_ptr, m_offset, m_stride);
        T* addr_t = reinterpret_cast<T*>(addr);
        *addr_t = val;
    }
    template<typename T> void set_value(int i, int j, T val)
    {
        void* addr = internal::get_addr(i, j, m_data_ptr, m_offset, m_stride, m_inner_stride);
        T* addr_t = reinterpret_cast<T*>(addr);
        *addr_t = val;
    }

protected:
    // read from PLY header file
    std::string m_name;
    Type        m_dtype;
    Type        m_stype;
    // set by user using read() or read_list()
    void*       m_data_ptr;
    int         m_list_size; // required constant size
    int         m_offset;
    int         m_stride; // outter stride
    int         m_inner_stride;
};

struct RElement
{
    std::string name;
    int count;
    std::vector<RProperty> properties;
};

// PLYReader -------------------------------------------------------------------

class PLYReader : public internal::ErrorManager
{
public:
    inline PLYReader();

    // Reading -----------------------------------------------------------------
public:
    inline bool read_header(const std::string& filename);
    inline bool read_header(std::istream& is);
    inline bool read_body(const std::string& filename);
    inline bool read_body(std::istream& is);

    // Internal reading --------------------------------------------------------
protected:
    inline bool read_body_ascii(std::istream& is);
    inline bool read_body_binary(std::istream& is);

    // Reading Info getters ---------------------------------------------------
public:
    inline bool ascii() const;
    inline bool binary() const;
    inline bool binary_little_endian() const;
    inline bool binary_big_endian() const;
    inline int  version() const;

public:
    inline bool has_element(const std::string& element_name) const;
    inline bool has_property(const std::string& element_name, const std::string& property_name) const;

    inline RElement& element(const std::string& element_name);
    inline RProperty& property(const std::string& element_name, const std::string& property_name);
    inline std::vector<RProperty>& properties(const std::string& element_name);

    inline const std::vector<std::string>& comments() const;

    inline const std::vector<RElement>& elements() const;
    inline       std::vector<RElement>& elements();

    inline int element_count(const std::string& element_name) const;

    // Data --------------------------------------------------------------------
protected:
    bool m_ascii;
    bool m_binary_little_endian;
    bool m_binary_big_endian;
    int  m_version;

    std::vector<std::string> m_comments;
    std::vector<RElement> m_elements;
};

// Writing --------------------------------------------------------------------

class WProperty
{
public:
    // non-list property
    inline WProperty(
        const std::string name, 
        Type dtype,
        const void* data_ptr,
        int offset,
        int stride)
        :
        m_name(name),
        m_data_ptr(data_ptr),
        m_list_size(0),
        m_dtype(dtype),
        m_offset(offset),
        m_stride(stride),
        m_inner_stride(0) 
        {}
    // list property
    inline WProperty(
        const std::string name, 
        Type dtype,
        int list_size, 
        const void* data_ptr,
        int offset,
        int stride,
        int inner_stride) 
        :
        m_name(name),
        m_data_ptr(data_ptr),
        m_list_size(list_size),
        m_dtype(dtype),
        m_offset(offset),
        m_stride(stride),
        m_inner_stride(inner_stride) 
        {}

public:
    inline bool is_list() const {return m_list_size > 0;}

    inline const std::string& name() const {return m_name;}
    inline Type dtype() const {return m_dtype;}
    // inline Type stype() const {return m_stype;} // custom stype not yet implemented
    inline int list_size() const {return m_list_size;}

public:
    template<typename T> T value(int i) const
    {
        const void* addr = internal::get_addr(i, m_data_ptr, m_offset, m_stride);
        const T* addr_t = reinterpret_cast<const T*>(addr);
        return *addr_t;
    }

    template<typename T> T value(int i, int j) const
    {
        const void* addr = internal::get_addr(i, j, m_data_ptr, m_offset, m_stride, m_inner_stride);
        const T* addr_t = reinterpret_cast<const T*>(addr);
        return *addr_t;
    }

protected:
    std::string m_name;
    const void* m_data_ptr;
    int         m_list_size;
    Type        m_dtype;
    int         m_offset;
    int         m_stride; // outter stride
    int         m_inner_stride;
};

struct WElement
{
    std::string name;
    int count;
    std::vector<WProperty> properties;
};

// PLYWriter -------------------------------------------------------------------

class PLYWriter : public internal::ErrorManager
{
public:
    inline PLYWriter();

    // Writing -----------------------------------------------------------------
public:
    inline bool write(const std::string& filename);
    inline bool write(std::ostream& os);

    // Internal writing --------------------------------------------------------
protected:
    inline bool write_header(std::ostream& os);
    inline bool write_body(std::ostream& os);

    inline bool write_body_ascii(std::ostream& os);
    inline bool write_body_binary(std::ostream& os);

    // Writing Info setters ---------------------------------------------------
public:
    inline void set_ascii();
    inline void set_binary();
    inline void set_binary_little_endian();
    inline void set_binary_big_endian();
    inline void set_version(int version);

public:
    inline void add_comment(const std::string& comment);
    inline void add_element(const std::string& element_name, int element_count);
    inline void add_property(
        const std::string& element_name, 
        const std::string& property_name,
        const void* data_ptr,
        Type dtype,
        int stride,
        int offset);
    inline void add_list_property(
        const std::string& element_name, 
        const std::string& property_name, 
        int list_size,
        const void* data_ptr,
        Type dtype,
        int offset,
        int stride,
        int inner_stride);
        
    // Data --------------------------------------------------------------------
protected:
    bool m_ascii;
    bool m_binary_little_endian;
    bool m_binary_big_endian;
    int  m_version;

    std::vector<std::string> m_comments;
    std::vector<WElement> m_elements;
};

} // namespace plyio













////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////











namespace plyio {

// Types -----------------------------------------------------------------------

namespace internal {

Type to_type(const std::string& str)
{
    if(str == "char"  ) return type_char;
    if(str == "uchar" ) return type_uchar;
    if(str == "short" ) return type_short;
    if(str == "ushort") return type_ushort;
    if(str == "int"   ) return type_int;
    if(str == "uint"  ) return type_uint;
    if(str == "float" ) return type_float;
    if(str == "double") return type_double;
    else                return type_unkown;
}

std::string to_string(Type type)
{
    switch(type)
    {
    case type_char:   return "char";
    case type_uchar:  return "uchar";
    case type_short:  return "short";
    case type_ushort: return "ushort";
    case type_int:    return "int";
    case type_uint:   return "uint";
    case type_float:  return "float";
    case type_double: return "double";
    default:          return "unkown";
    }
}

std::size_t size_of(Type type)
{
    switch(type)
    {
    case type_char:   return sizeof(char_t);    // 1
    case type_uchar:  return sizeof(uchar_t);   // 1
    case type_short:  return sizeof(short_t);   // 2
    case type_ushort: return sizeof(ushort_t);  // 2
    case type_int:    return sizeof(int_t);     // 4
    case type_uint:   return sizeof(uint_t);    // 4
    case type_float:  return sizeof(float_t);   // 4
    case type_double: return sizeof(double_t);  // 8
    default:          return 0;
    }
}


// String ----------------------------------------------------------------------

std::vector<std::string> to_tokens(const std::string& str)
{
    std::stringstream ss(str);
    std::string token;
    std::vector<std::string> tokens;
    while(std::getline(ss, token, ' ')) {
        if(not token.empty()) {
            tokens.emplace_back(std::move(token));
        }
    }
    return tokens;
}

} // namespace internal

// Error ----------------------------------------------------------------------

namespace internal {

bool ErrorManager::has_error() const
{
    return not m_errors.empty();
}

const std::vector<std::string>& ErrorManager::errors() const
{
    return m_errors;
}

bool ErrorManager::has_warning() const
{
    return not m_warnings.empty();
}

const std::vector<std::string>& ErrorManager::warnings() const
{
    return m_warnings;
}

// Memory ---------------------------------------------------------------------

// get address of the i-th element from data_ptr using offset and stride (in bytes)
const void* get_addr(int i, const void* data_ptr, int offset, int stride)
{
    const char* addr = static_cast<const char*>(data_ptr) + offset + i * stride;
    return static_cast<const void*>(addr);
}

void* get_addr(int i, void* data_ptr, int offset, int stride)
{
    char* addr = static_cast<char*>(data_ptr) + offset + i * stride;
    return static_cast<void*>(addr);
}

// get address of the j-th element in the i-th list from data_ptr using offset and inner/outter stride (in bytes)
const void* get_addr(int i, int j, const void* data_ptr, int offset, int outter_stride, int inner_stride)
{
    const char* addr = static_cast<const char*>(data_ptr) + offset + i * outter_stride + j * inner_stride;
    return static_cast<const void*>(addr);
}

void* get_addr(int i, int j, void* data_ptr, int offset, int outter_stride, int inner_stride) 
{
    char* addr = static_cast<char*>(data_ptr) + offset + i * outter_stride + j * inner_stride;
    return static_cast<void*>(addr);
}

} // namespace internal

// Reading --------------------------------------------------------------------

void RProperty::read(void* data_ptr, int offset, int stride)
{
    m_data_ptr = data_ptr;
    m_list_size = 0;
    m_offset = offset;
    m_stride = stride;
    m_inner_stride = 0;
}

void RProperty::read_list(void* data_ptr, int list_size, int offset, int stride, int inner_stride)
{
    m_data_ptr = data_ptr;
    m_list_size = list_size;
    m_offset = offset;
    m_stride = stride;
    m_inner_stride = inner_stride;
}

// PLYReader ------------------------------------------------------------------

PLYReader::PLYReader() :
    m_ascii(false),
    m_binary_little_endian(false),
    m_binary_big_endian(false),
    m_version(0),
    m_comments(0),
    m_elements(0)
{
}

bool PLYReader::read_header(const std::string& filename)
{
    m_errors.clear();
    std::ifstream ifs(filename);
    if(ifs.is_open()) {
        return this->read_header(ifs);
    }
    else {
        m_errors.push_back(
            "Failed to open file '" + filename + "'");
        return false;
    }
}

bool PLYReader::read_header(std::istream& is)
{
    // reset
    m_ascii = false;
    m_binary_little_endian = true;
    m_binary_big_endian = false;
    m_version = 0;
    m_comments.clear();
    m_elements.clear();
    m_errors.clear();

    int line_num = 1;
    std::string line;

    std::getline(is, line);
    if(line != "ply")
    {
        m_errors.push_back(
            "Line " + std::to_string(line_num) +
            ": expected 'ply', found '" + line + "'");
        return false;
    }

    bool end_header_found = false;
    while(std::getline(is, line))
    {
        ++line_num;
        const auto tokens = internal::to_tokens(line);

        if(tokens.empty())
        {
            continue; // skip empty lines
        }
        else if(line == "end_header")
        {
            end_header_found = true;
            break;
        }
        else if(tokens.front() == "format")
        {
            if(tokens.size() != 3) // format ascii 1.0
            {
                m_errors.push_back(
                    "Line " + std::to_string(line_num) +
                    ": expected 3 tokens (e.g. 'format ascii 1.0')," +
                    " found " + std::to_string(tokens.size()));
                return false;
            }
            m_ascii                 = tokens[1] == "ascii";
            m_binary_little_endian  = tokens[1] == "binary_little_endian";
            m_binary_big_endian     = tokens[1] == "binary_big_endian";
            if(not (m_ascii or m_binary_little_endian or m_binary_big_endian))
            {
                m_errors.push_back(
                    "Line " + std::to_string(line_num) +
                    ": 'ascii', 'binary_big_endian', or 'binary_little_endian' required,"
                    " found '" + tokens[1] + "'");
                return false;
            }
            m_version = std::stoi(tokens[2]); //TODO version is not one single integer
        }
        else if(tokens.front() == "comment")
        {
            m_comments.emplace_back(line.substr(8));
        }
        else if(tokens.front() == "element")
        {
            if(tokens.size() != 3) // element vertex 128
            {
                m_errors.push_back(
                    "Line " + std::to_string(line_num) +
                    ": expected 3 tokens (e.g. 'element vertex 128')," +
                    " found " + std::to_string(tokens.size()));
                return false;
            }
            const auto& name = tokens[1];
            const int count = std::stoi(tokens[2]);
            m_elements.push_back({name,count,{/*properties*/}});
        }
        else if(tokens.front() == "property")
        {
            if(m_elements.empty())
            {
                m_errors.push_back(
                    "Line " + std::to_string(line_num) +
                    ": element required before property declaration");
                return false;
            }
            else if(tokens.size() != 3 and // property float x
                    tokens.size() != 5)   // property list int int vertex_indices
            {
                m_errors.push_back(
                    "Line " + std::to_string(line_num) +
                    ": expected 3 or 5 tokens (e.g. 'property float x'" +
                    " or 'property list int int vertex_indices')," +
                    " found " + std::to_string(tokens.size()));
                return false;
            }

            if(tokens[1] == "list")
            {
                if(tokens.size() != 5) // property list int int vertex_indices
                {
                    m_errors.push_back(
                        "Line " + std::to_string(line_num) +
                        ": expected 5 tokens (e.g. 'property list int int vertex_indices')," +
                        " found " + std::to_string(tokens.size()));
                    return false;
                }
                const auto stype = internal::to_type(tokens[2]);
                const auto dtype = internal::to_type(tokens[3]);
                const auto& name = tokens[4];
                m_elements.back().properties.push_back({name,dtype,stype});
            }
            else
            {
                if(tokens.size() != 3) // property float x
                {
                    m_errors.push_back(
                        "Line " + std::to_string(line_num) +
                        ": expected 3 tokens (e.g. 'property float x')," +
                        " found " + std::to_string(tokens.size()));
                    return false;
                }
                const auto dtype = internal::to_type(tokens[1]);
                const auto name = tokens[2];
                m_elements.back().properties.push_back({name,dtype});
            }
        }
        else
        {
            m_warnings.push_back(
                "Line " + std::to_string(line_num) +
                ": unkown header line '" + line + "'");
        }
    } // while

    if(not end_header_found)
    {
        m_errors.push_back("Line 'end_header' not found");
        return false;
    }

    return end_header_found;
}

bool PLYReader::read_body(const std::string& filename)
{
//    m_errors.clear();
    std::ifstream ifs(filename);
    if(ifs.is_open())
    {
        // jump header
        std::string line;
        while(std::getline(ifs, line))
        {
            if(line == "end_header")
            {
                return this->read_body(ifs);
            }
        }
        m_errors.push_back("Line 'end_header' not found");
        return false;
    }
    else
    {
        m_errors.push_back("Failed to open file '" + filename + "'");
        return false;
    }
}

bool PLYReader::read_body(std::istream& is)
{
    if(m_ascii)
    {
        return this->read_body_ascii(is);
    }
    else if(m_binary_big_endian || m_binary_little_endian)
    {
        return this->read_body_binary(is);
    }
    else
    {
        m_errors.push_back("ascii, binary_big_endian, or binary_little_endian required");
        return false;
    }
}

// Internal reading ------------------------------------------------------------

bool PLYReader::read_body_ascii(std::istream& is)
{
    char_t   val0;
    uchar_t  val1;
    short_t  val2;
    ushort_t val3;
    int_t    val4;
    uint_t   val5;
    float_t  val6;
    double_t val7;

    short_t  tmp0;
    ushort_t tmp1;

    for(size_t idx_element = 0; idx_element < m_elements.size(); ++idx_element)
    {
        RElement& element = m_elements[idx_element];
        for(int i = 0; i < element.count; ++i)
        {
            for(size_t idx_property = 0; idx_property < element.properties.size(); ++idx_property)
            {
                RProperty& prop = element.properties[idx_property];
                if(prop.is_list())
                {
                    int size = 0;
                    switch (prop.stype())
                    {
                    case type_char:    is >> tmp0; val0 = tmp0; size = int(val0); break;
                    case type_uchar:   is >> tmp1; val1 = tmp1; size = int(val1); break;
                    case type_short:   is >> val2;              size = int(val2); break;
                    case type_ushort:  is >> val3;              size = int(val3); break;
                    case type_int:     is >> val4;              size = int(val4); break;
                    case type_uint:    is >> val5;              size = int(val5); break;
                    case type_float:   is >> val6;              size = int(val6); break;
                    case type_double:  is >> val7;              size = int(val7); break;
                    default:           PLYIO_ASSERT(false);
                    }

                    if(prop.list_size() < size) {
                        //TODO warning: extra values are ignored
                        size = prop.list_size();
                    }

                    switch (prop.dtype())
                    {
                    case type_char:   for(int j=0; j<size; ++j) {is >> tmp0; val0 = tmp0; if(not prop.ignore()){prop.set_value(i,j,val0);}} break;
                    case type_uchar:  for(int j=0; j<size; ++j) {is >> tmp1; val1 = tmp1; if(not prop.ignore()){prop.set_value(i,j,val1);}} break;
                    case type_short:  for(int j=0; j<size; ++j) {is >> val2;              if(not prop.ignore()){prop.set_value(i,j,val2);}} break;
                    case type_ushort: for(int j=0; j<size; ++j) {is >> val3;              if(not prop.ignore()){prop.set_value(i,j,val3);}} break;
                    case type_int:    for(int j=0; j<size; ++j) {is >> val4;              if(not prop.ignore()){prop.set_value(i,j,val4);}} break;
                    case type_uint:   for(int j=0; j<size; ++j) {is >> val5;              if(not prop.ignore()){prop.set_value(i,j,val5);}} break;
                    case type_float:  for(int j=0; j<size; ++j) {is >> val6;              if(not prop.ignore()){prop.set_value(i,j,val6);}} break;
                    case type_double: for(int j=0; j<size; ++j) {is >> val7;              if(not prop.ignore()){prop.set_value(i,j,val7);}} break;
                    default:          PLYIO_ASSERT(false);
                    }
                }
                else
                {
                    switch (prop.dtype())
                    {
                    case type_char:   is >> tmp0; val0 = tmp0; if(not prop.ignore()){prop.set_value(i, val0);} break;
                    case type_uchar:  is >> tmp1; val1 = tmp1; if(not prop.ignore()){prop.set_value(i, val1);} break;
                    case type_short:  is >> val2;              if(not prop.ignore()){prop.set_value(i, val2);} break;
                    case type_ushort: is >> val3;              if(not prop.ignore()){prop.set_value(i, val3);} break;
                    case type_int:    is >> val4;              if(not prop.ignore()){prop.set_value(i, val4);} break;
                    case type_uint:   is >> val5;              if(not prop.ignore()){prop.set_value(i, val5);} break;
                    case type_float:  is >> val6;              if(not prop.ignore()){prop.set_value(i, val6);} break;
                    case type_double: is >> val7;              if(not prop.ignore()){prop.set_value(i, val7);} break;
                    default:          PLYIO_ASSERT(false);
                    }
                }
            }
        }
    }
    return true;
}

bool PLYReader::read_body_binary(std::istream& is)
{
    char_t   val0;
    uchar_t  val1;
    short_t  val2;
    ushort_t val3;
    int_t    val4;
    uint_t   val5;
    float_t  val6;
    double_t val7;

//    short_t  tmp0;
//    ushort_t tmp1;

    for(size_t idx_element = 0; idx_element < m_elements.size(); ++idx_element)
    {
        RElement& element = m_elements[idx_element];
        for(int i = 0; i < element.count; ++i)
        {
            for(size_t idx_property = 0; idx_property < element.properties.size(); ++idx_property)
            {
                RProperty& prop = element.properties[idx_property];
                if(prop.is_list())
                {
                    int size = 0;
                    switch (prop.stype())
                    {
                    case type_char:       is.read(reinterpret_cast<char*>(&val0), sizeof(char_t));   size = int(val0); break;
                    case type_uchar:      is.read(reinterpret_cast<char*>(&val1), sizeof(uchar_t));  size = int(val1); break;
                    case type_short:      is.read(reinterpret_cast<char*>(&val2), sizeof(short_t));  size = int(val2); break;
                    case type_ushort:     is.read(reinterpret_cast<char*>(&val3), sizeof(ushort_t)); size = int(val3); break;
                    case type_int:        is.read(reinterpret_cast<char*>(&val4), sizeof(int_t));    size = int(val4); break;
                    case type_uint:       is.read(reinterpret_cast<char*>(&val5), sizeof(uint_t));   size = int(val5); break;
                    case type_float:      is.read(reinterpret_cast<char*>(&val6), sizeof(float_t));  size = int(val6); break;
                    case type_double:     is.read(reinterpret_cast<char*>(&val7), sizeof(double_t)); size = int(val7); break;
                    default:              PLYIO_ASSERT(false);
                    }

                    if(prop.list_size() < size) {
                        //TODO warning: extra values are ignored
                        size = prop.list_size();
                    }

                    switch (prop.dtype())
                    {
                    case type_char:       for(int j=0; j<size; ++j) {is.read(reinterpret_cast<char*>(&val0), sizeof(char_t));   if(not prop.ignore()){prop.set_value(i,j,val0);}} break;
                    case type_uchar:      for(int j=0; j<size; ++j) {is.read(reinterpret_cast<char*>(&val1), sizeof(uchar_t));  if(not prop.ignore()){prop.set_value(i,j,val1);}} break;
                    case type_short:      for(int j=0; j<size; ++j) {is.read(reinterpret_cast<char*>(&val2), sizeof(short_t));  if(not prop.ignore()){prop.set_value(i,j,val2);}} break;
                    case type_ushort:     for(int j=0; j<size; ++j) {is.read(reinterpret_cast<char*>(&val3), sizeof(ushort_t)); if(not prop.ignore()){prop.set_value(i,j,val3);}} break;
                    case type_int:        for(int j=0; j<size; ++j) {is.read(reinterpret_cast<char*>(&val4), sizeof(int_t));    if(not prop.ignore()){prop.set_value(i,j,val4);}} break;
                    case type_uint:       for(int j=0; j<size; ++j) {is.read(reinterpret_cast<char*>(&val5), sizeof(uint_t));   if(not prop.ignore()){prop.set_value(i,j,val5);}} break;
                    case type_float:      for(int j=0; j<size; ++j) {is.read(reinterpret_cast<char*>(&val6), sizeof(float_t));  if(not prop.ignore()){prop.set_value(i,j,val6);}} break;
                    case type_double:     for(int j=0; j<size; ++j) {is.read(reinterpret_cast<char*>(&val7), sizeof(double_t)); if(not prop.ignore()){prop.set_value(i,j,val7);}} break;
                    default:              PLYIO_ASSERT(false);
                    }
                }
                else
                {
                    switch (prop.dtype())
                    {
                    case type_char:       is.read(reinterpret_cast<char*>(&val0), sizeof(char_t));   if(not prop.ignore()){prop.set_value(i,val0);} break;
                    case type_uchar:      is.read(reinterpret_cast<char*>(&val1), sizeof(uchar_t));  if(not prop.ignore()){prop.set_value(i,val1);} break;
                    case type_short:      is.read(reinterpret_cast<char*>(&val2), sizeof(short_t));  if(not prop.ignore()){prop.set_value(i,val2);} break;
                    case type_ushort:     is.read(reinterpret_cast<char*>(&val3), sizeof(ushort_t)); if(not prop.ignore()){prop.set_value(i,val3);} break;
                    case type_int:        is.read(reinterpret_cast<char*>(&val4), sizeof(int_t));    if(not prop.ignore()){prop.set_value(i,val4);} break;
                    case type_uint:       is.read(reinterpret_cast<char*>(&val5), sizeof(uint_t));   if(not prop.ignore()){prop.set_value(i,val5);} break;
                    case type_float:      is.read(reinterpret_cast<char*>(&val6), sizeof(float_t));  if(not prop.ignore()){prop.set_value(i,val6);} break;
                    case type_double:     is.read(reinterpret_cast<char*>(&val7), sizeof(double_t)); if(not prop.ignore()){prop.set_value(i,val7);} break;
                    default:              PLYIO_ASSERT(false);
                    }
                }
            }
        }
    }
    return true;
}

// Reading Info getters -------------------------------------------------------

bool PLYReader::ascii() const
{
    return m_ascii;
}

bool PLYReader::binary() const
{
    return m_binary_big_endian || m_binary_little_endian;
}

bool PLYReader::binary_little_endian() const
{
    return m_binary_little_endian;
}

bool PLYReader::binary_big_endian() const
{
    return m_binary_big_endian;
}

int PLYReader::version() const
{
    return m_version;
}

bool PLYReader::has_element(const std::string& element_name) const
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [&element_name](const auto& e) {
        return e.name == element_name;
    });
    return it != m_elements.end();
}

bool PLYReader::has_property(const std::string& element_name, const std::string& property_name) const
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [&element_name](const auto& e) {
        return e.name == element_name;
    });
    if(it == m_elements.end()) {
        return false;
    }
    const auto& e = *it;
    const auto it2 = std::find_if(e.properties.begin(), e.properties.end(), [&property_name](const auto& p) {
        return p.name() == property_name;
    });
    return it2 != e.properties.end();
}

RElement& PLYReader::element(const std::string& element_name)
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [&element_name](const auto& e) {
        return e.name == element_name;
    });
    PLYIO_ASSERT(it != m_elements.end()); // element does not exist
    return *it;
}

RProperty& PLYReader::property(const std::string& element_name, const std::string& property_name)
{
    auto& e = element(element_name);
    const auto it = std::find_if(e.properties.begin(), e.properties.end(), [&property_name](const auto& p) {
        return p.name() == property_name;
    });
    PLYIO_ASSERT(it != e.properties.end()); // property does not exist
    return *it;
}

std::vector<RProperty>& PLYReader::properties(const std::string& element_name)
{
    auto& e = element(element_name);
    return e.properties;
}

const std::vector<std::string>& PLYReader::comments() const
{
    return m_comments;
}

const std::vector<RElement>& PLYReader::elements() const
{
    return m_elements;
}

std::vector<RElement>& PLYReader::elements()
{
    return m_elements;
}

int PLYReader::element_count(const std::string& element_name) const
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [&element_name](const auto& e) {
        return e.name == element_name;
    });
    return it == m_elements.end() ? 0 : it->count;
}

// PLYWriter -------------------------------------------------------------------

PLYWriter::PLYWriter() :
    m_ascii(false),
    m_binary_little_endian(true),
    m_binary_big_endian(false),
    m_version(1),
    m_comments(0),
    m_elements(0)
{
}

bool PLYWriter::write(const std::string& filename)
{
    std::ofstream ofs(filename);
    if(ofs.is_open())
    {
        return this->write(ofs);
    }
    else
    {
        m_errors.push_back("Failed to open file '" + filename + "'");
        return false;
    }
}

bool PLYWriter::write(std::ostream& os)
{
    return this->write_header(os) && this->write_body(os);
}

// Internal writing ------------------------------------------------------------

bool PLYWriter::write_header(std::ostream& os)
{
    os << "ply\n";
    os << "format ";

    if(m_ascii)
    {
        os << "ascii";
    }
    else if(m_binary_little_endian)
    {
        os << "binary_little_endian";
    }
    else if(m_binary_big_endian)
    {
        os << "binary_big_endian";
    }
    else
    {
        PLYIO_ASSERT(false); // neither ascii not binary
    }

    os << " " << m_version << ".0\n";   //TODO make this format clear: 1.0?

    for(const std::string& comment : m_comments)
    {
        os << "comment " << comment << "\n";
    }

    for(const WElement& element : m_elements)
    {
        os << "element " << element.name << " " << element.count << "\n";
        for(const WProperty& property : element.properties)
        {
            if(property.is_list())
            {
                // TODO custom stype instead of default int
                os << "property list " << internal::to_string(Type::type_int) << " " << internal::to_string(property.dtype()) << " " << property.name() << "\n";
            }
            else
            {
                os << "property " << internal::to_string(property.dtype()) << " " << property.name() << "\n";
            }
        }
    }

    os << "end_header\n";
    return true;
}

bool PLYWriter::write_body(std::ostream& os)
{
    if(m_ascii)
    {
        return this->write_body_ascii(os);
    }
    else if(m_binary_big_endian || m_binary_little_endian)
    {
        return this->write_body_binary(os);
    }
    else
    {
        PLYIO_ASSERT(false); // neither ascii not binary
        return false;
    }
}

bool PLYWriter::write_body_ascii(std::ostream& os)
{
    for(size_t idx_element = 0; idx_element < m_elements.size(); ++idx_element)
    {
        const WElement& element = m_elements[idx_element];
        for(int i = 0; i < element.count; ++i)
        {
            for(size_t idx_property = 0; idx_property < element.properties.size(); ++idx_property)
            {
                const WProperty& prop = element.properties[idx_property];
                if(prop.is_list())
                {
                    const int size = prop.list_size();
                    os << size << " ";

                    switch (prop.dtype())
                    {
                    case type_char:   for(int j=0; j<size; ++j) {os <<  short_t(prop.value<char_t>  (i,j)) << " ";} break;
                    case type_uchar:  for(int j=0; j<size; ++j) {os << ushort_t(prop.value<uchar_t> (i,j)) << " ";} break;
                    case type_short:  for(int j=0; j<size; ++j) {os <<          prop.value<short_t> (i,j)  << " ";} break;
                    case type_ushort: for(int j=0; j<size; ++j) {os <<          prop.value<ushort_t>(i,j)  << " ";} break;
                    case type_int:    for(int j=0; j<size; ++j) {os <<          prop.value<int_t>   (i,j)  << " ";} break;
                    case type_uint:   for(int j=0; j<size; ++j) {os <<          prop.value<uint_t>  (i,j)  << " ";} break;
                    case type_float:  for(int j=0; j<size; ++j) {os <<          prop.value<float_t> (i,j)  << " ";} break;
                    case type_double: for(int j=0; j<size; ++j) {os <<          prop.value<double_t>(i,j)  << " ";} break;
                    default:          PLYIO_ASSERT(false);
                    }
                }
                else
                {
                    switch (prop.dtype())
                    {
                    case type_char:   os <<  short_t(prop.value<char_t>  (i)); break;
                    case type_uchar:  os << ushort_t(prop.value<uchar_t> (i)); break;
                    case type_short:  os <<          prop.value<short_t> (i) ; break;
                    case type_ushort: os <<          prop.value<ushort_t>(i) ; break;
                    case type_int:    os <<          prop.value<int_t>   (i) ; break;
                    case type_uint:   os <<          prop.value<uint_t>  (i) ; break;
                    case type_float:  os <<          prop.value<float_t> (i) ; break;
                    case type_double: os <<          prop.value<double_t>(i) ; break;
                    default:          PLYIO_ASSERT(false);
                    }
                }
                os << " ";
            }
            os << "\n";
        }
    }
    return true;
}

bool PLYWriter::write_body_binary(std::ostream& os)
{
    for(size_t idx_element = 0; idx_element < m_elements.size(); ++idx_element)
    {
        const WElement& element = m_elements[idx_element];
        for(int i = 0; i < element.count; ++i)
        {
            for(size_t idx_property = 0; idx_property < element.properties.size(); ++idx_property)
            {
                const WProperty& prop = element.properties[idx_property];
                if(prop.is_list())
                {
                    const int size = prop.list_size();
                    os.write(reinterpret_cast<const char*>(&size), sizeof(int));

                    switch (prop.dtype())
                    {
                    case type_char:   for(int j=0; j<size; ++j) {const auto value = prop.value<char_t  >(i,j); os.write(reinterpret_cast<const char*>(&value), sizeof(char_t));}   break;
                    case type_uchar:  for(int j=0; j<size; ++j) {const auto value = prop.value<uchar_t >(i,j); os.write(reinterpret_cast<const char*>(&value), sizeof(uchar_t));}  break;
                    case type_short:  for(int j=0; j<size; ++j) {const auto value = prop.value<short_t >(i,j); os.write(reinterpret_cast<const char*>(&value), sizeof(short_t));}  break;
                    case type_ushort: for(int j=0; j<size; ++j) {const auto value = prop.value<ushort_t>(i,j); os.write(reinterpret_cast<const char*>(&value), sizeof(ushort_t));} break;
                    case type_int:    for(int j=0; j<size; ++j) {const auto value = prop.value<int_t   >(i,j); os.write(reinterpret_cast<const char*>(&value), sizeof(int_t));}    break;
                    case type_uint:   for(int j=0; j<size; ++j) {const auto value = prop.value<uint_t  >(i,j); os.write(reinterpret_cast<const char*>(&value), sizeof(uint_t));}   break;
                    case type_float:  for(int j=0; j<size; ++j) {const auto value = prop.value<float_t >(i,j); os.write(reinterpret_cast<const char*>(&value), sizeof(float_t));}  break;
                    case type_double: for(int j=0; j<size; ++j) {const auto value = prop.value<double_t>(i,j); os.write(reinterpret_cast<const char*>(&value), sizeof(double_t));} break;
                    default:          PLYIO_ASSERT(false);
                    }
                }
                else
                {
                    switch (prop.dtype())
                    {
                    case type_char:   {const auto value = prop.value<char_t  >(i); os.write(reinterpret_cast<const char*>(&value), sizeof(char_t));   break;}
                    case type_uchar:  {const auto value = prop.value<uchar_t >(i); os.write(reinterpret_cast<const char*>(&value), sizeof(uchar_t));  break;}
                    case type_short:  {const auto value = prop.value<short_t >(i); os.write(reinterpret_cast<const char*>(&value), sizeof(short_t));  break;}
                    case type_ushort: {const auto value = prop.value<ushort_t>(i); os.write(reinterpret_cast<const char*>(&value), sizeof(ushort_t)); break;}
                    case type_int:    {const auto value = prop.value<int_t   >(i); os.write(reinterpret_cast<const char*>(&value), sizeof(int_t));    break;}
                    case type_uint:   {const auto value = prop.value<uint_t  >(i); os.write(reinterpret_cast<const char*>(&value), sizeof(uint_t));   break;}
                    case type_float:  {const auto value = prop.value<float_t >(i); os.write(reinterpret_cast<const char*>(&value), sizeof(float_t));  break;}
                    case type_double: {const auto value = prop.value<double_t>(i); os.write(reinterpret_cast<const char*>(&value), sizeof(double_t)); break;}
                    default:          PLYIO_ASSERT(false);
                    }
                }
            }
        }
    }
    return true;
}

// Writing Info setters -------------------------------------------------------

void PLYWriter::set_ascii()
{
    m_ascii                 = true;
    m_binary_little_endian  = false;
    m_binary_big_endian     = false;
}

void PLYWriter::set_binary()
{
    m_ascii                 = false;
    m_binary_little_endian  = true;
    m_binary_big_endian     = false;
}

void PLYWriter::set_binary_little_endian()
{
    m_ascii                 = false;
    m_binary_little_endian  = true;
    m_binary_big_endian     = false;
}

void PLYWriter::set_binary_big_endian()
{
    m_ascii                 = false;
    m_binary_little_endian  = false;
    m_binary_big_endian     = true;
}

void PLYWriter::set_version(int version)
{
    m_version = version;
}

void PLYWriter::add_comment(const std::string& comment)
{
    m_comments.emplace_back(comment);
}

void PLYWriter::add_element(const std::string& element_name, int element_count)
{
    m_elements.push_back({element_name, element_count, {/*properties*/}});
}

void PLYWriter::add_property(
        const std::string& element_name, 
        const std::string& property_name,
        const void* data_ptr,
        Type dtype,
        int offset,
        int stride)
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [&element_name](const auto& e) {
        return e.name == element_name;
    });
    PLYIO_ASSERT(it != m_elements.end());

    it->properties.push_back(WProperty{
        property_name, 
        dtype, 
        data_ptr,  
        offset,
        stride});
}

void PLYWriter::add_list_property(
    const std::string& element_name, 
    const std::string& property_name, 
    int list_size,
    const void* data_ptr,
    Type dtype,
    int offset,
    int stride,
    int inner_stride)
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [&element_name](const auto& e) {
        return e.name == element_name;
    });
    PLYIO_ASSERT(it != m_elements.end());

    it->properties.push_back(WProperty{
        property_name, 
        dtype,
        list_size, 
        data_ptr, 
        offset,
        stride, 
        inner_stride});
}

} // namespace plyio
