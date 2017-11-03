#include "dune-raw-data/Overlays/RceFragment.hh"

#include "dam/HeaderFragmentUnpack.hh"
#include "dam/TpcStreamUnpack.hh"

#include <iostream>
#include <fstream>

dune::RceFragment::RceFragment(artdaq::Fragment const & fragment ) :
    _artdaq_fragment(fragment)
{
    uint64_t const *buf = reinterpret_cast<uint64_t const*>(
            _artdaq_fragment.dataBeginBytes() + 12);

    std::cout << "RCE Fragment ";
    for (int i = 0; i < 5; ++i)
        std::cout << std::hex << *(buf + i) << ' ';
    std::cout << std::dec << std::endl;

    HeaderFragmentUnpack const header (buf);
    if ( header.isData() )
    {
        _data_fragment = std::make_unique<DataFragmentUnpack>(buf);
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

void dune::RceFragment::dump(std::string const& outfilename) const
{
    std::ofstream b_stream(outfilename.c_str(),
            std::fstream::out | std::fstream::binary);

    size_t bytes = _artdaq_fragment.dataSizeBytes() - 12;
    char const* data = reinterpret_cast<decltype(data)>(
            _artdaq_fragment.dataBeginBytes() + 12);

    b_stream.write(data, bytes);
    b_stream.close();
}
