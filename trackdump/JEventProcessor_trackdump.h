//
//    File: JEventProcessor_trackdump.h
//

#ifndef _JEventProcessor_trackdump_
#define _JEventProcessor_trackdump_

#include <JANA/JEventProcessor.h>


class JEventProcessor_trackdump:public jana::JEventProcessor
{
 public:
  JEventProcessor_trackdump();
  ~JEventProcessor_trackdump();
  const char* className(void) {
    return "JEventProcessor_trackdump";
  }

 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_trackdump_
