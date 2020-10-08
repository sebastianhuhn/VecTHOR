/*
Copyright (c) 2015-2020 Sebastian Huhn < huhn@informatik.uni-bremen.de >

This file is part of VecTHOR < https://github.com/sebastianhuhn/VecTHOR >

VecTHOR is under MIT License:
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <TDRReader.h>

// Project includes
#include <TDRGen.h>
#include <Utils.h>

// System includes
#include <bitset>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <regex>
#include <sstream>

namespace vecthor {

using namespace std;
using namespace boost::filesystem;

const BitVec TDRReader::readTDR(const string &ext_file_name) {
  BitVec bit_vec;
  auto ext_file_path = path(ext_file_name);
  assert(exists(ext_file_path));

  std::ifstream ext_file_str;
  ext_file_str.open(ext_file_path.string());
  unsigned int i = 1u;
  unsigned int ctr = 0;
  for (std::string line_str; getline(ext_file_str, line_str); ++i) {
    for (const char &i_char : line_str) {
      if (!(i_char == '0' || i_char == '1' || i_char == 'X')) {
        if (i_char != ' ') {
          std::cout
              << boost::format(
                     "[w] Unsupported character '%c' in line %d got skipped.") %
                     i_char % i
              << std::endl;
        }
        continue;
      }
      if (i_char == 'X') {
        bit_vec.push_back('2');
        ++ctr;
      } else {
        bit_vec.push_back(i_char == '1');
      }
    }
  }
  if (ext_file_str.is_open()) {
    ext_file_str.close();
  }
  return bit_vec;
}

const BitVec TDRReader::readHexTDR(const string &ext_file_name) {
  BitVec bit_vec;
  auto ext_file_path = path(ext_file_name);
  assert(exists(ext_file_path));

  std::ifstream ext_file_str;
  ext_file_str.open(ext_file_path.string());
  unsigned int i = 1u;
  unsigned int ctr = 0;
  stringstream ss;
  std::regex e("([0-9]|[a-f])*");
  for (std::string line_str; getline(ext_file_str, line_str); ++i) {
    if (false && !regex_match(line_str.begin(), line_str.end(), e)) {
      std::cout << boost::format(
                       "[w] Unsupported character in line %s got skipped.") %
                       line_str
                << std::endl;
      continue;
    }
    assert(line_str.size() == 9);
    ss << hex << line_str;
    unsigned hex_num = 0u;
    ss >> hex_num;
    bitset<32> hex_bitset(hex_num);
    for (const char bit : hex_bitset.to_string()) {
      bit_vec.push_back(bit == '1');
    }
  }
  if (ext_file_str.is_open()) {
    ext_file_str.close();
  }
  if (true) {
    auto out_file_path = path("tdr_data");
    std::ofstream out_file;
    out_file.open(out_file_path.string());
    for (u_int8_t bit : bit_vec) {
      out_file << (int)bit;
    }
    out_file << endl;
    if (out_file.is_open()) {
      out_file.close();
    }
  }
  return bit_vec;
}
} // namespace vecthor
