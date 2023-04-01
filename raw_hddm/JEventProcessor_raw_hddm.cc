//
// JEventProcessor_raw_hddm.cc
//
// JANA event processor plugin to convert raw evio data to HDDM files
//
// Author: Richard Jones

#include "JEventProcessor_raw_hddm.h"
#include "TRIGGER/DL1Trigger.h"
#include "DAQ/DL1Info.h"
#include <HDDM/DEventWriterHDDM.h>

#include "TAGGER/DTAGMDigiHit.h"
#include "TTAB/DTTabUtilities.h"

// for initializing plugins
extern "C" {
   void InitPlugin(JApplication *app)
	{
		InitJANAPlugin(app);
		app->AddProcessor(new JEventProcessor_raw_hddm(), true);
   }
} // "extern C"


// variables to control which triggers get read out

//-------------------------------
// init
//-------------------------------
jerror_t JEventProcessor_raw_hddm::init(void)
{
    return NOERROR;
}

//-------------------------------
// brun
//-------------------------------
jerror_t JEventProcessor_raw_hddm::brun(JEventLoop *locEventLoop, int32_t runnumber)
{
    return NOERROR;
}

//-------------------------------
// evnt
//-------------------------------
jerror_t JEventProcessor_raw_hddm::evnt(JEventLoop *locEventLoop, uint64_t eventnumber)
{
    // Get HDDM writer
    vector<const DEventWriterHDDM*> locEventWriterHDDMVector;
    locEventLoop->Get(locEventWriterHDDMVector);

    bool is_random_trigger = false;

	const DL1Trigger *trig = NULL;
	try {
		locEventLoop->GetSingle(trig);
	} catch (...) {}

    // parse the triggers we want to save
	if (trig) {

		if (trig->fp_trig_mask & 0x800) {  // Trigger front-panel bit 11
			// Periodic pulser trigger fired
			is_random_trigger = true;
		}

	} 

    // Save events to skim file
    locEventWriterHDDMVector[0]->Write_HDDMEvent(locEventLoop, "raw"); 


    const DTTabUtilities* locTTabUtilities = nullptr;
    locEventLoop->GetSingle(locTTabUtilities);

    // First loop over all TAGMDigiHits and make DTAGMHits out of them
    std::vector<const DTAGMDigiHit*> digihits;
    locEventLoop->Get(digihits);
    for (unsigned int i=0; i < digihits.size(); i++) {
        std::cout << "row=" << digihits[i]->row << ", column=" << digihits[i]->column << std::endl;

#if 0
        // Throw away hits with firmware errors (post-summer 2016 firmware)
        if(CHECK_FADC_ERRORS && !locTTabUtilities->CheckFADC250_NoErrors(digihit->QF))
            continue;

        // Get pedestal, prefer associated event pedestal if it exists,
        // otherwise, use the average pedestal from CCDB
        double pedestal = fadc_pedestals[digihit->row][digihit->column];
        double nsamples_integral = (double)digihit->nsamples_integral;
        double nsamples_pedestal = (double)digihit->nsamples_pedestal;

        // nsamples_pedestal should always be positive for valid data - err on the side of caution for now
        if(nsamples_pedestal == 0) {
            jerr << "DTAGMDigiHit with nsamples_pedestal == 0 !   Event = " << eventnumber << endl;
            continue;
        }

        // digihit->pedestal is the sum of "nsamples_pedestal" samples
        // Calculate the average pedestal per sample
        if ( (digihit->pedestal>0) && locTTabUtilities->CheckFADC250_PedestalOK(digihit->QF) ) {
            pedestal = (double)digihit->pedestal/nsamples_pedestal;
        }

        // Subtract pedestal from pulse peak
        if (digihit->pulse_time == 0 || digihit->pedestal == 0 || digihit->pulse_peak == 0) continue;
        double pulse_peak = digihit->pulse_peak - pedestal;
#endif
    }

    return NOERROR;
}

//-------------------------------
// erun
//-------------------------------
jerror_t JEventProcessor_raw_hddm::erun(void)
{
   return NOERROR;
}

//-------------------------------
// fini
//-------------------------------
jerror_t JEventProcessor_raw_hddm::fini(void)
{
   return NOERROR;
}
