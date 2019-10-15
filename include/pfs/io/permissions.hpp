#pragma once
#include <type_traits>

namespace pfs {
namespace io {

enum permission {
      permission_none = 0
    , owner_read   = 1 << 0
    , owner_write  = 1 << 1
    , owner_exec   = 1 << 2
    , group_read   = 1 << 3
    , group_write  = 1 << 4
    , group_exec   = 1 << 5
    , others_read  = 1 << 6
    , others_write = 1 << 7
    , others_exec  = 1 << 8
};

using permissions = std::underlying_type<permission>::type;

}} // pfs::io

