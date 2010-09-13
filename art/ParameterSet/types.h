#ifndef FWCore_ParameterSet_types_h
#define FWCore_ParameterSet_types_h

// ----------------------------------------------------------------------
//
// declaration of type encoding/decoding functions
//
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// prolog


// ----------------------------------------------------------------------
// prerequisite source files and headers

#include "boost/cstdint.hpp"
#include "fhiclcpp/ParameterSet.h"

#include <string>
#include <vector>

// ----------------------------------------------------------------------
// contents

namespace edm
{
  //            destination    source

  // Bool
  bool  decode(bool        &, std::string const&);
  bool  encode(std::string &, bool);

  // vBool
  bool  decode(std::vector<bool> &, std::string       const&);
  bool  encode(std::string       &, std::vector<bool> const&);

  // Int32
  bool  decode(int         &, std::string const&);
  bool  encode(std::string &, int);

  // vInt32
  bool  decode(std::vector<int> &, std::string      const&);
  bool  encode(std::string      &, std::vector<int> const&);

  // Uint32
  bool  decode(unsigned    &, std::string const&);
  bool  encode(std::string &, unsigned);

  // vUint32
  bool  decode(std::vector<unsigned> &, std::string           const&);
  bool  encode(std::string           &, std::vector<unsigned> const&);

  // Int64
  bool  decode(boost::int64_t &, std::string const&);
  bool  encode(std::string    &, boost::int64_t);

  // vInt64
  bool  decode(std::vector<boost::int64_t> &, std::string      const&);
  bool  encode(std::string      &, std::vector<boost::int64_t> const&);

  // Uint64
  bool  decode(boost::uint64_t &, std::string const&);
  bool  encode(std::string     &, boost::uint64_t);

  // vUint64
  bool  decode(std::vector<boost::uint64_t> &, std::string           const&);
  bool  encode(std::string           &, std::vector<boost::uint64_t> const&);

  // Double
  bool  decode(double      &, std::string const&);
  bool  encode(std::string &, double);

  // vDouble
  bool  decode(std::vector<double> &, std::string         const&);
  bool  encode(std::string         &, std::vector<double> const&);

  // String
  bool  decode(std::string &, std::string const&);
  bool  encode(std::string &, std::string const&);

  // vString
  bool  decode(std::vector<std::string> &, std::string              const&);
  bool  encode(std::string              &, std::vector<std::string> const&);

  // FileInPath
  bool  decode(edm::FileInPath &, std::string const&);
  bool  encode(std::string     &, edm::FileInPath const&);

  // InputTag
  bool  decode(edm::InputTag &, std::string const&);
  bool  encode(std::string   &, edm::InputTag const&);

  // VInputTag
  bool  decode(std::vector<edm::InputTag>&, std::string const&);
  bool  encode(std::string &, std::vector<edm::InputTag> const&);

  // EventID
  bool  decode(edm::EventID&, std::string const&);
  bool  encode(std::string &, edm::EventID const&);

  // VEventID
  bool  decode(std::vector<edm::EventID>&, std::string const&);
  bool  encode(std::string &, std::vector<edm::EventID> const&);

  // SubRunID
  bool  decode(edm::SubRunID&, std::string const&);
  bool  encode(std::string &, edm::SubRunID const&);

  // VSubRunID
  bool  decode(std::vector<edm::SubRunID>&, std::string const&);
  bool  encode(std::string &, std::vector<edm::SubRunID> const&);

  // ParameterSet
  bool  decode(fhicl::ParameterSet &, std::string  const&);
  bool  encode(std::string  &, fhicl::ParameterSet const&);

  // vPSet
  bool  decode(std::vector<fhicl::ParameterSet> &, std::string const&);
  bool  encode(std::string &, std::vector<fhicl::ParameterSet> const&);

}  // namespace edm


// ----------------------------------------------------------------------
// epilog

#endif  // FWCore_ParameterSet_types_h
