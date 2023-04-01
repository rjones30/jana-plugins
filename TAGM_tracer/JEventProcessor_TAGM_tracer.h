//
//    File: JEventProcessor_TAGM_tracer.h
//

#ifndef _JEventProcessor_TAGM_tracer_
#define _JEventProcessor_TAGM_tracer_

#include <JANA/JEventProcessor.h>
#include <TTree.h>


class JEventProcessor_TAGM_tracer:public jana::JEventProcessor
{
 public:
  JEventProcessor_TAGM_tracer();
  ~JEventProcessor_TAGM_tracer();
  const char* className(void) {
    return "JEventProcessor_TAGM_tracer";
  }

  TTree *tevent;
  TTree *ttagm;
  TTree *tps;
  int tagm_row;
  int tagm_column;
  int tagm_tick;
  int tagm_rftick;
  int tagm_f1trig;
  double tagm_time;
  double tagm_trigtime;
  double tagm_rftime;
  double event_time;
  int psc_tick;
  double psc_time;

 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_TAGM_tracer_
