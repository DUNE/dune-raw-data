// FelixDecode.hh serves to decode root files containing artdaq Fragments with
// FELIX data.

#ifndef artdaq_dune_Overlays_FelixDecode_hh
#define artdaq_dune_Overlays_FelixDecode_hh

#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>
#include "art/Framework/Principal/Handle.h"
#include "artdaq-core/Data/Fragment.hh"
#include "canvas/Utilities/InputTag.h"
#include "dune-raw-data/Overlays/FelixFragment.hh"
#include "gallery/Event.h"

namespace dune {

class FelixDecoder {
 private:
  art::InputTag tag = "daq:FELIX:DAQ";
  std::vector<std::string> filenames;
  std::vector<artdaq::Fragment> frags_;

 public:
  // General information accessors.
  size_t total_fragments() const {
    return frags_.size();
  }

  size_t total_events() const {
    gallery::Event evt(filenames);
    return evt.numberOfEventsInFile();
  }

  // Check functions.
  void check_timestamps() const {
      // Loop over fragments within an event (likely just one).
      for (const auto& frag : frags_) {
        dune::FelixFragment flxfrag(frag);
        // Loop over frames in the fragment and check the timestamps.
        uint64_t prevtimestamp = flxfrag.timestamp(0);
        for (unsigned fr = 1; fr < flxfrag.total_frames(); ++fr) {
          // Check timestamp.
          if (flxfrag.timestamp(fr) - prevtimestamp != 25) {
            std::cout << "Timestamp does not increase by 25 at frame " << fr
                      << "!\n";
            std::cout << "It instead increases by "
                      << flxfrag.timestamp(fr) - prevtimestamp << "\n";
            return;
          }
          prevtimestamp = flxfrag.timestamp(fr);
        }  // Loop over frames.
      }    // Loop over fragments.

    std::cout << "Timestamp check passed successfully.\n";
  }

  void check_CCC() const {
    for (const auto& frag : frags_) {
      dune::FelixFragment flxfrag(frag);
      // Loop over frames in the fragment and check the COLDATA convert
      // counts.
      uint16_t prevCCC1 = flxfrag.coldata_convert_count(0, 0);
      uint16_t prevCCC2 = flxfrag.coldata_convert_count(0, 2);
      for (unsigned fr = 1; fr < flxfrag.total_frames(); ++fr) {
        // Check CCC.
        if (flxfrag.coldata_convert_count(fr, 0) - prevCCC1 != 1 &&
            flxfrag.coldata_convert_count(fr, 0) - prevCCC1 != -65535) {
          std::cout << "CCC1 does not increase by 1 at frame " << fr << "!\n";
          std::cout << "It instead increases by "
                    << flxfrag.coldata_convert_count(fr, 0) - prevCCC1 << '\n';
          return;
        }
        if (flxfrag.coldata_convert_count(fr, 2) - prevCCC2 != 1 &&
            flxfrag.coldata_convert_count(fr, 2) - prevCCC2 != -65535) {
          std::cout << "CCC2 does not increase by 1 at frame " << fr << "!\n";
          std::cout << "It instead increases by "
                    << flxfrag.coldata_convert_count(fr, 2) - prevCCC2 << '\n';
          return;
        }
        prevCCC1 = flxfrag.coldata_convert_count(fr, 0);
        prevCCC2 = flxfrag.coldata_convert_count(fr, 2);
        if (flxfrag.coldata_convert_count(fr, 0) !=
                flxfrag.coldata_convert_count(fr, 1) ||
            flxfrag.coldata_convert_count(fr, 2) !=
                flxfrag.coldata_convert_count(fr, 3)) {
          std::cout << "CCC in frame " << fr << " do not correspond!\n";
          return;
        }
      }  // Loop over frames.
    }    // Loop over fragments.

    std::cout << "CCC check passed successfully.\n";
  }

  // Accessor for artdaq::Fragments.
  artdaq::Fragment Fragment(const size_t& frag_num) const {
    if(frag_num > total_fragments()) {
      std::cout << "Fragment index out of range.\n";
      return frags_[0];
    }
    return frags_[frag_num];
  }

  // // Accessor for FelixFragments.
  // dune::FelixFragment FelixFragment(const size_t& frag_num) const {
  //   dune::FelixFragment result(Fragment(frag_num));
  //   return result;
  // }

  // Print function.
  void printFrames(const size_t& begin_frame, const size_t& end_frame, const size_t& frag_num = 0) const {
    artdaq::Fragment frag(Fragment(frag_num));
    dune::FelixFragment flxfrag(frag);
    for(unsigned i = begin_frame; i < end_frame; ++i) {
      std::cout << "Frame " << i << '\n';
      flxfrag.print(i);
    }
  }

  // Filename loading options.
  void load(const std::string& filename) { filenames.push_back(filename); }
  void load(const std::vector<std::string>& new_filenames) {
    for (const auto& filename : new_filenames) {
      filenames.push_back(filename);
    }
  }

  void init() {
    for (gallery::Event evt(filenames); !evt.atEnd(); evt.next()) {
      gallery::ValidHandle<std::vector<artdaq::Fragment>> const& frags =
          evt.getValidHandle<std::vector<artdaq::Fragment>>(tag);
      for (const auto& frag : *frags) {
        frags_.push_back(frag);
      }
    }
    std::cout << "Loaded " << frags_.size() << " fragments.\n";
  }

  // Constructors that load a file.
  FelixDecoder() = delete;
  FelixDecoder(const std::string& filename) {
    load(filename);
    init();
  }
  FelixDecoder(const std::vector<std::string>& filenames) {
    load(filenames);
    init();
  }
};

}  // namespace dune

#endif /* artdaq_dune_Overlays_FelixDecode_hh */