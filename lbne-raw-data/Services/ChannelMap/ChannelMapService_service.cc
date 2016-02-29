///////////////////////////////////////////////////////////////////////////////////////////////////
// Class:       ChannelMapService
// Module type: service
// File:        ChannelMapService_service.cc
// Author:      Mike Wallbank (m.wallbank@sheffield.ac.uk), February 2016
//              Based on the service written for use in LArSoft by David Adams
//
// Implementation of online-offline channel mapping reading from a file.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "ChannelMapService.h"

// Bad channel value
unsigned int bad() {
  unsigned int val = std::numeric_limits<unsigned int>::max();
  return val;
}

lbne::ChannelMapService::ChannelMapService(fhicl::ParameterSet const& pset) {

  std::string channelMapFile = pset.get<std::string>("FileName");

  std::string fullname;
  cet::search_path sp("FW_SEARCH_PATH");
  sp.find_file(channelMapFile, fullname);

  if (fullname.empty()) {
    std::cout << "Input file " << channelMapFile << " not found" << std::endl;
    throw cet::exception("File not found");
  }

  std::ifstream inFile(fullname, std::ios::in);
  std::string line;

  while (std::getline(inFile,line)) {
    unsigned int onlineChannel, rce, rcechannel, apa, plane, offlineChannel;
    std::stringstream linestream(line);
    linestream >> onlineChannel >> rce >> rcechannel >> apa >> plane >> offlineChannel;
    fOfflineToOnline[offlineChannel] = onlineChannel;
    fOnlineToOffline[onlineChannel] = offlineChannel;
    fAPAMap[onlineChannel] = apa;
    fPlaneMap[onlineChannel] = plane;
    fRCEMap[onlineChannel] = rce;
  }

  inFile.close();

}

lbne::ChannelMapService::ChannelMapService(fhicl::ParameterSet const& pset, art::ActivityRegistry&) : ChannelMapService(pset) {
}

unsigned int lbne::ChannelMapService::Offline(unsigned int onlineChannel) const {

  unsigned int offlineChannel = bad();

  if (fOnlineToOffline.count(onlineChannel) == 0) {
    std::cout << "Error: no offline channel mapped to online channel " << onlineChannel << std::endl;
    throw cet::exception("Online channel not found");
  }
  else
    offlineChannel = fOnlineToOffline.at(onlineChannel);

  return offlineChannel;

}

unsigned int lbne::ChannelMapService::Online(unsigned int offlineChannel) const {

  unsigned int onlineChannel = bad();

  if (fOfflineToOnline.count(offlineChannel) == 0) {
    std::cout << "Error: no online channel mapped to offline channel " << offlineChannel << std::endl;
    throw cet::exception("Offline channel not found");
  }
  else
    onlineChannel = fOfflineToOnline.at(offlineChannel);

  return onlineChannel;

}

unsigned int lbne::ChannelMapService::APAFromOnlineChannel(unsigned int onlineChannel) const {

  if (fAPAMap.count(onlineChannel) == 0) {
    std::cout << "Error: no APA information for online channel " << onlineChannel << std::endl;
    throw cet::exception("APA information not found");
  }

  return fAPAMap.at(onlineChannel);

}

unsigned int lbne::ChannelMapService::APAFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->APAFromOnlineChannel(onlineChannel);

}

unsigned int lbne::ChannelMapService::PlaneFromOnlineChannel(unsigned int onlineChannel) const {

  if (fPlaneMap.count(onlineChannel) == 0) {
    std::cout << "Error: no plane information for online channel " << onlineChannel << std::endl;
    throw cet::exception("Plane information not found");
  }

  return fPlaneMap.at(onlineChannel);

}

unsigned int lbne::ChannelMapService::PlaneFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->PlaneFromOnlineChannel(onlineChannel);

}

unsigned int lbne::ChannelMapService::RCEFromOnlineChannel(unsigned int onlineChannel) const {

  if (fRCEMap.count(onlineChannel) == 0) {
    std::cout << "Error: no RCE information for online channel " << onlineChannel << std::endl;
    throw cet::exception("RCE information not found");
  }

  return fRCEMap.at(onlineChannel);

}

unsigned int lbne::ChannelMapService::RCEFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->RCEFromOnlineChannel(onlineChannel);

}

DEFINE_ART_SERVICE(lbne::ChannelMapService)
