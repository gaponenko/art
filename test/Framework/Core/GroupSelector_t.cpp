#include <cassert>
#include <iostream>
#include <string>
#include <vector>


#include "art/Framework/Core/GroupSelectorRules.h"
#include "art/Framework/Core/GroupSelector.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/Exception.h"

typedef std::vector<art::BranchDescription const*> VCBDP;

void apply_gs(art::GroupSelector const& gs,
	      VCBDP const&  allbranches,
	      std::vector<bool>& results)
{
  VCBDP::const_iterator it  = allbranches.begin();
  VCBDP::const_iterator end = allbranches.end();
  for (; it != end; ++it) results.push_back(gs.selected(**it));
}

int doTest(fhicl::ParameterSet const& params,
	     char const* testname,
	     VCBDP const&  allbranches,
	     std::vector<bool>& expected)
{
  art::GroupSelectorRules gsr(params, "outputCommands", testname);
  art::GroupSelector gs;
  gs.initialize(gsr, allbranches);
  std::cout << "GroupSelector from "
	    << testname
	    << ": "
	    << gs
	    << std::endl;

  std::vector<bool> results;
  apply_gs(gs, allbranches, results);

  int rc = 0;
  if (expected != results) rc = 1;
  if (rc == 1) std::cerr << "FAILURE: " << testname << '\n';
  std::cout << "---" << std::endl;
  return rc;
}

int work()
{
  art::ModuleDescription mod;
  mod.parameterSetID_ = fhicl::ParameterSet().id();

  int rc = 0;
  // We pretend to have one module, with two products. The products
  // are of the same and, type differ in instance name.
  std::set<fhicl::ParameterSetID> psetsA;
  fhicl::ParameterSet modAparams;
  modAparams.put<int>("i", 2112);
  modAparams.put<std::string>("s", "hi");
  psetsA.insert(modAparams.id());

  //art::BranchDescription b1(art::InEvent, "modA", "PROD", "UglyProdTypeA", "ProdTypeA", "i1", md, psetsA);
  //art::BranchDescription b2(art::InEvent, "modA", "PROD", "UglyProdTypeA", "ProdTypeA", "i2", md, psetsA);
  art::BranchDescription b1(art::InEvent, "modA", "PROD", "UglyProdTypeA", "ProdTypeA", "i1",
			    mod);
  art::BranchDescription b2(art::InEvent, "modA", "PROD", "UglyProdTypeA", "ProdTypeA", "i2",
			    mod);

  // Our second pretend module has only one product, and gives it no
  // instance name.
  std::set<fhicl::ParameterSetID> psetsB;
  fhicl::ParameterSet modBparams;
  modBparams.put<double>("d", 2.5);
  psetsB.insert(modBparams.id());

  //art::BranchDescription b3(art::InEvent, "modB", "HLT", "UglyProdTypeB", "ProdTypeB", "", md, psetsB);
  art::BranchDescription b3(art::InEvent, "modB", "HLT", "UglyProdTypeB", "ProdTypeB", "",
			    mod);

  // Our third pretend is like modA, except it hass processName_ of
  // "USER"

  //art::BranchDescription b4(art::InEvent, "modA", "USER", "UglyProdTypeA", "ProdTypeA", "i1", md, psetsA);
  //art::BranchDescription b5(art::InEvent, "modA", "USER", "UglyProdTypeA", "ProdTypeA", "i2", md, psetsA);

  art::BranchDescription b4(art::InEvent, "modA", "USER", "UglyProdTypeA",
			    "ProdTypeA", "i1", mod);
  art::BranchDescription b5(art::InEvent, "modA", "USER", "UglyProdTypeA",
			    "ProdTypeA", "i2", mod);

  // These are pointers to all the branches that are available. In a
  // framework program, these would come from the ProductRegistry
  // which is used to initialze the OutputModule being configured.
  VCBDP allbranches;
  allbranches.push_back(&b1); // ProdTypeA_modA_i1. (PROD)
  allbranches.push_back(&b2); // ProdTypeA_modA_i2. (PROD)
  allbranches.push_back(&b3); // ProdTypeB_modB_HLT. (no instance name)
  allbranches.push_back(&b4); // ProdTypeA_modA_i1_USER.
  allbranches.push_back(&b5); // ProdTypeA_modA_i2_USER.

  // Test default parameters
  {
    bool wanted[] = { true, true, true, true, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));
    fhicl::ParameterSet noparams;

    rc += doTest(noparams, "default parameters", allbranches, expected);
  }

  // Keep all branches with instance name i2.
  {
    bool wanted[] = { false, true, false, false, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet keep_i2;
    std::string const keep_i2_rule = "keep *_*_i2_*";
    std::vector<std::string> cmds;
    cmds.push_back(keep_i2_rule);
    keep_i2.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(keep_i2, "keep_i2 parameters", allbranches, expected);
  }

  // Drop all branches with instance name i2.
  {
    bool wanted[] = { true, false, true, true, false };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet drop_i2;
    std::string const drop_i2_rule1 = "keep *";
    std::string const drop_i2_rule2 = "drop *_*_i2_*";
    std::vector<std::string> cmds;
    cmds.push_back(drop_i2_rule1);
    cmds.push_back(drop_i2_rule2);
    drop_i2.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(drop_i2, "drop_i2 parameters", allbranches, expected);
  }

  // Now try dropping all branches with product type "foo". There are
  // none, so all branches should be written.
  {
    bool wanted[] = { true, true, true, true, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet drop_foo;
    std::string const drop_foo_rule1 = "keep *_*_*_*"; // same as "keep *"
    std::string const drop_foo_rule2 = "drop foo_*_*_*";
    std::vector<std::string> cmds;
    cmds.push_back(drop_foo_rule1);
    cmds.push_back(drop_foo_rule2);
    drop_foo.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(drop_foo, "drop_foo parameters", allbranches, expected);
  }

  // Now try dropping all branches with product type "ProdTypeA".
  {
    bool wanted[] = { false, false, true, false, false };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet drop_ProdTypeA;
    std::string const drop_ProdTypeA_rule1 = "keep *";
    std::string const drop_ProdTypeA_rule2 = "drop ProdTypeA_*_*_*";
    std::vector<std::string> cmds;
    cmds.push_back(drop_ProdTypeA_rule1);
    cmds.push_back(drop_ProdTypeA_rule2);
    drop_ProdTypeA.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(drop_ProdTypeA,
		 "drop_ProdTypeA",
		 allbranches, expected);
  }

  // Keep only branches with instance name 'i1', from Production.
  {
    bool wanted[] = { true, false, false, false, false };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet keep_i1prod;
    std::string const keep_i1prod_rule = "keep *_*_i1_PROD";
    std::vector<std::string> cmds;
    cmds.push_back(keep_i1prod_rule);
    keep_i1prod.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(keep_i1prod,
		 "keep_i1prod",
		 allbranches, expected);
  }

  // First say to keep everything,  then  to drop everything, then  to
  // keep it again. The end result should be to keep everything.
  {
    bool wanted[] = { true, true, true, true, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet indecisive;
    std::string const indecisive_rule1 = "keep *";
    std::string const indecisive_rule2 = "drop *";
    std::string const indecisive_rule3 = "keep *";
    std::vector<std::string> cmds;
    cmds.push_back(indecisive_rule1);
    cmds.push_back(indecisive_rule2);
    cmds.push_back(indecisive_rule3);
    indecisive.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(indecisive,
		 "indecisive",
		 allbranches, expected);
  }

  // Keep all things, bu drop all things from modA, but later keep all
  // things from USER.
  {
    bool wanted[] = { false, false, true, true, true };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet params;
    std::string const rule1 = "keep *";
    std::string const rule2 = "drop *_modA_*_*";
    std::string const rule3 = "keep *_*_*_USER";
    std::vector<std::string> cmds;
    cmds.push_back(rule1);
    cmds.push_back(rule2);
    cmds.push_back(rule3);
    params.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(params,
		 "drop_modA_keep_user",
		 allbranches, expected);
  }

  // Exercise the wildcards * and ?
  {
    bool wanted[] = { true, true, true, false, false };
    std::vector<bool> expected(wanted, wanted+sizeof(wanted)/sizeof(bool));

    fhicl::ParameterSet params;
    std::string const rule1 = "drop *";
    std::string const rule2 = "keep Pr*A_m?dA_??_P?O*";
    std::string const rule3 = "keep *?*?***??*????*?***_??***?__*?***T";
    std::vector<std::string> cmds;
    cmds.push_back(rule1);
    cmds.push_back(rule2);
    cmds.push_back(rule3);
    params.put<std::vector<std::string> >("outputCommands", cmds);

    rc += doTest(params,
		 "excercise wildcards1",
		 allbranches, expected);
  }

  {
    // Now try an illegal specification: not starting with 'keep' or 'drop'
    try {
	fhicl::ParameterSet bad;
	std::string const bad_rule = "beep *_*_i2_*";
	std::vector<std::string> cmds;
	cmds.push_back(bad_rule);
	bad.put<std::vector<std::string> >("outputCommands", cmds);
	art::GroupSelectorRules gsr(bad, "outputCommands", "GroupSelectorTest");
	art::GroupSelector gs;
        gs.initialize(gsr, allbranches);
	std::cerr << "Failed to throw required exception\n";
	rc += 1;
    }
    catch (art::Exception const& x) {
	// OK, we should be here... now check exception type
	assert (x.categoryCode() == art::errors::Configuration);
    }
    catch (...) {
	std::cerr << "Wrong exception type\n";
	rc += 1;
    }
  }

  return rc;
}

int main()
{
  int rc = 0;
  try
    {
      rc = work();
    }
  catch (art::Exception& x)
    {
      std::cerr << "art::Exception caught:\n" << x << '\n';
      rc = 1;
    }
  catch (...)
    {
      std::cerr << "Unknown exception caught\n";
      rc = 2;
    }
  return rc;
}


