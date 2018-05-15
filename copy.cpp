#include "asdf.hpp"

#include <yaml-cpp/yaml.h>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

int main(int argc, char **argv) {
  cout << "asdf-copy: Copy the content of an ASDF file\n";
  cout << "Syntax: " << argv[0] << " <input> <output>\n";
  if (argc != 3) {
    cerr << "Wrong number of arguments\n";
    exit(1);
  }
  string inputfilename = argv[1];
  string outputfilename = argv[2];
  assert(!inputfilename.empty());
  assert(!outputfilename.empty());
  // Read input
  ifstream is(inputfilename, ios::binary | ios::in);
  auto project = ASDF::asdf(is);
  is.close();
  // Write output
  ofstream os(outputfilename, ios::binary | ios::trunc | ios::out);
  project.write(os);
  os.close();
  //
  cout << "Done.\n";
  return 0;
}
