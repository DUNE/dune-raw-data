// FelixFragmentReordered.hh was made to be an overlay to reordered FELIX
// frames. It inherits a lot from the original FelixFragment code.

#ifndef artdaq_dune_Overlays_FelixFragmentReordered_hh
#define artdaq_dune_Overlays_FelixFragmentReordered_hh

#include "FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "dune-raw-data/Overlays/FelixFragment.hh"
#include "dune-raw-data/Overlays/FelixReorder.hh"

#include <chrono>

namespace dune {
class FelixFragmentReordered;
}

class dune::FelixFragmentReordered {
 public:
  FelixFragmentReordered(artdaq::Fragment const& fragment)
      : artdaq_Fragment_(fragment) {
    std::cout << "Attempting to create a reordered FELIX Fragment.\n";
    head = reinterpret_cast<uint8_t const*>(artdaq_Fragment_.dataBeginBytes());
  }

  /* Define a public typedef for marking the WIB word size. */
  typedef FelixFragment::word_t word_t;
  typedef FelixFragment::adc_t adc_t; /* 12 bits in the current format. */
  typedef std::vector<adc_t> adc_v;

  size_t total_words() const {
    return artdaq_Fragment_.dataSizeBytes() / sizeof(word_t);
  }

  size_t total_frames() const {
    return artdaq_Fragment_.dataSizeBytes() /
           (frame_size - num_adcs_per_frame * 12 / 8 +
            num_adcs_per_frame * sizeof(adc_t));
  }

  size_t total_adc_values() const {
    return total_frames() * num_adcs_per_frame;
  }

  /* Frame header accessors. */
  uint8_t sof(const unsigned& frame_ID) const { return head_(frame_ID)->sof; }
  uint8_t version(const unsigned& frame_ID) const {
    return head_(frame_ID)->version;
  }
  uint8_t fiber_no(const unsigned& frame_ID) const {
    return head_(frame_ID)->fiber_no;
  }
  uint8_t slot_no(const unsigned& frame_ID) const {
    return head_(frame_ID)->slot_no;
  }
  uint8_t crate_no(const unsigned& frame_ID) const {
    return head_(frame_ID)->crate_no;
  }
  uint8_t mm(const unsigned& frame_ID) const { return head_(frame_ID)->mm; }
  uint8_t oos(const unsigned& frame_ID) const { return head_(frame_ID)->oos; }
  uint16_t wib_errors(const unsigned& frame_ID) const {
    return head_(frame_ID)->wib_errors;
  }
  uint64_t timestamp(const unsigned& frame_ID) const {
    return head_(frame_ID)->timestamp();
  }
  uint16_t wib_counter(const unsigned& frame_ID) const {
    return head_(frame_ID)->wib_counter;
  }
  word_t CRC32(const unsigned& frame_ID) const { return *crc32_(frame_ID); }

  /* Coldata block accessors. */
  uint8_t s1_error(const unsigned& frame_ID, const uint8_t& block_num) const {
    return blockhead_(frame_ID, block_num)->s1_error;
  }
  uint8_t s2_error(const unsigned& frame_ID, const uint8_t& block_num) const {
    return blockhead_(frame_ID, block_num)->s2_error;
  }
  uint16_t checksum_a(const unsigned& frame_ID,
                      const uint8_t& block_num) const {
    return blockhead_(frame_ID, block_num)->checksum_a();
  }
  uint16_t checksum_b(const unsigned& frame_ID,
                      const uint8_t& block_num) const {
    return blockhead_(frame_ID, block_num)->checksum_b();
  }
  uint16_t coldata_convert_count(const unsigned& frame_ID,
                                 const uint8_t& block_num) const {
    return blockhead_(frame_ID, block_num)->coldata_convert_count;
  }
  uint16_t error_register(const unsigned& frame_ID,
                          const uint8_t& block_num) const {
    return blockhead_(frame_ID, block_num)->error_register;
  }
  uint8_t HDR(const unsigned& frame_ID, const uint8_t& block_num,
              const uint8_t& HDR_num) const {
    return blockhead_(frame_ID, block_num)->HDR(HDR_num);
  }

  adc_t get_ADC(const unsigned& frame_ID, const unsigned& channel_ID) const {
    return *reinterpret_cast<adc_t const*>(
        artdaq_Fragment_.dataBeginBytes() + initial_adc_begin +
        (channel_ID * total_frames() + frame_ID) * adc_size);
  }

  adc_t get_ADC(const unsigned& channel_ID) const {
    const unsigned frame_ID = channel_ID / num_adcs_per_frame;
    return get_ADC(frame_ID, channel_ID % num_adcs_per_frame);
  }

  adc_t get_ADC(const unsigned& frame_ID, const uint8_t& block_ID,
                const uint8_t& stream_ID, const uint8_t& channel_ID) const {
    return get_ADC(frame_ID,
                   block_ID * num_adcs_per_block +
                       stream_ID * num_adcs_per_stream + channel_ID);
  }

  adc_v get_ADCs_by_channel(uint8_t channel_ID) const {
    adc_v output;
    const adc_t* offset = reinterpret_cast<adc_t const*>(
        artdaq_Fragment_.dataBeginBytes() + initial_adc_begin +
        channel_ID * total_frames() * adc_size);
    output.insert(output.begin(), offset, offset + total_frames());
    return output;
  }

  adc_v get_ADCs_by_channel(uint8_t block_ID, uint8_t stream_ID,
                            uint8_t channel_ID) const {
    uint8_t ch_ID = block_ID * num_adcs_per_block +
                    stream_ID * num_adcs_per_stream + channel_ID;
    return get_ADCs_by_channel(ch_ID);
  }

  const uint8_t* head;

 private:
  artdaq::Fragment const& artdaq_Fragment_;

  const size_t num_frames = artdaq_Fragment_.dataSizeBytes() / 596;

  // Frame variables derived from FelixReorder.hh.
  static const unsigned netio_header_size = FelixReorderer::netio_header_size;
  static const unsigned frame_size = FelixReorderer::frame_size;
  static const unsigned wib_header_size = FelixReorderer::wib_header_size;
  static const unsigned coldata_header_size =
      FelixReorderer::coldata_header_size;
  static const unsigned coldata_block_size = FelixReorderer::coldata_block_size;
  static const unsigned crc32_size = FelixReorderer::crc32_size;

  static const unsigned adc_size = FelixReorderer::adc_size;

  static const unsigned num_adcs_per_frame = FelixReorderer::num_adcs_per_frame;
  static const unsigned num_blocks_per_frame =
      FelixReorderer::num_blocks_per_frame;
  static const unsigned num_streams_per_block =
      FelixReorderer::num_streams_per_block;

  static const unsigned num_adcs_per_block = FelixReorderer::num_adcs_per_block;
  static const unsigned num_adcs_per_stream =
      FelixReorderer::num_adcs_per_stream;

  // Locations in the reordered fragment.
  const unsigned wib_headers_begin = netio_header_size;
  const unsigned crc32_begin = wib_headers_begin + num_frames * wib_header_size;
  const unsigned coldata_headers_begin = crc32_begin + num_frames * crc32_size;
  const unsigned initial_adc_begin =
      coldata_headers_begin +
      num_frames * coldata_header_size * num_blocks_per_frame;
  const unsigned adc_begin = initial_adc_begin + num_adcs_per_frame * adc_size;
  const unsigned end =
      adc_begin + (num_frames - 1) * num_adcs_per_frame * adc_size;

 protected:
  FelixFragment::WIBHeader const* head_(const unsigned& frame_num = 0) const {
    return reinterpret_cast<FelixFragment::WIBHeader const*>(
        artdaq_Fragment_.dataBeginBytes() + wib_headers_begin +
        frame_num * wib_header_size);
  }

  word_t const* crc32_(const unsigned& frame_num = 0) const {
    return reinterpret_cast<word_t const*>(artdaq_Fragment_.dataBeginBytes() +
                                           crc32_begin +
                                           frame_num * crc32_size);
  }

  FelixFragment::ColdataHeader const* blockhead_(
      const unsigned& frame_num = 0, const unsigned& block_num = 0) const {
    return reinterpret_cast<FelixFragment::ColdataHeader const*>(
        artdaq_Fragment_.dataBeginBytes() + coldata_headers_begin +
        frame_num * coldata_header_size * num_blocks_per_frame +
        block_num * coldata_header_size);
  }
};

#endif /* artdaq_dune_Overlays_FelixFragmentReordered_hh */