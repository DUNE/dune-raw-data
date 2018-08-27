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
#include "dune-raw-data/Overlays/FelixFormat.hh"

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
class FelixFragmentBase;
class FelixFragmentUnordered;
class FelixFragmentReordered;
class FelixFragment;

// Bit access function (from FrameGen).
inline uint32_t get32BitRange(const uint32_t& word, const uint8_t& begin,
                              const uint8_t& end) {
  return (word >> begin) & ((1 << (end - begin + 1)) - 1);
}
}  // namespace dune

//============================
// FELIX fragment base class
//============================
class dune::FelixFragmentBase {
 public:
  /* Struct to hold FelixFragment Metadata. */
  struct Metadata {
    typedef uint32_t data_t;

    data_t num_frames : 16;
    data_t reordered : 8;
    data_t compressed : 8;

    static size_t const size_words = 1ul; /* Units of Metadata::data_t */
  };

  static_assert(sizeof(Metadata) ==
                    Metadata::size_words * sizeof(Metadata::data_t),
                "FelixFragment::Metadata size changed");

  /* Static file reader for debugging purpose. */
  static artdaq::FragmentPtr fromFile(const std::string& fileName) {
    std::ifstream in(fileName, std::ios::binary);
    std::string contents((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    Metadata meta;
    static size_t fragment_ID = 1;
    std::unique_ptr<artdaq::Fragment> frag_ptr(artdaq::Fragment::FragmentBytes(
        contents.size(), 1, fragment_ID, dune::toFragmentType("FELIX"), meta));
    ++fragment_ID;

    frag_ptr->resizeBytes(contents.size());
    memcpy(frag_ptr->dataBeginBytes(), contents.c_str(), contents.size());
    in.close();
    return frag_ptr;
  }

  /* FELIX-specific metadata from the FelixBoardReader included here for
   * debugging. */
  struct FelixHeader;

  /* Frame field and accessors. */
  virtual uint8_t sof(const unsigned& frame_ID = 0) const = 0;
  virtual uint8_t version(const unsigned& frame_ID = 0) const = 0;
  virtual uint8_t fiber_no(const unsigned& frame_ID = 0) const = 0;
  virtual uint8_t slot_no(const unsigned& frame_ID = 0) const = 0;
  virtual uint8_t crate_no(const unsigned& frame_ID = 0) const = 0;
  virtual uint8_t mm(const unsigned& frame_ID = 0) const = 0;
  virtual uint8_t oos(const unsigned& frame_ID = 0) const = 0;
  virtual uint16_t wib_errors(const unsigned& frame_ID = 0) const = 0;
  virtual uint64_t timestamp(const unsigned& frame_ID = 0) const = 0;
  virtual uint16_t wib_counter(const unsigned& frame_ID = 0) const = 0;

  /* Coldata block accessors. */
  virtual uint8_t s1_error(const unsigned& frame_ID,
                           const uint8_t& block_num) const = 0;
  virtual uint8_t s2_error(const unsigned& frame_ID,
                           const uint8_t& block_num) const = 0;
  virtual uint16_t checksum_a(const unsigned& frame_ID,
                              const uint8_t& block_num) const = 0;
  virtual uint16_t checksum_b(const unsigned& frame_ID,
                              const uint8_t& block_num) const = 0;
  virtual uint16_t coldata_convert_count(const unsigned& frame_ID,
                                         const uint8_t& block_num) const = 0;
  virtual uint16_t error_register(const unsigned& frame_ID,
                                  const uint8_t& block_num) const = 0;
  virtual uint8_t hdr(const unsigned& frame_ID, const uint8_t& block_num,
                      const uint8_t& hdr_num) const = 0;

  // Functions to return a certain ADC value.
  virtual adc_t get_ADC(const unsigned& frame_ID, const uint8_t block_ID,
                        const uint8_t channel_ID) const = 0;
  virtual adc_t get_ADC(const unsigned& frame_ID,
                        const uint8_t channel_ID) const = 0;

  // Function to return all ADC values for a single channel.
  virtual adc_v get_ADCs_by_channel(const uint8_t block_ID,
                                    const uint8_t channel_ID) const = 0;
  virtual adc_v get_ADCs_by_channel(const uint8_t channel_ID) const = 0;
  // Function to return all ADC values for all channels in a map.
  virtual std::map<uint8_t, adc_v> get_all_ADCs() const = 0;

  // Function to print all timestamps.
  virtual void print_timestamps() const = 0;

  virtual void print(const unsigned i) const = 0;

  virtual void print_frames() const = 0;

  FelixFragmentBase(const artdaq::Fragment& fragment)
      : meta_(*(fragment.metadata<Metadata>())),
        artdaq_Fragment_(fragment.dataBeginBytes()),
        sizeBytes_(fragment.dataSizeBytes()) {}
  virtual ~FelixFragmentBase() {}

  // The number of words in the current event minus the header.
  virtual size_t total_words() const = 0;
  // The number of frames in the current event.
  virtual size_t total_frames() const = 0;
  // The number of ADC values describing data beyond the header
  virtual size_t total_adc_values() const = 0;
  // Largest ADC value possible
  virtual size_t adc_range(int daq_adc_bits = 12) {
    return (1ul << daq_adc_bits);
  }

  // Raw data access (const only).
  const uint8_t* dataBeginBytes() const {
    return reinterpret_cast<uint8_t const*>(artdaq_Fragment_);
  }
  size_t dataSizeBytes() const { return sizeBytes_; }

 protected:
  Metadata meta_;
  const void* artdaq_Fragment_;
  size_t sizeBytes_;
};

//==================================================
// FELIX fragment for an array of bare FELIX frames
//==================================================
class dune::FelixFragmentUnordered : public dune::FelixFragmentBase {
 public:
  /* FELIX-specific metadata from the FelixBoardReader included here for
   * debugging. */
  struct FelixHeader {
    word_t flx_head;  // 4byte: 00 00 cd ab
  };

  /* Frame field and accessors. */
  uint8_t sof(const unsigned& frame_ID = 0) const {
    return frame_(frame_ID)->sof();
  }
  uint8_t version(const unsigned& frame_ID = 0) const {
    return frame_(frame_ID)->version();
  }
  uint8_t fiber_no(const unsigned& frame_ID = 0) const {
    return frame_(frame_ID)->fiber_no();
  }
  uint8_t slot_no(const unsigned& frame_ID = 0) const {
    return frame_(frame_ID)->slot_no();
  }
  uint8_t crate_no(const unsigned& frame_ID = 0) const {
    return frame_(frame_ID)->crate_no();
  }
  uint8_t mm(const unsigned& frame_ID = 0) const {
    return frame_(frame_ID)->mm();
  }
  uint8_t oos(const unsigned& frame_ID = 0) const {
    return frame_(frame_ID)->oos();
  }
  uint16_t wib_errors(const unsigned& frame_ID = 0) const {
    return frame_(frame_ID)->wib_errors();
  }
  uint64_t timestamp(const unsigned& frame_ID = 0) const {
    return frame_(frame_ID)->timestamp();
  }
  uint16_t wib_counter(const unsigned& frame_ID = 0) const {
    return frame_(frame_ID)->wib_counter();
  }

  /* Coldata block accessors. */
  uint8_t s1_error(const unsigned& frame_ID, const uint8_t& block_num) const {
    return frame_(frame_ID)->s1_error(block_num);
  }
  uint8_t s2_error(const unsigned& frame_ID, const uint8_t& block_num) const {
    return frame_(frame_ID)->s2_error(block_num);
  }
  uint16_t checksum_a(const unsigned& frame_ID,
                      const uint8_t& block_num) const {
    return frame_(frame_ID)->checksum_a(block_num);
  }
  uint16_t checksum_b(const unsigned& frame_ID,
                      const uint8_t& block_num) const {
    return frame_(frame_ID)->checksum_b(block_num);
  }
  uint16_t coldata_convert_count(const unsigned& frame_ID,
                                 const uint8_t& block_num) const {
    return frame_(frame_ID)->coldata_convert_count(block_num);
  }
  uint16_t error_register(const unsigned& frame_ID,
                          const uint8_t& block_num) const {
    return frame_(frame_ID)->error_register(block_num);
  }
  uint8_t hdr(const unsigned& frame_ID, const uint8_t& block_num,
              const uint8_t& hdr_num) const {
    return frame_(frame_ID)->hdr(block_num, hdr_num);
  }

  // Functions to return a certain ADC value.
  adc_t get_ADC(const unsigned& frame_ID, const uint8_t block_ID,
                const uint8_t channel_ID) const {
    return frame_(frame_ID)->channel(block_ID, channel_ID);
  }
  adc_t get_ADC(const unsigned& frame_ID, const uint8_t channel_ID) const {
    return frame_(frame_ID)->channel(channel_ID);
  }

  // Function to return all ADC values for a single channel.
  adc_v get_ADCs_by_channel(const uint8_t block_ID,
                            const uint8_t channel_ID) const {
    adc_v output(total_frames());
    for (size_t i = 0; i < total_frames(); i++) {
      output[i] = get_ADC(i, block_ID, channel_ID);
    }
    return output;
  }
  adc_v get_ADCs_by_channel(const uint8_t channel_ID) const {
    adc_v output(total_frames());
    for (size_t i = 0; i < total_frames(); i++) {
      output[i] = get_ADC(i, channel_ID);
    }
    return output;
  }
  // Function to return all ADC values for all channels in a map.
  std::map<uint8_t, adc_v> get_all_ADCs() const {
    std::map<uint8_t, adc_v> output;
    for (int i = 0; i < 256; i++)
      output.insert(std::pair<uint8_t, adc_v>(i, get_ADCs_by_channel(i)));
    return output;
  }

  // Function to print all timestamps.
  void print_timestamps() const {
    for (unsigned int i = 0; i < total_frames(); i++) {
      std::cout << std::hex << frame_(i)->timestamp() << '\t' << std::dec << i
                << std::endl;
    }
  }

  void print(const unsigned i) const { frame_(i)->print(); }

  void print_frames() const {
    for (unsigned i = 0; i < total_frames(); i++) {
      frame_(i)->print();
    }
  }

  // The constructor simply sets its const private member "artdaq_Fragment_"
  // to refer to the artdaq::Fragment object
  FelixFragmentUnordered(artdaq::Fragment const& fragment)
      : FelixFragmentBase(fragment) {}

  // The number of words in the current event minus the header.
  size_t total_words() const { return sizeBytes_ / sizeof(word_t); }
  // The number of frames in the current event.
  size_t total_frames() const {
    return total_words() / FelixFrame::num_frame_words;
  }
  // The number of ADC values describing data beyond the header
  size_t total_adc_values() const {
    return total_frames() * FelixFrame::num_ch_per_frame;
  }

 protected:
  // Allow access to individual frames according to the FelixFrame structure.
  FelixFrame const* frame_(const unsigned& frame_num = 0) const {
    return static_cast<dune::FelixFrame const*>(artdaq_Fragment_) + frame_num;
  }
};

//=======================================================
// FELIX fragment for an array of reordered FELIX frames
//=======================================================
class dune::FelixFragmentReordered : public dune::FelixFragmentBase {
 public:
  /* FELIX-specific metadata from the FelixBoardReader included here for
   * debugging. */
  struct FelixHeader {};

  /* Frame field and accessors. */
  uint8_t sof(const unsigned& frame_ID = 0) const {
    return head_(frame_ID)->sof;
  }
  uint8_t version(const unsigned& frame_ID = 0) const {
    return head_(frame_ID)->version;
  }
  uint8_t fiber_no(const unsigned& frame_ID = 0) const {
    return head_(frame_ID)->fiber_no;
  }
  uint8_t slot_no(const unsigned& frame_ID = 0) const {
    return head_(frame_ID)->slot_no;
  }
  uint8_t crate_no(const unsigned& frame_ID = 0) const {
    return head_(frame_ID)->crate_no;
  }
  uint8_t mm(const unsigned& frame_ID = 0) const {
    return head_(frame_ID)->mm;
  }
  uint8_t oos(const unsigned& frame_ID = 0) const {
    return head_(frame_ID)->oos;
  }
  uint16_t wib_errors(const unsigned& frame_ID = 0) const {
    return head_(frame_ID)->wib_errors;
  }
  uint64_t timestamp(const unsigned& frame_ID = 0) const {
    return head_(frame_ID)->timestamp();
  }
  uint16_t wib_counter(const unsigned& frame_ID = 0) const {
    return head_(frame_ID)->wib_counter();
  }

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
  uint8_t hdr(const unsigned& frame_ID, const uint8_t& block_num,
              const uint8_t& hdr_num) const {
    return blockhead_(frame_ID, block_num)->hdr(hdr_num);
  }

  // Functions to return a certain ADC value.
  adc_t get_ADC(const unsigned& frame_ID, const uint8_t block_ID,
                const uint8_t channel_ID) const {
    return channel_(frame_ID, block_ID*64 + channel_ID);
  }
  adc_t get_ADC(const unsigned& frame_ID, const uint8_t channel_ID) const {
    return channel_(frame_ID, channel_ID);
  }

  // Function to return all ADC values for a single channel.
  adc_v get_ADCs_by_channel(const uint8_t block_ID,
                            const uint8_t channel_ID) const {
    adc_v output(total_frames());
    for (size_t i = 0; i < total_frames(); i++) {
      output[i] = get_ADC(i, block_ID, channel_ID);
    }
    return output;
  }
  adc_v get_ADCs_by_channel(const uint8_t channel_ID) const {
    adc_v output(total_frames());
    for (size_t i = 0; i < total_frames(); i++) {
      output[i] = get_ADC(i, channel_ID);
    }
    return output;
  }
  // Function to return all ADC values for all channels in a map.
  std::map<uint8_t, adc_v> get_all_ADCs() const {
    std::map<uint8_t, adc_v> output;
    for (int i = 0; i < 256; i++)
      output.insert(std::pair<uint8_t, adc_v>(i, get_ADCs_by_channel(i)));
    return output;
  }

  // Function to print all timestamps.
  void print_timestamps() const {
    // for (unsigned int i = 0; i < total_frames(); i++) {
    //   std::cout << std::hex << frames_()->timestamp(i) << '\t' << std::dec << i
    //             << std::endl;
    // }
  }

  void print(const unsigned i) const { 
    std::cout << "Frame " << i << " should be printed here.\n"; /* frames_()->print(i); */ }

  void print_frames() const {
    // for (unsigned i = 0; i < total_frames(); i++) {
    //   frames_()->print(i);
    // }
  }

  // The constructor simply sets its const private member "artdaq_Fragment_"
  // to refer to the artdaq::Fragment object
  FelixFragmentReordered(artdaq::Fragment const& fragment)
      : FelixFragmentBase(fragment) {}

  // The number of words in the current event minus the header.
  size_t total_words() const { return sizeBytes_ / sizeof(word_t); }

  // The number of frames in the current event.
  size_t total_frames() const { return meta_.num_frames; }

  // The number of ADC values describing data beyond the header
  size_t total_adc_values() const {
    return total_frames() * FelixFrame::num_ch_per_frame;
  }

 protected:
  // // Allow access to individual frames according to the ReorderedFelixFrames structure.
  // ReorderedFelixFrames const* frames_() const {
  //   return static_cast<dune::ReorderedFelixFrames const*>(artdaq_Fragment_);
  // }

  // Important positions within the data buffer.
  const unsigned int num_frames_ = meta_.num_frames;
  const unsigned int coldata_head_start =
      num_frames_ * sizeof(dune::WIBHeader);
  const unsigned int adc_start =
      coldata_head_start + num_frames_ * 4 * sizeof(dune::ColdataHeader);

  // Reordered frame format overlaid on the data.
  dune::WIBHeader const* head_(const unsigned int frame_num) const {
    return static_cast<dune::WIBHeader const*>(artdaq_Fragment_) + frame_num;
  }

  dune::ColdataHeader const* blockhead_(const unsigned int frame_num, const uint8_t block_num) const {
    return reinterpret_cast<dune::ColdataHeader const*>(
               static_cast<uint8_t const*>(artdaq_Fragment_) +
               coldata_head_start) +
           frame_num * 4 + block_num;
  }

  dune::adc_t channel_(const unsigned int frame_num,
                             const uint8_t ch_num) const {
    return *(reinterpret_cast<dune::adc_t const*>(
               static_cast<uint8_t const*>(artdaq_Fragment_) + adc_start) +
           frame_num + ch_num * num_frames_);
  }
};

//======================
// FELIX fragment class
//======================
class dune::FelixFragment : public FelixFragmentBase {
 public:
  FelixFragment(const artdaq::Fragment& fragment, const bool reordered = 0)
      : FelixFragmentBase(fragment) {
    if (reordered) {
      flxfrag = new FelixFragmentReordered(fragment);
    } else {
      flxfrag = new FelixFragmentUnordered(fragment);
    }
  }

  ~FelixFragment() { delete flxfrag; }

  /* Frame field and accessors. */
  uint8_t sof(const unsigned& frame_ID = 0) const {
    return flxfrag->sof(frame_ID);
  }
  uint8_t version(const unsigned& frame_ID = 0) const {
    return flxfrag->version(frame_ID);
  }
  uint8_t fiber_no(const unsigned& frame_ID = 0) const {
    return flxfrag->fiber_no(frame_ID);
  }
  uint8_t slot_no(const unsigned& frame_ID = 0) const {
    return flxfrag->slot_no(frame_ID);
  }
  uint8_t crate_no(const unsigned& frame_ID = 0) const {
    return flxfrag->crate_no(frame_ID);
  }
  uint8_t mm(const unsigned& frame_ID = 0) const {
    return flxfrag->mm(frame_ID);
  }
  uint8_t oos(const unsigned& frame_ID = 0) const {
    return flxfrag->oos(frame_ID);
  }
  uint16_t wib_errors(const unsigned& frame_ID = 0) const {
    return flxfrag->wib_errors(frame_ID);
  }
  uint64_t timestamp(const unsigned& frame_ID = 0) const {
    return flxfrag->timestamp(frame_ID);
  }
  uint16_t wib_counter(const unsigned& frame_ID = 0) const {
    return flxfrag->wib_counter(frame_ID);
  }

  /* Coldata block accessors. */
  uint8_t s1_error(const unsigned& frame_ID, const uint8_t& block_num) const {
    return flxfrag->s1_error(frame_ID, block_num);
  }
  uint8_t s2_error(const unsigned& frame_ID, const uint8_t& block_num) const {
    return flxfrag->s2_error(frame_ID, block_num);
  }
  uint16_t checksum_a(const unsigned& frame_ID,
                      const uint8_t& block_num) const {
    return flxfrag->checksum_a(frame_ID, block_num);
  }
  uint16_t checksum_b(const unsigned& frame_ID,
                      const uint8_t& block_num) const {
    return flxfrag->checksum_b(frame_ID, block_num);
  }
  uint16_t coldata_convert_count(const unsigned& frame_ID,
                                 const uint8_t& block_num) const {
    return flxfrag->coldata_convert_count(frame_ID, block_num);
  }
  uint16_t error_register(const unsigned& frame_ID,
                          const uint8_t& block_num) const {
    return flxfrag->error_register(frame_ID, block_num);
  }
  uint8_t hdr(const unsigned& frame_ID, const uint8_t& block_num,
              const uint8_t& hdr_num) const {
    return flxfrag->hdr(frame_ID, block_num, hdr_num);
  }

  // Functions to return a certain ADC value.
  adc_t get_ADC(const unsigned& frame_ID, const uint8_t block_ID,
                const uint8_t channel_ID) const {
    return flxfrag->get_ADC(frame_ID, block_ID, channel_ID);
  }
  adc_t get_ADC(const unsigned& frame_ID, const uint8_t channel_ID) const {
    return flxfrag->get_ADC(frame_ID, channel_ID);
  }

  // Function to return all ADC values for a single channel.
  adc_v get_ADCs_by_channel(const uint8_t block_ID,
                            const uint8_t channel_ID) const {
    return flxfrag->get_ADCs_by_channel(block_ID, channel_ID);
  }
  adc_v get_ADCs_by_channel(const uint8_t channel_ID) const {
    return flxfrag->get_ADCs_by_channel(channel_ID);
  }
  // Function to return all ADC values for all channels in a map.
  std::map<uint8_t, adc_v> get_all_ADCs() const {
    return flxfrag->get_all_ADCs();
  }

  // Function to print all timestamps.
  void print_timestamps() const { return flxfrag->print_timestamps(); }

  void print(const unsigned i) const { return flxfrag->print(i); }

  void print_frames() const { return flxfrag->print_frames(); }

  // The number of words in the current event minus the header.
  size_t total_words() const { return flxfrag->total_words(); }
  // The number of frames in the current event.
  size_t total_frames() const { return flxfrag->total_frames(); }
  // The number of ADC values describing data beyond the header
  size_t total_adc_values() const { return flxfrag->total_adc_values(); }

 private:
  const FelixFragmentBase* flxfrag;
};

#endif /* artdaq_dune_Overlays_FelixFragment_hh */
