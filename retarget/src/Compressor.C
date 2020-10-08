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

#include <Compressor.h>

// Project includes
#include <Decompressor.h>
#include <TDRGen.h>
#include <Utils.h>

// System includes
#include <iostream>
#include <iterator>
#include <math.h>
//#include <>

// Boost includes
#include <boost/format.hpp>
#include <boost/timer.hpp>
#include <boost/utility/binary.hpp>

namespace vecthor {

using namespace std;

// [C]ompressed [D]ata [W]ord [P]ermutations
int Compressor::calculateCDWByte(BitVecCItr start, BitVecCItr end,
                                 CDWMap &cdw_map) {
  if ((start == end)) {
    return 0;
  } // Nothing to do
  auto nr_elems = distance(start, end);
  if (nr_elems < 1 || nr_elems > 8) {
    return -1;
  } // Error

  while (start != end) {
    ReplacementPtr repl = classifyCDW(start, end);
    if (repl) {
      cdw_map[start][end] = repl;
    }
    calculateCDWByte(start + 1, end, cdw_map);
    calculateCDWByte(start, end - 1, cdw_map);
    ++start;
  }
  return 1;
}

ReplacementPtr Compressor::classifyCDW(BitVecCItr start, BitVecCItr end) {
  auto dist = distance(start, end);
  // Must be single bit, nibble or complete byte
  if (!Decompressor::isUDWLength(start, end)) {
    return 0;
  }
  std::string bit_str = serializeBitVec(start, end);
  //  uint32_t offset = pow( 10, dist);
  //  CDW result = static_cast<CDW>( offset + atoi( bit_str ) );
  CDW result = m_decomp_ptr->getCDW(bit_str);
  if (result == CDW::NONE) {
    return nullptr;
  }
  return ReplacementPtr(new Replacement(make_tuple(
      result, start, end, dist - m_decomp_ptr->getCDWLength(result))));
}

Edges Compressor::determineStart(CDWMap &cdw_map) {
  // dist, cost, replacement
  if (isDebug()) {
    dumpCDWmap(cdw_map);
  }
  Edges data;
  for (auto const &i : cdw_map) {
    for (auto const &ii : i.second) {
      auto dist = std::distance(i.first, ii.first);
      assert(dist > 0);
      assert(ii.second);
      auto benefit = get<3>(*ii.second);
      if (benefit > 0) {
        data.emplace_back(dist, benefit, ii.second);
      }
    }
  }
  data.shrink_to_fit();
  // Sorting regarding distance & cost
  sort(data.begin(), data.end(), [&](const Edge &a, const Edge &b) {
    return (std::get<1>(a) > std::get<1>(b)) &&
           (std::get<0>(a) > std::get<0>(b));
  });
  if (!data.size()) {
    cerr << "[e] Could not determine start point!" << endl;
  }
  if (isVerbose()) {
    std::cout
        << boost::format(
               "[v] Determined #%i valueable start points for calculation.") %
               data.size()
        << std::endl;
  }
  return data;
}

void Compressor::determineRoute(Edges &init, Route &route) {
  int i = 0;
  for (auto const &elem : init) {
    ReplacementPtr repl = nullptr;
    tie(ignore, ignore, repl) = elem;
    addToCoveredRoute(route, repl);
    i++;
  }
  if (isVerbose()) {
    std::cout << boost::format(
                     "[v] Determined first sequence of #%i replacements.") %
                     route.size()
              << std::endl;
  }
}

void Compressor::addToCoveredRoute(Route &route, ReplacementPtr repl) {

  BitVecCItr l_start, l_end;
  assert(repl);
  tie(ignore, l_start, l_end, ignore) = *repl;
  auto lb = distance(m_bit_vec_begin, l_start);
  auto len = distance(l_start, l_end);

  auto cov_start_it = m_cover_ptr->begin();
  cov_start_it += lb;
  auto cov_end_it = cov_start_it + len;
  if (isCovered(cov_start_it, cov_end_it)) {
    return;
  }
  if (isDebug()) {
    cout << boost::format("[d] Using CDW for pos [%i:%i]!") % lb %
                (lb + len - 1)
         << endl;
  }
  setCovered(cov_start_it, cov_end_it);
  addToRoute(route, repl);
}

void Compressor::fillGap(Route &route, BitVecItr l_start, BitVecItr l_end,
                         const CDWMap &cdw_map) {
  auto start_pos = getCoveragePos(l_start);
  auto end_pos = getCoveragePos(l_end);

  BitVecCItr bv_l_start = m_bit_vec_begin;
  std::advance(bv_l_start, start_pos);

  BitVecCItr bv_l_end = m_bit_vec_begin;
  std::advance(bv_l_end, end_pos);

  if (bv_l_start > m_bit_vec_end || bv_l_end > m_bit_vec_end) {
    return;
  }

  if (isDebug()) {
    std::cout << boost::format("[d] Handling gap between [%i:%i]") % start_pos %
                     end_pos
              << std::endl;
  }
  // Handle single bit gap
  if (isSingleBit(bv_l_start, bv_l_end)) {
    m_compr_stats.m_num_sbf++;
  }
  // Using CDW to determine a suitable replacement
  auto repl = classifyCDW(bv_l_start, bv_l_end);
  auto oit = cdw_map.find(bv_l_start);
  if (oit != cdw_map.end()) {
    auto const &inner_map = oit->second;
    auto iit = inner_map.find(bv_l_end);
    if (iit != inner_map.end() && iit->second) {
      addToRoute(route, iit->second);
      setCovered(l_start, l_end);
      return;
    }
  }
  fillGap(route, l_start, l_start + 1, cdw_map);
  fillGap(route, l_start + 1, l_end, cdw_map);
}

float Compressor::determineCoverage() const {
  assert(m_cover_ptr->size() > 0);
  float nr_one = count_if(m_cover_ptr->cbegin(), m_cover_ptr->cend(),
                          [](const u_int8_t val) { return val == 1; });
  return (nr_one / m_cover_ptr->size());
}

int Compressor::getCoveragePos(BitVecItr &cov_pos_it) const {
  return distance(m_cover_ptr->begin(), cov_pos_it);
}

void Compressor::mergeRepl(ReplacementPtr fst, ReplacementPtr snd) {
  CDW snd_cdw;
  short snd_benefit;
  tie(snd_cdw, ignore, ignore, snd_benefit) = *snd;
  assert((get<0>(*fst) == snd_cdw) && (get<2>(*fst) == get<1>(*snd)));

  ++m_compr_stats.m_num_cdw_repetition;
  m_compr_stats.m_num_red_repetition += m_decomp_ptr->getCDWLength(snd_cdw);
  m_compr_stats.m_counter_cdws[snd_cdw] -= 1;
  m_compr_stats.m_counter_cdws[CDW::XXX] += 1;

  get<0>(*snd) = CDW::XXX;
}

void Compressor::mergeRoute(Route &route) {
  if (isVerbose()) {
    cout << "[v] Finalizing replacement sequence: Merging identical pairs..."
         << endl;
  }
  auto pred_itr = route.begin();
  for (auto route_itr = route.begin() + 1; route_itr != route.end();
       pred_itr = route_itr++) {
    auto pred_cdw = get<0>(**pred_itr);
    auto cdw_i = get<0>(**route_itr);
    if ((pred_cdw == cdw_i)) {
      mergeRepl(*pred_itr, *route_itr);
    }
  }
  dumpRoute(route);
  if (isVerbose()) {
    std::cout << boost::format(
                     "[i] Compacting route: #%i possible repetition-merges.") %
                     m_compr_stats.m_num_cdw_repetition
              << std::endl;
  }
}

void Compressor::sortRoute(Route &route) {
  sort(route.begin(), route.end(), [](ReplacementPtr a, ReplacementPtr b) {
    return (get<1>(*a) < get<1>(*b));
  });
}

void Compressor::greedy(Route &route) {
  // prepare( m_bit_vec_begin, m_bit_vec_end );
  auto end_itr = m_bit_vec_end;
  if (isVerbose()) {
    cout << "[v] Building CDW map with byte-wise permutations..." << endl;
  }
  CDWMap cdw_map;
  const auto permute = m_config_ptr->getProperty(CFG::HEUR_PERMUTE);
  for (auto i_itr = m_bit_vec_begin; i_itr < end_itr; i_itr += permute) {
    if (i_itr + 8 > end_itr) {
      calculateCDWByte(i_itr, end_itr, cdw_map);
    } else {
      calculateCDWByte(i_itr, i_itr + 8, cdw_map);
    }
  }
  Edges edges = determineStart(cdw_map);
  dumpStart(edges);

  determineRoute(edges, route);
  m_compr_stats.m_num_s1_repls = route.size();
  dumpRoute(route);
  if (isVerbose()) {
    std::cout << boost::format("[v] Stage 1 completed: Actual coverage %f.") %
                     determineCoverage()
              << std::endl;
  }

  if (!isCompleteRoute()) {
    postProcRoute(route, cdw_map);
    m_compr_stats.m_num_s2_repls =
        (route.size() - m_compr_stats.m_num_s1_repls);
  }

  if (isVerbose()) {
    std::cout << boost::format("[v] Stage 2 completed: Actual coverage %f.") %
                     determineCoverage()
              << std::endl;
  }
  finalizeRoute(route);
  if (m_config_ptr->getProperty(CFG::MERGING)) {
    if (isVerbose()) {
      std::cout << "[v] Stage 3: Merging route..." << endl;
    }
    mergeRoute(route);
  }
}

Route Compressor::formal(Route &repl_vec) {
  assert(!repl_vec.empty());
  Route route = {};
  // prepare( m_bit_vec_begin, m_bit_vec_end );
  for (auto const &elem : repl_vec) {
    //    std::cout << "route length: " << route.size() << " org " <<
    //    repl_vec.size() << std::endl;
    addToCoveredRoute(route, elem);
  }
  sortRoute(route);
  determineCoverage();

  assert(isCompleteRoute());
  if (!isCompleteRoute()) {
    assert(false);
    m_compr_stats.m_num_s2_repls =
        (route.size() - m_compr_stats.m_num_s1_repls);
  }

  if (true || isVerbose()) {
    std::cout << boost::format("[v] Stage 2 completed: Actual coverage %f.") %
                     determineCoverage()
              << std::endl;
  }
  finalizeRoute(route);
  if (m_config_ptr->getProperty(CFG::MERGING)) {
    if (isVerbose()) {
      std::cout << "[v] Stage 3: Merging route..." << endl;
    }
    mergeRoute(route);
  }
  return route;
}

void Compressor::addToRoute(Route &route, ReplacementPtr repl) {
  assert(repl);
  route.push_back(repl);
  m_compr_stats.m_num_benefit += get<3>(*repl);
  m_compr_stats.m_counter_cdws[get<0>(*repl)] += 1;
}

void Compressor::finalizeRoute(Route &route) {
  assert(isCompleteRoute());
  if (isVerbose()) {
    cout << "[v] Finalizing replacement sequence: Sequential sort..." << endl;
  }
  m_compr_stats.m_num_replacements = route.size();
  sortRoute(route);
  dumpRoute(route);
}

bool Compressor::isCovered(BitVecCItr cov_start_it,
                           BitVecCItr cov_end_it) const {
  return (any_of(cov_start_it, cov_end_it,
                 [](const u_int8_t val) { return 1 == val; }));
}

bool Compressor::isSingleBit(BitVecCItr &l_start, BitVecCItr &l_end) const {
  return distance(l_start, l_end) == 1;
}

void Compressor::postProcRoute(Route &route, const CDWMap &cdw_map) {
  if (isVerbose()) {
    std::cout << boost::format("[i] Post processing route: Filling gaps...")
              << std::endl;
  }
  auto start_it = m_cover_ptr->begin();
  auto end_it = m_cover_ptr->end();

  for (auto pos_it = start_it; pos_it < end_it; ++pos_it) {
    if (!*pos_it) {
      auto l_end_it = pos_it;
      for (; l_end_it != end_it && !*l_end_it; ++l_end_it)
        ;
      fillGap(route, pos_it, l_end_it, cdw_map);
      pos_it = l_end_it;
    }
  }
}

void Compressor::setCovered(BitVecItr &l_start, BitVecItr &l_end) {
  for_each(l_start, l_end, [](u_int8_t &val) { val = 1; });
}

// Debugging methods

void Compressor::dumpCDWmap(const CDWMap &cdw_map, bool force) const {
  if (!isDebug() && !force) {
    return;
  }
  cout << "[DUMP] Dumping CDW_map:" << endl;
  for (auto &key : cdw_map) {
    for (auto &ikey : key.second) {
      dumpReplacement(ikey.second);
    }
  }
  cout << "[DUMP] ... ends!" << endl;
}

void Compressor::dumpStart(const Edges &data, bool force) const {
  if (!isDebug() && !force) {
    return;
  }
  cout << "[DUMP] Dumping start point:" << endl;
  for (auto const &i : data) {
    short dist, cost;
    ReplacementPtr repl = nullptr;
    std::tie(dist, cost, repl) = i;
    std::cout << boost::format("[d%i b%i]\t") % dist % cost;
    assert(repl);
    dumpReplacement(repl);
  }
  cout << "[DUMP] ... ends!" << endl;
}

void Compressor::dumpRoute(const Route &route, bool force) const {
  if (!isDebug() && !force) {
    return;
  }
  cout << "[DUMP] Dumping Route:" << endl;
  for (auto const &i : route) {
    dumpReplacement(i, force);
  }
  cout << "[DUMP] ... ends!" << endl;
}

void Compressor::dumpReplacement(const ReplacementPtr repl, bool force) const {
  if (!isDebug() && !force) {
    return;
  }
  CDW cdw;
  BitVecCItr start, end;
  int red;
  std::tie(cdw, start, end, red) = *repl;
  cout << boost::format("%s \t\t [%i:%i]\t\t-> %s [%i]") %
              serializeBitVec(start, end) %
              std::distance(m_bit_vec_begin, start) %
              (std::distance(m_bit_vec_begin, end) - 1) %
              m_decomp_ptr->CDWtoString(cdw) % red
       << endl;
}
} // namespace vecthor
