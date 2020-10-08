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

#pragma once

#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H

// Project includes
#include <Config.h>
#include <TypeDefs.h>

// System includes
#include <cstring>
#include <string>

// Boot includes
#include <boost/dynamic_bitset.hpp>

namespace vecthor {

class Stats;
class Decompressor {
public:
  using CDWStringMap = std::map<CDW, std::string>;
  using UDWStringMap = std::map<std::string, CDW>;
  using CDWBenefitMap = std::map<CDW, short>;

  Decompressor(const Config *cfg_ptr) : m_cfg_ptr(cfg_ptr) { preloadCDW(); }

  const std::vector<CDW> &getTBRs() const;
  const std::vector<std::string> &getTBCs() const;
  size_t numTBCs() const;
  size_t lengthTBCs() const;
  CDW getCDW(const boost::dynamic_bitset<> &bit_str) const;
  CDW getCDW(const std::string &bit_str) const;
  std::string CDWtoEncoding(CDW cdw) const;
  std::string CDWtoString(CDW cdw) const;

  short getCDWBenefit(CDW cdw);
  //    std::string CDWtoDecoding( CDW cdw ) const;

  static bool isUDWLength(BitVecCItr &l_start, BitVecCItr &l_end,
                          unsigned int lb = 0);
  static bool isEmptyCDW(CDW result);
  static bool isValidCDW(CDW result);
  static bool isStaticCDW(CDW result, bool ext_cdws);
  u_int8_t getCDWLength(CDW &cdw) const;

  virtual void determineCDW(BitVecCItr &, BitVecCItr &) {}
  virtual void clear() {}

  virtual Stats &getStats() {}
  void reset();
  void dumpEntries() const;
  void dumpConfiguration() const;

  const UDWStringMap extractUDW() const;

protected:
  bool storeDynCDW(std::string &cdw_repl);
  void preloadCDW();
  bool storeDynCDW(boost::dynamic_bitset<> cdw_repl);

protected:
  const Config *m_cfg_ptr;

private:
  //    struct UDWCompare : public std::binary_function< const char*, const
  //    char*, bool > {
  //        public:
  //        bool operator()( const char* str1, const char* str2 ) const
  //        {
  //            return std::strcmp( str1, str2 ) < 0;
  //        }
  //    };
  CDWStringMap m_cdw_map;
  UDWStringMap m_udw_map;
  // CDWStringMap m_cdw_str;
  CDWBenefitMap m_cdw_benefit;
  std::map<const CDW, u_int8_t> m_cdw_weight;

  std::vector<CDW> m_tbrs;
  std::vector<std::string> m_tbcs;
};
} // namespace vecthor
#endif // DECOMPRESSOR_H
