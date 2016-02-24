///////////////////////////////////////////////////////////////////////////////////////////////////
// Class:       ChannelMappingService
// Module type: service
// File:        ChannelMappingService_service.cc
// Author:      Mike Wallbank (m.wallbank@sheffield.ac.uk), February 2016
//              Based on the service written for use in LArSoft by David Adams
//
// Implementation of online-offline channel mapping reading from a file.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ChannelMappingService.h"

// Bad channel value
unsigned int bad() {
  unsigned int val = std::numeric_limits<unsigned int>::max();
  return val;
}

ChannelMappingService::ChannelMappingService(fhicl::ParameterSet const& pset) {

  fChannelMapFile = pset.get<std::string>("ChannelMapFile");

  std::ifstream inFile(fChannelMapFile, std::ios::in);
  std::string line;

  if (!inFile) {
    std::cout << "Input file " << fChannelMapFile << " not found" << std::endl;
    throw cet::exception("File not found");
  }

  while (std::getline(inFile,line)) {
    unsigned int onlineChannel, rce, rcechannel, apa, plane, offlineChannel;
    std::stringstream linestream(line);
    linestream >> onlineChannel >> rce >> rcechannel >> apa >> plane >> offlineChannel;
    fOfflineToOnline[offlineChannel] = onlineChannel;
    fOnlineToOffline[onlineChannel] = offlineChannel;
    fAPAMap[onlineChannel] = apa;
    fPlaneMap[onlineChannel] = plane;
  }

  inFile.close();

}

ChannelMappingService::ChannelMappingService(fhicl::ParameterSet const& pset, art::ActivityRegistry&) : ChannelMappingService(pset) {
}

unsigned int ChannelMappingService::Offline(unsigned int onlineChannel) const {

  unsigned int offlineChannel = bad();

  if (fOnlineToOffline.count(offlineChannel) == 0) {
    std::cout << "Error: no offline channel mapped to online channel " << onlineChannel << std::endl;
    throw cet::exception("Online channel not found");
  }

  return fOnlineToOffline.at(offlineChannel);

}

unsigned int ChannelMappingService::Online(unsigned int offlineChannel) const {

  unsigned int onlineChannel = bad();

  if (fOfflineToOnline.count(onlineChannel) == 0) {
    std::cout << "Error: no online channel mapped to offline channel " << offlineChannel << std::endl;
    throw cet::exception("Offline channel not found");
  }

  return fOfflineToOnline.at(onlineChannel);

}

unsigned int ChannelMappingService::APAFromOnlineChannel(unsigned int onlineChannel) const {

  if (fAPAMap.count(onlineChannel) == 0) {
    std::cout << "Error: no APA information for online channel " << onlineChannel << std::endl;
    throw cet::exception("APA information not found");
  }

  return fAPAMap.at(onlineChannel);

}

unsigned int ChannelMappingService::APAFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->APAFromOnlineChannel(onlineChannel);

}

unsigned int ChannelMappingService::PlaneFromOnlineChannel(unsigned int onlineChannel) const {

  if (fPlaneMap.count(onlineChannel) == 0) {
    std::cout << "Error: no plane information for online channel " << onlineChannel << std::endl;
    throw cet::exception("Plane information not found");
  }

  return fPlaneMap.at(onlineChannel);

}

unsigned int ChannelMappingService::PlaneFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->PlaneFromOnlineChannel(onlineChannel);

}

DEFINE_ART_SERVICE(ChannelMappingService)
