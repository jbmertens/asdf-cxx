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
  map<string, string> tags; // tag directives

  map<string, shared_ptr<ndarray>> data;
  // fits
  // wcs
  // shared_ptr<table> tab;
  shared_ptr<group> grp;
  map<string, YAML::Node> nodes;
  map<string, function<void(writer &w)>> writers;

public:
  asdf() = delete;
  asdf(const asdf &) = default;
  asdf(asdf &&) = default;
  asdf &operator=(const asdf &) = default;
  asdf &operator=(asdf &&) = default;

  asdf(const map<string, string> &tags,
       const map<string, shared_ptr<ndarray>> &data)
      : tags(tags), data(data) {}
  // asdf(const map<string, string> &tags, const shared_ptr<table> &tab)
  //     : tags(tags), tab(tab) {
  //   assert(tab);
  // }
  asdf(const map<string, string> &tags, const shared_ptr<group> &grp)
      : tags(tags), grp(grp) {
    assert(grp);
  }
  asdf(const map<string, string> &tags, const map<string, YAML::Node> &nodes)
      : tags(tags), nodes(nodes) {}
  asdf(const map<string, string> &tags,
       const map<string, function<void(writer &w)>> &writers)
      : tags(tags), writers(writers) {}

  asdf(const reader_state &rs, const YAML::Node &node);
  asdf(const copy_state &cs, const asdf &project);
  writer &to_yaml(writer &w) const;
  friend writer &operator<<(writer &w, const asdf &proj) {
    return proj.to_yaml(w);
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
