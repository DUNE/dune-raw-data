#ifndef lbne_raw_data_BuildInfo_GetPackageBuildInfo_hh
#define lbne_raw_data_BuildInfo_GetPackageBuildInfo_hh

#include "artdaq-core/Data/PackageBuildInfo.hh"

#include <string>

namespace lbnerawdata {

  struct GetPackageBuildInfo {

    static artdaq::PackageBuildInfo getPackageBuildInfo();
  };

}

#endif /* lbne_raw_data_BuildInfo_GetPackageBuildInfo_hh */
