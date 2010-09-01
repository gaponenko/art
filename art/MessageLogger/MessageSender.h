#ifndef FWCore_MessageLogger_MessageSender_h
#define FWCore_MessageLogger_MessageSender_h

#include "art/MessageLogger/ELstring.h"
#include "art/MessageLogger/ELseverityLevel.h"
#include "art/MessageLogger/ErrorObj.h"

#include <map>

// Change log
//
//  1  mf 8/25/08	error summary information for
//			LoggedErrorsSummary()
//

namespace edm
{

typedef std::pair<std::string, std::string> ErrorSummaryMapKey;
typedef std::map<ErrorSummaryMapKey, unsigned int>::iterator
						ErrorSummaryMapIterator;

class MessageSender
{
public:
  // ---  birth/death:
  MessageSender( ELseverityLevel const & sev,
  		 ELstring const & id,
		 bool verbatim );
  MessageSender( ELseverityLevel const & sev,
  		 ELstring const & id );
  ~MessageSender();

  // ---  stream out the next part of a message:
  template< class T >
    MessageSender &
    operator<< ( T const & t )
  {
    (*errorobj_p) << t;
    return *this;
  }

  // static data:						// change log 1
  static bool errorSummaryIsBeingKept;
  static bool freshError;
  static std::map<ErrorSummaryMapKey, unsigned int> errorSummaryMap;

private:
  // no copying:
  MessageSender( MessageSender const & );
  void operator = ( MessageSender const & );

  // data:
  ErrorObj *  errorobj_p;

};  // MessageSender


}  // namespace edm


#endif  // FWCore_MessageLogger_MessageSender_h
