// This code was written for the 1.0 WIB->FELIX frame format.
// 10-10-2017 Milo Vermeulen

#ifndef artdaq_dune_Overlays_FelixReorder_hh
#define artdaq_dune_Overlays_FelixReorder_hh

// Uncomment this to utilise previous value subtraction instead of pedestal
// subtraction. This improves compression but makes the reading of ADC values
// after reordering very costly.
// #define PREVIOUS_SUBTRACTION

#include <chrono>
#include <fstream>
#include <thread>
#include <vector>

#include "artdaq-core/Data/Fragment.hh"

namespace dune {

class FelixReorderer {
 public:
  static const unsigned netio_header_size = 0;
  static const unsigned frame_size = 117 * 4;
  static const unsigned wib_header_size = 4 * 4;
  static const unsigned coldata_header_size = 4 * 4;
  static const unsigned coldata_block_size = 28 * 4;
  static const unsigned crc32_size = 4;

  static const unsigned adc_size = 2;

  static const unsigned num_adcs_per_frame = 256;
  static const unsigned num_blocks_per_frame = 4;
  static const unsigned num_streams_per_block = 8;

  static const unsigned num_adcs_per_block =
      num_adcs_per_frame / num_blocks_per_frame;
  static const unsigned num_adcs_per_stream =
      num_adcs_per_block / num_streams_per_block;

 private:
  const uint8_t* head;
  const size_t num_frames;
  const size_t size = num_frames * frame_size;

  uint16_t initial_adcs[num_adcs_per_frame];

  void wib_header_copy(uint8_t* dest);
  void crc32_copy(uint8_t* dest);
  void coldata_header_copy(uint8_t* dest);
  void initial_adc_copy(uint8_t* dest);
  void adc_copy(uint8_t* dest);

 public:
  FelixReorderer(const uint8_t* data, const size_t& num_frames = 10000)
      : head(data), num_frames(num_frames){};

  const unsigned newSize =
      netio_header_size +
      (wib_header_size + coldata_header_size * num_blocks_per_frame +
       crc32_size + adc_size * num_adcs_per_frame) *
          num_frames;

  // Locations in the destination buffer.
  const unsigned wib_headers_begin = netio_header_size;
  const unsigned crc32_begin = wib_headers_begin + num_frames * wib_header_size;
  const unsigned coldata_headers_begin = crc32_begin + num_frames * crc32_size;
  const unsigned initial_adc_begin =
      coldata_headers_begin +
      num_frames * coldata_header_size * num_blocks_per_frame;
  const unsigned adc_begin = initial_adc_begin + num_adcs_per_frame * adc_size;
  const unsigned end =
      adc_begin + (num_frames - 1) * num_adcs_per_frame * adc_size;

  void reorder_copy(uint8_t* dest);
  friend void t_adc_copy(FelixReorderer* reord, uint8_t* dest,
                         const unsigned& t_inst, const unsigned& t_tot);
  void single_frame_from_buffer(const uint8_t* src, uint8_t* dest,
                                const size_t& i);
  void frames_from_buffer(const uint8_t* src, uint8_t* dest,
                          const size_t& num_frames);
};

void FelixReorderer::wib_header_copy(uint8_t* dest) {
  // Store WIB-headers next to each other.
  const uint8_t* src = head + netio_header_size;
  for (unsigned i = 0; i < num_frames; ++i) {
    memcpy(dest, src, wib_header_size);
    dest += wib_header_size;
    src += frame_size;
  }
}

void FelixReorderer::crc32_copy(uint8_t* dest) {
  // Store CRC32s next to each other.
  const uint8_t* src =
      head + netio_header_size + wib_header_size + 4 * coldata_block_size;
  for (unsigned i = 0; i < num_frames; ++i) {
    memcpy(dest, src, crc32_size);
    dest += crc32_size;
    src += frame_size;
  }
}

void FelixReorderer::coldata_header_copy(uint8_t* dest) {
  // Store COLDATA headers next to each other.
  const uint8_t* src = head + netio_header_size + wib_header_size;
  for (unsigned i = 0; i < num_frames; ++i) {
    for (unsigned j = 0; j < num_blocks_per_frame; ++j) {
      memcpy(dest, src, coldata_header_size);
      dest += coldata_header_size;
      src += coldata_block_size;
    }
    src += crc32_size + wib_header_size;
  }
}

void FelixReorderer::initial_adc_copy(uint8_t* dest) {
  // Store all initial ADC values in uint16_t.
  const dune::FelixFragment::WIBFrame* src =
      reinterpret_cast<dune::FelixFragment::WIBFrame const*>(head +
                                                             netio_header_size);
  for (unsigned i = 0; i < num_adcs_per_frame; ++i) {
    initial_adcs[i] = src->channel(i);
    memcpy(dest, &initial_adcs[i], adc_size);
    dest += adc_size;
  }
}

// ADC copy function to be executed by individual threads.
void t_adc_copy(FelixReorderer* reord, uint8_t* dest, const unsigned& t_inst,
                const unsigned& t_tot) {
  // Thread starting point and range in ADC channel space.
  unsigned t_ch_range = reord->num_adcs_per_frame / t_tot;
  unsigned t_ch_begin = t_ch_range * t_inst;
  // The last thread should clean up the remainder of the channels.
  if (t_inst == t_tot - 1 && reord->num_adcs_per_frame % t_tot != 0) {
    t_ch_range = reord->num_adcs_per_frame - t_ch_begin;
  }

  // Copy initial ADCs for thread use.
  uint16_t t_initial_adcs[reord->num_adcs_per_frame];
  for (unsigned i = 0; i < reord->num_adcs_per_frame; ++i) {
    t_initial_adcs[i] = reord->initial_adcs[i];
  }
  // Store all ADC values in uint16_t.
  const dune::FelixFragment::WIBFrame* src =
      reinterpret_cast<dune::FelixFragment::WIBFrame const*>(
          reord->head + reord->netio_header_size);
  for (unsigned i = 1; i < reord->num_frames; ++i) {
    for (unsigned ch = t_ch_begin; ch < t_ch_begin + t_ch_range; ++ch) {
      // Enter the difference between the current value and the previous one.
      uint16_t reduced_adc = (src + i)->channel(ch) - t_initial_adcs[ch];
      const size_t dest_offset =
          ((i - 1) * reord->num_adcs_per_frame + ch) * reord->adc_size;
      memcpy(dest + dest_offset, &reduced_adc, reord->adc_size);
#ifdef PREVIOUS_SUBTRACTION
      // Update the initial ADC values to the new values.
      t_initial_adcs[ch] += reduced_adc;
#endif
    }
  }
}

void FelixReorderer::adc_copy(uint8_t* dest) {
  const unsigned t_tot = 8;
  std::thread* t_vec = new std::thread[t_tot - 1];

  for (unsigned i = 0; i < t_tot - 1; ++i) {
    t_vec[i] = std::thread(t_adc_copy, this, dest, i, t_tot);
  }
  t_adc_copy(this, dest, t_tot - 1, t_tot);
  for (unsigned i = 0; i < t_tot - 1; ++i) {
    t_vec[i].join();
  }
}

void FelixReorderer::reorder_copy(uint8_t* dest) {
  auto wib_start = std::chrono::high_resolution_clock::now();
  wib_header_copy(dest + wib_headers_begin);
  auto crc32_start = std::chrono::high_resolution_clock::now();
  crc32_copy(dest + crc32_begin);
  auto colhead_start = std::chrono::high_resolution_clock::now();
  coldata_header_copy(dest + coldata_headers_begin);
  auto iadc_start = std::chrono::high_resolution_clock::now();
  initial_adc_copy(dest + initial_adc_begin);
  auto adc_start = std::chrono::high_resolution_clock::now();
  adc_copy(dest + adc_begin);
  auto end = std::chrono::high_resolution_clock::now();

  std::cout
      << "\nMemcpy speed tests:\n"
      << "WIB header copy time: "
      << std::chrono::duration_cast<std::chrono::microseconds>(crc32_start -
                                                               wib_start)
             .count()
      << "usec\n"
      << "CRC32 copy time: "
      << std::chrono::duration_cast<std::chrono::microseconds>(colhead_start -
                                                               crc32_start)
             .count()
      << "usec\n"
      << "COLDATA header copy time: "
      << std::chrono::duration_cast<std::chrono::microseconds>(iadc_start -
                                                               crc32_start)
             .count()
      << "usec\n"
      << "Initial ADC copy time: "
      << std::chrono::duration_cast<std::chrono::microseconds>(adc_start -
                                                               iadc_start)
             .count()
      << "usec\n"
      << "ADC copy time: "
      << std::chrono::duration_cast<std::chrono::microseconds>(end - adc_start)
             .count()
      << "usec\n"
      << "Total copy time: "
      << std::chrono::duration_cast<std::chrono::microseconds>(end - wib_start)
             .count()
      << "usec\n\n";
}

// The following section of code generates frame from a reordered buffer, but requires FrameGen to work.
// void FelixReorderer::single_frame_from_buffer(const uint8_t* src, uint8_t* dest,
//                                               const size_t& i) {
//   framegen::Frame frame;
//   // Install WIB header.
//   const dune::FelixFragment::WIBHeader* head =
//       reinterpret_cast<dune::FelixFragment::WIBHeader const*>(
//           src + wib_headers_begin + wib_header_size * i);
//   frame.set_sof(head->sof);
//   frame.set_version(head->version);
//   frame.set_fiber_no(head->fiber_no);
//   frame.set_crate_no(head->crate_no);
//   frame.set_slot_no(head->slot_no);
//   frame.set_mm(head->mm);
//   frame.set_oos(head->oos);
//   frame.set_wib_errors(head->wib_errors);
//   frame.set_z(head->z);
//   frame.set_timestamp(head->timestamp());
//   frame.set_wib_counter(head->wib_counter);

//   // Install CRC32.
//   const uint32_t* cbegin =
//       reinterpret_cast<uint32_t const*>(src + crc32_begin + crc32_size * i);
//   frame.set_CRC32(*(cbegin));

//   // Install COLDATA headers.
//   frame.clearReserved();

//   const dune::FelixFragment::ColdataHeader* bhead =
//       reinterpret_cast<dune::FelixFragment::ColdataHeader const*>(
//           src + coldata_headers_begin +
//           coldata_header_size * i * num_blocks_per_frame);
//   for (unsigned j = 0; j < num_blocks_per_frame; ++j) {
//     frame.set_s1_error(j, bhead->s1_error);
//     frame.set_s2_error(j, bhead->s2_error);
//     frame.set_checksum_a(j, bhead->checksum_a());
//     frame.set_checksum_b(j, bhead->checksum_b());
//     frame.set_coldata_convert_count(j, bhead->coldata_convert_count);
//     frame.set_error_register(j, bhead->error_register);
//     for (unsigned k = 0; k < 8; ++k) {
//       frame.set_HDR(j, k, bhead->HDR(k));
//     }

//     ++bhead;
//   }

//   // Install COLDATA.
//   if (i == 0) {
//     for (unsigned j = 0; j < num_adcs_per_frame; ++j) {
//       const uint16_t* abegin =
//           reinterpret_cast<uint16_t const*>(src + initial_adc_begin);
//       frame.set_channel(j, *(abegin + j));
//       initial_adcs[j] = *(abegin + j);
//     }
//   } else {
//     for (unsigned j = 0; j < num_adcs_per_frame; ++j) {
//       const uint16_t* abegin = reinterpret_cast<uint16_t const*>(
//           src + adc_begin + num_adcs_per_frame * adc_size * (i - 1));
//       frame.set_channel(j, *(abegin + j) + initial_adcs[j]);
// #ifdef PREVIOUS_SUBTRACTION
//       initial_adcs[j] = *(abegin + j) + initial_adcs[j];
// #endif
//     }
//   }

//   // Clean up just in case.
//   frame.clearReserved();

//   // Copy to destination buffer.
//   memcpy(dest + i * frame_size, reinterpret_cast<char*>(&frame), frame_size);
// }

// void FelixReorderer::frames_from_buffer(const uint8_t* src, uint8_t* dest,
//                                         const size_t& num_frames) {
//   for (size_t i = 0; i < num_frames; ++i) {
//     single_frame_from_buffer(src, dest, i);
//   }
// }

artdaq::Fragment FelixReorder(const uint8_t* src, const size_t& num_frames) {
  FelixReorderer reorderer(src, num_frames);
  artdaq::Fragment result;
  result.resizeBytes(reorderer.newSize);
  reorderer.reorder_copy(result.dataBeginBytes());

  return result;
}

}  // namespace dune

#endif /* artdaq_dune_Overlays_FelixReorder_hh */