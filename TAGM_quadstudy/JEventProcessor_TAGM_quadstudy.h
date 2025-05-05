// $Id$
//
//    File: JEventProcessor_TAGM_quadstudy.h
// Created: Sat May  7 10:03:37 EDT 2016
// Creator: jonesrt (on Linux gluey.phys.uconn.edu 2.6.32-573.22.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_TAGM_quadstudy_
#define _JEventProcessor_TAGM_quadstudy_

#include <JANA/JEventProcessor.h>
#include <TTree.h>

class JEventProcessor_TAGM_quadstudy : public JEventProcessor {
 public:
  JEventProcessor_TAGM_quadstudy();
  ~JEventProcessor_TAGM_quadstudy();
  const char* className(void){return "JEventProcessor_TAGM_quadstudy";}

    TTree *ttagm;

    int tagm_hits;
    int tagm_row[999];
    int tagm_column[999];
    float tagm_pintegral[999];
    float tagm_tadc[999];
    int ps_hits;
    float ps_Epair[999];
    float ps_tleft[999];
    float ps_tright[999];

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
};

#endif // _JEventProcessor_TAGM_quadstudy_

