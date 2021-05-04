#define BOOST_TEST_MODULE (make_tool_t)
#include "boost/test/unit_test.hpp"

#include "art/Utilities/make_tool.h"
#include "art/test/Utilities/tools/ClassTool.h"
#include "art/test/Utilities/tools/FunctionTool.h"
#include "art/test/Utilities/tools/NestedClassTool.h"
#include "art/test/Utilities/tools/NestedFunctionInClassTool.h"
#include "art/test/Utilities/tools/NestedFunctionTool.h"
#include "art/test/Utilities/tools/OperationBase.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace {
  auto
  pset_from_oss(std::ostringstream const& ss)
  {
    return fhicl::ParameterSet::make(ss.str());
  }
} // namespace

using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(make_tool_t)

BOOST_AUTO_TEST_CASE(tool_class)
{
  fhicl::ParameterSet ps;
  ps.put("tool_type", "ClassTool"s);
  auto t1 = art::make_tool<arttest::ClassTool>(ps);
  BOOST_TEST(t1->addOne(17) == 18);
}

BOOST_AUTO_TEST_CASE(tool_function)
{
  fhicl::ParameterSet ps;
  ps.put("tool_type", "FunctionTool"s);
  int i{17};
  auto addOne1 = art::make_tool<decltype(arttest::addOne)>(ps, "addOne");
  auto addOne2 = art::make_tool<int(int)>(ps, "addOne");
  BOOST_TEST(addOne1(i) == 18);
  BOOST_TEST(addOne2(i) == 18);
}

BOOST_AUTO_TEST_CASE(nested_function_tools)
{
  std::ostringstream ss;
  ss << "tool_type: NestedFunctionTool\n"
     << "addOneTool: {"
     << "  tool_type: FunctionTool"
     << "}";
  auto const& ps = pset_from_oss(ss);
  auto callThroughToAddOne =
    art::make_tool<decltype(arttest::callThroughToAddOne)>(
      ps, "callThroughToAddOne");
  auto const& nestedPS = ps.get<fhicl::ParameterSet>("addOneTool");
  BOOST_TEST(callThroughToAddOne(nestedPS, 17) == 18);
}

BOOST_AUTO_TEST_CASE(nested_class_tools)
{
  std::ostringstream ss;
  ss << "tool_type: NestedClassTool\n"
     << "addOneTool: {"
     << "  tool_type: ClassTool"
     << "}";
  auto const& ps = pset_from_oss(ss);
  auto t = art::make_tool<arttest::NestedClassTool>(ps);
  BOOST_TEST(t->callThroughToAddOne(17) == 18);
}

BOOST_AUTO_TEST_CASE(nested_function_in_class_tools)
{
  std::ostringstream ss;
  ss << "tool_type: NestedFunctionInClassTool\n"
     << "addOneTool: {"
     << "  tool_type: FunctionTool"
     << "}";
  auto const& ps = pset_from_oss(ss);
  auto t = art::make_tool<arttest::NestedFunctionInClassTool>(ps);
  BOOST_TEST(t->callThroughToAddOne(17) == 18);
}

BOOST_AUTO_TEST_CASE(polymorphic_tools)
{
  int i{17};
  {
    fhicl::ParameterSet ps;
    ps.put("tool_type", "AddNumber");
    ps.put("incrementBy", 7);
    art::make_tool<arttest::OperationBase>(ps)->adjustNumber(i);
    BOOST_TEST(i == 24);
  }
  {
    fhicl::ParameterSet ps;
    ps.put("tool_type", "SubtractNumber");
    art::make_tool<arttest::OperationBase>(ps)->adjustNumber(i);
    BOOST_TEST(i == 23);
  }
  {
    fhicl::ParameterSet ps;
    ps.put("tool_type", "MultiplyNumber");
    art::make_tool<arttest::OperationBase>(ps)->adjustNumber(i);
    BOOST_TEST(i == 46);
  }
}

BOOST_AUTO_TEST_SUITE_END()
