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

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

// Project includes
#include <CompressorCore.h>
#include <Stats.h>

// System includes
#include <iostream>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

namespace vecthor {

// template< typename D >
class Compressor : public CompressorCore {

public:
  explicit Compressor(const Config *config, const Decompressor *decompr)
      : CompressorCore(config, decompr) {}

  void greedy(Route &route);
  Route formal(Route &repl_vec);

private:
  void addToRoute(Route &route, ReplacementPtr repl);
  int calculateCDWByte(BitVecCItr start, BitVecCItr end, CDWMap &cdw_map);
  ReplacementPtr classifyCDW(BitVecCItr start, BitVecCItr end);

  float determineCoverage() const;

  void dumpCDWmap(const CDWMap &cdw_map, bool force = false) const;
  void dumpStart(const Edges &data, bool force = false) const;
  void dumpRoute(const Route &route, bool force = false) const;
  void dumpReplacement(const ReplacementPtr repl, bool force = false) const;

  Edges determineStart(CDWMap &cdw_map);
  void determineRoute(Edges &init, Route &route);

  void fillGap(Route &route, BitVecItr l_start, BitVecItr l_end,
               const CDWMap &cdw_map);
  void finalizeRoute(Route &route);
  int getCoveragePos(BitVecItr &cov_pos_it) const;

  void mergeRepl(ReplacementPtr fst, ReplacementPtr snd);
  void mergeRoute(Route &route);
  bool isCovered(BitVecCItr cov_start_it, BitVecCItr cov_end_it) const;

  bool isSingleBit(BitVecCItr &l_start, BitVecCItr &l_end) const;
  void postProcRoute(Route &route, const CDWMap &cdw_map);

  void printCDWUsage() const;
  void setCovered(BitVecItr &l_start, BitVecItr &l_end);

  void sortRoute(Route &route);
  void addToCoveredRoute(Route &route, ReplacementPtr repl);
  // Templates

private:
};
} // namespace vecthor

#endif // COMPRESSOR_H
