#include "art/Framework/Art/novaapp.h"

#include "art/Framework/Art/BasicOutputOptionsHandler.h"
#include "art/Framework/Art/BasicSourceOptionsHandler.h"
#include "art/Framework/Art/DebugOptionsHandler.h"
#include "art/Framework/Art/FileCatalogOptionsHandler.h"
#include "art/Framework/Art/OptionsHandlers.h"
#include "art/Framework/Art/run_art.h"
#include "art/Utilities/FirstAbsoluteOrLookupWithDotPolicy.h"

namespace  bpo = boost::program_options;

int novaapp(int argc, char * argv[])
{
  char const * fhicl_env = getenv("FHICL_FILE_PATH");
  std::string search_path;
  if (fhicl_env == nullptr) {
    std::cerr
        << "Expected environment variable FHICL_FILE_PATH is "
        << "missing or empty: using \".\"\n";
    search_path = ".:";
  }
  else {
    search_path = std::string(fhicl_env) + ":";
  }
  art::FirstAbsoluteOrLookupWithDotPolicy lookupPolicy(search_path);
  // Empty options_description.
  bpo::options_description all_desc;
  // Create and store options handlers.
  art::OptionsHandlers handlers;
  handlers.reserve(4); // -ish.
  // Add new handlers here. Do *not* add a BasicOptionsHandler: it will
  // be done for you.
  handlers.emplace_back(new art::BasicSourceOptionsHandler(all_desc));
  handlers.emplace_back(new art::BasicOutputOptionsHandler(all_desc));
  handlers.emplace_back(new art::DebugOptionsHandler(all_desc));
  handlers.emplace_back(new art::FileCatalogOptionsHandler(all_desc));
  return art::run_art(argc, argv, all_desc, lookupPolicy, std::move(handlers));
}
