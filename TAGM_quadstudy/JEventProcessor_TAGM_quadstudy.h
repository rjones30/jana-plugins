// $Id$
//
//    File: JEventProcessor_TAGM_quadstudy.h
// Created: Sat May  7 10:03:37 EDT 2016
// Creator: jonesrt (on Linux gluey.phys.uconn.edu 2.6.32-573.22.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_TAGM_quadstudy_
#define _JEventProcessor_TAGM_quadstudy_

#include <JANA/JEventProcessor.h>
#include <TTree.h>

class JEventProcessor_TAGM_quadstudy:public jana::JEventProcessor{
	public:
		JEventProcessor_TAGM_quadstudy();
		~JEventProcessor_TAGM_quadstudy();
		const char* className(void){return "JEventProcessor_TAGM_quadstudy";}

    TTree *ttagm;
    int tagm_hits;
    int tagm_row[999];
    int tagm_column[999];
    float tagm_pintegral[999];
    float tagm_tadc[999];
    int ps_hits;
    float ps_Epair[999];
    float ps_tleft[999];
    float ps_tright[999];

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_TAGM_quadstudy_

