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

#include "Validator.h"

#include <sstream>

// Boost Includes
#include <boost/filesystem.hpp>

namespace vecthor {

using namespace std;
namespace bf = boost::filesystem;

Validator::Validator(BitVec *bit_vec, Config *cfg, Decompressor *decompr)
    : m_bit_vec_golden(bit_vec), m_cfg(cfg), m_decompr(decompr),
      m_udw_map_vec(), m_bit_vec_chunk() {}

void Validator::storeReplace(const Decompressor::UDWStringMap &udws) {
  m_udw_map_vec.push_back(swapPairs(udws));
}

void Validator::storeChunk(const Route &route) {
  std::vector<CDW> cdws;
  for (auto entry : route) {
    CDW cdw;
    std::tie(cdw, std::ignore, std::ignore, std::ignore) = *entry;
    cdws.push_back(cdw);
  }
  m_bit_vec_chunk.push_back(cdws);
}

bool Validator::validate() {
  if (m_udw_map_vec.size() != m_bit_vec_chunk.size()) {
    return false;
  }
  auto valid_path = bf::path(m_cfg->getFile(FILE::VALIDATION_FILE));
  m_valid_file.open(valid_path.string(), ios_base::app);

  if (m_cfg->isVerbose()) {
    cout << "[i] Start generation of validation file " << valid_path.string()
         << endl;
  }

  std::stringstream bitvec_recalc, bitvec_golden;
  for (unsigned int i = 0; i < m_udw_map_vec.size(); ++i) {
    const auto &cdws = m_udw_map_vec.at(i);
    const auto &repls = m_bit_vec_chunk.at(i);
    for (unsigned j = 0; j < repls.size(); ++j) {
      const auto &repl = repls[j];
      if (repl == CDW::XXX) {
        assert(j > 0);
        auto k = j - 1;
        while (repls[k] == CDW::XXX) {
          assert(k >= 0);
          --k;
        }
        bitvec_recalc << cdws.at(repls[k]);
      } else {
        m_valid_file << m_decompr->CDWtoString(repl);
        if (i < m_udw_map_vec.size()) {
          m_valid_file << ".";
        }
        bitvec_recalc << cdws.at(repl);
      }
    }
  }
  if (m_valid_file.is_open()) {
    m_valid_file << endl;
    m_valid_file.close();
  }
  for (const auto &bit : *m_bit_vec_golden) {
    bitvec_golden << (int)bit;
  }
  if (bitvec_golden.str() == bitvec_recalc.str()) {
    return true;
  }
  return false;
}
} // namespace vecthor
