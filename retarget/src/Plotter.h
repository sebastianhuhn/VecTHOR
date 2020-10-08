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

#ifndef PLOTTER_H
#define PLOTTER_H

// Project includes
#include <TypeDefs.h>

// System includes
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// Boost includes
#include <boost/filesystem.hpp>

#define MAX_PLOTS 10
#define MAX_TITLE_LENGTH 40

namespace vecthor {
namespace bf = boost::filesystem;

class Plotter {
public:
  enum PlotType { ScatterPlot, HistoPlot, Plot3D };

  enum class CFGATTR {
    NAME,
    DATAFILENAME,
    XLABEL,
    YLABEL,
    ZLABEL,
    XTICS,
    YTICS,
    ZTICS,
    XRANGE,
    YRANGE,
    ZRANGE,
    TITLE,
    TERMINAL,
    OUTPUT,
    SIZE,
    GRID,
    DESCRIPTION,
    DESCRIPTIONPOS,
    STYLEDATA,
    STYLEFILL,
    FONT,
    FONTSIZE,
    LABEL,
    LABELPOS,
    AZIMUT,
    ELEVATION,
    PLOT,
    SPLOT,
    GRAPHTITLE,
    USING,
    EVERY,
    SYMBOL
  };

  using ConfigMap = std::map<CFGATTR, std::string>;
  using ConfigEntry = ConfigMap::value_type;

  using ConfigLookupMap = std::map<CFGATTR, std::string>;
  Plotter();

  template <PlotType T> void initTypeConfig();
  int writeConfig(ConfigMap &user_settings);
  // Templates
public:
  template <typename S, typename T>
  int writeData(std::vector<S> &fst_values, std::vector<T> &snd_values) {
    std::vector<int> rd_values;
    return (writeData(fst_values, snd_values, rd_values));
  }

private:
  bool isSkippable(CFGATTR attr) const;
  bool isEmpty(CFGATTR attr) const;
  bool isQuoted(CFGATTR attr) const;
  std::string getAttribute(CFGATTR attr) const;
  void generatePlotCfg(std::ofstream &out) const;
  void generatePlot(std::ofstream &out) const;

  int getConfigLength() { return m_cfg.size(); }
  void initConfig();

  // Templates
private:
  template <typename S, typename T, typename U>
  int writeData(std::vector<T> &fst_values, std::vector<S> &snd_values,
                std::vector<U> &rd_values) {
    using namespace std;
    bool is_3d;
    char filename[MAX_TITLE_LENGTH];
    m_counter++;
    is_3d = (rd_values.size() != 0);
    if ((fst_values.size() != snd_values.size()) ||
        (is_3d && rd_values.size() != fst_values.size())) {
      cerr << "Error: Data list length not equal!" << endl;
      return false;
    }
    if (is_3d) {
      sprintf(filename, "splot_%d", m_counter);
    } else {
      sprintf(filename, "plot_%d", m_counter);
    }
    if (!m_cfg.count(CFGATTR::DATAFILENAME)) {
      cerr << "[e] No data file name for the " << m_counter
           << ". plotl specified." << endl;
      return false;
    }

    auto buffer_path = bf::path(m_cfg[CFGATTR::DATAFILENAME]);
    ofstream out;
    out.open(buffer_path.string(), ios_base::out);

    for (unsigned int i = 0; i < fst_values.size(); i++) {
      out << fst_values[i] << "\t" << snd_values[i];
      if (is_3d) {
        out << "\t" << rd_values[i];
      }
      out << "\n";
    }
    if (out.is_open()) {
      out.close();
    }
    return (out.good() ? fst_values.size() : -1);
  }

private:
  ConfigMap m_cfg;
  ConfigLookupMap m_lookup;
  std::vector<std::string> m_raw_config;

  int m_counter = 0;
  bool m_init = false;
};

} // namespace vecthor
#endif
