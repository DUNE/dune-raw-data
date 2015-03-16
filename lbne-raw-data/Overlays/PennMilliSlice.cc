#include "lbne-raw-data/Overlays/PennMilliSlice.hh"
#include <iostream>
#include <bitset>

//#define __DEBUG_payload__

lbne::PennMilliSlice::PennMilliSlice(uint8_t* address) : buffer_(address)
{
}

lbne::PennMilliSlice::Header::millislice_size_t lbne::PennMilliSlice::size() const
{
  return header_()->millislice_size;
}

#ifdef PENN_DONT_REBLOCK_USLICES
lbne::PennMilliSlice::Header::microslice_count_t lbne::PennMilliSlice::microSliceCount() const
{
  return header_()->microslice_count;
}
#endif

lbne::PennMilliSlice::Header::sequence_id_t lbne::PennMilliSlice::sequenceID() const
{
  return header_()->sequence_id;
}

lbne::PennMilliSlice::Header::version_t lbne::PennMilliSlice::version() const
{
  return header_()->version;
}

lbne::PennMilliSlice::Header::timestamp_t lbne::PennMilliSlice::endTimestamp() const
{
  return header_()->end_timestamp;
}

lbne::PennMilliSlice::Header::ticks_t lbne::PennMilliSlice::widthTicks() const
{
  return header_()->width_in_ticks;
}

lbne::PennMilliSlice::Header::ticks_t lbne::PennMilliSlice::overlapTicks() const
{
  return header_()->overlap_in_ticks;
}

lbne::PennMilliSlice::Header::payload_count_t lbne::PennMilliSlice::payloadCount() const
{
  return header_()->payload_count;
}

lbne::PennMilliSlice::Header::payload_count_t lbne::PennMilliSlice::payloadCount(
										 lbne::PennMilliSlice::Header::payload_count_t& counter, 
										 lbne::PennMilliSlice::Header::payload_count_t& trigger,
										 lbne::PennMilliSlice::Header::payload_count_t& timestamp) const
{
  counter   = header_()->payload_count_counter;
  trigger   = header_()->payload_count_trigger;
  timestamp = header_()->payload_count_timestamp;
  return payloadCount();
}

#ifdef PENN_DONT_REBLOCK_USLICES
// Returns the requested MicroSlice if the requested slice was found,
// otherwise returns an empty pointer
std::unique_ptr<lbne::PennMicroSlice> lbne::PennMilliSlice::microSlice(uint32_t index) const
{
  std::unique_ptr<PennMicroSlice> mslice_ptr;
  if (index < microSliceCount()) {
    uint8_t* ms_ptr = data_(index);
    mslice_ptr.reset(new PennMicroSlice(ms_ptr));
  }
  return mslice_ptr;
}
#endif

//Returns the requested payload
uint8_t* lbne::PennMilliSlice::payload(uint32_t index, 
				       lbne::PennMicroSlice::Payload_Header::data_packet_type_t& data_packet_type,
				       lbne::PennMicroSlice::Payload_Header::short_nova_timestamp_t& short_nova_timestamp,
				       size_t& payload_size) const
{
  uint32_t i = 0;
  uint8_t* pl_ptr = buffer_ + sizeof(Header);
  while(pl_ptr < (buffer_ + size())) {
    lbne::PennMicroSlice::Payload_Header* payload_header = reinterpret_cast<lbne::PennMicroSlice::Payload_Header*>(pl_ptr);
    lbne::PennMicroSlice::Payload_Header::data_packet_type_t type = payload_header->data_packet_type;
    if(i == index) {
      data_packet_type = type;
      short_nova_timestamp = payload_header->short_nova_timestamp;
#ifdef __DEBUG_payload__
    std::cout << "PennMilliSlice::payload() payload  " << i << " has type 0x"
	      << std::hex << (unsigned int)data_packet_type << std::dec
	      << " " << std::bitset<8>(data_packet_type)
	      << " and timestamp " << short_nova_timestamp
	      << " " << std::bitset<32>(short_nova_timestamp)
	      << " payload header bits " << std::bitset<32>(*((uint32_t*)pl_ptr))
	      << std::endl;
#endif
      pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words;
      switch(type)
	{
	case lbne::PennMicroSlice::DataTypeCounter:
	  payload_size = lbne::PennMicroSlice::payload_size_counter;
	  break;
	case lbne::PennMicroSlice::DataTypeTrigger:
	  payload_size = lbne::PennMicroSlice::payload_size_trigger;
	  break;
	case lbne::PennMicroSlice::DataTypeTimestamp:
	  payload_size = lbne::PennMicroSlice::payload_size_timestamp;
	  break;
	case lbne::PennMicroSlice::DataTypeSelftest:
	  payload_size = lbne::PennMicroSlice::payload_size_selftest;
	  break;
	case lbne::PennMicroSlice::DataTypeChecksum:
	  payload_size = lbne::PennMicroSlice::payload_size_checksum;
	  break;
	default:
	  std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type << std::endl;
	  payload_size = 0;
	  break;
	}//switch(type)
      return pl_ptr;
    }
    switch(type)
      {
      case lbne::PennMicroSlice::DataTypeCounter:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_counter;
	break;
      case lbne::PennMicroSlice::DataTypeTrigger:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_trigger;
	break;
      case lbne::PennMicroSlice::DataTypeTimestamp:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_timestamp;
	break;
      case lbne::PennMicroSlice::DataTypeSelftest:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_selftest;
	break;
      case lbne::PennMicroSlice::DataTypeChecksum:
	pl_ptr += lbne::PennMicroSlice::Payload_Header::size_words + lbne::PennMicroSlice::payload_size_checksum;
	break;
      default:
	std::cerr << "Unknown data packet type found 0x" << std::hex << (unsigned int)type << std::endl;
	return nullptr;
	break;
      }//switch(type)
    i++;
  }
  std::cerr << "Could not find payload with ID " << index << " (the data buffer has overrun)" << std::endl;
  payload_size = 0;
  return nullptr;
}

uint32_t lbne::PennMilliSlice::calculateChecksum() const
{
  try {
    boost::crc_32_type checksum;
    checksum.process_bytes(buffer_, this->size());
    return checksum.checksum();
  }
  catch ( ... ) {
    std::cout << "Error caught in PennMilliSlice::calculateChecksum()" << std::endl;
    //TODO handle error cleanly here
    return 0;
  }
}

lbne::PennMilliSlice::Header const* lbne::PennMilliSlice::header_() const
{
  return reinterpret_cast<Header const*>(buffer_);
}

// returns a pointer to the requested MicroSlice
uint8_t* lbne::PennMilliSlice::data_(int index) const
{
  uint8_t* ms_ptr = buffer_ + sizeof(Header);
  for (int idx = 0; idx < index; ++idx) {
    PennMicroSlice tmp_ms(ms_ptr);
    ms_ptr += tmp_ms.size();
  }
  return ms_ptr;
}
