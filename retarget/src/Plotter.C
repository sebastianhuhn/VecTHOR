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

#include <Plotter.h>

// System includes

// Boost includes
#include <boost/format.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>

namespace vecthor {

using namespace std;

Plotter::Plotter() : m_cfg() {
  m_lookup = {{CFGATTR::LABEL, "set label"},
              {CFGATTR::LABELPOS, " at "},
              {CFGATTR::DESCRIPTION, "set label 2"},
              {CFGATTR::DESCRIPTIONPOS, " at "},
              {CFGATTR::XLABEL, "set xlabel"},
              {CFGATTR::YLABEL, "set ylabel"},
              {CFGATTR::ZLABEL, "set zlabel"},
              {CFGATTR::XTICS, "set xtics"},
              {CFGATTR::YTICS, "set ytics"},
              {CFGATTR::ZTICS, "set ztics"},
              {CFGATTR::XRANGE, "set xrange"},
              {CFGATTR::YRANGE, "set yrange"},
              {CFGATTR::ZRANGE, "set zrange"},
              {CFGATTR::TITLE, "set title"},
              {CFGATTR::GRID, "set grid"},
              {CFGATTR::AZIMUT, "set view"},
              {CFGATTR::ELEVATION, ","},
              {CFGATTR::TERMINAL, "set terminal"},
              {CFGATTR::OUTPUT, "set output"},
              {CFGATTR::SIZE, " size "},
              {CFGATTR::FONT, " font "},
              {CFGATTR::FONTSIZE, ","},
              {CFGATTR::STYLEFILL, "set style fill"},
              {CFGATTR::STYLEDATA, "set style data"},
              {CFGATTR::USING, " using "},
              {CFGATTR::EVERY, " every "},
              {CFGATTR::SYMBOL, " with "},
              {CFGATTR::PLOT, "plot"},
              {CFGATTR::SPLOT, "splot"},
              {CFGATTR::GRAPHTITLE, " title "}};
  initConfig();
}

int Plotter::writeConfig(ConfigMap &user_settings) {
  if (!m_init) {
    throw std::runtime_error(
        boost::str(boost::format("[e] Initialization is incomplete!")));
  }
  if (!user_settings.empty()) {
    for (auto &entry : user_settings) {
      m_cfg[entry.first] = entry.second;
    }
  }
  auto buffer_path = bf::path(m_cfg[CFGATTR::NAME]);
  ofstream out;
  out.open(buffer_path.string(), ios_base::out);

  generatePlotCfg(out);

  if (out.is_open()) {
    out.close();
  }
  return (out.good()) ? m_cfg.size() : -1;
}

string Plotter::getAttribute(Plotter::CFGATTR attr) const {
  if (isEmpty(attr)) {
    return "";
  }
  auto it = m_lookup.find(attr);
  if (it == m_lookup.end()) {
    throw std::runtime_error(
        boost::str(boost::format("[e] Unsupported plotter option '%i' !") %
                   (unsigned int)attr));
  }
  return it->second;
}

void Plotter::generatePlotCfg(ofstream &out) const {
  for (auto const &cfg : m_raw_config) {
    out << cfg << endl;
  }
  for (auto entry : m_cfg) {
    auto attr = entry.first;
    if (isSkippable(attr)) {
      continue;
    }
    string str;
    switch (attr) {
    case CFGATTR::DESCRIPTION: {
      bool extra_opt = m_cfg.count(CFGATTR::DESCRIPTIONPOS);
      str = boost::str(
          boost::format("%s \"%s\" %s %s") % getAttribute(attr) % entry.second %
          (extra_opt ? getAttribute(CFGATTR::DESCRIPTIONPOS) : "") %
          (extra_opt ? "" : ""));
      break;
    }
    case CFGATTR::LABEL: {
      bool extra_opt = m_cfg.count(CFGATTR::LABELPOS);
      str = boost::str(boost::format("%s \"%s\" %s %s") % getAttribute(attr) %
                       entry.second %
                       (extra_opt ? getAttribute(CFGATTR::LABELPOS) : "") %
                       (extra_opt ? "" : ""));
      break;
    }
    case CFGATTR::AZIMUT: {
      bool extra_opt = m_cfg.count(CFGATTR::ELEVATION);
      str = boost::str(boost::format("%s %s %s %s") % getAttribute(attr) %
                       entry.second %
                       (extra_opt ? getAttribute(CFGATTR::ELEVATION) : "") %
                       (extra_opt ? "" : ""));
      break;
    }
    case CFGATTR::PLOT:
    case CFGATTR::SPLOT:
      generatePlot(out);
      break;
    case CFGATTR::FONT: {
      bool extra_opt = m_cfg.count(CFGATTR::FONTSIZE);
      str = boost::str(boost::format("%s \"%s\" %s %s") % getAttribute(attr) %
                       entry.second %
                       (extra_opt ? getAttribute(CFGATTR::FONTSIZE) : "") %
                       (extra_opt ? m_cfg.at(CFGATTR::FONTSIZE) : ""));
      break;
    }
    default:
      str = boost::str(
          boost::format("%s %s") % getAttribute(attr) %
          (isQuoted(attr) ? "\"" + entry.second + "\"" : entry.second));
      break;
    }
    out << str << endl;
  }
}

void Plotter::generatePlot(ofstream &out) const {
  string str;
  if (m_cfg.count(CFGATTR::PLOT)) {
    str = boost::str(boost::format("%s \"%s\"") % getAttribute(CFGATTR::PLOT) %
                     m_cfg.at(CFGATTR::PLOT));
  } else {
    str = boost::str(boost::format("%s \"%s\"") % getAttribute(CFGATTR::SPLOT) %
                     m_cfg.at(CFGATTR::SPLOT));
  }
  if (m_cfg.count(CFGATTR::EVERY)) {
    str += boost::str(boost::format("%s %s") % getAttribute(CFGATTR::EVERY) %
                      m_cfg.at(CFGATTR::EVERY));
  }

  if (m_cfg.count(CFGATTR::USING)) {
    str += boost::str(boost::format("%s %s") % getAttribute(CFGATTR::USING) %
                      m_cfg.at(CFGATTR::USING));
  }

  if (m_cfg.count(CFGATTR::SYMBOL)) {
    str += boost::str(boost::format("%s %s") % getAttribute(CFGATTR::SYMBOL) %
                      m_cfg.at(CFGATTR::SYMBOL));
  }

  if (m_cfg.count(CFGATTR::GRAPHTITLE)) {
    str += boost::str(boost::format("%s \"%s\"") %
                      getAttribute(CFGATTR::GRAPHTITLE) %
                      m_cfg.at(CFGATTR::GRAPHTITLE));
  } else {
    str += " notit";
  }
  out << str;
}

// ----------------------

bool Plotter::isSkippable(Plotter::CFGATTR attr) const {
  switch (attr) {
  case CFGATTR::NAME:
  case CFGATTR::DATAFILENAME:
  case CFGATTR::FONTSIZE:
  case CFGATTR::LABELPOS:
  case CFGATTR::DESCRIPTIONPOS:
  case CFGATTR::EVERY:
  case CFGATTR::USING:
  case CFGATTR::SIZE:
  case CFGATTR::SYMBOL:
  case CFGATTR::GRAPHTITLE:
    return true;
  default:
    return false;
  }
}

bool Plotter::isEmpty(Plotter::CFGATTR attr) const {
  switch (attr) {
  case CFGATTR::NAME:
    return true;
  default:
    return false;
  }
}

bool Plotter::isQuoted(CFGATTR attr) const {
  switch (attr) {
  case CFGATTR::TITLE:
  case CFGATTR::DESCRIPTION:
  case CFGATTR::OUTPUT:
  case CFGATTR::FONT:
  case CFGATTR::GRID:
  case CFGATTR::LABEL:
  case CFGATTR::XLABEL:
  case CFGATTR::YLABEL:
  case CFGATTR::ZLABEL:
    return true;
  default:
    return false;
  }
}

// ----------------------

void Plotter::initConfig() {
  m_cfg[CFGATTR::DATAFILENAME] = "data.txt";
  m_cfg[CFGATTR::NAME] = "default_plot.gpl";
  m_cfg[CFGATTR::TERMINAL] = "png";
  m_cfg[CFGATTR::SIZE] = "800,600";
  m_cfg[CFGATTR::OUTPUT] = "default_plot.png";
}

template <> void Plotter::initTypeConfig<Plotter::ScatterPlot>() {
  m_cfg[CFGATTR::PLOT] = "plot.txt";
  m_cfg[CFGATTR::USING] = "1:2";
  m_init = true;
}

template <> void Plotter::initTypeConfig<Plotter::HistoPlot>() {
  m_cfg[CFGATTR::STYLEDATA] = "histogram";
  m_cfg[CFGATTR::STYLEFILL] = "solid border -1";
  m_cfg[CFGATTR::PLOT] = "plot.txt";
  // m_cfg["every_1"] = "1::0";
  m_cfg[CFGATTR::USING] = "1:xtic(2)";
  m_init = true;
  m_raw_config.push_back("set style histogram cluster gap 2");
  m_raw_config.push_back("set grid y");
  m_raw_config.push_back(
      "set terminal postscript landscape enhanced color font 'Helvetica,10'");
  m_raw_config.push_back(
      "set border 3 front lt black linewidth 1.000 dashtype solid");
}

template <> void Plotter::initTypeConfig<Plotter::Plot3D>() {
  m_cfg[CFGATTR::SPLOT] = "splot.txt";
  m_cfg[CFGATTR::USING] = "1:2:3";
  m_init = true;
}
} // namespace vecthor
