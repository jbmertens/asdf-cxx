#include "asdf.hpp"
#include "config.hpp"

#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/emitterstyle.h>
#include <yaml-cpp/yaml.h>

#ifdef HAVE_BZIP2
#include <bzlib.h>
#endif

#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#endif

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <limits>
#include <sstream>
#include <vector>

namespace ASDF {

const string asdf_format_version = "1.0.0";
const string asdf_standard_version = "1.1.0";

////////////////////////////////////////////////////////////////////////////////

void writer_state::flush(ostream &os) {
  if (tasks.empty())
    return;
  YAML::Emitter index;
  index << YAML::Flow;
  index << YAML::BeginSeq;
  for (auto &&task : tasks) {
    index << os.tellp();
    move(task)(os);
  }
  tasks.clear();
  index << YAML::EndSeq;
  os << "#ASDF BLOCK INDEX\n"
     << "%YAML 1.1\n"
     << "---\n"
     << index.c_str() << "\n"
     << "...\n";
}

////////////////////////////////////////////////////////////////////////////////

// Check consistency between id enum and tuple element
static_assert(is_same<get_scalar_type_t<id_bool8>, bool8_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_int8>, int8_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_int16>, int16_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_int32>, int32_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_int64>, int64_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_uint8>, uint8_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_uint16>, uint16_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_uint32>, uint32_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_uint64>, uint64_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_float32>, float32_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_float64>, float64_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_complex64>, complex64_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_complex128>, complex128_t>::value,
              "");
static_assert(is_same<get_scalar_type_t<id_ascii>, ascii_t>::value, "");
static_assert(is_same<get_scalar_type_t<id_ucs4>, ucs4_t>::value, "");

static_assert(get_scalar_type_id<bool8_t>::value == id_bool8, "");
static_assert(get_scalar_type_id<int8_t>::value == id_int8, "");
static_assert(get_scalar_type_id<int16_t>::value == id_int16, "");
static_assert(get_scalar_type_id<int32_t>::value == id_int32, "");
static_assert(get_scalar_type_id<int64_t>::value == id_int64, "");
static_assert(get_scalar_type_id<uint8_t>::value == id_uint8, "");
static_assert(get_scalar_type_id<uint16_t>::value == id_uint16, "");
static_assert(get_scalar_type_id<uint32_t>::value == id_uint32, "");
static_assert(get_scalar_type_id<uint64_t>::value == id_uint64, "");
static_assert(get_scalar_type_id<float32_t>::value == id_float32, "");
static_assert(get_scalar_type_id<float64_t>::value == id_float64, "");
static_assert(get_scalar_type_id<complex64_t>::value == id_complex64, "");
static_assert(get_scalar_type_id<complex128_t>::value == id_complex128, "");
static_assert(get_scalar_type_id<ascii_t>::value == id_ascii, "");
static_assert(get_scalar_type_id<ucs4_t>::value == id_ucs4, "");

size_t get_scalar_type_size(scalar_type_id_t scalar_type_id) {
  switch (scalar_type_id) {
  case id_bool8:
    return sizeof(bool8_t);
  case id_int8:
    return sizeof(int8_t);
  case id_int16:
    return sizeof(int16_t);
  case id_int32:
    return sizeof(int32_t);
  case id_int64:
    return sizeof(int64_t);
  case id_uint8:
    return sizeof(uint8_t);
  case id_uint16:
    return sizeof(uint16_t);
  case id_uint32:
    return sizeof(uint32_t);
  case id_uint64:
    return sizeof(uint64_t);
  case id_float32:
    return sizeof(float32_t);
  case id_float64:
    return sizeof(float64_t);
  case id_complex64:
    return sizeof(complex64_t);
  case id_complex128:
    return sizeof(complex128_t);
    // case id_ascii
    // case id_ucs4
  }
  assert(0);
}

string yaml_encode(scalar_type_id_t scalar_type_id) {
  switch (scalar_type_id) {
  case id_bool8:
    return "bool8";
  case id_int8:
    return "int8";
  case id_int16:
    return "int16";
  case id_int32:
    return "int32";
  case id_int64:
    return "int64";
  case id_uint8:
    return "uint8";
  case id_uint16:
    return "uint16";
  case id_uint32:
    return "uint32";
  case id_uint64:
    return "uint64";
  case id_float32:
    return "float32";
  case id_float64:
    return "float64";
  case id_complex64:
    return "complex64";
  case id_complex128:
    return "complex128";
    // case id_ascii
    // case id_ucs4
  }
  assert(0);
}

template <typename T> string yaml_encode(const complex<T> &val) {
  YAML::Emitter re;
  re << val.real();
  YAML::Emitter im;
  im << val.imag();
  ostringstream buf;
  buf << re.c_str();
  if (im.c_str()[0] != '-')
    buf << "+";
  buf << im.c_str() << "i";
  return buf.str();
}

YAML::Node emit_scalar(const void *data, scalar_type_id_t scalar_type_id) {
  YAML::Node node;
  switch (scalar_type_id) {
  case id_bool8:
    node = *reinterpret_cast<const bool8_t *>(data);
    break;
  case id_int8:
    node = *reinterpret_cast<const int8_t *>(data);
    break;
  case id_int16:
    node = *reinterpret_cast<const int16_t *>(data);
    break;
  case id_int32:
    node = *reinterpret_cast<const int32_t *>(data);
    break;
  case id_int64:
    node = *reinterpret_cast<const int64_t *>(data);
    break;
  case id_uint8:
    node = *reinterpret_cast<const uint8_t *>(data);
    break;
  case id_uint16:
    node = *reinterpret_cast<const uint16_t *>(data);
    break;
  case id_uint32:
    node = *reinterpret_cast<const uint32_t *>(data);
    break;
  case id_uint64:
    node = *reinterpret_cast<const uint64_t *>(data);
    break;
  case id_float32:
    node = *reinterpret_cast<const float32_t *>(data);
    break;
  case id_float64:
    node = *reinterpret_cast<const float64_t *>(data);
    break;
  case id_complex64: {
    node.SetTag("core/complex-1.0.0");
    node = yaml_encode(*reinterpret_cast<const complex64_t *>(data));
    break;
  }
  case id_complex128: {
    node.SetTag("core/complex-1.0.0");
    node = yaml_encode(*reinterpret_cast<const complex128_t *>(data));
    break;
  }
  // case id_ascii
  // case id_ucs4
  default:
    assert(0);
  }
  return node;
}

////////////////////////////////////////////////////////////////////////////////

enum class byteorder_t { big, little };

string yaml_encode(byteorder_t byteorder) {
  switch (byteorder) {
  case byteorder_t::big:
    return "big";
  case byteorder_t::little:
    return "little";
  }
  assert(0);
};

byteorder_t host_byteorder() {
  const uint64_t magic{0x0102030405060708};
  const array<unsigned char, 8> magic_big{0x01, 0x02, 0x03, 0x04,
                                          0x05, 0x06, 0x07, 0x08};
  const array<unsigned char, 8> magic_little{0x08, 0x07, 0x06, 0x05,
                                             0x04, 0x03, 0x02, 0x01};
  if (memcmp(&magic, &magic_big, 8) == 0)
    return byteorder_t::big;
  if (memcmp(&magic, &magic_little, 8) == 0)
    return byteorder_t::little;
  assert(0);
}

YAML::Node emit_inline_array(const unsigned char *data,
                             scalar_type_id_t scalar_type_id,
                             const vector<int64_t> &shape,
                             const vector<int64_t> &strides) {
  size_t rank = shape.size();
  assert(strides.size() == rank);
  if (rank == 0) {
    // 0-dimensional array
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    // node[0] = data.at(offset);
    node[0] = emit_scalar(data, scalar_type_id);
    return node;
  }
  if (rank == 1) {
    // 1-dimensional array
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);
    for (size_t i = 0; i < shape.at(0); ++i)
      node[i] = emit_scalar(data + i * strides.at(0), scalar_type_id);
    return node;
  }
  // multi-dimensional array
  YAML::Node node;
  // TODO: Try emitting these as Flow, with a Newline at the end
  vector<int64_t> shape1(rank - 1);
  for (size_t d = 0; d < rank - 1; ++d)
    shape1.at(d) = shape.at(d + 1);
  vector<int64_t> strides1(rank - 1);
  for (size_t d = 0; d < rank - 1; ++d)
    strides1.at(d) = strides.at(d + 1);
  for (size_t i = 0; i < shape.at(0); ++i)
    node[i] = emit_inline_array(data + i * strides.at(0), scalar_type_id,
                                shape1, strides1);
  return node;
}

template <typename T>
void output(vector<unsigned char> &header, const T &data) {
  // Always output in big-endian as required for the header
  for (ptrdiff_t i = sizeof(T) - 1; i >= 0; --i)
    header.push_back(reinterpret_cast<const unsigned char *>(&data)[i]);
}

// TODO: stream the block (e.g. when compressing), then write the
// correct header later
void ndarray::write_block(ostream &os) const {
  vector<unsigned char> header;
  // block_magic_token
  // (Incidentally, this spells "SBLK", with the highest bit of the
  // "S" set to one)
  const array<unsigned char, 4> block_magic_token{0xd3, 0x42, 0x4c, 0x4b};
  for (auto ch : block_magic_token)
    output(header, ch);
  // header_size (not yet known)
  auto header_size_pos = header.size();
  uint16_t unknown_header_size = 0;
  output(header, unknown_header_size);
  auto header_prefix_length = header.size();
  // flags
  uint32_t flags = 0;
  output(header, flags);
  // compression
  array<unsigned char, 4> comp;
  shared_ptr<generic_blob_t> outdata;
  switch (compression) {
  case compression_t::none:
    comp = {0, 0, 0, 0};
    outdata = data;
    break;
  case compression_t::bzip2: {
#ifdef HAVE_BZIP2
    comp = {'b', 'z', 'p', '2'};
    // Allocate 600 bytes plus 1% more
    outdata = make_shared<blob_t<unsigned char>>(vector<unsigned char>(
        600 + data->bytes() + (data->bytes() + 99) / 100));
    const int level = 9;
    bz_stream strm;
    strm.bzalloc = NULL;
    strm.bzfree = NULL;
    strm.opaque = NULL;
    BZ2_bzCompressInit(&strm, level, 0, 0);
    strm.next_in = reinterpret_cast<char *>(const_cast<void *>(data->ptr()));
    strm.next_out = reinterpret_cast<char *>(outdata->ptr());
    uint64_t avail_in = data->bytes();
    uint64_t avail_out = outdata->bytes();
    for (;;) {
      uint64_t this_avail_in =
          min(uint64_t(numeric_limits<unsigned int>::max()), avail_in);
      uint64_t this_avail_out =
          min(uint64_t(numeric_limits<unsigned int>::max()), avail_out);
      strm.avail_in = this_avail_in;
      strm.avail_out = this_avail_out;
      auto state = this_avail_in < avail_in ? BZ_RUN : BZ_FINISH;
      int iret = BZ2_bzCompress(&strm, state);
      avail_in -= this_avail_in - strm.avail_in;
      avail_out -= this_avail_out - strm.avail_out;
      if (iret == BZ_STREAM_END)
        break;
      assert(iret == BZ_RUN_OK);
    }
    assert(avail_in == 0);
    outdata->resize(outdata->bytes() - avail_out);
    if (outdata->bytes() >= data->bytes()) {
      // Skip compression if it does not reduce the size
      comp = {0, 0, 0, 0};
      outdata = data;
    }
#else
    // Fall back to no compression if bzip2 is not available
    comp = {0, 0, 0, 0};
    outdata = data;
#endif
    break;
  }
  case compression_t::zlib: {
#ifdef HAVE_ZLIB
    comp = {'z', 'l', 'i', 'b'};
    // Allocate 6 bytes plus 5 bytes per 16 kByte more
    outdata = make_shared<blob_t<unsigned char>>(vector<unsigned char>(
        (6 + data->bytes() + (data->bytes() + 16383) / 16384 * 5)));
    const int level = 9;
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    int iret = deflateInit(&strm, level);
    assert(iret == Z_OK);
    strm.next_in =
        reinterpret_cast<unsigned char *>(const_cast<void *>(data->ptr()));
    strm.next_out = reinterpret_cast<unsigned char *>(outdata->ptr());
    uint64_t avail_in = data->bytes();
    uint64_t avail_out = outdata->bytes();
    for (;;) {
      uint64_t this_avail_in =
          min(uint64_t(numeric_limits<uInt>::max()), avail_in);
      uint64_t this_avail_out =
          min(uint64_t(numeric_limits<uInt>::max()), avail_out);
      strm.avail_in = this_avail_in;
      strm.avail_out = this_avail_out;
      auto state = this_avail_in < avail_in ? Z_NO_FLUSH : Z_FINISH;
      int iret = deflate(&strm, state);
      avail_in -= this_avail_in - strm.avail_in;
      avail_out -= this_avail_out - strm.avail_out;
      if (iret == Z_STREAM_END)
        break;
      assert(iret == Z_OK);
    }
    assert(avail_in == 0);
    outdata->resize(outdata->bytes() - avail_out);
#if 0
    assert(data->bytes() < numeric_limits<uInt>::max());
    strm.avail_in = data->bytes();
    strm.next_in =
        reinterpret_cast<unsigned char *>(const_cast<void *>(data->ptr()));
    assert(outdata->bytes() < numeric_limits<uInt>::max());
    strm.avail_out = outdata->bytes();
    strm.next_out = reinterpret_cast<unsigned char *>(outdata->ptr());
    iret = deflate(&strm, Z_FINISH);
    assert(iret == Z_STREAM_END);
    assert(strm.total_in == data->bytes());
    outdata->resize(strm.total_out);
    deflateEnd(&strm);
#endif
    if (outdata->bytes() >= data->bytes()) {
      // Skip compression if it does not reduce the size
      comp = {0, 0, 0, 0};
      outdata = data;
    }
#else
    // Fall back to no compression if zlib is not available
    comp = {0, 0, 0, 0};
    outdata = data;
#endif
    break;
  }
  default:
    assert(0);
  }
  for (auto ch : comp)
    output(header, ch);
  // allocated_space
  uint64_t allocated_space = outdata->bytes();
  output(header, allocated_space);
  // used_space
  uint64_t used_space = allocated_space; // no padding
  output(header, used_space);
  // data_space
  uint64_t data_space = data->bytes();
  output(header, data_space);
  // checksum
  array<unsigned char, 16> checksum;
#if HAVE_OPENSSL
  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx, outdata->ptr(), outdata->bytes());
  MD5_Final(checksum.data(), &ctx);
#else
  checksum = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
  for (auto ch : checksum)
    output(header, ch);
  // fill in header_size
  uint16_t header_size = header.size() - header_prefix_length;
  vector<unsigned char> header_size_buf;
  output(header_size_buf, header_size);
  for (size_t p = 0; p < header_size_buf.size(); ++p)
    header.at(header_size_pos + p) = header_size_buf.at(p);
  // write header
  os.write(reinterpret_cast<const char *>(header.data()), header.size());
  // write data
  os.write(reinterpret_cast<const char *>(outdata->ptr()), outdata->bytes());
  // write padding
  vector<char> padding(allocated_space - used_space);
  os.write(padding.data(), padding.size());
}

YAML::Node ndarray::to_yaml(writer_state &ws) const {
  YAML::Node node;
  node.SetTag("core/ndarray-1.0.0");
  if (block_format == block_format_t::block) {
    // source
    const auto &self = *this;
    uint64_t idx = ws.add_task([=](ostream &os) { self.write_block(os); });
    node["source"] = idx;
  } else {
    // data
    node["data"] = emit_inline_array(
        static_cast<const unsigned char *>(data->ptr()) + offset,
        scalar_type_id, shape, strides);
  }
  // mask
  assert(mask.empty());
  // datatype
  node["datatype"] = yaml_encode(scalar_type_id);
  if (block_format == block_format_t::block) {
    // byteorder
    node["byteorder"] = yaml_encode(host_byteorder());
  }
  // shape
  node["shape"] = shape;
  node["shape"].SetStyle(YAML::EmitterStyle::Flow);
  if (block_format == block_format_t::block) {
    // offset
    node["offset"] = offset;
    // strides
    node["strides"] = strides;
    node["strides"].SetStyle(YAML::EmitterStyle::Flow);
  }
  return node;
}

////////////////////////////////////////////////////////////////////////////////

YAML::Node column::to_yaml(writer_state &ws) const {
  YAML::Node node;
  node.SetTag("core/column-1.0.0");
  node["name"] = name;
  node["data"] = data->to_yaml(ws);
  if (!description.empty())
    node["description"] = description;
  return node;
}

YAML::Node table::to_yaml(writer_state &ws) const {
  YAML::Node cols;
  for (size_t i = 0; i < columns.size(); ++i)
    cols[i] = columns[i]->to_yaml(ws);
  YAML::Node node;
  node.SetTag("core/table-1.0.0");
  node["columns"] = cols;
  return node;
}

////////////////////////////////////////////////////////////////////////////////

YAML::Node software(const string &name, const string &author,
                    const string &homepage, const string &version) {
  YAML::Node node;
  node.SetTag("core/software-1.0.0");
  assert(!name.empty());
  node[name] = name;
  if (!author.empty())
    node["author"] = author;
  if (!homepage.empty())
    node["homepage"] = homepage;
  assert(!version.empty());
  node["version"] = version;
  return node;
}

YAML::Node asdf::to_yaml(writer_state &ws) const {
  const auto &asdf_library =
      software("asdf-cxx", "Erik Schnetter",
               "https://github.com/eschnett/asdf-cxx", ASDF_VERSION);
  YAML::Node tabs;
  for (size_t i = 0; i < tables.size(); ++i)
    tabs[i] = tables[i]->to_yaml(ws);
  YAML::Node node;
  // node.SetStyle(YAML::EmitterStyle::BeginDoc);
  // node.SetStyle(YAML::EmitterStyle::EndDoc);
  node.SetTag("core/asdf-1.0.0");
  node["asdf_library"] = asdf_library;
  node["tables"] = tabs;
  return node;
}

void asdf::write(ostream &os) const {
  writer_state ws;
  // TODO: Use YAML::Emitter(os) instead
  const auto &node = to_yaml(ws);
  os << "#ASDF " << asdf_format_version << "\n"
     << "#ASDF_STANDARD " << asdf_standard_version << "\n"
     << "# This is an ASDF file <https://asdf-standard.readthedocs.io/>.\n"
     << "%YAML 1.1\n"
     << "%TAG ! tag:stsci.edu:asdf/\n"
     << "---\n"
     << node << "\n"
     << "...\n";
  ws.flush(os);
}

} // namespace ASDF
