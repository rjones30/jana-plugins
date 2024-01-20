//
//    File: JEventProcessor_PStagstudy.cc
//

//#define VERBOSE 1

#include <iostream>
#include <stdint.h>
#include <vector>
#include <TMath.h>
#include <TInterpreter.h>

#include "JEventProcessor_PStagstudy.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;

#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGMDigiHit.h>
#include <TAGGER/DTAGMTDCDigiHit.h>
#include <TAGGER/DTAGHHit.h>
#include <TAGGER/DTAGHDigiHit.h>
#include <TAGGER/DTAGHTDCDigiHit.h>
#include <DAQ/Df250WindowRawData.h>
#include <TTAB/DTranslationTable.h>
#include <RF/DRFTime.h>
#include <PAIR_SPECTROMETER/DPSCPair.h>
#include <PAIR_SPECTROMETER/DPSPair.h>
#include <PAIR_SPECTROMETER/DPSCDigiHit.h>
#include <TRIGGER/DL1Trigger.h>
#include <PID/DBeamPhoton.h>
#include <DAQ/DCODAEventInfo.h>
#include <DAQ/DCODAROCInfo.h>
#include <DAQ/Df250PulseData.h>
#include <GlueX.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

// main trigger is bit 0, ps triggers are bit 3
//#define SELECT_TRIGGER_TYPE 3

void report_bad_pmax(const DTAGMHit *itagm) {
   std::cout << "  DTAGMHit: column=" << itagm->column
             << " row=" << itagm->row << std::endl;
   std::vector<const DTAGMDigiHit*> digi_hits;
   itagm->Get(digi_hits);
   std::vector<const DTAGMDigiHit*>::iterator atagm;
   for (atagm = digi_hits.begin(); atagm != digi_hits.end(); ++atagm) {
      std::cout << "    DTAGMDigiHit: column=" << (*atagm)->column
                << " row=" << (*atagm)->row << std::endl;
   }
   std::vector<const DTAGMTDCDigiHit*> tdc_digi_hits;
   itagm->Get(tdc_digi_hits);
   std::vector<const DTAGMTDCDigiHit*>::iterator ttagm;
   for (ttagm = tdc_digi_hits.begin(); ttagm != tdc_digi_hits.end(); ++ttagm) {
      std::cout << "    DTAGMTDCDigiHit: column=" << (*ttagm)->column
                << " row=" << (*ttagm)->row << std::endl;
   }
}

extern "C"{
   void InitPlugin(JApplication *app) {
     InitJANAPlugin(app);
     app->AddProcessor(new JEventProcessor_PStagstudy());
   }
}

JEventProcessor_PStagstudy::JEventProcessor_PStagstudy()
 : pstags(0), lockflag(0)
{
}


JEventProcessor_PStagstudy::~JEventProcessor_PStagstudy() {
}

void JEventProcessor_PStagstudy::lock() {
   japp->RootWriteLock();
   if (lockflag) {
      std::cerr << "Error in JEventProcessor_PStagstudy::lock() - "
                   "deadlock!" << std::endl;
   }
   else {
      lockflag = 1;
   }
}

void JEventProcessor_PStagstudy::unlock() {
   if (lockflag == 0) {
      std::cerr << "Error in JEventProcessor_PStagstudy::evnt() - "
                   "double unlock!" << std::endl;
   }
   else {
      lockflag = 0;
   }
   japp->RootUnLock();
}

const DTranslationTable::DChannelInfo
JEventProcessor_PStagstudy::GetDetectorIndex(const DTranslationTable *ttab,
                                             DTranslationTable::csc_t csc)
{
   DTranslationTable::DChannelInfo index;
   try {
      index = ttab->GetDetectorIndex(csc);
   }
   catch(...) {
   }
   return index;
}

jerror_t JEventProcessor_PStagstudy::init(void) {
   lock();

   gInterpreter->GenerateDictionary("std::vector<std::vector<unsigned short> >", "vector"); 

   pstags = new TTree("pstags", "PS tag study");
   pstags->Branch("runno", &runno, "runno/i");
   pstags->Branch("eventno", &eventno, "eventno/i");
   pstags->Branch("timestamp", &timestamp, "timestamp/l");

   pstags->Branch("nrf", &nrf, "nrf/I[0,999]");
   pstags->Branch("rf_sys", rf_sys, "rf_sys[nrf]/I");
   pstags->Branch("rf_time", rf_time, "rf_time[nrf]/D");

   pstags->Branch("ntagm", &ntagm, "ntagm/I[0,999]");
   pstags->Branch("tagm_seqno", tagm_seqno, "tagm_seqno[ntagm]/I");
   pstags->Branch("tagm_channel", tagm_channel, "tagm_channel[ntagm]/I");
   pstags->Branch("tagm_peak", tagm_peak, "tagm_peak[ntagm]/F");
   pstags->Branch("tagm_pint", tagm_pint, "tagm_pint[ntagm]/F");
   pstags->Branch("tagm_tadc", tagm_tadc, "tagm_tadc[ntagm]/F");
   pstags->Branch("tagm_toth", tagm_toth, "tagm_toth[ntagm]/F");
   pstags->Branch("tagm_ttdc", tagm_ttdc, "tagm_ttdc[ntagm]/F");
   pstags->Branch("tagm_time", tagm_time, "tagm_time[ntagm]/F");
   pstags->Branch("tagm_Etag", tagm_Etag, "tagm_Etag[ntagm]/F");
   pstags->Branch("tagm_pmax", tagm_pmax, "tagm_pmax[ntagm]/F");
   pstags->Branch("tagm_ped", tagm_ped, "tagm_ped[ntagm]/F");
   pstags->Branch("tagm_multi", tagm_multi, "tagm_multi[ntagm]/I");
   pstags->Branch("tagm_qf", tagm_qf, "tagm_qf[ntagm]/I");
   pstags->Branch("tagm_bg", tagm_bg, "tagm_bg[ntagm]/I");
   pstags->Branch("tagm_has_adc", tagm_has_adc, "tagm_has_adc[ntagm]/I");
   pstags->Branch("tagm_has_tdc", tagm_has_tdc, "tagm_has_tdc[ntagm]/I");
   pstags->Branch("tagm_nped", tagm_nped, "tagm_nped[ntagm]/I");
   pstags->Branch("tagm_nint", tagm_nint, "tagm_nint[ntagm]/I");
   pstags->Branch("tagm_raw_waveform", &tagm_raw_waveform, 30000, 1);

   pstags->Branch("ntagh", &ntagh, "ntagh/I[0,999]");
   pstags->Branch("tagh_seqno", tagh_seqno, "tagh_seqno[ntagh]/I");
   pstags->Branch("tagh_counter", tagh_counter, "tagh_counter[ntagh]/I");
   pstags->Branch("tagh_peak", tagh_peak, "tagh_peak[ntagh]/F");
   pstags->Branch("tagh_pint", tagh_pint, "tagh_pint[ntagh]/F");
   pstags->Branch("tagh_tadc", tagh_tadc, "tagh_tadc[ntagh]/F");
   pstags->Branch("tagh_toth", tagh_toth, "tagh_toth[ntagh]/F");
   pstags->Branch("tagh_ttdc", tagh_ttdc, "tagh_ttdc[ntagh]/F");
   pstags->Branch("tagh_time", tagh_time, "tagh_time[ntagh]/F");
   pstags->Branch("tagh_Etag", tagh_Etag, "tagh_Etag[ntagh]/F");
   pstags->Branch("tagh_pmax", tagh_pmax, "tagh_pmax[ntagh]/F");
   pstags->Branch("tagh_ped", tagh_ped, "tagh_ped[ntagh]/F");
   pstags->Branch("tagh_multi", tagh_multi, "tagh_multi[ntagh]/I");
   pstags->Branch("tagh_qf", tagh_qf, "tagh_qf[ntagh]/I");
   pstags->Branch("tagh_bg", tagh_bg, "tagh_bg[ntagh]/I");
   pstags->Branch("tagh_has_adc", tagh_has_adc, "tagh_has_adc[ntagh]/I");
   pstags->Branch("tagh_has_tdc", tagh_has_tdc, "tagh_has_tdc[ntagh]/I");
   pstags->Branch("tagh_nped", tagh_nped, "tagh_nped[ntagh]/I");
   pstags->Branch("tagh_nint", tagh_nint, "tagh_nint[ntagh]/I");
   pstags->Branch("tagh_raw_waveform", &tagh_raw_waveform, 30000, 1);

   pstags->Branch("nbeam", &nbeam, "nbeam/I[0,999]");
   pstags->Branch("beam_sys", beam_sys, "beam_sys[nbeam]/I");
   pstags->Branch("beam_E", beam_E, "beam_E[nbeam]/F");
   pstags->Branch("beam_t", beam_t, "beam_t[nbeam]/F");
   pstags->Branch("beam_z", beam_z, "beam_z[nbeam]/F");

   pstags->Branch("npairps", &npairps, "npairps/I[0,999]");
   pstags->Branch("Epair", Epair, "Epair[npairps]/F");
   pstags->Branch("tpair", tpair, "tpair[npairps]/F");
   pstags->Branch("psleft_peak", psleft_peak, "psleft_peak[npairps]/F");
   pstags->Branch("psright_peak", psright_peak, "psright_peak[npairps]/F");
   pstags->Branch("psleft_pint", psleft_pint, "psleft_pint[npairps]/F");
   pstags->Branch("psright_pint", psright_pint, "psright_pint[npairps]/F");
   pstags->Branch("psleft_time", psleft_time, "psleft_time[npairps]/F");
   pstags->Branch("psright_time", psright_time, "psright_time[npairps]/F");
   pstags->Branch("psEleft", psEleft, "psEleft[npairps]/F");
   pstags->Branch("psEright", psEright, "psEright[npairps]/F");
   pstags->Branch("pstleft", pstleft, "pstleft[npairps]/F");
   pstags->Branch("pstright", pstright, "pstright[npairps]/F");
   pstags->Branch("nleft_ps", nleft_ps, "nleft_ps[npairps]/I");
   pstags->Branch("nright_ps", nright_ps, "nright_ps[npairps]/I");
   pstags->Branch("psleft_raw_waveform", &psleft_raw_waveform, 30000, 1);
   pstags->Branch("psright_raw_waveform", &psright_raw_waveform, 30000, 1);

   pstags->Branch("npairpsc", &npairpsc, "npairpsc/I[0,999]");
   pstags->Branch("pscleft_seqno", pscleft_seqno, "pscleft_seqno[npairpsc]/I");
   pstags->Branch("pscright_seqno", pscright_seqno, "pscright_seqno[npairpsc]/I");
   pstags->Branch("pscleft_module", pscleft_module, "pscleft_module[npairpsc]/I");
   pstags->Branch("pscright_module", pscright_module, "pscright_module[npairpsc]/I");
   pstags->Branch("pscleft_peak", pscleft_peak, "pscleft_peak[npairpsc]/F");
   pstags->Branch("pscright_peak", pscright_peak, "pscright_peak[npairpsc]/F");
   pstags->Branch("pscleft_pint", pscleft_pint, "pscleft_pint[npairpsc]/F");
   pstags->Branch("pscright_pint", pscright_pint, "pscright_pint[npairpsc]/F");
   pstags->Branch("pscleft_ttdc", pscleft_ttdc, "pscleft_ttdc[npairpsc]/F");
   pstags->Branch("pscright_ttdc", pscright_ttdc, "pscright_ttdc[npairpsc]/F");
   pstags->Branch("pscleft_tadc", pscleft_tadc, "pscleft_tadc[npairpsc]/F");
   pstags->Branch("pscright_tadc", pscright_tadc, "pscright_tadc[npairpsc]/F");
   pstags->Branch("pscleft_t", pscleft_t, "pscleft_t[npairpsc]/F");
   pstags->Branch("pscright_t", pscright_t, "pscright_t[npairpsc]/F");
   pstags->Branch("pscleft_ped", pscleft_ped, "pscleft_ped[npairpsc]/F");
   pstags->Branch("pscright_ped", pscright_ped, "pscright_ped[npairpsc]/F");
   pstags->Branch("pscleft_qf", pscleft_qf, "pscleft_qf[npairpsc]/I");
   pstags->Branch("pscright_qf", pscright_qf, "pscright_qf[npairpsc]/I");
   pstags->Branch("pscleft_raw_waveform", &pscleft_raw_waveform, 30000, 1);
   pstags->Branch("pscright_raw_waveform", &pscright_raw_waveform, 30000, 1);

   unlock();
   return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_PStagstudy::brun(JEventLoop *eventLoop, int32_t runnumber)
{
   // This is called whenever the run number changes
   return NOERROR;
}

jerror_t JEventProcessor_PStagstudy::evnt(JEventLoop *eventLoop, uint64_t eventnumber) {
   // This is called for every event. Use of common resources like writing
   // to a file or filling a histogram should be mutex protected. Using
   // loop-Get(...) to get reconstructed objects (and thereby activating the
   // reconstruction algorithm) should be done outside of any mutex lock
   // since multiple threads may call this method at the same time.

   // only examine PS triggers
   const DL1Trigger *trig_words = 0;
   uint32_t trig_mask, fp_trig_mask;
   try {
      eventLoop->GetSingle(trig_words);
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
#ifdef SELECT_TRIGGER_TYPE
   if ((trig_bits & (1 << SELECT_TRIGGER_TYPE)) == 0)
      return NOERROR;
#else
   if (trig_bits == 0)
      trig_bits = 0;
#endif

   lock();
 
   std::vector<const DCODAEventInfo*> event_info;
   eventLoop->Get(event_info);
   if (event_info.size() == 0)
      return NOERROR;

   runno = event_info[0]->run_number;
   eventno = event_info[0]->event_number;
   timestamp = event_info[0]->avg_timestamp;

   std::vector<const DRFTime*> rf_times;
   eventLoop->Get(rf_times, "PSC");
   std::vector<const DRFTime*>::iterator irf;
   nrf = 0;
   for (irf = rf_times.begin(); irf != rf_times.end(); ++irf) {
      rf_sys[nrf] = 0x4000;
      rf_time[nrf] = (*irf)->dTime;
      nrf++;
   }
   eventLoop->Get(rf_times, "TAGH");
   for (irf = rf_times.begin(); irf != rf_times.end(); ++irf) {
      rf_sys[nrf] = 0x800;
      rf_time[nrf] = (*irf)->dTime;
      nrf++;
   }

   // get the raw window data, if any
   std::vector<const Df250WindowRawData*> traces;
   eventLoop->Get(traces);

   std::vector<const DTranslationTable*> ttables;
   eventLoop->Get(ttables);
   if (ttables.size() != 1) {
      std::cout << "Serious error in PStagstudy plugin - "
                << "unable to acquire the DAQ translation table!"
                << std::endl;
      return NOERROR;
   }
   const DTranslationTable *ttab = ttables[0];

   tagm_raw_waveform.clear();
   tagh_raw_waveform.clear();
   psleft_raw_waveform.clear();
   psright_raw_waveform.clear();
   pscleft_raw_waveform.clear();
   pscright_raw_waveform.clear();

   std::vector<const DTAGMHit*> tagm_hits;
   eventLoop->Get(tagm_hits, "Calib");
   std::vector<const DTAGMHit*>::iterator itagm;
   int ntagm_per_channel[6][128] = {0};
   ntagm = 0;
   for (itagm = tagm_hits.begin(); itagm != tagm_hits.end(); ++itagm) {
      int row = (*itagm)->row;
      int column = (*itagm)->column;
      tagm_seqno[ntagm] = ntagm_per_channel[row][column]++;
      tagm_channel[ntagm] = column + row * 1000;
      tagm_peak[ntagm] = (*itagm)->pulse_peak;
      tagm_pint[ntagm] = (*itagm)->integral;
      tagm_tadc[ntagm] = (*itagm)->time_fadc;
      tagm_toth[ntagm] = 999;
      tagm_ttdc[ntagm] = (*itagm)->time_tdc;
      tagm_Etag[ntagm] = (*itagm)->E;
      tagm_time[ntagm] = (*itagm)->t;
      tagm_multi[ntagm] = 0;
      tagm_pmax[ntagm] = 999;
      tagm_ped[ntagm] = 999;
      tagm_qf[ntagm] = 999;
      tagm_bg[ntagm] = (*itagm)->bg;
      tagm_has_adc[ntagm] = (*itagm)->has_fADC;
      tagm_has_tdc[ntagm] = (*itagm)->has_TDC;
      tagm_nped[ntagm] = 999;
      tagm_nint[ntagm] = 999;
      std::vector<const DTAGMDigiHit*> digi_hits;
      (*itagm)->Get(digi_hits);
      std::vector<const DTAGMDigiHit*>::iterator atagm;
      for (atagm = digi_hits.begin(); atagm != digi_hits.end(); ++atagm) {
         if ((*atagm)->row == row &&
             (*atagm)->column == column)
         {
            tagm_pmax[ntagm] = (*atagm)->pulse_peak;
            tagm_ped[ntagm] = (*atagm)->pedestal;
            tagm_qf[ntagm] = (*atagm)->QF;
            tagm_nped[ntagm] = (*atagm)->nsamples_pedestal;
            tagm_nint[ntagm] = (*atagm)->nsamples_integral;
            std::vector<const Df250PulseData*> pulse_data;
            (*atagm)->Get(pulse_data);
            std::vector<const Df250PulseData*>::iterator ptagm;
            for (ptagm = pulse_data.begin(); ptagm != pulse_data.end(); ++ptagm) {
               tagm_toth[ntagm] = (*ptagm)->nsamples_over_threshold * 4;
               // f_qpedestal = ((*ptagm)->QF_pedestal)? 1 : 0;
               // f_latepulse = ((*ptagm)->QF_NSA_beyond_PTW)? 1 : 0;
               // f_underflow = ((*ptagm)->QF_underflow)? 1 : 0;
               // f_overflow = ((*ptagm)->QF_overflow)? 1 : 0;
               // f_notpeak = ((*ptagm)->QF_vpeak_beyond_NSA)? 1 : 0;
               // f_nopeak = ((*ptagm)->QF_vpeak_not_found)? 1 : 0;
               // f_badped = ((*ptagm)->QF_bad_pedestal)? 1 : 0;
            }
         }
      }
      std::vector<const DTAGMHit*> assoc_hits;
      (*itagm)->Get(assoc_hits);
      std::vector<const DTAGMHit*>::iterator jtagm;
      int mtagm = 0;
      for (jtagm = assoc_hits.begin(); jtagm != assoc_hits.end(); ++jtagm) {
         ntagm++;
         tagm_seqno[ntagm] = ntagm_per_channel[(*jtagm)->row][(*jtagm)->column]++;
         tagm_channel[ntagm] = (*jtagm)->column + (*jtagm)->row * 1000;
         tagm_peak[ntagm] = (*jtagm)->pulse_peak;
         tagm_pint[ntagm] = (*jtagm)->integral;
         tagm_tadc[ntagm] = (*jtagm)->time_fadc;
         tagm_toth[ntagm] = 999;
         tagm_ttdc[ntagm] = (*jtagm)->time_tdc;
         tagm_Etag[ntagm] = (*jtagm)->E;
         tagm_time[ntagm] = (*jtagm)->t;
         tagm_multi[ntagm] = ++mtagm;
         tagm_pmax[ntagm] = 999;
         tagm_ped[ntagm] = 999;
         tagm_qf[ntagm] = 999;
         tagm_bg[ntagm] = (*jtagm)->bg;
         tagm_has_adc[ntagm] = (*jtagm)->has_fADC;
         tagm_has_tdc[ntagm] = (*jtagm)->has_TDC;
         tagm_nped[ntagm] = 999;
         tagm_nint[ntagm] = 999;
         (*jtagm)->Get(digi_hits);
         for (atagm = digi_hits.begin(); atagm != digi_hits.end(); ++atagm) {
            if ((*atagm)->row == (*jtagm)->row &&
                (*atagm)->column == (*jtagm)->column)
            {
               tagm_pmax[ntagm] = (*atagm)->pulse_peak;
               tagm_ped[ntagm] = (*atagm)->pedestal;
               tagm_qf[ntagm] = (*atagm)->QF;
               tagm_nped[ntagm] = (*atagm)->nsamples_pedestal;
               tagm_nint[ntagm] = (*atagm)->nsamples_integral;
               std::vector<const Df250PulseData*> pulse_data;
               (*atagm)->Get(pulse_data);
               std::vector<const Df250PulseData*>::iterator ptagm;
               for (ptagm = pulse_data.begin(); ptagm != pulse_data.end(); ++ptagm) {
                  tagm_toth[ntagm] = (*ptagm)->nsamples_over_threshold * 4;
                  // f_qpedestal = ((*ptagm)->QF_pedestal)? 1 : 0;
                  // f_latepulse = ((*ptagm)->QF_NSA_beyond_PTW)? 1 : 0;
                  // f_underflow = ((*ptagm)->QF_underflow)? 1 : 0;
                  // f_overflow = ((*ptagm)->QF_overflow)? 1 : 0;
                  // f_notpeak = ((*ptagm)->QF_vpeak_beyond_NSA)? 1 : 0;
                  // f_nopeak = ((*ptagm)->QF_vpeak_not_found)? 1 : 0;
                  // f_badped = ((*ptagm)->QF_bad_pedestal)? 1 : 0;
               }
            }
         }
      }
      std::vector<unsigned short> trace;
      std::vector<const Df250WindowRawData*>::iterator itrace;
      for (itrace = traces.begin(); itrace != traces.end(); ++itrace) {
         DTranslationTable::csc_t csc = {(*itrace)->rocid, (*itrace)->slot, (*itrace)->channel};
         const DTranslationTable::DChannelInfo chaninfo = GetDetectorIndex(ttab, csc);
         if (chaninfo.det_sys == DTranslationTable::TAGM) {
            if ((int)chaninfo.tagm.row == row && (int)chaninfo.tagm.col == column) {
               trace = (*itrace)->samples;
            }
         }
      }
      tagm_raw_waveform.push_back(trace);
      ntagm++;
   }

   std::vector<const DTAGHHit*> tagh_hits;
   eventLoop->Get(tagh_hits, "Calib");
   std::vector<const DTAGHHit*>::iterator itagh;
   int ntagh_per_counter[512] = {0};
   ntagh = 0;
   for (itagh = tagh_hits.begin(); itagh != tagh_hits.end(); ++itagh) {
      tagh_seqno[ntagh] = ntagh_per_counter[(*itagh)->counter_id]++;
      tagh_counter[ntagh] = (*itagh)->counter_id;
      tagh_peak[ntagh] = (*itagh)->pulse_peak;
      tagh_pint[ntagh] = (*itagh)->integral;
      tagh_tadc[ntagh] = (*itagh)->time_fadc;
      tagh_toth[ntagh] = 999;
      tagh_ttdc[ntagh] = (*itagh)->time_tdc;
      tagh_Etag[ntagh] = (*itagh)->E;
      tagh_time[ntagh] = (*itagh)->t;
      tagh_multi[ntagh] = 0;
      tagh_pmax[ntagh] = 999;
      tagh_ped[ntagh] = 999;
      tagh_qf[ntagh] = 999;
      tagh_bg[ntagh] = (*itagh)->bg;
      tagh_has_adc[ntagh] = (*itagh)->has_fADC;
      tagh_has_tdc[ntagh] = (*itagh)->has_TDC;
      tagh_nped[ntagh] = 999;
      tagh_nint[ntagh] = 999;
      std::vector<const DTAGHDigiHit*> digi_hits;
      (*itagh)->Get(digi_hits);
      std::vector<const DTAGHDigiHit*>::iterator atagh;
      for (atagh = digi_hits.begin(); atagh != digi_hits.end(); ++atagh) {
         if ((*atagh)->counter_id == (*itagh)->counter_id) {
            tagh_pmax[ntagh] = (*atagh)->pulse_peak;
            tagh_ped[ntagh] = (*atagh)->pedestal;
            tagh_qf[ntagh] = (*atagh)->QF;
            tagh_nped[ntagh] = (*atagh)->nsamples_pedestal;
            tagh_nint[ntagh] = (*atagh)->nsamples_integral;
            std::vector<const Df250PulseData*> pulse_data;
            (*atagh)->Get(pulse_data);
            std::vector<const Df250PulseData*>::iterator ptagh;
            for (ptagh = pulse_data.begin(); ptagh != pulse_data.end(); ++ptagh) {
               tagm_toth[ntagh] = (*ptagh)->nsamples_over_threshold * 4;
               // f_qpedestal = ((*ptagh)->QF_pedestal)? 1 : 0;
               // f_latepulse = ((*ptagh)->QF_NSA_beyond_PTW)? 1 : 0;
               // f_underflow = ((*ptagh)->QF_underflow)? 1 : 0;
               // f_overflow = ((*ptagh)->QF_overflow)? 1 : 0;
               // f_notpeak = ((*ptagh)->QF_vpeak_beyond_NSA)? 1 : 0;
               // f_nopeak = ((*ptagh)->QF_vpeak_not_found)? 1 : 0;
               // f_badped = ((*ptagh)->QF_bad_pedestal)? 1 : 0;
            }
         }
      }
      std::vector<const DTAGHHit*> assoc_hits;
      (*itagh)->Get(assoc_hits);
      std::vector<const DTAGHHit*>::iterator jtagh;
      int mtagh = 0;
      for (jtagh = assoc_hits.begin(); jtagh != assoc_hits.end(); ++jtagh) {
         ntagh++;
         tagh_seqno[ntagh] = ntagh_per_counter[(*jtagh)->counter_id]++;
         tagh_counter[ntagh] = (*jtagh)->counter_id;
         tagh_peak[ntagh] = (*jtagh)->pulse_peak;
         tagh_pint[ntagh] = (*jtagh)->integral;
         tagh_tadc[ntagh] = (*jtagh)->time_fadc;
         tagh_toth[ntagh] = 999;
         tagh_ttdc[ntagh] = (*jtagh)->time_tdc;
         tagh_Etag[ntagh] = (*jtagh)->E;
         tagh_time[ntagh] = (*jtagh)->t;
         tagh_multi[ntagh] = ++mtagh;
         tagh_pmax[ntagh] = 999;
         tagh_ped[ntagh] = 999;
         tagh_qf[ntagh] = 999;
         tagh_bg[ntagh] = (*jtagh)->bg;
         tagh_has_adc[ntagh] = (*jtagh)->has_fADC;
         tagh_has_tdc[ntagh] = (*jtagh)->has_TDC;
         tagh_nped[ntagh] = 999;
         tagh_nint[ntagh] = 999;
         (*jtagh)->Get(digi_hits);
         for (atagh = digi_hits.begin(); atagh != digi_hits.end(); ++atagh) {
            if ((*atagh)->counter_id == (*itagh)->counter_id) {
               tagh_pmax[ntagh] = (*atagh)->pulse_peak;
               tagh_ped[ntagh] = (*atagh)->pedestal;
               tagh_qf[ntagh] = (*atagh)->QF;
               tagh_nped[ntagh] = (*atagh)->nsamples_pedestal;
               tagh_nint[ntagh] = (*atagh)->nsamples_integral;
               std::vector<const Df250PulseData*> pulse_data;
               (*atagh)->Get(pulse_data);
               std::vector<const Df250PulseData*>::iterator ptagh;
               for (ptagh = pulse_data.begin(); ptagh != pulse_data.end(); ++ptagh) {
                  tagh_toth[ntagh] = (*ptagh)->nsamples_over_threshold * 4;
                  // f_qpedestal = ((*ptagh)->QF_pedestal)? 1 : 0;
                  // f_latepulse = ((*ptagh)->QF_NSA_beyond_PTW)? 1 : 0;
                  // f_underflow = ((*ptagh)->QF_underflow)? 1 : 0;
                  // f_overflow = ((*ptagh)->QF_overflow)? 1 : 0;
                  // f_notpeak = ((*ptagh)->QF_vpeak_beyond_NSA)? 1 : 0;
                  // f_nopeak = ((*ptagh)->QF_vpeak_not_found)? 1 : 0;
                  // f_badped = ((*ptagh)->QF_bad_pedestal)? 1 : 0;
               }
            }
         }
      }
      std::vector<unsigned short> trace;
      std::vector<const Df250WindowRawData*>::iterator itrace;
      for (itrace = traces.begin(); itrace != traces.end(); ++itrace) {
         DTranslationTable::csc_t csc = {(*itrace)->rocid, (*itrace)->slot, (*itrace)->channel};
         const DTranslationTable::DChannelInfo chaninfo = GetDetectorIndex(ttab, csc);
         if (chaninfo.det_sys == DTranslationTable::TAGH) {
            if ((int)chaninfo.tagh.id == (*itagh)->counter_id) {
               trace = (*itrace)->samples;
            }
         }
      }
      tagh_raw_waveform.push_back(trace);
      ntagh++;
   }

   std::vector<const DBeamPhoton*> beams;
   eventLoop->Get(beams);
   std::vector<const DBeamPhoton*>::iterator ibeam;
   nbeam = 0;
   for (ibeam = beams.begin(); ibeam != beams.end(); ++ibeam) {
      beam_sys[nbeam] = (*ibeam)->dSystem;
      beam_E[nbeam] = (*ibeam)->energy();
      beam_t[nbeam] = (*ibeam)->time();
      beam_z[nbeam] = (*ibeam)->z();
      nbeam++;
   }

   std::vector<const DPSPair*> ps_pairs;
   eventLoop->Get(ps_pairs);
   std::vector<const DPSPair*>::iterator ipair;
   npairps = 0;
   for (ipair = ps_pairs.begin(); ipair != ps_pairs.end(); ++ipair) {
      Epair[npairps] = (*ipair)->left->E + (*ipair)->right->E;
      tpair[npairps] = ((*ipair)->left->t + (*ipair)->right->t)/2;
      psleft_peak[npairps] = (*ipair)->left->pulse_peak;
      psright_peak[npairps] = (*ipair)->right->pulse_peak;
      psleft_pint[npairps] = (*ipair)->left->integral;
      psright_pint[npairps] = (*ipair)->right->integral;
      psleft_time[npairps] = (*ipair)->left->t_tile;
      psright_time[npairps] = (*ipair)->right->t_tile;
      psEleft[npairps] = (*ipair)->left->E;
      psEright[npairps] = (*ipair)->right->E;
      pstleft[npairps] = (*ipair)->left->t;
      pstright[npairps] = (*ipair)->right->t;
      nleft_ps[npairps] = (*ipair)->right->ntiles;
      nright_ps[npairps] = (*ipair)->right->ntiles;

      std::vector<unsigned short> ltrace;
      std::vector<unsigned short> rtrace;
      std::vector<const Df250WindowRawData*>::iterator itrace;
      for (itrace = traces.begin(); itrace != traces.end(); ++itrace) {
         DTranslationTable::csc_t csc = {(*itrace)->rocid, (*itrace)->slot, (*itrace)->channel};
         const DTranslationTable::DChannelInfo chaninfo = GetDetectorIndex(ttab, csc);
         if (chaninfo.det_sys == DTranslationTable::PS && chaninfo.ps.side == 0) {
            if ((int)chaninfo.ps.id == (*ipair)->left->column) {
               ltrace = (*itrace)->samples;
            }
         }
         else if (chaninfo.det_sys == DTranslationTable::PS && chaninfo.ps.side == 1) {
            if ((int)chaninfo.ps.id == (*ipair)->right->column) {
               rtrace = (*itrace)->samples;
            }
         }
      }
      psleft_raw_waveform.push_back(ltrace);
      psright_raw_waveform.push_back(rtrace);
      ++npairps;
   }

   std::vector<const DPSCPair*> psc_pairs;
   eventLoop->Get(psc_pairs);
   std::vector<const DPSCPair*>::iterator icpair;
   int npsc_per_module[2][9] = {0};
   double tpsc_per_module[2][9] = {0};
   npairpsc = 0;
   for (icpair = psc_pairs.begin(); icpair != psc_pairs.end(); ++icpair) {
      int mod0 = (*icpair)->ee.first->module;
      int mod1 = (*icpair)->ee.second->module;
      if (tpsc_per_module[0][mod0] != (*icpair)->ee.first->time_tdc) {
         tpsc_per_module[0][mod0] = (*icpair)->ee.first->time_tdc;
         ++npsc_per_module[0][mod0];
      }
      if (tpsc_per_module[1][mod1] != (*icpair)->ee.second->time_tdc) {
         tpsc_per_module[1][mod1] = (*icpair)->ee.second->time_tdc;
         ++npsc_per_module[1][mod1];
      }
      pscleft_seqno[npairpsc] = npsc_per_module[0][mod0] - 1;
      pscright_seqno[npairpsc] = npsc_per_module[1][mod1] - 1;
      pscleft_module[npairpsc] = (*icpair)->ee.first->module;
      pscright_module[npairpsc] = (*icpair)->ee.second->module;
      pscleft_peak[npairpsc] = (*icpair)->ee.first->pulse_peak;
      pscright_peak[npairpsc] = (*icpair)->ee.second->pulse_peak;
      pscleft_pint[npairpsc] = (*icpair)->ee.first->integral;
      pscright_pint[npairpsc] = (*icpair)->ee.second->integral;
      pscleft_ttdc[npairpsc] = (*icpair)->ee.first->time_tdc;
      pscright_ttdc[npairpsc] = (*icpair)->ee.second->time_tdc;
      pscleft_tadc[npairpsc] = (*icpair)->ee.first->time_fadc;
      pscright_tadc[npairpsc] = (*icpair)->ee.second->time_fadc;
      pscleft_t[npairpsc] = (*icpair)->ee.first->t;
      pscright_t[npairpsc] = (*icpair)->ee.second->t;
      pscleft_ped[npairpsc] = 999;
      pscright_ped[npairpsc] = 999;
      pscleft_qf[npairpsc] = 999;
      pscright_qf[npairpsc] = 999;
      std::vector<const DPSCDigiHit*> digi_hits;
      (*icpair)->ee.first->Get(digi_hits);
      std::vector<const DPSCDigiHit*>::iterator apsc;
      for (apsc = digi_hits.begin(); apsc != digi_hits.end(); ++apsc) {
         pscleft_ped[npairpsc] = (*apsc)->pedestal;
         pscleft_qf[npairpsc] = (*apsc)->QF;
      }
      (*icpair)->ee.second->Get(digi_hits);
      for (apsc = digi_hits.begin(); apsc != digi_hits.end(); ++apsc) {
         pscright_ped[npairpsc] = (*apsc)->pedestal;
         pscright_qf[npairpsc] = (*apsc)->QF;
      }

      std::vector<unsigned short> ltrace;
      std::vector<unsigned short> rtrace;
      std::vector<const Df250WindowRawData*>::iterator itrace;
      for (itrace = traces.begin(); itrace != traces.end(); ++itrace) {
         DTranslationTable::csc_t csc = {(*itrace)->rocid, (*itrace)->slot, (*itrace)->channel};
         const DTranslationTable::DChannelInfo chaninfo = GetDetectorIndex(ttab, csc);
         if (chaninfo.det_sys == DTranslationTable::PSC) {
            if ((int)chaninfo.psc.id == (*icpair)->ee.first->module) {
               ltrace = (*itrace)->samples;
            }
            else if ((int)chaninfo.psc.id == (*icpair)->ee.second->module) {
               rtrace = (*itrace)->samples;
            }
         }
      }
      pscleft_raw_waveform.push_back(ltrace);
      pscright_raw_waveform.push_back(rtrace);
      npairpsc++;
   }

#ifdef VERBOSE
   printf("Filling pstags with ntagm=%d, ntagh=%d, npairps=%d, npairpsc=%d\n",
          ntagm, ntagh, npairps, npairpsc);
#endif
   pstags->Fill();

   unlock();
   return NOERROR;
}

jerror_t JEventProcessor_PStagstudy::erun(void) {
   return NOERROR;
}

jerror_t JEventProcessor_PStagstudy::fini(void) {
   lock();

   pstags->Write();

   unlock();
   return NOERROR;
}
