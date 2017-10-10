// Probabilistic Question-Answering system
// @2017 Sarge Rogatch
// This software is distributed under GNU AGPLv3 license. See file LICENSE in repository root for details.

#include "stdafx.h"
#include "../PqaCore/CEDivTargPriorsSubtask.h"
#include "../PqaCore/CESetPriorsSubtaskSum.h"
#include "../PqaCore/CESetPriorsTask.h"
#include "../PqaCore/CECreateQuizOperation.h"
#include "../PqaCore/CEUpdatePriorsTask.h"
#include "../PqaCore/CEUpdatePriorsSubtaskMul.h"
#include "../PqaCore/CEQuiz.h"
#include "../PqaCore/CpuEngine.h"

using namespace SRPlat;

namespace ProbQA {

template class CECreateQuizStart<SRDoubleNumber>;
template class CECreateQuizResume<SRDoubleNumber>;

template<typename taNumber> void CECreateQuizStart<taNumber>::UpdateLikelihoods(BaseCpuEngine &baseCe,
  CEBaseQuiz &baseQuiz)
{
  auto &PTR_RESTRICT engine = static_cast<CpuEngine<taNumber>&>(baseCe);
  auto &PTR_RESTRICT quiz = static_cast<CEQuiz<taNumber>&>(baseQuiz);

  const EngineDimensions& dims = engine.GetDims();
  const SRThreadCount nWorkers = engine.GetWorkers().GetWorkerCount();

  SRMemTotal mtCommon;
  const SRByteMem miSubtasks(nWorkers * std::max(SRBucketSummatorPar<taNumber>::_cSubtaskMemReq,
    SRMaxSizeof<CESetPriorsSubtaskSum<taNumber>, CEDivTargPriorsSubtask<CESetPriorsTask<taNumber>>>::value),
    SRMemPadding::None, mtCommon);
  const SRByteMem miSplit(SRPoolRunner::CalcSplitMemReq(nWorkers), SRMemPadding::Both, mtCommon);
  const SRByteMem miBuckets(SRBucketSummatorPar<taNumber>::GetMemoryRequirementBytes(nWorkers), SRMemPadding::Both,
    mtCommon);

  SRSmartMPP<uint8_t> commonBuf(engine.GetMemPool(), mtCommon._nBytes);
  SRPoolRunner pr(engine.GetWorkers(), miSubtasks.BytePtr(commonBuf));
  SRBucketSummatorPar<taNumber> bsp(nWorkers, miBuckets.BytePtr(commonBuf));

  const TPqaId nTargetVects = SRSimd::VectsFromComps<double>(dims._nTargets);
  const SRPoolRunner::Split targSplit = SRPoolRunner::CalcSplit(miSplit.BytePtr(commonBuf), nTargetVects, nWorkers);

  CESetPriorsTask<taNumber> spTask(engine, quiz, bsp);
  {
    SRRWLock<false> rwl(engine.GetRws());
    // Zero out exponents, copy mantissas, prepare for summing
    pr.RunPreSplit<CESetPriorsSubtaskSum<taNumber>>(spTask, targSplit);
  }
  spTask._sumPriors.Set1(bsp.ComputeSum(pr));
  // Divide the likelihoods by their sum so to get probabilities
  pr.RunPreSplit<CEDivTargPriorsSubtask<CESetPriorsTask<taNumber>>>(spTask, targSplit);
}

template<typename taNumber> void CECreateQuizResume<taNumber>::UpdateLikelihoods(BaseCpuEngine &baseCe,
  CEBaseQuiz &baseQuiz)
{
  auto &PTR_RESTRICT engine = static_cast<CpuEngine<taNumber>&>(baseCe);
  auto &PTR_RESTRICT quiz = static_cast<CEQuiz<taNumber>&>(baseQuiz);

  //The input must have been validated
  const EngineDimensions& dims = engine.GetDims();
  const SRThreadCount nWorkers = engine.GetWorkers().GetWorkerCount();

  SRMemTotal mtCommon;
  const SRByteMem miSubtasks(nWorkers * std::max(CpuEngine<taNumber>::_cNormPriorsMemReqPerSubtask,
    SRMaxSizeof<CEUpdatePriorsSubtaskMul<taNumber>>::value), SRMemPadding::None, mtCommon);
  const SRByteMem miSplit(SRPoolRunner::CalcSplitMemReq(nWorkers), SRMemPadding::Both, mtCommon);
  const SRByteMem miBuckets(SRBucketSummatorPar<taNumber>::GetMemoryRequirementBytes(nWorkers), SRMemPadding::Both,
    mtCommon);

  SRSmartMPP<uint8_t> commonBuf(engine.GetMemPool(), mtCommon._nBytes);
  SRPoolRunner pr(engine.GetWorkers(), miSubtasks.BytePtr(commonBuf));
  SRBucketSummatorPar<taNumber> bsp(nWorkers, miBuckets.BytePtr(commonBuf));

  const TPqaId nTargetVects = SRSimd::VectsFromComps<double>(dims._nTargets);
  const SRPoolRunner::Split targSplit = SRPoolRunner::CalcSplit(miSplit.BytePtr(commonBuf), nTargetVects, nWorkers);
  {
    CEUpdatePriorsTask<taNumber> task(engine, quiz, _nAnswered, _pAQs, CalcVectsInCache());
    SRRWLock<false> rwl(engine.GetRws());
    // Copy from B and update the likelihoods with the questions answered.
    pr.RunPreSplit<CEUpdatePriorsSubtaskMul<taNumber>>(task, targSplit);
  }
  // Normalize to probabilities
  _err = engine.NormalizePriors(quiz, pr, bsp, targSplit);
}

} // namespace ProbQA
