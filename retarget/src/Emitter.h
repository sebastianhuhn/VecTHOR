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

#ifndef TESTBENCHEMITTER_H
#define TESTBENCHEMITTER_H

// Project includes
#include <Config.h>
#include <Decompressor.h>
#include <DynDecompressor.h>
#include <P2SBuffer.h>
#include <Stats.h>
#include <TypeDefs.h>

// System includes
#include <fstream>
#include <iostream>

// Boost Includes
#include <boost/filesystem.hpp>

namespace vecthor {

class Emitter {

public:
  Emitter(const Config *config_ptr, const Decompressor *decomp_ptr = nullptr)
      : m_cfg_ptr(config_ptr), m_decomp_ptr(decomp_ptr), m_stats(config_ptr) {}

  void writeGoldenFile(const BitVec &bit_vec);
  inline EmitterStats &getStats() { return m_stats; }

  EmitterStats getStats() const { return m_stats; }

protected:
  VALUE getValue(const char &c);
  bool isHigh(VALUE val) { return (val == VALUE::HIGH); }
  bool isLow(VALUE val) { return (val == VALUE::LOW); }

protected:
  const Config *m_cfg_ptr;
  const Decompressor *m_decomp_ptr;
  EmitterStats m_stats;
};

class LegacyEmitter : public Emitter {
public:
  LegacyEmitter(const Config *config) : Emitter(config) {}
  void operator()(const BitVec &bit_vec);

private:
  void writeJTAG(const BitVec &bit_vec, std::ostream *stream);
};

class CompressedEmitter : public Emitter {
public:
  CompressedEmitter(const Config *config, const Decompressor *decomp_ptr);
  ~CompressedEmitter();
  void init();
  void finalize();
  void operator()(const Route &route, unsigned int delay = 0);
  void writeResyncFile(const P2SBuffer::DataCollector &data_collector,
                       unsigned int delay);

private:
  void addSignalValue(const char &a, const char &b);
  void writeJTAG(const Route &route);
  void writePreload(unsigned int delay);
  void writeComprInstr();
  void writePreloadInstr();

private:
  std::ofstream m_compr_file;
  Signals m_signals;
};

} // namespace vecthor
#endif // TESTBENCHEMITTER_H
