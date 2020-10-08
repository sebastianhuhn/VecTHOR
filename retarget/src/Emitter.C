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

#include <Emitter.h>

// Project includes
#include <Decompressor.h>
//#include <HardwareEmitter.h>
#include <TypeDefs.h>
#include <Utils.h>

// System includes
#include <algorithm>
#include <bitset>
#include <tuple>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace vecthor {
using namespace std;
namespace bf = boost::filesystem;
/*
 * Emitter
 */
void Emitter::writeGoldenFile(const BitVec &bit_vec) {
  auto golden_path = bf::path(m_cfg_ptr->getFile(FILE::GOLDEN_FILE));
  ofstream golden_file_str;
  golden_file_str.open(golden_path.string(), ios_base::app); // CHANGED

  if (m_cfg_ptr->isVerbose()) {
    cout << "[i] Writing golden file " << golden_path.string() << endl;
  }
  writeBitVec(bit_vec, &golden_file_str, false);
  if (golden_file_str.is_open()) {
    golden_file_str.close();
  }
}

/*
 * LegacyEmitter
 */
void LegacyEmitter::operator()(const BitVec &bit_vec) {
  auto legacy_path = bf::path(m_cfg_ptr->getFile(FILE::LEGACY_FILE));
  auto prefix_path = bf::path(m_cfg_ptr->getFile(FILE::LEGACY_PREFIX));
  auto suffix_path = bf::path(m_cfg_ptr->getFile(FILE::LEGACY_SUFFIX));
  auto verbose = m_cfg_ptr->isVerbose();
  if (verbose) {
    cout << "[i] Start generation of legacy JTAG " << legacy_path.string()
         << endl;
    cout << "[i] Appending prefix file " << prefix_path.string() << endl;
  }

  if (exists(prefix_path)) {
    bf::copy_file(prefix_path.string(), legacy_path.string(),
                  bf::copy_option::overwrite_if_exists);
  } else {
    throw std::runtime_error(
        boost::str(boost::format("[e] File '%s' does not exist!\n") %
                   prefix_path.string()));
  }
  ofstream legacy_file_str;
  legacy_file_str.open(legacy_path.string(), ios_base::app);

  writeJTAG(bit_vec, &legacy_file_str);

  if (verbose) {
    cout << "[i] Appending suffix file " << suffix_path.string() << endl;
  }
  if (exists(suffix_path)) {
    ifstream suffix_file_str;
    suffix_file_str.open(suffix_path.string());
    legacy_file_str << suffix_file_str.rdbuf();
    if (suffix_file_str.is_open()) {
      suffix_file_str.close();
    }
  } else {
    throw std::runtime_error(boost::str(
        boost::format("[e] File '%s' does not exist!") % suffix_path.string()));
  }
  if (legacy_file_str.is_open()) {
    legacy_file_str.close();
  }
  if (verbose) {
    cout << "[i] Generation of legacy JTAG file successfully done!" << endl;
  }
  m_stats.m_config_cycles = 0;
  m_stats.printStats("Legacy");
}

void LegacyEmitter::writeJTAG(const BitVec &bit_vec, ostream *stream) {
  if (!bit_vec.size()) {
    throw std::runtime_error(
        boost::str(boost::format("[e] Bit vec is still empty!")));
  }
  bool first = true;
  for (const bool bit : bit_vec) {
    if (first) {
      *stream << "\ttdi_i = #" << (2 * CYCLE_TIME) << " 1'b" << bit << ";"
              << endl;
      m_stats.m_cycles += 2;
      first = false;
    } else {
      *stream << "\ttdi_i = #" << CYCLE_TIME << " 1'b" << bit << ";" << endl;
    }
    ++m_stats.m_cycles;
  }
}

/*
 * CompressedEmitter
 */

CompressedEmitter::CompressedEmitter(const Config *config,
                                     const Decompressor *decomp_ptr)
    : Emitter(config, decomp_ptr) {
  init();
}

CompressedEmitter::~CompressedEmitter() { finalize(); }

void CompressedEmitter::init() {
  auto prefix_path = bf::path(m_cfg_ptr->getFile(FILE::COMPRESSED_PREFIX));
  auto compressed_path = bf::path(m_cfg_ptr->getFile(FILE::COMPRESSED_FILE));
  m_compr_file.open(compressed_path.string(), ios_base::app);

  if (m_cfg_ptr->isVerbose()) {
    cout << "[i] Start generation of compressed JTAG "
         << compressed_path.string() << endl;
    cout << "[i] Appending prefix file " << prefix_path.string() << endl;
  }

  if (exists(prefix_path)) {
    bf::copy_file(prefix_path.string(), compressed_path.string(),
                  bf::copy_option::overwrite_if_exists);
  } else {
    throw std::runtime_error(
        boost::str(boost::format("[e] File '%s' does not exist!\n") %
                   prefix_path.string()));
  }
}

void CompressedEmitter::finalize() {
  auto suffix_path = bf::path(m_cfg_ptr->getFile(FILE::COMPRESSED_SUFFIX));
  if (m_cfg_ptr->isVerbose()) {
    cout << "[i] Appending suffix file " << suffix_path.string() << endl;
  }
  if (exists(suffix_path)) {
    ifstream suffix_file_str;
    suffix_file_str.open(suffix_path.string());
    m_compr_file << suffix_file_str.rdbuf();
    if (suffix_file_str.is_open()) {
      suffix_file_str.close();
    }
  } else {
    throw std::runtime_error(boost::str(
        boost::format("[e] File '%s' does not exist!") % suffix_path.string()));
  }
  if (m_compr_file.is_open()) {
    m_compr_file.close();
  }
  if (m_cfg_ptr->isVerbose()) {
    cout << "[i] Generation of compressed JTAG file successfully done!" << endl;
  }
  m_stats.printStats("Compressed");

  if (m_cfg_ptr->getProperty(CFG::HW_EMIT)) { //TODO: To be merged with dev branch
    // HardwareEmitter hw_emitter(m_cfg_ptr, &m_signals);
    // hw_emitter();
  }
}

void CompressedEmitter::operator()(const Route &route, unsigned int delay) {

  if (m_cfg_ptr->getProperty(CFG::DYNAMIC)) {
    cout << boost::format(
                "[i] Writing preloading sequence with #CFG %i and delay %i ") %
                m_decomp_ptr->numTBCs() % delay
         << endl;
    writePreload(delay);
  }
  writeComprInstr();
  writeJTAG(route);
}

void CompressedEmitter::writeResyncFile(
    const P2SBuffer::DataCollector &data_collector, unsigned int delay) {
  auto resync_path = bf::path(m_cfg_ptr->getFile(FILE::RESYNC_FILE));
  ofstream resync_path_str;
  resync_path_str.open(resync_path.string());
  resync_path_str << string(delay, '-').c_str();
  unsigned last_index = 0;
  for (auto const &elem : data_collector) {
    resync_path_str << string((elem.first - last_index), '-').c_str()
                    << string(elem.second, 'D');
    last_index = elem.first;
  }
  if (resync_path_str.is_open()) {
    resync_path_str.close();
  }
  if (m_cfg_ptr->isVerbose()) {
    cout << "[i] Generation of resyned data file successfully done!" << endl;
  }
}

void CompressedEmitter::writeJTAG(const Route &route) {
  if (!route.size()) {
    throw std::runtime_error(
        boost::str(boost::format("[e] Route is still empty!")));
  }
  auto tdi_state = VALUE::INIT;
  for (auto route_it = route.begin(); route_it != route.end(); route_it++) {
    bool first = true;

    auto cdw = get<0>(**route_it);
    string cdw_str = m_decomp_ptr->CDWtoEncoding(cdw);
    reverse(cdw_str.begin(), cdw_str.end());

    for (const char &c : cdw_str) {
      if (first) {
        m_compr_file << "\n\t// [COMPR_DR] " << cdw_str << endl;
        ++m_stats.m_compr_dr;
        m_compr_file << "\t#" << CYCLE_TIME << ";" << endl;
        m_compr_file << "\ttms_i = 1'b0;" << endl;
        m_compr_file << "\ttdi_i = 1'b" << c << ";" << endl;
        addSignalValue(c, '0'); // HW Emitter
        ++m_stats.m_cycles;
        first = false;
      } else {
        m_compr_file << "\ttdi_i = #" << CYCLE_TIME << " 1'b" << c << ";"
                     << endl;
        addSignalValue(c, '2'); // HW Emitter
        ++m_stats.m_cycles;
      }
      tdi_state = getValue(c);
    }
    // Handling repitition
    auto succ_it = route_it + 1;
    auto last_elem = (succ_it == route.end());
    if (m_cfg_ptr->getProperty(CFG::MERGING) && Decompressor::isEmptyCDW(cdw)) {
      ++m_stats.m_compre_repeat;
      m_compr_file << "\n\t// [COMPR_REPEAT]" << endl;
      m_compr_file << "\t#" << CYCLE_TIME << ";" << endl;
      addSignalValue('2', '2'); // HW Emitter
      ++m_stats.m_cycles;

      if (!last_elem && Decompressor::isEmptyCDW(get<0>(**succ_it))) {
        ++m_stats.m_multi_rep;
        continue;
      }
      continue;
    }
    // Finalize CDW by COMPR_EXIT
    ++m_stats.m_compr_exit;
    m_compr_file << "\n\t// [COMPR_EXIT]" << endl;
    m_compr_file << "\t#" << CYCLE_TIME << ";" << endl;
    m_compr_file << "\ttms_i = 1'b1;" << endl;
    addSignalValue('2', '1'); // HW Emitter
    if (m_cfg_ptr->getProperty(CFG::MERGING) && last_elem) {
      m_compr_file << "\n\t// [Update_DR]" << endl;
      m_compr_file << "\t#" << CYCLE_TIME << ";"
                   << endl; // Already counted as cycle
      if (tdi_state == VALUE::LOW) {
        m_compr_file << "\ttdi_i = 1'b1;" << endl;
        addSignalValue('1', '2'); // HW Emitter
      } else {
        m_compr_file << "\ttdi_i = 1'b0;" << endl;
        addSignalValue('0', '2'); // HW Emitter
      }
    }
    ++m_stats.m_cycles;
  }
}

void CompressedEmitter::writePreload(unsigned int delay) {
  if ((m_decomp_ptr->numTBCs() > 0) || (delay > 0)) {
    //        m_compr_file << "\n\t// -> [Select_DR]" << endl;
    //        m_compr_file << "\ttms_i = 1'b1;" << endl;
    //        m_compr_file << "\n\t// -> [Cap_DR]" << endl;
    //        m_compr_file << "\ttms_i =  #" << CYCLE_TIME << " 1'b0;" << endl;
    writePreloadInstr();
  }
#ifdef BUFFER_CTR_SIZE
  m_compr_file << "\n// Shifting delay data: " << delay << endl;
  std::bitset<BUFFER_CTR_SIZE> delay_binary(delay);
  int idx_end = delay_binary.size() - 1;
  for (int idx = idx_end; idx >= 0; --idx) {
    if (idx == idx_end) {
      m_compr_file << "\n\t// [CFG_DR]" << endl;
      m_compr_file << "\t#" << CYCLE_TIME << ";" << endl;
      ++m_stats.m_config_cycles;
      m_compr_file << "\ttms_i = 1'b0;" << endl;
      m_compr_file << "\ttdi_i = 1'b" << delay_binary[idx] << ";" << endl;
      cout << "DELAY" << (int)delay_binary[idx] << endl;
      //   addSignalValue( char( delay_binary[idx] ) , '0' );   // HW Emitter
      //   TODO: To be merged
    } else {
      m_compr_file << "\ttdi_i = #" << CYCLE_TIME << " 1'b" << delay_binary[idx]
                   << ";" << endl;
      //   addSignalValue( (int) delay_binary[idx] , '2' );   // HW Emitter
      ++m_stats.m_config_cycles;
    }
  }
  m_compr_file << "\n\t// [COMPR_EXIT]" << endl;
  m_compr_file << "\t#" << CYCLE_TIME << ";" << endl;
  ++m_stats.m_config_cycles;
  m_compr_file << "\ttms_i = 1'b1;" << endl;
  addSignalValue('2', '1'); // HW Emitter
#endif

  m_compr_file << "\n// Shifting configuration data" << endl;
  for (auto elem : m_decomp_ptr->getTBCs()) {
    std::string config = string(elem);
    // std::cout << boost::format("Processing %s with length %i") % config %
    // config.length() << std::endl;
    assert(config.length() == 4 || config.length() == 8);
    if (config.length() == 4) {
      config += '0';
    } else {
      config.insert(4, "1");
    }
    ++m_stats.m_config_cycles;
    for (auto rit = config.crbegin(); rit != config.crend(); ++rit) {
      if (rit == config.crbegin()) {
        m_compr_file << "\n\t// [CFG_DR]" << endl;
        m_compr_file << "\t#" << CYCLE_TIME << ";" << endl;
        ++m_stats.m_config_cycles;
        m_compr_file << "\ttms_i = 1'b0;" << endl;
        m_compr_file << "\ttdi_i = 1'b" << *rit << ";" << endl;
        addSignalValue(*rit, '0'); // HW Emitter
      } else {
        m_compr_file << "\ttdi_i = #" << CYCLE_TIME << " 1'b" << *rit << ";"
                     << endl;
        addSignalValue(*rit, '2'); // HW Emitter
        ++m_stats.m_config_cycles;
      }
    }
    m_compr_file << "\n\t// [COMPR_EXIT]" << endl;
    m_compr_file << "\t#" << CYCLE_TIME << ";" << endl;
    ++m_stats.m_config_cycles;
    m_compr_file << "\ttms_i = 1'b1;" << endl;
    addSignalValue('2', '1');
  }

  if (m_decomp_ptr->numTBCs() > 0) {
    m_compr_file << "\n\t// -> [Update_DR]" << endl;
    m_compr_file << "\ttms_i =  #" << CYCLE_TIME << " 1'b1;" << endl;
    addSignalValue('2', '1');
  }
  m_compr_file << "\n// ... configuration data ends.\n" << endl;
}

void CompressedEmitter::writeComprInstr() {
  auto infix_path = bf::path(m_cfg_ptr->getFile(FILE::DYNCOMPRESSED_INFIX));
  if (m_cfg_ptr->isVerbose()) {
    cout << "[i] Appending infix file " << infix_path.string() << endl;
  }
  ifstream infix_file_str;
  infix_file_str.open(infix_path.string());
  m_compr_file << infix_file_str.rdbuf();
  if (infix_file_str.is_open()) {
    infix_file_str.close();
  }
}

void CompressedEmitter::writePreloadInstr() {
  auto preload_path = bf::path(m_cfg_ptr->getFile(FILE::DYNCOMPRESSED_PRELOAD));
  if (m_cfg_ptr->isVerbose()) {
    cout << "[i] Appending preload file " << preload_path.string() << endl;
  }
  ifstream preload_file_str;
  preload_file_str.open(preload_path.string());
  m_compr_file << preload_file_str.rdbuf();
  if (preload_file_str.is_open()) {
    preload_file_str.close();
  }
}

void CompressedEmitter::addSignalValue(const char &a, const char &b) {
  m_signals.push_back(std::make_tuple(getValue(a), getValue(b)));
}

VALUE Emitter::getValue(const char &c) {
  if (c == '0') {
    return VALUE::LOW;
  } else if (c == '1') {
    return VALUE::HIGH;
  } else if (c == '2') {
    return VALUE::NOP;
  }

  throw std::runtime_error(boost::str(
      boost::format(
          "[e] Something went wrong while conversion of '%s' to value type!") %
      c));
  return VALUE::UNSUPPORTED;
}
} // namespace vecthor
