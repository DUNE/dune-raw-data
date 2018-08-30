#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "dune-raw-data/Overlays/FelixFragment.hh"
#include "dune-raw-data/Overlays/FelixReordererFacility.hh"

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
  // Get all files.
  std::vector<int> event_nums = {7258, 7263, 7264, 7269, 7276, 7283, 7284, 7287, 7294, 7296};
  // MC event numbers
  // std::vector<int> event_nums = {3059515, 3059537, 3059542, 3059574, 3059575,
  //                                3059577, 3059599, 3059603, 3059620, 3059622};
  std::vector<std::string> filenames;
  for (auto event : event_nums) {
    for (unsigned p = 0; p < 3; ++p) {
      for (unsigned f = 1; f < 10; ++f) {
        filenames.push_back(
            "/dune/app/users/milov/kevlar/newerrun/uBdat/Run_8700-SubRun_145-Event_" +
            std::to_string(event) + "-Plane_" + std::to_string(p) + "-Frame_0" +
            std::to_string(f) + ".dat");
      }
      if (p == 2) {
        for (unsigned f = 10; f < 14; ++f) {
          filenames.push_back(
              "/dune/app/users/milov/kevlar/newerrun/uBdat/"
              "Run_8700-SubRun_145-Event_" +
              std::to_string(event) + "-Plane_" + std::to_string(p) +
              "-Frame_" + std::to_string(f) + ".dat");
        }
      }
    }
  }
  // filenames.push_back("/nfs/home/np04daq/milo/frametests/channelid/felix-data-milo.dat");

  for (auto filename : filenames) {
    // Create a regular fragment from file.
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
      std::cout << "Could not open file " << filename << ".\n";
      continue;
    }
    std::string contents((std::istreambuf_iterator<char>(in)),
                         (std::istreambuf_iterator<char>()));
    in.close();

    dune::FelixFragmentBase::Metadata meta;
    meta.num_frames = contents.size()/sizeof(dune::FelixFrame);
    meta.reordered = 0;
    meta.compressed = 0;
    std::unique_ptr<artdaq::Fragment> frag_ptr(artdaq::Fragment::FragmentBytes(
        contents.size(), 1, 1, dune::toFragmentType("FELIX"), meta));
    frag_ptr->resizeBytes(contents.size());
    memcpy(frag_ptr->dataBeginBytes(), contents.c_str(), contents.size());

    dune::FelixFragment flxfrg(*frag_ptr);
    const size_t frames = flxfrg.total_frames();

    // // Create a compressed fragment from file.
    // std::ifstream incomp(filename+".gz", std::ios::binary);
    // if (!incomp.is_open()) {
    //   std::cout << "Could not open file " << filename+".gz" << ".\n";
    //   continue;
    // }
    // std::string compcontents((std::istreambuf_iterator<char>(incomp)),
    //                      (std::istreambuf_iterator<char>()));
    // incomp.close();

    // dune::FelixFragmentBase::Metadata compmeta;
    // compmeta.num_frames = 6024;//contents.size() / sizeof(dune::FelixFrame);
    // compmeta.reordered = 1;
    // compmeta.compressed = 0;
    // std::unique_ptr<artdaq::Fragment> compfrag_ptr(artdaq::Fragment::FragmentBytes(
    //     compcontents.size(), 1, 1, dune::toFragmentType("FELIX"), compmeta));
    // compfrag_ptr->resizeBytes(compcontents.size());
    // memcpy(compfrag_ptr->dataBeginBytes(), compcontents.c_str(), compcontents.size());

    // dune::FelixFragment compflxfrg(*compfrag_ptr);

    // Test whether FelixFragment and FelixFragmentReordered correspond.
    std::cout << "### MEOW -> Compressed FELIX Fragment test.\n";

    artdaq::Fragment reordfrg(
        dune::FelixReorder(frag_ptr->dataBeginBytes(), frames));
    // reordfrg.metadata<dune::FelixFragmentBase::Metadata>()->num_frames = 6000;
    // reordfrg.metadata<dune::FelixFragmentBase::Metadata>()->reordered = 1;
    dune::FelixFragment compflxfrg(reordfrg);

    std::cout << "  -> Total words: " << compflxfrg.total_words() << '\n';
    std::cout << "  -> Total frames: " << compflxfrg.total_frames() << '\n';
    std::cout << "  -> Total adc values: " << compflxfrg.total_adc_values()
              << "\n\n";

    std::cout << "### WOOF -> WIB frame test.\n";
    std::cout << " -> SOF: " << unsigned(flxfrg.sof(0)) << "\n";
    std::cout << " -> Version: " << unsigned(flxfrg.version(0)) << "\n";
    std::cout << " -> FiberNo: " << unsigned(flxfrg.fiber_no(0)) << "\n";
    std::cout << " -> SlotNo: " << unsigned(flxfrg.slot_no(0)) << "\n";
    std::cout << " -> CrateNo: " << unsigned(flxfrg.crate_no(0)) << "\n";
    std::cout << " -> Timestamp: " << std::hex << flxfrg.timestamp(0)
              << std::dec;
    std::cout << "\n\n";

    std::cout << "### WOOF -> Reordered WIB frame test.\n";
    std::cout << " -> SOF: " << unsigned(compflxfrg.sof(0)) << "\n";
    std::cout << " -> Version: " << unsigned(compflxfrg.version(0)) << "\n";
    std::cout << " -> FiberNo: " << unsigned(compflxfrg.fiber_no(0)) << "\n";
    std::cout << " -> SlotNo: " << unsigned(compflxfrg.slot_no(0)) << "\n";
    std::cout << " -> CrateNo: " << unsigned(compflxfrg.crate_no(0)) << "\n";
    std::cout << " -> Timestamp: " << std::hex << compflxfrg.timestamp(0)
              << std::dec;
    std::cout << "\n\n";

    std::cout << "### MEOW -> Comparing " << frames << " frames.\n";
    std::cout << "First compressed frame:\n";
    compflxfrg.print(0);
    auto compare_begin = std::chrono::high_resolution_clock::now();
    for (unsigned i = 0; i < frames; ++i) {
      // std::cout << i << '\t';
      BOOST_REQUIRE_EQUAL(flxfrg.sof(i), compflxfrg.sof(i));
      BOOST_REQUIRE_EQUAL(flxfrg.version(i), compflxfrg.version(i));
      BOOST_REQUIRE_EQUAL(flxfrg.fiber_no(i), compflxfrg.fiber_no(i));
      BOOST_REQUIRE_EQUAL(flxfrg.slot_no(i), compflxfrg.slot_no(i));
      BOOST_REQUIRE_EQUAL(flxfrg.crate_no(i), compflxfrg.crate_no(i));
      BOOST_REQUIRE_EQUAL(flxfrg.timestamp(i), compflxfrg.timestamp(i));
      for (unsigned j = 0; j < 4; ++j) {
        // std::cout << j << std::endl;
        BOOST_REQUIRE_EQUAL(flxfrg.s1_error(i, j), compflxfrg.s1_error(i, j));
        BOOST_REQUIRE_EQUAL(flxfrg.s2_error(i, j), compflxfrg.s2_error(i, j));
        BOOST_REQUIRE_EQUAL(flxfrg.checksum_a(i, j),
                            compflxfrg.checksum_a(i, j));
        BOOST_REQUIRE_EQUAL(flxfrg.checksum_b(i, j),
                            compflxfrg.checksum_b(i, j));
        BOOST_REQUIRE_EQUAL(flxfrg.coldata_convert_count(i, j),
                            compflxfrg.coldata_convert_count(i, j));
        BOOST_REQUIRE_EQUAL(flxfrg.error_register(i, j),
                            compflxfrg.error_register(i, j));
        for (unsigned h = 0; h < 8; ++h) {
          BOOST_REQUIRE_EQUAL(flxfrg.hdr(i, j, h), compflxfrg.hdr(i, j, h));
        }
      }
      for (unsigned ch = 0; ch < 256; ++ch) {
        BOOST_REQUIRE_EQUAL(flxfrg.get_ADC(i, ch), compflxfrg.get_ADC(i, ch));
      }
    }
    auto compare_end = std::chrono::high_resolution_clock::now();
    std::cout << "### MEOW -> Tests successful.\n";
    std::cout << "Took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     compare_end - compare_begin)
                     .count()
              << " ms.\n";

    // // Write reordered fragment to file for compression testing.
    // std::string outname = filename;
    // outname.insert(outname.size() - 4, "_reordered_prev-subtracted");
    // std::ofstream ofile(outname);
    // ofile.write(reinterpret_cast<char const*>(compflxfrg.dataBeginBytes()),
    //             compflxfrg.dataSizeBytes());
    // ofile.close();
  } // Loop over files.
}

BOOST_AUTO_TEST_SUITE_END()

#pragma GCC diagnostic pop
