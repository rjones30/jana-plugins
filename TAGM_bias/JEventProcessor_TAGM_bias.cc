// $Id$
//
//    File: JEventProcessor_TAGM_bias.cc
// Created: Fri Nov 17 09:21:56 EST 2017
// Creator: aebarnes (on Linux ifarm1401.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#include "JEventProcessor_TAGM_bias.h"
using namespace jana;

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
	app->AddProcessor(new JEventProcessor_TAGM_bias());
}
} // "C"


//------------------
// JEventProcessor_TAGM_bias (Constructor)
//------------------
JEventProcessor_TAGM_bias::JEventProcessor_TAGM_bias()
{

}

//------------------
// ~JEventProcessor_TAGM_bias (Destructor)
//------------------
JEventProcessor_TAGM_bias::~JEventProcessor_TAGM_bias()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_TAGM_bias::init(void)
{
	// This is called once at program startup. 

	for (uint32_t i = 0; i < NCOLUMNS; ++i)
	{
		h_spectra[i] = new TH1I(Form("h_spectra_%i", i+1),
					Form("Pulse spectrum for col %i;\
					Pulse height (ADC counts)", i+1),
					4200, 0, 4200);
	}

	h_source = new TH1I("source",";Datasource", 3, -0.5, 2.5);
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_TAGM_bias::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_TAGM_bias::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// loop->Get(mydataclasses);
	//
	// japp->RootFillLock(this);
	//  ... fill historgrams or trees ...
	// japp->RootFillUnLock(this);

	vector<const DTAGMDigiHit*> digi_hits;
	loop->Get( digi_hits );

	japp->RootFillLock(this);

	for (uint32_t i = 0; i < digi_hits.size(); ++i)
	{
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

		if (!pedestal > 0 || t == 0 || pulse_peak == 0 || !row == 0 || nsamp_ped == 0) continue;

		double ped = pedestal / nsamp_ped;
		double peak = pulse_peak - ped;

		h_source->Fill(datasource);

		h_spectra[col - 1]->Fill(peak);
	}

	japp->RootFillUnLock(this);

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_TAGM_bias::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_TAGM_bias::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}

