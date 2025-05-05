//
//    File: JEventProcessor_TAGM_tracer.h
//

#ifndef _JEventProcessor_TAGM_tracer_
#define _JEventProcessor_TAGM_tracer_

#include <JANA/JEventProcessor.h>
#include <TTree.h>


class JEventProcessor_TAGM_tracer : public JEventProcessor {
 public:
  JEventProcessor_TAGM_tracer();
  ~JEventProcessor_TAGM_tracer();
  const char* className(void) {return "JEventProcessor_TAGM_tracer"; }

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
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
};

#endif // _JEventProcessor_TAGM_tracer_
