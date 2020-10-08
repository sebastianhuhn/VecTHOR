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

#include <CompressorCore.h>

// Project includes

// System includes
#include <algorithm>
#include <assert.h>
#include <string>

namespace vecthor {

using namespace std;

bool CompressorCore::isCompleteRoute() const {
  return (all_of(m_cover_ptr->cbegin(), m_cover_ptr->cend(),
                 [](const u_int8_t val) { return 1 == val; }));
}

void CompressorCore::prepare(BitVecCItr &bv_begin, BitVecCItr &bv_end) {
  reset();
  m_bit_vec_begin = bv_begin;
  m_bit_vec_end = bv_end;
  auto bv_size = distance(m_bit_vec_begin, m_bit_vec_end);
  assert(bv_size > 0);
  m_compr_stats.m_num_bit = bv_size;
  m_cover_ptr = new BitVec(bv_size, 0);
}

void CompressorCore::reset() {
  delete (m_cover_ptr);
  m_cover_ptr = nullptr;
  m_compr_stats.clear();
}
} // namespace vecthor
