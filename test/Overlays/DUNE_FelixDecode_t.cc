 #include <stdint.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "art/Framework/Principal/Handle.h"
#include "artdaq-core/Data/Fragment.hh"
#include "canvas/Utilities/InputTag.h"
#include "dune-raw-data/Overlays/FelixDecode.hh"
#include "dune-raw-data/Overlays/FelixFragment.hh"
#include "dune-raw-data/Overlays/FelixReordererFacility.hh"
#include "gallery/Event.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#define BOOST_TEST_MODULE(MilliSlice_t)
#ifdef HAVE_CANVAS
#include "cetlib/quiet_unit_test.hpp"
#else
#include "boost/test/auto_unit_test.hpp"
#endif

BOOST_AUTO_TEST_SUITE(FelixDecode_test)

BOOST_AUTO_TEST_CASE(BaselineTest) {
  std::string filename = "/data0/np04_raw_run003311_0001_dl1.root.copied";
      std::string outputDestination = "noise_records/test";

  // Initialise a decoder object from the file.
  dune::FelixDecoder flxdec(filename);

  for(unsigned i = 0; i < 1000 /* flxdec.total_fragments() */; ++i) {
    // Save fragment to file.
    std::string frag_number = std::to_string(i);
    frag_number = std::string(4 - frag_number.size(), '0') + frag_number;

    std::string filename = "run/run003311_0001_" + frag_number + ".dat";
    // std::cout << "Writing to " << filename << ".\n";
    std::ofstream ofile(filename);
    artdaq::Fragment frag = flxdec.Fragment(i);
    ofile.write(reinterpret_cast<char const*>(frag.dataBeginBytes())+8*4, frag.dataSizeBytes()-8*4);
    ofile.close();

    dune::FelixFragment flxfrag(frag);
    artdaq::Fragment reordfrag = dune::FelixReorder(frag.dataBeginBytes()+8*4,
                               (frag.dataSizeBytes()-8*4) / 464);

    std::string reordfilename = filename.insert(filename.size()-4, "-reordered");
    // std::cout << "Writing to " << reordfilename << ".\n";
    std::ofstream reordfile(reordfilename);
    reordfile.write(reinterpret_cast<char const*>(reordfrag.dataBeginBytes()),
                    reordfrag.dataSizeBytes());
    reordfile.close();

    // std::cout << i << '\t' << (unsigned)flxfrag.crate_no() << '\n';
  }

 // flxdec.check_all_timestamps();
 // flxdec.check_all_CCCs();
 // flxdec.check_all_IDs();

  // artdaq::Fragment frag = flxdec.Fragment(0);
  // dune::FelixFragment flxfrag(frag);
  // std::cout << (unsigned)flxfrag.crate_no() << "   " << (unsigned)flxfrag.slot_no() << "   "
  //           << (unsigned)flxfrag.fiber_no() << '\n';

  // Print RMS values to file.
  // flxdec.calculateNoiseRMS(outputDestination);

  // for(unsigned i = 0; i < flxdec.total_fragments(); ++i) {
  //   artdaq::Fragment frag = flxdec.Fragment(i);
  //   dune::FelixFragment flxfrag(frag);
  //   std::cout << i << "     " << (unsigned)flxfrag.crate_no() << "  "
  //             << (unsigned)flxfrag.slot_no() << "   "
  //             << (unsigned)flxfrag.fiber_no() << '\n';
  // }

  // // Print frames 4,5,6 of the 8th fragment.
  // flxdec.printFrames(3, 5, 7);
}

BOOST_AUTO_TEST_SUITE_END()

#pragma GCC diagnostic pop
