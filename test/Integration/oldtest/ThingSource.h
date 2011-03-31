#ifndef test_Integration_oldtest_ThingSource_h
#define test_Integration_oldtest_ThingSource_h

/** \class ThingSource
 *
 * \version   1st Version Dec. 27, 2005

 *
 ************************************************************/

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/GeneratedInputSource.h"
#include "FWCore/Integration/test/ThingAlgorithm.h"

namespace arttest {
  class ThingSource : public art::GeneratedInputSource {
  public:

    // The following is not yet used, but will be the primary
    // constructor when the parameter set system is available.
    //
    explicit ThingSource(fhicl::ParameterSet const& pset, art::InputSourceDescription const& desc);

    virtual ~ThingSource();

    virtual bool produce(art::Event& e);

    virtual void beginRun(art::Run& r);

    virtual void endRun(art::Run& r);

    virtual void beginSubRun(art::SubRun& sr);

    virtual void endSubRun(art::SubRun& sr);

  private:
    ThingAlgorithm alg_;
  };
}
#endif /* test_Integration_oldtest_ThingSource_h */

// Local Variables:
// mode: c++
// End:
