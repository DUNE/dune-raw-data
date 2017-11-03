#ifndef artdaq_dune_Overlays_RceFragment_hh
#define artdaq_dune_Overlays_RceFragment_hh

#include <memory>
#include <string>
#include "dam/DataFragmentUnpack.hh"
#include "dam/TpcFragmentUnpack.hh"
#include "artdaq-core/Data/Fragment.hh"

namespace dune
{
    class RceFragment;
}

class TpcStreamUnpack;

class dune::RceFragment
{
    public:
        RceFragment(artdaq::Fragment const & fragment );
        int size() const { return _n_streams; }
        TpcStreamUnpack const * get_stream(int i) const;
        void dump(std::string const& outfilename) const;

    private:
        artdaq::Fragment const & _artdaq_fragment;
        std::unique_ptr<DataFragmentUnpack> _data_fragment;
        std::unique_ptr<TpcFragmentUnpack> _tpc_fragment;
        int _n_streams = 0;
};
#endif 
