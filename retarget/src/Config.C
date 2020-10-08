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

#include <Config.h>

// Project includes
#include <TypeDefs.h>

// System includes
#include <iostream>
#include <yaml-cpp/yaml.h>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

namespace vecthor {
using namespace std;
namespace bf = boost::filesystem;
namespace po = boost::program_options;

Config::Config() : m_cfg_map(), m_file_map() {
  m_cfg_str = {{CFG::MERGING, "merging_repititions"},
               {CFG::DYNAMIC, "dynamic"},
               {CFG::HEUR_INNER_FREQ, "heur_inner_freq"},
               {CFG::HEUR_OUTER_FREQ, "heur_outer_freq"},
               {CFG::HEUR_WEIGHT, "heur_weight"},
               {CFG::HEUR_PERMUTE, "heur_permute"},
               {CFG::SAT, "SAT"},
               {CFG::SAT_SEC, "SAT_SEC"},
               {CFG::SAT_CONFL, "SAT_CONFL"},
               {CFG::SAT_RESTART, "SAT_RESTART"},
               {CFG::MAX_CDWS, "max_cdws"},
               {CFG::PART_SIZE, "part_size"},
               {CFG::VERBOSE, "verbose"},
               {CFG::DEBUG, "debug"},
               {CFG::STATS, "stats"},
               {CFG::BENCHMARK, "benchmark"},
               {CFG::PLOT, "plot"},
               {CFG::HEX, "hex"},
               {CFG::P2S_BUFFER, "P2S_BUFFER"},
               {CFG::USE_CONF_FILE, "use_conf_file"},
               {CFG::USE_EXT_FILE, "use_ext_file"},
               {CFG::GEN_LEGACY, "gen_legacy"},
               {CFG::GEN_COMPRESSED, "gen_compressed"},
               {CFG::GEN_GOLDEN, "gen_golden"},
               {CFG::HW_EMIT, "hw_emit"},
               {CFG::ALLOW_X, "allow_x"},
               {CFG::VALIDATE, "validate"}};
  m_file_str = {{FILE::LEGACY_PREFIX, "legacy_prefix"},
                {FILE::LEGACY_SUFFIX, "legacy_suffix"},
                {FILE::DYNCOMPRESSED_PRELOAD, "dyncompressed_preload"},
                {FILE::DYNCOMPRESSED_INFIX, "dyncompressed_infix"},
                {FILE::COMPRESSED_PREFIX, "compressed_prefix"},
                {FILE::COMPRESSED_SUFFIX, "compressed_suffix"},
                {FILE::COMPRESSED_FILE, "compressed_file"},
                {FILE::CONFIG_FILE, "config_file"},
                {FILE::RESYNC_FILE, "resync_file"},
                {FILE::EXT_FILE, "ext_file"},
                {FILE::LEGACY_FILE, "legacy_file"},
                {FILE::GOLDEN_FILE, "golden_file"},
                {FILE::VALIDATION_FILE, "validation_file"}};
  initialize();
}

Config::~Config() {
  if (m_benchmark_filep && m_benchmark_filep->is_open()) {
    m_benchmark_filep->close();
  }
}

string Config::CFGtoString(CFG cfg) const {
  if (m_cfg_str.find(cfg) == m_cfg_str.end()) {
    return "";
  }
  return m_cfg_str.at(cfg);
}

void Config::initialize() {
  m_file_map[FILE::EXT_FILE] = "tdr_data.pattern";
  m_file_map[FILE::LEGACY_FILE] = "legacy_jtag.out";
  m_file_map[FILE::COMPRESSED_FILE] = "compressed_jtag.out";
  m_file_map[FILE::GOLDEN_FILE] = "tdr_data.golden";
  m_file_map[FILE::VALIDATION_FILE] = "tdr_data.valid";
  m_file_map[FILE::CONFIG_FILE] = "default.conf";
  m_file_map[FILE::RESYNC_FILE] = "resync.data";
}

CFG Config::getCFGType(string &property) const {
  CFG retType;
  auto predicate = [&property, &retType](CfgStringMap::value_type entry) {
    retType = entry.first;
    return entry.second == property;
  };
  if (!std::any_of(m_cfg_str.begin(), m_cfg_str.end(), predicate)) {
    return CFG::UNSUPPORTED;
  }
  return retType;
}

FILE Config::getFILEType(string &property) const {
  FILE retType;
  auto predicate = [&property, &retType](FileStringMap::value_type entry) {
    retType = entry.first;
    return entry.second == property;
  };
  if (!std::any_of(m_file_str.begin(), m_file_str.end(), predicate)) {
    return FILE::UNSUPPORTED;
  }
  return retType;
}

Config::OfStrPtr Config::getBenchmarkFile() const {
  assert(m_benchmark_filep->is_open());
  return m_benchmark_filep;
}

unsigned int Config::getProperty(CFG cfg) const {
  if (m_cfg_map.find(cfg) == m_cfg_map.end()) {
    return false;
  }
  //    std::istringstream is(m_cfg_map.at(cfg));
  //    bool retVal;
  //    is >> std::boolalpha >> retVal;
  return m_cfg_map.at(cfg);
}

const string &Config::getFile(FILE file) const {
  if (m_file_map.find(file) == m_file_map.end()) {
    throw std::runtime_error(
        boost::str(boost::format("[e] Unknown file type requested!")));
  }
  return m_file_map.at(file);
}

void Config::setProperty(CFG cfg, int value) { m_cfg_map[cfg] = value; }

void Config::setFile(FILE file, string &filename) {
  m_file_map[file] = filename;
}

// Parsing command line parameters
bool Config::parseArgs(int argc, char **argv) {
  po::options_description options("Allowed options");
  string config_file, ext_file, legacy_file, compressed_file, golden_file;
  // clang-format off
    options.add_options()
          ( "Help", "produces help message" )
          ( "Verbose", "be verbose" )
          ( "Debug", "prints lots of debug info" )
          ( "Stats", "prints stats" )
          ( "Plot", "generates plots" )
          ( "Hex", "processes data as hex" )
          ( "ConfigFile", po::value< string >( &config_file ), "reads configuration file" )
          ( "ReadTDR", po::value< string >( &ext_file ), "reads external TDR data file" )
          ( "NumRTDR", po::value< unsigned int > ( &m_num_rtdr ), "number of bytes to be generated (random)" )
          ( "LegacyJTAG", po::value< string >( &legacy_file ), "generates legacy JTAG sequence" )
          ( "CompressedJTAG", po::value< string >( &compressed_file ), "generates compressed JTAG sequence" )
          ( "WriteGolden", po::value< string >( &golden_file ), "write golden file for comparison" )
        ;
  // clang-format on
  po::variables_map vmap;
  auto parser = po::basic_command_line_parser<char>(argc, argv);
  auto parsed_cmds = parser.options(options).allow_unregistered().run();
  po::store(parsed_cmds, vmap);
  po::notify(vmap);

  if (!po::collect_unrecognized(parsed_cmds.options, po::exclude_positional)
           .empty() ||
      vmap.count("Help")) {
    printIcon();
    cout << "[i]" << options << endl;
    return 0;
  }
  if (vmap.count("Verbose")) {
    cout << "[i] Enabling verbose mode." << endl;
    setProperty(CFG::VERBOSE);
  }
  if (vmap.count("Debug")) {
    cout << "[i] Enabling debug mode." << endl;
    setProperty(CFG::DEBUG);
  }
  if (vmap.count("Stats")) {
    cout << "[i] Enabling stat output." << endl;
    setProperty(CFG::STATS);
  }
  if (vmap.count("Plot")) {
    cout << "[i] Enabling plot generation." << endl;
    setProperty(CFG::PLOT);
  }
  if (vmap.count("Hex")) {
    cout << "[i] Enabling hex data processing." << endl;
    setProperty(CFG::HEX);
  }
  if (vmap.count("ConfigFile")) {
    if (isVerbose()) {
      cout << boost::format("[i] Using config file '%s'.") % config_file
           << endl;
    }
    setProperty(CFG::USE_CONF_FILE);
    setFile(FILE::CONFIG_FILE, config_file);
  }
  if (vmap.count("ReadTDR")) {
    cout << "[i] Reading external TDR file: " << ext_file << endl;
    setProperty(CFG::USE_EXT_FILE);
    setFile(FILE::EXT_FILE, ext_file);
  }
  if (vmap.count("NumRTDR") && isVerbose()) {
    cout << boost::format("[i] Setting number of bit to be generated to %i.") %
                m_num_rtdr
         << endl;
  }
  if (vmap.count("LegacyJTAG")) {
    if (isVerbose()) {
      cout << boost::format("[i] Legacy JTAG file '%s' will be generated.") %
                  legacy_file
           << endl;
    }
    setProperty(CFG::GEN_LEGACY);
    setFile(FILE::LEGACY_FILE, legacy_file);
  }
  if (vmap.count("CompressedJTAG")) {
    if (isVerbose()) {
      cout << boost::format(
                  "[i] Compressed JTAG file '%s' will be generated.") %
                  compressed_file
           << endl;
    }
    setProperty(CFG::GEN_COMPRESSED);
    setFile(FILE::COMPRESSED_FILE, compressed_file);
  }
  if (vmap.count("WriteGolden")) {
    if (isVerbose()) {
      cout << boost::format("[i] Golden file '%s' will be generated.") %
                  golden_file
           << endl;
    }
    setProperty(CFG::GEN_GOLDEN);
    setFile(FILE::GOLDEN_FILE, golden_file);
  }
  return 1;
}

bool Config::parseConfig() {
  auto config_file = bf::path(getFile(FILE::CONFIG_FILE));
  assert(bf::exists(config_file));
  YAML::Node config = YAML::LoadFile(config_file.string());
  assert(config.Type() == YAML::NodeType::Map);
  if (auto const &vecthor_main = config["vecthor"]) {
    for (auto i = vecthor_main.begin(); i != vecthor_main.end(); i++) {
      auto key = i->first.as<string>();
      CFG cfgType;
      FILE fileType;
      if ((cfgType = getCFGType(key)) != CFG::UNSUPPORTED) {
        try {
          auto value = i->second.as<bool>();
          setProperty(cfgType, value);
        } catch (YAML::BadConversion &excep) {
          auto value = i->second.as<int>();
          setProperty(cfgType, value);
        }
      } else if ((fileType = getFILEType(key)) != FILE::UNSUPPORTED) {
        auto value = i->second.as<string>();
        setFile(fileType, value);
      }
    }
  }

  //      if ( auto scheduler = config ["scheduler"] ) {
  //        if ( scheduler.Type() != YAML::NodeType::Sequence ) {
  //          std::cerr << "[w] Expected map definitions in the scheduler
  //          section." << std::endl;
  //        }
  //        else {
  //          m_scheduler_config = scheduler_config_ptr (new
  //          scheduler_config());

  //          for ( auto index = 0u; index < scheduler.size(); ++index) {
  //            auto entry = scheduler[index];
  //            assert ( entry.Type() == YAML::NodeType::Map );

  //            for ( auto iter = entry.begin(); iter != entry.end(); ++iter ) {
  //              auto key = iter->first.as < std::string >();
  //              auto content = iter->second;

  //              auto &step = m_scheduler_config->add_step();
  //              load_scheduler_config( key, content, step );
  //            }
  //          }
  //        }
  //      }
  //    }
  return true;
}

void Config::prepare(const std::string &run_name) {
  if (getProperty(CFG::BENCHMARK)) {
    auto benchmark_size_path = bf::path(run_name + ".benchmark");
    m_benchmark_filep = shared_ptr<ofstream>(new ofstream());
    m_benchmark_filep->open(benchmark_size_path.string(), ios_base::out);
  }
}

void Config::dump() {
  cout << "[DUMP] Dumping Config:" << endl;
  for (auto const &entry : m_cfg_map) {
    std::cout << boost::format("[v] Option: '%s' = '%i' ") %
                     (m_cfg_str[entry.first]) % entry.second
              << std::endl;
  }
  for (auto const &entry : m_file_map) {
    std::cout << boost::format("[v] File: '%s' = '%i' ") %
                     (m_file_str[entry.first]) % entry.second
              << std::endl;
  }
  cout << "[DUMP] ... ends!" << endl;
}

void Config::printIcon() { std::cout << "" << std::endl; }
} // namespace vecthor
