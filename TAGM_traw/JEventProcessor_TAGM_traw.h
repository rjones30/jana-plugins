
#ifndef _JEventProcessor_TAGM_traw_
#define _JEventProcessor_TAGM_traw_

#include <JANA/JEventProcessor.h>


class JEventProcessor_TAGM_traw : public JEventProcessor {
 public:
  JEventProcessor_TAGM_traw();
  ~JEventProcessor_TAGM_traw();
  const char* className(void){return "JEventProcessor_TAGM_traw";}

  TTree *fTraw;

  // Declaration of leaf types
  Int_t runno;
  Int_t eventno;
  Int_t trigger;
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
  TBranch *b_trigger;
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
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
};

#endif // _JEventProcessor_TAGM_traw_
