// Probabilistic Question-Answering system
// @2017 Sarge Rogatch
// This software is distributed under GNU AGPLv3 license. See file LICENSE in repository root for details.

#pragma once

namespace ProbQA {

class CudaMain {
public:
  static void SetDevice(const int iDevice, const bool bFirstInProcess);
};

} // namespace ProbQA
