#ifndef art_Framework_Core_Principal_h
#define art_Framework_Core_Principal_h

/*----------------------------------------------------------------------

Principal: This is the implementation of the classes responsible
for management of EDProducts. It is not seen by reconstruction code.

The major internal component of the Principal is the Group, which
contains an EDProduct and its associated Provenance, along with
ancillary transient information regarding the two. Groups are handled
through shared pointers.

The Principal returns BasicHandle, rather than a shared
pointer to a Group, when queried.

(Historical note: prior to April 2007 this class was named DataBlockImpl)

----------------------------------------------------------------------*/

#include "art/Framework/Core/FCPfwd.h"
#include "art/Framework/Core/Group.h"
#include "art/Framework/Core/NoDelayedReader.h"
#include "art/Persistency/Common/BasicHandle.h"
#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Common/OutputHandle.h"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "art/Utilities/InputTag.h"
#include "art/Utilities/TypeID.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/value_ptr.h"
#include "cpp0x/memory"
#include <map>
#include <string>
#include <vector>

namespace art {
  class Principal : public EDProductGetter {
  public:
    typedef std::map<BranchID, std::shared_ptr<Group> > GroupCollection;
    typedef GroupCollection::const_iterator const_iterator;
    typedef ProcessHistory::const_iterator ProcessNameConstIterator;
    typedef std::shared_ptr<const Group> SharedConstGroupPtr;
    typedef std::vector<BasicHandle> BasicHandleVec;
    typedef GroupCollection::size_type      size_type;

    typedef std::shared_ptr<Group> SharedGroupPtr;
    typedef std::string ProcessName;

    Principal(cet::exempt_ptr<ProductRegistry const> reg,
              ProcessConfiguration const& pc,
              ProcessHistoryID const& hist,
              std::auto_ptr<BranchMapper> mapper,
              std::shared_ptr<DelayedReader> rtrv);

    virtual ~Principal();

    EDProductGetter const* prodGetter() const {return this;}

    OutputHandle getForOutput(BranchID const& bid, bool getProd) const;

    BasicHandle  getBySelector(TypeID const& tid,
                               SelectorBase const& s) const;

    BasicHandle  getByLabel(TypeID const& tid,
			    std::string const& label,
			    std::string const& productInstanceName,
			    std::string const& processName) const;

    void getMany(TypeID const& tid,
		 SelectorBase const&,
		 BasicHandleVec& results) const;

    void getManyByType(TypeID const& tid,
		 BasicHandleVec& results) const;

    // Return a vector of BasicHandles to the products which:
    //   1. are sequences,
    //   2. and have the nested type 'value_type'
    //   3. and for which typeID is the same as or a public base of
    //      this value_type,
    //   4. and which matches the given selector
    size_t getMatchingSequence(TypeID const& typeID,
			       SelectorBase const& selector,
			       BasicHandleVec& results,
			       bool stopIfProcessHasMatch) const;

    void
    readImmediate() const;

    void
    readProvenanceImmediate() const;

    ProcessHistory const& processHistory() const;

    ProcessConfiguration const& processConfiguration() const {return processConfiguration_;}

    ProductRegistry const& productRegistry() const {return *preg_;}

    std::shared_ptr<DelayedReader> store() const {return store_;}
    BranchMapper const &branchMapper() const {return *branchMapperPtr_;}

    // ----- Mark this Principal as having been updated in the
    // current Process.
    void addToProcessHistory() const;

    size_t  size() const { return groups_.size(); }

    const_iterator begin() const {return groups_.begin();}
    const_iterator end() const {return groups_.end();}

    Provenance
    getProvenance(BranchID const& bid) const;

    // Obtain the branch type suitable for products to be put in the
    // principal.
    virtual BranchType branchType() const = 0;

    // Make my DelayedReader get the EDProduct for a Group or
    // trigger unscheduled execution if required.  The Group is
    // a cache, and so can be modified through the const reference.
    // We do not change the *number* of groups through this call, and so
    // *this is const.
    void resolveProduct(Group const& g, bool fillOnDemand) const;

  protected:
    // ----- Add a new Group
    // *this takes ownership of the Group, which in turn owns its
    // data.
    void addGroup_(std::auto_ptr<Group> g);
    Group*  getExistingGroup(Group const& g);
    void replaceGroup(std::auto_ptr<Group> g);
    SharedConstGroupPtr const getGroup(BranchID const& oid,
                                       bool resolveProd,
                                       bool resolveProv,
                                       bool fillOnDemand) const;
    BranchMapper &branchMapper() {return *branchMapperPtr_;}


  private:
    virtual EDProduct const* getIt(ProductID const&) const;

    virtual void addOrReplaceGroup(std::auto_ptr<Group> g) = 0;


    virtual ProcessHistoryID const& processHistoryID() const = 0;

    virtual void setProcessHistoryID(ProcessHistoryID const& phid) const = 0;

    virtual bool unscheduledFill(std::string const& moduleLabel) const = 0;

    // Used for indices to find groups by type and process
    typedef std::map<std::string, std::vector<BranchID> > ProcessLookup;
    typedef std::map<std::string, ProcessLookup> TypeLookup;

    size_t findGroups(TypeID const& typeID,
		      TypeLookup const& typeLookup,
		      SelectorBase const& selector,
		      BasicHandleVec& results,
		      bool stopIfProcessHasMatch) const;

    void findGroupsForProcess(std::string const& processName,
                              ProcessLookup const& processLookup,
                              SelectorBase const& selector,
                              BasicHandleVec& results) const;

    std::shared_ptr<ProcessHistory> processHistoryPtr_;

    ProcessConfiguration const& processConfiguration_;

    mutable bool processHistoryModified_;

    // A vector of groups.
    GroupCollection groups_; // products and provenances are persistent

    // Pointer to the product registry. There is one entry in the registry
    // for each EDProduct in the event.
    cet::exempt_ptr<ProductRegistry const> preg_;

    // Pointer to the 'mapper' that will get provenance information
    // from the persistent store.
    cet::value_ptr<BranchMapper> branchMapperPtr_;

    // Pointer to the 'source' that will be used to obtain EDProducts
    // from the persistent store.
    std::shared_ptr<DelayedReader> store_;
  };

}
#endif /* art_Framework_Core_Principal_h */

// Local Variables:
// mode: c++
// End:
