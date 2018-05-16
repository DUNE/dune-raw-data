#include <stdint.h>
#include <bitset>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "artdaq-core/Data/Fragment.hh"
#include "dune-raw-data/Overlays/FelixCompress.hh"
#include "dune-raw-data/Overlays/FelixFragment.hh"
#include "dune-raw-data/Overlays/FelixReorder.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#define BOOST_TEST_MODULE(MilliSlice_t)
#ifdef HAVE_CANVAS
#include "cetlib/quiet_unit_test.hpp"
#else
#include "boost/test/auto_unit_test.hpp"
#endif

BOOST_AUTO_TEST_SUITE(FelixFragment_test)

BOOST_AUTO_TEST_CASE(BaselineTest) {
  // Get all files.
  std::vector<int> event_nums = {3059515, 3059537, 3059542, 3059574, 3059575,
                                 3059577, 3059599, 3059603, 3059620, 3059622};
  std::vector<std::string> filenames;
  for (auto event : event_nums) {
    for (unsigned p = 0; p < 3; ++p) {
      for (unsigned f = 1; f < 10; ++f) {
        filenames.push_back(
            "/nashome/m/milov/Documents/kevlar/run/"
            "Run_1-SubRun_6120-Event_" +
            std::to_string(event) + "-Plane_" + std::to_string(p) + "-Frame_" +
            std::to_string(f) + ".dat");
      }
      if (p == 2) {
        for (unsigned f = 10; f < 14; ++f) {
          filenames.push_back(
              "/nashome/m/milov/Documents/kevlar/run/"
              "Run_1-SubRun_6120-Event_" +
              std::to_string(event) + "-Plane_" + std::to_string(p) +
              "-Frame_" + std::to_string(f) + ".dat");
        }
      }
    }
  }

  // Write a file with results.
  std::ofstream ofile("compression_results.dat");
  ofile << "#Compression factor\tCompression time\tNoise RMS\n";

  // Averaging values of different files.
  std::vector<double> comp_factors;
  std::vector<size_t> comp_times;

  for (auto filename : filenames) {
    // std::string filename = filenames[0];
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
      std::cout << "Could not open file.\n";
      return;
    }
    std::cout << "Reading from " << filename << ".\n";
    std::string contents((std::istreambuf_iterator<char>(in)),
                         (std::istreambuf_iterator<char>()));

    dune::FelixFragmentBase::Metadata meta;
    std::unique_ptr<artdaq::Fragment> frag_ptr(artdaq::Fragment::FragmentBytes(
        contents.size(), 1, 1, dune::toFragmentType("FELIX"), meta));
    frag_ptr->resizeBytes(contents.size());
    memcpy(frag_ptr->dataBeginBytes(), contents.c_str(), contents.size());
    in.close();

    dune::FelixFragment flxfrg(*frag_ptr);

    std::cout << "### WOOF -> Test for the presence of 9600 frames...\n";
    const size_t frames = 9600;

    std::cout << "  -> Total words: " << flxfrg.total_words() << '\n';
    std::cout << "  -> Total frames: " << flxfrg.total_frames() << '\n';
    std::cout << "  -> Total adc values: " << flxfrg.total_adc_values() << '\n';

    BOOST_REQUIRE_EQUAL(flxfrg.total_words(), frames * 120);
    BOOST_REQUIRE_EQUAL(flxfrg.total_frames(), frames);
    BOOST_REQUIRE_EQUAL(flxfrg.total_adc_values(), frames * 256);
    std::cout << "\n\n";

    std::cout << "### WOOF -> WIB frame test.\n";
    BOOST_REQUIRE_EQUAL(sizeof(dune::FelixFrame), 480);
    std::cout << " -> SOF: " << unsigned(flxfrg.sof(0)) << "\n";
    std::cout << " -> Version: " << unsigned(flxfrg.version(0)) << "\n";
    std::cout << " -> FiberNo: " << unsigned(flxfrg.fiber_no(0)) << "\n";
    std::cout << " -> SlotNo: " << unsigned(flxfrg.slot_no(0)) << "\n";
    std::cout << " -> CrateNo: " << unsigned(flxfrg.crate_no(0)) << "\n";
    std::cout << " -> Timestamp: " << std::hex << flxfrg.timestamp(0)
              << std::dec;
    std::cout << "\n\n";

    // Noise characterisation per channel, then average over fragment.
    double frag_rms = 0;
    std::vector<double> chan_avg(256,0);
    std::vector<double> chan_rms(256,0);
    // Determine the average per channel.
    for(unsigned fr = 0; fr < 256; ++fr) {
      for(unsigned ti = 0; ti < 9600; ++ti) {
        chan_avg[fr] += flxfrg.get_ADC(fr, ti);
      }
      chan_avg[fr] /= 9600;
    }
    // Determine the RMS per channel.
    for(unsigned vi = 0; vi < 256; ++vi) {
      for(unsigned ti = 0; ti < 9600; ++ti) {
        chan_rms[vi] += pow(flxfrg.get_ADC(vi, ti) - chan_avg[vi], 2);
      }
      chan_rms[vi] = sqrt(chan_rms[vi]/(chan_rms.size()-1));
      frag_rms += chan_rms[vi];
    }
    // Determine the average RMS per fragment.
    frag_rms /= 256;
    std::cout << "FRAGMENT RMS: " << frag_rms << '\n';

    // Compression tests.
    std::cout << "### MEOW -> WIB frame compression test.\n";

    auto comp_begin = std::chrono::high_resolution_clock::now();
    std::vector<char> compfrg(dune::FelixCompress(flxfrg));
    auto comp_end = std::chrono::high_resolution_clock::now();
    artdaq::Fragment decompfrg(dune::FelixDecompress(compfrg));
    auto decomp_end = std::chrono::high_resolution_clock::now();

    std::cout << "Compressed buffer size: " << compfrg.size() << ".\n"
              << "Compression factor: " << (double)flxfrg.dataSizeBytes() / compfrg.size() << '\n'
              << "Compression time taken: "
              << std::chrono::duration_cast<std::chrono::microseconds>(
                     comp_end - comp_begin)
                     .count()
              << " us.\n"
              << "Decompression time taken: "
              << std::chrono::duration_cast<std::chrono::microseconds>(
                     decomp_end - comp_end)
                     .count()
              << " us.\n\n"
              << "Noise RMS: " << frag_rms << '\n';

    comp_factors.push_back((double)flxfrg.dataSizeBytes() / compfrg.size());
    comp_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(
                     comp_end - comp_begin)
                     .count());

    ofile << (double)flxfrg.dataSizeBytes() / compfrg.size() << '\t' << std::chrono::duration_cast<std::chrono::microseconds>(
                     comp_end - comp_begin)
                     .count() << '\t' << frag_rms << '\n';

    // // Test whether the original and decompressed frames correspond.
    // const dune::FelixFrame* orig =
    //     reinterpret_cast<dune::FelixFrame const*>(flxfrg.dataBeginBytes());
    // const dune::FelixFrame* decomp =
    //     reinterpret_cast<dune::FelixFrame
    //     const*>(decompfrg.dataBeginBytes());
    // for (unsigned i = 0; i < frames; ++i) {
    //   BOOST_REQUIRE_EQUAL((orig + i)->version(), (decomp + i)->version());
    //   for (unsigned j = 0; j < 256; ++j) {
    //     BOOST_REQUIRE_EQUAL((orig + i)->channel(j), (decomp +
    //     i)->channel(j));
    //   }
    // }
  }  // Loop over files.

  ofile.close();

  // Calculate average compression factor and time, complete with error.
  double comp_factor = 0;
  for(auto c : comp_factors) { comp_factor += c;}
  comp_factor /= comp_factors.size();
  
  double comp_time = 0;
  for(auto c : comp_times) { comp_time += c;}
  comp_time /= comp_times.size();

  double comp_factor_err = 0;
  for(auto c : comp_factors) { comp_factor_err += pow(comp_factor - c, 2); }
  comp_factor_err = sqrt(comp_factor_err/(comp_factors.size()-1));

  double comp_time_err = 0;
  for(auto c : comp_times) { comp_time_err += pow(comp_time - c, 2); }
  comp_time_err = sqrt(comp_time_err/(comp_times.size()-1));

  std::cout << "Average compression factor: " << comp_factor << " +- " << comp_factor_err << "\nAverage compression time: " << comp_time << " +- " << comp_time_err << '\n';

  std::cout << "### WOOF WOOF -> Done...\n";
}

BOOST_AUTO_TEST_SUITE_END()

#pragma GCC diagnostic pop
