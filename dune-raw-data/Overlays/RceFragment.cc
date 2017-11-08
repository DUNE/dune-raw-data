#include "dune-raw-data/Overlays/RceFragment.hh"

#include "dam/HeaderFragmentUnpack.hh"
#include "dam/TpcStreamUnpack.hh"

#include <iostream>
#include <fstream>

dune::RceFragment::RceFragment(const uint64_t* data_ptr) : _data_ptr(data_ptr)
{
    _init();
}

dune::RceFragment::RceFragment(artdaq::Fragment const& afrag) 
{
    _data_ptr = (uint64_t const*)(afrag.dataBeginBytes() + 12);
    _init();

    auto* buf = _data_ptr;
    std::cout << "RCE Fragment ";
    for (int i = 0; i < 5; ++i)
        std::cout << std::hex << *(buf + i) << ' ';
    std::cout << std::dec << std::endl;
}

void dune::RceFragment::_init()
{
    HeaderFragmentUnpack const header (_data_ptr);
    if ( header.isData() )
    {
        _data_fragment = std::make_unique<DataFragmentUnpack>(_data_ptr);
        if (_data_fragment->isTpcNormal())
        {
            _tpc_fragment = std::make_unique<TpcFragmentUnpack>(*_data_fragment);
            _n_streams = _tpc_fragment->getNStreams();
        }
    }
    else
    {
        // LogWarning
    }
}

TpcStreamUnpack const *
dune::RceFragment::get_stream(int i) const
{
    if (_n_streams <= 0)
        return nullptr;
    return _tpc_fragment->getStream(i);
}

void dune::RceFragment::hexdump(std::ostream& out, int n_words) const
{
    out << std::hex;
    for (int i = 0; i < n_words; ++i)
        out << *(_data_ptr + i) << " ";
    out << std::dec << std::endl;
}
