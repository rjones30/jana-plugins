//
//    File: JEventProcessor_trackdump.cc
//


#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>
#include <TMath.h>

#include "JEventProcessor_trackdump.h"
#include <JANA/JApplication.h>
#include <TRACKING/DTrackWireBased.h>

using namespace std;
using namespace jana;

#include <GlueX.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <pthread.h>

pthread_mutex_t my_cout_mutex;

extern "C"{
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_trackdump());
  }
}

JEventProcessor_trackdump::JEventProcessor_trackdump()
{
}


JEventProcessor_trackdump::~JEventProcessor_trackdump() {
}

jerror_t JEventProcessor_trackdump::init(void) {
  pthread_mutex_init(&my_cout_mutex, NULL);
  return NOERROR;
}


jerror_t JEventProcessor_trackdump::brun(JEventLoop *eventLoop, int32_t runnumber) {
  // This is called whenever the run number changes
  return NOERROR;
}

jerror_t JEventProcessor_trackdump::evnt(JEventLoop *eventLoop, uint64_t eventnumber) {
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop-Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.

  std::vector<const DTrackWireBased*> tracks;
  eventLoop->Get(tracks);
  pthread_mutex_lock(&my_cout_mutex);
  printf("trackdump: event %ld, got %ld DTrackWireBased objects\n", eventnumber, tracks.size());
  int count=0;
  for (auto track : tracks) {
     std::vector<std::pair<std::string, std::string> > items;
     track->toStrings(items);
     std::cout << "properties of DTrackWireTrack " << ++count << ":" << std::endl;
     for (auto item : items) {
        std::cout << "  " << item.first << ": " << item.second << std::endl;
     }
     std::cout << std::endl;
  }
  pthread_mutex_unlock(&my_cout_mutex);
  return NOERROR;
}

jerror_t JEventProcessor_trackdump::erun(void) {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

jerror_t JEventProcessor_trackdump::fini(void) {
  // Called before program exit after event processing is finished.
  return NOERROR;
}
