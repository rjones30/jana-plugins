
#ifndef _JEventProcessor_TAGM_traw_
#define _JEventProcessor_TAGM_traw_

#include <JANA/JEventProcessor.h>


class JEventProcessor_TAGM_traw:public jana::JEventProcessor{
 public:
  JEventProcessor_TAGM_traw();
  ~JEventProcessor_TAGM_traw();
  const char* className(void){return "JEventProcessor_TAGM_traw";}

  TTree *fTraw;

  // Declaration of leaf types
  Int_t runno;
  Int_t eventno;
  Int_t nhits;
  Int_t nraw;
  Int_t row;
  Int_t col;
  Int_t npulse;
  Int_t pi[4];
  Int_t pt[4];
  Int_t peak[4];
  Int_t npi[4];
  Int_t qf[4];
  Int_t ped;
  Int_t nped;
  Int_t nsamples;
  Int_t samples[2048];
  Int_t invalid_samples;
  Int_t overflow_samples;
  Double_t rftime;

  // List of branches
  TBranch *b_runno;
  TBranch *b_eventno;
  TBranch *b_nhits;
  TBranch *b_nraw;
  TBranch *b_row;
  TBranch *b_col;
  TBranch *b_npulse;
  TBranch *b_pi;
  TBranch *b_pt;
  TBranch *b_peak;
  TBranch *b_npi;
  TBranch *b_qf;
  TBranch *b_ped;
  TBranch *b_nped;
  TBranch *b_nsamples;
  TBranch *b_samples;
  TBranch *b_invalid_samples;
  TBranch *b_overflow_samples;
  TBranch *b_rftime;

 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_TAGM_traw_
