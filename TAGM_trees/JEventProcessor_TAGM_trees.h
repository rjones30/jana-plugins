
#ifndef _JEventProcessor_TAGM_trees_
#define _JEventProcessor_TAGM_trees_

#include <JANA/JEventProcessor.h>


class JEventProcessor_TAGM_trees : public JEventProcessor {
 public:
  JEventProcessor_TAGM_trees();
  ~JEventProcessor_TAGM_trees();
  const char* className(void){return "JEventProcessor_TAGM_trees";}

  TTree *fTree;

  // Declaration of leaf types
  Int_t runno;
  Int_t eventno;
  Int_t trigger;
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
  TBranch *b_trigger;
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
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
};

#endif // _JEventProcessor_TAGM_trees_
