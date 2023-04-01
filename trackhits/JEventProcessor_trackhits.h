//
//    File: JEventProcessor_trackhits.h
//

#ifndef _JEventProcessor_trackhits_
#define _JEventProcessor_trackhits_

#include <JANA/JEventProcessor.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TTree.h>


class JEventProcessor_trackhits:public jana::JEventProcessor
{
 public:
  JEventProcessor_trackhits();
  ~JEventProcessor_trackhits();
  const char* className(void) {
    return "JEventProcessor_trackhits";
  }

 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  TH2D *h2w;
  TH2D *h2t;
  TH1D *h1p;
  TTree *ptree;
  struct ptree_row {
    double x;
    double y;
    int layer;
  } ptreed;
};

#endif // _JEventProcessor_trackhits_
