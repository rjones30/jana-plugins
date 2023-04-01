//
//    File: JEventProcessor_trackhits.cc
//


#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>
#include <TMath.h>

#include "JEventProcessor_trackhits.h"
#include <JANA/JApplication.h>
#include <TRACKING/DTrackTimeBased.h>
#include <TRACKING/DTrackWireBased.h>

using namespace std;
using namespace jana;

#include <GlueX.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>

extern "C"{
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_trackhits());
  }
}

JEventProcessor_trackhits::JEventProcessor_trackhits()
{
}

JEventProcessor_trackhits::~JEventProcessor_trackhits() {
}

jerror_t JEventProcessor_trackhits::init(void) {
  h2w = new TH2D("h2w", "wire-based track hits vs track angle", 70, 0, 140, 50, 0, 50);
  h2w->GetXaxis()->SetTitle("track theta (deg)");
  h2w->GetYaxis()->SetTitle("track hit count");
  h2t = new TH2D("h2t", "time-based track hits vs track angle", 70, 0, 140, 50, 0, 50);
  h2t->GetXaxis()->SetTitle("track theta (deg)");
  h2t->GetYaxis()->SetTitle("track hit count");
  h1p = new TH1D("h1p", "fdc pseudo-hit count for event", 50, 0, 50);
  h1p->GetXaxis()->SetTitle("pseudo-hit count");
  h1p->GetYaxis()->SetTitle("events");
  ptree = new TTree("ptree", "pseudo-hits");
  ptree->Branch("ptreed", &ptreed, "x/D:y/D:layer/I");
  return NOERROR;
}


jerror_t JEventProcessor_trackhits::brun(JEventLoop *eventLoop, int32_t runnumber) {
  // This is called whenever the run number changes
  return NOERROR;
}

jerror_t JEventProcessor_trackhits::evnt(JEventLoop *eventLoop, uint64_t eventnumber) {
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop-Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.

  std::vector<const DTrackWireBased*> wtracks;
  eventLoop->Get(wtracks);
  for (auto track : wtracks) {
     double theta = asin(track->pperp() / track->pmag());
     //std::cout << "theta=" << theta * 180/M_PI << ", nhits=" << ntrackhits << std::endl;
     h2w->Fill(theta * 180/M_PI, track->Ndof);
  }
  std::vector<const DTrackTimeBased*> ttracks;
  eventLoop->Get(ttracks);
  for (auto track : ttracks) {
     double theta = asin(track->pperp() / track->pmag());
     //std::cout << "theta=" << theta * 180/M_PI << ", nhits=" << ntrackhits << std::endl;
     h2t->Fill(theta * 180/M_PI, track->Ndof);
  }
  std::vector<const DFDCPseudo*> pseudos;
  eventLoop->Get(pseudos);
  h1p->Fill(pseudos.size());
  for (auto pseudo : pseudos) {
     ptreed.layer = pseudo->wire->layer;
     ptreed.x = pseudo->xy.X();
     ptreed.y = pseudo->xy.Y();
     ptree->Fill();
  }
  return NOERROR;
}

jerror_t JEventProcessor_trackhits::erun(void) {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

jerror_t JEventProcessor_trackhits::fini(void) {
  // Called before program exit after event processing is finished.
  TFile f("trackhits.root", "RECREATE");
  h2w->Write();
  h2t->Write();
  h1p->Write();
  ptree->Write();
  return NOERROR;
}
