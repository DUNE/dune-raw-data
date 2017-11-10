///////////////////////////////////////////////////////////////////////////////////////////////////
// Class:       PdChannelMapService
// Module type: service
// File:        PdChannelMapService_service.cc
// Author:      Mike Wallbank (m.wallbank@sheffield.ac.uk), February 2016
//              Based on the service written for use in LArSoft by David Adams
//
// Implementation of online-offline channel mapping reading from a file.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PdChannelMapService.h"

// Bad channel value
unsigned int bad() {
  unsigned int val = std::numeric_limits<unsigned int>::max();
  return val;
}

dune::PdChannelMapService::PdChannelMapService(fhicl::ParameterSet const& pset) {

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

  while (std::getline(inFile,line)) {
    unsigned int onlineChannel, rce, rcechannel, regulator, regulatorpin, asic, asicchannel, apa, plane, offlineChannel;
    std::stringstream linestream(line);
    linestream >> onlineChannel >> rce >> rcechannel >> regulator >> regulatorpin >> asic >> asicchannel >> apa >> plane >> offlineChannel;
    fRCEToOnline[rce][rcechannel] = onlineChannel;
    fOfflineToOnline[offlineChannel] = onlineChannel;
    fOnlineToOffline[onlineChannel] = offlineChannel;
    fAPAMap[onlineChannel] = apa;
    fPlaneMap[onlineChannel] = plane;
    fRCEMap[onlineChannel] = rce;
    fRCEChannelMap[onlineChannel] = rcechannel;
    fRegulatorMap[onlineChannel] = regulator;
    fRegulatorPinMap[onlineChannel] = regulatorpin;
    fASICMap[onlineChannel] = asic;
    fASICChannelMap[onlineChannel] = asicchannel;
  }

  inFile.close();

}

dune::PdChannelMapService::PdChannelMapService(fhicl::ParameterSet const& pset, art::ActivityRegistry&) : PdChannelMapService(pset) {
}

unsigned int dune::PdChannelMapService::OnlineFromRCE(unsigned int rceID, unsigned int rceChannel) const {
  unsigned int onlineChannel = bad();
  if (fRCEToOnline.count(rceID) == 0 || fRCEToOnline.at(rceID).count(rceChannel) == 0) {
    std::cout << "Error: no online channel mapped to RCE ID " << rceID << "or its internal channel " << rceChannel << std::endl;
    throw cet::exception("RCE ID or RCE channel not found");
  }
  else
    onlineChannel = fRCEToOnline.at(rceID).at(rceChannel);

  return onlineChannel;  
}

unsigned int dune::PdChannelMapService::Offline(unsigned int onlineChannel) const {

  unsigned int offlineChannel = bad();

  if (fOnlineToOffline.count(onlineChannel) == 0) {
    std::cout << "Error: no offline channel mapped to online channel " << onlineChannel << std::endl;
    throw cet::exception("Online channel not found");
  }
  else
    offlineChannel = fOnlineToOffline.at(onlineChannel);

  return offlineChannel;

}

unsigned int dune::PdChannelMapService::Online(unsigned int offlineChannel) const {

  unsigned int onlineChannel = bad();

  if (fOfflineToOnline.count(offlineChannel) == 0) {
    std::cout << "Error: no online channel mapped to offline channel " << offlineChannel << std::endl;
    throw cet::exception("Offline channel not found");
  }
  else
    onlineChannel = fOfflineToOnline.at(offlineChannel);

  return onlineChannel;

}

unsigned int dune::PdChannelMapService::APAFromOnlineChannel(unsigned int onlineChannel) const {

  if (fAPAMap.count(onlineChannel) == 0) {
    std::cout << "Error: no APA information for online channel " << onlineChannel << std::endl;
    throw cet::exception("APA information not found");
  }

  return fAPAMap.at(onlineChannel);

}

unsigned int dune::PdChannelMapService::APAFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->APAFromOnlineChannel(onlineChannel);

}

unsigned int dune::PdChannelMapService::PlaneFromOnlineChannel(unsigned int onlineChannel) const {

  if (fPlaneMap.count(onlineChannel) == 0) {
    std::cout << "Error: no plane information for online channel " << onlineChannel << std::endl;
    throw cet::exception("Plane information not found");
  }

  return fPlaneMap.at(onlineChannel);

}

unsigned int dune::PdChannelMapService::PlaneFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->PlaneFromOnlineChannel(onlineChannel);

}

unsigned int dune::PdChannelMapService::RCEFromOnlineChannel(unsigned int onlineChannel) const {

  if (fRCEMap.count(onlineChannel) == 0) {
    std::cout << "Error: no RCE information for online channel " << onlineChannel << std::endl;
    throw cet::exception("RCE information not found");
  }

  return fRCEMap.at(onlineChannel);

}

unsigned int dune::PdChannelMapService::RCEFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->RCEFromOnlineChannel(onlineChannel);

}

unsigned int dune::PdChannelMapService::RCEChannelFromOnlineChannel(unsigned int onlineChannel) const {

  if (fRCEChannelMap.count(onlineChannel) == 0) {
    std::cout << "Error: no RCE channel information for online channel " << onlineChannel << std::endl;
    throw cet::exception("RCE channel information not found");
  }

  return fRCEChannelMap.at(onlineChannel);

}

unsigned int dune::PdChannelMapService::RCEChannelFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->RCEChannelFromOnlineChannel(onlineChannel);

}

unsigned int dune::PdChannelMapService::RegulatorFromOnlineChannel(unsigned int onlineChannel) const {

  if (fRegulatorMap.count(onlineChannel) == 0) {
    std::cout << "Error: no regulator information for online channel " << onlineChannel << std::endl;
    throw cet::exception("Regulator information not found");
  }

  return fRegulatorMap.at(onlineChannel);

}

unsigned int dune::PdChannelMapService::RegulatorFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->RegulatorFromOnlineChannel(onlineChannel);

}

unsigned int dune::PdChannelMapService::RegulatorPinFromOnlineChannel(unsigned int onlineChannel) const {

  if (fRegulatorPinMap.count(onlineChannel) == 0) {
    std::cout << "Error: no regulator pin information for online channel " << onlineChannel << std::endl;
    throw cet::exception("Regulator pin information not found");
  }

  return fRegulatorPinMap.at(onlineChannel);

}

unsigned int dune::PdChannelMapService::RegulatorPinFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->RegulatorPinFromOnlineChannel(onlineChannel);

}

unsigned int dune::PdChannelMapService::ASICFromOnlineChannel(unsigned int onlineChannel) const {

  if (fASICMap.count(onlineChannel) == 0) {
    std::cout << "Error: no ASIC information for online channel " << onlineChannel << std::endl;
    throw cet::exception("ASIC information not found");
  }

  return fASICMap.at(onlineChannel);

}

unsigned int dune::PdChannelMapService::ASICFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->ASICFromOnlineChannel(onlineChannel);

}

unsigned int dune::PdChannelMapService::ASICChannelFromOnlineChannel(unsigned int onlineChannel) const {

  if (fASICChannelMap.count(onlineChannel) == 0) {
    std::cout << "Error: no ASIC channel information for online channel " << onlineChannel << std::endl;
    throw cet::exception("ASIC channel information not found");
  }

  return fASICChannelMap.at(onlineChannel);

}

unsigned int dune::PdChannelMapService::ASICChannelFromOfflineChannel(unsigned int offlineChannel) const {

  unsigned int onlineChannel = this->Online(offlineChannel);
  return this->ASICChannelFromOnlineChannel(onlineChannel);

}

DEFINE_ART_SERVICE(dune::PdChannelMapService)