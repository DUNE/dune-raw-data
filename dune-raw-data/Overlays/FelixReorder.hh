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

#define NUMBER_THREADS 1

namespace dune {

class FelixReorderer {
 public:
  static const unsigned netio_header_size = 0;
  static const unsigned frame_size = sizeof(FelixFrame);
  static const unsigned wib_header_size = sizeof(WIBHeader);
  static const unsigned coldata_header_size = sizeof(ColdataHeader);
  static const unsigned coldata_block_size = sizeof(ColdataBlock);
  // static const unsigned crc_size = 4;

  static const unsigned adc_size = sizeof(adc_t);

  static const unsigned num_ch_per_frame = FelixFrame::num_ch_per_frame;
  static const unsigned num_blocks_per_frame = FelixFrame::num_block_per_frame;
  static const unsigned num_streams_per_block = 8;

  static const unsigned num_ch_per_block = FelixFrame::num_ch_per_block;
    static const unsigned num_adcs_per_stream =
      num_ch_per_block / num_streams_per_block;

 private:
  const uint8_t* head;
  const size_t num_frames;
  const size_t size = num_frames * frame_size;

  uint16_t initial_adcs[num_ch_per_frame];

  void wib_header_copy(uint8_t* dest);
  void coldata_header_copy(uint8_t* dest);
  void initial_adc_copy(uint8_t* dest);
  void adc_copy(uint8_t* dest);

 public:
  FelixReorderer(const uint8_t* data, const size_t& num_frames = 10000)
      : head(data), num_frames(num_frames){};

  const unsigned newSize =
      netio_header_size +
      (wib_header_size + coldata_header_size * num_blocks_per_frame + adc_size * num_ch_per_frame) * num_frames;

  // Locations in the destination buffer.
  const unsigned wib_headers_begin = netio_header_size;
  const unsigned coldata_headers_begin = wib_headers_begin + num_frames * wib_header_size;
  const unsigned adc_begin =
      coldata_headers_begin +
      num_frames * coldata_header_size * num_blocks_per_frame;
  const unsigned end =
      adc_begin + (num_frames - 1) * num_ch_per_frame * adc_size;

  void reorder_copy(uint8_t* dest);
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
  for (unsigned i = 0; i < num_ch_per_frame; ++i) {
    initial_adcs[i] = src->channel(i);
    memcpy(dest, &initial_adcs[i], adc_size);
    dest += adc_size;
  }
}

// ADC copy function to be executed by individual threads.
void t_adc_copy_by_channel(FelixReorderer* reord, uint8_t* dest,
                           const unsigned& t_inst, const unsigned& t_tot) {
  // Thread starting point and range in ADC channel space.
  unsigned t_ch_range = reord->num_ch_per_frame / t_tot;
  unsigned t_ch_begin = t_ch_range * t_inst;
  // The last thread should clean up the remainder of the channels.
  if (t_inst == t_tot - 1 && reord->num_ch_per_frame % t_tot != 0) {
    t_ch_range = reord->num_ch_per_frame - t_ch_begin;
  }

  // Store all ADC values in uint16_t.
  const dune::FelixFrame* src =
      reinterpret_cast<dune::FelixFrame const*>(
          reord->head + reord->netio_header_size);
          
  for (unsigned fr = 0; fr < reord->num_frames; ++fr) {
    for (unsigned ch = t_ch_begin; ch < t_ch_begin + t_ch_range; ++ch) {
      adc_t curr_val = (src + fr)->channel(ch);
      // Only for previous value subtraction.
      // if(fr != 0) {
      //   curr_val -= (src + fr - 1)->channel(ch);
      // }
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
  // auto wib_start = std::chrono::high_resolution_clock::now();
  wib_header_copy(dest + wib_headers_begin);
  // auto colhead_start = std::chrono::high_resolution_clock::now();
  coldata_header_copy(dest + coldata_headers_begin);
  // auto adc_start = std::chrono::high_resolution_clock::now();
  adc_copy(dest + adc_begin);
  // auto end = std::chrono::high_resolution_clock::now();

  // std::cout
  //     << "\nMemcpy speed tests:\n"
  //     << "WIB header copy time: "
  //     << std::chrono::duration_cast<std::chrono::microseconds>(colhead_start -
  //                                                              wib_start)
  //            .count()
  //     << "usec\n"
  //     // << "CRC copy time: "
  //     // << std::chrono::duration_cast<std::chrono::microseconds>(crc_start -
  //     //                                                          colhead_start)
  //     //        .count()
  //     // << "usec\n"
  //     << "COLDATA header copy time: "
  //     << std::chrono::duration_cast<std::chrono::microseconds>(adc_start -
  //                                                              colhead_start)
  //            .count()
  //     << "usec\n"
  //     << "ADC copy time: "
  //     << std::chrono::duration_cast<std::chrono::microseconds>(end - adc_start)
  //            .count()
  //     << "usec\n"
  //     << "Total copy time: "
  //     << std::chrono::duration_cast<std::chrono::microseconds>(end - wib_start)
  //            .count()
  //     << "usec\n\n";
}

artdaq::Fragment FelixReorder(const uint8_t* src, const size_t& num_frames = 10000) {
  FelixReorderer reorderer(src, num_frames);
  artdaq::Fragment result;
  result.resizeBytes(reorderer.newSize);
  reorderer.reorder_copy(result.dataBeginBytes());

  return result;
}

}  // namespace dune

#endif /* artdaq_dune_Overlays_FelixReorder_hh */
