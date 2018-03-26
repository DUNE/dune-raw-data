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
  
  unsigned int GetFEMBChannelFromRCEStreamChannel(unsigned int rceCh) const;
  
  // Map instrumentation numbers (crate:slot:fiber:FEMBchannel) to offline channel number
  unsigned int GetOfflineNumberFromDetectorElements(unsigned int crate, unsigned int slot, unsigned int fiber, unsigned int fembchannel);
  	  

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

  // hardcoded sizes

  // Note -- we are assuming that FELIX with its two fibers per fragment knows the fiber numbers and that we aren't
  // encoding double-size channel lists for FELIX with single fiber numbers.

  size_t fNChans = 15360;
  size_t fNCrates = 6;
  size_t fNSlots = 5;
  size_t fNFibers = 4;
  size_t fNFEMBChans = 128; 

  // control behavior in case we need to fall back to default behavior

  bool fHaveWarnedAboutBadCrateNumber;
  bool fHaveWarnedAboutBadSlotNumber;
  bool fHaveWarnedAboutBadFiberNumber;

  // Maps
  unsigned int farrayCsfcToOffline[6][5][4][128];  // implement as an array.  Do our own bounds checking

  // lookup tables as functions of offline channel number

  unsigned int fvAPAMap[15360]; // APA(=crate)
  unsigned int fvWIBMap[15360];	// WIB(slot)
  unsigned int fvFEMBMap[15360];	// FEMB(fiber)
  unsigned int fvFEMBChannelMap[15360];	// FEMB internal channel
  unsigned int fvStreamChannelMap[15360];	// RCE(FELIX) internal channel
  unsigned int fvSlotIdMap[15360]; // global WIB(slot) ID
  unsigned int fvFiberIdMap[15360]; // global FEMB(fiber) ID
  unsigned int fvChipMap[15360]; // Chip
  unsigned int fvChipChannelMap[15360]; //Chip internal channel 
  unsigned int fvASICMap[15360]; // ASIC
  unsigned int fvASICChannelMap[15360]; // ASIC internal channel
  unsigned int fvPlaneMap[15360]; // Plane type

  void check_offline_channel(unsigned int offlineChannel) const
  {
  if (offlineChannel > fNChans)
    {      
      throw cet::exception("PdspChannelMapService") << "Offline Channel Number out of range: " << offlineChannel << "\n"; 
    }
  };
};

DECLARE_ART_SERVICE(dune::PdspChannelMapService, LEGACY)

#endif
