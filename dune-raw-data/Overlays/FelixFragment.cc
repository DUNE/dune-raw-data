#include "dune-raw-data/Overlays/FelixFragment.hh"

#include "cetlib/exception.h"

#if 0
/*namespace {
  unsigned int pop_count (unsigned int n) {
    unsigned int c;
    for (c = 0; n; c++) n &= n - 1;
    return c;
  }
}*/
#endif

// File reading.
artdaq::FragmentPtr dune::FelixFragment::fromFile(const std::string& fileName) {
  std::ifstream in(fileName, std::ios::binary);
  std::string contents((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
  artdaq::Fragment::timestamp_t ts(0);
  dune::FelixFragment::Metadata meta;
  std::unique_ptr<artdaq::Fragment> frag_ptr(artdaq::Fragment::FragmentBytes(
      468, 1, 1, dune::toFragmentType("FELIX"), meta, ts));

  frag_ptr->resizeBytes(contents.size());
  memcpy(frag_ptr->dataBeginBytes(), contents.c_str(), contents.size());
  return frag_ptr;
}


// Functions to return an ADC value.
dune::FelixFragment::adc_t dune::FelixFragment::get_ADC(
    uint64_t frame_ID, uint8_t block_ID, uint8_t stream_ID,
    uint8_t channel_ID) const {
  return frame_(frame_ID)->channel(block_ID, stream_ID, channel_ID);
}
dune::FelixFragment::adc_t dune::FelixFragment::get_ADC(
    uint64_t frame_ID, uint8_t channel_ID) const {
  const int block_ID = channel_ID / num_ch_per_block;
  const int stream_ID = (channel_ID % num_ch_per_block) / num_stream_per_block;
  const int new_ch_ID = channel_ID % num_ch_per_stream;
  return dune::FelixFragment::get_ADC(frame_ID, block_ID, stream_ID, new_ch_ID);
}
dune::FelixFragment::adc_v dune::FelixFragment::get_ADCs_by_channel(
    uint8_t channel_ID) const {
  const int block_ID = channel_ID / num_ch_per_block;
  const int stream_ID = (channel_ID % num_ch_per_block) / num_stream_per_block;
  const int new_ch_ID = channel_ID % num_ch_per_stream;
  return dune::FelixFragment::get_ADCs_by_channel(block_ID, stream_ID,
                                                  new_ch_ID);
}

// Function to return all ADC values for a single channel [0:255].
dune::FelixFragment::adc_v dune::FelixFragment::get_ADCs_by_channel(
    uint8_t block_ID, uint8_t stream_ID, uint8_t channel_ID) const {
  adc_v output(total_frames());
  for (size_t i = 0; i < total_frames(); i++) {
    output[i] = get_ADC(i, block_ID, stream_ID, channel_ID);
  }
  return output;
}

// Function to return all ADC values for all channels in a map.
std::map<uint8_t, dune::FelixFragment::adc_v>
dune::FelixFragment::get_all_ADCs() const {
  std::map<uint8_t, adc_v> output;
  for (int i = 0; i < 256; i++)
    output.insert(std::pair<uint8_t, adc_v>(i, get_ADCs_by_channel(i)));
  return output;
}

