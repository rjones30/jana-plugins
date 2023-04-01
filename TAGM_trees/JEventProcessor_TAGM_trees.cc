#include <stdint.h>
#include <vector>
#include <TMath.h>
#include <TDirectory.h>
#include <TTree.h>

#include "JEventProcessor_TAGM_trees.h"
#include <JANA/JApplication.h>
#include <TAGGER/DTAGMDigiHit.h>
#include <RF/DRFTime.h>


// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(jana::JApplication *app) {
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_TAGM_trees());
  }
}


JEventProcessor_TAGM_trees::JEventProcessor_TAGM_trees() {
}


JEventProcessor_TAGM_trees::~JEventProcessor_TAGM_trees() {
}


jerror_t JEventProcessor_TAGM_trees::init(void) {

  // lock all root operations
  japp->RootWriteLock();

  fTree = new TTree("fadc","");
  fTree->Branch("runno", &runno, "runno/I");
  fTree->Branch("eventno", &eventno, "eventno/I");
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
  japp->RootUnLock();

  return NOERROR;
}


jerror_t JEventProcessor_TAGM_trees::brun(jana::JEventLoop *eventLoop, int runnumber) {
  // This is called whenever the run number changes
  return NOERROR;
}


jerror_t JEventProcessor_TAGM_trees::evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber) {
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
 
  std::vector<const DRFTime*> rf_times;
  std::vector<const DRFTime*>::const_iterator irf;
  eventLoop->Get(rf_times, "TAGH");
  for (irf = rf_times.begin(); irf != rf_times.end(); ++irf) {
    rftime = (*irf)->dTime;
  }

  std::vector<const DTAGMDigiHit*> hits;
  eventLoop->Get(hits);

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

  japp->RootWriteLock();
  std::vector<const DTAGMDigiHit*>::const_iterator hiter;
  for (hiter = hits.begin(); hiter != hits.end(); ++hiter) {
    runno = eventLoop->GetJEvent().GetRunNumber();
    eventno = eventLoop->GetJEvent().GetEventNumber();
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
  japp->RootUnLock();
  return NOERROR;
}


jerror_t JEventProcessor_TAGM_trees::erun(void) {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}


jerror_t JEventProcessor_TAGM_trees::fini(void) {
  // Called before program exit after event processing is finished.
  return NOERROR;
}
