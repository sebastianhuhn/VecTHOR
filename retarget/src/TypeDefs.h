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

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

// System includes
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

// Definements
#define CYCLE_TIME 10
#define BUFFER_CTR_SIZE 12
#define USE_EXT_CDWS false

namespace vecthor {
enum class VALUE : uint8_t {
  LOW = 0,
  HIGH = 1,
  NOP = 2,
  INIT = 3,
  UNSUPPORTED = 4
};

enum class CDW {
  NONE,
  XXX, // -
  HXX, // 1
  LXX, // 0

  LLX, // 00
  LHX, // 01
  HLX, // 10
  HHX, // 11

  LLL, // 000
  LLH, // 001
  LHL, // 010
  LHH, // 011
  HLL, // 100
  HLH, // 101
  HHL, // 110
  HHH  // 111
#if true
  ,
  LLLL, // 0000
  LLLH, // 0001
  LLHL, // 0010
  LLHH, // 0011
  LHLL, // 0100
  LHLH, // 0101
  LHHL, // 0110
  LHHH, // 0111
  HLLL, // 1000
  HLLH, // 1001
  HLHL, // 1010
  HLHH, // 1011
  HHLL, // 1100
  HHLH, // 1101
  HHHL, // 1110
  HHHH, // 1111
#endif
};

enum class REPL_FIELD : uint8_t { CDW, START, END, WEIGHT };

using BitVec = std::vector<u_int8_t>;
using BitVecCPtr = std::shared_ptr<const BitVec>;
using BitVecItr = BitVec::iterator;
using BitVecCItr = BitVec::const_iterator;

// CDW, Start, End, Benefit
using Replacement = std::tuple<CDW, BitVecCItr, BitVecCItr, short>;
using ReplacementPtr = std::shared_ptr<Replacement>;

using CDWMap = std::map<BitVecCItr, std::map<BitVecCItr, ReplacementPtr>>;
using CDWMapItem = std::pair<BitVecCItr, std::map<BitVecCItr, ReplacementPtr>>;
// Dist, Cost, Replacement
using Edge = std::tuple<short, short, ReplacementPtr>;
using Edges = std::vector<Edge>;
// Route
using Route = std::vector<ReplacementPtr>;
// Decompressor
using FrequencyContainer = std::map<std::string, unsigned int>;
using FrequencyData = std::vector<std::pair<unsigned int, std::string>>;

// HW Emitter
using Signals = std::vector<std::tuple<VALUE, VALUE>>;

} // namespace vecthor
#endif // TYPEDEFS_H
