#include "art/Framework/Core/detail/Producer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/detail/get_failureToPut_flag.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSetRegistry.h"

using namespace hep::concurrency;
using namespace std;

namespace art {
  namespace detail {

    Producer::Producer() = default;
    Producer::~Producer() noexcept = default;

    void
    Producer::doRespondToOpenInputFile(FileBlock const& fb)
    {
      respondToOpenInputFile(fb);
    }

    void
    Producer::respondToOpenInputFile(FileBlock const&)
    {}

    void
    Producer::doRespondToCloseInputFile(FileBlock const& fb)
    {
      respondToCloseInputFile(fb);
    }

    void
    Producer::respondToCloseInputFile(FileBlock const&)
    {}

    void
    Producer::doRespondToOpenOutputFiles(FileBlock const& fb)
    {
      respondToOpenOutputFiles(fb);
    }

    void
    Producer::respondToOpenOutputFiles(FileBlock const&)
    {}

    void
    Producer::doRespondToCloseOutputFiles(FileBlock const& fb)
    {
      respondToCloseOutputFiles(fb);
    }

    void
    Producer::respondToCloseOutputFiles(FileBlock const&)
    {}

    void
    Producer::doBeginJob()
    {
      setupQueues();
      failureToPutProducts(md_);
      beginJob();
    }

    void
    Producer::beginJob()
    {}

    void
    Producer::doEndJob()
    {
      endJob();
    }

    void
    Producer::endJob()
    {}

    bool
    Producer::doBeginRun(RunPrincipal& rp,
                         cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      Run r{rp, md_, RangeSet::forRun(rp.runID())};
      beginRun(r);
      r.DataViewImpl::movePutProductsToPrincipal(rp);
      return true;
    }

    void
    Producer::beginRun(Run&)
    {}

    bool
    Producer::doEndRun(RunPrincipal& rp,
                       cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      Run r{rp, md_, rp.seenRanges()};
      endRun(r);
      r.DataViewImpl::movePutProductsToPrincipal(rp);
      return true;
    }

    void
    Producer::endRun(Run&)
    {}

    bool
    Producer::doBeginSubRun(SubRunPrincipal& srp,
                            cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      SubRun sr{srp, md_, RangeSet::forSubRun(srp.subRunID())};
      beginSubRun(sr);
      sr.DataViewImpl::movePutProductsToPrincipal(srp);
      return true;
    }

    void
    Producer::beginSubRun(SubRun&)
    {}

    bool
    Producer::doEndSubRun(SubRunPrincipal& srp,
                          cet::exempt_ptr<CurrentProcessingContext const> cpc)
    {
      detail::CPCSentry sentry{*cpc};
      SubRun sr{srp, md_, srp.seenRanges()};
      endSubRun(sr);
      sr.DataViewImpl::movePutProductsToPrincipal(srp);
      return true;
    }

    void
    Producer::endSubRun(SubRun&)
    {}

    bool
    Producer::doEvent(EventPrincipal& ep,
                      ScheduleID const sid,
                      CurrentProcessingContext const* cpc,
                      std::atomic<size_t>& counts_run,
                      std::atomic<size_t>& counts_passed,
                      std::atomic<size_t>& /*counts_failed*/)
    {
      detail::CPCSentry sentry{*cpc};
      Event e{ep, md_};
      ++counts_run;
      produceWithScheduleID(e, sid);
      e.DataViewImpl::movePutProductsToPrincipal(
        ep, checkPutProducts_, &expectedProducts<InEvent>());
      ++counts_passed;
      return true;
    }

    void
    Producer::failureToPutProducts(ModuleDescription const& md)
    {
      auto const& mainID = md.mainParameterSetID();
      auto const& scheduler_pset =
        fhicl::ParameterSetRegistry::get(mainID).get<fhicl::ParameterSet>(
          "services.scheduler");
      auto const& module_pset =
        fhicl::ParameterSetRegistry::get(md.parameterSetID());
      checkPutProducts_ =
        detail::get_failureToPut_flag(scheduler_pset, module_pset);
    }

  } // namespace detail
} // namespace art