// $Id$
//
//    File: JEventProcessor_TAGM_quadstudy.cc
// Created: Sat May  7 10:03:37 EDT 2016
// Creator: jonesrt (on Linux gluey.phys.uconn.edu 2.6.32-573.22.1.el6.x86_64 x86_64)
//

#include <iostream>
#include <stdint.h>
#include <vector>
#include <TMath.h>

#include "JEventProcessor_TAGM_quadstudy.h"
#include <JANA/JApplication.h>

using namespace jana;

#include <TAGGER/DTAGMHit.h>
#include <RF/DRFTDCDigiTime.h>
#include <PAIR_SPECTROMETER/DPSPair.h>
#include <GlueX.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_TAGM_quadstudy());
}
} // "C"


//------------------
// JEventProcessor_TAGM_quadstudy (Constructor)
//------------------
JEventProcessor_TAGM_quadstudy::JEventProcessor_TAGM_quadstudy()
 : ttagm(0)
{

}

//------------------
// ~JEventProcessor_TAGM_quadstudy (Destructor)
//------------------
JEventProcessor_TAGM_quadstudy::~JEventProcessor_TAGM_quadstudy()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_TAGM_quadstudy::init(void)
{
	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:
	//
	// japp->RootWriteLock();
	//  ... fill historgrams or trees ...
	// japp->RootUnLock();
	//

    // lock all root operations
    japp->RootWriteLock();

    ttagm = new TTree("ttagm", "tagm-ps coincidences");
    ttagm->Branch("tagm_hits", &tagm_hits, "tagm_hits/I");
    ttagm->Branch("tagm_row", tagm_row, "tagm_row[tagm_hits]/I");
    ttagm->Branch("tagm_column", tagm_column, "tagm_column[tagm_hits]/I");
    ttagm->Branch("tagm_pintegral", tagm_pintegral, "tagm_pintegral[tagm_hits]/F");
    ttagm->Branch("tagm_tadc", tagm_tadc, "tagm_tadc[tagm_hits]/F");
    ttagm->Branch("ps_hits", &ps_hits, "ps_hits/I");
    ttagm->Branch("ps_Epair", ps_Epair, "ps_Epair[ps_hits]/F");
    ttagm->Branch("ps_tleft", ps_tleft, "ps_tleft[ps_hits]/F");
    ttagm->Branch("ps_tright", ps_tright, "ps_tright[ps_hits]/F");

    // unlock
    japp->RootUnLock();

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_TAGM_quadstudy::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_TAGM_quadstudy::evnt(JEventLoop *loop, uint64_t eventnumber)
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

    tagm_hits = 0;
    std::vector<const DTAGMHit*> tagms;
    loop->Get(tagms);
    std::vector<const DTAGMHit*>::iterator ittagm;
    for (ittagm = tagms.begin(); ittagm != tagms.end(); ++ittagm) {
        tagm_row[tagm_hits] = (*ittagm)->row;
        tagm_column[tagm_hits] = (*ittagm)->column;
        tagm_pintegral[tagm_hits] = (*ittagm)->integral;
        tagm_tadc[tagm_hits] = (*ittagm)->time_fadc;
        ++tagm_hits;
    }
    ps_hits = 0;
    std::vector<const DPSPair*> pairs;
    loop->Get(pairs);
    std::vector<const DPSPair*>::iterator itpair;
    for (itpair = pairs.begin(); itpair != pairs.end(); ++itpair) {
        ps_Epair[ps_hits] = (*itpair)->ee.first->E + (*itpair)->ee.second->E;
        ps_tleft[ps_hits] = (*itpair)->ee.first->t;
        ps_tright[ps_hits] = (*itpair)->ee.second->t;
        ++ps_hits;
    }

    if (tagm_hits * ps_hits > 0)
        ttagm->Fill();
    japp->RootUnLock();

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_TAGM_quadstudy::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_TAGM_quadstudy::fini(void)
{
	// Called before program exit after event processing is finished.
    japp->RootWriteLock();
    ttagm->Write();
    japp->RootUnLock();
	return NOERROR;
}

