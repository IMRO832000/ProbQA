#pragma once

#include "../PqaCore/Interface/PqaCommon.h"

namespace ProbQA {

template<typename taNumber> class CpuEngine;
template<typename taNumber> class CESubtask;

template<typename taNumber> class CETask {
  CpuEngine<taNumber> *_pCe;
  SRPlat::SRConditionVariable _isComplete;
  // Number of active subtasks
  std::atomic<TPqaId> _nToDo;
public:
  explicit CETask(CpuEngine<taNumber> *pCe, const TPqaId nToDo = 0);
  // Returns the value after the increment.
  TPqaId IncToDo(const TPqaId by = 1) { return by + _nToDo.fetch_add(by, std::memory_order_relaxed); }
  void OnSubtaskComplete(CESubtask<taNumber> *pSubtask);
};

} // namespace ProbQA
