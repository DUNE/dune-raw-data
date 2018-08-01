#include "dune-raw-data/Overlays/ToyFragment.hh"

#include "cetlib/exception.h"

namespace dune {

  CTBFragment( artdaq::Fragment const & f ) : 

    _n_words( f.dataSizeBytes()/CTBFragment::WordSize() ),
    artdq_Fragment_( f ) 
  { ; } 


  //--------------------------------
  // word getters
  //--------------------------------
  
  const ptb::content::word::word_t* CTBFragment::Word( unsigned int i ) const {
    
    if ( i >= NWords() ) return nullptr ;

    return reinterpret_cast<const ptb::content::word::word_t*>( artdaq_Fragment_.dataBegin() + i * CTBFragment::WordSize() )  ;
    
  }


  const ptb::content::word::feedback_t* CTBFragment::Feedback( unsigned int i ) const {

    const ptb::content::word::word_t* w = Word( i )  ;
    
    if ( ! w )  return nullptr ;

    return CTBFragment::Feedback( *w ) ;
    
  }


  const ptb::content::word::ch_status_t* ChStatus( unsigned int i ) const {

    const ptb::content::word::word_t* w = Word( i )  ;

    if ( ! w )  return nullptr ;

    return CTBFragment::ChStatus( *w ) ;

  }

 
  const ptb::content::word::timestamp_t* Timestamp( unsigned int i ) const {

    const ptb::content::word::word_t* w = Word( i )  ;

    if ( ! w )  return nullptr ;

    return CTBFragment::Timestamp( *w ) ;

  }


  const ptb::content::word::trigger_t * Trigger( unsigned int i )  const {

    const ptb::content::word::word_t* w = Word( i )  ;

    if ( ! w )  return nullptr ;

    return CTBFragment::Trigger( *w ) ;

  }

 
  
  // casting methods

  const ptb::content::word::feedback_t * Feedback ( const ptb::content::word::word_t & w ) {

    if ( w.word_type != ptb::content::word::t_fback ) return nullptr ;

    return reinterpret_cast<const ptb::content::word::feedback_t*>( & w ) ; 

  }

  const ptb::content::word::ch_status_t* ChStatus ( const ptb::content::word::word_t & w ) {
    
    if ( w.word_type != ptb::content::word::t_ch ) return nullptr ;

    return reinterpret_cast<const ptb::content::word::ch_status_t*>( & w ) ;

  }

  const ptb::content::word::timestamp_t* Timestamp( const ptb::content::word::word_t & w ) {
  
    if ( w.word_type != ptb::content::word::t_ts ) return nullptr ;

    return reinterpret_cast<const ptb::content::word::timestamp_t*>( & w ) ;

  }

  const ptb::content::word::trigger_t*   Trigger  ( const ptb::content::word::word_t & w ) {

    if ( w.word_type != ptb::content::word::t_lt && w.word_type != ptb::content::word::t_gt ) return nullptr ;

    return reinterpret_cast<const ptb::content::word::trigger_t*>( & w ) ;

  }

}  // namespace dune

