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

#include <DynDecompressor.h>

// Project includes
#include <Plotter.h>

// System includes
#include <iostream>
#include <map>
#include <memory>
#include <tuple>
#include <type_traits>

// Boost includes
#include <boost/format.hpp>

namespace vecthor {

using namespace std;

DynDecompressor::DynDecompressor(const Config *cfg_ptr)
    : Decompressor(cfg_ptr), m_num_det_cdw(0), m_lb_freq_inner(0),
      m_lb_freq_outer(0), m_byte_weight(0), m_freq_container(), m_freq_data(),
      m_cdw_container(), m_stats(m_cfg_ptr) {
  preloadConfiguration();
}

void DynDecompressor::clear() {
  m_freq_container.clear();
  m_freq_data.clear();
  m_cdw_container.clear();
}

void DynDecompressor::determineCDW(BitVecCItr &bv_begin, BitVecCItr &bv_end) {
  // Calc permutation
  for (auto bv_itr = bv_begin; bv_itr != bv_end; ++bv_itr) {
    if (bv_itr + 8 > bv_end) {
      walk(bv_itr, bv_end);
    } else {
      walk(bv_itr, bv_itr + 8);
    }
  }

  extractData(); // Get data from permutation map
  sortFrequencyData();
  string vec_to_str = serializeBitVec(bv_begin, bv_end);

  removeInternalIntersects(vec_to_str); // Remove "internal intersections"
  sortFrequencyData();
  if (m_cfg_ptr->getProperty(CFG::PLOT) || true) {
    plot("plot_stage1");
  }
  removeExternalsIntersects(vec_to_str); // Remove "external intersections"
  if (m_cfg_ptr->getProperty(CFG::PLOT) || true) {
    plot("plot_stage2");
  }
  //   storeDynCDW(m_cdw_container);
}

void DynDecompressor::plot(string plot_name) {
  using PC = Plotter::CFGATTR;
  Plotter plotter;
  Plotter::ConfigMap cfg;
  plotter.initTypeConfig<Plotter::HistoPlot>();
  cfg[PC::TITLE] = m_cfg_ptr->getFile(FILE::EXT_FILE);
  cfg[PC::DATAFILENAME] = plot_name + "_data.txt";
  cfg[PC::NAME] = plot_name + ".gpl";
  cfg[PC::OUTPUT] = plot_name + ".png";
  cfg[PC::PLOT] = plot_name + ".txt";
  cfg[PC::XLABEL] = "test";
  cfg[PC::XTICS] = "rotate by -45 scale 0";
  plotter.writeConfig(cfg);
  vector<long double> fst;
  vector<string> snd;
  for (auto const &elem : m_freq_data) {
    fst.push_back(elem.first);
    snd.push_back(elem.second);
  }
  plotter.writeData(fst, snd);
}

void DynDecompressor::extractData() {
  for (auto itr = m_freq_container.begin(); itr != m_freq_container.end();
       ++itr) {
    if (itr->second > 0) {
      auto weight = (itr->first.length() == 8) * m_byte_weight;
      m_freq_data.emplace_back((itr->second * (1 + weight)), itr->first);
    }
  }
}

inline bool DynDecompressor::hasNextInner(unsigned int i) const {
  return (m_lb_freq_inner > 0) && (i < m_freq_data.size()) &&
         ((m_num_det_cdw < m_cfg_ptr->getProperty(CFG::MAX_CDWS)) ||
          (m_freq_data[i].first >= m_lb_freq_inner));
}

inline bool DynDecompressor::hasNextOuter(unsigned int i) const {
  return (m_lb_freq_outer > 0) && (i < m_freq_data.size()) &&
         ((m_num_det_cdw < m_cfg_ptr->getProperty(CFG::MAX_CDWS)) ||
          (m_freq_data[i].first >= m_lb_freq_outer));
}

inline bool DynDecompressor::checkAll(boost::dynamic_bitset<> &covered,
                                      size_t lb, size_t ub) {
  for (unsigned int i = lb; i < ub; ++i) {
    if (covered.test(i)) {
      return true;
    }
  }
  return false;
}

inline void DynDecompressor::setAll(boost::dynamic_bitset<> &covered, size_t lb,
                                    size_t ub) {
  for (unsigned int i = lb; i < ub; ++i) {
    covered.set(i);
  }
}

inline void DynDecompressor::sortFrequencyData(FrequencyData::iterator begin_it,
                                               FrequencyData::iterator end_it) {
  using FreqEntry = FrequencyData::value_type;
  sort(begin_it, end_it, [](const FreqEntry &a, const FreqEntry &b) {
    return (a.first > b.first);
  });
}

void DynDecompressor::removeExternalsIntersects(const string &vec_to_str) {
  //  unsigned int number = 0;
  boost::dynamic_bitset<> covered(vec_to_str.length());
  // Assume first one
  auto &fst_elem = m_freq_data[0];
  removeExternals(vec_to_str, fst_elem, covered);
  assumeCDW(fst_elem.second);
  //
  auto num_max_cdw = m_cfg_ptr->getProperty(CFG::MAX_CDWS);
  auto start_idx = m_num_det_cdw;
  while (m_num_det_cdw < num_max_cdw) {
    // Calculate next candidate
    for (unsigned int i = start_idx; hasNextOuter(i); ++i) {
      auto cloned_covered = covered;
      auto &elem = m_freq_data[i];
      // auto num =
      elem.first = removeExternals(vec_to_str, elem, cloned_covered);
    }
    sortFrequencyData(++m_freq_data.begin(), m_freq_data.end());
    // ... ends.
    if (start_idx >= m_freq_data.size()) {
      break;
    }
    auto &elem = m_freq_data[start_idx];
    elem.first = removeExternals(vec_to_str, elem, covered);
    assumeCDW(elem.second);
    ++start_idx;
  }
  m_freq_data.resize(num_max_cdw * 2);
  if (m_cfg_ptr->getProperty(CFG::DEBUG)) {
    dumpCoverMap(covered);
    dumpConfiguration();
  }
  if (m_cfg_ptr->getProperty(CFG::VERBOSE)) {
    cout << boost::format(
                "[i] Number of uncovered bit after DynDecompress: %i\t") %
                (covered.size() - covered.count())
         << endl;
  }
  if (m_cfg_ptr->getProperty(CFG::STATS)) {
    m_stats.printStats("Dyn");
  }
}

unsigned int
DynDecompressor::removeExternals(const std::string &vec_to_str,
                                 const FrequencyData::value_type &elem,
                                 boost::dynamic_bitset<> &covered) {
  unsigned int number = 0;
  size_t idx = 0;
  for (size_t ret = 0; ret != string::npos;) {
    ret = vec_to_str.find(elem.second, idx);
    idx = ret + elem.second.length();
    if (!checkAll(covered, ret, idx)) {
      ++number;
      setAll(covered, ret, idx);
    }
  }
  auto weight = (elem.second.length() == 8) * m_byte_weight;
  if (m_cfg_ptr->isDebug()) {
    cout << boost::format("[d] New frequency for '%s': %u -> %u") %
                elem.second % elem.first % number
         << endl;
  }
  return number * (1 + weight);
}

void DynDecompressor::removeInternalIntersects(const string &vec_to_str) {
  unsigned int max = 0;
  for (unsigned int i = 0; hasNextInner(i); ++i) {
    auto &elem = m_freq_data[i];
    auto num = removeInternals(vec_to_str, elem);
    if (m_cfg_ptr->isDebug()) {
      cout << boost::format("[d] New frequency for '%s': %u -> %u") %
                  elem.second % elem.first % num
           << endl;
    }
    elem.first = num;
    max = i;
  }
  m_freq_data.resize(max + 1);
}

unsigned int
DynDecompressor::removeInternals(const string &vec_to_str,
                                 const FrequencyData::value_type &elem) {
  unsigned int number = 0;
  size_t idx = 0;
  for (size_t ret = 0; ret != string::npos;) {
    ret = vec_to_str.find(elem.second, idx);
    idx = ret + elem.second.length();
    ++number;
  }

  auto weight = (elem.second.length() == 8) * m_byte_weight;
  return number * (1 + weight);
}

void DynDecompressor::walk(BitVecCItr bv_start, BitVecCItr bv_end) {
  if (bv_start == bv_end) {
    return;
  }
  auto nr_elems = distance(bv_start, bv_end);
  assert(nr_elems >= 1 && nr_elems <= 8);
  for (; bv_start != bv_end; ++bv_start) {
    if (isUDWLength(bv_start, bv_end, 4)) {
      string str(serializeBitVec(bv_start, bv_end));
      ++m_freq_container[str];
    }
    //        if( m_cfg_ptr->getProperty( CFG::HEUR_PERMUTE) > 0 ){
    walk(bv_start + 1, bv_end);
    walk(bv_start, bv_end - 1);
    //        }
  }
}

void DynDecompressor::assumeCDW(std::string &cdw_repl) {
  m_cdw_container.push_back(cdw_repl);
  if (m_cfg_ptr->getProperty(CFG::VERBOSE)) {
    std::cout << boost::format("[v] Assuming CDW for '%s'.") % cdw_repl
              << std::endl;
  }
  if (storeDynCDW(cdw_repl)) {
    ++m_num_det_cdw;
    m_stats.m_config_bit += cdw_repl.length();
  }
}

void DynDecompressor::preloadConfiguration() {
  m_lb_freq_inner = m_cfg_ptr->getProperty(CFG::HEUR_INNER_FREQ);
  m_lb_freq_outer = m_cfg_ptr->getProperty(CFG::HEUR_OUTER_FREQ);
  m_byte_weight = m_cfg_ptr->getProperty(CFG::HEUR_WEIGHT);
}

void DynDecompressor::dumpCoverMap(boost::dynamic_bitset<> &covered) const {
  cout << "###########################" << endl;
  cout << "[DUMP] Cover map of CDWs:" << endl;
  cout << "[DUMP] Using CDWs: ";

  for (auto const &elem : m_cdw_container) {
    cout << elem << " ";
  }

  cout << endl;
  unsigned int ctr = 1;
  for (boost::dynamic_bitset<>::size_type i = 0; i < covered.size(); ++i) {
    cout << covered[i];
    if (ctr % 64 == 0) {
      cout << "\n";
    } else if (ctr % 8 == 0) {
      cout << " ";
    }
    ctr++;
  }
  cout << endl;
  cout << "[DUMP] ... ends!" << endl;
  cout << "###########################" << endl;
}

void DynDecompressor::dumpFreqContainer() const {
  cout << "###########################" << endl;
  cout << "[DUMP] Dumping possible combinations of CDWs:" << endl;
  for (auto const &elem : m_freq_data) {
    std::cout << boost::format("%i [%s]") % elem.first % elem.second
              << std::endl;
  }
  cout << "[DUMP] ... ends!" << endl;
  cout << "###########################" << endl;
}
} // namespace vecthor
