#include <stdint.h>
#include <vector>
#include <TMath.h>
#include <TDirectory.h>
#include <TTree.h>

#include "JEventProcessor_TAGM_trees.h"
#include <JANA/JApplication.h>
#include <TAGGER/DTAGMDigiHit.h>
#include <TRIGGER/DL1Trigger.h>
#include <RF/DRFTime.h>

#include <JANA/Services/JLockService.h>

// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_TAGM_trees());
  }
}


JEventProcessor_TAGM_trees::JEventProcessor_TAGM_trees() {
}


JEventProcessor_TAGM_trees::~JEventProcessor_TAGM_trees() {
}


void JEventProcessor_TAGM_trees::Init() {

  // lock all root operations
  auto app = GetApplication();
  auto lock_svc = app->GetService<JLockService>();
  lock_svc->RootWriteLock();

  fTree = new TTree("fadc","");
  fTree->Branch("runno", &runno, "runno/I");
  fTree->Branch("eventno", &eventno, "eventno/I");
  fTree->Branch("trigger", &trigger, "trigger/I");
  fTree->Branch("row", &row, "row/I");
  fTree->Branch("col", &col, "col/I");
  fTree->Branch("pi", &pi, "pi/I");
  fTree->Branch("pt", &pt, "pt/I");
  fTree->Branch("ped", &ped, "ped/I");
  fTree->Branch("qf", &qf, "qf/I");
  fTree->Branch("npi", &npi, "npi/I");
  fTree->Branch("nped", &nped, "nped/I");
  fTree->Branch("peak", &peak, "peak/I");
  fTree->Branch("rftime", &rftime, "rftime/D");

  // unlock
  lock_svc->RootUnLock();
}


void JEventProcessor_TAGM_trees::BeginRun(const std::shared_ptr<const JEvent>& event) {
}


void JEventProcessor_TAGM_trees::Process(const std::shared_ptr<const JEvent>& event) {
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // event->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
 
  auto app = GetApplication();
  auto lock_svc = app->GetService<JLockService>();

  const DL1Trigger *trig_words = 0;
  uint32_t trig_mask, fp_trig_mask;
  try {
     event->GetSingle(trig_words);
  } catch(...) {};
  if (trig_words) {
     trig_mask = trig_words->trig_mask;
     fp_trig_mask = trig_words->fp_trig_mask;
  }
  else {
     trig_mask = 0;
     fp_trig_mask = 0;
  }
  int trig_bits = fp_trig_mask > 0 ? 10 + fp_trig_mask : trig_mask;

  std::vector<const DRFTime*> rf_times;
  std::vector<const DRFTime*>::const_iterator irf;
  event->Get(rf_times, "TAGH");
  for (irf = rf_times.begin(); irf != rf_times.end(); ++irf) {
    rftime = (*irf)->dTime;
  }

  std::vector<const DTAGMDigiHit*> hits;
  event->Get(hits);

#if 0
std::cout << "------------new event-----------------" << std::endl;
for (auto hit : hits) {
   std::cout << "   --- new hit ---" << std::endl;
   std::vector<pair<string,string> > items;
   hit->toStrings(items);
   for (auto item : items) {
      std::cout << item.first << " = " << item.second << std::endl;
   }
}
#endif

  lock_svc->RootWriteLock();
  std::vector<const DTAGMDigiHit*>::const_iterator hiter;
  for (hiter = hits.begin(); hiter != hits.end(); ++hiter) {
    runno = event->GetRunNumber();
    eventno = event->GetEventNumber();
    trigger = trig_bits;
    row = (*hiter)->row;
    col = (*hiter)->column;
    pi = (*hiter)->pulse_integral;
    pt = (*hiter)->pulse_time;
    ped = (*hiter)->pedestal;
    qf = (*hiter)->QF;
    npi = (*hiter)->nsamples_integral;
    nped = (*hiter)->nsamples_pedestal;
    peak = (*hiter)->pulse_peak;
    fTree->Fill();
  }
  lock_svc->RootUnLock();
}


void JEventProcessor_TAGM_trees::EndRun() {
}


void JEventProcessor_TAGM_trees::Finish() {
}
