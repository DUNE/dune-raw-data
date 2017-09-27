#include "dune-raw-data/Overlays/FelixFragment.hh"
#include <vector>
#include <map>
#include <bitset>
#include <stdint.h>
#include <iostream>
#include <memory>


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"


#define BOOST_TEST_MODULE(MilliSlice_t)
#ifdef HAVE_CANVAS
 #include "cetlib/quiet_unit_test.hpp"
#else
#include "boost/test/auto_unit_test.hpp"
#endif


BOOST_AUTO_TEST_SUITE(FelixFragment_test)

BOOST_AUTO_TEST_CASE(BaselineTest)
{

  std::cout << "### WOOF -> Testing fromFile creation...\n"; 
  artdaq::FragmentPtr frag = dune::FelixFragment::fromFile("/home/rolandsipos/Projects/frames/WIB-dump-472bytes.dat");
  dune::FelixFragment flxfrg(*frag);
  std::cout << "\n\n";


  std::cout << "### WOOF -> Test for the presence of a single frame...\n";
  const uint8_t words = 118;
  const size_t frames = 1;
  const size_t adc_values = 256;

  std::cout << "  -> Total words: " << flxfrg.total_words() << '\n';
  std::cout << "  -> Total frames: " << flxfrg.total_frames() << '\n';
  std::cout << "  -> Total adc values: " << flxfrg.total_adc_values() << '\n';

  BOOST_REQUIRE_EQUAL(flxfrg.total_words(), words);
  BOOST_REQUIRE_EQUAL(flxfrg.total_frames(), frames);
  BOOST_REQUIRE_EQUAL(flxfrg.total_adc_values(), adc_values);
  std::cout << "\n\n";

 
  std::cout << "### WOOF -> WIB frame test...\n";
  const size_t frame_size = 468;
  BOOST_REQUIRE_EQUAL(sizeof(dune::FelixFragment::WIBFrame), frame_size);
  std::cout << " -> SOF: "  << unsigned(flxfrg.sof(0)) << "\n";
  std::cout << " -> Version: " << unsigned(flxfrg.version(0)) << "\n";
  std::cout << " -> FiberNo: " << unsigned(flxfrg.fiber_no(0)) << "\n";
  std::cout << " -> SlotNo: "  << unsigned(flxfrg.slot_no(0)) << "\n";
  std::cout << " -> CrateNo: " << unsigned(flxfrg.crate_no(0)) << "\n";
  std::cout << " -> Timestamp: " << std::hex << flxfrg.timestamp(0) << std::dec << "\n";
  // Check, if we could recover the following bytes: ce e2 f7 22 1b
  BOOST_REQUIRE_EQUAL(flxfrg.timestamp(0), 888571109915);
  std::cout << "\n\n";


  std::cout << "### WOOF -> Dumping frames...\n";
  flxfrg.print_frames();
  std::cout << "\n\n";


  std::cout << "### WOOF WOOF -> Done...\n";
}

#if 0
BOOST_AUTO_TEST_CASE(TinyBufferTest)
{

  // *** Test a buffer that is too small for even the header

}

BOOST_AUTO_TEST_CASE(SmallBufferTest)
{

  // *** Test a buffer that is too small for two microslices

}

BOOST_AUTO_TEST_CASE(CopyTest)
{
}

BOOST_AUTO_TEST_CASE(BufferReuseTest)
{
}
#endif

BOOST_AUTO_TEST_SUITE_END()

#pragma GCC diagnostic pop
