// $Id$
//
//    File: JEventProcessor_ecounter.cc
// Created: Sat May  7 10:03:37 EDT 2016
// Creator: jonesrt (on Linux gluey.phys.uconn.edu 2.6.32-573.22.1.el6.x86_64 x86_64)
//

#include <iostream>
#include <stdint.h>

#include <TRIGGER/DL1Trigger.h>

#include "JEventProcessor_ecounter.h"
#include <JANA/JApplication.h>
#include <TRIGGER/DTrigger.h>

using namespace jana;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_ecounter());
}
} // "C"


//------------------
// JEventProcessor_ecounter (Constructor)
//------------------
JEventProcessor_ecounter::JEventProcessor_ecounter()
{}

//------------------
// ~JEventProcessor_ecounter (Destructor)
//------------------
JEventProcessor_ecounter::~JEventProcessor_ecounter()
{}

//------------------
// init
//------------------
jerror_t JEventProcessor_ecounter::init(void)
{
	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:
	//
	// japp->RootWriteLock();
	//  ... fill historgrams or trees ...
	// japp->RootUnLock();
	//

	japp->RootWriteLock();
	triggers = new TTree("triggers", "triggers");
	triggers->Branch("masks", &trig, "mask/i:fp_mask/i:event_no/l:is_event/i");
	hmask[0] = new TH2I("hmask0", "mask vs fp_mask", 3276, 0, 32768, 3276, 0, 32768);
	hmask[1] = new TH2I("hmask1", "mask vs mask", 3276, 0, 32768, 3276, 0, 32768);
	hmask[2] = new TH2I("hmask2", "fp_mask vs fp_mask", 3276, 0, 32768, 3276, 0, 32768);
	hmask[3] = new TH2I("hmask3", "fp_mask vs mask", 3276, 0, 32768, 3276, 0, 32768);
	japp->RootUnLock();
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_ecounter::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_ecounter::evnt(JEventLoop *eventLoop, uint64_t eventnumber)
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
	// japp->RootWriteLock();
	//  ... fill historgrams or trees ...
	// japp->RootUnLock();

	japp->RootWriteLock();
	const DL1Trigger *trig_words = 0;
	try {
		eventLoop->GetSingle(trig_words);
	} catch(...) {};
	if (trig_words) {
		trig.mask = trig_words->trig_mask;
		trig.fp_mask = trig_words->fp_trig_mask;
	}
	else {
		trig.mask = 0;
		trig.fp_mask = 0;
	}

	const DTrigger* locTrigger = NULL;
	eventLoop->GetSingle(locTrigger);
	if (trig_words == 0) {
	   ((DTrigger*)locTrigger)->Set_L1TriggerBits(0);
	   ((DTrigger*)locTrigger)->Set_L1FrontPanelTriggerBits(0); 
	}
	trig.event_no = eventnumber;
	trig.is_event = locTrigger->Get_IsPhysicsEvent();

	triggers->Fill();
	hmask[0]->Fill(trig.mask, trig.fp_mask);
	hmask[1]->Fill(trig.mask, trig.mask);
	hmask[2]->Fill(trig.fp_mask, trig.fp_mask);
	hmask[3]->Fill(trig.fp_mask, trig.mask);
	japp->RootUnLock();
	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_ecounter::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.

	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_ecounter::fini(void)
{
	// Called before program exit after event processing is finished.

	return NOERROR;
}
