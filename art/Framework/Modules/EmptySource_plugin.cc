// ======================================================================
//
// EmptySource_plugin
//
// ======================================================================


// --- Framework support:
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/GeneratedInputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/ParameterSet/ParameterSet.h"


// --- Contents:
namespace edm {
  class EmptySource;
}
using edm::EmptySource;


// ======================================================================


class edm::EmptySource
  : public GeneratedInputSource
{
public:
  explicit EmptySource( ParameterSet           const &
                      , InputSourceDescription const & );
  ~EmptySource();

private:
  virtual bool produce( Event & );

};  // EmptySource


// ======================================================================


EmptySource::EmptySource( ParameterSet           const & pset
                        , InputSourceDescription const & desc )
  : GeneratedInputSource( pset, desc )
{ }


EmptySource::~EmptySource()
{ }


bool
  EmptySource::produce( Event & )
{
  return true;
}


// ======================================================================


DEFINE_FWK_INPUT_SOURCE(EmptySource);
