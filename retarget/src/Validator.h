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

#ifndef VALIDATOR_H
#define VALIDATOR_H

// System includes
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

// Project includes
#include <Config.h>
#include <Decompressor.h>
#include <TypeDefs.h>

namespace vecthor {

template <class T1, class T2> std::map<T2, T1> swapPairs(std::map<T1, T2> m) {
  std::map<T2, T1> m1;

  for (auto &&item : m) {
    m1.emplace(item.second, item.first);
  }

  return m1;
}

class Validator {
public:
  Validator(BitVec *bit_vec, Config *cfg, Decompressor *decompr);
  void storeReplace(const Decompressor::UDWStringMap &udws);
  void storeChunk(const Route &route);

  bool validate();

private:
private:
  BitVec *m_bit_vec_golden;
  Config *m_cfg;
  Decompressor *m_decompr;
  std::vector<Decompressor::CDWStringMap> m_udw_map_vec;
  std::vector<std::vector<CDW>> m_bit_vec_chunk;

  std::ofstream m_valid_file;
};
} // namespace vecthor
#endif // VALIDATOR_H
