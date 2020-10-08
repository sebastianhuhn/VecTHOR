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

#ifndef DYNDECOMPRESSOR_H
#define DYNDECOMPRESSOR_H

// Project includes
#include <Decompressor.h>
#include <Stats.h>
#include <TypeDefs.h>

// System includes
#include <string>

// Boost includes

namespace vecthor {

class DynDecompressor : public Decompressor {

public:
  DynDecompressor(const Config *cfg_ptr);
  void clear() override;
  void determineCDW(BitVecCItr &bv_begin, BitVecCItr &bv_end) override;

private:
  void assumeCDW(std::string &cdw_repl);
  void preloadConfiguration();
  void extractData();
  void plot(std::string plot_name = "default_plot");
  inline void sortFrequencyData() {
    sortFrequencyData(m_freq_data.begin(), m_freq_data.end());
  }
  inline void sortFrequencyData(FrequencyData::iterator begin_it,
                                FrequencyData::iterator end_it);

  unsigned int removeExternals(const std::string &vec_to_str,
                               const FrequencyData::value_type &elem,
                               boost::dynamic_bitset<> &covered);
  void removeExternalsIntersects(const std::string &vec_to_str);
  void removeInternalIntersects(const std::string &vec_to_str);
  unsigned int removeInternals(const std::string &vec_to_str,
                               const FrequencyData::value_type &elem);
  void walk(BitVecCItr bv_start, BitVecCItr bv_end);
  void dumpCoverMap(boost::dynamic_bitset<> &covered) const;
  void dumpFreqContainer() const;

  inline bool hasNextInner(unsigned int i) const;
  inline bool hasNextOuter(unsigned int i) const;
  inline bool checkAll(boost::dynamic_bitset<> &covered, std::size_t lb,
                       std::size_t ub);
  inline void setAll(boost::dynamic_bitset<> &covered, std::size_t lb,
                     std::size_t ub);

  inline Stats &getStats() override { return m_stats; }

private:
  unsigned int m_num_det_cdw, m_lb_freq_inner, m_lb_freq_outer,
      m_byte_weight; // 0: ID, 1: doubled, 2: trippled
  FrequencyContainer m_freq_container;
  FrequencyData m_freq_data;
  std::vector<std::string> m_cdw_container;
  DecompressorStats m_stats;
};

} // namespace vecthor
#endif
