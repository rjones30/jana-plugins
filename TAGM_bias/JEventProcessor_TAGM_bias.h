// $Id$
//
//    File: JEventProcessor_TAGM_bias.h
// Created: Fri Nov 17 09:21:56 EST 2017
// Creator: aebarnes (on Linux ifarm1401.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_TAGM_bias_
#define _JEventProcessor_TAGM_bias_

#include <JANA/JEventProcessor.h>

class JEventProcessor_TAGM_bias:public jana::JEventProcessor{
	public:
		JEventProcessor_TAGM_bias();
		~JEventProcessor_TAGM_bias();
		const char* className(void){return "JEventProcessor_TAGM_bias";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_TAGM_bias_

