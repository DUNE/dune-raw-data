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
  if (argc < 3) {
    std::cout << "ERROR: need both inputfile(s) and destination.\n";
    return 1;
  }

  // Number of files to be considered.
  const unsigned total_files = 10;

  // The user has to input both an input file and a destination.
  std::vector<std::string> filenames;
  for(unsigned file = 1; file < std::min<unsigned>(total_files, argc-1); ++file) {
    filenames.push_back(argv[file]);
  }
  std::string destination = argv[argc-1];
  std::string print = "";
  if (argc == 4) {
    print = argv[3];
  }

  // Initialise a decoder object from the file.
  dune::FelixDecoder flxdec(filenames);

  // Check fragment length here for now.
  artdaq::Fragment frag = flxdec.Fragment(0);
  // artdaq::Fragment newfrag;
  // newfrag.setTimestamp(frag.timestamp());
  // newfrag.resize(frag.dataSizeBytes());
  // memcpy(newfrag.dataBeginBytes(), frag.dataBeginBytes(), frag.dataSizeBytes());

  // dune::FelixFragment::Metadata newmeta = {0xabc, 1, 0, 0, 6024, 500, 6000};
  // newfrag.setMetadata(newmeta);

  dune::FelixFragment::Metadata meta = *frag.metadata<dune::FelixFragment::Metadata>();
  std::cout << "METADATA: " << (unsigned)meta.num_frames << "   "
            << (unsigned)meta.reordered << "   " << (unsigned)meta.compressed
            << "   " << (unsigned)meta.offset_frames << "   "
            << (unsigned)meta.window_frames << '\n';
  dune::FelixFragment flxfrag(frag);
  if(flxfrag.total_frames() < 6000 || flxfrag.total_frames() > 6036) {
    std::cout << "WARNING: first fragment has a strange size: " << flxfrag.total_frames() << ".\n";
  }

  // // Print identifiers.
  // for (unsigned f = 0; f < flxdec.total_fragments(); ++f) {
  //   artdaq::Fragment frag = flxdec.Fragment(f);
  //   dune::FelixFragment flxfrag(frag);
  //   std::cout << (unsigned)flxfrag.slot_no() << ' ';
  // }

  // flxdec.check_all_timestamps();
  // flxdec.check_all_CCCs();
  // flxdec.check_all_IDs();

  // Print RMS values to file.
  flxdec.analyse(destination);

  return 0;
}
