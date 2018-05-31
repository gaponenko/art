#ifndef art_Framework_Core_detail_ModuleGraphInfoMap_h
#define art_Framework_Core_detail_ModuleGraphInfoMap_h

#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Principal/Consumer.h"
#include "art/Persistency/Provenance/ModuleType.h"

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace art {
  namespace detail {

    struct ModuleGraphInfo {
      ModuleType module_type{ModuleType::non_art};
      std::set<ProductInfo> produced_products{};
      std::set<ProductInfo> consumed_products{};
      // 'select_events' is only used for analyzers and output
      // modules.
      std::set<std::string> select_events{};
      std::set<std::string> paths{};
    };

    std::ostream& operator<<(std::ostream& os, ModuleGraphInfo const& info);

    using path_name_t = std::string;
    using module_name_t = std::string;
    using names_t = std::vector<std::string>;
    using configs_t = std::vector<WorkerInPath::ConfigInfo>;
    using name_set_t = std::set<std::string>;
    using paths_to_modules_t = std::map<path_name_t, configs_t>;
    using collection_map_t = std::map<module_name_t, ModuleGraphInfo>;
    using collection_t = std::vector<collection_map_t::value_type>;
    using distance_t = collection_t::difference_type;

    class ModuleGraphInfoMap {
    public:
      explicit ModuleGraphInfoMap(collection_map_t const& paths);

      auto const&
      name(std::size_t const i) const
      {
        return modules_[i].first;
      }

      auto const&
      info(std::size_t const i) const
      {
        return modules_[i].second;
      }

      auto
      size() const
      {
        return modules_.size();
      }

      auto
      begin() const
      {
        return modules_.begin();
      }

      auto
      end() const
      {
        return modules_.end();
      }

      auto info(module_name_t const& name) const -> ModuleGraphInfo const&;
      auto vertex_index(module_name_t const& name) const -> distance_t;

    private:
      collection_t const modules_;
      collection_t::const_iterator const begin_;
      collection_t::const_iterator const end_;
    };
  }
}

#endif /* art_Framework_Core_detail_ModuleGraphInfoMap_h */

// Local Variables:
// mode: c++
// End: