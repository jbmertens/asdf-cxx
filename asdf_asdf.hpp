#ifndef ASDF_ASDF_HPP
#define ASDF_ASDF_HPP

#include "asdf_config.hpp"
#include "asdf_group.hpp"
#include "asdf_ndarray.hpp"

#include <yaml-cpp/yaml.h>

#include <map>
#include <memory>

namespace ASDF {
using namespace std;

// ASDF

class asdf {
  map<string, shared_ptr<ndarray>> data;
  // fits
  // wcs
  // shared_ptr<table> tab;
  shared_ptr<group> grp;
  map<string, YAML::Node> nodes;
  // function<map<string, YAML::Node>(writer_state &ws)> writer;

public:
  asdf() = delete;
  asdf(const asdf &) = default;
  asdf(asdf &&) = default;
  asdf &operator=(const asdf &) = default;
  asdf &operator=(asdf &&) = default;

  asdf(const map<string, shared_ptr<ndarray>> &data) : data(data) {}
  // asdf(const shared_ptr<table> &tab) : tab(tab) { assert(tab); }
  asdf(const shared_ptr<group> &grp) : grp(grp) { assert(grp); }
  asdf(const map<string, YAML::Node> &nodes) : nodes(nodes) {}
  // asdf(const function<map<string, YAML::Node>(writer_state &ws)> &writer)
  //     : writer(writer) {}

  asdf(const reader_state &rs, const YAML::Node &node);
  asdf(const copy_state &cs, const asdf &project);
  writer_state &to_yaml(writer_state &ws) const;
  friend writer_state &operator<<(writer_state &ws, const asdf &proj) {
    return proj.to_yaml(ws);
  }

  asdf(istream &is);
  asdf copy(const copy_state &cs) const;
  void write(ostream &os) const;
};

} // namespace ASDF

#define ASDF_ASDF_HPP_DONE
#endif // #ifndef ASDF_ASDF_HPP
#ifndef ASDF_ASDF_HPP_DONE
#error "Cyclic include depencency"
#endif
