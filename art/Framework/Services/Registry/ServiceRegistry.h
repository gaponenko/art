#ifndef ServiceRegistry_ServiceRegistry_h
#define ServiceRegistry_ServiceRegistry_h

//
// Package:     ServiceRegistry
// Class  :     ServiceRegistry
//
/**\class ServiceRegistry ServiceRegistry.h FWCore/ServiceRegistry/interface/ServiceRegistry.h

 Description: Manages the 'thread specific' instance of Services

*/


#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Framework/Services/Registry/ServiceLegacy.h"
#include "art/Framework/Services/Registry/ServicesManager.h"

#include "fhiclcpp/ParameterSet.h"


namespace art {
   class FwkImpl;

   namespace serviceregistry {
      template< typename T> class ServiceWrapper;
   }

   class ServiceRegistry
   {
   public:

      class Operate {
        public:
         Operate(const ServiceToken& iToken) :
         oldToken_(ServiceRegistry::instance().setContext(iToken))
         {}
         ~Operate() {
            ServiceRegistry::instance().unsetContext(oldToken_);
         }

         //override operator new to stop use on heap?
        private:
         Operate(const Operate&); //stop default
         const Operate& operator=(const Operate&); //stop default
         ServiceToken oldToken_;
      };

      friend class art::FwkImpl;
      friend int main(int argc, char* argv[]);
      friend class Operate;

      virtual ~ServiceRegistry();

      // ---------- const member functions ---------------------
      template<class T> T& get() const 
	{
	  if(0==manager_.get()) // shared_ptr has null pointer
	    {
	      throw art::Exception(art::errors::NotFound,"Service")
		<<" no ServiceRegistry has been set for this thread";
            }
	  return manager_-> template get<T>();
	}

      template<class T>
         bool isAvailable() const {
            if(0 == manager_.get()) {
               throw art::Exception(art::errors::NotFound,"Service")
               <<" no ServiceRegistry has been set for this thread";
            }
            return manager_-> template isAvailable<T>();
         }
      /** The token can be passed to another thread in order to have the
         same services available in the other thread.
         */
      ServiceToken presentToken() const;
      // ---------- static member functions --------------------
      static ServiceRegistry& instance();

      // ---------- member functions ---------------------------


   public: // Made public (temporarily) at the request of Emilio Meschi.
      static ServiceToken createSet(const std::vector<fhicl::ParameterSet>&);
      static ServiceToken createSet(const std::vector<fhicl::ParameterSet>&,
                                    ServiceToken,
                                    serviceregistry::ServiceLegacy);
      /// create a service token that holds the service defined by iService
      template<class T>
         static ServiceToken createContaining(std::auto_ptr<T> iService){
            std::vector<fhicl::ParameterSet> config;
            boost::shared_ptr<serviceregistry::ServicesManager> manager( new serviceregistry::ServicesManager(config) );
            boost::shared_ptr<serviceregistry::ServiceWrapper<T> >
               wrapper(new serviceregistry::ServiceWrapper<T>(iService));
            manager->put(wrapper);
            return manager;
         }
      template<class T>
         static ServiceToken createContaining(std::auto_ptr<T> iService,
                                              ServiceToken iToken,
                                              serviceregistry::ServiceLegacy iLegacy){
            std::vector<fhicl::ParameterSet> config;
            boost::shared_ptr<serviceregistry::ServicesManager> manager( new serviceregistry::ServicesManager(iToken,
                                                                                                              iLegacy,
                                                                                                              config) );
            boost::shared_ptr<serviceregistry::ServiceWrapper<T> >
            wrapper(new serviceregistry::ServiceWrapper<T>(iService));
            manager->put(wrapper);
            return manager;
         }
      /// create a service token that holds the service held by iWrapper
      template<class T>
         static ServiceToken createContaining(boost::shared_ptr<serviceregistry::ServiceWrapper<T> > iWrapper) {
            std::vector<fhicl::ParameterSet> config;
            boost::shared_ptr<serviceregistry::ServicesManager> manager( new serviceregistry::ServicesManager(config) );
            manager->put(iWrapper);
            return manager;
         }
      template<class T>
         static ServiceToken createContaining(boost::shared_ptr<serviceregistry::ServiceWrapper<T> > iWrapper,
                                              ServiceToken iToken,
                                              serviceregistry::ServiceLegacy iLegacy){
            std::vector<fhicl::ParameterSet> config;
            boost::shared_ptr<serviceregistry::ServicesManager> manager( new serviceregistry::ServicesManager(iToken,
                                                                                                              iLegacy,
                                                                                                              config) );
            manager->put(iWrapper);
            return manager;
         }

private:

      //returns old token
      ServiceToken setContext(const ServiceToken& iNewToken);
      void unsetContext(const ServiceToken& iOldToken);

      ServiceRegistry();
      ServiceRegistry(const ServiceRegistry&); // stop default

      const ServiceRegistry& operator=(const ServiceRegistry&); // stop default

      // ---------- member data --------------------------------
      boost::shared_ptr<serviceregistry::ServicesManager> manager_;
   };

}  // namespace art

#endif
