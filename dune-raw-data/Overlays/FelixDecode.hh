// FelixDecode.hh serves to decode root files containing artdaq Fragments with
// FELIX data.

#ifndef artdaq_dune_Overlays_FelixDecode_hh
#define artdaq_dune_Overlays_FelixDecode_hh

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

namespace dune {

class FelixDecoder {
 private:
  art::InputTag tag = "daq:FELIX:DAQ";
  std::vector<std::string> filenames;

 public:
  // Filename loading options.
  void load(const std::string& filename) { filenames.push_back(filename); }
  void load(const std::vector<std::string>& new_filenames) {
    for (const auto& filename : new_filenames) {
      filenames.push_back(filename);
    }
  }

  // General information accessors.
  size_t total_fragments(const size_t& evt_num) const {
    gallery::Event evt(filenames);
    evt.goToEntry(evt_num);
    gallery::ValidHandle<std::vector<artdaq::Fragment>> const& frags =
        evt.getValidHandle<std::vector<artdaq::Fragment>>(tag);
    return frags->size();
  }

  size_t total_fragments() const {
    size_t result = 0;
    for (gallery::Event evt(filenames); !evt.atEnd(); evt.next()) {
      gallery::ValidHandle<std::vector<artdaq::Fragment>> const& frags =
          evt.getValidHandle<std::vector<artdaq::Fragment>>(tag);
      result += frags->size();
    }

    return result;
  }

  size_t total_events() const {
    gallery::Event evt(filenames);
    return evt.numberOfEventsInFile();
  }

  // Check functions.
  void check_timestamps() const {
    for (gallery::Event evt(filenames); !evt.atEnd(); evt.next()) {
      std::cout << "Event number " << evt.eventEntry() << " loaded.\n";
      gallery::ValidHandle<std::vector<artdaq::Fragment>> const& frags =
          evt.getValidHandle<std::vector<artdaq::Fragment>>(tag);
      std::cout << "Found valid artdaq::Fragments of size " << (*frags).size()
                << " \n";

      // Loop over fragments within an event (likely just one).
      for (const auto& frag : *frags) {
        dune::FelixFragment flxfrag(frag);
        // Loop over frames in the fragment and check the timestamps.
        uint64_t prevtimestamp = flxfrag.timestamp(0);
        for (unsigned fr = 1; fr < flxfrag.total_frames(); ++fr) {

          // Check timestamp.
          if (flxfrag.timestamp(fr) - prevtimestamp != 25) {
            std::cout << "Timestamp does not increase by 25 at frame " << fr
                      << "!\n";
            std::cout << flxfrag.timestamp(fr) - prevtimestamp << "\n";
            return;
          }
          prevtimestamp = flxfrag.timestamp(fr);
        } // Loop over frames.
      } // Loop over fragments.
    } // Loop over events.

    std::cout << "Timestamp check passed successfully.\n";
  }
  
  void check_CCC() const {
    for (gallery::Event evt(filenames); !evt.atEnd(); evt.next()) {
      std::cout << "Event number " << evt.eventEntry() << " loaded.\n";
      gallery::ValidHandle<std::vector<artdaq::Fragment>> const& frags =
          evt.getValidHandle<std::vector<artdaq::Fragment>>(tag);
      std::cout << "Found valid artdaq::Fragments of size " << (*frags).size()
                << " \n";

      // Loop over fragments within an event (likely just one).
      for (const auto& frag : *frags) {
        dune::FelixFragment flxfrag(frag);
        // Loop over frames in the fragment and check the COLDATA convert counts.
        uint16_t prevCCC1 = flxfrag.coldata_convert_count(0, 0);
        uint16_t prevCCC2 = flxfrag.coldata_convert_count(0, 2);
        for (unsigned fr = 1; fr < flxfrag.total_frames(); ++fr) {
          // Check CCC.
          if (flxfrag.coldata_convert_count(fr, 0) - prevCCC1 != 1 &&
              flxfrag.coldata_convert_count(fr, 0) - prevCCC1 != -65535) {
            std::cout << "CCC1 does not increase by 1 at frame " << fr << "!\n";
            std::cout << flxfrag.coldata_convert_count(fr, 0) - prevCCC1
                      << '\n';
            return;
          }
          if (flxfrag.coldata_convert_count(fr, 2) - prevCCC2 != 1 &&
              flxfrag.coldata_convert_count(fr, 2) - prevCCC2 != -65535) {
            std::cout << "CCC2 does not increase by 1 at frame " << fr << "!\n";
            std::cout << flxfrag.coldata_convert_count(fr, 2) - prevCCC2
                      << '\n';
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
        } // Loop over frames.
      } // Loop over fragments.
    } // Loop over events.

    std::cout << "Timestamp check passed successfully.\n";
  }

  // Accessor for FelixFragments.
  dune::FelixFragment FelixFragment(const size_t& evt_num, const size_t& frag_num = 0) const {
    gallery::Event evt(filenames);
    evt.goToEntry(evt_num);
    gallery::ValidHandle<std::vector<artdaq::Fragment>> const& frags =
        evt.getValidHandle<std::vector<artdaq::Fragment>>(tag);
    dune::FelixFragment result(frags->at(frag_num));
    return result;
  }

  // Constructors that load a file.
  FelixDecoder(const std::string& filename) { filenames.push_back(filename); }
  FelixDecoder(const std::vector<std::string> &new_filenames) {
    for (const auto& filename : new_filenames) {
      filenames.push_back(filename);
    }
  }
};

}  // namespace dune

#endif /* artdaq_dune_Overlays_FelixDecode_hh */