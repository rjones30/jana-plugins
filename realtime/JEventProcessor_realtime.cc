//
//    File: JEventProcessor_realtime.cc
//

//#define VERBOSE 1

#include "JEventProcessor_realtime.h"
#include <JANA/JApplication.h>
#include <DAQ/DCODAEventInfo.h>
#include <DAQ/DCODAControlEvent.h>
#include <DAQ/DBeamCurrent.h>
#include <DAQ/DTSscalers.h>
#include "rxConnection.h"

using namespace std;
using namespace jana;

#include <ctime>

extern "C"{
   void InitPlugin(JApplication *app) {
     InitJANAPlugin(app);
     app->AddProcessor(new JEventProcessor_realtime());
   }
}

JEventProcessor_realtime::JEventProcessor_realtime()
{
}

JEventProcessor_realtime::~JEventProcessor_realtime()
{
}

jerror_t JEventProcessor_realtime::init(void)
{
   bc_factory = new DBeamCurrent_factory();
   bc_factory->init();
   return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_realtime::brun(JEventLoop *eventLoop, int32_t runnumber)
{
   bc_factory->brun(eventLoop, runnumber);

   string RCDB_CONNECTION;
   if (getenv("RCDB_CONNECTION")!= NULL)
      RCDB_CONNECTION = getenv("RCDB_CONNECTION");
   else
      RCDB_CONNECTION = "mysql://rcdb@hallddb.jlab.org/rcdb";   // default to outward-facing MySQL DB

   rcdb::rxConnection rcdbconn(RCDB_CONNECTION);
   epoch_reference  = rcdbconn.GetRunStartTime(runnumber);
   return NOERROR;
}

jerror_t JEventProcessor_realtime::evnt(JEventLoop *eventLoop, uint64_t eventnumber)
{
   std::vector<const DCODAControlEvent*> controls;
   eventLoop->Get(controls);
   std::vector<const DCODAControlEvent*>::iterator ictrl;
   for (ictrl = controls.begin(); ictrl != controls.end(); ++ictrl) {
      std::cout << "found control event with unix_time "
                << (*ictrl)->unix_time << std::endl;
      epoch_reference = (*ictrl)->unix_time;
   } 

   bc_factory->evnt(eventLoop, eventnumber);
   double ticks_per_sec = bc_factory->ticks_per_sec;

   std::vector<const DBeamCurrent*> currents;
   eventLoop->Get(currents);
   std::vector<const DBeamCurrent*>::iterator icur;
   for (icur = currents.begin(); icur != currents.end(); ++icur) {
      std::cout << "found DBeamCurrent with unix_time "
                << (*ictrl)->unix_time << std::endl;
      beamcurrent = (*icur)->Ibeam;
      bctime = (*icur)->t;
   } 

   std::vector<const DCODAEventInfo*> event_info;
   eventLoop->Get(event_info);
   if (event_info.size() == 0)
      return NOERROR;

   runno = event_info[0]->run_number;
   eventno = event_info[0]->event_number;
   timestamp = event_info[0]->avg_timestamp;

   std::time_t epoch_time = epoch_reference + timestamp/ticks_per_sec;
   std::tm* tm_local = std::localtime(&epoch_time);
   char timestr[100];
   std::strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_local);

   std::cout << "run " << runno
	     << ", event " << eventno
	     << ", timestamp " << timestamp
	     << ", epoch_reference " << epoch_reference
	     << ", unix time " << epoch_reference + timestamp/ticks_per_sec
	     << ", time string " << timestr
	     << std::endl;

   std::vector<const DTSscalers*> scalers;
   eventLoop->Get(scalers);
   std::vector<const DTSscalers*>::iterator isc;
   for (isc = scalers.begin(); isc != scalers.end(); ++isc) {
      std::cout << "scalers found with time " << (*isc)->time << std::endl;
   }
   return NOERROR;
}

jerror_t JEventProcessor_realtime::erun(void)
{
   return NOERROR;
}

jerror_t JEventProcessor_realtime::fini(void)
{
   return NOERROR;
}
