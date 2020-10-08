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

#ifndef CONFIG_H
#define CONFIG_H

// Project includes
#include <Utils.h>

// System includes
#include <map>
#include <ostream>
#include <string>

namespace vecthor {

enum class CFG : u_int8_t {
  UNSUPPORTED,
  MERGING,
  DYNAMIC,
  HEUR_INNER_FREQ,
  HEUR_OUTER_FREQ,
  HEUR_WEIGHT,
  HEUR_PERMUTE,
  SAT,
  SAT_SEC,
  SAT_CONFL,
  SAT_RESTART,
  MAX_CDWS,
  PART_SIZE,
  //  MAX_BYTE_CDWS,
  EXT_CDWS,
  // SAT_SBI,
  VERBOSE,
  DEBUG,
  STATS,
  BENCHMARK,
  PLOT,
  HEX,
  P2S_BUFFER,
  USE_EXT_FILE,
  USE_CONF_FILE,
  GEN_LEGACY,
  GEN_COMPRESSED,
  GEN_GOLDEN,
  HW_EMIT,
  ALLOW_X,
  VALIDATE
};

enum class FILE : u_int8_t {
  UNSUPPORTED,
  LEGACY_PREFIX,
  LEGACY_SUFFIX,
  DYNCOMPRESSED_INFIX,
  DYNCOMPRESSED_PRELOAD,
  COMPRESSED_PREFIX,
  COMPRESSED_SUFFIX,
  COMPRESSED_FILE,
  CONFIG_FILE,
  RESYNC_FILE,
  EXT_FILE,
  LEGACY_FILE,
  GOLDEN_FILE,
  VALIDATION_FILE
};

class Config {

  using OfStrPtr = std::shared_ptr<std::ofstream>;

public:
  Config();
  ~Config();
  bool isDebug() const { return getProperty(CFG::DEBUG); }
  bool isVerbose() const { return getProperty(CFG::VERBOSE); }

  CFG getCFGType(std::string &property) const;
  FILE getFILEType(std::string &property) const;
  OfStrPtr getBenchmarkFile() const;

  unsigned int getProperty(CFG cfg) const;
  const std::string &getFile(FILE file) const;

  void setProperty(CFG cfg, int value = 1);
  void setFile(FILE file, std::string &filename);

  bool parseArgs(int argc, char **argv);
  bool parseConfig();
  void prepare(const std::string &run_name);

  void preloadCDW();
  void dump();

  void printIcon();

private:
  void initialize();
  std::string CFGtoString(CFG cfg) const;

public:
  unsigned int m_num_rtdr = 16;

private:
  using CfgStringMap = std::map<CFG, std::string>;
  std::map<CFG, int> m_cfg_map;
  CfgStringMap m_cfg_str;

  using FileStringMap = std::map<FILE, std::string>;
  std::map<FILE, std::string> m_file_map;
  FileStringMap m_file_str;

  OfStrPtr m_benchmark_filep;
};
} // namespace vecthor
#endif // CONFIG_H
