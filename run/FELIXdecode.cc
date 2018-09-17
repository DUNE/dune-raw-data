#include "dune-raw-data/Overlays/FelixDecode.hh"
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "art/Framework/Principal/Handle.h"
#include "artdaq-core/Data/Fragment.hh"
#include "canvas/Utilities/InputTag.h"
#include "dune-raw-data/Overlays/FelixFragment.hh"
#include "gallery/Event.h"

int main(int argc, char *argv[]) {
  // Checking of input arguments.
  if (argc > 4) {
    std::cout << "WARNING: only the first three input arguments are accepted.\n";
  } else if (argc < 3) {
    std::cout << "ERROR: need both inputfile and destination.\n";
    return 1;
  }

  // The user has to input both an input file and a destination.
  std::string filename = argv[1];
  std::string destination = argv[2];
  std::string print = "";
  if (argc == 4) {
    print = argv[3];
  }

  // Initialise a decoder object from the file.
  dune::FelixDecoder flxdec(filename);

  // Check fragment length here for now.
  artdaq::Fragment frag = flxdec.Fragment(0);
  dune::FelixFragment::Metadata meta = *frag.metadata<dune::FelixFragment::Metadata>();
  std::cout << "METADATA: " << (unsigned)meta.num_frames << "   " << (unsigned)meta.reordered << "   " << (unsigned)meta.compressed << '\n';
  dune::FelixFragment flxfrag(frag);
  if(flxfrag.total_frames() < 6000 || flxfrag.total_frames() > 6036) {
    std::cout << "WARNING: first fragment has a strange size: " << flxfrag.total_frames() << ".\n";
  }

  flxdec.check_all_timestamps();
  flxdec.check_all_CCCs();
  flxdec.check_all_IDs();

  // // Print RMS values to file.
  // if(print != "noNoise") {
  //   flxdec.calculateNoiseRMS(destination);
  // }

  return 0;
}
