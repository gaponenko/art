#ifndef art_Framework_Core_Schedule_h
#define art_Framework_Core_Schedule_h
// vim: set sw=2 expandtab :

// ======================================================================
// Schedule
//
//  A schedule is a sequence of trigger paths. After construction, events
//  can be fed to the object and passed through all the modules in the
//  schedule. All accounting about processing of events by modules and
//  paths is contained here or in object held by containment.
//
//  The trigger results producer is generated and managed here. This
//  class also manages calls to endjob and beginjob.
//
// A TriggerResults object will always be inserted into the event for
// any schedule. The producer of the TriggerResults EDProduct is always
// the last module in the trigger path. The TriggerResultInserter is
// given a fixed label of "TriggerResults".
//
// Processing of an event happens by pushing the event through the
// Paths. The scheduler performs the reset() on each of the workers
// independent of the Path objects.
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Path.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/trim.h"
#include "fhiclcpp/ParameterSet.h"

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace art {
  class Schedule {
  public:
    Schedule(ScheduleID,
             PathManager&,
             ActionTable const&,
             std::unique_ptr<Worker> triggerResultsInserter);

    // Disable copy/move operations
    Schedule(Schedule const&) = delete;
    Schedule(Schedule&&) = delete;
    Schedule& operator=(Schedule const&) = delete;
    Schedule& operator=(Schedule&&) = delete;

    // API presented to EventProcessor
    void process(Transition, Principal&);
    void process_event(tbb::task* endPathTask,
                       tbb::task* eventLoopTask,
                       EventPrincipal&);
    void beginJob();
    void endJob();
    void respondToOpenInputFile(FileBlock const&);
    void respondToCloseInputFile(FileBlock const&);
    void respondToOpenOutputFiles(FileBlock const&);
    void respondToCloseOutputFiles(FileBlock const&);

    // Tasking Structure
    void pathsDoneTask(tbb::task* endPathTask,
                       tbb::task* eventLoopTask,
                       EventPrincipal&,
                       std::exception_ptr const*);

    // Implementation details.
    void process_event_pathsDone(tbb::task* endPathTask,
                                 tbb::task* eventLoopTask,
                                 EventPrincipal&);

  private:
    bool skipNonReplicated_(Worker const&);

    // const after ctor.
    ScheduleContext const sc_;
    std::atomic<ActionTable const*> actionTable_;
    std::atomic<PathsInfo*> triggerPathsInfo_;
    std::unique_ptr<Worker> results_inserter_;

    // Dynamic: cause an error if more than one thread processes an
    // event.
    std::atomic<int> runningWorkerCnt_{};
  };
} // namespace art

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_Schedule_h */
