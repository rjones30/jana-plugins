//
//    File: JEventProcessor_PStagstudy.h
//

#ifndef _JEventProcessor_PStagstudy_
#define _JEventProcessor_PStagstudy_

#include <JANA/JEventProcessor.h>
#include <TTree.h>
#include <TH1S.h>
#include <vector>


class JEventProcessor_PStagstudy:public jana::JEventProcessor
{
 public:
   JEventProcessor_PStagstudy();
   ~JEventProcessor_PStagstudy();
   const char* className(void) {
     return "JEventProcessor_PStagstudy";
   }

   TTree *pstags;
   int lockflag;
   void lock();
   void unlock();

   int runno;
   int eventno;
   unsigned long int timestamp;

   int nrf;
   int rf_sys[999];
   double rf_time[999];

   int ntagm;
   int tagm_seqno[999];
   int tagm_channel[999];
   float tagm_Etag[999];
   float tagm_peak[999];
   float tagm_pint[999];
   float tagm_tadc[999];
   float tagm_toth[999];
   float tagm_ttdc[999];
   float tagm_time[999];
   float tagm_pmax[999];
   float tagm_ped[999];
   int tagm_multi[999];
   int tagm_qf[999];
   int tagm_bg[999];
   int tagm_has_adc[999];
   int tagm_has_tdc[999];
   int tagm_nped[999];
   int tagm_nint[999];
   std::vector<TH1S*> tagm_raw_waveform;

   int ntagh;
   int tagh_seqno[999];
   int tagh_counter[999];
   float tagh_Etag[999];
   float tagh_peak[999];
   float tagh_pint[999];
   float tagh_tadc[999];
   float tagh_toth[999];
   float tagh_ttdc[999];
   float tagh_time[999];
   float tagh_pmax[999];
   float tagh_ped[999];
   int tagh_multi[999];
   int tagh_qf[999];
   int tagh_bg[999];
   int tagh_has_adc[999];
   int tagh_has_tdc[999];
   int tagh_nped[999];
   int tagh_nint[999];
   std::vector<TH1S*> tagh_raw_waveform;

   int nbeam;
   int beam_sys[999];
   float beam_E[999];
   float beam_t[999];
   float beam_z[999];

   int npairps;
   float Epair[999];
   float tpair[999];
   float psleft_peak[999];
   float psright_peak[999];
   float psleft_pint[999];
   float psright_pint[999];
   float psleft_time[999];
   float psright_time[999];
   float psEleft[999];
   float psEright[999];
   float pstleft[999];
   float pstright[999];
   int nleft_ps[999];
   int nright_ps[999];
   std::vector<TH1S*> psleft_raw_waveform;
   std::vector<TH1S*> psright_raw_waveform;

   int npairpsc;
   int pscleft_seqno[999];
   int pscright_seqno[999];
   int pscleft_module[999];
   int pscright_module[999];
   float pscleft_peak[999];
   float pscright_peak[999];
   float pscleft_pint[999];
   float pscright_pint[999];
   float pscleft_ttdc[999];
   float pscright_ttdc[999];
   float pscleft_tadc[999];
   float pscright_tadc[999];
   float pscleft_t[999];
   float pscright_t[999];
   float pscleft_ped[999];
   float pscright_ped[999];
   int pscleft_qf[999];
   int pscright_qf[999];
   std::vector<TH1S*> pscleft_raw_waveform;
   std::vector<TH1S*> pscright_raw_waveform;

 private:
   jerror_t init(void);
   jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);
   jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);
   jerror_t erun(void);
   jerror_t fini(void);
};

#endif // _JEventProcessor_PStagstudy_
