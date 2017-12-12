// FelixFragment.hh edited from ToyFragment.hh by Milo Vermeulen and Roland
// Sipos (2017) to accept WIB frames.

#ifndef artdaq_dune_Overlays_FelixFragment_hh
#define artdaq_dune_Overlays_FelixFragment_hh

// Uncomment to use the bit field ADC access method.
// MAV 30-10-2017: The bit field ADC access method is ~twice as fast and
// provides the same results as the alternative.
#define BITFIELD_METHOD

#include "FragmentType.hh"
#include "artdaq-core/Data/Fragment.hh"

#include <bitset>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// Implementation of "FelixFragment", an artdaq::FelixFragment overlay
// class used for WIB->FELIX frames.
//
// The intention of this class is to provide an Overlay for a 12-bit ADC
// module.

namespace dune {
  // fwd declare FelixFragment
  class FelixFragment;

  // Bit access function (from FrameGen).
  inline uint32_t get32BitRange(const uint32_t& word, const uint8_t& begin, const uint8_t& end) {
    return (word >> begin) & ((1 << (end - begin + 1)) - 1);
  }
}

/* The FelixFragment class interprets the data buffer
 * of an artdaq::Fragment, produced by a FELIX BoardReader. */
class dune::FelixFragment {

public:
  /* Constants for general FelixFragment use. */
  static constexpr size_t num_frame_hdr_words = 4;
  static constexpr size_t num_COLDATA_hdr_words = 4;
  static constexpr size_t num_frame_words = 117;
  static constexpr size_t num_frame_bytes = 468;
  static constexpr size_t num_COLDATA_words = 28;

  static constexpr size_t num_ch_per_frame = 256;
  static constexpr size_t num_ch_per_block = 64;
  static constexpr size_t num_stream_per_block = 8;
  static constexpr size_t num_ch_per_stream = 8;

  /* Define a public typedef for marking the WIB word size. */
  typedef uint32_t word_t;
  typedef uint16_t adc_t;  /* 12 bits in the current format. */
  typedef std::vector<adc_t> adc_v;

  /* Struct to hold FelixFragment Metadata. 
   * Currently not used, but it will be important
   * for the more complex Fragment version. */
  struct Metadata {
    typedef uint32_t data_t;

    data_t board_serial_number : 16;
    data_t num_adc_bits : 8;
    data_t unused : 8;

    static size_t const size_words = 1ul;  /* Units of Metadata::data_t */
  };

  static_assert( sizeof(Metadata) == Metadata::size_words * sizeof(Metadata::data_t), 
                 "FelixFragment::Metadata size changed");

  /* Static file reader for debugging purpose. */
  static artdaq::FragmentPtr fromFile(const std::string& fileName);


  /* FELIX-specific metadata from the FelixBoardReader included here for debugging. */
  struct FelixHeader {
    word_t flx_head;  // 4byte: 00 00 cd ab
  };


  /* Frame header from the WIB. */
  struct WIBHeader {
    word_t sof:8, version:5, fiber_no:3, slot_no:5, crate_no:3, /*reserved*/:8;
    word_t mm:1, oos:1, /*reserved*/:14, wib_errors:16;
    word_t timestamp_1 /*:32*/;
    word_t timestamp_2:16, wib_counter:15, z:1;

    uint64_t timestamp() const {
      uint64_t final_ts = (uint64_t)timestamp_1 | (uint64_t)timestamp_2 << 32;
      return bool(z) ? final_ts : final_ts | (uint64_t)wib_counter << 48;
    }

    uint16_t WIB_counter() const { return z ? wib_counter:0; }

    /* Print functions for debugging. */
    void print() const {
      std::cout << "SOF:" << unsigned(sof) << " version:" << unsigned(version)
                << " fiber:" << unsigned(fiber_no)
                << " slot:" << unsigned(slot_no)
                << " crate:" << unsigned(crate_no) << " mm:" << unsigned(mm)
                << " oos:" << unsigned(oos)
                << " wib_errors:" << unsigned(wib_errors)
                << " timestamp: " << timestamp() << '\n';
    }
    void printHex() const {
      std::cout << std::hex << "SOF:" << sof << " version:" << version
                << " fiber:" << fiber_no << " slot:" << slot_no
                << " crate:" << crate_no << " mm:" << mm << " oos:" << oos
                << " wib_errors:" << wib_errors << " timestamp: " << timestamp()
                << std::dec << '\n';
    }

    void printBits() const {
      std::cout << "SOF:" << std::bitset<8>(sof)
                << " version:" << std::bitset<5>(version)
                << " fiber:" << std::bitset<3>(fiber_no)
                << " slot:" << std::bitset<5>(slot_no)
                << " crate:" << std::bitset<3>(crate_no) << " mm:" << bool(mm)
                << " oos:" << bool(oos)
                << " wib_errors:" << std::bitset<16>(wib_errors)
                << " timestamp: " << timestamp() << '\n'
                << " Z: " << z << '\n';
    }
  };


  /* Coldata data containers in the WIB frame. */
  struct ColdataHeader {
    word_t s1_error:4, s2_error:4, /*reserved*/:8, checksum_a_1:8, checksum_b_1:8;
    word_t checksum_a_2:8, checksum_b_2:8, coldata_convert_count:16;
    word_t error_register:16, /*reserved*/:16;
    word_t hdr;

    uint16_t checksum_a() const {
      return (uint16_t)checksum_a_1 | ((uint16_t)checksum_a_2) << 8;
    }
    uint16_t checksum_b() const {
      return (uint16_t)checksum_b_1 | ((uint16_t)checksum_b_2) << 8;
    }
    uint8_t HDR(const uint8_t& num) const {
      uint8_t mask = (1 << 4) - 1;
      return (hdr >> ((num % 8) * 4)) & mask;
    }

    /* Print functions for debugging. */
    void print() const {
      std::cout << "s1_error:" << unsigned(s1_error)
                << " s2_error:" << unsigned(s2_error)
                << " checksum_a1:" << unsigned(checksum_a_1)
                << " checksum_b1:" << unsigned(checksum_b_1)
                << " checksum_a2:" << unsigned(checksum_a_2)
                << " checksum_b1:" << unsigned(checksum_b_2)
                << " coldata_convert_count:" << unsigned(coldata_convert_count)
                << " error_register:" << unsigned(error_register) << "\n";
      for (unsigned i = 0; i < 8; i++) {
        std::cout << " HDR" << i << ":" << unsigned(HDR(i));
      }
      std::cout << '\n';
    }
    void printHex() const {
      std::cout << std::hex << "s1_error:" << s1_error
                << " s2_error:" << s2_error << " checksum_a1:" << checksum_a_1
                << " checksum_b1:" << checksum_b_1
                << " checksum_a2:" << checksum_a_2
                << " checksum_b1:" << checksum_b_2
                << " coldata_convert_count:" << coldata_convert_count
                << " error_register:" << error_register << "\n";
      for (unsigned i = 0; i < 8; i++) {
        std::cout << " HDR" << i << ":" << std::hex << HDR(i);
      }
      std::cout << '\n';
    }
    void printBits() const {
      std::cout << "s1_error:" << std::bitset<4>(s1_error)
                << " s2_error:" << std::bitset<4>(s2_error)
                << " checksum_a1:" << std::bitset<8>(checksum_a_1)
                << " checksum_b1:" << std::bitset<8>(checksum_b_1)
                << " checksum_a2:" << std::bitset<8>(checksum_a_2)
                << " checksum_b2:" << std::bitset<8>(checksum_b_2)
                << " coldata_convert_count:"
                << std::bitset<16>(coldata_convert_count)
                << " error_register:" << std::bitset<16>(error_register)
                << "\n";
      for (unsigned i = 0; i < 8; i++) {
        std::cout << " HDR" << i << ":" << std::bitset<4>(HDR(i));
      }
      std::cout << '\n';
    }
  };


/* We provide 2 ways to access the ADC values. 
 * A version that is self explanatory, easy to understand,
 * but costy in calculation. 
 * And a second version that is a bit more cryptic,
 * but more efficient.
 * */
#ifdef BITFIELD_METHOD
  /* Each segment contains two streams.
   * This structure is heavily based on the ODS file that
   * explains the WIB -> FELIX data format. */
  struct ColdataSegment {
    /* Example: adc3ch5_2 = 2nd part of the 5th channel of the 3rd stream. */
    uint32_t adc1ch1_1:8, adc2ch1_1:8, adc1ch1_2:4, adc1ch2_1:4, 
             adc2ch1_2:4, adc2ch2_1:4;
    uint32_t adc1ch2_2:8, adc2ch2_2:8, adc1ch3_1:8, adc2ch3_1:8;
    uint32_t adc1ch3_2:4, adc1ch4_1:4, adc2ch3_2:4, adc2ch4_1:4, 
             adc1ch4_2:8, adc2ch4_2:8;
    uint32_t adc1ch5_1:8, adc2ch5_1:8, adc1ch5_2:4, adc1ch6_1:4, 
             adc2ch5_2:4, adc2ch6_1:4;
    uint32_t adc1ch6_2:8, adc2ch6_2:8, adc1ch7_1:8, adc2ch7_1:8;
    uint32_t adc1ch7_2:4, adc1ch8_1:4, adc2ch7_2:4, adc2ch8_1:4, 
             adc1ch8_2:8, adc2ch8_2:8;

    /* This could probably be prettier, 
     * and not the most efficient for sure. */
    adc_t channel(const uint8_t& adc, const uint8_t& ch) const {
      if (adc % 2 == 0) {
        switch (ch) {
          case 0:
            return adc1ch1_1 | adc1ch1_2 << 8;
            break;
          case 1:
            return adc1ch2_1 | adc1ch2_2 << 4;
            break;
          case 2:
            return adc1ch3_1 | adc1ch3_2 << 8;
            break;
          case 3:
            return adc1ch4_1 | adc1ch4_2 << 4;
            break;
          case 4:
            return adc1ch5_1 | adc1ch5_2 << 8;
            break;
          case 5:
            return adc1ch6_1 | adc1ch6_2 << 4;
            break;
          case 6:
            return adc1ch7_1 | adc1ch7_2 << 8;
            break;
          case 7:
            return adc1ch8_1 | adc1ch8_2 << 4;
            break;
        }
      } else {
        switch (ch) {
          case 0:
            return adc2ch1_1 | adc2ch1_2 << 8;
            break;
          case 1:
            return adc2ch2_1 | adc2ch2_2 << 4;
            break;
          case 2:
            return adc2ch3_1 | adc2ch3_2 << 8;
            break;
          case 3:
            return adc2ch4_1 | adc2ch4_2 << 4;
            break;
          case 4:
            return adc2ch5_1 | adc2ch5_2 << 8;
            break;
          case 5:
            return adc2ch6_1 | adc2ch6_2 << 4;
            break;
          case 6:
            return adc2ch7_1 | adc2ch7_2 << 8;
            break;
          case 7:
            return adc2ch8_1 | adc2ch8_2 << 4;
            break;
        }
      }
      // Something went wrong.
      return 0;
    }

    adc_t channel(const uint8_t& ch) const {
      return channel(ch / num_ch_per_stream, ch % num_ch_per_stream);
    }
  };
#endif

  struct ColdataBlock {
    ColdataHeader head;

#ifdef BITFIELD_METHOD
    ColdataSegment segment[4];

    adc_t channel(uint8_t adc, uint8_t ch) const {
      uint8_t segment_num = adc / 2;
      uint8_t adc_num = adc % 2;
      return segment[segment_num].channel(adc_num, ch);
    }

    adc_t channel(uint8_t ch) const {
      return channel(ch / num_ch_per_stream, ch % num_ch_per_stream);
    }
#else

    word_t adcs[24];

    adc_t channel(const uint8_t& adc, const uint8_t& ch) const {
      /* All channel values are split in a first and second part. Because two
       * streams are put side-by-side, the effective word size per stream is 16 bits. */
      uint8_t first_word = (adc / 2) * 6 + 12 * ch / 16;
      uint8_t first_offset = (12 * ch) % 16;

      /* The split is at 8 bits for even channels and at 4 for odd ones. */
      uint8_t split = 4 * (2 - ch % 2);

      uint8_t second_word = first_word + (first_offset + split) / 16;
      uint8_t second_offset = (first_offset + split) % 16;

      /* Move offsets 8-15 to 16-23. */
      first_offset += (first_offset / 8) * 8;
      second_offset += (second_offset / 8) * 8;

      /* Left-shift odd streams by 8 bits. */
      first_offset += (adc % 2) * 8;
      second_offset += (adc % 2) * 8;

      return get32BitRange(adcs[first_word], first_offset, first_offset + split - 1) 
           | get32BitRange(adcs[second_word], second_offset, second_offset + 12 - split - 1)
           << split;
    }
#endif

    void printADCs() const {
      std::cout << "\t\t0\t1\t2\t3\t4\t5\t6\t7\n";
      for (int i = 0; i < 8; i++) {
        std::cout << "Stream " << i << ":\t";
        for (int j = 0; j < 8; j++) {
          std::cout << std::hex << channel(i, j) << '\t';
        }
        std::cout << '\n';
      }
    }
  };


  /* The WIBFrame struct. */
  struct WIBFrame {
    WIBHeader head; // Contains a single WIB Header
    ColdataBlock block[4]; // Contains 4 ColdataBlocks
    word_t CRC32; // And a CRC32 

    /* Accessors for ADCs. */
    adc_t channel(const uint8_t& block_num, const uint8_t& adc, const uint8_t& ch) const {
      return block[block_num].channel(adc, ch);
    }
    adc_t channel(const uint8_t& ch) const {
      uint8_t block_num = ch / num_ch_per_block;
      uint8_t adc_num = (ch % num_ch_per_block) / num_ch_per_stream;
      uint8_t ch_num = ch % num_ch_per_stream;
      return channel(block_num, adc_num, ch_num);
    }

    /* WIB header accessors. */
    uint8_t sof() const { return head.sof; }
    uint8_t version() const { return head.version; }
    uint8_t fiber_no() const { return head.fiber_no; }
    uint8_t slot_no() const { return head.slot_no; }
    uint8_t crate_no() const { return head.crate_no; }
    uint8_t mm() const { return head.mm; }
    uint8_t oos() const { return head.oos; }
    uint16_t wib_errors() const { return head.wib_errors; }
    uint64_t timestamp() const { return head.timestamp(); }
    uint16_t wib_counter() const { return head.wib_counter; }

    // Coldata block accessors.
    uint8_t s1_error(const uint8_t& block_num) const {
      return block[block_num].head.s1_error;
    }
    uint8_t s2_error(const uint8_t& block_num) const {
      return block[block_num].head.s2_error;
    }
    uint16_t checksum_a(const uint8_t& block_num) const {
      return block[block_num].head.checksum_a();
    }
    uint16_t checksum_b(const uint8_t& block_num) const {
      return block[block_num].head.checksum_b();
    }
    uint16_t coldata_convert_count(const uint8_t& block_num) const {
      return block[block_num].head.coldata_convert_count;
    }
    uint16_t error_register(const uint8_t& block_num) const {
      return block[block_num].head.error_register;
    }
    uint8_t HDR(const uint8_t& block_num, const uint8_t& HDR_num) const {
      return block[block_num].head.HDR(HDR_num);
    }

    void print() const {
      std::cout << "### WOOF: \n Header: \n";
      head.printHex();
      for (unsigned i = 0; i < 4; ++i) {
        std::cout << "CD block " << i << ".:\n";
        block[i].head.print();
        block[i].printADCs();
      }
    }
  };

  /* Frame field and accessors. */
  uint8_t sof(const unsigned& frame_ID) const {
    return frame_(frame_ID)->head.sof;
  }
  uint8_t version(const unsigned& frame_ID) const {
    return frame_(frame_ID)->head.version;
  }
  uint8_t fiber_no(const unsigned& frame_ID) const {
    return frame_(frame_ID)->head.fiber_no;
  }
  uint8_t slot_no(const unsigned& frame_ID) const {
    return frame_(frame_ID)->head.slot_no;
  }
  uint8_t crate_no(const unsigned& frame_ID) const {
    return frame_(frame_ID)->head.crate_no;
  }
  uint8_t mm(const unsigned& frame_ID) const {
    return frame_(frame_ID)->head.mm;
  }
  uint8_t oos(const unsigned& frame_ID) const {
    return frame_(frame_ID)->head.oos;
  }
  uint16_t wib_errors(const unsigned& frame_ID) const {
    return frame_(frame_ID)->head.wib_errors;
  }
  uint64_t timestamp(const unsigned& frame_ID) const {
    return frame_(frame_ID)->head.timestamp();
  }
  uint16_t wib_counter(const unsigned& frame_ID) const {
    return frame_(frame_ID)->head.wib_counter;
  }

  /* Coldata block accessors. */
  uint8_t s1_error(const unsigned& frame_ID, const uint8_t& block_num) const {
    return frame_(frame_ID)->block[block_num].head.s1_error;
  }
  uint8_t s2_error(const unsigned& frame_ID, const uint8_t& block_num) const {
    return frame_(frame_ID)->block[block_num].head.s2_error;
  }
  uint16_t checksum_a(const unsigned& frame_ID,
                      const uint8_t& block_num) const {
    return frame_(frame_ID)->block[block_num].head.checksum_a();
  }
  uint16_t checksum_b(const unsigned& frame_ID,
                      const uint8_t& block_num) const {
    return frame_(frame_ID)->block[block_num].head.checksum_b();
  }
  uint16_t coldata_convert_count(const unsigned& frame_ID,
                                 const uint8_t& block_num) const {
    return frame_(frame_ID)->block[block_num].head.coldata_convert_count;
  }
  uint16_t error_register(const unsigned& frame_ID,
                          const uint8_t& block_num) const {
    return frame_(frame_ID)->block[block_num].head.error_register;
  }
  uint8_t HDR(const unsigned& frame_ID, const uint8_t& block_num,
              const uint8_t& HDR_num) const {
    return frame_(frame_ID)->block[block_num].head.HDR(HDR_num);
  }
  word_t CRC32(const unsigned& frame_ID) const {
    return frame_(frame_ID)->CRC32;
  }

  // Function to return a given ADC value.
  adc_t get_ADC(uint64_t frame_ID, uint8_t block_ID, uint8_t stream_ID,
                uint8_t channel_ID) const;
  adc_t get_ADC(uint64_t frame_ID, uint8_t channel_ID) const;

  // Function to return all ADC values for a single channel.
  adc_v get_ADCs_by_channel(uint8_t block_ID, uint8_t stream_ID,
                            uint8_t channel_ID) const;
  adc_v get_ADCs_by_channel(uint8_t channel_ID) const;
  // Function to return all ADC values for all channels in a map.
  std::map<uint8_t, adc_v> get_all_ADCs() const;

  // Function to print all timestamps.
  void print_timestamps() const {
    for (unsigned int i = 0; i < total_frames(); i++) {
      std::cout << std::hex << frame_(i)->timestamp() << '\t' << std::dec << i
                << std::endl;
    }
  }

  void print_frames() const {
    for (unsigned i = 0; i < total_frames(); i++) {
      frame_(i)->print();
    }
  }

  // The constructor simply sets its const private member "artdaq_Fragment_"
  // to refer to the artdaq::Fragment object
  FelixFragment(artdaq::Fragment const& fragment) : artdaq_Fragment_(fragment) {
    std::cout << "Attempting to create a FELIX Fragment.\n";
  }

  // The number of words in the current event minus the header.
  size_t total_words() const {
    return artdaq_Fragment_.dataSizeBytes() / sizeof(word_t);
    // artdaq::Fragments store their data in uint64_t as opposed to the uint32_t
    // used by FelixFragment.
  }

  // The number of frames in the current event.
  size_t total_frames() const { return total_words() / num_frame_words; }

  // The number of ADC values describing data beyond the header
  size_t total_adc_values() const { return total_frames() * num_ch_per_frame; }

  // Largest ADC value possible
  size_t adc_range(int daq_adc_bits = 12) { return (1ul << daq_adc_bits); }

  // Functions like findBadADCs to check whether any ADC values are corrupt
  // are to be added.

 protected:
  // Allow access to individual frames according to the WIBFrame structure.
  WIBFrame const* frame_(const unsigned& frame_num = 0) const {
    return reinterpret_cast<dune::FelixFragment::WIBFrame const*>(
        artdaq_Fragment_.dataBeginBytes() + frame_num * num_frame_bytes);
  }

 private:
  artdaq::Fragment const& artdaq_Fragment_;
};

#endif /* artdaq_dune_Overlays_FelixFragment_hh */

