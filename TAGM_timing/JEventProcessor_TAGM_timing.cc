//
//    File: JEventProcessor_TAGM_timing.cc
//

//#define VERBOSE 1

#include <iostream>
#include <stdint.h>
#include <vector>
#include <map>
#include <TMath.h>

#include "JEventProcessor_TAGM_timing.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;

#include <JANA/Services/JLockService.h>

#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGMDigiHit.h>
#include <TAGGER/DTAGMTDCDigiHit.h>
#include <TAGGER/DTAGHHit.h>
#include <TAGGER/DTAGHDigiHit.h>
#include <TAGGER/DTAGHTDCDigiHit.h>
#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulseData.h>
#include <RF/DRFTDCDigiTime.h>
#include <RF/DRFTime.h>
#include <TRIGGER/DL1Trigger.h>
#include <PID/DBeamPhoton.h>
#include <DAQ/DCODAROCInfo.h>
#include <GlueX.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

// main trigger is bit 0, ps triggers are bit 3
#define SELECT_TRIGGER_TYPE 0

// The individual fadc250 readout thresholds are stored separately and
// loaded into the frontend modules by the CODA daq at run start time.
// Since they are not saved in any database for automatic access, I 
// simply hard-code them here for specific run periods.

#define ROCTAGM1_WINTER_2023_V1_CNF 1

const int tagm_fadc250_channels(128);
const int tagm_fadc250_rocid(71);
const int tagm_fadc250_slot0(3);

#if defined ROCTAGM1_SPRING_2017_V2_CNF

int fadc250_readout_threshold[128] = {
  350, 310, 354, 397, 411, 419, 301, 343, 384, 348, 342, 335, 345, 333, 301, 280,
  293, 339, 367, 286, 291, 329, 359, 228, 262, 254, 284, 451, 347, 211, 195, 167,
  148, 144, 152, 143, 154, 222, 187, 160, 175, 185, 203, 157, 179, 175, 201, 233,
  232, 164, 170, 183, 406, 598, 582, 555, 663, 453, 447, 550, 598, 618, 706, 536,
  536, 663, 686, 526, 518, 647, 524, 730, 500, 543, 791, 528, 607, 696, 615, 526,
  198, 659, 390, 711, 581, 618, 718, 699, 454, 656, 597, 597, 554, 480, 577, 601,
  492, 581, 773, 666, 706, 559, 731, 749, 728, 600, 668, 654, 652, 619, 566, 423,
  673, 648, 608, 581, 522, 567, 596, 669, 405, 214, 1000, 1000, 1000, 1000, 1000, 1000
};

#elif defined ROCTAGM1_SPRING_2017_V3_CNF

int fadc250_readout_threshold[128] = {
  354, 310, 350, 419, 411, 397, 348, 342, 335, 345, 333, 384, 343, 301, 293, 280,
  301, 286, 267, 339, 359, 329, 291, 254, 262, 228, 347, 451, 284, 148, 144, 152,
  143, 154, 167, 195, 211, 160, 187, 222, 203, 185, 175, 175, 179, 157, 232, 233,
  201, 183, 170, 164, 582, 598, 406, 453, 663, 555, 598, 550, 447, 536, 706, 618,
  686, 663, 536, 647, 518, 526, 500, 730, 524, 528, 791, 543, 615, 696, 607, 659,
  198, 526, 581, 711, 390, 699, 718, 618, 597, 554, 480, 577, 601, 597, 656, 454,
  773, 581, 492, 559, 706, 666, 728, 749, 731, 654, 668, 600, 566, 619, 652, 608,
  581, 522, 567, 596, 648, 673, 423, 669, 405, 214, 1000, 1000, 1000, 1000, 1000, 1000
};

#elif defined ROCTAGM1_SPRING_2018_V1_CNF

int fadc250_readout_threshold[128] = {
  250, 250, 228, 251, 250, 250, 251, 250, 252, 250, 251, 250, 250, 251, 224, 251, 
  250, 245, 250, 250, 251, 251, 250, 150, 234, 192, 251, 250, 204, 154, 154, 154, 
  154, 154, 154, 183, 196, 220, 157, 188, 162, 168, 164, 153, 145, 143, 209, 190, 
  172, 148, 164, 170, 251, 250, 250, 250, 250, 251, 250, 251, 250, 250, 249, 250, 
  250, 250, 251, 250, 250, 250, 251, 250, 250, 250, 250, 250, 250, 249, 250, 250, 
  251, 250, 250, 250, 250, 250, 250, 250, 252, 250, 252, 251, 251, 250, 250, 250, 
  250, 250, 251, 250, 250, 250, 250, 251, 250, 250, 250, 250, 250, 250, 251, 251, 
  252, 250, 251, 252, 250, 250, 250, 250, 250, 208, 1000, 1000, 1000, 1000, 1000, 1000
};

#elif defined ROCTAGM1_FALL_2018_V1_CNF

int fadc250_readout_threshold[128] = {
  250, 250, 228, 251, 250, 250, 251, 250, 252, 250, 251, 250, 250, 251, 224, 251, 
  250, 245, 250, 250, 251, 251, 250, 150, 234, 192, 251, 250, 204, 154, 154, 154, 
  154, 154, 154, 183, 196, 220, 157, 188, 162, 168, 164, 153, 145, 143, 209, 190, 
  172, 148, 164, 170, 251, 250, 250, 250, 250, 251, 250, 251, 250, 250, 249, 250, 
  250, 250, 251, 250, 250, 250, 251, 250, 250, 250, 250, 250, 250, 249, 250, 250, 
  251, 250, 250, 250, 250, 250, 250, 250, 252, 250, 252, 251, 251, 250, 250, 250, 
  250, 250, 251, 250, 250, 250, 250, 251, 250, 250, 250, 250, 250, 250, 251, 251, 
  252, 250, 251, 252, 250, 250, 250, 250, 250, 208, 1000, 1000, 1000, 1000, 1000, 1000
};

#elif defined ROCTAGM1_SPRING_2019_V1_CNF

int fadc250_readout_threshold[128] = {
  420, 424, 396, 321, 381, 373, 456, 456, 456, 456, 456, 456, 415, 421, 416, 428,
  451, 451, 438, 424, 414, 423, 387, 426, 396, 416, 416, 437, 420, 394, 394, 394,
  394, 394, 394, 378, 348, 391, 415, 388, 381, 410, 372, 368, 361, 388, 429, 450,
  436, 420, 443, 437, 285, 297, 286, 246, 305, 305, 277, 304, 286, 316, 330, 319,
  311, 329, 290, 290, 302, 319, 145, 316, 304, 305, 347, 359, 315, 344, 325, 323,
  340, 328, 302, 313, 255, 338, 360, 247, 318, 318, 318, 318, 318, 318, 321, 261,
  328, 332, 325, 296, 329, 323, 331, 347, 360, 327, 307, 322, 305, 324, 360, 300,
  300, 300, 300, 300, 300, 353, 291, 331, 225, 203, 1000, 1000, 1000, 1000, 1000, 1000
};

#elif defined ROCTAGM1_FALL_2019_V1_CNF

int fadc250_readout_threshold[128] = {
  480, 482, 460, 318, 446, 398, 676, 616, 564, 518, 578, 518, 422, 400, 484, 502,
  528, 514, 454, 482, 468, 492, 466, 494, 464, 512, 482, 506, 498, 524, 476, 480,
  502, 492, 476, 446, 398, 420, 496, 426, 456, 504, 436, 432, 444, 424, 486, 534,
  478, 450, 486, 518, 272, 222, 244, 222, 258, 276, 222, 254, 252, 242, 280, 290,
  268, 260, 256, 262, 272, 278, 300, 276, 250, 266, 280, 332, 266, 326, 288, 286,
  290, 302, 236, 276, 226, 276, 284, 282, 404, 392, 264, 388, 474, 264, 300, 274,
  294, 296, 284, 226, 264, 294, 284, 302, 314, 264, 292, 298, 262, 276, 310, 340,
  316, 268, 422, 436, 268, 326, 248, 304, 162, 166, 999, 999, 999, 999, 999, 999
};

#elif defined ROCTAGM1_SPRING_2020_V1_CNF

int fadc250_readout_threshold[128] = {
  252, 252, 244, 188, 238, 220, 330, 306, 286, 268, 292, 268, 230, 220, 254, 260,
  270, 266, 242, 254, 246, 256, 246, 258, 246, 264, 254, 262, 260, 270, 250, 252,
  262, 256, 250, 238, 220, 228, 258, 230, 242, 262, 234, 232, 238, 230, 254, 274,
  252, 240, 254, 268, 168, 148, 158, 148, 162, 170, 150, 162, 160, 158, 172, 176,
  166, 164, 162, 164, 170, 172, 180, 170, 160, 166, 172, 192, 166, 190, 176, 174,
  176, 182, 154, 170, 150, 170, 174, 174, 222, 216, 166, 216, 250, 166, 180, 170,
  178, 178, 174, 150, 166, 178, 174, 182, 186, 166, 176, 180, 164, 170, 184, 196,
  186, 166, 228, 234, 166, 190, 158, 182, 124, 126, 999, 999, 999, 999, 999, 999
};

#elif defined ROCTAGM1_FALL_2021_V1_CNF

int fadc250_readout_threshold[128] = {
  252, 252, 244, 188, 238, 220, 330, 306, 286, 268, 292, 268, 230, 220, 254, 260,
  270, 266, 242, 254, 246, 256, 246, 258, 246, 264, 254, 262, 260, 270, 250, 252,
  262, 256, 250, 238, 220, 228, 258, 230, 242, 262, 234, 232, 238, 230, 254, 274,
  252, 240, 254, 268, 168, 148, 158, 148, 162, 170, 150, 162, 160, 158, 172, 176,
  166, 164, 162, 164, 170, 172, 180, 170, 160, 166, 172, 192, 166, 190, 176, 174,
  176, 182, 154, 170, 150, 170, 174, 174, 222, 216, 166, 216, 250, 166, 180, 170,
  178, 178, 174, 150, 166, 178, 174, 182, 186, 166, 176, 180, 164, 170, 184, 196,
  186, 166, 228, 234, 166, 190, 158, 182, 124, 126, 999, 999, 999, 999, 999, 999
};

#elif defined ROCTAGM1_SUMMER_2022_V1_CNF

int fadc250_readout_threshold[128] = {
  215, 212, 212, 178, 198, 211, 199, 199, 199, 199, 199, 223, 206, 172, 205, 210,
  213, 189, 203, 195, 179, 180, 174, 162, 162, 173, 150, 167, 166, 199, 199, 199,
  199, 199, 155, 151, 141, 140, 156, 144, 159, 159, 158, 152, 159, 157, 169, 166,
  162, 169, 163, 166, 133, 127, 130, 127, 132, 135, 125, 129, 128, 128, 135, 136,
  134, 131, 130, 131, 137, 137, 130, 199, 129, 132, 134, 146, 135, 138, 135, 135,
  140, 141, 136, 136, 127, 120, 133, 144, 199, 199, 199, 199, 199, 133, 137, 137,
  138, 146, 145, 142, 143, 134, 137, 133, 126, 132, 139, 136, 135, 135, 141, 199,
  199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199
};

#elif defined ROCTAGM1_FALL_2022_V1_CNF

int fadc250_readout_threshold[128] = {
  209, 208, 206, 176, 199, 207, 199, 199, 199, 199, 199, 218, 201, 170, 211, 206,
  213, 192, 191, 199, 183, 187, 180, 166, 169, 187, 167, 179, 172, 199, 199, 199,
  199, 199, 162, 168, 152, 146, 166, 151, 163, 174, 167, 156, 165, 164, 174, 177,
  172, 174, 172, 175, 133, 128, 135, 129, 132, 134, 126, 129, 128, 130, 132, 135,
  135, 132, 128, 133, 133, 137, 133, 136, 128, 128, 129, 134, 136, 147, 134, 134,
  143, 141, 139, 138, 128, 120, 128, 145, 199, 199, 199, 199, 199, 135, 137, 128,
  138, 138, 138, 141, 135, 133, 138, 132, 130, 132, 141, 138, 135, 133, 136, 199,
  199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199
};

#elif defined ROCTAGM1_WINTER_2023_V1_CNF

int fadc250_readout_threshold[128] = {
 267, 277, 275, 217, 258, 268, 287, 287, 287, 287, 287, 287, 254, 214, 276, 272,
 282, 249, 240, 258, 231, 235, 226, 207, 210, 233, 187, 207, 191, 193, 193, 193,
 193, 193, 193, 199, 172, 171, 201, 187, 197, 218, 209, 187, 202, 207, 218, 229,
 216, 220, 218, 229, 163, 152, 158, 151, 162, 164, 147, 155, 151, 154, 164, 172,
 160, 155, 149, 157, 159, 170, 155, 162, 152, 157, 157, 179, 159, 174, 164, 158,
 165, 167, 120, 160, 150, 131, 151, 166, 158, 158, 158, 158, 158, 158, 166, 163,
 163, 172, 171, 162, 163, 158, 168, 145, 145, 154, 168, 164, 155, 155, 159, 157,
 157, 157, 157, 157, 157, 160, 154, 231, 167, 171, 199, 199, 199, 199, 199, 199
};

#endif

int fadc250_rowcolumn[128] = {3,2,1,6,5,4,1009,2009,3009,4009,5009,9,8,7,12,11,
                              10,15,14,13,18,17,16,21,20,19,24,23,22,1027,2027,3027,
                              4027,5027,27,26,25,30,29,28,33,32,31,36,35,34,39,38,
                              37,42,41,40,45,44,43,48,47,46,51,50,49,54,53,52,
                              57,56,55,60,59,58,63,62,61,66,65,64,69,68,67,72,
                              71,70,75,74,73,78,77,76,1081,2081,3081,4081,5081,81,80,79,
                              84,83,82,87,86,85,90,89,88,93,92,91,96,95,94,1099,
                              2099,3099,4099,5099,99,98,97,100,101,102,
                              123,124,125,126,127,128};
std::map<int, int> fadc250_channel_from_rowcolumn;

extern "C"{
   void InitPlugin(JApplication *app) {
     InitJANAPlugin(app);
     app->Add(new JEventProcessor_TAGM_timing());
   }
}

JEventProcessor_TAGM_timing::JEventProcessor_TAGM_timing()
 : tags(0)
{
}


JEventProcessor_TAGM_timing::~JEventProcessor_TAGM_timing() {
}


void JEventProcessor_TAGM_timing::Init() {

   // lock all root operations
   auto app = GetApplication();
   auto lock_svc = app->GetService<JLockService>();
   lock_svc->RootWriteLock();
   tags = new TTree("tags", "TAGM timing study");
   tags->Branch("adctime", &adctime, "adctime/F");
   tags->Branch("tdctime", &tdctime, "tdctime/F");
   tags->Branch("hangtime", &hangtime, "hangtime/F");
   tags->Branch("pulsepeak", &pulsepeak, "pulsepeak/F");
   tags->Branch("pulseint", &pulseint, "pulseint/F");
   tags->Branch("pedestal", &pedestal, "pedestal/F");
   tags->Branch("lasttime", &lasttime, "lasttime/F");
   tags->Branch("lastpeak", &lastpeak, "lastpeak/F");
   tags->Branch("rftime", &rftime, "rftime/F");
   tags->Branch("f_qpedestal", &f_qpedestal, "f_qpedestal/I");
   tags->Branch("f_latepulse", &f_latepulse, "f_latepulse/I");
   tags->Branch("f_overflow", &f_overflow, "f_overflow/I");
   tags->Branch("f_underflow", &f_underflow, "f_underflow/I");
   tags->Branch("f_notpeak", &f_notpeak, "f_notpeak/I");
   tags->Branch("f_nopeak", &f_nopeak, "f_nopeak/I");
   tags->Branch("f_badped", &f_badped, "f_badped/I");
   tags->Branch("row", &row, "row/I");
   tags->Branch("column", &column, "column/I");
   tags->Branch("baseline", &baseline, "baseline/F");
   tags->Branch("threshold", &threshold, "threshold/F");
   tags->Branch("nraw", &nraw, "nraw/I[0,1024]");
   tags->Branch("sample", sample, "sample[nraw]/I");

   for (int i=0; i < tagm_fadc250_channels; ++i) {
      char name[99], title[99];
      hpedestal[i] = new TH1D((sprintf(name, "hpedestal_%d", i), name),
                              (sprintf(title, "pulse pedestal channel %d", i), title),
                              200, 0, 200);
      for (int j=0; j<25; ++j) {
         hpedestal[i]->Fill(100);
      }
   }

   lock_svc->RootUnLock();
}


void JEventProcessor_TAGM_timing::BeginRun(const std::shared_ptr<const JEvent>& event) {
   for (int fadc250chan=0; fadc250chan < tagm_fadc250_channels; ++fadc250chan) {
      int rowcol = fadc250_rowcolumn[fadc250chan];
      fadc250_channel_from_rowcolumn[rowcol] = fadc250chan;
   }
}


void JEventProcessor_TAGM_timing::Process(const std::shared_ptr<const JEvent>& event) {
   // This is called for every event. Use of common resources like writing
   // to a file or filling a histogram should be mutex protected. Using
   // loop->Get(...) to get reconstructed objects (and thereby activating the
   // reconstruction algorithm) should be done outside of any mutex lock
   // since multiple threads may call this method at the same time.

   // only examine physics triggers
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
#ifdef SELECT_TRIGGER_TYPE
   if ((trig_bits & (1 << SELECT_TRIGGER_TYPE)) == 0)
      return;
#endif

  auto app = GetApplication();
  auto lock_svc = app->GetService<JLockService>();

   rftime = 999;
   const double rf_period = 4.008;
   std::vector<const DRFTime*> rf_times;
   event->Get(rf_times, "TAGH");
   std::vector<const DRFTime*>::iterator irf;
   for (irf = rf_times.begin(); irf != rf_times.end(); ++irf) {
      int nper = round((*irf)->dTime / rf_period);
      rftime = (*irf)->dTime - nper * rf_period;
   }
 
   std::vector<const DTAGMHit*> tagm_hits;
   event->Get(tagm_hits, "Calib");
   std::vector<std::vector<float> > timelist;
   std::vector<std::vector<float> > peaklist;
   for (int i=0; i < tagm_fadc250_channels; ++i) {
      std::vector<float> evec;
      timelist.push_back(evec);
      peaklist.push_back(evec);
   }
   std::vector<const DTAGMHit*>::iterator itagm;
   for (itagm = tagm_hits.begin(); itagm != tagm_hits.end(); ++itagm) {
      column = (*itagm)->column;
      row = (*itagm)->row;
      int channel = fadc250_channel_from_rowcolumn[row * 1000 + column];
      threshold = fadc250_readout_threshold[channel];
      pulsepeak = (*itagm)->pulse_peak;
      pulseint = (*itagm)->integral;
      adctime = (*itagm)->time_fadc;
      tdctime = (*itagm)->time_tdc;
      pedestal = 0;
      hangtime = 0;
      lasttime = 0;
      lastpeak = 0;
      for (int i=0; i < (int)timelist[channel].size(); ++i) {
         if (timelist[channel][i] > lasttime) {
            lasttime = timelist[channel][i];
            lastpeak = peaklist[channel][i];
         }
      }
      timelist[channel].push_back(adctime);
      peaklist[channel].push_back(pulsepeak);
      std::vector<const DTAGMDigiHit*> digi_hits;
      (*itagm)->Get(digi_hits);
      std::vector<const DTAGMDigiHit*>::iterator atagm;
      for (atagm = digi_hits.begin(); atagm != digi_hits.end(); ++atagm) {
         if ((*atagm)->row == (*itagm)->row &&
             (*atagm)->column == (*itagm)->column)
         {
            std::vector<const Df250PulseData*> pulse_data;
            (*atagm)->Get(pulse_data);
            std::vector<const Df250PulseData*>::iterator ptagm;
            for (ptagm = pulse_data.begin(); ptagm != pulse_data.end(); ++ptagm) {
               hangtime = (*ptagm)->nsamples_over_threshold * 4;
               f_qpedestal = ((*ptagm)->QF_pedestal)? 1 : 0;
               f_latepulse = ((*ptagm)->QF_NSA_beyond_PTW)? 1 : 0;
               f_underflow = ((*ptagm)->QF_underflow)? 1 : 0;
               f_overflow = ((*ptagm)->QF_overflow)? 1 : 0;
               f_notpeak = ((*ptagm)->QF_vpeak_beyond_NSA)? 1 : 0;
               f_nopeak = ((*ptagm)->QF_vpeak_not_found)? 1 : 0;
               f_badped = ((*ptagm)->QF_bad_pedestal)? 1 : 0;
            }
            pedestal = (*atagm)->pedestal;
            pedestal /= (*atagm)->nsamples_pedestal;
            hpedestal[channel]->Fill(pedestal);
         }
      }
      int maxbin(hpedestal[channel]->GetMaximumBin());
      double bsum[2] = {0,0};
      for (int i=-1; i<2; ++i) {
         bsum[0] += hpedestal[channel]->GetBinContent(maxbin + i);
         bsum[1] += hpedestal[channel]->GetBinContent(maxbin + i) *
                    hpedestal[channel]->GetXaxis()->GetBinCenter(maxbin + i);
      }
      baseline = bsum[1] / bsum[0];

      nraw = 0;
      std::vector<const Df250WindowRawData*> traces;
      event->Get(traces);
      std::vector<const Df250WindowRawData*>::iterator itrace;
      for (itrace = traces.begin(); itrace != traces.end(); ++itrace) {
         if ((*itrace)->rocid == tagm_fadc250_rocid) {
            int f250chan = ((*itrace)->slot - tagm_fadc250_slot0) * 16 +
                            (*itrace)->channel;
            if (f250chan == channel) {
               nraw = (*itrace)->samples.size();
               for (int i=0; i < nraw; ++i)
                  sample[i] = (*itrace)->samples[i];
            }
         }
      }
      
      tags->Fill();
   }
   lock_svc->RootUnLock();
}

void JEventProcessor_TAGM_timing::EndRun() {
}

void JEventProcessor_TAGM_timing::Finish() {
}
