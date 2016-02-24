////////////////////////////////////////////////////////////////////////
// Class:       TestRemoveCachedProduct
// Module Type: analyzer
// File:        TestRemoveCachedProduct_module.cc
//
// Generated at Thu Jul 24 13:06:11 2014 by Christopher Green using artmod
// from cetpkgsupport v1_06_02.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "cetlib/quiet_unit_test.hpp"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "test/TestObjects/ToyProducts.h"

namespace arttest {
  class TestRemoveCachedProduct;
}

class arttest::TestRemoveCachedProduct : public art::EDAnalyzer {
public:
  explicit TestRemoveCachedProduct(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  TestRemoveCachedProduct(TestRemoveCachedProduct const &) = delete;
  TestRemoveCachedProduct(TestRemoveCachedProduct &&) = delete;
  TestRemoveCachedProduct & operator = (TestRemoveCachedProduct const &) = delete;
  TestRemoveCachedProduct & operator = (TestRemoveCachedProduct &&) = delete;
  void analyze(art::Event const & e) override;

  void endSubRun(art::SubRun const & sr) override;
  void endRun(art::Run const & r) override;

private:

  // Declare member data here.

};


arttest::TestRemoveCachedProduct::TestRemoveCachedProduct(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p)  // ,
 // More initializers here.
{}

void arttest::TestRemoveCachedProduct::analyze(art::Event const & e)
{
  art::Handle<std::vector<int> > hi;
  BOOST_CHECK(!hi.isValid());
  BOOST_CHECK(!e.removeCachedProduct(hi));
  BOOST_CHECK(e.getByLabel("m1a", hi));
  BOOST_CHECK(hi.isValid());
  BOOST_CHECK_EQUAL(hi->size(), 16ul);
  BOOST_CHECK(e.removeCachedProduct(hi));
  BOOST_CHECK(!hi.isValid());
}

void arttest::TestRemoveCachedProduct::endSubRun(art::SubRun const & sr)
{
  art::Handle<IntProduct> hi;
  BOOST_CHECK(!hi.isValid());
  BOOST_CHECK(!sr.removeCachedProduct(hi));
  sr.getByLabel("m2", hi);
  BOOST_CHECK(hi.isValid());
  BOOST_CHECK_EQUAL(hi->value, 1);
  BOOST_CHECK(sr.removeCachedProduct(hi));
  BOOST_CHECK(!hi.isValid());
}

void arttest::TestRemoveCachedProduct::endRun(art::Run const & r)
{
  art::Handle<IntProduct> hi;
  BOOST_CHECK(!hi.isValid());
  BOOST_CHECK(!r.removeCachedProduct(hi));
  r.getByLabel("m3", hi);
  BOOST_CHECK(hi.isValid());
  BOOST_CHECK_EQUAL(hi->value, 2);
  BOOST_CHECK(r.removeCachedProduct(hi));
  BOOST_CHECK(!hi.isValid());
}

DEFINE_ART_MODULE(arttest::TestRemoveCachedProduct)
