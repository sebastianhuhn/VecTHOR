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

#ifndef COMPRESSORCORE_H
#define COMPRESSORCORE_H

// Project includes
#include <Config.h>
#include <Decompressor.h>
#include <Stats.h>
#include <TypeDefs.h>

// System includes
#include <iostream>
#include <string>

namespace vecthor {

class CompressorCore {

public:
  explicit CompressorCore(const Config *config, const Decompressor *decomp)
      : m_config_ptr(config), m_decomp_ptr(decomp), m_compr_stats(config),
        m_cover_ptr(nullptr) {}

  inline void printStats(const std::string &title,
                         std::ostream &out = std::cout) const {
    m_compr_stats.printStats(title, out);
  }

  inline CompressorStats &getStats() { return m_compr_stats; }

  void prepare(BitVecCItr &bv_begin, BitVecCItr &bv_end);
  void reset();

protected:
  bool isCompleteRoute() const;
  bool isDebug() const { return m_config_ptr->isDebug(); }
  bool isVerbose() const { return m_config_ptr->isVerbose(); }

protected:
  BitVecCItr m_bit_vec_begin;
  BitVecCItr m_bit_vec_end;

  const Config *m_config_ptr;
  const Decompressor *m_decomp_ptr;
  CompressorStats m_compr_stats;
  BitVec *m_cover_ptr;

private:
};
} // namespace vecthor

#endif // COMPRESSCORE_H
