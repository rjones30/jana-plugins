//
//    File: JEventProcessor_TAGM_timing.h
//

#ifndef _JEventProcessor_TAGM_timing_
#define _JEventProcessor_TAGM_timing_

#include <JANA/JEventProcessor.h>
#include <TTree.h>
#include <TH1D.h>


class JEventProcessor_TAGM_timing : public JEventProcessor {
 public:
   JEventProcessor_TAGM_timing();
   ~JEventProcessor_TAGM_timing();
   const char* className(void) { return "JEventProcessor_TAGM_timing"; }

   TTree *tags;

   // basic pulse parameters
   float adctime;      // fadc250 pulse time (ns) within PTW
   float tdctime;      // f1tdc leading-edge time (ns)
   float hangtime;     // pulse time-over-threshold (ns)
   float pulsepeak;    // peak adc value
   float pulseint;     // pulse integral
   float pedestal;     // pedestal in PTW
   float lasttime;     // adctime of previous pulse in window, or 0 if none
   float lastpeak;     // peak of previous pulse in window, or 0 if none
   float rftime;       // nearest rf clock edge (-2ns, 2ns)

   // fadc250 pulse flags
   int f_qpedestal;    // pedestal quality bit (0=good, 1=bad)
   int f_latepulse;    // pulse overlaps end of PTW (0=no, 1=yes)
   int f_overflow;     // at least one sample in [-NSB,+NSA] is underflow (0=no, 1=yes)
   int f_underflow;    // at least one sample in [-NSB,+NSA] is overflow (0=no, 1=yes)
   int f_notpeak;      // peak is later than sample +NSA-1 (0=no, 1=yes)
   int f_nopeak;       // peak finding algorithm failed (0=no, 1=yes)
   int f_badped;       // pedestal is outside valid range (0=no, 1=yes)

   // accummulated channel info
   int row;
   int column;
   float baseline;     // MP pedestal value in recent history for this channel
   float threshold;    // pulse readout threshold for this channel

   // raw waveform, if available
   int nraw;           // number of raw fadc250 samples, one per 4ns
   int sample[1024];   // sample[0],..,sample[nraw-1]

   // histograms for accummulated statistics
   TH1D *hpedestal[128];

 private:
   void Init() override;
   void BeginRun(const std::shared_ptr<const JEvent>& event) override;
   void Process(const std::shared_ptr<const JEvent>& event) override;
   void EndRun() override;
   void Finish() override;
};

#endif // _JEventProcessor_TAGM_timing_
