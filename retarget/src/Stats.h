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

#ifndef STATS_H
#define STATS_H

// Project includes
#include <Config.h>
#include <TypeDefs.h>

// System includes
#include <iostream>
#include <map>
#include <string>

// Boost includes

namespace vecthor {

class Stats {
public:
  Stats(const Config *cfg_ptr) : m_cfg_ptr(cfg_ptr), m_stats_db() {}

  virtual void printStats(const std::string &title = "",
                          std::ostream &out = std::cout) const = 0;
  virtual void collectBenchmarkData() = 0;
  virtual void printBenchmarkData(const std::string &title = "") const = 0;
  inline std::string separatorToken();

protected:
  const Config *m_cfg_ptr;
  std::map<std::string, void *> m_stats_db;
};

class CompressorStats : public Stats {

public:
  CompressorStats(const Config *cfg_ptr) : Stats(cfg_ptr) {}

  void printStats(const std::string &title = "",
                  std::ostream &out = std::cout) const;
  void collectBenchmarkData() override;
  void clear();
  void printBenchmarkData(const std::string &title = "") const override;

public:
  unsigned int m_num_sbf = 0;
  unsigned int m_num_benefit = 0;
  unsigned int m_num_bit = 0;
  unsigned int m_num_replacements = 0;
  unsigned int m_num_s1_repls = 0;
  unsigned int m_num_s2_repls = 0;
  unsigned int m_num_cdw_repetition = 0;
  unsigned int m_num_red_repetition = 0;

  unsigned int m_num_overall_bit = 0;
  unsigned int m_num_overall_compressed_bit = 0;
  mutable unsigned int m_num_overall_mc_overhead_bit = 0;

  std::map<CDW, int> m_counter_cdws = std::map<CDW, int>();
  std::map<unsigned, long long> m_counter_cdws_length =
      std::map<unsigned, long long>();

private:
  inline int getComprBit() const;
  void printCDWUsage(std::ostream &out) const;
};

class EmitterStats : public Stats {
public:
  EmitterStats(const Config *cfg_ptr) : Stats(cfg_ptr) {}

  void printStats(const std::string &title = "",
                  std::ostream &out = std::cout) const;
  void collectBenchmarkData() override;
  void clear();
  void printBenchmarkData(const std::string &title = "") const override;

public:
  unsigned int m_cycles = 3; // PRE = 2, POST = 1
  unsigned int m_config_cycles = 3;
  unsigned int m_tdi_resets = 0;
  unsigned int m_compr_dr = 0;
  unsigned int m_compr_exit = 0;
  unsigned int m_compre_repeat = 0;
  unsigned int m_multi_rep = 0;
  // TODO: Not yet implemented
  // unsigned int m_num_overall_legacy_cycles = 0;
  // unsigned int m_num_overall_config_cycles = 0;
  // unsigned int m_num_overall_compresed_cycles = 0;
private:
};

class DecompressorStats : public Stats {
public:
  DecompressorStats(const Config *cfg_ptr) : Stats(cfg_ptr) {}

  void printStats(const std::string &title = "",
                  std::ostream &out = std::cout) const;
  void collectBenchmarkData() override;
  void printBenchmarkData(const std::string &title = "") const override;

public:
  unsigned int m_config_bit = 0;
  unsigned int m_overall_config_bit = 0;

private:
};

class FormalDecompressorStats : public Stats {
public:
  FormalDecompressorStats(const Config *cfg_ptr) : Stats(cfg_ptr) {}

  void printStats(const std::string &title = "",
                  std::ostream &out = std::cout) const;
  void collectBenchmarkData() override;
  void printBenchmarkData(const std::string &title = "") const override;

public:
  unsigned int m_restarts = 0;
  unsigned int m_ccs = 0;
  unsigned int m_bin_clauses = 0;
  unsigned int m_constraints = 0;
  unsigned int m_vars = 0;
  unsigned int m_det_cdws = 0;
  unsigned int m_det_static_cdws = 0;
  unsigned int m_det_sbis = 0;
  unsigned int m_merge_vars = 0;
  unsigned int m_act_merges = 0;
  unsigned int m_config_bit = 0;
  unsigned int m_overall_config_bit = 0;

private:
};

} // namespace vecthor
#endif
