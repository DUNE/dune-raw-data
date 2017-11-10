///////////////////////////////////////////////////////////////////////////////////////////////////
// Class:       PdChannelMapService
// Module type: service
// File:        PdChannelMapService.h
// Author:      Mike Wallbank (m.wallbank@sheffield.ac.uk), February 2016
//              Based on the service written for use in LArSoft by David Adams
//
// Implementation of online-offline channel mapping reading from a file.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PdChannelMapService_H
#define PdChannelMapService_H

#include <map>
#include <vector>
#include <iostream>
#include <limits>
#include <fstream>

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace dune {
  class PdChannelMapService;
}

class dune::PdChannelMapService {

public:

  PdChannelMapService(fhicl::ParameterSet const& pset);
  PdChannelMapService(fhicl::ParameterSet const& pset, art::ActivityRegistry&);
  
  /// Map RCE fragment ID and RCE internal channel number to online channel number
  unsigned int OnlineFromRCE(unsigned int rceID, unsigned int rceChannel) const;

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

  /// Returns regulator number
  unsigned int RegulatorFromOnlineChannel(unsigned int onlineChannel) const;
  unsigned int RegulatorFromOfflineChannel(unsigned int offlineChannel) const;

  /// Returns regulator pin number
  unsigned int RegulatorPinFromOnlineChannel(unsigned int onlineChannel) const;
  unsigned int RegulatorPinFromOfflineChannel(unsigned int offlineChannel) const;

  /// Returns ASIC number
  unsigned int ASICFromOnlineChannel(unsigned int onlineChannel) const;
  unsigned int ASICFromOfflineChannel(unsigned int offlineChannel) const;

  /// Returns ASIC channel number
  unsigned int ASICChannelFromOnlineChannel(unsigned int onlineChannel) const;
  unsigned int ASICChannelFromOfflineChannel(unsigned int offlineChannel) const;

private:
 
  // Maps
  std::map<unsigned int, std::map<unsigned int, unsigned int> > fRCEToOnline;
  std::map<unsigned int, unsigned int> fOnlineToOffline;
  std::map<unsigned int, unsigned int> fOfflineToOnline;

  std::map<unsigned int, unsigned int> fPlaneMap;
  std::map<unsigned int, unsigned int> fAPAMap;
  std::map<unsigned int, unsigned int> fRCEMap;
  std::map<unsigned int, unsigned int> fRCEChannelMap;
  std::map<unsigned int, unsigned int> fRegulatorMap;
  std::map<unsigned int, unsigned int> fRegulatorPinMap;
  std::map<unsigned int, unsigned int> fASICMap;
  std::map<unsigned int, unsigned int> fASICChannelMap;

};

DECLARE_ART_SERVICE(dune::PdChannelMapService, LEGACY)

#endif