// Probabilistic Question-Answering system
// @2017 Sarge Rogatch
// This software is distributed under GNU AGPLv3 license. See file LICENSE in repository root for details.

#pragma once

#include "../SRPlatform/Interface/SRException.h"

namespace SRPlat {

DERIVE_SREXCEPTION(SRDefaultLoggerAlreadyInitialized, SR);
DERIVE_SREXCEPTION(SRDefaultLoggerConcurrentInitialization, SR);

} // namespace SRPlat
