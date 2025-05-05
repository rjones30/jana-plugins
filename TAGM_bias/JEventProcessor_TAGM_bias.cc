// $Id$
//
//    File: JEventProcessor_TAGM_bias.cc
// Created: Fri Nov 17 09:21:56 EST 2017
// Creator: aebarnes (on Linux ifarm1401.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#include "JEventProcessor_TAGM_bias.h"

#include <JANA/Services/JLockService.h>

#include "TAGGER/DTAGMDigiHit.h"

#include <TH1.h>

static const uint32_t NCOLUMNS = 102;

static TH1I *h_spectra[NCOLUMNS];
static TH1I *h_source;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
   void InitPlugin(JApplication *app){
      InitJANAPlugin(app);
      app->Add(new JEventProcessor_TAGM_bias());
   }
} // "C"


JEventProcessor_TAGM_bias::JEventProcessor_TAGM_bias() {
}


JEventProcessor_TAGM_bias::~JEventProcessor_TAGM_bias() {
}


void JEventProcessor_TAGM_bias::Init() {

   // lock all root operations
   auto app = GetApplication();
   auto lock_svc = app->GetService<JLockService>();
   lock_svc->RootWriteLock();

   for (uint32_t i = 0; i < NCOLUMNS; ++i) {
      h_spectra[i] = new TH1I(Form("h_spectra_%i", i+1),
                              Form("Pulse spectrum for col %i;\
                              Pulse height (ADC counts)", i+1),
                              4200, 0, 4200);
   }

   h_source = new TH1I("source",";Datasource", 3, -0.5, 2.5);
 
   // unlock
   lock_svc->RootUnLock();
}


void JEventProcessor_TAGM_bias::BeginRun(const std::shared_ptr<const JEvent>& event) {
}


void JEventProcessor_TAGM_bias::Process(const std::shared_ptr<const JEvent>& event) {
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // event->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
 
   auto app = GetApplication();
   auto lock_svc = app->GetService<JLockService>();


   std::vector<const DTAGMDigiHit*> digi_hits;
   event->Get( digi_hits );

   for (uint32_t i = 0; i < digi_hits.size(); ++i) {
      const DTAGMDigiHit *digi = digi_hits[i];
      int row = digi->row;
      int col = digi->column;
      //uint32_t pulse_integral = digi->pulse_integral;
      uint32_t pulse_peak = digi->pulse_peak;
      uint32_t t = digi->pulse_time;
      uint32_t pedestal = digi->pedestal;
      //uint32_t nsamp_int = digi->nsamples_integral;
      uint32_t nsamp_ped = digi->nsamples_pedestal;
      uint32_t datasource = digi->datasource;

      if (pedestal <= 0 || t == 0 || pulse_peak == 0 || row != 0 || nsamp_ped == 0) continue;

      double ped = pedestal / nsamp_ped;
      double peak = pulse_peak - ped;
      h_source->Fill(datasource);
      h_spectra[col - 1]->Fill(peak);
   }

   lock_svc->RootUnLock();
}


void JEventProcessor_TAGM_bias::EndRun() {
}


void JEventProcessor_TAGM_bias::Finish() {
}

