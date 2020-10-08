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

#include <VecTHOR.h>

// Project includes
#include <Compressor.h>
#include <DynDecompressor.h>
#include <Emitter.h>
#include <FormalDecompressor.h>
#include <P2SBuffer.h>
#include <TDRGen.h>
#include <TDRReader.h>
#include <Utils.h>
#include <Validator.h>

// System includes
#include <fstream>
#include <iostream>
#include <string>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/timer.hpp>

using namespace std;
using namespace vecthor;
namespace bf = boost::filesystem;

namespace vecthor {

void VecTHOR::finalize() {
  if (m_config.isVerbose()) {
    cout << "[i] Finalizing program run." << endl;
  }
  m_compressor.release();
}

void VecTHOR::init() {
  m_run_name = "VecTHOR_";
  if (m_config.getProperty(CFG::GEN_COMPRESSED)) {
    if (m_config.getProperty(CFG::DYNAMIC) && m_config.getProperty(CFG::SAT)) {
      m_decompressor = DecompressorPtr(new FormalDecompressor(&m_config));
      m_run_name +=
          "formal_CDWs_" + to_string(m_config.getProperty(CFG::MAX_CDWS)) + "_";
    } else if (m_config.getProperty(CFG::DYNAMIC)) {
      m_decompressor = DecompressorPtr(new DynDecompressor(&m_config));
      m_run_name += "dyn_";
    } else {
      m_decompressor = DecompressorPtr(new Decompressor(&m_config));
      m_run_name += "static_";
    }
    m_compressor =
        CompressorPtr(new Compressor(&m_config, m_decompressor.get()));
  }
}

bool VecTHOR::prepare() {
  if (m_config.getProperty(CFG::USE_EXT_FILE) &&
      m_config.getProperty(CFG::HEX)) {
    auto filename = m_config.getFile(FILE::EXT_FILE);
    m_bit_vec = TDRReader::readHexTDR(filename);
    m_run_name += "FILE_" + to_string(m_bit_vec.size());
  } else if (m_config.getProperty(CFG::USE_EXT_FILE)) {
    auto filename = m_config.getFile(FILE::EXT_FILE);
    m_bit_vec = TDRReader::readTDR(filename);
    m_run_name += "FILE_" + to_string(m_bit_vec.size());
  } else {
    m_bit_vec = TDRGen::generateRTDR((unsigned)m_config.m_num_rtdr,
                                     m_config.getProperty(CFG::ALLOW_X));
    m_run_name += "RTDR_" + to_string(m_config.m_num_rtdr);
  }

  assert(m_bit_vec.size());
  if (m_config.isDebug()) {
    cout << "[d] Read or generated bit vec:" << endl;
    writeBitVec(m_bit_vec);
  }

  if (m_config.getProperty(CFG::GEN_GOLDEN)) {
    Emitter emitter(&m_config);
    emitter.writeGoldenFile(m_bit_vec);
  }

  if (m_config.getProperty(CFG::GEN_LEGACY)) {
    LegacyEmitter emitter(&m_config);
    emitter(m_bit_vec);
  }
  if (m_config.getProperty(CFG::GEN_COMPRESSED)) {
    m_emitter = CompressedEmitterPtr(
        new CompressedEmitter(&m_config, m_decompressor.get()));
  }
  if (m_config.getProperty(CFG::DYNAMIC)) {
    cout << "[i] Using dynamic Decompressor." << endl;
  }
  if (m_config.getProperty(CFG::PART_SIZE) > 0) {
    auto part_size = m_config.getProperty(CFG::PART_SIZE);
    cout << "[i] Using partitioning: " << part_size << endl;
    m_run_name += "_PART_" + to_string(part_size);
  }
  if (m_config.getProperty(CFG::VALIDATE)) {
    cout << "[i] Using software-based Validation." << endl;
    m_validator = ValidatorPtr(
        new Validator(&m_bit_vec, &m_config, m_decompressor.get()));
  }

  m_config.prepare(m_run_name);
  return run();
}

bool VecTHOR::run() {
  // Insert partitioning
  unsigned int part_size = m_config.getProperty(CFG::PART_SIZE);
  BitVecCItr vec_begin = m_bit_vec.cbegin();
  BitVecCItr vec_end = m_bit_vec.cend();
  bool last = false;
  int part = 1;
  do {
    boost::timer timer;
    if (part_size > 0) {
      // Enough uncovered elements left for full partition?
      int dist = distance(vec_begin, m_bit_vec.cend());
      if (dist <= part_size) { // Already reached end
        vec_end = m_bit_vec.cend();
        last = true;
      } else {
        vec_end = vec_begin;
        advance(
            vec_end,
            part_size); // vec_end must point to one elem after (like end-it)
      }
      std::cout << std::string(27, '&')
                << boost::format(
                       "\n[i] Using partitioning [%i:%i] : %i of %i\n") %
                       distance(m_bit_vec.cbegin(), vec_begin) %
                       distance(m_bit_vec.cbegin(), vec_end) % part %
                       ceil((float)m_bit_vec.size() / part_size)
                << std::string(27, '&') << std::endl;
    }
    m_compressor->prepare(vec_begin, vec_end); // Using refs
    //        m_emitter->getStats().clear();
    //      m_decompressor->dumpEntries();
    m_decompressor->determineCDW(vec_begin, vec_end);

    cout << "[i] Decompression time:\t" << timer.elapsed() << endl;

    if (m_config.getProperty(CFG::GEN_COMPRESSED)) {
      timer.restart();
      Route route = Route();

      if (m_config.getProperty(CFG::DYNAMIC) &&
          m_config.getProperty(CFG::SAT)) {
        cout << "[i] Extracting replacments from formal model" << endl;
        auto ptr = reinterpret_cast<FormalDecompressor *>(m_decompressor.get());
        Route repls;
        ptr->processModel(repls);
        if (m_config.getProperty(CFG::BENCHMARK)) {
          m_decompressor->getStats().collectBenchmarkData();
        }
        route = m_compressor->formal(repls);
      } else {
        m_compressor->greedy(route);
      }
      cout << "[i] Compression time:\t" << timer.elapsed() << endl;
      unsigned int delay = 0;
      if (m_config.getProperty(CFG::P2S_BUFFER)) {
        P2SBuffer m_buffer(&m_config, m_decompressor.get());
        if (m_config.isVerbose()) {
          std::cout << boost::format(
                           "[i] Determing buffer under- or overruns...")
                    << std::endl;
        }
        delay = m_buffer.processRoute(route, 2 * m_bit_vec.size());
        if (m_config.getProperty(CFG::STATS)) {
          m_emitter->getStats().printStats();
        }
        m_emitter->writeResyncFile(m_buffer.getCollector(), delay); // TODO
      }
      m_decompressor->dumpEntries();
      if (m_config.getProperty(CFG::BENCHMARK)) {
        m_compressor->getStats().collectBenchmarkData();
      }
      m_compressor->printStats("Benchmark");
      if (m_config.getProperty(CFG::VALIDATE)) {
        auto cdws = m_decompressor->extractUDW();
        m_validator->storeReplace(cdws);
        m_validator->storeChunk(route);
      }
      // std::cout << "Calling emitter" << std::endl;
      (*m_emitter)(route, delay); // TODO
    }
    vec_begin = vec_end;
    ++part;
    reset();
  } while (part_size > 0 && !last);
  if (m_config.getProperty(CFG::BENCHMARK)) {
    m_compressor->getStats().printBenchmarkData();
    m_decompressor->getStats().printBenchmarkData();
    m_emitter->getStats().printBenchmarkData();
  }
  return 1;
}

void VecTHOR::reset() {
  m_compressor->reset();
  m_decompressor->clear();
  m_decompressor->reset();
}

void VecTHOR::validate() {
  if (!m_validator->validate()) {
    throw std::runtime_error(
        boost::str(boost::format("[e] Software-based Validation failed!")));
  } else {
    std::cout << "[i] Software-based Validation passed!" << std::endl;
  }
}

} // namespace vecthor
// Namespace closed

int main(int argc, char **argv) {
  VecTHOR vecthor_i;
  if (!vecthor_i.getConfig().parseArgs(argc, argv)) {
    throw std::runtime_error(
        boost::str(boost::format("[e] Parsing call arguments failed!")));
  }
  if (!vecthor_i.getConfig().parseConfig()) {
    throw std::runtime_error(
        boost::str(boost::format("[e] Parsing config file failed!")));
  }

  if (vecthor_i.getConfig().getProperty(CFG::VERBOSE) || true) {
    vecthor_i.getConfig().dump();
  }

  if (vecthor_i.getConfig().getProperty(CFG::MAX_CDWS) > 12) {
    vecthor_i.getConfig().setProperty(
        CFG::EXT_CDWS, USE_EXT_CDWS); // TODO: Derive from 'NUM MAX_CDWS'
  }
  vecthor_i.init();

  if (!vecthor_i.prepare()) {
    throw std::runtime_error(boost::str(
        boost::format("[e] Something went wrong while running programm")));
  }
  if (vecthor_i.getConfig().getProperty(CFG::VALIDATE)) {
    vecthor_i.validate();
  }
  // vecthor_i.reset();

  vecthor_i.finalize();

  return 0;
}
