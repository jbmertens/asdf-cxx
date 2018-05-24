#ifndef ASDF_HPP
#define ASDF_HPP

#include "asdf_asdf.hpp"
#include "asdf_byteorder.hpp"
#include "asdf_datatype.hpp"
#include "asdf_group.hpp"
#include "asdf_io.hpp"
#include "asdf_ndarray.hpp"
#include "asdf_stl.hpp"
#include "asdf_table.hpp"

// namespace ASDF {

// template <typename T>
// writer_state &operator<<(writer_state &ws, const T &value) {
//   ws.emitter << value;
//   return ws;
// }

// } // namespace ASDF

#define ASDF_HPP_DONE
#endif // #ifndef ASDF_HPP
#ifndef ASDF_HPP_DONE
#error "Cyclic include depencency"
#endif
