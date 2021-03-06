#ifndef dune_artdaq_Overlays_TimingFragment_hh
#define dune_artdaq_Overlays_TimingFragment_hh

#include "artdaq-core/Data/Fragment.hh"
//#include "artdaq-core/Data/features.hh"
#include "cetlib/exception.h"

#include <ostream>
#include <vector>

// Implementation of "TimingFragment".
//
// From Dave Newbold mail May 2017
// The data words (uint32_t) given by the hardware are
//    d(0) <= X"aa000600";          -- DAQ word 0 (cookie)
//    d(1) <= X"0000000" & scmd;    -- DAQ word 1 (scmd = trigger type, tcmd = the reserved zero bits)
//    d(2) <= tstamp(31 downto 0);  -- DAQ word 2 (tstampl = low bits of timestamp)
//    d(3) <= tstamp(63 downto 32); -- DAQ word 3 (tstamph = high bits of time stamp)
//    d(4) <= evtctr;               -- DAQ word 4 (evtctr = event counter)
//    d(5) <= X"000000000";         -- Dummy checksum, not implemented yet (cksum)
//
// These are stored in the fragment exactly as received and are manipulated
// to be readable by the getters in this class.
//
// The definition of the timing commands (scmd) in the DAQ word 1 are
//  0: spill_start
//  1: spill_end
//  2: calib
//  3: trigger
//  4: time_sync

namespace dune {
  class TimingFragment;

  // Let the "<<" operator dump the TimingFragment's data to stdout
  std::ostream & operator << (std::ostream &, TimingFragment const &);
}

class dune::TimingFragment {
  // There is no metadata in the timing block

  // Constructor.  This class keeps a copy of a pointer to the fragment.
  // Basically that is how it works.
  public: 
  TimingFragment(artdaq::Fragment const & f ) : artdaq_Fragment_(f) {}

  // The following structure is overlayed onto the data in the fragment, starting
  // at the beginning.
  struct Body {
    uint32_t cookie  : 32;
    uint32_t scmd    :  4;
    uint32_t tcmd    : 28;
    uint32_t tstampl : 32;
    uint32_t tstamph : 32;
    uint32_t evtctr  : 32;
    uint32_t cksum   : 32;

    static size_t const size = 6ul;   // Units of header uint32_t
  };

  // Here are the getters
  uint32_t get_cookie() const  { return body_()->cookie;  }
  uint32_t get_scmd() const    { return body_()->scmd;    }
  uint32_t get_tcmd() const    { return body_()->tcmd;    }
  uint32_t get_tstampl() const { return body_()->tstampl; }
  uint32_t get_tstamph() const { return body_()->tstamph; }
  uint32_t get_evtctr() const  { return body_()->evtctr;  }
  uint32_t get_cksum() const   { return body_()->cksum;   }

  uint64_t get_tstamp() const { 
     uint64_t l = body_()->tstampl;
     uint64_t h = body_()->tstamph;
     return (l | (h<<32));
  } 

  static size_t size() { return Body::size; /* body_()->size; */}    
         // This returns the constant size of the block (cureently 6 uint32_ts)

protected:
  // This function allows the getter classes below to be simple.  
  // It is modelled after header_() in ToyFragment.hh
  // header_() in ToyFragment.hh is protected as well.
  Body const * body_() const {
    return reinterpret_cast<TimingFragment::Body const *>( artdaq_Fragment_.dataBeginBytes());
  }

  // This asks the compiler to check if we change te format that we didn't forget to cheange the size as well
  // However if we do change the format, we must figure out how to make this class backward compatible.
  static_assert (sizeof (Body) == Body::size * sizeof (uint32_t), "TimingFragment::Data block size changed");

private:
  artdaq::Fragment const & artdaq_Fragment_;
};

#endif /* dune_artdaq_Overlays_TimingFragment_hh */
