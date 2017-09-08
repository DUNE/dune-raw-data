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

// TODO: Another constructor to read binary frames from a file.
//demo::FelixFragment::FelixFragment(std::string filename) {
//    std::cout << "Attempting to create a FELIX Fragment from file.\n";
//    
//    // Open and check the file.
//    std::ifstream ifile(filename, std::ifstream::ate | std::ifstream::binary);
//    if(!ifile) {
//        std::cout << "Error (FelixFragment): file " << filename <<
//        " could not be opened.\n"
//        exit 1;
//    }
//    
//    // Prepare the internal frame buffer and fill it.
//    int num_bytes = ifile.tellg();
//    artdaq_Fragment_.resize(num_bytes/4);
//    
//    ifile.seekg(0, ifile.beg);
//    for(size_t i=0; i<num_bytes; i++)
//      *(artdaq_Fragment_.dataAdress()) = (uint32_t)ifile.get() | ifile.get()<<8 | ifile.get()<<16 | ifile.get()<<24;
//    
//    // Update header information.
//    header_()->event_size = num_bytes/4+2; // Two words for the header.
//    
//    ifile.close();
//}

// Function to return all ADC values for a single channel [0:255].
dune::FelixFragment::adc_v dune::FelixFragment::get_ADCs_by_channel(uint8_t channel_ID) const {
  adc_v output(total_frames());
  for(size_t i=0; i<total_frames(); i++) {
    // The structure of a WIB->FELIX frame is nontrivial. See the link below
    // for documents detailing the division of frames into blocks, streams
    // and channels.
    // https://github.com/FELIXDAQ/FrameGen/tree/master/docs
    uint8_t block_num = channel_ID/4; // [0:3]
    uint8_t stream_num = (channel_ID%block_num)/8; // [0:7]
    uint8_t channel_num = (channel_ID%block_num)%8; // [0:7]
    
    uint32_t word_num = 117*i + 4 + 28*block_num + 3*stream_num;
    const artdaq::Fragment::const_iterator begin = artdaq_Fragment_.dataBegin();
    // Some channel values are broken up between words.
    switch(channel_num) {
      case 2:
        output[i] = getBitRange(*(begin+word_num),24,31) | getBitRange(*(begin+word_num+1),0,3)<<8;
        break;
      case 5:
        output[i] = getBitRange(*(begin+word_num+1),28,31) | getBitRange(*(begin+word_num+2),0,7)<<4;
        break;
      default:
        output[i] = getBitRange(*(begin+word_num+channel_num*12/32),(channel_num*12)%32,((channel_num+1)*12-1)%32);
        break;
    }
  }
  return output;
}

// Function to return all ADC values for all channels in a map.
std::map<uint8_t, dune::FelixFragment::adc_v> dune::FelixFragment::get_all_ADCs() const {
  std::map<uint8_t, adc_v> output;
  for(int i=0; i<256; i++)
    output.insert(std::pair<uint8_t, adc_v>(i, get_ADCs_by_channel(i)));
  return output;
}

std::ostream & dune::operator << (std::ostream & os, FelixFragment const & /*f*/) {
  os << "FelixFragment streaming to ostream is only partiallyy supported.\n";
  /*os << "FelixFragment event size: "
     << f.hdr_event_size()
     << ", trigger number: "
     << f.hdr_trigger_number()
     << "\n";
  */
  
  return os;
}
