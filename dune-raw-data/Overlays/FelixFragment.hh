// FelixFragment.hh edited by Milo Vermeulen and Roland Sipos (2017) to accept WIB frames.
// FelixFragment.hh was in turn edited from ToyFragment.hh.

#ifndef artdaq_dune_Overlays_FelixFragment_hh
#define artdaq_dune_Overlays_FelixFragment_hh

#include "artdaq-core/Data/Fragment.hh"

#include <iostream>
#include <vector>
#include <string>
#include <map>


// Implementation of "FelixFragment", an artdaq::FelixFragment overlay
// class used for WIB->FELIX frames.
//
// The intention of this class is to provide an Overlay for a 12-bit ADC
// module.

namespace dune {
    class FelixFragment;
    
    // Let the "<<" operator dump the FelixFragment's data to stdout
    std::ostream & operator << (std::ostream &, FelixFragment const &);
    
    // Bit access function (from FrameGen).
    uint32_t getBitRange(const uint32_t& word, int begin, int end) {
        if(begin==0 && end==31)
            return word;
        else
            return (word>>begin)&((1<<(end-begin+1))-1);
    }
}

class dune::FelixFragment {
public:
    
    // Define a public typedef for marking Word size.
    typedef  uint32_t word_t;
    
    // Struct to hold FelixFrgament Metadata (unchanged).
    struct Metadata {
        
        typedef uint32_t data_t;
        
        data_t board_serial_number : 16;
        data_t num_adc_bits : 8;
        data_t unused : 8;
        
        static size_t const size_words = 1ul; // Units of Metadata::data_t
    };
    
    static_assert (sizeof (Metadata) == Metadata::size_words * sizeof (Metadata::data_t), "FelixFragment::Metadata size changed");
    
    // The FelixFragment::Header takes data from the hardware per event.
    // It is not to be confused with the individual frame headers, which
    // are part of the raw data. (unchanged)
    
    struct Header {
        typedef uint32_t data_t; // Not to be confused with Metadata::data_t.
        
        typedef uint32_t event_size_t;
        typedef uint32_t trigger_number_t;
        
        
        event_size_t event_size : 28;
        event_size_t unused_1   :  4;
        
        trigger_number_t trigger_number : 32;
        
        static size_t const size_words = 2ul; // Units of Header::data_t
    };
    
    static_assert (sizeof (Header) == Header::size_words * sizeof (Header::data_t), "FelixFragment::Header size changed");
    
    // The constructor simply sets its const private member "artdaq_Fragment_"
    // to refer to the artdaq::Fragment object
    FelixFragment(artdaq::Fragment const & fragment ) : artdaq_Fragment_(fragment) {
        std::cout << "Attempting to create a FELIX Fragment.\n";
    }
    
    // TODO: Another constructor to read binary frames from a file.
    //FelixFragment(std::string filename);
    
    // const getter functions for the data in the header
    Header::event_size_t hdr_event_size() const { return header_()->event_size; }
    Header::trigger_number_t hdr_trigger_number() const { return header_()->trigger_number; }
    static constexpr size_t hdr_size_words() { return Header::size_words; }
    
    
    // The goal is to have the following methods:
    // Giovanna: Basically, I would like a user to be exposed to a method
    // fragment::get_ADCs_by_channel(channel ID) : vector<ADC>
    // and maybe a second method fragment::get_all_ADCs() : map<channelID, vector<ADCs>> .
    // Eventually there may be ancillary methods like get_channelIDs(), get_time_frame(), …
    
    // FelixFragment is intended to represent/interpret data that has an
    // inherent size of 12 bits (unsigned). This is represented by the
    // adc_t type that is declared here.
    typedef uint16_t adc_t;
    typedef std::vector<adc_t> adc_v;
    
    // Function to return all ADC values for a single channel.
    adc_v get_ADCs_by_channel(uint8_t channel_ID) const;
    // Function to return all ADC values for all channels in a map.
    std::map<uint8_t, adc_v> get_all_ADCs() const;
    
    // The number of words in the current event minus the header.
    size_t total_words() const {
        return (hdr_event_size() - hdr_size_words());
    }
    
    // The number of frames in the current event.
    size_t total_frames() const {
        return total_words() / 468;
    }
    
    // The number of ADC values describing data beyond the header
    size_t total_adc_values() const {
        return total_frames() * 256;
    }
    
    // Start of the stored frames, returned as a pointer to the first word.
    Header::data_t const * dataBeginFrames() const {
        return reinterpret_cast<Header::data_t const *>(header_() + hdr_size_words());
    }
    
    // End of the stored frames, returned as a pointer to the first word.
    Header::data_t const * dataEndFramess() const {
        return dataBeginFrames() + total_words();
    }
    
    // Functions like findBadADCs to check whether any ADC values are corrupt
    // are to be added.
    
    // Largest ADC value possible
    size_t adc_range(int daq_adc_bits = 12) {
        return (1ul << daq_adc_bits );
    }
    
protected:
    
    // Functions to translate between size (in bytes) of an ADC, size of
    // this fragment overlay's concept of a unit of data (i.e.,
    // Header::data_t) and size of an artdaq::Fragment's concept of a
    // unit of data (the artdaq::Fragment::value_type).
    
    static constexpr size_t adcs_per_frame_() {
        return 256;
    }
    
    // header_() simply takes the address of the start of this overlay's
    // data (i.e., where the FelixFragment::Header object begins) and
    // casts it as a pointer to FelixFragment::Header
    
    Header const * header_() const {
        return reinterpret_cast<FelixFragment::Header const *>(artdaq_Fragment_.dataBeginBytes());
    }
    
private:
    
    artdaq::Fragment const & artdaq_Fragment_;
};

#endif /* artdaq_demo_Overlays_FelixFragment_hh */
