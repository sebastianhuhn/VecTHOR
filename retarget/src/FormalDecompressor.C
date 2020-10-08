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

#include <FormalDecompressor.h>

// Project includes
#include <Utils.h>

// System includes
#include <iostream>
#include <limits>
#include <math.h>
#include <set>
#include <tuple>

// Boost includes
#include <boost/format.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>

// Clasp includes
#include "FormalDecompressor.h"
#include "clasp/clause.h"
#include "clasp/heuristics.h"
#include "clasp/weight_constraint.h"

namespace vecthor {

using namespace std;

FormalDecompressor::FormalDecompressor(const Config *cfg_ptr)
    : Decompressor(cfg_ptr), m_ctx(new Clasp::SharedContext()),
      m_heu(new Clasp::ClaspVmtf(8)),
      m_enum(new Clasp::ModelEnumerator(
          Clasp::ModelEnumerator::Strategy::strategy_backtrack)),
      m_params(new Clasp::SolveParams()), m_limits(new Clasp::SolveLimits()),
      m_solver(m_ctx->master()), m_sdata(nullptr), m_constr(nullptr),
      m_stats(m_cfg_ptr) {
  m_solver->setHeuristic(m_heu.get());
  m_act_var1 = shared_ptr<Clasp::Var>(
      new Clasp::Var(m_ctx->addVar(Clasp::Var_t::atom_var)));
  m_act_var2 = shared_ptr<Clasp::Var>(
      new Clasp::Var(m_ctx->addVar(Clasp::Var_t::atom_var)));
  m_act_var3 = shared_ptr<Clasp::Var>(
      new Clasp::Var(m_ctx->addVar(Clasp::Var_t::atom_var)));
}

void FormalDecompressor::determineCDW(BitVecCItr &bv_begin,
                                      BitVecCItr &bv_end) {
  m_bit_vec_begin = bv_begin;
  m_bit_vec_end = bv_end;
  initSolver();
  solve();
}

bool FormalDecompressor::addClause(Clasp::LitVec &lits) {
  m_ctx->startAddConstraints();
  Clasp::ClauseCreator cls(m_solver);
  cls.start(Clasp::ConstraintType::static_constraint);
  for (auto const &elem : lits) {
    cls.add(elem);
  }
  Clasp::ClauseCreator::Result cls_res = cls.end();
  return cls_res.ok();
}

void FormalDecompressor::prepareBitVec(LengthConfig &len_cfg) {
  processSBIs();
  for (auto &elem : len_cfg) {
    auto length = elem.first;
    auto &cdw_map = *elem.second;
    for (auto begin_it = m_bit_vec_begin;
         (begin_it != m_bit_vec_end && begin_it + length <= m_bit_vec_end);
         ++begin_it) {
      buildOverlappings(begin_it, begin_it + length, length, cdw_map);
    }
  }
  processOverlappings();
  //    len_cfg.push_back( make_pair(1, nullptr) );
  for (auto &elem : len_cfg) {
    processMerges(elem.first);
  }
  processSBIMerges();
  cout << "[i] SAT instance is built!" << endl;
}

void FormalDecompressor::processBinary() {
  for (auto const &elem : m_bin_clauses) {
    m_ctx->addBinary(elem.first, elem.second);
  }
  assert(m_ctx->numBinary() == m_bin_clauses.size());
}

void FormalDecompressor::processSBIs() {
  size_t size = distance(m_bit_vec_begin, m_bit_vec_end);
  for (unsigned int i = 0; i < size; ++i) {
    Clasp::Var sbi_var = m_ctx->addVar(Clasp::Var_t::atom_var);
    m_sbi_map[i] = sbi_var;
    m_overlap_map[i].push_back(sbi_var);
    Clasp::Literal sbi_lit = Clasp::Literal(sbi_var, false);
    m_weighted_codeword_lits.push_back(Clasp::WeightLiteral(sbi_lit, 3));
    m_weighted_sbi_lits.push_back(Clasp::WeightLiteral(sbi_lit, 1));
  }
}

void FormalDecompressor::extractModelUDWValue(
    const VarUDWMap::value_type &elem) {
  boost::fusion::for_each(elem.second, [&](const Clasp::Var &udw_var) {
    if (valToBool(m_var_model[udw_var].first)) {
      ++m_stats.m_det_cdws;
      if (isStaticCDW(getCDW(elem.first),
                      (m_cfg_ptr->getProperty(CFG::MAX_CDWS) > 8))) {
        ++m_stats.m_det_static_cdws;
      }
      storeDynCDW(elem.first);
    }
  });
}

void FormalDecompressor::processModel(Route &repl_vec) {
  // Var ->
  for (auto const &udw_tuple : m_udw_map) {
    extractModelUDWValue(udw_tuple);
  }
  for (auto const &udw_tuple : m_byteudw_map) {
    extractModelUDWValue(udw_tuple);
  }
  m_stats.m_config_bit = lengthTBCs();
  VarIndexMap::left_const_iterator it = m_idx_map.left.begin();
  for (; it != m_idx_map.left.end(); ++it) {
    auto &var = it->first;
    auto &it_pair = it->second;
    if (valToBool(m_var_model[var].first)) {
      auto start_it = it_pair.first;
      auto end_it = it_pair.second;
      auto bit_vec = serializeBitVec(start_it, end_it);
      CDW cdw = getCDW(bit_vec);
      assert(isValidCDW(cdw));
      auto benefit = getCDWBenefit(cdw);
      ReplacementPtr repl = std::make_shared<Replacement>(
          std::make_tuple(cdw, start_it, end_it, benefit));
      repl_vec.push_back(repl);
    }
  }

  for (auto const &elem : m_sbi_map) {
    if (valToBool(m_var_model[elem.second].first)) {
      ++m_stats.m_det_sbis;
      auto index = elem.first;
      //          assert( index < m_bit_vec->size() );
      auto start_it = m_bit_vec_begin;
      std::advance(start_it, index);
      CDW cdw = getCDW(to_string(*start_it));
      //          cout << boost::format("[i] Prod SBI for idx %i: %s") % index %
      //          (to_string(*start_it)) << endl;
      assert(isValidCDW(cdw));
      auto benefit = getCDWBenefit(cdw);
      ReplacementPtr repl = std::make_shared<Replacement>(
          std::make_tuple(cdw, start_it, start_it + 1, benefit));
      repl_vec.push_back(repl);
    }
  }
  if (m_cfg_ptr->getProperty(CFG::STATS)) {
    if (m_cfg_ptr->getProperty(CFG::BENCHMARK)) {
      m_stats.collectBenchmarkData();
    }
    m_stats.printStats("SAT");
  }
}

void FormalDecompressor::buildOverlappings(BitVecCItr l_start, BitVecCItr l_end,
                                           unsigned int length,
                                           VarUDWMap &cdw_map) {
  assert(distance(l_start, l_end) == length);
  auto start = m_bit_vec_begin;
  auto idx_start = std::distance(start, l_start);

  Clasp::Var repl_var =
      m_ctx->addVar(Clasp::Var_t::atom_var); // TODO2: Need #X^2 REPL vars
  m_idx_map.insert(
      VarIndexMap::value_type(repl_var, make_pair(l_start, l_end)));

  // TODO: Hierher?
  unsigned int num_x = countBitVecX(l_start, l_end);
  for (unsigned int i = pow(2, num_x); i > 0; i--) {
    // cout << "Alloc var" << endl;
  }
  // cout << boost::format("[i] Found %u Xs in data.") % num_x << endl;

  //-------------------
  boost::dynamic_bitset<> part_bv; //( length, convertBitVec( l_start, l_end )
                                   //); //TODO2: Change to vector<int>
  auto const &udw_var_tuple = cdw_map[part_bv];

  Clasp::LitVec lits;
  lits.push_back(Clasp::Literal(repl_var, true));
  // Add weight for codeword-based optimization
  if (length == 4) {
    m_weighted_codeword_lits.push_back(
        Clasp::WeightLiteral(Clasp::Literal(repl_var, false), 2));
  } else if (length == 8) {
    m_weighted_codeword_lits.push_back(
        Clasp::WeightLiteral(Clasp::Literal(repl_var, false), 1));
  }
  // --
  boost::fusion::for_each(
      udw_var_tuple,
      [&](const Clasp::Var &udw_var) { // Overlapping => ( UDW4 or UDW8 )
        lits.push_back(Clasp::Literal(udw_var, false));
      });
  addClause(lits);

  for (unsigned int i = idx_start; i < idx_start + length; ++i) {
    m_overlap_map[i].push_back(repl_var);
  }
  if (m_cfg_ptr->isDebug()) {
    auto idx_end = idx_start + length - 1;
    cout << boost::format("'%s' [%i:%i] \t -> var_%i") % part_bv % idx_start %
                idx_end % repl_var
         << endl;
  }
}

void FormalDecompressor::processOverlappings() {
  // IDX -> overlapping chunks
  for (auto const &elem : m_overlap_map) {
    if (elem.second.size() <= 1) {
      continue;
    } // No possible conflicts.
    auto end_it = elem.second.cend();
    for (auto start_it = elem.second.cbegin(); start_it != end_it; ++start_it) {
      for (auto it = start_it + 1; it != end_it; ++it) {
        if (*start_it == *it) {
          continue;
        } // Skip
        m_bin_clauses.insert(make_pair(Clasp::Literal(*start_it, true),
                                       Clasp::Literal(*it, true)));
      }
    }
  }
}
/* Ensure at least one replacement (by CDW4 or CDW8)*/
void FormalDecompressor::enforceCoverage() {
  for (auto const &idx : m_overlap_map) {
    Clasp::LitVec lits = {};
    for (auto const &entry : idx.second) {
      lits.push_back(Clasp::Literal(entry, false));
    }
    addClause(lits);
  }
}

void FormalDecompressor::addUDWConstraint(VarUDWMap::value_type &elem) {
  bool ext_cdws = m_cfg_ptr->getProperty(CFG::MAX_CDWS) > 12; // TODO for all
  auto cdw = getCDW(elem.first);
  if (!isStaticCDW(cdw, ext_cdws)) {
    Clasp::Var udw_var_4, udw_var_8;
    tie(udw_var_4, udw_var_8) = elem.second;
    m_weighted_cdw_4_lits.push_back(
        Clasp::WeightLiteral(Clasp::Literal(udw_var_4, false), 1));
    m_weighted_cdw_8_lits.push_back(
        Clasp::WeightLiteral(Clasp::Literal(udw_var_8, false), 1));
    m_weighted_codeword_lits.push_back(
        Clasp::WeightLiteral(Clasp::Literal(udw_var_4, false), 1));
    m_weighted_codeword_lits.push_back(
        Clasp::WeightLiteral(Clasp::Literal(udw_var_8, false), 2));
  }
}

void FormalDecompressor::addCDWConstraint(unsigned int max_cdws) {
  cout << "[v] Setting CDW constraint to " << max_cdws << endl;
  for (auto &elem : m_byteudw_map) {
    addUDWConstraint(elem);
  }
  for (auto &elem : m_udw_map) {
    addUDWConstraint(elem);
  }
  Clasp::wsum_t *bound_4 = new Clasp::wsum_t(3);
  Clasp::wsum_t *bound_8 =
      new Clasp::wsum_t((m_cfg_ptr->getProperty(CFG::MAX_CDWS) - 1));

  Clasp::WeightConstraint::CPair constr_cpair_4 =
      Clasp::WeightConstraint::create(*m_solver,
                                      Clasp::Literal(*m_act_var1, true),
                                      m_weighted_cdw_4_lits, *bound_4, 1u);
  Clasp::WeightConstraint::CPair constr_cpair_8 =
      Clasp::WeightConstraint::create(*m_solver,
                                      Clasp::Literal(*m_act_var1, true),
                                      m_weighted_cdw_8_lits, *bound_8, 1u);

  assert(constr_cpair_4.ok() && constr_cpair_8.ok());
}

void FormalDecompressor::addSBIConstraint(unsigned int max_sbis) {
  Clasp::wsum_t *bound = new Clasp::wsum_t(max_sbis);
  Clasp::WeightConstraint::CPair constr_cpair = Clasp::WeightConstraint::create(
      *m_solver, Clasp::Literal(*m_act_var2, true), m_weighted_sbi_lits,
      *bound);
  assert(constr_cpair.ok());
}

void FormalDecompressor::initSolver() {
  m_params->reduce.fInit = 0;
  m_params->reduce.fGrow = 0;
  m_params->reduce.fMax = 0;
  m_params->randProb = 0.00;
  m_limits->conflicts = m_cfg_ptr->getProperty(CFG::SAT_CONFL);
  m_limits->restarts = m_cfg_ptr->getProperty(CFG::SAT_RESTART);

  LengthConfig len_cfg;
  len_cfg.emplace_back(make_pair(4, &m_udw_map));
  len_cfg.emplace_back(make_pair(8, &m_byteudw_map));

  calculateCDW(len_cfg);
  prepareBitVec(len_cfg);

  m_ctx->startAddConstraints();
  processBinary();
  enforceCoverage();
  addCDWConstraint(m_cfg_ptr->getProperty(CFG::MAX_CDWS));
  modelMinimization(MinimizationType::SBI);

  cout << "[i] Initialization of SAT solver done!" << endl;
}

void FormalDecompressor::calculateCDW(LengthConfig &len_cfg) {

  for (auto &elem : len_cfg) {
    unsigned int length = elem.first;
    auto &cdw_map = *elem.second;
    auto up = pow(2, length);
    for (unsigned int i = 0; i < up; ++i) {
      boost::dynamic_bitset<> udw(length, i);
      Clasp::Var var4 = m_ctx->addVar(Clasp::Var_t::atom_var);
      Clasp::Var var8 = m_ctx->addVar(Clasp::Var_t::atom_var);
      cdw_map[udw] = make_tuple(var4, var8);
    }
  }
}

void FormalDecompressor::extractModel() {
  assert(m_enum->enumerated() > 0);
  cout << "[i] Extracting Model..." << endl;
  const Clasp::Model &model = m_enum->lastModel();

  for (auto const &elem : m_udw_map) {
    boost::fusion::for_each(elem.second, [&](const Clasp::Var &udw_var) {
      m_var_model[udw_var] =
          make_pair(model.value(udw_var),
                    boost::str(boost::format("cdw_var%i") % udw_var));
    });
  }
  for (auto const &elem : m_byteudw_map) {
    boost::fusion::for_each(elem.second, [&](const Clasp::Var &udw_var) {
      m_var_model[udw_var] =
          make_pair(model.value(udw_var),
                    boost::str(boost::format("byte_cdw_var%i") % udw_var));
    });
  }

  // TODO: Fixme
  VarIndexMap::left_const_iterator it = m_idx_map.left.begin();
  for (; it != m_idx_map.left.end(); ++it) {
    auto &elem = it->first;
    m_var_model[elem] = make_pair(
        model.value(elem), boost::str(boost::format("overlap_var%i") % elem));
  }
  for (auto const &elem : m_sbi_map) {
    m_var_model[elem.second] =
        make_pair(model.value(elem.second),
                  boost::str(boost::format("sbi_var%i") % elem.second));
  }
  m_var_model[*m_act_var1] =
      make_pair(model.value(*m_act_var1), "cdw_act_var1");
  m_var_model[*m_act_var2] =
      make_pair(model.value(*m_act_var2), "cdw_act_var2");
  m_var_model[*m_act_var3] =
      make_pair(model.value(*m_act_var3), "cdw_act_var3");

  if (m_cfg_ptr->isDebug()) {
    for (auto const &entry : m_var_model) {
      std::cout << boost::format("[d] var_%i = %s (%s)") %
                       (unsigned)entry.first %
                       valToChar((unsigned)entry.second.first) %
                       entry.second.second
                << std::endl;
    }
  }
  m_stats.m_act_merges = 0;
  // Merges
  for (auto const &elem : m_merge_map) {
    if (valToBool(model.value(elem.first))) {
      ++m_stats.m_act_merges;
    }
  }
}

void FormalDecompressor::modelMinimization(MinimizationType min_type) {
  m_ctx->startAddConstraints();
  Clasp::MinimizeBuilder min_builder;
  switch (min_type) {
  case MinimizationType::Codwords: {
    min_builder.addRule(m_weighted_codeword_lits,
                        m_weighted_codeword_lits.size());
    assert(min_builder.numRules() == 1 &&
           min_builder.numLits() == m_weighted_codeword_lits.size());
    break;
  }
  case MinimizationType::SBI: {
    min_builder.addRule(m_weighted_sbi_lits, m_weighted_sbi_lits.size());
    assert(min_builder.numRules() == 1 &&
           min_builder.numLits() == m_weighted_sbi_lits.size());
    break;
  }
  case MinimizationType::Merge: {
    min_builder.addRule(m_weighted_merge_lits, m_weighted_merge_lits.size());
    assert(min_builder.numRules() == 1 &&
           min_builder.numLits() == m_weighted_merge_lits.size());
    break;
  }
  case MinimizationType::SBIMerge: {
    min_builder.addRule(m_weighted_sbi_lits, m_weighted_sbi_lits.size());
    min_builder.addRule(m_weighted_merge_lits, m_weighted_merge_lits.size());
    assert(min_builder.numRules() == 2 &&
           min_builder.numLits() ==
               m_weighted_merge_lits.size() + m_weighted_sbi_lits.size());
    break;
  }
  default:
    assert(false);
    break;
  }

  if ((m_sdata = min_builder.build(*m_ctx)) != 0 &&
      m_sdata->setMode(Clasp::MinimizeMode_t::optimize)) { // 0 := bound

    m_constr =
        m_sdata->attach(*m_solver, Clasp::MinimizeMode_t::Strategy::opt_bb,
                        Clasp::MinimizeMode_t::bb_step_def);
    m_constr->integrate(*m_solver);
    m_enum->init(*m_ctx, m_sdata, 0); // BEFORE: ctx.endInit!

    if (!m_ctx->endInit()) { // BEFORE: min_builder.creation !
      throw std::runtime_error(boost::str(
          boost::format("[e] Initialization of SAT solver failed!")));
    }
  } else {
    assert(false);
  }

  assert(m_sdata->optimize());
  assert(m_constr->valid(*m_solver));
}

void FormalDecompressor::modelMergeAnd(Clasp::Var var_a, Clasp::Var var_b,
                                       Clasp::Var merge_var,
                                       MinimizationType type) {
  // Model: A ^ B = C (merge var)
  m_bin_clauses.insert(make_pair(Clasp::Literal(var_a, false),
                                 Clasp::Literal(merge_var, true))); // A or !C
  m_bin_clauses.insert(make_pair(Clasp::Literal(var_b, false),
                                 Clasp::Literal(merge_var, true))); // B or !C

  if (m_cfg_ptr->isDebug()) {
    cout << boost::format("[d] [MERGE] var_%i ^ var_%i = merge_%i") % var_a %
                var_b % merge_var
         << endl;
  }
  Clasp::LitVec lits; // !A OR !B OR C
  lits.push_back(Clasp::Literal(var_a, true));
  lits.push_back(Clasp::Literal(var_b, true));
  lits.push_back(Clasp::Literal(merge_var, false));
  addClause(lits);
  if (type == MinimizationType::Merge) {
    m_weighted_merge_lits.push_back(
        Clasp::WeightLiteral(Clasp::Literal(merge_var, true), 1));
  } else if (type == MinimizationType::SBI) {
    m_weighted_merge_lits.push_back(
        Clasp::WeightLiteral(Clasp::Literal(merge_var, true), 2));
  } else {
    assert(false);
  }
}

void FormalDecompressor::processMerges(unsigned int length) {
  unsigned int end_offset = 2 * length;

  for (auto it = m_bit_vec_begin; it + end_offset <= m_bit_vec_end; ++it) {
    auto start_a = it;
    auto end_a = it + length;
    auto end_b = it + end_offset;

    if (serializeBitVec(start_a, end_a) != serializeBitVec(end_a, end_b)) {
      continue;
    }
    ++m_stats.m_merge_vars;
    auto overlap_fst = std::make_pair(start_a, end_a);
    auto overlap_snd = std::make_pair(end_a, end_b);
    auto overlap_fst_it = m_idx_map.right.find(overlap_fst);
    assert(overlap_fst_it != m_idx_map.right.end());
    auto overlap_snd_it = m_idx_map.right.find(overlap_snd);
    assert(overlap_snd_it != m_idx_map.right.end());

    Clasp::Var merge_var = m_ctx->addVar(
        Clasp::Var_t::atom_var); // Variable for the merge being active
    assert(m_ctx->validVar(merge_var));
    Clasp::Var var_b = overlap_fst_it->second;
    Clasp::Var var_c = overlap_snd_it->second;
    assert(m_ctx->validVar(var_b) && m_ctx->validVar(var_c));
    m_merge_map[merge_var] = make_pair(var_b, var_c);
    modelMergeAnd(var_b, var_c, merge_var, MinimizationType::Merge);
  }
}

void FormalDecompressor::processSBIMerges() {
  for (BitVecCItr it = m_bit_vec_begin; it + 1 != m_bit_vec_end; ++it) {
    if (*it != *(it + 1)) {
      continue;
    }
    auto pos_b = std::distance(m_bit_vec_begin, it);
    Clasp::Var var_b = m_sbi_map[pos_b];
    Clasp::Var var_c = m_sbi_map[(pos_b + 1)];

    Clasp::Var merge_var = m_ctx->addVar(
        Clasp::Var_t::atom_var); // Variable for the merge being active
    m_merge_map[merge_var] = make_pair(var_b, var_c);
    modelMergeAnd(var_b, var_c, merge_var, MinimizationType::SBI);
  }
}

void FormalDecompressor::solve() {
  m_stats.m_bin_clauses = m_ctx->numBinary();
  m_stats.m_constraints = m_ctx->numConstraints();
  m_stats.m_vars = m_ctx->numVars();

  m_enum->start(*m_solver);
  m_assumptions.push_back(
      Clasp::Literal(*m_act_var1, false)); // Enforce act_lit to be true.
  m_assumptions.push_back(
      Clasp::Literal(*m_act_var3, false)); // Enforce act_lit to be true.

  Clasp::SequentialSolve fst_seq_solver(m_enum.get(), *m_limits);
  //    seq_solver.setEnumLimit( 10000 );
  bool more = fst_seq_solver.solve(*m_ctx, m_assumptions, nullptr);
  m_enum->commit(*m_solver);
  Clasp::wsum_t last_optimum = *(m_enum->minimizer()->sum());
  auto num_models = m_enum->enumerated();
  m_stats.m_restarts = m_solver->stats.restarts;
  m_stats.m_ccs = m_solver->stats.conflicts;
  cout << boost::format("[i] First solving done: Space exceeded: %i - Level "
                        "optimum: %i - Enumerated: %i") %
              !more % last_optimum % num_models
       << endl;
  extractModel();
  dump();

  // ############################
  // Secondary solve process
  if (m_cfg_ptr->getProperty(CFG::SAT_SEC)) {
    m_enum->reset();
    if (last_optimum == 0) {
      ++last_optimum;
    }
    last_optimum = ceil(last_optimum * 1.05);
    modelMinimization(MinimizationType::Codwords);
    addSBIConstraint(
        last_optimum); // Adding enum constriant: Opt. target of initial search!
    m_assumptions.push_back(Clasp::Literal(*m_act_var2, false));
    cout << boost::format("[i] Adding new PBO constraint with limit %i !") %
                last_optimum
         << endl;
    Clasp::SequentialSolve snd_seq_solver(m_enum.get(), *m_limits);
    more = snd_seq_solver.solve(*m_ctx, m_assumptions, nullptr);
    num_models = m_enum->enumerated();
    last_optimum = *(m_enum->minimizer()->sum());
    cout << boost::format("[i] Second solving done: Space exceeded: %i - Level "
                          "optimum: %i - Enumerated: %i") %
                !more % last_optimum % num_models
         << endl;
    if (num_models > 0) {
      m_enum->commit(*m_solver);
      extractModel();
    }
  }
  // ############################
}

void FormalDecompressor::clear() {
  // m_assumptions.clear();
  m_stats = FormalDecompressorStats(m_cfg_ptr);
  m_sbi_map.clear();
  m_idx_map.clear();
  m_overlap_map.clear();
  m_merge_map.clear();
  m_udw_map.clear();
  m_byteudw_map.clear();
  m_var_model.clear();

  m_sdata = nullptr;
  m_constr = nullptr;

  // delete m_heu;
  // delete m_enum;
  // delete m_params;
  // delete m_limits;

  m_weighted_merge_lits.clear();
  m_weighted_codeword_lits.clear();
  m_weighted_sbi_lits.clear();
  m_weighted_cdw_4_lits.clear();
  m_weighted_cdw_8_lits.clear();
  m_bin_clauses.clear();

  // m_heu->detach(*m_solver);;
  // delete m_ctx;
  m_ctx->reset();
  m_enum = make_shared<Clasp::ModelEnumerator>(
      Clasp::ModelEnumerator::Strategy::strategy_backtrack);
  m_params = make_shared<Clasp::SolveParams>();
  m_limits = make_shared<Clasp::SolveLimits>();

  m_solver = m_ctx->master();
  m_solver->setHeuristic(new Clasp::ClaspVmtf(32));
  m_act_var1 = shared_ptr<Clasp::Var>(
      new Clasp::Var(m_ctx->addVar(Clasp::Var_t::atom_var)));
  m_act_var2 = shared_ptr<Clasp::Var>(
      new Clasp::Var(m_ctx->addVar(Clasp::Var_t::atom_var)));
  m_act_var3 = shared_ptr<Clasp::Var>(
      new Clasp::Var(m_ctx->addVar(Clasp::Var_t::atom_var)));
}

void FormalDecompressor::dump(bool force) const {
  if (!m_cfg_ptr->isDebug() && !force) {
    return;
  }
  cout << "###########################" << endl;
  cout << "[DUMP] SAT Solver binary clauses:" << endl;
  for (auto const &elem : m_bin_clauses) {
    cout << boost::format("%s%i OR %s%i") % (elem.first.sign() ? "!" : "") %
                elem.first.var() % (elem.second.sign() ? "!" : "") %
                elem.second.var()
         << endl;
  }
  cout << "[DUMP] ... ends!" << endl;
  cout << "[DUMP] SAT Solver index constraints:" << endl;
  for (auto const &elem : m_overlap_map) {
    cout << boost::format("idx_%s -> ( ") % (unsigned)elem.first;
    for (auto const &entry : elem.second) {
      cout << boost::format("var_%i ") % entry;
    }
    cout << ")" << endl;
  }

  cout << "[DUMP] ... ends!" << endl;
  cout << "[DUMP] SAT Solver SBI vars:" << endl;
  for (auto const &elem : m_sbi_map) {
    cout << boost::format("sbi_%i -> var_%i") % elem.second % elem.first
         << endl;
  }
  cout << "[DUMP] ... ends!" << endl;
  cout << "[DUMP] SAT Solver CDW vars:" << endl;
  for (auto const &elem : m_udw_map) {
    auto udw_tuple = elem.second;
    cout << boost::format("'%s' \t -> ( var_%s@4 (%s), var_%s@8 (%s) ) ") %
                elem.first % (get<0>(udw_tuple)) %
                dumpValue(get<0>(udw_tuple)) % (get<1>(udw_tuple)) %
                dumpValue(get<1>(udw_tuple))
         << endl;
  }
  for (auto const &elem : m_byteudw_map) {
    auto udw_tuple = elem.second;
    cout << boost::format("'%s' \t -> ( var_%s@4 (%s), var_%s@8 (%s)  ) ") %
                elem.first % (get<0>(udw_tuple)) %
                dumpValue(get<0>(udw_tuple)) % (get<1>(udw_tuple)) %
                dumpValue(get<1>(udw_tuple))
         << endl;
  }
  cout << "[DUMP] ... ends!" << endl;
  cout << "###########################" << endl;
}

char FormalDecompressor::dumpValue(Clasp::Var var) const {
  auto var_it = m_var_model.find(var);
  assert(var_it != m_var_model.end());

  return (valToChar(var_it->second.first));
}

char FormalDecompressor::valToChar(const Clasp::ValueRep var_value) {
  switch (var_value) {
  case Clasp::value_true:
    return 'T';
    break;
  case Clasp::value_false:
    return 'F';
    break;
  case Clasp::value_free:
    return 'X';
    break;
  default:
    return '?';
    break;
  }
}

bool FormalDecompressor::valToBool(const Clasp::ValueRep var_value) {
  switch (var_value) {
  case Clasp::value_false:
    return false;
  case Clasp::value_true:
    return true;
  case Clasp::value_free:
    throw std::runtime_error(boost::str(
        boost::format("[e] Expecting assigned value for var %i") % var_value));
  default:
    return false;
  }
}
} // namespace vecthor
