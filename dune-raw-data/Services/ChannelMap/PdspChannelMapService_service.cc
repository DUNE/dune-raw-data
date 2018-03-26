///////////////////////////////////////////////////////////////////////////////////////////////////
// Class:       PdspChannelMapService
// Module type: service
// File:        PdspChannelMapService.h
// Author:      Jingbo Wang (jiowang@ucdavis.edu), February 2018
//
// Implementation of hardware-offline channel mapping reading from a file.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PdspChannelMapService.h"

// Bad channel value
unsigned int bad() {
  unsigned int val = std::numeric_limits<unsigned int>::max();
  return val;
}

dune::PdspChannelMapService::PdspChannelMapService(fhicl::ParameterSet const& pset) {

  std::string channelMapFile = pset.get<std::string>("FileName");

  std::string fullname;
  cet::search_path sp("FW_SEARCH_PATH");
  sp.find_file(channelMapFile, fullname);

  if (fullname.empty()) {
    std::cout << "Input file " << channelMapFile << " not found" << std::endl;
    throw cet::exception("File not found");
  }
  else
    std::cout << "TPC Channel Map: Building TPC map from file " << channelMapFile << std::endl;

  std::ifstream inFile(fullname, std::ios::in);
  std::string line;
  int counter = 0;

  while (std::getline(inFile,line)) {
    counter++;
    unsigned int crateNo, slotNo, fiberNo, FEMBChannel, StreamChannel, slotID, fiberID, chipNo, chipChannel, asicNo, asicChannel, planeType, offlineChannel;
    std::stringstream linestream(line);
    linestream >> crateNo >> slotNo >> fiberNo>> FEMBChannel >> StreamChannel >> slotID >> fiberID >> chipNo >> chipChannel >> asicNo >> asicChannel >> planeType >> offlineChannel;

    // fill lookup tables.  Throw an exception if any number is out of expected bounds.
    // checking for negative values produces compiler warnings as these are unsigned ints

    if (offlineChannel >= fNChans)
      {
	throw cet::exception("PdspChannelMapService") << "Ununderstood Offline Channel: " << offlineChannel << "\n";
      }
    if (crateNo >= fNCrates)
      {
	throw cet::exception("PdspChannelMapService") << "Ununderstood Crate Number: " << crateNo << "\n";
      }
    if (slotNo >= fNSlots)
      {
	throw cet::exception("PdspChannelMapService") << "Ununderstood Slot Number: " << slotNo << "\n";
      }
    if (fiberNo >= fNFibers)
      {
	throw cet::exception("PdspChannelMapService") << "Ununderstood Fiber Number: " << fiberNo << "\n";
      }
    if (StreamChannel >= fNFEMBChans)
      {
	throw cet::exception("PdspChannelMapService") << "Ununderstood FEMB (Stream) Channel Number: " << StreamChannel << "\n";
      }

    farrayCsfcToOffline[crateNo][slotNo][fiberNo][StreamChannel] = offlineChannel;
    fvAPAMap[offlineChannel] = crateNo; 
    fvWIBMap[offlineChannel] = slotNo; 
    fvFEMBMap[offlineChannel] = fiberNo; 
    fvFEMBChannelMap[offlineChannel] = FEMBChannel; 
    fvStreamChannelMap[offlineChannel] = StreamChannel;
    fvSlotIdMap[offlineChannel] = slotID; 
    fvFiberIdMap[offlineChannel] = fiberID; 
    fvChipMap[offlineChannel] = chipNo; 
    fvChipChannelMap[offlineChannel] = chipChannel;
    fvASICMap[offlineChannel] = asicNo;
    fvASICChannelMap[offlineChannel] = asicChannel;
    fvPlaneMap[offlineChannel] = planeType;

  }
  inFile.close();

  fHaveWarnedAboutBadCrateNumber = false;
  fHaveWarnedAboutBadSlotNumber = false;
  fHaveWarnedAboutBadFiberNumber = false;
}

dune::PdspChannelMapService::PdspChannelMapService(fhicl::ParameterSet const& pset, art::ActivityRegistry&) : PdspChannelMapService(pset) {
}

unsigned int dune::PdspChannelMapService::GetOfflineNumberFromDetectorElements(unsigned int crate, unsigned int slot, unsigned int fiber, unsigned int streamchannel) {

  unsigned int offlineChannel;
  unsigned int lcrate = crate;
  unsigned int lslot = slot;
  unsigned int lfiber = fiber;

  if (crate >= fNCrates)
    {
      if (!fHaveWarnedAboutBadCrateNumber)
	{
	  mf::LogWarning("PdspChannelMapService: Crate Number too high, using crate number zero as a fallback.  Ununderstood crate number: ") << crate;
	  fHaveWarnedAboutBadCrateNumber = true;
	}
      lcrate = 0;
    }

  if (slot >= fNSlots)
    {
      if (!fHaveWarnedAboutBadSlotNumber)
	{
	  mf::LogWarning("PdspChannelMapService: Slot Number too high, using slot number zero as a fallback.  Ununderstood slot number: ") << slot;
	  fHaveWarnedAboutBadSlotNumber = true;
	}
      lslot = 0;
    }

  if (fiber >= fNFibers)
    {
      if (!fHaveWarnedAboutBadFiberNumber)
	{
	  mf::LogWarning("PdspChannelMapService: Fiber Number too high, using fiber number zero as a fallback.  Ununderstood fiber number: ") << fiber;
	  fHaveWarnedAboutBadFiberNumber = true;
	}
      lfiber = 0;
    }

  if (streamchannel >= fNFEMBChans)
    {
      throw cet::exception("PdspChannelMapService") << "Ununderstood Stream (FEMB) chan: " 
						    << crate << " " << slot << " " << fiber << " " << streamchannel << "\n";
    }

  offlineChannel = farrayCsfcToOffline[lcrate][lslot][lfiber][streamchannel];
  return offlineChannel;
}

unsigned int dune::PdspChannelMapService::APAFromOfflineChannel(unsigned int offlineChannel) const {
  check_offline_channel(offlineChannel);
  return fvAPAMap[offlineChannel];
}

unsigned int dune::PdspChannelMapService::WIBFromOfflineChannel(unsigned int offlineChannel) const {
  check_offline_channel(offlineChannel);
  return fvWIBMap[offlineChannel];

}

unsigned int dune::PdspChannelMapService::FEMBFromOfflineChannel(unsigned int offlineChannel) const {
  check_offline_channel(offlineChannel);
  return fvFEMBMap[offlineChannel];
}


unsigned int dune::PdspChannelMapService::FEMBChannelFromOfflineChannel(unsigned int offlineChannel) const {
  check_offline_channel(offlineChannel);
  return fvFEMBChannelMap[offlineChannel];
}

unsigned int dune::PdspChannelMapService::StreamChannelFromOfflineChannel(unsigned int offlineChannel) const {
  check_offline_channel(offlineChannel);
  return fvStreamChannelMap[offlineChannel];
}

unsigned int dune::PdspChannelMapService::SlotIdFromOfflineChannel(unsigned int offlineChannel) const {
  check_offline_channel(offlineChannel);
  return fvSlotIdMap[offlineChannel];
}

unsigned int dune::PdspChannelMapService::FiberIdFromOfflineChannel(unsigned int offlineChannel) const {
  check_offline_channel(offlineChannel);
  return fvFiberIdMap[offlineChannel];
}

unsigned int dune::PdspChannelMapService::ChipFromOfflineChannel(unsigned int offlineChannel) const {
  check_offline_channel(offlineChannel);
  return fvChipMap[offlineChannel];
}

unsigned int dune::PdspChannelMapService::ChipChannelFromOfflineChannel(unsigned int offlineChannel) const {
  check_offline_channel(offlineChannel);
  return fvChipChannelMap[offlineChannel];
}

unsigned int dune::PdspChannelMapService::ASICFromOfflineChannel(unsigned int offlineChannel) const {
  check_offline_channel(offlineChannel);
  return fvASICMap[offlineChannel];
}

unsigned int dune::PdspChannelMapService::ASICChannelFromOfflineChannel(unsigned int offlineChannel) const {
  check_offline_channel(offlineChannel);
  return fvASICChannelMap[offlineChannel];
}

unsigned int dune::PdspChannelMapService::PlaneFromOfflineChannel(unsigned int offlineChannel) const {
  check_offline_channel(offlineChannel);
  return fvFEMBMap[offlineChannel];
}

DEFINE_ART_SERVICE(dune::PdspChannelMapService)
