#ifndef art_Framework_Core_detail_SharedModule_h
#define art_Framework_Core_detail_SharedModule_h
// vim: set sw=2 expandtab :

#include "art/Utilities/SharedResource.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <memory>
#include <set>
#include <string>
#include <type_traits>

namespace art::detail {
  class SharedResources;

  class SharedModule {
  public:
    SharedModule();
    explicit SharedModule(std::string const& moduleLabel);

    hep::concurrency::SerialTaskQueueChain* serialTaskQueueChain() const;
    std::set<std::string> const& sharedResources() const;

    void createQueues(SharedResources const& resources);

  protected:
    template <BranchType BT = InEvent, typename... T>
    void serialize(T const&...);

    template <BranchType BT = InEvent, typename... T>
    void serializeExternal(T const&...);

    template <BranchType BT = InEvent>
    void
    async()
    {
      static_assert(
        BT == InEvent,
        "async is currently supported only for the 'InEvent' level.");
      asyncDeclared_ = true;
    }

  private:
    void implicit_serialize();
    void serialize_for(std::string const& name);

    template <typename... T>
    void
    serialize_for_resource(T const&... t)
    {
      static_assert(
        std::conjunction_v<std::is_same<detail::SharedResource_t, T>...>);
      if (sizeof...(t) == 0) {
        implicit_serialize();
      } else {
        (serialize_for(t.name), ...);
      }
    }

    template <typename... T>
    void
    serialize_for_external_resource(T const&... t)
    {
      static_assert(std::conjunction_v<std::is_same<std::string, T>...>);
      if (sizeof...(t) == 0) {
        implicit_serialize();
      } else {
        (serialize_for(t), ...);
      }
    }

    std::string moduleLabel_;
    std::set<std::string> resourceNames_{};
    bool asyncDeclared_{false};
    std::unique_ptr<hep::concurrency::SerialTaskQueueChain> chain_{nullptr};
  };

  template <BranchType, typename... T>
  void
  SharedModule::serialize(T const&... resources)
  {
    serialize_for_resource(resources...);
  }

  template <BranchType, typename... T>
  void
  SharedModule::serializeExternal(T const&... resources)
  {
    serialize_for_external_resource(resources...);
  }
}

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_detail_SharedModule_h */
