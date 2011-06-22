#ifndef art_Framework_Modules_MixFilter_h
#define art_Framework_Modules_MixFilter_h

// ======================================================================
//
// The MixFilter class template is used to create filters capable of
// mixing products from a secondary event stream into the primary event.
//
// The MixFilter class template requires the use of a type T as its
// template parameter; this type T must supply the following non-static
// member functions:
//
//    T(fhicl::ParameterSet const &p, art::MixHelper &helper);
//
//    // Construct an object of type T. The ParameterSet provided will
//    // be the configuration of the module constructing the T. The
//    // helper is not copyable and must be used in the constructor to
//    // register:
//    //
//    // a. Any mixing operations to be carried out by this module by
//    //    means of declareMixOp<> calls.
//    //
//    // b. Any non-mix (e.g. bookkeeping) products by means of
//    //    produces<> calls.
//    //
//    // Further details may be found in
//    // art/Framework/ProductMix/MixHelper.h.
//
//    size_t nSecondaries() const;
//
//    // Provide the number of secondary events to be mixed into the
//    // current primary event.
//
// In addition, T may optionally provide any or all of the following
// member functions; each will be called at the appropriate time iff
// it is declared.  (There must, of course, be a function definition
// corresponding to each declared function.)
//
//    void startEvent();
//
//    // Reset internal cache information at the start of the current
//    // event.
//
//    void processEventIDs(art::EventIDSequence const &seq);
//
//    // Receive the ordered sequence of EventIDs that will be mixed into
//    // the current event; useful for bookkeeping purposes.
//
//    void finalizeEvent(art::Event &t);
//
//    // Do end-of-event tasks (e.g., inserting bookkeeping objects into
//    // the primary event).
//
// Functions declared to the MixHelper to actually carry out the mixing
// of the products may be (1) member functions of this or another class;
// or (2) free functions or (3) function objects.
// ======================================================================

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/IO/ProductMix/MixContainerTypes.h"
#include "art/Framework/IO/ProductMix/MixHelper.h"
#include "art/Framework/IO/ProductMix/MixOpBase.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Utilities/detail/metaprogramming.h"
#include "cpp0x/type_traits"

namespace art {
  template <class T>
  class MixFilter;

  namespace detail {
    // Template metaprogramming.

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void startEvent()?
    template <typename T, void (T::*)()> struct startEvent_function;

    template <typename T> struct do_not_call_startEvent {
    public:
      void operator()(T &t) {}
    };

    template <typename T> struct call_startEvent {
    public:
      void operator()(T &t) { t.startEvent(); }
    };

    template <typename T>
    no_tag
    has_startEvent_helper(...);

    template <typename T>
    yes_tag
    has_startEvent_helper(startEvent_function<T, &T::startEvent> * dummy);

    template <typename T>
    struct has_startEvent {
      static bool const value =
        sizeof(has_startEvent_helper<T>(0)) == sizeof(yes_tag);
    };
    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void
    // processEventIDs(EventIDSequence const &)?
    template <typename T, void (T::*)(EventIDSequence const &)> struct processEventIDs_function;

    template <typename T> struct do_not_call_processEventIDs {
    public:
      void operator()(T &, EventIDSequence const &) {}
    };

    template <typename T> struct call_processEventIDs {
    public:
      void operator()(T &t, EventIDSequence const &seq) { t.processEventIDs(seq); }
    };

    template <typename T>
    no_tag
    has_processEventIDs_helper(...);

    template <typename T>
    yes_tag
    has_processEventIDs_helper(processEventIDs_function<T, &T::processEventIDs> * dummy);

    template <typename T>
    struct has_processEventIDs {
      static bool const value =
        sizeof(has_processEventIDs_helper<T>(0)) == sizeof(yes_tag);
    };
    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void finalizeEvent(Event&)?
    template <typename T, void (T::*)(Event &)> struct finalizeEvent_function;

    template <typename T> struct do_not_call_finalizeEvent {
    public:
      void operator()(T &, Event &) {}
    };

    template <typename T> struct call_finalizeEvent {
    public:
      void operator()(T &t, Event &e) { t.finalizeEvent(e); }
    };

    template <typename T>
    no_tag
    has_finalizeEvent_helper(...);

    template <typename T>
    yes_tag
    has_finalizeEvent_helper(finalizeEvent_function<T, &T::finalizeEvent> * dummy);

    template <typename T>
    struct has_finalizeEvent {
      static bool const value =
        sizeof(has_finalizeEvent_helper<T>(0)) == sizeof(yes_tag);
    };
    ////////////////////////////////////////////////////////////////////

  } // detail namespace

} // art namespace

template <class T>
class art::MixFilter : public art::EDFilter {
public:
  typedef T MixDetail;
  explicit MixFilter(fhicl::ParameterSet const &p);

  virtual void beginJob();
  virtual bool filter(art::Event &e);

private:
  MixHelper helper_;
  MixDetail detail_;
};

template <class T>
art::MixFilter<T>::MixFilter(fhicl::ParameterSet const &p)
  :
  EDFilter(),
  helper_((createEngine(get_seed_value(p)),p), *this), // See note below
  detail_(p, helper_)
{
  // Note that the random number engine is created in the initializer
  // list as part of a comma-separated argument bundle to the
  // constructor of MixHelper. This enables the engine to be obtained
  // by the helper and or detail objects in their constructors via a
  // service handle to the random number generator service. Do NOT
  // remove the seemingly-superfluous parentheses.
}

template <class T>
void
art::MixFilter<T>::beginJob() {
  helper_.postRegistrationInit();
}

template <class T>
bool
art::MixFilter<T>::filter(art::Event &e) {
  // 1. Call detail object's startEvent() if it exists.
  typename std::conditional<detail::has_startEvent<T>::value,
    detail::call_startEvent<T>,
    detail::do_not_call_startEvent<T> >::type maybe_call_startEvent;
  maybe_call_startEvent(detail_);

  // 2. Ask detail object how many events to read.
  size_t nSecondaries = detail_.nSecondaries();

  // 3. Decide which events we're reading and prime the event tree cache.
  EntryNumberSequence enSeq;
  EventIDSequence eIDseq;
  enSeq.reserve(nSecondaries);
  eIDseq.reserve(nSecondaries);
  if (!helper_.generateEventSequence(nSecondaries, enSeq, eIDseq)) {
    throw Exception(errors::FileReadError)
      << "Insufficient secondary events available to mix.\n";
  }

  // 4. Give the event ID sequence to the detail object.
  typename std::conditional<detail::has_processEventIDs<T>::value,
    detail::call_processEventIDs<T>,
    detail::do_not_call_processEventIDs<T> >::type maybe_call_processEventIDs;
  maybe_call_processEventIDs(detail_, eIDseq);

  // 3. Make the MixHelper read info into all the products, invoke the
  // mix functions and put the products into the event.
  helper_.mixAndPut(enSeq, e);

  // 4. Call detail object's finalizeEvent() if it exists.
  typename std::conditional<detail::has_finalizeEvent<T>::value,
    detail::call_finalizeEvent<T>,
    detail::do_not_call_finalizeEvent<T> >::type
    maybe_call_finalizeEvent;
  maybe_call_finalizeEvent(detail_, e);
  return true;
}

#endif /* art_Framework_Modules_MixFilter_h */

// Local Variables:
// mode: c++
// End:
