#include "lbne-raw-data/Overlays/TpcMicroSlice.hh"
#include <cmath>
#include <iostream>

lbne::TpcMicroSlice::TpcMicroSlice(uint8_t* address) : buffer_(address)
{
}

// Returns the size of the TpcMicroSlice
lbne::TpcMicroSlice::Header::microslice_size_t lbne::TpcMicroSlice::size() const
{
  // Temporary modification for interim uslice format - size is in word 0 in bytes  
  return (header_()->raw_header_data[0]);
}

// Returns the sequence ID of the TpcMicroSlice
lbne::TpcMicroSlice::Header::sequence_id_t lbne::TpcMicroSlice::sequence_id() const
{
  // Sequence ID is word 1 of header
  return reinterpret_cast<lbne::TpcMicroSlice::Header::sequence_id_t>(header_()->raw_header_data[1]);
}

// Returns the type ID of the TpcMicroSlice
lbne::TpcMicroSlice::Header::type_id_t lbne::TpcMicroSlice::type_id() const
{
  // type ID is word 2 of header
  return reinterpret_cast<lbne::TpcMicroSlice::Header::type_id_t>(header_()->raw_header_data[2]);
}

// Returns the software message of the TpcMicroSlice
lbne::TpcMicroSlice::Header::softmsg_t lbne::TpcMicroSlice::software_message() const
{
  // software message is words 3 & 4  of header
  return reinterpret_cast<lbne::TpcMicroSlice::Header::softmsg_t>(((uint64_t)header_()->raw_header_data[3])<<32|header_()->raw_header_data[4]);
}

// Returns the firmware message of the TpcMicroSlice
lbne::TpcMicroSlice::Header::firmmsg_t lbne::TpcMicroSlice::firmware_message() const
{
  // firmware message is words 5 & 6  of header
  return reinterpret_cast<lbne::TpcMicroSlice::Header::firmmsg_t>(((uint64_t)header_()->raw_header_data[5])<<32|header_()->raw_header_data[6]);
}


// Returns the number of nanoslices in the microslice
lbne::TpcMicroSlice::Header::nanoslice_count_t lbne::TpcMicroSlice::nanoSliceCount() const
{
	// Temporary hack - retrieve nanoslice size from first nanoslice, then infer
	// nanoslice count based on size of microslice. This is potentially brittle, and
	// should be encoded in the microslice header instead

	std::unique_ptr<TpcNanoSlice> nanoslice;
	uint8_t* ns_ptr = data_(0);
	nanoslice.reset(new TpcNanoSlice(ns_ptr));

	std::size_t microsliceDataSize = this->size() - sizeof(Header);
	return static_cast<lbne::TpcMicroSlice::Header::nanoslice_count_t>
		(microsliceDataSize / nanoslice->size());
}

// Returns the requested NanoSlice if the requested slice was found,
// otherwise returns an empty pointer
std::unique_ptr<lbne::TpcNanoSlice> lbne::TpcMicroSlice::nanoSlice(uint32_t index) const
{
	std::unique_ptr<TpcNanoSlice> nslice_ptr;
	if (index < nanoSliceCount()) {
		uint8_t* ns_ptr = data_(index);
		nslice_ptr.reset(new TpcNanoSlice(ns_ptr));
	}
	return nslice_ptr;
}

// Puts the ADC for nanoslice[index] and sample[sample] in memory [value]
// Returns success if the nanoslice is within the count and the access succeeds

bool lbne::TpcMicroSlice::nanosliceSampleValue(uint32_t index, uint32_t sample, uint16_t& value) const
{
  if (index < nanoSliceCount()) {
    uint8_t *ns_ptr = data_(index);
    TpcNanoSlice nslice(ns_ptr);
    return nslice.sampleValue(sample,value);
  }
  return false;
}

lbne::TpcNanoSlice::Header::nova_timestamp_t lbne::TpcMicroSlice::nanosliceNova_timestamp(uint32_t index) const
{
  if (index < nanoSliceCount()) {
    uint8_t *ns_ptr = data_(index);
    TpcNanoSlice nslice(ns_ptr);
    return nslice.nova_timestamp();
  }
  return false;
}

// Returns a pointer to the header
lbne::TpcMicroSlice::Header const* lbne::TpcMicroSlice::header_() const
{
  return reinterpret_cast<Header const *>(buffer_);
}

// Returns a pointer to the requested NanoSlice
uint8_t* lbne::TpcMicroSlice::data_(uint32_t index) const
{
  uint8_t* ns_ptr = reinterpret_cast<uint8_t *>(buffer_ + sizeof(Header));
  TpcNanoSlice tmp_ns(ns_ptr);
  return ns_ptr + (index * tmp_ns.size());
}
