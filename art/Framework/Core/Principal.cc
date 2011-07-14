#include "art/Framework/Core/Principal.h"

#include "art/Framework/Core/Selector.h"
#include "art/Persistency/Common/BasicHandle.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"
#include "cetlib/container_algorithms.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <utility>


using namespace cet;
using namespace std;


namespace art {

   Principal::Principal(cet::exempt_ptr<ProductRegistry const> reg,
                       ProcessConfiguration const& pc,
                       ProcessHistoryID const& hist,
                       std::shared_ptr<BranchMapper> mapper,
                       std::shared_ptr<DelayedReader> rtrv) :
    EDProductGetter(),
    processHistoryPtr_(std::shared_ptr<ProcessHistory>(new ProcessHistory)),
    processConfiguration_(pc),
    processHistoryModified_(false),
    groups_(),
    preg_(reg),
    branchMapperPtr_(mapper),
    store_(rtrv)
  {
    if (hist.isValid()) {
      assert(! ProcessHistoryRegistry::empty());
      bool found = ProcessHistoryRegistry::get(hist, *processHistoryPtr_);
      assert(found);
    }
  }

  Principal::~Principal() {
  }

  Group*
  Principal::getExistingGroup(Group const& group) {
    GroupCollection::const_iterator it = groups_.find(group.productDescription().branchID());
    if (it == groups_.end()) return 0;
    return it->second.get();
  }

  void
  Principal::addGroup_(auto_ptr<Group> group) {
    ConstBranchDescription const& bd = group->productDescription();
    assert (!bd.className().empty());
    assert (!bd.friendlyClassName().empty());
    assert (!bd.moduleLabel().empty());
    assert (!bd.processName().empty());
    SharedGroupPtr g(group);
    groups_.insert(make_pair(bd.branchID(), g));
  }

  void
  Principal::replaceGroup(auto_ptr<Group> group) {
    ConstBranchDescription const& bd = group->productDescription();
    assert (!bd.className().empty());
    assert (!bd.friendlyClassName().empty());
    assert (!bd.moduleLabel().empty());
    assert (!bd.processName().empty());
    SharedGroupPtr g(group);
    groups_[bd.branchID()]->replace(*g);
  }

  void
  Principal::addToProcessHistory() const {
    if (processHistoryModified_) return;
    ProcessHistory& ph = *processHistoryPtr_;
    string const& processName = processConfiguration_.processName();
    for (ProcessHistory::const_iterator
	   it = ph.begin(),
	   itEnd = ph.end(); it != itEnd; ++it) {
      if (processName == it->processName()) {
        throw art::Exception(errors::Configuration)
	  << "The process name " << processName
	  << " was previously used on these products.\n"
          << "Please modify the configuration file to use a "
	  << "distinct process name.\n";
      }
    }
    ph.push_back(processConfiguration_);
    //OPTIMIZATION NOTE:  As of 0_9_0_pre3
    // For very simple Sources (e.g. EmptyEvent) this routine takes up nearly 50% of the time per event.
    // 96% of the time for this routine is being spent in computing the
    // ProcessHistory id which happens because we are reconstructing the ProcessHistory for each event.
    // (The process ID is first computed in the call to 'insertMapped(..)' below.)
    // It would probably be better to move the ProcessHistory construction out to somewhere
    // which persists for longer than one Event
    ProcessHistoryRegistry::put(ph);
    setProcessHistoryID(ph.id());
    processHistoryModified_ = true;
  }

  ProcessHistory const&
  Principal::processHistory() const {
    return *processHistoryPtr_;
  }

  Principal::SharedConstGroupPtr const
  Principal::getGroup(BranchID const& bid, bool resolveProd, bool resolveProv, bool fillOnDemand) const {
    GroupCollection::const_iterator it = groups_.find(bid);
    if (it == groups_.end()) {
      return SharedConstGroupPtr();
    }
    SharedConstGroupPtr const& g = it->second;
    if (resolveProv && (g->provenanceAvailable() || g->onDemand())) {
      if(g->onDemand()) {
         //must execute the unscheduled to get the provenance
         this->resolveProduct(*g, true);
         //check if this failed (say because of a caught exception)
         if( 0 == g->product()) {
            //behavior is the same as if the group wasn't there
            return SharedConstGroupPtr();
         }
      }
      this->resolveProvenance(*g);
    }
    if (resolveProd && !g->productUnavailable()) {
      this->resolveProduct(*g, fillOnDemand);
      if(g->onDemand() && 0 == g->product()) {
         //behavior is the same as if the group wasn't there
         return SharedConstGroupPtr();
      }
    }
    return g;
  }

  BasicHandle
  Principal::getBySelector(TypeID const& productType,
                           SelectorBase const& sel) const {

    BasicHandleVec results;

    int nFound = findGroups(productType,
                            preg_->productLookup(),
                            sel,
                            results,
                            true);

    if (nFound == 0) {
      std::shared_ptr<cet::exception> whyFailed( new art::Exception(art::errors::ProductNotFound) );
      *whyFailed
        << "getBySelector: Found zero products matching all criteria\n"
        << "Looking for type: " << productType << "\n";
      return BasicHandle(whyFailed);
    }
    if (nFound > 1) {
      throw art::Exception(art::errors::ProductNotFound)
        << "getBySelector: Found "<<nFound<<" products rather than one which match all criteria\n"
        << "Looking for type: " << productType << "\n";
    }
    return results[0];
  }

  BasicHandle
  Principal::getByLabel(TypeID const& productType,
                        string const& label,
                        string const& productInstanceName,
                        string const& processName) const
  {

    BasicHandleVec results;

    art::Selector sel(art::ModuleLabelSelector(label) &&
                      art::ProductInstanceNameSelector(productInstanceName) &&
                      art::ProcessNameSelector(processName));

    int nFound = findGroups(productType,
                            preg_->productLookup(),
                            sel,
                            results,
                            true);

    if (nFound == 0) {
      std::shared_ptr<cet::exception> whyFailed( new art::Exception(art::errors::ProductNotFound) );
      *whyFailed
        << "getByLabel: Found zero products matching all criteria\n"
        << "Looking for type: " << productType << "\n"
        << "Looking for module label: " << label << "\n"
        << "Looking for productInstanceName: " << productInstanceName << "\n"
        << (processName.empty() ? "" : "Looking for process: ") << processName << "\n";
      return BasicHandle(whyFailed);
    }
    if (nFound > 1) {
      throw art::Exception(art::errors::ProductNotFound)
        << "getByLabel: Found "<<nFound<<" products rather than one which match all criteria\n"
        << "Looking for type: " << productType << "\n"
        << "Looking for module label: " << label << "\n"
        << "Looking for productInstanceName: " << productInstanceName << "\n"
        << (processName.empty() ? "" : "Looking for process: ") << processName << "\n";
    }
    return results[0];
  }


  void
  Principal::getMany(TypeID const& productType,
                     SelectorBase const& sel,
                     BasicHandleVec& results) const {

    findGroups(productType,
               preg_->productLookup(),
               sel,
               results,
               false);

    return;
  }

  void
  Principal::getManyByType(TypeID const& productType,
                           BasicHandleVec& results) const {

    art::MatchAllSelector sel;

    findGroups(productType,
               preg_->productLookup(),
               sel,
               results,
               false);
    return;
  }

  size_t
  Principal::getMatchingSequence(TypeID const& typeID,
                                 SelectorBase const& selector,
                                 BasicHandleVec& results,
                                 bool stopIfProcessHasMatch) const {

    // One new argument is the element lookup container
    // Otherwise this just passes through the arguments to findGroups
    return findGroups(typeID,
                      preg_->elementLookup(),
                      selector,
                      results,
                      stopIfProcessHasMatch);
  }

  void
  Principal::readImmediate() const {
    readProvenanceImmediate();
    for (Principal::const_iterator i = begin(), iEnd = end(); i != iEnd; ++i) {
      if (!i->second->productUnavailable()) {
        resolveProduct(*i->second, false);
      }
    }
  }

  void
  Principal::readProvenanceImmediate() const {
    for (Principal::const_iterator i = begin(), iEnd = end(); i != iEnd; ++i) {
      if (i->second->provenanceAvailable()) {
        resolveProvenance(*i->second);
      }
    }
    branchMapperPtr_->setDelayedRead(false);
  }

  size_t
  Principal::findGroups(TypeID const& typeID,
                        TypeLookup const& typeLookup,
                        SelectorBase const& selector,
                        BasicHandleVec& results,
                        bool stopIfProcessHasMatch) const {
    assert(results.empty());

    // A class without a dictionary cannot be in an Event/SubRun/Run.
    // First, we check if the class has a dictionary.  If it does not,
    // we return immediately.  This is necessary to avoid an exception
    // being thrown inside TypeID::friendlyClassName().
    if (!typeID.hasDictionary()) {
      return 0;
    }

    TypeLookup::const_iterator i = typeLookup.find(typeID.friendlyClassName());

    if (i == typeLookup.end()) {
      return 0;
    }

    const ProcessLookup& processLookup = i->second;

    // Handle groups for current process, note that we need to
    // look at the current process even if it is not in the processHistory
    // because of potential unscheduled (onDemand) production
    findGroupsForProcess(processConfiguration_.processName(),
                         processLookup,
                         selector,
                         results);

    // Loop over processes in reverse time order.  Sometimes we want to stop
    // after we find a process with matches so check for that at each step.
    for (ProcessHistory::const_reverse_iterator iproc = processHistory().rbegin(),
           eproc = processHistory().rend();
         iproc != eproc && (results.empty() || !stopIfProcessHasMatch);
         ++iproc) {

      // We just dealt with the current process before the loop so skip it
      if (iproc->processName() == processConfiguration_.processName()) continue;

      findGroupsForProcess(iproc->processName(),
                           processLookup,
                           selector,
                           results);
    }
    return results.size();
  }

  void
  Principal::findGroupsForProcess(string const& processName,
                                  ProcessLookup const& processLookup,
                                  SelectorBase const& selector,
                                  BasicHandleVec& results) const {

    ProcessLookup::const_iterator j = processLookup.find(processName);

    if (j == processLookup.end()) return;

    // This is a vector of indexes into the productID vector
    // These indexes point to groups with desired process name (and
    // also type when this function is called from findGroups)
    vector<BranchID> const& vindex = j->second;

    for (vector<BranchID>::const_iterator ib(vindex.begin()), ie(vindex.end());
         ib != ie;
         ++ib) {
      SharedConstGroupPtr const& group = getGroup(*ib, false, false, false);
      if(group.get() == 0) {
        continue;
      }

      if (selector.match(group->productDescription())) {

        // Skip product if not available.
        if (!group->productUnavailable()) {
          this->resolveProduct(*group, true);
          // If the product is a dummy filler, group will now be marked unavailable.
          // Unscheduled execution can fail to produce the EDProduct so check
          if (!group->productUnavailable() && !group->onDemand()) {
            // Found a good match, save it
            BasicHandle bh(group->product(), group->provenance());
            bh.provenance()->setStore(branchMapperPtr_);
            results.push_back(bh);
          }
        }
      }
    }
    return;
  }

  void
  Principal::resolveProduct(Group const& g, bool fillOnDemand) const {
    if (g.productUnavailable()) {
      throw art::Exception(errors::ProductNotFound,"InaccessibleProduct")
        << "resolve_: product is not accessible\n"
        << g.provenance() << '\n';
    }

    if (g.product()) return; // nothing to do.

    // Try unscheduled production.
    if (g.onDemand()) {
      if (fillOnDemand) unscheduledFill(g.productDescription().moduleLabel());
      return;
    }

    // must attempt to load from persistent store
    BranchKey const bk = BranchKey(g.productDescription());
    auto_ptr<EDProduct> edp(store_->getProduct(bk, this));

    // Now fix up the Group
    g.setProduct(edp);
  }

  void
  Principal::resolveProvenance(Group const& g) const {
    if (!g.productProvenancePtr()) {
      // Now fix up the Group
      g.setProvenance(branchMapperPtr()->branchToEntryInfo(g.productDescription().branchID()));
    }
  }

  OutputHandle
  Principal::getForOutput(BranchID const& bid, bool getProd) const {
    SharedConstGroupPtr const& g = getGroup(bid, getProd, true, false);
    if (g.get() == 0) {
      return OutputHandle();
    }
    if (getProd && (g->product() == 0 || !g->product()->isPresent()) &&
            g->productDescription().present() &&
            g->productDescription().branchType() == InEvent &&
            productstatus::present(g->productProvenancePtr()->productStatus())) {
        throw art::Exception(art::errors::LogicError, "Principal::getForOutput\n")
         << "A product with a status of 'present' is not actually present.\n"
         << "The branch name is " << g->productDescription().branchName() << "\n"
         << "Contact a framework developer.\n";
    }
    if (!g->product() && !g->productProvenancePtr()) {
      return OutputHandle();
    }
    return OutputHandle(g->product(), &g->productDescription(), g->productProvenancePtr());
  }

  Provenance
  Principal::getProvenance(BranchID const& bid) const {
    SharedConstGroupPtr const& g = getGroup(bid, false, true, true);
    if (g.get() == 0) {
      throw art::Exception(art::errors::ProductNotFound,"InvalidID")
        << "getProvenance: no product with given branch id: "<< bid << "\n";
    }

    if (g->onDemand()) {
      unscheduledFill(g->productDescription().moduleLabel());
    }
    // We already tried to produce the unscheduled products above
    // If they still are not there, then throw
    if (g->onDemand()) {
      throw art::Exception(art::errors::ProductNotFound)
        << "getProvenance: no product with given BranchID: "<< bid <<"\n";
    }

    return *g->provenance();
  }

  // This one is mostly for test printout purposes
  // No attempt to trigger on demand execution
  // Skips provenance when the EDProduct is not there
  void
  Principal::getAllProvenance(vector<Provenance const*> & provenances) const {
    provenances.clear();
    for (const_iterator i = begin(), iEnd = end(); i != iEnd; ++i) {
      if (i->second->provenanceAvailable()) {
        resolveProvenance(*i->second);
        if (i->second->provenance()->productProvenanceSharedPtr() &&
            i->second->provenance()->isPresent() &&
            i->second->provenance()->product().present())
           provenances.push_back(i->second->provenance());
        }
    }
  }

  void
  Principal::recombine(Principal & other, vector<BranchID> const& bids) {
    for (vector<BranchID>::const_iterator it = bids.begin(), itEnd = bids.end(); it != itEnd; ++it) {
      groups_[*it].swap(other.groups_[*it]);
    }
    store_->mergeReaders(other.store());
    branchMapperPtr_->mergeMappers(other.branchMapperPtr());
  }

  EDProduct const*
  Principal::getIt(ProductID const& pid) const {
    assert(0);
    return 0;
  }
}
