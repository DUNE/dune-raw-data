// This code was written for the 1.0 WIB->FELIX frame format.
// 10-10-2017 Milo Vermeulen

#ifndef artdaq_dune_Overlays_FelixReorder_hh
#define artdaq_dune_Overlays_FelixReorder_hh

#include <chrono>
#include <fstream>
#include <thread>
#include <vector>

#include "artdaq-core/Data/Fragment.hh"
#include "dune-raw-data/Overlays/FelixFormat.hh"

#define NUMBER_THREADS 6

namespace dune {

class FelixReorderer {
 public:
  static const unsigned netio_header_size = 0;
  static const unsigned frame_size = 120 * 4;
  static const unsigned wib_header_size = 4 * 4;
  static const unsigned coldata_header_size = 4 * 4;
  static const unsigned coldata_block_size = 28 * 4;
  static const unsigned crc_size = 4;

  static const unsigned adc_size = 2;

  static const unsigned num_adcs_per_frame = 256;
  static const unsigned num_blocks_per_frame = 4;
  static const unsigned num_streams_per_block = 8;

  static const unsigned num_adcs_per_block = 64;
    static const unsigned num_adcs_per_stream =
      num_adcs_per_block / num_streams_per_block;

 private:
  const uint8_t* head;
  const size_t num_frames;
  const size_t size = num_frames * frame_size;

  uint16_t initial_adcs[num_adcs_per_frame];

  void wib_header_copy(uint8_t* dest);
  void crc_copy(uint8_t* dest);
  void coldata_header_copy(uint8_t* dest);
  void initial_adc_copy(uint8_t* dest);
  void adc_copy(uint8_t* dest);

 public:
  FelixReorderer(const uint8_t* data, const size_t& num_frames = 10000)
      : head(data), num_frames(num_frames){};

  const unsigned newSize =
      netio_header_size +
      (wib_header_size + coldata_header_size * num_blocks_per_frame +
       crc_size + adc_size * num_adcs_per_frame) *
          num_frames;

  // Locations in the destination buffer.
  const unsigned wib_headers_begin = netio_header_size;
  const unsigned crc_begin = wib_headers_begin + num_frames * wib_header_size;
  const unsigned coldata_headers_begin = crc_begin + num_frames * crc_size;
  const unsigned adc_begin =
      coldata_headers_begin +
      num_frames * coldata_header_size * num_blocks_per_frame;
  const unsigned end =
      adc_begin + (num_frames - 1) * num_adcs_per_frame * adc_size;

  void reorder_copy(uint8_t* dest);
  friend void t_adc_copy_by_tick(FelixReorderer* reord, uint8_t* dest,
                                 const unsigned& t_inst, const unsigned& t_tot);
  friend void t_adc_copy_by_channel(FelixReorderer* reord, uint8_t* dest,
                                    const unsigned& t_inst,
                                    const unsigned& t_tot);
  void single_frame_from_buffer(const uint8_t* src, uint8_t* dest,
                                const size_t& i);
  void frames_from_buffer(const uint8_t* src, uint8_t* dest,
                          const size_t& num_frames);
};

void FelixReorderer::wib_header_copy(uint8_t* dest) {
  // Store WIB-headers next to each other.
  const dune::FelixFrame* src =
      reinterpret_cast<dune::FelixFrame const*>(head + netio_header_size);
  for (unsigned i = 0; i < num_frames; ++i) {
    memcpy(dest, src->wib_header(), wib_header_size);
    dest += wib_header_size;
    ++src;
  }
}

void FelixReorderer::crc_copy(uint8_t* dest) {
  // Store CRCs next to each other.
  const dune::FelixFrame* src =
      reinterpret_cast<dune::FelixFrame const*>(head + netio_header_size);
  for (unsigned i = 0; i < num_frames; ++i) {
    const uint32_t curr_crc = src->CRC();
    memcpy(dest, &curr_crc, crc_size);
    dest += crc_size;
    ++src;
  }
}

void FelixReorderer::coldata_header_copy(uint8_t* dest) {
  // Store COLDATA headers next to each other.
  const dune::FelixFrame* src =
      reinterpret_cast<dune::FelixFrame const*>(head + netio_header_size);
  for (unsigned i = 0; i < num_frames; ++i) {
    for (unsigned j = 0; j < num_blocks_per_frame; ++j) {
      memcpy(dest, src->coldata_header(j), coldata_header_size);
      dest += coldata_header_size;
    }
    ++src;
  }
}

void FelixReorderer::initial_adc_copy(uint8_t* dest) {
  // Store all initial ADC values in uint16_t.
  const dune::FelixFrame* src =
      reinterpret_cast<dune::FelixFrame const*>(head + netio_header_size);
  for (unsigned i = 0; i < num_adcs_per_frame; ++i) {
    initial_adcs[i] = src->channel(i);
    memcpy(dest, &initial_adcs[i], adc_size);
    dest += adc_size;
  }
}

// ADC copy function to be executed by individual threads.
void t_adc_copy_by_tick(FelixReorderer* reord, uint8_t* dest,
                        const unsigned& t_inst, const unsigned& t_tot) {
  // Thread starting point and range in ADC channel space.
  unsigned t_ch_range = reord->num_adcs_per_frame / t_tot;
  unsigned t_ch_begin = t_ch_range * t_inst;
  // The last thread should clean up the remainder of the channels.
  if (t_inst == t_tot - 1 && reord->num_adcs_per_frame % t_tot != 0) {
    t_ch_range = reord->num_adcs_per_frame - t_ch_begin;
  }
  
  // Store all ADC values in uint16_t.
  const dune::FelixFrame* src =
      reinterpret_cast<dune::FelixFrame const*>(
          reord->head + reord->netio_header_size);
  for (unsigned fr = 0; fr < reord->num_frames; ++fr) {
    for (unsigned ch = t_ch_begin; ch < t_ch_begin + t_ch_range; ++ch) {
      uint16_t reduced_adc = (src + fr)->channel(ch);
      const size_t dest_offset =
          ((fr - 1) * reord->num_adcs_per_frame + ch) * reord->adc_size;
      memcpy(dest + dest_offset, &reduced_adc, reord->adc_size);
    }
  }
}

// ADC copy function to be executed by individual threads.
void t_adc_copy_by_channel(FelixReorderer* reord, uint8_t* dest,
                           const unsigned& t_inst, const unsigned& t_tot) {
  // Thread starting point and range in ADC channel space.
  unsigned t_ch_range = reord->num_adcs_per_frame / t_tot;
  unsigned t_ch_begin = t_ch_range * t_inst;
  // The last thread should clean up the remainder of the channels.
  if (t_inst == t_tot - 1 && reord->num_adcs_per_frame % t_tot != 0) {
    t_ch_range = reord->num_adcs_per_frame - t_ch_begin;
  }

  // Store all ADC values in uint16_t.
  const dune::FelixFrame* src =
      reinterpret_cast<dune::FelixFrame const*>(
          reord->head + reord->netio_header_size);
  for (unsigned fr = 0; fr < reord->num_frames; ++fr) {
    for (unsigned ch = t_ch_begin; ch < t_ch_begin + t_ch_range; ++ch) {
      adc_t curr_val = (src + fr)->channel(ch);
      memcpy(dest + (ch * reord->num_frames + fr) * reord->adc_size, &curr_val,
             reord->adc_size);
    }
  }
}

void FelixReorderer::adc_copy(uint8_t* dest) {
  const unsigned t_tot = NUMBER_THREADS;
  std::thread* t_vec = new std::thread[t_tot - 1];

  for (unsigned i = 0; i < t_tot - 1; ++i) {
    t_vec[i] = std::thread(t_adc_copy_by_channel, this, dest, i, t_tot);
  }
  t_adc_copy_by_channel(this, dest, t_tot - 1, t_tot);
  for (unsigned i = 0; i < t_tot - 1; ++i) {
    t_vec[i].join();
  }
}

void FelixReorderer::reorder_copy(uint8_t* dest) {
  auto wib_start = std::chrono::high_resolution_clock::now();
  wib_header_copy(dest + wib_headers_begin);
  auto colhead_start = std::chrono::high_resolution_clock::now();
  coldata_header_copy(dest + coldata_headers_begin);
  auto crc_start = std::chrono::high_resolution_clock::now();
  crc_copy(dest + crc_begin);
  auto adc_start = std::chrono::high_resolution_clock::now();
  adc_copy(dest + adc_begin);
  auto end = std::chrono::high_resolution_clock::now();

  std::cout
      << "\nMemcpy speed tests:\n"
      << "WIB header copy time: "
      << std::chrono::duration_cast<std::chrono::microseconds>(colhead_start -
                                                               wib_start)
             .count()
      << "usec\n"
      << "CRC copy time: "
      << std::chrono::duration_cast<std::chrono::microseconds>(crc_start -
                                                               colhead_start)
             .count()
      << "usec\n"
      << "COLDATA header copy time: "
      << std::chrono::duration_cast<std::chrono::microseconds>(adc_start -
                                                               crc_start)
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

// The following section of code generates frames from a reordered buffer, but
// requires FrameGen to work.
// void FelixReorderer::single_frame_from_buffer(const uint8_t* src, uint8_t*
// dest,
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

//   // Install CRC.
//   const uint32_t* cbegin =
//       reinterpret_cast<uint32_t const*>(src + crc_begin + crc_size * i);
//   frame.set_CRC(*(cbegin));

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

artdaq::Fragment FelixReorder(const uint8_t* src, const size_t& num_frames = 10000) {
  FelixReorderer reorderer(src, num_frames);
  artdaq::Fragment result;
  result.resizeBytes(reorderer.newSize);
  reorderer.reorder_copy(result.dataBeginBytes());

  return result;
}

}  // namespace dune

#endif /* artdaq_dune_Overlays_FelixReorder_hh */