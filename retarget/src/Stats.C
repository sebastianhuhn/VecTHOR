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

#include <Stats.h>

// Project includes

// System includes
#include <cmath>
#include <iostream>
#include <numeric>

// Boost includes
#include <boost/format.hpp>

namespace vecthor {

using namespace std;

void EmitterStats::printStats(const string &title, ostream &out) const {
  out << "###########################" << endl;
  out << boost::format("[s] Printing %s emitter stats summary:") % title
      << endl;
  out << boost::format("[%s] #Cycles:\t%i") % title % m_cycles << endl;
  out << boost::format("[%s] #CFG-Cycles:\t%i") % title % m_config_cycles
      << endl;
  out << boost::format("[%s] #Sum-Cycles:\t%i") % title %
             (m_cycles + m_config_cycles)
      << endl;
  out << boost::format("[%s] #Sum-Cycle-time:\t%i") % title %
             ((m_cycles + m_config_cycles) * CYCLE_TIME)
      << endl;
  out << "#TDI resets:\t\t" << m_tdi_resets << endl;
  out << "#COMPR_DR:\t\t\t" << m_compr_dr << endl;
  out << "#COMPR_EXIT:\t\t" << m_compr_exit << endl;
  out << "#COMPR_REAPT:\t\t" << m_compre_repeat << endl;
  out << "#Multi-REP:\t\t" << m_multi_rep << endl;
  out << "###########################" << endl;
}

void EmitterStats::collectBenchmarkData() {}

void EmitterStats::clear() {
  m_cycles = 3; // PRE = 2, POST = 1
  m_config_cycles = 3;
  m_tdi_resets = 0;
  m_compr_dr = 0;
  m_compr_exit = 0;
  m_compre_repeat = 0;
  m_multi_rep = 0;
}

void EmitterStats::printBenchmarkData(const string &title) const {
  auto out = m_cfg_ptr->getBenchmarkFile();
  *out << title << "Emitter Stats" << endl;
  *out << "#Configuration [Cycles]:\t" << m_config_cycles << endl;
  *out << "#Data [Cycles]:\t\t\t" << m_cycles << endl;
  auto sum_cycles = m_config_cycles + m_cycles;
  *out << "#Sum [Cycles]:\t\t\t" << sum_cycles << endl;
  float cycle_factor = (float)sum_cycles / 1;
    *out << boost::format("Cycle factor: \t%f") % cycle_factor << endl;
  float cycle_percent = (float)(1 - cycle_factor) * 100;
  *out << boost::format("Cycle percent: \t%f") % cycle_percent << endl;
  // TODO: Add Multichannel Stats (w/o postprocessing)
  *out << endl;
}

void CompressorStats::printStats(const string &title, ostream &out) const {
  if (!m_cfg_ptr->getProperty(CFG::STATS)) {
    return;
  }
  out << "###########################" << endl;
  out << "[s] Printing stats summary:" << endl;
  out << "#Processed bit:\t" << m_num_bit << endl;
  out << "#Compressed bit:\t" << getComprBit() << endl;
  out << "###########################" << endl;
  out << "#Replacements:\t\t" << m_num_replacements << endl;
  out << "#Stage 1 repls:\t\t" << m_num_s1_repls << endl;
  out << "#Stage 2 repls:\t\t" << m_num_s2_repls << endl;
  float sbi_percentage = (float)m_num_sbf / m_num_replacements;
  out << boost::format("#Single bit injections:\t%i ( %f )") % m_num_sbf %
             sbi_percentage
      << endl;
  if (m_cfg_ptr->getProperty(CFG::MERGING)) {
    out << boost::format("#Repititions (costs):\t%i ( %i )") %
               m_num_cdw_repetition % m_num_red_repetition
        << endl;
  }
  out << "Total benefit:\t\t" << m_num_benefit << endl;
  float compression_factor = (float)getComprBit() / m_num_bit;
  out << boost::format("Compression factor: \t%f") % compression_factor << endl;
  float compression_percent = (float)(1 - compression_factor) * 100;
  out << boost::format("Compression percent: \t%f") % compression_percent
      << endl;
  out << "... summary ends." << endl;
  printCDWUsage(out);
  out << "###########################" << endl;
}

void CompressorStats::collectBenchmarkData() {
  m_num_overall_bit += m_num_bit;
  m_num_overall_compressed_bit += getComprBit();
  m_counter_cdws_length.clear();
  for (auto const i : m_counter_cdws) {
    auto len = floor(log2((int)i.first));
    m_counter_cdws_length[len] += (unsigned)i.second;
  }
}

void CompressorStats::clear() {
  m_num_sbf = 0;
  m_num_benefit = 0;
  m_num_bit = 0;
  m_num_replacements = 0;
  m_num_s1_repls = 0;
  m_num_s2_repls = 0;
  m_num_cdw_repetition = 0;
  m_num_red_repetition = 0;
}

void CompressorStats::printBenchmarkData(const string &title) const {
  auto out = m_cfg_ptr->getBenchmarkFile();
  *out << "Compressor Stats:" << endl;
  *out << "#Legacy [Bit]:\t\t" << m_num_overall_bit << endl;
  *out << "#Compressed [Bit]:\t" << m_num_overall_compressed_bit << endl;
  float compression_factor =
      (float)m_num_overall_compressed_bit / m_num_overall_bit;
  *out << boost::format("Compression factor: \t%f") % compression_factor
       << endl;
  float compression_percent = (float)(1 - compression_factor) * 100;
  *out << boost::format("Compression percent: \t%f") % compression_percent
       << endl;
  *out << endl;
}

int CompressorStats::getComprBit() const {
  int compr_bit = m_num_bit - m_num_benefit;
  if (m_num_red_repetition > 1) {
    compr_bit -= m_num_red_repetition - 1;
  }
  return compr_bit;
}

void CompressorStats::printCDWUsage(ostream &out) const {
  out << "###########################" << endl;
  out << "[s] CDW Usage:" << endl;
  for (auto const i : m_counter_cdws) {
    out << boost::format("'%s'\t\t %i") % ((int)i.first) % i.second << endl;
  }
  float sum = accumulate(begin(m_counter_cdws_length),
                         std::end(m_counter_cdws_length), 0,
                         [](const long long previous,
                            const std::pair<const unsigned, long long> &p) {
                           return previous + p.second;
                         });
  out << "[i] Number of codewords: " << sum << endl;
  out << "[s] CDW Length Distribution:" << endl;
  long cycles =  6 + 6 + 12 + 50; // ETS'19: twice a six instruction sequence plus + 50 bit protocol overhead.
  long cycles_multichan = cycles;
  for (auto const i : m_counter_cdws_length) {
    out << boost::format("\t|%u|\t\t %u \t(%.2f)") % i.first % i.second %
               (i.second / sum)
        << endl;
    if (i.first == 0) {
      cycles += i.second; // RLE repititions
    } else if (i.first == 2) {
      cycles_multichan += i.second;
    }
    cycles_multichan += i.second;
    cycles += (i.first * i.second);
  }
  out << "[i] Number of compressed cycles: " << cycles << endl;
  // TODO: Calc cycle factor + rate
  //  out << boost::format( "Cycle factor: \t%f" ) % cycle_factor << endl;
  out << "[i] Number of multichannel cycles: " << cycles_multichan << endl;
  out << "[i] Number of multichannel data overhead: "
      << m_num_overall_mc_overhead_bit << endl;
}

void DecompressorStats::printStats(const string &title, ostream &out) const {
  if (!m_cfg_ptr->getProperty(CFG::STATS)) {
    return;
  }
  out << "###########################" << endl;
  out << "[s] Dyn-Decompressor:" << endl;
  out << "#Configuration DR bit:\t\t" << m_config_bit << endl;
  out << "###########################" << endl;
}

void DecompressorStats::collectBenchmarkData() {
  m_overall_config_bit += m_config_bit;
}

void DecompressorStats::printBenchmarkData(const string &title) const {
  auto out = m_cfg_ptr->getBenchmarkFile();
  *out << "Decompressor Stats" << endl;
  *out << "#Configuration [Bit]:\t" << m_overall_config_bit;
}

void FormalDecompressorStats::printStats(const string &title,
                                         ostream &out) const {
  if (!m_cfg_ptr->getProperty(CFG::STATS)) {
    return;
  }
  out << "###########################" << endl;
  out << "[s] Dyn-Decompressor:" << endl;
  out << "#Configuration DR bit:\t\t" << m_config_bit << endl;
  out << "###########################" << endl;
  out << "[s] SAT-based decompressor:" << endl;
  out << "#Allocated vars:\t\t" << m_vars << endl;
  out << "#Binary clauses:\t\t" << m_bin_clauses << endl;
  out << "#Constraints:\t\t" << m_constraints << endl;
  out << "#Restarts:\t\t\t" << m_restarts << endl;
  out << "#Conflicts-Clauses:\t\t" << m_ccs << endl;
  out << boost::format("#Act. CDWs (static):\t%i ( %i )") % m_det_cdws %
             m_det_static_cdws
      << endl;
  out << "#Act. SBIs:\t\t\t" << m_det_sbis << endl;
  out << boost::format("#Act. merge vars:\t\t%i ( %i ) ") % m_merge_vars %
             m_act_merges
      << endl;
  out << "###########################" << endl;
}

void FormalDecompressorStats::collectBenchmarkData() {
  m_overall_config_bit += m_config_bit; // TODO: Not yet implemented
}

void FormalDecompressorStats::printBenchmarkData(const string &title) const {
  auto out = m_cfg_ptr->getBenchmarkFile();
  *out << "FormalDecompressor Stats" << endl;
  *out << endl;
}

inline std::string Stats::separatorToken() { return std::string(10, '#'); }
} // namespace vecthor
