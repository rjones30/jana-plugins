//
//    File: JEventProcessor_realtime.h
//

#ifndef _JEventProcessor_realtime_
#define _JEventProcessor_realtime_

#include <JANA/JEventProcessor.h>
#include <TTAB/DTranslationTable.h>
#include <DAQ/DBeamCurrent_factory.h>

class JEventProcessor_realtime:public jana::JEventProcessor
{
 public:
   JEventProcessor_realtime();
   ~JEventProcessor_realtime();
   const char* className(void) {
     return "JEventProcessor_realtime";
   }

   int runno;
   int eventno;
   unsigned long int timestamp;
   unsigned long int epoch_reference;
   unsigned long int bctime;
   double beamcurrent;

   DBeamCurrent_factory *bc_factory;

 private:
   jerror_t init(void);
   jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);
   jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);
   jerror_t erun(void);
   jerror_t fini(void);
};

#endif // _JEventProcessor_realtime_
