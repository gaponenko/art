#ifndef art_Framework_Core_PathManager_h
#define art_Framework_Core_PathManager_h
// vim: set sw=2 expandtab :

// ======================================================================
// PathManager.
//
// Class to handle the processing of the configuration of modules in
// art, including the creation of paths and construction of modules as
// appropriate.
//
// Intended to be constructed early, prior to services, since
// TriggerNamesService will need some of the information herein at
// construction time.
// ======================================================================

#include "art/Framework/Core/PathsInfo.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/detail/EnabledModules.h"
#include "art/Framework/Core/detail/ModuleConfigInfo.h"
#include "art/Framework/Core/detail/ModuleKeyAndType.h"
#include "art/Framework/Core/detail/graph_type_aliases.h"
#include "art/Persistency/Provenance/ModuleType.h"
#include "art/Utilities/PerScheduleContainer.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "cetlib/LibraryManager.h"
#include "fhiclcpp/ParameterSet.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace art {

  class ActionTable;
  class ActivityRegistry;
  class GlobalTaskGroup;
  class ModuleBase;
  class UpdateOutputCallbacks;

  namespace detail {
    class SharedResources;
  }

  class PathManager {
  public:
    PathManager(fhicl::ParameterSet const& procPS,
                UpdateOutputCallbacks& preg,
                ProductDescriptions& productsToProduce,
                ActionTable const& exceptActions,
                ActivityRegistry const& areg,
                detail::EnabledModules const& enabled_modules);

    PathManager(PathManager const&) = delete;
    PathManager(PathManager&&) = delete;
    PathManager& operator=(PathManager const&) = delete;
    PathManager& operator=(PathManager&&) = delete;

    std::vector<PathSpec> triggerPathSpecs() const;

    void createModulesAndWorkers(
      GlobalTaskGroup& task_group,
      detail::SharedResources& resources,
      std::vector<std::string> const& producing_services);
    PathsInfo& triggerPathsInfo(ScheduleID);
    PerScheduleContainer<PathsInfo>& triggerPathsInfo();
    PathsInfo& endPathInfo(ScheduleID);
    PerScheduleContainer<PathsInfo>& endPathInfo();

  private:
    struct ModulesByThreadingType {
      std::map<module_label_t, std::shared_ptr<ModuleBase>> shared{};
      std::map<module_label_t,
               PerScheduleContainer<std::shared_ptr<ModuleBase>>>
        replicated{};
    };

    std::map<std::string, detail::ModuleConfigInfo> moduleInformation_(
      detail::EnabledModules const& enabled_modules) const;

    ModulesByThreadingType makeModules_(ScheduleID::size_type n);
    using maybe_module_t = std::variant<ModuleBase*, std::string>;
    maybe_module_t makeModule_(fhicl::ParameterSet const& module_pset,
                               ModuleDescription const& md,
                               ScheduleID) const;
    std::vector<WorkerInPath> fillWorkers_(
      PathContext const& pc,
      std::vector<WorkerInPath::ConfigInfo> const& wci_list,
      ModulesByThreadingType const& modules,
      std::map<std::string, std::shared_ptr<Worker>>& workers,
      GlobalTaskGroup& task_group,
      detail::SharedResources& resources);
    std::shared_ptr<Worker> makeWorker_(ModulesByThreadingType const& modules,
                                        ModuleDescription const& md,
                                        WorkerParams const& wp);
    ModuleType loadModuleType_(std::string const& lib_spec) const;
    ModuleThreadingType loadModuleThreadingType_(
      std::string const& lib_spec) const;

    // Module-graph implementation
    detail::collection_map_t getModuleGraphInfoCollection_(
      std::vector<std::string> const& producing_services);
    void fillModuleOnlyDeps_(
      std::string const& path_name,
      detail::configs_t const& worker_configs,
      std::map<std::string, std::set<ProductInfo>> const& produced_products,
      std::map<std::string, std::set<std::string>> const& viewable_products,
      detail::collection_map_t& info_collection) const;
    void fillSelectEventsDeps_(detail::configs_t const& worker_configs,
                               detail::collection_map_t& info_collection) const;

    std::vector<std::string> triggerPathNames_() const;
    std::vector<std::string> prependedTriggerPathNames_() const;

    // Member Data
    UpdateOutputCallbacks& outputCallbacks_;
    ActionTable const& exceptActions_;
    ActivityRegistry const& actReg_;
    fhicl::ParameterSet procPS_;
    art::detail::module_entries_for_ordered_path_t triggerPathSpecs_;
    PerScheduleContainer<PathsInfo> triggerPathsInfo_;
    PerScheduleContainer<PathsInfo> endPathInfo_;
    ProductDescriptions& productsToProduce_;

    cet::LibraryManager lm_{Suffixes::module()};
    //  The following data members are only needed to delay the
    //  creation of modules until after the service system has
    //  started.  We can move them back to the ctor once that is
    //  fixed.
    std::string processName_{};
    std::map<std::string, detail::ModuleConfigInfo> allModules_{};
    art::detail::paths_to_modules_t protoTrigPathLabels_{};
    art::detail::configs_t protoEndPathLabels_{};
  };
} // namespace art

#endif /* art_Framework_Core_PathManager_h */

// Local Variables:
// mode: c++
// End:
