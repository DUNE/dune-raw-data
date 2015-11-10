#include "lbne-raw-data/Overlays/Utilities.hh"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include <sstream>
#include <bitset>

namespace lbne {

void display_bits(void* memstart, size_t nbytes, std::string sourcename) {

  std::stringstream bitstr;
  bitstr << "The " << nbytes << "-byte chunk of memory beginning at " << static_cast<void*>(memstart) << " is : ";

  for(unsigned int i = 0; i < nbytes; i++) {
    bitstr << std::bitset<8>(*((lbne::reinterpret_cast_checked<uint8_t*>(memstart))+i)) << " ";
  }

  mf::LogInfo(sourcename.c_str()) << bitstr.str();
}

}