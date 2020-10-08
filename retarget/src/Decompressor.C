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

#include <Decompressor.h>

// System includes
#include <sstream>
#include <type_traits>

// Boost includes
#include <boost/assign/std/map.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/reversed.hpp>

//#define ETS 1

namespace vecthor {

using namespace std;
namespace ba = boost::assign;

string Decompressor::CDWtoString(CDW cdw) const {
  if (m_cdw_map.find(cdw) == m_cdw_map.end()) {
    throw std::runtime_error(boost::str(
        boost::format("[e] Unknown CDW '%i' requested!") % ((int)cdw)));
  }
  return m_cdw_map.at(cdw);
}

short Decompressor::getCDWBenefit(CDW cdw) {
  if (m_cdw_benefit.find(cdw) == m_cdw_benefit.end()) {
    for (auto const &elem : m_udw_map) {
      if (elem.second == cdw) {
        size_t udw_length = elem.first.length();
        auto cdw_length = getCDWLength(cdw);
        short benefit = udw_length - cdw_length;
        m_cdw_benefit[cdw] = benefit;
        // std::cout << boost::format("UDW %i - CDW %i - Diff %i ")  %
        // udw_length % cdw_length % (udw_length-cdw_length) << std::endl;
        return benefit;
      }
    }
    return 0;
  }
  return m_cdw_benefit.at(cdw);
}

const std::vector<CDW> &Decompressor::getTBRs() const { return m_tbrs; }

const std::vector<std::string> &Decompressor::getTBCs() const { return m_tbcs; }

size_t Decompressor::numTBCs() const { return m_tbcs.size(); }

size_t Decompressor::lengthTBCs() const {
  size_t length = 0;
  for (auto const &tbc : m_tbcs) {
    length += tbc.size();
  }
  return length;
}

CDW Decompressor::getCDW(const boost::dynamic_bitset<> &bit_str) const {
  string str;
  to_string(bit_str, str);
  return getCDW(str);
}

CDW Decompressor::getCDW(const std::string &bit_str) const {
  if (m_udw_map.find(bit_str) == m_udw_map.end()) {
    return CDW::NONE;
  }
  return m_udw_map.at(bit_str);
}

string Decompressor::CDWtoEncoding(CDW cdw) const {
  if (m_cdw_map.find(cdw) == m_cdw_map.end()) {
    throw std::runtime_error(boost::str(
        boost::format("[e] Unknown CDW '%i' requested!") % ((int)cdw)));
  }
  return m_cdw_map.at(cdw);
}

void Decompressor::preloadCDW() {
  if (m_cfg_ptr->getProperty(CFG::EXT_CDWS)) {
    m_cdw_map = {
        {CDW::XXX, ""},   {CDW::LXX, "0"},   {CDW::HXX, "1"},
        {CDW::LLX, "00"}, {CDW::HLX, "10"},  {CDW::HHX, "11"},
        {CDW::LHX, "01"}, {CDW::LLL, "000"}, {CDW::LLH, "001"},
        //                      { CDW::LHL, "010" },   { CDW::LHH, "011" },   {
        //                      CDW::HLL, "100" }, { CDW::HLH, "101" },   {
        //                      CDW::HHL, "110" },   { CDW::HHH, "111" },   {
        //                      CDW::LLLL, "0000" }, { CDW::LLLH, "0001" }, {
        //                      CDW::LLHL, "0010" }, { CDW::LLHH, "0011" }, {
        //                      CDW::LHLL, "0100" }, { CDW::LHLH, "0101" }, {
        //                      CDW::LHHL, "0110" }, { CDW::LHHH, "0111" }, {
        //                      CDW::HLLL, "1000" }, { CDW::HLLH, "1001" }, {
        //                      CDW::HLHL, "1010" }, { CDW::HLHH, "1011" }, {
        //                      CDW::HHLL, "1100" }, { CDW::HHLH, "1101" }, {
        //                      CDW::HHHL, "1110" }, { CDW::HHHH, "1111" }
    };
    m_udw_map = {
        //   { CDW::XXX , "" }        // Empty
#ifdef ETS
        {"1", CDW::LXX},          // 0
        {"00000000", CDW::HXX},   // 1
        {"1111", CDW::LLX},       // 00
        {"0101", CDW::LHX},       // 01
        {"0110", CDW::HLX},       // 10
        {"0", CDW::HHX},          // 11                  ,
        {"0101010101", CDW::LLL}, // 000
        {"1010", CDW::LLH},       // 001
//                      { "0000", CDW::LHL },       // 010
//                      { "10101010", CDW::LHH },   // 011
//                      { "1000", CDW::HLL },       // 100
//                      { "1001", CDW::HLH },       // 101
//                      { "0001", CDW::HHL },       // 110
//                      { "11111111", CDW::HHH },   // 111
#else
        {"0", CDW::LXX},    // 0
        {"1", CDW::HXX},    // 1
        {"0000", CDW::LLX}, // 00
        {"0001", CDW::LHX}, // 01
        {"0011", CDW::HLX}, // 10
        {"1100", CDW::HHX}, // 11                  ,
        {"0101", CDW::LLL}, // 000
        {"1010", CDW::LLH}, // 001
//                      { "0000", CDW::LHL },       // 010
//                      { "10101010", CDW::LHH },   // 011
//                      { "1000", CDW::HLL },       // 100
//                      { "1001", CDW::HLH },       // 101
//                      { "0001", CDW::HHL },       // 110
//                      { "11111111", CDW::HHH },   // 111
#endif
        //                      { "00110011", CDW::LLLL },  // 0000
        //                      { "11001100", CDW::LLLH },  // 0001
        //                      { "10001000", CDW::LLHL },  // 0010
        //                      { "10011001", CDW::LLHH },  // 0011
        //                      { "00010001", CDW::LHLL },  // 0100
        //                      { "00100010", CDW::LHLH },  // 0101
        //                      { "00010001", CDW::LHHL },  // 0110
        //                      { "01100110", CDW::LHHH },  // 0111
        //                      { "11110000", CDW::HLLL },  // 1000
        //                      { "00001111", CDW::HLLH },  // 1001
        //                      { "11000000", CDW::HLHL },  // 1010
        //                      { "00000011", CDW::HLHH },  // 1011
        //                      { "00110000", CDW::HHLL },  // 1100
        //                      { "11001111", CDW::HHLH },  // 1101
        //                      { "11110011", CDW::HHHL },  // 1110
        //                      { "00001100", CDW::HHHH }
    }; // 1111
  } else {
    m_cdw_map = {{CDW::XXX, ""},    {CDW::LXX, "0"},   {CDW::HXX, "1"},
                 {CDW::LLX, "00"},  {CDW::HLX, "10"},  {CDW::HHX, "11"},
                 {CDW::LHX, "01"},  {CDW::LLL, "000"}, {CDW::LLH, "001"},
                 {CDW::LHL, "010"}, {CDW::LHH, "011"}, {CDW::HLL, "100"},
                 {CDW::HLH, "101"}, {CDW::HHL, "110"}, {CDW::HHH, "111"}};
    m_udw_map = {                    //   { CDW::XXX , "" }        // Empty
                 {"0", CDW::LXX},    // 0
                 {"1", CDW::HXX},    // 1
                                     //{ "0000", CDW::LLX },  // 00
                 {"0010", CDW::LLX}, // 00
                 {"1101", CDW::LHX}, // 01
                                     //{ "0110", CDW::LHX },
                 {"0011", CDW::HLX}, // 10
                 {"1100", CDW::HHX}, // 11                  ,
                 {"0101", CDW::LLL}, // 000
                 {"1010", CDW::LLH}, // 001
                 {"1111", CDW::LHL}, // 010
                 {"1001", CDW::LHH}, // 011
                 {"0110", CDW::HLL}, // 100

                 {"0000", CDW::HLH},  // 101
                                      //{ "0010", CDW::HLH },      // 101
                 {"0100", CDW::HHL},  // 110
                 {"1000", CDW::HHH}}; // 111
  }

  m_tbcs = {};
  m_cdw_benefit = {};
  if (m_cfg_ptr->getProperty(CFG::EXT_CDWS)) {
    m_tbrs = {CDW::HHHH, CDW::HHHL, CDW::HHLH, CDW::HHLL, CDW::HLHH, CDW::HLHL,
              CDW::HLLH, CDW::HLLL, CDW::LHHH, CDW::LHHL, CDW::LHLH, CDW::LHLL,
              CDW::LLHH, CDW::LLHL, CDW::LLLH, CDW::LLLL, CDW::HHH,  CDW::HHL,
              CDW::HLH,  CDW::HLL,  CDW::LHH,  CDW::LHL,  CDW::LLH,  CDW::LLL,
              CDW::HHX,  CDW::HLX,  CDW::LHX,  CDW::LLX};
  } else {
    if (m_cfg_ptr->getProperty(CFG::MAX_CDWS) > 8) {
      m_tbrs = {CDW::HHH, CDW::HHL, CDW::HLH, CDW::HLL, CDW::LHH, CDW::LHL,
                CDW::LLH, CDW::LLL, CDW::HHX, CDW::HLX, CDW::LHX, CDW::LLX};
    } else {
      m_tbrs = {CDW::HHH, CDW::HHL, CDW::HLH, CDW::HLL,
                CDW::LHH, CDW::LHL, CDW::LLH, CDW::LLL};
    }
    //  };
  }
  for (auto const &elem : m_cdw_map) {
    m_cdw_weight[elem.first] = elem.second.length();
  }
  assert(m_cdw_weight.size() == m_cdw_map.size());
}

void Decompressor::reset() {
  //      preloadCDW();
  m_cdw_benefit.clear();
  m_tbcs.clear();
  if (m_cfg_ptr->getProperty(CFG::EXT_CDWS)) {
    m_tbrs = {CDW::HHHH, CDW::HHHL, CDW::HHLH, CDW::HHLL, CDW::HLHH, CDW::HLHL,
              CDW::HLLH, CDW::HLLL, CDW::LHHH, CDW::LHHL, CDW::LHLH, CDW::LHLL,
              CDW::LLHH, CDW::LLHL, CDW::LLLH, CDW::LLLL, CDW::HHH,  CDW::HHL,
              CDW::HLH,  CDW::HLL,  CDW::LHH,  CDW::LHL,  CDW::LLH,  CDW::LLL,
              CDW::HHX,  CDW::HLX,  CDW::LHX,  CDW::LLX};
  } else {
    m_tbrs = {CDW::HHH, CDW::HHL, CDW::HLH, CDW::HLL, CDW::LHH, CDW::LHL,
              CDW::LLH, CDW::LLL, CDW::HHX, CDW::HLX, CDW::LHX, CDW::LLX};
  }
}

void Decompressor::dumpEntries() const {
  cout << "###########################" << endl;
  cout << "[DUMP] Entries of decompressor:" << endl;
  for (auto const &elem : m_udw_map) {
    //        cout << boost::format( "'%s'    \t  %s(%i)" ) % elem.first %
    //        CDWtoString( elem.second ) % ( ( int )elem.second ) << endl;
    cout << boost::format("%s <= %s (%i)") % elem.first %
                CDWtoString(elem.second) % ((int)elem.second)
         << endl;
  }
  cout << "[DUMP] ... ends!" << endl;
  cout << "###########################" << endl;
}

void Decompressor::dumpConfiguration() const {
  cout << "###########################" << endl;
  cout << "[DUMP] Configuration of decompressor:" << endl;
  for (auto const &elem : m_tbcs) {
    cout << boost::format("%s") % elem << endl;
  }
  cout << "[DUMP] ... ends!" << endl;
  cout << "###########################" << endl;
}

bool Decompressor::isUDWLength(BitVecCItr &l_start, BitVecCItr &l_end,
                               unsigned int lb) {
  auto dist = distance(l_start, l_end);
  return ((lb <= dist) && (dist == 1 || dist == 4 || dist == 8));
}

bool Decompressor::isEmptyCDW(CDW result) { return (result == CDW::XXX); }

bool Decompressor::isValidCDW(CDW result) { return (result != CDW::NONE); }

bool Decompressor::isStaticCDW(CDW result, bool ext_cdws) {
  switch (result) {
  case CDW::LXX:
  case CDW::HXX:
    return true;
  case CDW::LLX:
  case CDW::LHX:
  case CDW::HLX:
  case CDW::HHX:
    //            return !ext_cdws;
    return false;
    // TODO !!!
  default:
    return false;
  }
}

bool Decompressor::storeDynCDW(std::string &cdw_repl) {
  auto udw_it = m_udw_map.find(cdw_repl);
  if (udw_it == m_udw_map.end() || // Not already covered
      (udw_it != m_udw_map.end() &&
       find(m_tbrs.begin(), m_tbrs.end(), udw_it->second) !=
           m_tbrs.end())) { // Marked to be overwritten
           assert( !m_tbrs.empty() );
    if (m_tbrs.empty()) {
      std::cout << boost::format("[e] Overfill '%s' !") % cdw_repl << std::endl;
      return true;
    }
    auto tbr = m_tbrs.back();
    m_tbrs.pop_back();

    if (m_cfg_ptr->isVerbose() || true) {
      std::cout << boost::format(
                       "[i] Inserting '%s' by replacing CDW '%s' (%i)!") %
                       cdw_repl % CDWtoString(tbr) % ((int)tbr)
                << std::endl;
    }
    m_tbcs.push_back(cdw_repl);
    for (auto &elem : boost::adaptors::reverse(m_udw_map)) {
      if (elem.second == tbr) {
        m_udw_map.erase(elem.first);
      }
    }
    m_udw_map[cdw_repl] = tbr;
    return true;
  }
  if (m_cfg_ptr->isVerbose()) {
    std::cout << boost::format("[i] Skipping '%s' insertion - already covered "
                               "by default CDW set!") %
                     cdw_repl
              << std::endl;
  }
  return false;
}

bool Decompressor::storeDynCDW(boost::dynamic_bitset<> cdw_repl) {
  string cdw_repl_str;
  to_string(cdw_repl, cdw_repl_str);
  return storeDynCDW(cdw_repl_str);
}

const Decompressor::UDWStringMap Decompressor::extractUDW() const {
  return m_udw_map;
}

u_int8_t Decompressor::getCDWLength(CDW &cdw) const {
  return m_cdw_weight.at(cdw);
}
} // namespace vecthor
