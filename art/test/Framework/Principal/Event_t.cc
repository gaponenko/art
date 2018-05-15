// vim: set sw=2 expandtab :
#define BOOST_TEST_MODULE (Event_t)

// =====================================================================
// Event_t tests the art::Event transactional object.  It does this by
// creating products that originate from a "source", and by producing
// one product in the "current" process.
//
// A large amount of boilerplate is required to construct an
// art::Event object.  Required items include:
//
//   - A well-formed ProcessHistory
//   - Properly-constructed product-lookup tables
//
// =====================================================================

#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ModuleType.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Version/GetReleaseVersion.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <typeinfo>
#include <utility>
#include <vector>

using namespace art;
using namespace std;
using namespace string_literals;

using fhicl::ParameterSet;
using product_t = arttest::IntProduct;

namespace {

  [[gnu::unused]] art::EventID
  make_id()
  {
    return art::EventID{2112, 47, 25};
  }

  [[gnu::unused]] constexpr art::Timestamp
  make_timestamp()
  {
    return art::Timestamp{1};
  }

  [[gnu::unused]] std::string
  module_class_name()
  {
    return "IntProducer";
  }

} // namespace

// This is a gross hack, to allow us to test the event
namespace art {

  class EDProducer {
  public:
    static void
    commitEvent(EventPrincipal& ep, Event& e)
    {
      e.movePutProductsToPrincipal(ep);
    }
  };

} // namespace art

class ProductTablesFixture {
public:
  ProductTablesFixture();

  ModuleDescription currentModuleDescription_{};
  // The moduleConfigurations_ data member is a map that with the
  // following semantics:
  //   process_name <-> (module_name <-> module_configuration)
  std::map<std::string, std::map<std::string, ParameterSet>>
    moduleConfigurations_{};
  std::map<std::string, ModuleDescription> moduleDescriptions_{};

  // The descriptions_ data member is used only to create the product
  // lookup tables.
  ProductDescriptions descriptions_{};
  ProductTables producedProducts_{ProductTables::invalid()};
  ProductTable presentProducts_{};
  std::map<std::string, TypeLabelLookup_t> earlyLookup_{};
  TypeLabelLookup_t currentLookup_{};

private:
  // The 'Present' non-type parameter refers to if the product is
  // present from the source (true) or if the product is produced
  // (false).
  template <typename T, bool Present>
  ModuleDescription const& registerProduct(
    std::string const& tag,
    std::string const& moduleLabel,
    std::string const& processName,
    std::string const& productInstanceName = {});
};

ProductTablesFixture::ProductTablesFixture()
{
  constexpr bool presentFromSource{true};
  constexpr bool produced{false};

  // Register products for "EARLY" process
  registerProduct<product_t, presentFromSource>(
    "nolabel_tag", "modOne", "EARLY");
  registerProduct<product_t, presentFromSource>(
    "int1_tag", "modMulti", "EARLY", "int1");
  registerProduct<product_t, presentFromSource>(
    "int2_tag", "modMulti", "EARLY", "int2");
  registerProduct<product_t, presentFromSource>(
    "int3_tag", "modMulti", "EARLY");

  // Register products for "LATE" process
  registerProduct<product_t, presentFromSource>(
    "int1_tag_late", "modMulti", "LATE", "int1");

  // Fill the lookups for "source-like" products
  {
    presentProducts_ = ProductTables{descriptions_}.get(InEvent);
    descriptions_.clear();
  }

  // Register single IntProduct for the "CURRENT" process
  currentModuleDescription_ = registerProduct<product_t, produced>(
    "current_tag", "modMulti", "CURRENT", "int1");

  // Create the lookup that we will use for the current-process module
  {
    producedProducts_ = ProductTables{descriptions_};
    descriptions_.clear();
  }
}

template <class T, bool Present>
ModuleDescription const&
ProductTablesFixture::registerProduct(std::string const& tag,
                                      std::string const& moduleLabel,
                                      std::string const& processName,
                                      std::string const& productInstanceName)
{
  ParameterSet moduleParams;
  moduleParams.put("module_type", module_class_name());
  moduleParams.put("module_label", moduleLabel);

  moduleConfigurations_[processName][moduleLabel] = moduleParams;

  ParameterSet processParams;
  processParams.put("process_name", processName);
  processParams.put(moduleLabel, moduleParams);

  ProcessConfiguration const process{
    processName, processParams.id(), getReleaseVersion()};

  ModuleDescription const localModuleDescription{moduleParams.id(),
                                                 module_class_name(),
                                                 moduleLabel,
                                                 ModuleThreadingType::legacy,
                                                 process,
                                                 Present};

  TypeID const product_type{typeid(T)};

  moduleDescriptions_[tag] = localModuleDescription;
  TypeLabel typeLabel{
    product_type, productInstanceName, SupportsView<T>::value, false};
  if (Present) {
    typeLabel = TypeLabel{
      product_type, productInstanceName, SupportsView<T>::value, moduleLabel};
  }
  BranchDescription pd{
    InEvent, typeLabel, moduleLabel, moduleParams.id(), process};
  if (Present) {
    earlyLookup_[tag].emplace(typeLabel, pd);
  } else {
    currentLookup_.emplace(typeLabel, pd);
  }
  descriptions_.push_back(pd);
  return moduleDescriptions_[tag];
}

struct EventTestFixture {
public:
  EventTestFixture();

  template <class T>
  ProductID addSourceProduct(std::unique_ptr<T>&& product,
                             std::string const& tag,
                             std::string const& instanceName = {});

  ProductTablesFixture& ptf();
  std::unique_ptr<RunPrincipal> runPrincipal_{};
  std::unique_ptr<SubRunPrincipal> subRunPrincipal_{};
  std::unique_ptr<EventPrincipal> principal_{nullptr};
  std::unique_ptr<Event> currentEvent_{nullptr};
};

EventTestFixture::EventTestFixture()
{
  // Construct process history for event.  This takes several lines of
  // code but other than the process names none of it is used or
  // interesting.
  ProcessHistory processHistory;
  for (auto const& process : ptf().moduleConfigurations_) {
    auto const& process_name = process.first;
    // Skip current process since it is not yet part of the history.
    if (process_name == "CURRENT") {
      continue;
    }

    // Construct the ParameterSet for each process
    auto const& module_configurations = process.second;
    ParameterSet processParameterSet;
    processParameterSet.put("process_name", process_name);
    for (auto const& modConfig : module_configurations) {
      processParameterSet.put(modConfig.first, modConfig.second);
    }

    ProcessConfiguration const processConfiguration{
      process.first, processParameterSet.id(), getReleaseVersion()};
    processHistory.push_back(processConfiguration);
  }

  auto const processHistoryID = processHistory.id();
  ProcessHistoryRegistry::emplace(processHistoryID, processHistory);

  // When any new product is added to the event principal at
  // movePutProductsToPrincipal, the "CURRENT" process will go into the
  // ProcessHistory because the process name comes from the
  // currentModuleDescription stored in the principal.  On the other hand, when
  // addSourceProduct is called, another event is created with a fake
  // moduleDescription containing the old process name and that is used to
  // create the group in the principal used to look up the object.

  constexpr auto time = make_timestamp();
  EventID const id{make_id()};
  ProcessConfiguration const& pc =
    ptf().currentModuleDescription_.processConfiguration();

  RunAuxiliary const runAux{id.run(), time, time};
  runPrincipal_ = std::make_unique<RunPrincipal>(runAux, pc, nullptr);

  SubRunAuxiliary const subRunAux{runPrincipal_->run(), 1u, time, time};
  subRunPrincipal_ = std::make_unique<SubRunPrincipal>(subRunAux, pc, nullptr);
  subRunPrincipal_->setRunPrincipal(runPrincipal_.get());

  EventAuxiliary const eventAux{id, time, true};
  auto history = std::make_unique<History>();
  const_cast<ProcessHistoryID&>(history->processHistoryID()) = processHistoryID;
  principal_ = std::make_unique<EventPrincipal>(
    eventAux, pc, &ptf().presentProducts_, std::move(history));
  principal_->setSubRunPrincipal(subRunPrincipal_.get());
  principal_->createGroupsForProducedProducts(ptf().producedProducts_);
  principal_->enableLookupOfProducedProducts(ptf().producedProducts_);

  currentEvent_ =
    std::make_unique<Event>(*principal_, ptf().currentModuleDescription_);
}

// Add the given product, of type T, to the current event, as if it
// came from the module/source specified by the given tag.
template <class T>
ProductID
EventTestFixture::addSourceProduct(std::unique_ptr<T>&& product,
                                   std::string const& tag,
                                   std::string const& instanceName)
{
  auto description = ptf().moduleDescriptions_.find(tag);
  if (description == ptf().moduleDescriptions_.end())
    throw art::Exception(errors::LogicError)
      << "Failed to find a module description for tag: " << tag << '\n';

  auto lookup = ptf().earlyLookup_.find(tag);
  if (lookup == ptf().earlyLookup_.end())
    throw art::Exception(errors::LogicError)
      << "Failed to find a product lookup for tag: " << tag << '\n';

  Event temporaryEvent{*principal_, description->second};
  ProductID const id{temporaryEvent.put(std::move(product), instanceName)};

  EDProducer::commitEvent(*principal_, temporaryEvent);
  return id;
}

ProductTablesFixture&
EventTestFixture::ptf()
{
  static ProductTablesFixture ptf_s;
  return ptf_s;
}

BOOST_FIXTURE_TEST_SUITE(Event_t, EventTestFixture)

BOOST_AUTO_TEST_CASE(badProductID)
{
  auto const pid = ProductID::invalid();
  InputTag tag{};
  if (auto pd = currentEvent_->getProductDescription(pid)) {
    tag = pd->inputTag();
  }
  BOOST_CHECK(tag.empty());
}

BOOST_AUTO_TEST_CASE(emptyEvent)
{
  BOOST_REQUIRE(currentEvent_.get());
  BOOST_REQUIRE_EQUAL(currentEvent_->id(), make_id());
  BOOST_REQUIRE(currentEvent_->time() == make_timestamp());
}

BOOST_AUTO_TEST_CASE(getBySelectorFromEmpty)
{
  ModuleLabelSelector const byModuleLabel{"mod1"};
  Handle<int> nonesuch;
  BOOST_REQUIRE(!nonesuch.isValid());
  BOOST_REQUIRE(!currentEvent_->get(byModuleLabel, nonesuch));
  BOOST_REQUIRE(!nonesuch.isValid());
  BOOST_REQUIRE(nonesuch.failedToGet());
  BOOST_REQUIRE_THROW(*nonesuch, cet::exception);
}

BOOST_AUTO_TEST_CASE(dereferenceDfltHandle)
{
  Handle<int> nonesuch;
  BOOST_REQUIRE_THROW(*nonesuch, cet::exception);
  BOOST_REQUIRE_THROW(nonesuch.product(), cet::exception);
}

BOOST_AUTO_TEST_CASE(putAnIntProduct)
{
  auto three = make_unique<arttest::IntProduct>(3);
  currentEvent_->put(move(three), "int1");
  EDProducer::commitEvent(*principal_, *currentEvent_);
}

BOOST_AUTO_TEST_CASE(putAndGetAnIntProduct)
{
  auto four = make_unique<arttest::IntProduct>(4);
  currentEvent_->put(move(four), "int1");
  EDProducer::commitEvent(*principal_, *currentEvent_);

  ProcessNameSelector const should_match{"CURRENT"};
  ProcessNameSelector const should_not_match{"NONESUCH"};
  // The ProcessNameSelector does not understand the "current_process"
  // string.  See notes in the Selector header file.
  ProcessNameSelector const should_also_not_match{"current_process"};
  Handle<arttest::IntProduct> h;
  BOOST_REQUIRE(currentEvent_->get(should_match, h));
  BOOST_REQUIRE(h.isValid());
  h.clear();
  BOOST_REQUIRE_THROW(*h, cet::exception);
  BOOST_REQUIRE(!currentEvent_->get(should_not_match, h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);
  BOOST_REQUIRE(!currentEvent_->get(should_also_not_match, h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);
}

BOOST_AUTO_TEST_CASE(getByProductID)
{
  using handle_t = Handle<product_t>;

  ProductID wanted;
  {
    auto one = std::make_unique<product_t>(1);
    auto const id1 = addSourceProduct(std::move(one), "int1_tag", "int1");
    BOOST_REQUIRE(id1 != ProductID{});
    wanted = id1;

    auto two = std::make_unique<product_t>(2);
    auto const id2 = addSourceProduct(std::move(two), "int2_tag", "int2");
    BOOST_REQUIRE(id2 != ProductID{});
    BOOST_REQUIRE(id2 != id1);
  }
  Handle<arttest::IntProduct> h;
  currentEvent_->get(wanted, h);
  BOOST_REQUIRE(h.isValid());
  BOOST_REQUIRE_EQUAL(h.id(), wanted);
  BOOST_REQUIRE_EQUAL(h->value, 1);

  ProductID const notpresent{};
  BOOST_REQUIRE(!currentEvent_->get(notpresent, h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE(h.failedToGet());
  BOOST_REQUIRE_THROW(*h, cet::exception);
}

BOOST_AUTO_TEST_CASE(transaction)
{
  // Put a product into an Event, and make sure that if we don't
  // movePutProductsToPrincipal, there is no product in the EventPrincipal
  // afterwards.
  BOOST_REQUIRE_EQUAL(principal_->size(), 6u);
  {
    auto three = make_unique<arttest::IntProduct>(3);
    currentEvent_->put(move(three), "int1");
    BOOST_REQUIRE_EQUAL(principal_->size(), 6u);
    // DO NOT COMMIT!
  }
  // The Event has been destroyed without a movePutProductsToPrincipal -- we
  // should not have any products in the EventPrincipal.
  BOOST_REQUIRE_EQUAL(principal_->size(), 6u);
}

BOOST_AUTO_TEST_CASE(getByInstanceName)
{
  using handle_t = Handle<product_t>;
  using handle_vec = std::vector<handle_t>;

  auto one = std::make_unique<product_t>(1);
  auto two = std::make_unique<product_t>(2);
  auto three = std::make_unique<product_t>(3);
  auto four = std::make_unique<product_t>(4);
  addSourceProduct(std::move(one), "int1_tag", "int1");
  addSourceProduct(std::move(two), "int2_tag", "int2");
  addSourceProduct(std::move(three), "int3_tag");
  addSourceProduct(std::move(four), "nolabel_tag");

  Selector const sel{ProductInstanceNameSelector{"int2"} &&
                     ModuleLabelSelector{"modMulti"}};
  handle_t h;
  BOOST_REQUIRE(currentEvent_->get(sel, h));
  BOOST_REQUIRE_EQUAL(h->value, 2);

  Selector const sel2{ProductInstanceNameSelector{"int2"} ||
                      ProductInstanceNameSelector{"int1"}};
  handle_vec handles;
  currentEvent_->getMany(sel2, handles);
  BOOST_REQUIRE_EQUAL(handles.size(), std::size_t{2});

  std::string const instance;
  Selector const sel1{ProductInstanceNameSelector{instance} &&
                      ModuleLabelSelector{"modMulti"}};

  BOOST_REQUIRE(currentEvent_->get(sel1, h));
  BOOST_REQUIRE_EQUAL(h->value, 3);
  handles.clear();

  currentEvent_->getMany(ModuleLabelSelector{"modMulti"}, handles);
  BOOST_REQUIRE_EQUAL(handles.size(), 3u);
  handles.clear();
  currentEvent_->getMany(ModuleLabelSelector{"nomatch"}, handles);
  BOOST_REQUIRE(handles.empty());

  std::vector<Handle<int>> nomatches;
  currentEvent_->getMany(ModuleLabelSelector{"modMulti"}, nomatches);
  BOOST_REQUIRE(nomatches.empty());
}

BOOST_AUTO_TEST_CASE(getBySelector)
{
  using handle_t = Handle<product_t>;
  using handle_vec = std::vector<handle_t>;

  auto one = std::make_unique<product_t>(1);
  auto two = std::make_unique<product_t>(2);
  auto three = std::make_unique<product_t>(3);
  auto four = std::make_unique<product_t>(4);
  addSourceProduct(std::move(one), "int1_tag", "int1");
  addSourceProduct(std::move(two), "int2_tag", "int2");
  addSourceProduct(std::move(three), "int3_tag");
  addSourceProduct(std::move(four), "nolabel_tag");

  auto oneHundred = std::make_unique<product_t>(100);
  addSourceProduct(std::move(oneHundred), "int1_tag_late", "int1");

  auto twoHundred = std::make_unique<product_t>(200);
  currentEvent_->put(std::move(twoHundred), "int1");
  EDProducer::commitEvent(*principal_, *currentEvent_);

  Selector const sel{ProductInstanceNameSelector{"int2"} &&
                     ModuleLabelSelector{"modMulti"} &&
                     ProcessNameSelector{"EARLY"}};
  handle_t h;
  BOOST_REQUIRE(currentEvent_->get(sel, h));
  BOOST_REQUIRE_EQUAL(h->value, 2);

  Selector const sel1{ProductInstanceNameSelector{"nomatch"} &&
                      ModuleLabelSelector{"modMulti"} &&
                      ProcessNameSelector{"EARLY"}};
  BOOST_REQUIRE(!currentEvent_->get(sel1, h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);

  Selector const sel2{ProductInstanceNameSelector{"int2"} &&
                      ModuleLabelSelector{"nomatch"} &&
                      ProcessNameSelector{"EARLY"}};
  BOOST_REQUIRE(!currentEvent_->get(sel2, h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);

  Selector const sel3{ProductInstanceNameSelector{"int2"} &&
                      ModuleLabelSelector{"modMulti"} &&
                      ProcessNameSelector{"nomatch"}};
  BOOST_REQUIRE(!currentEvent_->get(sel3, h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);

  Selector const sel4{ModuleLabelSelector{"modMulti"} &&
                      ProcessNameSelector{"EARLY"}};
  // multiple selections throw
  BOOST_REQUIRE_THROW(currentEvent_->get(sel4, h), cet::exception);

  Selector const sel5{ModuleLabelSelector{"modMulti"} &&
                      ProcessNameSelector{"LATE"}};
  currentEvent_->get(sel5, h);
  BOOST_REQUIRE_EQUAL(h->value, 100);

  Selector const sel6{ModuleLabelSelector{"modMulti"} &&
                      ProcessNameSelector{"CURRENT"}};
  currentEvent_->get(sel6, h);
  BOOST_REQUIRE_EQUAL(h->value, 200);

  Selector const sel7{ModuleLabelSelector{"modMulti"}};
  currentEvent_->get(sel7, h);
  BOOST_REQUIRE_EQUAL(h->value, 200);
  handle_vec handles;
  currentEvent_->getMany(ModuleLabelSelector{"modMulti"}, handles);
  BOOST_REQUIRE_EQUAL(handles.size(), 5u);
  int sum = 0;
  for (int k = 0; k < 5; ++k) {
    sum += handles[k]->value;
  }
  BOOST_REQUIRE_EQUAL(sum, 306);
}

BOOST_AUTO_TEST_CASE(getByLabel)
{
  using handle_t = Handle<product_t>;
  using handle_vec = std::vector<handle_t>;

  auto one = std::make_unique<product_t>(1);
  auto two = std::make_unique<product_t>(2);
  auto three = std::make_unique<product_t>(3);
  auto four = std::make_unique<product_t>(4);
  addSourceProduct(std::move(one), "int1_tag", "int1");
  addSourceProduct(std::move(two), "int2_tag", "int2");
  addSourceProduct(std::move(three), "int3_tag");
  addSourceProduct(std::move(four), "nolabel_tag");

  auto oneHundred = std::make_unique<product_t>(100);
  addSourceProduct(std::move(oneHundred), "int1_tag_late", "int1");

  auto twoHundred = std::make_unique<product_t>(200);
  currentEvent_->put(std::move(twoHundred), "int1");

  EDProducer::commitEvent(*principal_, *currentEvent_);
  handle_t h;
  BOOST_REQUIRE(currentEvent_->getByLabel("modMulti", h));
  BOOST_REQUIRE_EQUAL(h->value, 3);
  BOOST_REQUIRE(currentEvent_->getByLabel("modMulti", "int1", h));
  BOOST_REQUIRE_EQUAL(h->value, 200);
  BOOST_REQUIRE(currentEvent_->getByLabel("modMulti", "int1", "CURRENT", h));
  BOOST_REQUIRE_EQUAL(h->value, 200);
  BOOST_REQUIRE(
    currentEvent_->getByLabel("modMulti", "int1", "current_process", h));
  BOOST_REQUIRE_EQUAL(h->value, 200);
  BOOST_REQUIRE(!currentEvent_->getByLabel("modMulti", "nomatch", h));
  BOOST_REQUIRE(!h.isValid());
  BOOST_REQUIRE_THROW(*h, cet::exception);

  InputTag const inputTag{"modMulti", "int1"};
  BOOST_REQUIRE(currentEvent_->getByLabel(inputTag, h));
  BOOST_REQUIRE_EQUAL(h->value, 200);

  GroupQueryResult bh{principal_->getByLabel(WrappedTypeID::make<product_t>(),
                                             "modMulti",
                                             "int1",
                                             ProcessTag{"LATE", "CURRENT"})};
  h = handle_t{bh};
  BOOST_REQUIRE_EQUAL(h->value, 100);

  GroupQueryResult bh2{
    principal_->getByLabel(WrappedTypeID::make<product_t>(),
                           "modMulti",
                           "int1",
                           ProcessTag{"nomatch", "CURRENT"})};
  BOOST_REQUIRE(!bh2.succeeded());
}

BOOST_AUTO_TEST_CASE(getByLabelSpecialProcessNames)
{
  addSourceProduct(std::make_unique<product_t>(2), "int2_tag", "int2");
  currentEvent_->put(std::make_unique<product_t>(200), "int1");

  // Try to retrieve something that does not exist in the current process.
  InputTag const badEarlyTag{"modMulti", "int2", "current_process"};
  BOOST_REQUIRE_THROW(currentEvent_->getValidHandle<product_t>(badEarlyTag),
                      cet::exception);

  // Verify that it can be read using the 'input_source' process name
  Handle<product_t> h;
  InputTag const goodEarlyTag{"modMulti", "int2", "input_source"};
  BOOST_REQUIRE(currentEvent_->getByLabel(goodEarlyTag, h));
  BOOST_REQUIRE_EQUAL(h->value, 2);

  // Verify that "int1" cannot be looked up using the "input_source"
  // process name.
  InputTag const badCurrentTag{"modMulti", "int1", "input_source"};
  BOOST_REQUIRE_THROW(currentEvent_->getValidHandle<product_t>(badCurrentTag),
                      cet::exception);
}

BOOST_AUTO_TEST_CASE(getManyByType)
{
  using handle_t = Handle<product_t>;
  using handle_vec = std::vector<handle_t>;

  auto one = std::make_unique<product_t>(1);
  auto two = std::make_unique<product_t>(2);
  auto three = std::make_unique<product_t>(3);
  auto four = std::make_unique<product_t>(4);
  addSourceProduct(std::move(one), "int1_tag", "int1");
  addSourceProduct(std::move(two), "int2_tag", "int2");
  addSourceProduct(std::move(three), "int3_tag");
  addSourceProduct(std::move(four), "nolabel_tag");

  auto oneHundred = std::make_unique<product_t>(100);
  addSourceProduct(std::move(oneHundred), "int1_tag_late", "int1");

  auto twoHundred = std::make_unique<product_t>(200);
  currentEvent_->put(std::move(twoHundred), "int1");

  EDProducer::commitEvent(*principal_, *currentEvent_);
  handle_vec handles;
  currentEvent_->getManyByType(handles);
  BOOST_REQUIRE_EQUAL(handles.size(), 6u);
  int sum = 0;
  for (int k = 0; k < 6; ++k) {
    sum += handles[k]->value;
  }
  BOOST_REQUIRE_EQUAL(sum, 310);
}

BOOST_AUTO_TEST_CASE(printHistory)
{
  auto const& history = currentEvent_->processHistory();
  std::ofstream out{"history.log"};

  cet::copy_all(
    history,
    std::ostream_iterator<ProcessHistory::const_iterator::value_type>(out,
                                                                      "\n"));
}

BOOST_AUTO_TEST_SUITE_END()
