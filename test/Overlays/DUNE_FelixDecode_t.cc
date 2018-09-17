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
  // // Generate frames to put into a file.
  // std::ofstream framefile("frames.dat");
  // dune::FelixFrame frame[6000];
  // for(unsigned i = 0; i < 6000; ++i) {
  //   // WIB header fields.
  //   frame[i].set_version(1);
  //   frame[i].set_fiber_no(1);
  //   frame[i].set_slot_no(3);
  //   frame[i].set_crate_no(6);
  //   frame[i].set_mm(0);
  //   frame[i].set_oos(0);
  //   frame[i].set_wib_errors(0);
  //   frame[i].set_timestamp(i*25);
  //   frame[i].set_z(0);

  //   // COLDATA header fields.
  //   for(unsigned b = 0; b < 4; ++b) {
  //     frame[i].set_s1_error(b, 0);
  //     frame[i].set_s2_error(b, 0);
  //     frame[i].set_checksum_a(b, 0xabab);
  //     frame[i].set_checksum_b(b, 0xcdcd);
  //     frame[i].set_coldata_convert_count(b, i);
  //     frame[i].set_error_register(b, 0);
  //     for(unsigned h = 0; h < 8; ++h) {
  //       frame[i].set_hdr(b, h, h);
  //     }
  //   }

  //   // Channel values. Set to be a counter
  //   for(unsigned c = 0; c < 256; ++c) {
  //     frame[i].set_channel(c, c);
  //   }
  // }
  // framefile.write(reinterpret_cast<char const*>(&frame[0]), 6000*464);
  // framefile.close();

  // Uncompressed (no metadata)
  // std::string filename = "/pnfs/dune/tape_backed/dunepro/protodune/np04/beam/detector/None/raw/04/99/61/01/np04_raw_run004155_0096_dl2.root";
  // Compressed ({1, 0, 0} metadata)
  // std::string filename = "/pnfs/dune/tape_backed/dunepro/protodune/np04/beam/detector/None/raw/04/99/35/53/np04_raw_run004149_0024_dl4.root";
  // std::string outputDestination = "noise_records/test";
  std::string filename =
      "/pnfs/dune/tape_backed/dunepro/protodune/np04/beam/detector/None/raw/05/"
      "16/24/57/np04_raw_run004424_0001_dl5.root";

  // Initialise a decoder object from the file.
  dune::FelixDecoder flxdec(filename);

  for(unsigned i = 0; i < 10 /* flxdec.total_fragments() */; ++i) {
    // Load fragments into overlay.
    artdaq::Fragment frag = flxdec.Fragment(i);
    dune::FelixFragment flxfrag(frag);
    std::cout << "\nFragment " << i << "\tCrate " << (unsigned)flxfrag.crate_no() << '\t';
    std::cout << "Slot " << (unsigned)flxfrag.slot_no() << '\t';
    std::cout << "Fiber " << (unsigned)flxfrag.fiber_no() << '\n';

    dune::FelixFragment::Metadata meta =
        *frag.metadata<dune::FelixFragment::Metadata>();
    std::cout << "METADATA: " << (unsigned)meta.num_frames << "   " << (unsigned)meta.reordered << "   " << (unsigned)meta.compressed << '\n';
    // artdaq::Fragment uncompfrag(flxfrag.uncompressed_fragment());
    // dune::FelixFragment uncompflxfrag(uncompfrag);
    // std::cout << "Uncompressed fragment size: " << uncompfrag.dataSizeBytes() << '\n';
    
    // std::ofstream ofile("frag.dat");
    // ofile.write((char*)flxfrag.dataBeginBytes(), flxfrag.dataSizeBytes());
    // ofile.close();

    // std::ofstream offile("fragment.dat");
    // ofile.write((char*)uncompflxfrag.dataBeginBytes(), uncompflxfrag.dataSizeBytes());
    // ofile.close();

    // Print out relevant information.
    // dune::FelixFragment::Metadata meta = *(frag.metadata<dune::FelixFragment::Metadata>());
    // std::cout << "Metadata information:\n";
    // std::cout << "Number of frames: " << meta.num_frames << '\n';
    // std::cout << "Reordered: " << (unsigned)meta.reordered << '\n';
    // std::cout << "Compressed: " << (unsigned)meta.compressed << '\n';
    // std::cout << "Number of frames considered: " << flxfrag.total_frames() << '\n';
    std::cout << "Fragment data size: " << frag.dataSizeBytes() << '\n';

    // for(unsigned fr = 0; fr < flxfrag.total_frames(); ++fr) {
    //   for(unsigned b = 0; b < 4; ++b) {
    //     std::cout << b << '\t' << flxfrag.coldata_convert_count(fr, b) << '\t';
    //   }
    //   std::cout << '\n';
    // }
    // std::cout << "\n\n";

    // // Save fragment to file.
    // std::string frag_number = std::to_string(i);
    // frag_number = std::string(4 - frag_number.size(), '0') + frag_number;

    // std::string filename = "run/run003311_0001_" + frag_number + ".dat";
    // // std::cout << "Writing to " << filename << ".\n";
    // std::ofstream ofile(filename);
    // artdaq::Fragment frag = flxdec.Fragment(i);
    // ofile.write(reinterpret_cast<char const*>(frag.dataBeginBytes())+8*4, frag.dataSizeBytes()-8*4);
    // ofile.close();

    // dune::FelixFragment flxfrag(frag);
    // artdaq::Fragment reordfrag = dune::FelixReorder(frag.dataBeginBytes()+8*4,
    //                            (frag.dataSizeBytes()-8*4) / 464);

    // std::string reordfilename = filename.insert(filename.size()-4, "-reordered");
    // // std::cout << "Writing to " << reordfilename << ".\n";
    // std::ofstream reordfile(reordfilename);
    // reordfile.write(reinterpret_cast<char const*>(reordfrag.dataBeginBytes()),
    //                 reordfrag.dataSizeBytes());
    // reordfile.close();

    // std::cout << i << '\t' << (unsigned)flxfrag.crate_no() << '\n';
  }
  // std::cout << "Starting checks.\n";
  // flxdec.check_all_timestamps();
  // flxdec.check_all_CCCs();
  // flxdec.check_all_IDs();

  // artdaq::Fragment frag = flxdec.Fragment(0);
  // dune::FelixFragment flxfrag(frag);
  // std::cout << (unsigned)flxfrag.crate_no() << "   " << (unsigned)flxfrag.slot_no() << "   "
  //           << (unsigned)flxfrag.fiber_no() << '\n';

  // Print RMS values to file.
  // flxdec.calculateNoiseRMS("RMSnoise.dat");

  // // Print frames 4,5,6 of the 8th fragment.
  // flxdec.printFrames(3, 5, 7);
}

BOOST_AUTO_TEST_SUITE_END()

#pragma GCC diagnostic pop
