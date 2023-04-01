
#ifndef _JEventProcessor_TAGM_trees_
#define _JEventProcessor_TAGM_trees_

#include <JANA/JEventProcessor.h>


class JEventProcessor_TAGM_trees:public jana::JEventProcessor{
 public:
  JEventProcessor_TAGM_trees();
  ~JEventProcessor_TAGM_trees();
  const char* className(void){return "JEventProcessor_TAGM_trees";}

  TTree *fTree;

  // Declaration of leaf types
  Int_t runno;
  Int_t eventno;
  Int_t row;
  Int_t col;
  Int_t pi;
  Int_t pt;
  Int_t ped;
  Int_t qf;
  Int_t npi;
  Int_t nped;
  Int_t peak;
  Double_t rftime;

  // List of branches
  TBranch *b_runno;
  TBranch *b_eventno;
  TBranch *b_row;
  TBranch *b_col;
  TBranch *b_pi;
  TBranch *b_pt;
  TBranch *b_ped;
  TBranch *b_qf;
  TBranch *b_npi;
  TBranch *b_nped;
  TBranch *b_peak;
  TBranch *b_rftime;

 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_TAGM_trees_
