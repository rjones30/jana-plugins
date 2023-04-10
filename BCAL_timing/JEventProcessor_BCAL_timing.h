//
//    File: JEventProcessor_BCAL_timing.h
//

#ifndef _JEventProcessor_BCAL_timing_
#define _JEventProcessor_BCAL_timing_

#include <JANA/JEventProcessor.h>
#include <TTree.h>
#include <HDDM/hddm_s.hpp>
#include <HDDM/hddm_r.hpp>

class JEventProcessor_BCAL_timing:public jana::JEventProcessor
{
 public:
   JEventProcessor_BCAL_timing();
   ~JEventProcessor_BCAL_timing();
   const char* className(void) {
     return "JEventProcessor_BCAL_timing";
   }

   TTree *trest;
   int lockflag;
   void lock();
   void unlock();

   // event info
   int eventNo;
   int runNo;

   // shower and track parameters
   int track_ptype;      // track particle type, g3 convention
   float track_px;       // track x momentum, GeV/c
   float track_py;       // track y momentum, GeV/c
   float track_pz;       // track z momentum, GeV/c
   float track_x0;       // track vertex x, cm
   float track_y0;       // track vertex y, cm
   float track_z0;       // track vertex z, cm
   float track_t0;       // track vertex t, ns
   float shower_x;       // shower centroid x, cm
   float shower_y;       // shower centroid y, cm
   float shower_z;       // shower centroid z, cm
   float shower_E;       // shower energy, GeV
   float shower_t;       // shower time, ns
   float track_dEdxCDC;  // dEdX in CDC, keV/cm
   float track_dEdxFDC;  // dEdX in FDC, keV/cm
   float match_tflight;  // time of flight from vertex to bcal impact, ns
   float match_tlength;  // length along track from vertex to bcal impact, ns
   float match_deltaphi; // delta phi of shower from track impact, radians
   float match_deltaz;   // delta z of shower from track impact, cm

   // low-level bcal pulse data
#define MAX_BCAL_NHITS 999
   int nhits;            // number of bcal cells contributing to this shower
   int hit_modseclay[MAX_BCAL_NHITS]; // bcal module * 100 + sector * 10 + layer
   int hit_multi[MAX_BCAL_NHITS];  // hit multiplicity for this cell
   float uphit_E[MAX_BCAL_NHITS];   // upstream hit pulse peak, GeV
   float uphit_t[MAX_BCAL_NHITS];   // upstream hit time, ns
   int uphit_peak[MAX_BCAL_NHITS]; // upstream hit pulse peak, adc counts
   float uphit_tadc[MAX_BCAL_NHITS]; // upstream hit pulse time, ns
   int uphit_tadc_raw[MAX_BCAL_NHITS]; // upstream hit pulse time, 62ps clock
   float uphit_ttdc[MAX_BCAL_NHITS]; // upstream hit leading-edge time, ns
   int uphit_ttdc_raw[MAX_BCAL_NHITS]; // upstream hit leading-edge time, 50ps clock
   float dnhit_E[MAX_BCAL_NHITS];   // downstream hit pulse peak, GeV
   float dnhit_t[MAX_BCAL_NHITS];   // downstream hit time, ns
   int dnhit_peak[MAX_BCAL_NHITS]; // downstream hit pulse peak, adc counts
   float dnhit_tadc[MAX_BCAL_NHITS]; // downstream hit pulse time, ns
   int dnhit_tadc_raw[MAX_BCAL_NHITS]; // downstream hit pulse time, 62ps clock
   float dnhit_ttdc[MAX_BCAL_NHITS]; // downstream hit leading-edge time, ns
   int dnhit_ttdc_raw[MAX_BCAL_NHITS]; // downstream hit leading-edge time, 50ps clock

   int nrows_seen_by_this_instance;
   hddm_r::HDDM *make_rest_record(jana::JEventLoop *h);
   int32_t Convert_UnsignedIntToSigned(uint32_t) const;

 private:
   jerror_t init(void);
   jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);
   jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);
   jerror_t erun(void);
   jerror_t fini(void);
};

#endif // _JEventProcessor_BCAL_timing_
