//
//    File: JEventProcessor_FCAL_tracer.cc
//


#include <iostream>
#include <stdint.h>
#include <vector>
#include <TMath.h>

#include "JEventProcessor_TAGM_tracer.h"
#include <JANA/JApplication.h>
#include <JANA/Services/JLockService.h>


#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGMDigiHit.h>
#include <TAGGER/DTAGMTDCDigiHit.h>
#include <RF/DRFTDCDigiTime.h>
#include <PAIR_SPECTROMETER/DPSCTDCDigiHit.h>
#include <DAQ/DF1TDCTriggerTime.h>
#include <DAQ/DF1TDCHit.h>
#include <DAQ/DCODAROCInfo.h>
#include <GlueX.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

extern "C"{
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_TAGM_tracer());
  }
}

JEventProcessor_TAGM_tracer::JEventProcessor_TAGM_tracer()
 : tevent(0), ttagm(0), tps(0)
{
}


JEventProcessor_TAGM_tracer::~JEventProcessor_TAGM_tracer() {
}

void JEventProcessor_TAGM_tracer::Init() {

  // lock all root operations
  auto app = GetApplication();
  auto lock_svc = app->GetService<JLockService>();
  lock_svc->RootWriteLock();

  ttagm = new TTree("ttagm", "tagm hit times");
  ttagm->Branch("row", &tagm_row, "row/I");
  ttagm->Branch("column", &tagm_column, "column/I");
  ttagm->Branch("tick", &tagm_tick, "tick/I");
  ttagm->Branch("rftick", &tagm_rftick, "rftick/I");
  ttagm->Branch("time", &tagm_time, "time/D");
  ttagm->Branch("rftime", &tagm_rftime, "rftime/D");
  ttagm->Branch("f1trig", &tagm_f1trig, "f1trig/I");
  ttagm->Branch("trigtime", &tagm_trigtime, "trigtime/D");
  ttagm->Branch("eventtime", &event_time, "eventtime/D");
  tevent = new TTree("tevent", "event times");
  tevent->Branch("time", &event_time, "time/D");
  tps = new TTree("tps", "ps times");
  tps->Branch("tick", &psc_tick, "tick/I");
  tps->Branch("time", &psc_time, "time/D");

  // unlock
  lock_svc->RootUnLock();
}


void JEventProcessor_TAGM_tracer::BeginRun(const std::shared_ptr<const JEvent>& event) {
}


void JEventProcessor_TAGM_tracer::Process(const std::shared_ptr<const JEvent>& event) {
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
 
  auto app = GetApplication();
  auto lock_svc = app->GetService<JLockService>();

  static double f1tdc_hires_tick = (32/152.)*(128/232.)/2; // ns
  // for details on where these values come from,
  // see appendix A in GlueX-doc-2686.
  static double f1tdc_rollover_period = f1tdc_hires_tick * 64467;
  static double timestamp_clock_period = 4; // ns

  event_time = 0;
  std::vector<const DCODAROCInfo*> rocinfos;
  event->Get(rocinfos);
  std::vector<const DCODAROCInfo*>::iterator itroc;
  for (itroc = rocinfos.begin(); itroc != rocinfos.end(); ++itroc) {
    if ((*itroc)->rocid == 75)  { // this is the tagger tdc crate
      event_time = timestamp_clock_period * double((*itroc)->timestamp);
      break;
    }
  }
  if (event_time > 0) {
    tevent->Fill();
    double event_t0 = int(event_time / f1tdc_rollover_period) *
                                       f1tdc_rollover_period;
    tagm_rftick = 0;
    tagm_rftime = 0;
    std::vector<const DRFTDCDigiTime*> rftdcs;
    event->Get(rftdcs);
    std::vector<const DRFTDCDigiTime*>::iterator itrf;
    for (itrf = rftdcs.begin(); itrf != rftdcs.end(); ++itrf) {
       if ((*itrf)->dSystem == SYS_TAGH) {
         if (tagm_rftime == 0) {
            tagm_rftick = (*itrf)->time;
            tagm_rftime = (*itrf)->time * f1tdc_hires_tick + event_t0;
         }
       }
    }
    std::vector<const DF1TDCTriggerTime*> triggers;
    event->Get(triggers);
    std::vector<const DTAGMTDCDigiHit*> tagmtdcs;
    event->Get(tagmtdcs);
    std::vector<const DTAGMTDCDigiHit*>::iterator iter;
    for (iter = tagmtdcs.begin(); iter != tagmtdcs.end(); ++iter) {
      tagm_f1trig = 0;
      tagm_trigtime = 0;
      const DF1TDCHit *F1hit;
      (*iter)->GetSingle(F1hit);
      if (F1hit != 0) {
         tagm_f1trig = F1hit->trig_time;
         std::vector<const DF1TDCTriggerTime*>::iterator itrig;
         for (itrig = triggers.begin(); itrig != triggers.end(); ++itrig) {
            if (F1hit->rocid == (*itrig)->rocid &&
                F1hit->slot == (*itrig)->slot)
            {
               tagm_trigtime = (*itrig)->time;
            }
         }
      }
      tagm_row = (*iter)->row;
      tagm_column = (*iter)->column;
      tagm_tick = (*iter)->time;
      tagm_time = (*iter)->time * f1tdc_hires_tick + event_t0;
      ttagm->Fill();
    }
    std::vector<const DPSCTDCDigiHit*> psctdcs;
    event->Get(psctdcs);
    std::vector<const DPSCTDCDigiHit*>::iterator itpsc;
    for (itpsc = psctdcs.begin(); itpsc != psctdcs.end(); ++itpsc) {
      psc_tick = (*itpsc)->time;
      psc_time = (*itpsc)->time * f1tdc_hires_tick + event_t0;
      tps->Fill();
    }
  }
  lock_svc->RootUnLock();
}


void JEventProcessor_TAGM_tracer::EndRun() {
}


void JEventProcessor_TAGM_tracer::Finish() {
}
