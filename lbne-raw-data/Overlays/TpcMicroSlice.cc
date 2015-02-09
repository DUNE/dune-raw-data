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
  // Temporary modification for interim uslice format - size is in word 0 in bytes
  // Sequence ID is word 1 of header
  return reinterpret_cast<lbne::TpcMicroSlice::Header::sequence_id_t>(header_()->raw_header_data[1]);
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

// Returns a pointer to the header
lbne::TpcMicroSlice::Header const* lbne::TpcMicroSlice::header_() const
{
  return reinterpret_cast<Header const *>(buffer_);
}

// Returns a pointer to the requested NanoSlice
uint8_t* lbne::TpcMicroSlice::data_(uint32_t index) const
{
  uint8_t* ns_ptr = reinterpret_cast<uint8_t *>(buffer_ + sizeof(Header));
  for (uint32_t idx = 0; idx < index; ++idx) {
    TpcNanoSlice tmp_ns(ns_ptr);
    ns_ptr += tmp_ns.size();
  }
  return ns_ptr;
}
