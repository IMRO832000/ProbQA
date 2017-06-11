#pragma once

#include "../SRPlatform/Interface/SRMacros.h"

namespace SRPlat {

typedef HANDLE SRHandle;

template <SRHandle taEmpty> class SRSmartHandle {
  SRHandle _srh;
public:
  explicit SRSmartHandle(SRHandle srh = taEmpty) : _srh(srh) {
  }
  ~SRSmartHandle() {
    if (_srh != taEmpty) {
      if (!CloseHandle(_srh)) {
        SR_DLOG_WINFAIL_GLE(Error);
      }
    }
  }
};

} // namespace SRPlat