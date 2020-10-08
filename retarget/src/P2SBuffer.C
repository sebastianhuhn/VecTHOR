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

#include <P2SBuffer.h>

// Project includes
#include <Decompressor.h>
#include <Plotter.h>

// System includes
#include <algorithm>
#include <iostream>

// Boost includes
#include <boost/format.hpp>
#include <boost/timer.hpp>

namespace vecthor {
using namespace std;

unsigned int P2SBuffer::processRoute(Route &route, std::size_t max_cycles) {
  boost::timer buf_timer;
  unsigned int delay = 0;
  m_buf.resize(max_cycles);
  unsigned int i = 0;
  u_int8_t last_length = 0u;
  for (auto const &elem : route) {
    assert(elem);
    auto cdw = get<0>(*elem);
    assert(m_decompr->isValidCDW(cdw));
    auto dist = std::distance(get<1>(*elem), get<2>(*elem));
    if (!m_decompr->isEmptyCDW(cdw)) {
      last_length = m_decompr->getCDWLength(cdw);
    }
    ++i += last_length;
    //        std::cout << boost::format("[d] P2SBuffer at %i with dist %i") % i
    //        % dist << std::endl;
    m_collector.emplace_back(i, dist);
  }
  m_collector.shrink_to_fit();
  simulateDataSink();
  dumpBuffer();
  int min_val = *min_element(m_buf.begin(), m_buf.end());
  cout << "[i] Initial min. value: " << min_val << endl;
  if (min_val < 0) {
    delay = determineDelay(abs(min_val), max_cycles);
  }
  int max_val = *max_element(m_buf.begin(), m_buf.end());
  cout << "[i] Required buffer size: \t" << max_val << endl;
  cout << "[i] Bufffer time: \t\t" << buf_timer.elapsed() << endl;
  //    plot( "data_sink" );
  return delay;
}

void P2SBuffer::simulateDataSink(unsigned int delay) {
  unsigned int idx = delay;
  for (auto const &elem : m_collector) {
    --m_buf[idx]; // Consume one bit
    auto data_cycle = elem.first;
    auto data_num = elem.second;
    // assert( cycle <= idx );
    if ((delay > 0) && (data_cycle < delay)) {
      m_buf[delay - 1] += data_num;
      continue;
    }
    while (idx < data_cycle) {
      // std::cout << idx << ":" << data_cycle << std::endl;
      assert(idx < m_buf.size());
      m_buf[idx + 1] = m_buf[idx] - 1;
      ++idx;
    }
    if (idx == data_cycle) {
      m_buf[idx] = data_num;
      auto prev_val = m_buf[idx - 1];
      if (prev_val > 0) {
        m_buf[idx] += prev_val;
      }
    }
  }
  if (m_cfg_ptr->isVerbose()) {
    cout << "[i] P2S Simulation ends:\t" << idx << endl;
  }
}

unsigned int P2SBuffer::determineDelay(unsigned int delay, size_t max_cycles) {
  int min_val = 0;
  unsigned int l_delay = delay;
  do {
    if (m_cfg_ptr->isVerbose()) {
      cout << "[v] Inserting init. delay: " << l_delay
           << " - min. value: " << min_val << endl;
    }
    auto prev_size = m_buf.size();
    m_buf.clear();
    m_buf.resize(prev_size + delay);
    simulateDataSink(l_delay);
    min_val = *min_element(m_buf.begin(), m_buf.end());
    dumpBuffer();
    ++l_delay;
  } while ((min_val < 0) && (l_delay < max_cycles));
  ++l_delay;
  dumpCollector();
  dumpBuffer();
  cout << "[i] Required delay: \t" << l_delay << endl;
  return l_delay;
}

void P2SBuffer::dumpBuffer(bool force) const {
  if (!m_cfg_ptr->isDebug() && !force) {
    return;
  }
  cout << "###########################" << endl;
  cout << boost::format("[DUMP] Data buffer (#lines %i):") % m_buf.size()
       << endl;
  for (unsigned int i = 0; i < m_buf.size(); ++i) {
    // for( auto const& elem : m_buf ) {
    cout << boost::format("Data: %i \t %i") % m_buf.at(i) % i << endl;
  }
  cout << "[DUMP] ... ends!" << endl;
  cout << "###########################" << endl;
}

void P2SBuffer::dumpCollector(bool force) const {
  if (!m_cfg_ptr->isDebug() && !force) {
    return;
  }
  cout << "###########################" << endl;
  cout << "[DUMP] Data sink of route:" << endl;
  unsigned int ctr = 0;
  for (auto const &elem : m_collector) {
    cout << boost::format("CDW %u: After %u - data: %u") % ++ctr % elem.first %
                elem.second
         << endl;
  }
  cout << "[DUMP] ... ends!" << endl;
  cout << "###########################" << endl;
}

void P2SBuffer::plot(string plot_name) const {
  using PC = Plotter::CFGATTR;
  Plotter plotter;
  Plotter::ConfigMap cfg;
  plotter.initTypeConfig<Plotter::HistoPlot>();
  cfg[PC::TITLE] = m_cfg_ptr->getFile(FILE::EXT_FILE);
  cfg[PC::DATAFILENAME] = plot_name + ".txt";
  cfg[PC::TERMINAL] = "eps";
  cfg[PC::NAME] = plot_name + ".gpl";
  cfg[PC::OUTPUT] = plot_name + ".eps";
  cfg[PC::PLOT] = plot_name + ".txt";
  cfg[PC::XLABEL] = "shift cycle";
  cfg[PC::XTICS] = "rotate by -90 scale 0.5";
  plotter.writeConfig(cfg);
  vector<long double> fst;
  vector<long double> snd;
  for (unsigned int i = 0; i < m_buf.size(); ++i) {
    fst.push_back(m_buf.at(i));
    snd.push_back(i);
  }
  plotter.writeData(fst, snd);
}
} // namespace vecthor
