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

#ifndef P2SCONVERT_H
#define P2SCONVERT_H

// System includes
#include <array>
#include <iostream>
#include <string>
#include <vector>

// Project includes
#include <Config.h>
#include <TypeDefs.h>

#define MAX_BUFFER 100

namespace vecthor {

class Decompressor;

class P2SBuffer {

public:
  using DataBuffer = std::vector<int>; // Currently avail. data at time i
  using DataCollector =
      std::vector<std::pair<unsigned int, unsigned int>>; // Fst: data cycle,
                                                          // Snd: data num

  P2SBuffer(const Config *config, const Decompressor *decompr)
      : m_decompr(decompr), m_cfg_ptr(config) {}

  const DataCollector &getCollector() const { return m_collector; }
  unsigned int processRoute(Route &route, std::size_t max_cycles);

private:
  void simulateDataSink(unsigned int delay = 0);
  unsigned int determineDelay(unsigned int delay, std::size_t max_cycles);
  void dumpBuffer(bool force = false) const;
  void dumpCollector(bool force = false) const;
  void plot(std::string plot_name) const;

private:
  unsigned int m_max_buf;
  DataBuffer m_buf;
  DataCollector m_collector;
  const Decompressor *m_decompr;
  const Config *m_cfg_ptr;
};
} // namespace vecthor

#endif // P2SCONVERT_H
