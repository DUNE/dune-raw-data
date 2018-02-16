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
    unsigned int crateNo, slotNo, fiberNo, FEMBChannel, slotID, fiberID, chipNo, chipChannel, asicNo, asicChannel, planeType, offlineChannel;
    std::stringstream linestream(line);
    linestream >> crateNo >> slotNo >> fiberNo>> FEMBChannel >> slotID >> fiberID >> chipNo >> chipChannel >> asicNo >> asicChannel >> planeType >> offlineChannel;
    key_csfc csfc(crateNo, slotNo, fiberNo, FEMBChannel);
    fCsfcToOffline[csfc] = offlineChannel;    
    fAPAMap[offlineChannel] = crateNo; 
    fWIBMap[offlineChannel] = slotNo; 
    fFEMBMap[offlineChannel] = fiberNo; 
    fFEMBChannelMap[offlineChannel] = FEMBChannel; 
    fSlotIdMap[offlineChannel] = slotID; 
    fFiberIdMap[offlineChannel] = fiberID; 
    fChipMap[offlineChannel] = chipNo; 
    fChipChannelMap[offlineChannel] = chipChannel;
    fASICMap[offlineChannel] = asicNo;
    fASICChannelMap[offlineChannel] = asicChannel;
    fPlaneMap[offlineChannel] = planeType;
  }
 
  inFile.close();
}

dune::PdspChannelMapService::PdspChannelMapService(fhicl::ParameterSet const& pset, art::ActivityRegistry&) : PdspChannelMapService(pset) {
}

unsigned int dune::PdspChannelMapService::GetOfflineNumberFromDetectorElements(unsigned int crate, unsigned int slot, unsigned int fiber, unsigned int fembchannel) const {
	unsigned int offlineChannel = bad();
	key_csfc csfc(crate, slot, fiber, fembchannel);
	offlineChannel = fCsfcToOffline.find(csfc)->second;
	return offlineChannel;
}

unsigned int dune::PdspChannelMapService::APAFromOfflineChannel(unsigned int offlineChannel) const {

  if (fAPAMap.count(offlineChannel) == 0) {
    std::cout << "Error: no APA/Crate information for offline channel " << offlineChannel << std::endl;
    throw cet::exception("APA/Crate information not found");
  }
  std::cout<<"offline channel = "<<offlineChannel<<std::endl;
  return fAPAMap.at(offlineChannel);
}

unsigned int dune::PdspChannelMapService::WIBFromOfflineChannel(unsigned int offlineChannel) const {

  if (fWIBMap.count(offlineChannel) == 0) {
    std::cout << "Error: no WIB/Slot information for offline channel " << offlineChannel << std::endl;
    throw cet::exception("WIB/Slot information not found");
  }
  std::cout<<"offline channel = "<<offlineChannel<<std::endl;
  return fWIBMap.at(offlineChannel);
}

unsigned int dune::PdspChannelMapService::FEMBFromOfflineChannel(unsigned int offlineChannel) const {

  if (fFEMBMap.count(offlineChannel) == 0) {
    std::cout << "Error: no FEMB/Fiber information for offline channel " << offlineChannel << std::endl;
    throw cet::exception("FEMB/Fiber information not found");
  }
  std::cout<<"offline channel = "<<offlineChannel<<std::endl;
  return fFEMBMap.at(offlineChannel);
}

unsigned int dune::PdspChannelMapService::FEMBChannelFromOfflineChannel(unsigned int offlineChannel) const {

  if (fFEMBChannelMap.count(offlineChannel) == 0) {
    std::cout << "Error: no FEMB channel information for offline channel " << offlineChannel << std::endl;
    throw cet::exception("FEMB channel information not found");
  }
  std::cout<<"offline channel = "<<offlineChannel<<std::endl;
  return fFEMBChannelMap.at(offlineChannel);
}

unsigned int dune::PdspChannelMapService::SlotIdFromOfflineChannel(unsigned int offlineChannel) const {

  if (fSlotIdMap.count(offlineChannel) == 0) {
    std::cout << "Error: no Slot ID information for offline channel " << offlineChannel << std::endl;
    throw cet::exception("Slot ID information not found");
  }
  std::cout<<"offline channel = "<<offlineChannel<<std::endl;
  return fSlotIdMap.at(offlineChannel);
}

unsigned int dune::PdspChannelMapService::FiberIdFromOfflineChannel(unsigned int offlineChannel) const {

  if (fFiberIdMap.count(offlineChannel) == 0) {
    std::cout << "Error: no Fiber ID information for offline channel " << offlineChannel << std::endl;
    throw cet::exception("Fiber ID information not found");
  }
  std::cout<<"offline channel = "<<offlineChannel<<std::endl;
  return fFiberIdMap.at(offlineChannel);
}

unsigned int dune::PdspChannelMapService::ChipFromOfflineChannel(unsigned int offlineChannel) const {

  if (fChipMap.count(offlineChannel) == 0) {
    std::cout << "Error: no Chip information for offline channel " << offlineChannel << std::endl;
    throw cet::exception("Chip information not found");
  }
  std::cout<<"offline channel = "<<offlineChannel<<std::endl;
  return fChipMap.at(offlineChannel);
}

unsigned int dune::PdspChannelMapService::ChipChannelFromOfflineChannel(unsigned int offlineChannel) const {

  if (fChipChannelMap.count(offlineChannel) == 0) {
    std::cout << "Error: no Chip channel information for offline channel " << offlineChannel << std::endl;
    throw cet::exception("Chip channel information not found");
  }
  std::cout<<"offline channel = "<<offlineChannel<<std::endl;
  return fChipChannelMap.at(offlineChannel);
}

unsigned int dune::PdspChannelMapService::ASICFromOfflineChannel(unsigned int offlineChannel) const {

  if (fASICMap.count(offlineChannel) == 0) {
    std::cout << "Error: no ASIC information for offline channel " << offlineChannel << std::endl;
    throw cet::exception("ASIC information not found");
  }
  std::cout<<"offline channel = "<<offlineChannel<<std::endl;
  return fASICMap.at(offlineChannel);
}

unsigned int dune::PdspChannelMapService::ASICChannelFromOfflineChannel(unsigned int offlineChannel) const {

  if (fASICChannelMap.count(offlineChannel) == 0) {
    std::cout << "Error: no ASIC channel information for offline channel " << offlineChannel << std::endl;
    throw cet::exception("ASIC channel information not found");
  }
  std::cout<<"offline channel = "<<offlineChannel<<std::endl;
  return fASICChannelMap.at(offlineChannel);
}

unsigned int dune::PdspChannelMapService::PlaneFromOfflineChannel(unsigned int offlineChannel) const {

  if (fPlaneMap.count(offlineChannel) == 0) {
    std::cout << "Error: no plane information for offline channel " << offlineChannel << std::endl;
    throw cet::exception("Plane information not found");
  }

  return fPlaneMap.at(offlineChannel);

}

DEFINE_ART_SERVICE(dune::PdspChannelMapService)
