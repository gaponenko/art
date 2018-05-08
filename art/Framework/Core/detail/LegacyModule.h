#ifndef art_Framework_Core_detail_LegacyModule_h
#define art_Framework_Core_detail_LegacyModule_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/detail/EngineCreator.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Utilities/ScheduleID.h"

namespace art {
  namespace detail {

    class LegacyModule : public SharedModule, private EngineCreator {
    public:
      auto
      scheduleID() const
      {
        return scheduleID_;
      }

      void
      setScheduleID(ScheduleID const sid) noexcept
      {
        scheduleID_ = sid;
      }

      using base_engine_t = EngineCreator::base_engine_t;
      using seed_t = EngineCreator::seed_t;
      using label_t = EngineCreator::label_t;

      base_engine_t&
      createEngine(seed_t const seed)
      {
        return EngineCreator::createEngine(ScheduleID::first(), seed);
      }

      base_engine_t&
      createEngine(seed_t const seed, std::string const& kind_of_engine_to_make)
      {
        return EngineCreator::createEngine(
          ScheduleID::first(), seed, kind_of_engine_to_make);
      }

      base_engine_t&
      createEngine(seed_t seed,
                   std::string const& kind_of_engine_to_make,
                   label_t const& engine_label)
      {
        return EngineCreator::createEngine(
          ScheduleID::first(), seed, kind_of_engine_to_make, engine_label);
      }

      class ScheduleIDSentry;

    private:
      ScheduleID scheduleID_;
    };

    class LegacyModule::ScheduleIDSentry {
    public:
      ScheduleIDSentry(LegacyModule& mod, ScheduleID const sid) noexcept
        : mod_{mod}
      {
        mod_.setScheduleID(sid);
      }

      ~ScheduleIDSentry() noexcept { mod_.setScheduleID(ScheduleID{}); }

    private:
      LegacyModule& mod_;
    };
  }
}

  // Local Variables:
  // mode: c++
  // End:

#endif /* art_Framework_Core_detail_LegacyModule_h */
