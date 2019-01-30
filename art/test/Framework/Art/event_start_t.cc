#define BOOST_TEST_MODULE (event start test)
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Art/detail/event_start.h"
#include "canvas/Utilities/Exception.h"

#include <string_view>

namespace {
  bool
  invalid_event_id(art::Exception const& e, std::string const& spec)
  {
    std::string const expected{"The specification '" + spec +
                               "' is not a valid EventID"};
    std::string_view const msg{e.what()};
    return msg.find(expected) != std::string_view::npos;
  }
}

using art::detail::event_start;

BOOST_AUTO_TEST_SUITE(event_start_test)

BOOST_AUTO_TEST_CASE(well_formed)
{
  auto const [r, sr, e] = event_start("1:2:3");
  BOOST_CHECK_EQUAL(r, 1u);
  BOOST_CHECK_EQUAL(sr, 2u);
  BOOST_CHECK_EQUAL(e, 3u);
}

BOOST_AUTO_TEST_CASE(leading_and_trailing_spaces_allowed)
{
  auto const [r, sr, e] = event_start(" 1:2  :   3");
  BOOST_CHECK_EQUAL(r, 1u);
  BOOST_CHECK_EQUAL(sr, 2u);
  BOOST_CHECK_EQUAL(e, 3u);
}

BOOST_AUTO_TEST_CASE(deprecated)
{
  auto const [r, sr, e] = event_start("4");
  BOOST_CHECK_EQUAL(r, 1u);
  BOOST_CHECK_EQUAL(sr, 0u);
  BOOST_CHECK_EQUAL(e, 4u);
}

#define VERIFY_EXCEPTION_WITH_MSG(spec)                                        \
  BOOST_CHECK_EXCEPTION(event_start(spec), art::Exception, [](auto const& e) { \
    return invalid_event_id(e, spec);                                          \
  })

BOOST_AUTO_TEST_CASE(out_of_range_run)
{
  VERIFY_EXCEPTION_WITH_MSG("-1:0:0");
  VERIFY_EXCEPTION_WITH_MSG("0:0:0");
  VERIFY_EXCEPTION_WITH_MSG("4294967295:0:0");
}

BOOST_AUTO_TEST_CASE(out_of_range_subrun)
{
  VERIFY_EXCEPTION_WITH_MSG("1:-1:0");
  VERIFY_EXCEPTION_WITH_MSG("1:4294967295:0");
}

BOOST_AUTO_TEST_CASE(out_of_range_event)
{
  VERIFY_EXCEPTION_WITH_MSG("1:0:-1");
  VERIFY_EXCEPTION_WITH_MSG("1:0:4294967295");
}

BOOST_AUTO_TEST_CASE(wrong_number_of_colons)
{
  VERIFY_EXCEPTION_WITH_MSG("1:1");
  VERIFY_EXCEPTION_WITH_MSG("1:1:1:2");
}

BOOST_AUTO_TEST_CASE(bad_fields)
{
  VERIFY_EXCEPTION_WITH_MSG("1:  - 2:3");
  VERIFY_EXCEPTION_WITH_MSG("1:1:");
  VERIFY_EXCEPTION_WITH_MSG("1:hello billy:3");
  VERIFY_EXCEPTION_WITH_MSG("1:2.8:3");
}

BOOST_AUTO_TEST_SUITE_END()

#undef VERIFY_EXCEPTION_WITH_MSG
