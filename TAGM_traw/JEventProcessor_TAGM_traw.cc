#include <stdint.h>
#include <vector>
#include <TMath.h>
#include <TDirectory.h>
#include <TTree.h>

#include "JEventProcessor_TAGM_traw.h"
#include <JANA/JApplication.h>
#include <TAGGER/DTAGMDigiHit.h>
#include <TRIGGER/DL1Trigger.h>
#include <DAQ/Df250WindowRawData.h>
#include <RF/DRFTime.h>

#include <JANA/Services/JLockService.h>

int fadc_column[] = {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                       3,  2,  1,  6,  5,  4,  9,  9,  9,  9,  9,  9,  8,  7, 12, 11,
                      10, 15, 14, 13, 18, 17, 16, 21, 20, 19, 24, 23, 22, 27, 27, 27,
                      27, 27, 27, 26, 25, 30, 29, 28, 33, 32, 31, 36, 35, 34, 39, 38,
                      37, 42, 41, 40, 45, 44, 43, 48, 47, 46, 51, 50, 49, 54, 53, 52,
                      57, 56, 55, 60, 59, 58, 63, 62, 61, 66, 65, 64, 69, 68, 67, 72,
                      71, 70, 75, 74, 73, 78, 77, 76, 81, 81, 81, 81, 81, 81, 80, 79,
                      84, 83, 82, 88, 89, 90, 85, 86, 87, 93, 92, 91, 96, 95, 94, 99,
                      99, 99, 99, 99, 99, 98, 97,102,101,100,994,995,996,997,998,999};

int fadc_row[] = {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,
                    4,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  0,  0,  0,
                    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
                    2,  3,  4,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_TAGM_traw());
  }
}


JEventProcessor_TAGM_traw::JEventProcessor_TAGM_traw() {
}


JEventProcessor_TAGM_traw::~JEventProcessor_TAGM_traw() {
}


void JEventProcessor_TAGM_traw::Init() {

  // lock all root operations
  auto app = GetApplication();
  auto lock_svc = app->GetService<JLockService>();
  lock_svc->RootWriteLock();

  fTraw = new TTree("fadc","");
  fTraw->Branch("runno", &runno, "runno/I");
  fTraw->Branch("eventno", &eventno, "eventno/I");
  fTraw->Branch("trigger", &trigger, "trigger/I");
  fTraw->Branch("nhits", &nhits, "nhits/I");
  fTraw->Branch("nraw", &nraw, "nraw/I");
  fTraw->Branch("row", &row, "row/I");
  fTraw->Branch("col", &col, "col/I");
  fTraw->Branch("npulse", &npulse, "npulse/I");
  fTraw->Branch("pi", &pi, "pi[npulse]/I");
  fTraw->Branch("pt", &pt, "pt[npulse]/I");
  fTraw->Branch("peak", &peak, "peak[npulse]/I");
  fTraw->Branch("npi", &npi, "npi[npulse]/I");
  fTraw->Branch("qf", &qf, "qf[npulse]/I");
  fTraw->Branch("ped", &ped, "ped/I");
  fTraw->Branch("nped", &nped, "nped/I");
  fTraw->Branch("nsamples", &nsamples, "nsamples/I");
  fTraw->Branch("samples", &samples, "samples[nsamples]/I");
  fTraw->Branch("invalid_samples", &invalid_samples, "invalid_samples/I");
  fTraw->Branch("overflow_samples", &overflow_samples, "overflow_samples/I");
  fTraw->Branch("rftime", &rftime, "rftime/D");

  // unlock
  lock_svc->RootUnLock();
}


void JEventProcessor_TAGM_traw::BeginRun(const std::shared_ptr<const JEvent>& event) {
}


void JEventProcessor_TAGM_traw::Process(const std::shared_ptr<const JEvent>& event) {
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
 
  auto app = GetApplication();
  auto lock_svc = app->GetService<JLockService>();

  runno = event->GetRunNumber();
  eventno = event->GetEventNumber();

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
  trigger = fp_trig_mask > 0 ? 10 + fp_trig_mask : trig_mask;

  std::vector<const DRFTime*> rf_times;
  std::vector<const DRFTime*>::const_iterator irf;
  event->Get(rf_times, "TAGH");
  for (irf = rf_times.begin(); irf != rf_times.end(); ++irf) {
    rftime = (*irf)->dTime;
  }

  std::vector<const DTAGMDigiHit*> hits;
  std::vector<const Df250WindowRawData*> raw;
  event->Get(hits);
  nhits = hits.size();
  event->Get(raw);
  nraw = 0;
  std::vector<const Df250WindowRawData*>::iterator riter;
  for (riter = raw.begin(); riter != raw.end(); ++riter) {
    int slot_chan = (*riter)->slot * 16 + (*riter)->channel;
    if ((*riter)->rocid != 71 || slot_chan <= 0 || slot_chan >= 176)
      continue;
    ++nraw;
  }
  if (nraw < 25) {
    lock_svc->RootUnLock();
    return;
  }

  for (riter = raw.begin(); riter != raw.end(); ++riter) {
    int slot_chan = (*riter)->slot * 16 + (*riter)->channel;
    if ((*riter)->rocid != 71 || slot_chan <= 0 || slot_chan >= 176)
      continue;
    col = fadc_column[slot_chan];
    row = fadc_row[slot_chan];
    nsamples = (*riter)->samples.size();
    for (int i=0; i < nsamples; ++i)
      samples[i] = (*riter)->samples[i];
    invalid_samples = (*riter)->invalid_samples;
    overflow_samples = (*riter)->overflow;
    npulse = 0;
    std::vector<const DTAGMDigiHit*>::const_iterator hiter;
    for (hiter = hits.begin(); hiter != hits.end(); ++hiter) {
      if ((*hiter)->row == row && (*hiter)->column == col) {
        ped = (*hiter)->pedestal;
        nped = (*hiter)->nsamples_pedestal;
        pi[npulse] = (*hiter)->pulse_integral;
        pt[npulse] = (*hiter)->pulse_time;
        peak[npulse] = (*hiter)->pulse_peak;
        npi[npulse] = (*hiter)->nsamples_integral;
        qf[npulse] = (*hiter)->QF;
        ++npulse;
      }
    }
    fTraw->Fill();
  }
  lock_svc->RootUnLock();
}


void JEventProcessor_TAGM_traw::EndRun() {
}


void JEventProcessor_TAGM_traw::Finish() {
}
