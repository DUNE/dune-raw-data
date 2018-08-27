#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "art/Framework/Principal/Handle.h"
#include "artdaq-core/Data/Fragment.hh"
#include "canvas/Utilities/InputTag.h"
#include "dune-raw-data/Overlays/FelixDecode.hh"
#include "dune-raw-data/Overlays/FelixFragment.hh"
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
  std::vector<std::string> filenames;
  std::string filename = "/data3/np04_raw_run002644_1_dl2.root.copied";
  filenames.push_back(filename);
  // for(unsigned i = 0; i < 15; ++i) {
  //  std::string filename = filename_base;
  //  filename.insert(filename.find("dl1"), std::to_string(i)+"_");
  //  std::cout << filename << '\n';
  //  filenames.push_back(filename);
  //}

  dune::FelixDecoder flxdec(filenames);
  flxdec.check_timestamps(); // Checks increments by 25 and reports otherwise.
  flxdec.check_CCC(); // Checks increments by 1 and correspondence between CCCs.

  for(unsigned i = 0; i < flxdec.total_fragments(); ++i) {
    // You have to copy fragments into new containers if you want to use the overlay on them.
    artdaq::Fragment frag(flxdec.Fragment(i));
    dune::FelixFragment flxfrag(frag);
    std::cout << frag.timestamp() - flxfrag.timestamp() << '\n';
  }

  // Print frames 4,5,6 of the 8th fragment.
  flxdec.printFrames(3, 5, 7);

  // // File(s) to be loaded.
  // std::string filename =
  //     "/dune/app/users/milov/dune-raw-data/np04_raw_run001803_1_dl1.root";
  // std::vector<std::string> filenames;
  // filenames.push_back(filename);

  // // Tag to load FELIX data.
  // art::InputTag tag("daq:FELIX:DAQ");

  // // Loop over events.
  // for (gallery::Event evt(filenames); !evt.atEnd(); evt.next()) {
  //   std::cout << "Event number " << evt.eventEntry() << " loaded.\n";
  //   gallery::ValidHandle<std::vector<artdaq::Fragment>> const& frags =
  //       evt.getValidHandle<std::vector<artdaq::Fragment>>(tag);
  //   std::cout << "Found valid artdaq::Fragments of size " << (*frags).size()
  //             << " \n";

  //   // Loop over fragments within an event (likely just one).
  //   for(const auto& frag : *frags) {
  //     dune::FelixFragment flxfrag(frag);
  //     // Check fragment timestamp.
  //     std::cout << frag.timestamp() << '\t' << flxfrag.timestamp() << '\t' << frag.timestamp() - flxfrag.timestamp() << '\n';
  //     // Loop over frames in the fragment and check the timestamps and COLDATA
  //     // convert counts.
  //     uint64_t prevtimestamp = flxfrag.timestamp(0);
  //     uint16_t prevCCC1 = flxfrag.coldata_convert_count(0,0);
  //     uint16_t prevCCC2 = flxfrag.coldata_convert_count(0,2);
  //     for(unsigned fr = 1; fr < 10 /*flxfrag.total_frames()*/; ++fr) {
  //       // // Print the first few frames.
  //       // flxfrag.print(fr);

  //       std::cout << "tick, crate, slot, fiber: " << fr << ", " << (unsigned)flxfrag.crate_no(fr) << ", " << (unsigned)flxfrag.slot_no(fr) << ", " << (unsigned)flxfrag.fiber_no(fr) << "\n";

  //       // Check timestamp.
  //       if(flxfrag.timestamp(fr) - prevtimestamp != 25) {
  //         std::cout << "Timestamp does not increase by 25 at frame " << fr << "!\n";
  //         std::cout << flxfrag.timestamp(fr) - prevtimestamp << "\n";
  //         return;
  //       }
  //       prevtimestamp = flxfrag.timestamp(fr);

  //       // Check CCC.
  //       if (flxfrag.coldata_convert_count(fr, 0) - prevCCC1 != 1 &&
  //           flxfrag.coldata_convert_count(fr, 0) - prevCCC1 != -65535) {
  //         std::cout << "CCC1 does not increase by 1 at frame " << fr << "!\n";
  //         std::cout << flxfrag.coldata_convert_count(fr,0) - prevCCC1 << '\n';
  //         return;
  //       }
  //       if (flxfrag.coldata_convert_count(fr, 2) - prevCCC2 != 1 &&
  //           flxfrag.coldata_convert_count(fr, 2) - prevCCC2 != -65535) {
  //         std::cout << "CCC2 does not increase by 1 at frame " << fr << "!\n";
  //         std::cout << flxfrag.coldata_convert_count(fr,2) - prevCCC2 << '\n';
  //         return;
  //       }
  //       prevCCC1 = flxfrag.coldata_convert_count(fr,0);
  //       prevCCC2 = flxfrag.coldata_convert_count(fr,2);
  //       if(flxfrag.coldata_convert_count(fr, 0) != flxfrag.coldata_convert_count(fr, 1) || flxfrag.coldata_convert_count(fr, 2) != flxfrag.coldata_convert_count(fr, 3)) {
  //         std::cout << "CCC in frame " << fr << " do not correspond!\n";
  //         return;
  //       }
  //       // break;
  //     } // Loop over frames.
  //     std::cout << "All checks good.\n";

  //     // break;
  //   } // Loop over fragments.
  //   // break;
  // }
}

BOOST_AUTO_TEST_SUITE_END()

#pragma GCC diagnostic pop
