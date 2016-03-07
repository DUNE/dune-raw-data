///////////////////////////////////////////////////////////////////////////////////////////////////
// Class:       ChannelMapService
// Module type: service
// File:        ChannelMapService.h
// Author:      Mike Wallbank (m.wallbank@sheffield.ac.uk), February 2016
//              Based on the service written for use in LArSoft by David Adams
//
// Implementation of online-offline channel mapping reading from a file.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ChannelMapService_H
#define ChannelMapService_H

#include <map>
#include <vector>
#include <iostream>
#include <limits>
#include <fstream>

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace lbne {
  class ChannelMapService;
}

class lbne::ChannelMapService {

public:

  ChannelMapService(fhicl::ParameterSet const& pset);
  ChannelMapService(fhicl::ParameterSet const& pset, art::ActivityRegistry&);

  /// Map online to offline
  unsigned int Offline(unsigned int onlineChannel) const;

  /// Map offline to online
  unsigned int Online(unsigned int offlineChannel) const;

  /// Returns APA
  unsigned int APAFromOnlineChannel(unsigned int onlineChannel) const;
  unsigned int APAFromOfflineChannel(unsigned int offlineChannel) const;

  /// Returns plane
  unsigned int PlaneFromOnlineChannel(unsigned int onlineChannel) const;
  unsigned int PlaneFromOfflineChannel(unsigned int offlineChannel) const;

  /// Returns RCE
  unsigned int RCEFromOnlineChannel(unsigned int onlineChannel) const;
  unsigned int RCEFromOfflineChannel(unsigned int offlineChannel) const;

  /// Returns RCE channel number
  unsigned int RCEChannelFromOnlineChannel(unsigned int onlineChannel) const;
  unsigned int RCEChannelFromOfflineChannel(unsigned int offlineChannel) const;

private:
 
  // Maps
  std::map<unsigned int, unsigned int> fOnlineToOffline;
  std::map<unsigned int, unsigned int> fOfflineToOnline;

  std::map<unsigned int, unsigned int> fPlaneMap;
  std::map<unsigned int, unsigned int> fAPAMap;
  std::map<unsigned int, unsigned int> fRCEMap;
  std::map<unsigned int, unsigned int> fRCEChannelMap;

};

DECLARE_ART_SERVICE(lbne::ChannelMapService, LEGACY)

#endif
