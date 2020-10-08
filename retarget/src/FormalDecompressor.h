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

#ifndef FORMALDECOMPRESSOR_H
#define FORMALDECOMPRESSOR_H

// Project includes
#include <Config.h>
#include <Decompressor.h>
#include <Stats.h>
#include <TypeDefs.h>

// System includes
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <vector>

// Boost includes
#include <boost/bimap.hpp>
#include <boost/dynamic_bitset.hpp>

// Clasp includes
#include "clasp/model_enumerators.h"
#include "clasp/shared_context.h"
#include "clasp/solve_algorithms.h"
#include "clasp/solver.h"

namespace vecthor {

class FormalDecompressor : public Decompressor {
  // Var -> (Start-Iterator), (End-Iterator)
  //    using VarIndexMap = std::map< Clasp::Var, std::pair< BitVecCItr,
  //    BitVecCItr > >;
  using VarIndexMap =
      boost::bimap<Clasp::Var, std::pair<BitVecCItr, BitVecCItr>>;

  // Idx -> Var
  using VarSBIMap = std::map<unsigned int, Clasp::Var>;

  // Idx -> {Var_1, ... Var_n} @ IDX
  using VarOverlapMap = std::map<unsigned int, std::vector<Clasp::Var>>;

  // Var -> ( End-Iterator, Start-Iterator ) @ Mergeable Overlaps
  using VarMergeMap = std::map<Clasp::Var, std::pair<Clasp::Var, Clasp::Var>>;

  // Bitset -> ( Var-CDW4, Var-CDW8 )
  using VarUDWMap =
      std::map<boost::dynamic_bitset<>, std::tuple<Clasp::Var, Clasp::Var>>;

  using VarModelMap =
      std::map<Clasp::Var, std::pair<Clasp::ValueRep, std::string>>;
  using BinaryClauses = std::set<std::pair<Clasp::Literal, Clasp::Literal>>;
  using LengthConfig = std::vector<std::pair<unsigned int, VarUDWMap *>>;

public:
  enum class MinimizationType { SBI, SBIMerge, MergeSBI, Merge, Codwords };

  FormalDecompressor(const Config *cfg_ptr);
  void determineCDW(BitVecCItr &bv_begin, BitVecCItr &bv_end) override;
  void processModel(Route &repl_vec);
  void clear() override;

  inline Stats &getStats() override { return m_stats; }

private:
  bool addClause(Clasp::LitVec &lits);
  void addCDWConstraint(unsigned int max_cdws);
  void addUDWConstraint(VarUDWMap::value_type &elem);
  void addSBIConstraint(unsigned int max_sbis);
  void buildOverlappings(BitVecCItr l_start, BitVecCItr l_end,
                         unsigned int length, VarUDWMap &cdw_map);
  void processOverlappings();
  void enforceCoverage();
  void calculateCDW(LengthConfig &len_cfg);
  void extractModel();
  void extractModelUDWValue(const VarUDWMap::value_type &elem);
  void modelMergeAnd(Clasp::Var var_b, Clasp::Var var_c, Clasp::Var merge_var,
                     MinimizationType type);
  void modelMinimization(MinimizationType min_type);
  void processMerges(unsigned int length);
  void processSBIMerges();
  void prepareBitVec(LengthConfig &len_cfg);
  void processBinary();
  void processSBIs();
  void initSolver();
  void solve();
  void dump(bool force = false) const;
  char dumpValue(Clasp::Var var) const;
  static inline char valToChar(const Clasp::ValueRep var_value);
  static inline bool valToBool(const Clasp::ValueRep var_value);

private:
  std::shared_ptr<Clasp::SharedContext> m_ctx;
  std::shared_ptr<Clasp::DecisionHeuristic> m_heu;
  std::shared_ptr<Clasp::ModelEnumerator> m_enum;
  std::shared_ptr<Clasp::SolveParams> m_params;
  std::shared_ptr<Clasp::SolveLimits> m_limits;
  std::shared_ptr<Clasp::Var> m_act_var1, m_act_var2, m_act_var3;

  Clasp::Solver *m_solver;
  Clasp::SharedMinimizeData *m_sdata;
  Clasp::MinimizeConstraint *m_constr;
  Clasp::LitVec m_assumptions;

  VarSBIMap m_sbi_map;
  VarIndexMap m_idx_map;
  VarOverlapMap m_overlap_map;
  VarMergeMap m_merge_map;
  VarUDWMap m_udw_map;
  VarUDWMap m_byteudw_map;
  VarModelMap m_var_model;

  Clasp::WeightLitVec m_weighted_merge_lits;
  Clasp::WeightLitVec m_weighted_codeword_lits;
  Clasp::WeightLitVec m_weighted_sbi_lits;
  Clasp::WeightLitVec m_weighted_cdw_4_lits, m_weighted_cdw_8_lits;

  BinaryClauses m_bin_clauses;
  FormalDecompressorStats m_stats;
  BitVecCItr m_bit_vec_begin;
  BitVecCItr m_bit_vec_end;
};

} // namespace vecthor

#endif // FORMALDECOMPRESSOR_H
