// $Id$
//
//    File: JEventProcessor_ecounter.h
// Created: Sat May  7 10:03:37 EDT 2016
// Creator: jonesrt (on Linux gluey.phys.uconn.edu 2.6.32-573.22.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_ecounter_
#define _JEventProcessor_ecounter_

#include <JANA/JEventProcessor.h>
#include <Compact_ROOT.h>
#define TH2I Compact_TH2I

#include <TTree.h>

class JEventProcessor_ecounter:public jana::JEventProcessor{
	public:
		JEventProcessor_ecounter();
		~JEventProcessor_ecounter();
		const char* className(void){return "JEventProcessor_ecounter";}

		TTree *triggers;
		struct triggers_t {
			unsigned int mask;
			unsigned int fp_mask;
            unsigned long int event_no;
            int is_event;
		} trig;

        TH2I *hmask[4];

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_ecounter_

