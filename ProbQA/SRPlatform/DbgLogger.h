// Probabilistic Question-Answering system
// @2017 Sarge Rogatch
// This software is distributed under GNU AGPLv3 license. See file LICENSE in repository root for details.

#pragma once

#include "../SRPlatform/Interface/ISRLogger.h"

namespace SRPlat {

class DbgLogger : public ISRLogger {
public:
  virtual bool Log(const Severity sev, const SRString& message) override final;
  virtual SRString GetFileName() override final;
};

} // namespace SRPlat
