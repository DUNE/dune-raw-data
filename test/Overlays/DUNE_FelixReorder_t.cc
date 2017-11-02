#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "dune-raw-data/Overlays/FelixFragment.hh"
#include "dune-raw-data/Overlays/FelixFragmentReordered.hh"
#include "dune-raw-data/Overlays/FelixReorder.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#define BOOST_TEST_MODULE(MilliSlice_t)
#ifdef HAVE_CANVAS
#include "cetlib/quiet_unit_test.hpp"
#else
#include "boost/test/auto_unit_test.hpp"
#endif

BOOST_AUTO_TEST_SUITE(FelixReorder_test)

BOOST_AUTO_TEST_CASE(BaselineTest) {
  std::cout << "Reordering frames in files." << std::endl;
  for (unsigned plane = 0; plane < 3; ++plane) {
    for (unsigned link = 1; link < 11; ++link) {
      // Open the relevant files.
      std::string filepath = "/home/felixdev/milo/uBframes/Plane_" +
                             std::to_string(plane) + "-Link_" +
                             std::to_string(link) + ".dat";
      std::cout << "\n\n## Opening and loading file " << filepath << "\n\n";
      std::string out_path =
          "/home/felixdev/milo/uBframes/reordered/Reordered_Plane_" +
          std::to_string(plane) + "-Link_" + std::to_string(link) + ".dat";

      std::ifstream ifile(filepath, std::ios::ate);
      if (!ifile.is_open()) {
        std::cout << "File " << filepath << " could not be accessed"
                  << std::endl;
        return;
      }
      // Read the file contents into a fragment.
      const size_t ifile_size = ifile.tellg();
      artdaq::Fragment frag;
      frag.resizeBytes(ifile_size);
      ifile.seekg(0, std::ios::beg);
      ifile.read(reinterpret_cast<char*>(frag.dataBeginBytes()), ifile_size);
      ifile.close();
      std::cout << "Input buffer size: " << ifile_size << ".\n";

      // Fill a fragment with the reordered output and save it to file.
      artdaq::Fragment reord_frag(
          dune::FelixReorder(frag.dataBeginBytes(), 9600));
      std::cout << "Reordered fragment data size: "
                << reord_frag.dataSizeBytes() << '\n';

      std::ofstream ofile(out_path);
      ofile.write(reinterpret_cast<char*>(reord_frag.dataBeginBytes()),
                  reord_frag.dataSizeBytes());
      ofile.close();

      // Create the two FelixFragments.
      dune::FelixFragment flxfrg(frag);
      dune::FelixFragmentReordered reordflxfrg(reord_frag);

      // Compare the two fragments.
      std::cout << "\nComparing the untouched and reordered fragments.\n";
      auto comp_start = std::chrono::high_resolution_clock::now();
      for (unsigned i = 0; i < flxfrg.total_frames(); ++i) {
        BOOST_REQUIRE_EQUAL(flxfrg.sof(i), reordflxfrg.sof(i));
        BOOST_REQUIRE_EQUAL(flxfrg.version(i), reordflxfrg.version(i));
        BOOST_REQUIRE_EQUAL(flxfrg.fiber_no(i), reordflxfrg.fiber_no(i));
        BOOST_REQUIRE_EQUAL(flxfrg.slot_no(i), reordflxfrg.slot_no(i));
        BOOST_REQUIRE_EQUAL(flxfrg.crate_no(i), reordflxfrg.crate_no(i));
        BOOST_REQUIRE_EQUAL(flxfrg.timestamp(i), reordflxfrg.timestamp(i));
        BOOST_REQUIRE_EQUAL(flxfrg.CRC32(i), reordflxfrg.CRC32(i));
        for (unsigned j = 0; j < dune::FelixReorderer::num_blocks_per_frame;
             ++j) {
          BOOST_REQUIRE_EQUAL(flxfrg.s1_error(i, j),
                              reordflxfrg.s1_error(i, j));
          BOOST_REQUIRE_EQUAL(flxfrg.s2_error(i, j),
                              reordflxfrg.s2_error(i, j));
          BOOST_REQUIRE_EQUAL(flxfrg.checksum_a(i, j),
                              reordflxfrg.checksum_a(i, j));
          BOOST_REQUIRE_EQUAL(flxfrg.checksum_b(i, j),
                              reordflxfrg.checksum_b(i, j));
          BOOST_REQUIRE_EQUAL(flxfrg.coldata_convert_count(i, j),
                              reordflxfrg.coldata_convert_count(i, j));
          BOOST_REQUIRE_EQUAL(flxfrg.error_register(i, j),
                              reordflxfrg.error_register(i, j));
          for (unsigned h = 0; h < 8; ++h) {
            BOOST_REQUIRE_EQUAL(flxfrg.HDR(i, j, h), reordflxfrg.HDR(i, j, h));
          }
        }
        for (unsigned ch = 0; ch < dune::FelixReorderer::num_adcs_per_frame;
             ++ch) {
          BOOST_REQUIRE_EQUAL(flxfrg.get_ADC(i, ch),
                              reordflxfrg.get_ADC(i, ch));
        }
      }
      auto comp_end = std::chrono::high_resolution_clock::now();
      std::cout << "Comparison took "
                << std::chrono::duration_cast<std::chrono::milliseconds>(
                       comp_end - comp_start)
                       .count()
                << " msec.\n";
      std::cout << "No errors detected.\n";
    }
  }

  std::cout << "\n\nProgram end." << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()

#pragma GCC diagnostic pop
