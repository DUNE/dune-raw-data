///////////////////////////////////////////////////////////////////////////////////////////////////
// Class:       PdspChannelMapService
// Module type: service
// File:        PdspChannelMapService.h
// Author:      Jingbo Wang (jiowang@ucdavis.edu), February 2018
//
// Implementation of hardware-offline channel mapping reading from a file.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PdspChannelMapService_H
#define PdspChannelMapService_H

#include <map>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <limits>
#include <fstream>

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace dune {
  class PdspChannelMapService;
}

class dune::PdspChannelMapService {

public:

  PdspChannelMapService(fhicl::ParameterSet const& pset);
  PdspChannelMapService(fhicl::ParameterSet const& pset, art::ActivityRegistry&);
  
  typedef std::tuple<int,int,int,int> key_csfc;
   
  struct key_hash : public std::unary_function<key_csfc, std::size_t>
  {
    std::size_t operator()(const key_csfc& k) const {
      return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k) ^ std::get<3>(k);
    }
  };
   
  struct key_equal : public std::binary_function<key_csfc, key_csfc, bool> {
    bool operator()(const key_csfc& v0, const key_csfc& v1) const {
      return (
        std::get<0>(v0) == std::get<0>(v1) &&
        std::get<1>(v0) == std::get<1>(v1) &&
        std::get<2>(v0) == std::get<2>(v1) &&
        std::get<3>(v0) == std::get<3>(v1)
      );
    }
  };

  typedef std::unordered_map<const key_csfc,int,key_hash,key_equal> channel_map;
  	
  unsigned int GetFEMBChannelFromRCEStreamChannel(unsigned int rceCh) const;
  
  // Map instrumentation numbers (crate:slot:fiber:FEMBchannel) to offline channel number
  unsigned int GetOfflineNumberFromDetectorElements(unsigned int crate, unsigned int slot, unsigned int fiber, unsigned int fembchannel) const;
  	
  

  /// Returns APA/crate
  unsigned int APAFromOfflineChannel(unsigned int offlineChannel) const;
  
  /// Returns WIB/slot
  unsigned int WIBFromOfflineChannel(unsigned int offlineChannel) const;
  
  /// Returns FEMB/fiber
  unsigned int FEMBFromOfflineChannel(unsigned int offlineChannel) const;
  
  /// Returns FEMB channel
  unsigned int FEMBChannelFromOfflineChannel(unsigned int offlineChannel) const;
  
  /// Returns RCE(FELIX) stream(frame) channel
  unsigned int StreamChannelFromOfflineChannel(unsigned int offlineChannel) const;
  
  /// Returns global slot ID
  unsigned int SlotIdFromOfflineChannel(unsigned int offlineChannel) const;
  
  /// Returns global fiber ID
  unsigned int FiberIdFromOfflineChannel(unsigned int offlineChannel) const;

  /// Returns chip number
  unsigned int ChipFromOfflineChannel(unsigned int offlineChannel) const;
  
  /// Returns chip channel number
  unsigned int ChipChannelFromOfflineChannel(unsigned int offlineChannel) const;
  
  /// Returns ASIC number
  unsigned int ASICFromOfflineChannel(unsigned int offlineChannel) const;
  
  /// Returns ASIC channel number
  unsigned int ASICChannelFromOfflineChannel(unsigned int offlineChannel) const;
  
  /// Returns plane
  unsigned int PlaneFromOfflineChannel(unsigned int offlineChannel) const;
  
  /////////////////////////\ ProtoDUNE-SP channel map fundtions //////////////////////////////
  unsigned int OfflineFromCSFC;

private:
 
  // Maps
  channel_map fCsfcToOffline;
  std::unordered_map<unsigned int, unsigned int> fAPAMap; // APA(crate)
  std::unordered_map<unsigned int, unsigned int> fWIBMap;	// WIB(slot)
  std::unordered_map<unsigned int, unsigned int> fFEMBMap;	// FEMB(fiber)
  std::unordered_map<unsigned int, unsigned int> fFEMBChannelMap;	// FEMB internal channel
  std::unordered_map<unsigned int, unsigned int> fStreamChannelMap;	// RCE(FELIX) internal channel
  std::unordered_map<unsigned int, unsigned int> fSlotIdMap; // global WIB(slot) ID
  std::unordered_map<unsigned int, unsigned int> fFiberIdMap; // global FEMB(fiber) ID
  std::unordered_map<unsigned int, unsigned int> fChipMap; // Chip
  std::unordered_map<unsigned int, unsigned int> fChipChannelMap; //Chip internal channel 
  std::unordered_map<unsigned int, unsigned int> fASICMap; // ASIC
  std::unordered_map<unsigned int, unsigned int> fASICChannelMap; // ASIC internal channel
  std::unordered_map<unsigned int, unsigned int> fPlaneMap; // Plane type

};

DECLARE_ART_SERVICE(dune::PdspChannelMapService, LEGACY)

#endif
