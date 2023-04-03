//
//    File: JEventProcessor_BCAL_timing.cc
//

//#define VERBOSE 1

#include <iostream>
#include <stdint.h>
#include <TMath.h>
#include <vector>

#include "JEventProcessor_BCAL_timing.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;

#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulseData.h>
#include <TRIGGER/DL1Trigger.h>
#include <GlueX.h>

#include <TRACKING/DTrackFitter.h>
#include <PID/DMCReaction.h>
#include <PID/DBeamPhoton.h>
#include <TRACKING/DMCThrown.h>
#include <FCAL/DFCALShower.h>
#include <CCAL/DCCALShower.h>
#include <PID/DNeutralShower.h>
#include <PID/DDetectorMatches.h>
#include <BCAL/DBCALShower.h>
#include <TOF/DTOFPoint.h>
#include <START_COUNTER/DSCHit.h>
#include <TRACKING/DTrackTimeBased.h>
#include <TRIGGER/DTrigger.h>
#include <DIRC/DDIRCPmtHit.h>
#include <HDDM/DEventHitStatistics.h>
#include <RF/DRFTime.h>
#include <BCAL/DBCALCluster.h>
#include <BCAL/DBCALPoint.h>
#include <BCAL/DBCALUnifiedHit.h>
#include <BCAL/DBCALTDCHit.h>
#include <BCAL/DBCALHit.h>

#define REST_WRITE_TRACK_EXIT_PARAMS 1
#define REST_WRITE_DIRC_HITS 1
#define REST_WRITE_TRACK_EXIT_PARAMS 1
#define REST_WRITE_CCAL_SHOWERS 1

// main trigger is bit 0, ps triggers are bit 3
#define SELECT_TRIGGER_TYPE 0

extern "C"{
   void InitPlugin(JApplication *app) {
     InitJANAPlugin(app);
     app->AddProcessor(new JEventProcessor_BCAL_timing());
   }
}

JEventProcessor_BCAL_timing::JEventProcessor_BCAL_timing()
 : trest(0), lockflag(0)
{
}


JEventProcessor_BCAL_timing::~JEventProcessor_BCAL_timing()
{
}

void JEventProcessor_BCAL_timing::lock()
{
   japp->RootWriteLock();
   if (lockflag) {
      std::cerr << "Error in JEventProcessor_BCAL_timing::lock() - "
                   "deadlock!" << std::endl;
   }
   else {
      lockflag = 1;
   }
}

void JEventProcessor_BCAL_timing::unlock()
{
   if (lockflag == 0) {
      std::cerr << "Error in JEventProcessor_BCAL_timing::evnt() - "
                   "double unlock!" << std::endl;
   }
   else {
      lockflag = 0;
   }
   japp->RootUnLock();
}

jerror_t JEventProcessor_BCAL_timing::init(void)
{
   lock();

   trest = new TTree("trest", "BCAL timing study");
   trest->Branch("eventNo", &eventNo, "eventNo/I");
   trest->Branch("runNo", &runNo, "runNo/I");
   trest->Branch("track_ptype", &track_ptype, "track_ptype/I");
   trest->Branch("track_px", &track_px, "track_px/F");
   trest->Branch("track_py", &track_py, "track_py/F");
   trest->Branch("track_pz", &track_pz, "track_pz/F");
   trest->Branch("track_x0", &track_x0, "track_x/F");
   trest->Branch("track_y0", &track_y0, "track_y/F");
   trest->Branch("track_z0", &track_z0, "track_z/F");
   trest->Branch("track_t0", &track_t0, "track_t/F");
   trest->Branch("shower_x", &shower_x, "shower_/F");
   trest->Branch("shower_y", &shower_y, "shower_/F");
   trest->Branch("shower_z", &shower_z, "shower_/F");
   trest->Branch("shower_E", &shower_E, "shower_/F");
   trest->Branch("shower_t", &shower_t, "shower_/F");
   trest->Branch("track_dEdxCDC", &track_dEdxCDC, "track_dEdxCDC/F");
   trest->Branch("track_dEdxFDC", &track_dEdxFDC, "track_dEdxFDC/F");
   trest->Branch("match_tflight", &match_tflight, "match_tfligh/F");
   trest->Branch("match_tlength", &match_tlength, "match_tlengt/F");
   trest->Branch("match_deltaphi", &match_deltaphi, "match_deltaph/F");
   trest->Branch("match_deltaz", &match_deltaz, "match_delta/F");
   
   trest->Branch("nhits", &nhits, "nhits/I[0,999]");
   trest->Branch("hit_modseclay", hit_modseclay, "hit_modseclay[nhits]/I");
   trest->Branch("hit_multi", hit_multi, "hit_multi[nhits]/I");
   trest->Branch("uphit_E", uphit_E, "uphit_E[nhits]/F");
   trest->Branch("uphit_t", uphit_t, "uphit_t[nhits]/F");
   trest->Branch("uphit_peak", uphit_peak, "uphit_peak[nhits]/I");
   trest->Branch("uphit_tadc", uphit_tadc, "uphit_tadc[nhits]/F");
   trest->Branch("uphit_tadc_raw", uphit_tadc_raw, "uphit_tadc_raw[nhits]/I");
   trest->Branch("uphit_ttdc", uphit_ttdc, "uphit_ttdc[nhits]/F");
   trest->Branch("uphit_ttdc_raw", uphit_ttdc_raw, "uphit_ttdc_raw[nhits]/I");
   trest->Branch("dnhit_E", dnhit_E, "dnhit_E[nhits]/F");
   trest->Branch("dnhit_t", dnhit_t, "dnhit_t[nhits]/F");
   trest->Branch("dnhit_peak", dnhit_peak, "dnhit_peak[nhits]/I");
   trest->Branch("dnhit_tadc", dnhit_tadc, "dnhit_tadc[nhits]/F");
   trest->Branch("dnhit_tadc_raw", dnhit_tadc_raw, "dnhit_tadc_raw[nhits]/I");
   trest->Branch("dnhit_ttdc", dnhit_ttdc, "dnhit_ttdc[nhits]/F");
   trest->Branch("dnhit_ttdc_raw", dnhit_ttdc_raw, "dnhit_ttdc_raw[nhits]/I");

   unlock();
   return NOERROR;
}


jerror_t JEventProcessor_BCAL_timing::brun(JEventLoop *eventLoop, int32_t runnumber)
{
   return NOERROR;
}

jerror_t JEventProcessor_BCAL_timing::evnt(JEventLoop *eventLoop, uint64_t eventnumber)
{
   // This is called for every event. Use of common resources like writing
   // to a file or filling a histogram should be mutex protected. Using
   // loop-Get(...) to get reconstructed objects (and thereby activating the
   // reconstruction algorithm) should be done outside of any mutex lock
   // since multiple threads may call this method at the same time.

   // only examine physics triggers
   const DL1Trigger *trig_words = 0;
   uint32_t trig_mask, fp_trig_mask;
   try {
      eventLoop->GetSingle(trig_words);
   } catch(...) {};
   if (trig_words) {
      trig_mask = trig_words->trig_mask;
      fp_trig_mask = trig_words->fp_trig_mask;
   }
   else {
      trig_mask = 0;
      fp_trig_mask = 0;
   }
   int trig_bits = fp_trig_mask > 0 ? 10 + fp_trig_mask : trig_mask;
#ifdef SELECT_TRIGGER_TYPE
   if ((trig_bits & (1 << SELECT_TRIGGER_TYPE)) == 0)
      return NOERROR;
#endif

   hddm_r::HDDM *record = make_rest_record(eventLoop);
   hddm_r::ChargedTrackList charged_tracks = record->getChargedTracks();
   hddm_r::BcalShowerList bcal_showers = record->getBcalShowers();
   hddm_r::BcalMatchParamsList bcal_matches = record->getBcalMatchParamses();
   hddm_r::BcalDOCAtoTrackList bcal_docas = record->getBcalDOCAtoTracks();
   std::vector<const DBCALShower*> dbcal_showers;
   std::vector<const DBCALCluster*> dbcal_clusters;
   std::vector<const DBCALPoint*> dbcal_points;
   std::vector<const DBCALUnifiedHit*> dbcal_hits;
   std::vector<const DBCALTDCHit*> dbcal_tdcs;
   std::vector<const DBCALHit*> dbcal_adcs;
   eventLoop->Get(dbcal_showers);
   eventNo = record->getReconstructedPhysicsEvent().getEventNo();
   runNo = record->getReconstructedPhysicsEvent().getRunNo();

   // remember to delete record when done, otherwise memleak!

   nhits = 0;
   hddm_r::BcalMatchParamsList::iterator match;
   for (match = bcal_matches.begin(); match != bcal_matches.end(); ++match) {
      int itrack = match->getTrack();
      int ishower = match->getShower();
      match_tflight = match->getTflight();
      match_tlength = match->getPathlength();
      match_deltaphi = match->getDeltaphi();
      match_deltaz = match->getDeltaz();
      track_ptype = charged_tracks(itrack).getPtype();
      track_px = charged_tracks(itrack).getTrackFit().getPx();
      track_py = charged_tracks(itrack).getTrackFit().getPy();
      track_pz = charged_tracks(itrack).getTrackFit().getPz();
      track_x0 = charged_tracks(itrack).getTrackFit().getX0();
      track_y0 = charged_tracks(itrack).getTrackFit().getY0();
      track_z0 = charged_tracks(itrack).getTrackFit().getZ0();
      track_t0 = charged_tracks(itrack).getTrackFit().getT0();
      track_dEdxCDC = charged_tracks(itrack).getDEdxDC().getDEdxCDC();
      track_dEdxFDC = charged_tracks(itrack).getDEdxDC().getDEdxFDC();
      shower_x = bcal_showers(ishower).getX();
      shower_y = bcal_showers(ishower).getY();
      shower_z = bcal_showers(ishower).getZ();
      shower_E = bcal_showers(ishower).getE();
      shower_t = bcal_showers(ishower).getT();
      dbcal_showers[ishower]->Get(dbcal_clusters);
      for (int ic=0; ic < (int)dbcal_clusters.size(); ++ic) {
         dbcal_clusters[ic]->Get(dbcal_points);
         for (int ip=0; ip < (int)dbcal_points.size(); ++ip) {
            dbcal_points[ip]->Get(dbcal_hits);
            for (int ih=0; ih < (int)dbcal_hits.size(); ++ih) {
               dbcal_hits[ih]->Get(dbcal_adcs);
               dbcal_hits[ih]->Get(dbcal_tdcs);
               hit_modseclay[nhits] = dbcal_hits[ih]->module * 100 +
                                      dbcal_hits[ih]->sector * 10 +
                                      dbcal_hits[ih]->layer;
               hit_multi[nhits] = 1;
               uphit_E[nhits] = -999;
               uphit_t[nhits] = -999;
               uphit_peak[nhits] = -999;
               uphit_tadc[nhits] = -999;
               uphit_tadc_raw[nhits] = -999;
               uphit_ttdc[nhits] = -999;
               uphit_ttdc_raw[nhits] = -999;
               dnhit_E[nhits] = -999;
               dnhit_t[nhits] = -999;
               dnhit_peak[nhits] = -999;
               dnhit_tadc[nhits] = -999;
               dnhit_tadc_raw[nhits] = -999;
               dnhit_ttdc[nhits] = -999;
               dnhit_ttdc_raw[nhits] = -999;
               nhits += 1;
               int iend = dbcal_hits[ih]->end;
               int ihit;
               for (ihit = 0; ihit < nhits; ++ihit) {
                  if (hit_modseclay[ihit] == hit_modseclay[nhits]) {
                     hit_multi[ihit] += 1;
                     if ((iend == 0 && uphit_peak[ihit] <= 0) ||
                         (iend != 0 && dnhit_peak[ihit] <= 0))
                     {
                        nhits -= 1;
                        break;
                     }
                  }
               }
               if (iend == 0) {               // upstream
                  uphit_E[ihit] = dbcal_hits[ih]->E;
                  uphit_t[ihit] = dbcal_hits[ih]->t;
                  if (dbcal_adcs.size() > 0) {
                     uphit_peak[ihit] = dbcal_adcs[0]->pulse_peak;
                     uphit_tadc[ihit] = dbcal_adcs[0]->t;
                     uphit_tadc_raw[ihit] = dbcal_adcs[0]->t_raw;
                  }
                  if (dbcal_tdcs.size() > 0) {
                     uphit_ttdc[ihit] = dbcal_tdcs[0]->t;
                     uphit_ttdc_raw[ihit] = dbcal_tdcs[0]->t_raw;
                  }
               }
               else {                         // downstream
                  dnhit_E[ihit] = dbcal_hits[ih]->E;
                  dnhit_t[ihit] = dbcal_hits[ih]->t;
                  if (dbcal_adcs.size() > 0) {
                     dnhit_peak[ihit] = dbcal_adcs[0]->pulse_peak;
                     dnhit_tadc[ihit] = dbcal_adcs[0]->t;
                     dnhit_tadc_raw[ihit] = dbcal_adcs[0]->t_raw;
                  }
                  if (dbcal_tdcs.size() > 0) {
                     dnhit_ttdc[ihit] = dbcal_tdcs[0]->t;
                     dnhit_ttdc_raw[ihit] = dbcal_tdcs[0]->t_raw;
                  }
               }
            }
         }
      }
   }

   delete record;

   lock();
   if (nhits > 0) {
      trest->Fill();
   }
   unlock();

   return NOERROR;
}

jerror_t JEventProcessor_BCAL_timing::erun(void)
{
   return NOERROR;
}

jerror_t JEventProcessor_BCAL_timing::fini(void)
{
   lock();
   trest->Write();
   unlock();

   return NOERROR;
}

hddm_r::HDDM *JEventProcessor_BCAL_timing::make_rest_record(JEventLoop *locEventLoop)
{
	std::vector<const DMCReaction*> reactions;
	locEventLoop->Get(reactions);

	std::vector<const DRFTime*> rftimes;
	locEventLoop->Get(rftimes);

	std::vector<const DBeamPhoton*> locBeamPhotons;
	locEventLoop->Get(locBeamPhotons);

	std::vector<const DBeamPhoton*> locBeamPhotons_TAGGEDMCGEN;
	locEventLoop->Get(locBeamPhotons_TAGGEDMCGEN, "TAGGEDMCGEN");

	std::vector<const DFCALShower*> fcalshowers;
	locEventLoop->Get(fcalshowers);

	std::vector<const DBCALShower*> bcalshowers;
	locEventLoop->Get(bcalshowers);

	std::vector<const DCCALShower*> ccalshowers;
	if(REST_WRITE_CCAL_SHOWERS) {
	    locEventLoop->Get(ccalshowers);
	}
  
	std::vector<const DTOFPoint*> tofpoints;
	locEventLoop->Get(tofpoints);

	std::vector<const DSCHit*> starthits;
	locEventLoop->Get(starthits);

	std::vector<const DTrackTimeBased*> tracks;
	locEventLoop->Get(tracks);

	std::vector<const DDetectorMatches*> locDetectorMatches;
	locEventLoop->Get(locDetectorMatches);

	std::vector<const DDIRCPmtHit*> locDIRCPmtHits;
	locEventLoop->Get(locDIRCPmtHits);

	std::vector<const DEventHitStatistics*> hitStats;
	locEventLoop->Get(hitStats);

	std::vector<const DTrigger*> locTriggers;
	locEventLoop->Get(locTriggers);

	//Check to see if there are any objects to write out.  If so, don't write out an empty event
	bool locOutputDataPresentFlag = false;
	if((!reactions.empty()) || (!locBeamPhotons.empty()) || (!tracks.empty()))
		locOutputDataPresentFlag = true;
	else if((!fcalshowers.empty()) || (!bcalshowers.empty()) || (!tofpoints.empty()) || (!starthits.empty()))
		locOutputDataPresentFlag = true;
	//don't need to check detector matches: no matches if none of the above objects
	if(!locOutputDataPresentFlag)
		return 0; //had correct response to data

	hddm_r::HDDM *locRecord = new hddm_r::HDDM();
	hddm_r::ReconstructedPhysicsEventList res = locRecord->addReconstructedPhysicsEvents(1);

	// load the run and event numbers
	JEvent& event = locEventLoop->GetJEvent();
	res().setRunNo(event.GetRunNumber());
	//The REST type for this is int64_t, whereas the event type is uint64_t
	//This copy is lazy: the last bit is lost.  However, we should never need the last bit.
	res().setEventNo(event.GetEventNumber());

	// push any DMCReaction objects to the output record
	for (size_t i=0; i < reactions.size(); i++)
	{
		hddm_r::ReactionList rea = res().addReactions(1);
		rea().setType(reactions[i]->type);
		rea().setWeight(reactions[i]->weight);
		rea().setEbeam(reactions[i]->beam.energy());
		rea().setTargetType(reactions[i]->target.PID());

		if(i != 0)
			break;

		std::vector<const DMCThrown*> throwns;
		locEventLoop->Get(throwns);
		hddm_r::VertexList ver = rea().getVertices();
		DLorentzVector locPreviousX4(-9.9E9, -9.9E9, -9.9E9, -9.9E9);
		for(size_t it=0; it < throwns.size(); ++it)
		{
			DLorentzVector locThrownX4(throwns[it]->position(), throwns[it]->time());
			if((locThrownX4.T() != locPreviousX4.T()) || (locThrownX4.Vect() != locPreviousX4.Vect()))
			{
				//new vertex
				ver = rea().addVertices(1);
				hddm_r::OriginList ori = ver().addOrigins(1);
				ori().setT(locThrownX4.T());
				ori().setVx(locThrownX4.X());
				ori().setVy(locThrownX4.Y());
				ori().setVz(locThrownX4.Z());
				locPreviousX4 = locThrownX4;
			}

			hddm_r::ProductList pro = ver().addProducts(1);
			pro().setId(throwns[it]->myid);
			pro().setParentId(throwns[it]->parentid);
			int pdgtype = throwns[it]->pdgtype;
			if (pdgtype == 0)
				pdgtype = PDGtype((Particle_t)throwns[it]->type);
			pro().setPdgtype(pdgtype);
			hddm_r::MomentumList mom = pro().addMomenta(1);
			mom().setE(throwns[it]->energy());
			mom().setPx(throwns[it]->px());
			mom().setPy(throwns[it]->py());
			mom().setPz(throwns[it]->pz());
		}
	}

	// push any DRFTime objects to the output record
	for (size_t i=0; i < rftimes.size(); i++)
	{
		hddm_r::RFtimeList rf = res().addRFtimes(1);
		rf().setTsync(rftimes[i]->dTime);
	}

	// push any DBeamPhoton objects to the output record
	for(size_t loc_i = 0; loc_i < locBeamPhotons.size(); ++loc_i)
	{
		if(locBeamPhotons[loc_i]->dSystem == SYS_TAGM)
		{
			hddm_r::TagmBeamPhotonList locTagmBeamPhotonList = res().addTagmBeamPhotons(1);
			locTagmBeamPhotonList().setT(locBeamPhotons[loc_i]->time());
			locTagmBeamPhotonList().setE(locBeamPhotons[loc_i]->energy());
			hddm_r::TagmChannelList locTagmChannelList = locTagmBeamPhotonList().addTagmChannels(1);
			locTagmChannelList().setColumn(locBeamPhotons[loc_i]->dCounter);
		}
		else if(locBeamPhotons[loc_i]->dSystem == SYS_TAGH)
		{
			hddm_r::TaghBeamPhotonList locTaghBeamPhotonList = res().addTaghBeamPhotons(1);
			locTaghBeamPhotonList().setT(locBeamPhotons[loc_i]->time());
			locTaghBeamPhotonList().setE(locBeamPhotons[loc_i]->energy());
			hddm_r::TaghChannelList locTaghChannelList = locTaghBeamPhotonList().addTaghChannels(1);
			locTaghChannelList().setCounter(locBeamPhotons[loc_i]->dCounter);
		}
	}
	for(size_t loc_i = 0; loc_i < locBeamPhotons_TAGGEDMCGEN.size(); ++loc_i)
	{
		if(locBeamPhotons_TAGGEDMCGEN[loc_i]->dSystem == SYS_TAGM)
		{
			hddm_r::TagmBeamPhotonList locTagmBeamPhotonList = res().addTagmBeamPhotons(1);
			locTagmBeamPhotonList().setJtag("TAGGEDMCGEN");
			locTagmBeamPhotonList().setT(locBeamPhotons_TAGGEDMCGEN[loc_i]->time());
			locTagmBeamPhotonList().setE(locBeamPhotons_TAGGEDMCGEN[loc_i]->energy());
			hddm_r::TagmChannelList locTagmChannelList = locTagmBeamPhotonList().addTagmChannels(1);
			locTagmChannelList().setColumn(locBeamPhotons_TAGGEDMCGEN[loc_i]->dCounter);
		}
		else if(locBeamPhotons_TAGGEDMCGEN[loc_i]->dSystem == SYS_TAGH)
		{
			hddm_r::TaghBeamPhotonList locTaghBeamPhotonList = res().addTaghBeamPhotons(1);
			locTaghBeamPhotonList().setJtag("TAGGEDMCGEN");
			locTaghBeamPhotonList().setT(locBeamPhotons_TAGGEDMCGEN[loc_i]->time());
			locTaghBeamPhotonList().setE(locBeamPhotons_TAGGEDMCGEN[loc_i]->energy());
			hddm_r::TaghChannelList locTaghChannelList = locTaghBeamPhotonList().addTaghChannels(1);
			locTaghChannelList().setCounter(locBeamPhotons_TAGGEDMCGEN[loc_i]->dCounter);
		}
	}

	// push any DFCALShower objects to the output record
	for (size_t i=0; i < fcalshowers.size(); i++)
	{
		hddm_r::FcalShowerList fcal = res().addFcalShowers(1);
		DVector3 pos = fcalshowers[i]->getPosition();
		fcal().setX(pos(0));
		fcal().setY(pos(1));
		fcal().setZ(pos(2));
		fcal().setT(fcalshowers[i]->getTime());
		fcal().setE(fcalshowers[i]->getEnergy());
		fcal().setXerr(fcalshowers[i]->xErr());
		fcal().setYerr(fcalshowers[i]->yErr());
		fcal().setZerr(fcalshowers[i]->zErr());
		fcal().setTerr(fcalshowers[i]->tErr());
		fcal().setEerr(fcalshowers[i]->EErr());
		fcal().setXycorr(fcalshowers[i]->XYcorr());
		fcal().setXzcorr(fcalshowers[i]->XZcorr());
		fcal().setYzcorr(fcalshowers[i]->YZcorr());
		fcal().setEzcorr(fcalshowers[i]->EZcorr());
		fcal().setTzcorr(fcalshowers[i]->ZTcorr());

		// further correlations (an extension of REST format so code is different.)
		hddm_r::FcalCorrelationsList locFcalCorrelationsList = fcal().addFcalCorrelationses(1);
		locFcalCorrelationsList().setEtcorr(fcalshowers[i]->ETcorr());
		locFcalCorrelationsList().setExcorr(fcalshowers[i]->EXcorr());
		locFcalCorrelationsList().setEycorr(fcalshowers[i]->EYcorr());
		locFcalCorrelationsList().setTxcorr(fcalshowers[i]->XTcorr());
		locFcalCorrelationsList().setTycorr(fcalshowers[i]->YTcorr());

        // add in classification based on MVA		
        //hddm_r::FcalShowerClassificationList locFcalShowerClassificationList = fcal().addFcalShowerClassifications(1);
        //        locFcalShowerClassificationList().setClassifierOuput(fcalshowers[i]->getClassifierOutput());

        // add in shower properties used for MVA algorithm, etc.
        
        hddm_r::FcalShowerPropertiesList locFcalShowerPropertiesList = fcal().addFcalShowerPropertiesList(1);
        locFcalShowerPropertiesList().setDocaTrack(fcalshowers[i]->getDocaTrack());
        locFcalShowerPropertiesList().setTimeTrack(fcalshowers[i]->getTimeTrack());
        locFcalShowerPropertiesList().setSumU(fcalshowers[i]->getSumU());
        locFcalShowerPropertiesList().setSumV(fcalshowers[i]->getSumV());
        locFcalShowerPropertiesList().setE1E9(fcalshowers[i]->getE1E9());
        locFcalShowerPropertiesList().setE9E25(fcalshowers[i]->getE9E25());
        hddm_r::FcalShowerNBlocksList locFcalShowerNBlocksList = fcal().addFcalShowerNBlockses(1);
	locFcalShowerNBlocksList().setNumBlocks(fcalshowers[i]->getNumBlocks());

    }
            

	// push any DBCALShower objects to the output record
	for (size_t i=0; i < bcalshowers.size(); i++)
	{
		hddm_r::BcalShowerList bcal = res().addBcalShowers(1);
		DVector3 pos(bcalshowers[i]->x,bcalshowers[i]->y,bcalshowers[i]->z);
		bcal().setX(bcalshowers[i]->x);
		bcal().setY(bcalshowers[i]->y);
		bcal().setZ(bcalshowers[i]->z);
		bcal().setT(bcalshowers[i]->t);
		bcal().setE(bcalshowers[i]->E);
		bcal().setXerr(bcalshowers[i]->xErr());
		bcal().setYerr(bcalshowers[i]->yErr());
		bcal().setZerr(bcalshowers[i]->zErr());
		bcal().setTerr(bcalshowers[i]->tErr());
		bcal().setEerr(bcalshowers[i]->EErr());
		bcal().setXycorr(bcalshowers[i]->XYcorr());
		bcal().setXzcorr(bcalshowers[i]->XZcorr());
		bcal().setYzcorr(bcalshowers[i]->YZcorr());
		bcal().setEzcorr(bcalshowers[i]->EZcorr());
		bcal().setTzcorr(bcalshowers[i]->ZTcorr());

		// further correlations (an extension of REST format so code is different.)
		hddm_r::BcalCorrelationsList locBcalCorrelationsList = bcal().addBcalCorrelationses(1);
		locBcalCorrelationsList().setEtcorr(bcalshowers[i]->ETcorr());
		locBcalCorrelationsList().setExcorr(bcalshowers[i]->EXcorr());
		locBcalCorrelationsList().setEycorr(bcalshowers[i]->EYcorr());
		locBcalCorrelationsList().setTxcorr(bcalshowers[i]->XTcorr());
		locBcalCorrelationsList().setTycorr(bcalshowers[i]->YTcorr());

		hddm_r::PreshowerList locPreShowerList = bcal().addPreshowers(1);
		locPreShowerList().setPreshowerE(bcalshowers[i]->E_preshower);

		hddm_r::WidthList locWidthList = bcal().addWidths(1);
		locWidthList().setSigLong(bcalshowers[i]->sigLong);
		locWidthList().setSigTrans(bcalshowers[i]->sigTrans);
		locWidthList().setSigTheta(bcalshowers[i]->sigTheta);

		//N_cell
		hddm_r::BcalClusterList bcalcluster = bcal().addBcalClusters(1);
		bcalcluster().setNcell(bcalshowers[i]->N_cell);

		hddm_r::BcalLayersList bcallayerdata = bcal().addBcalLayerses(1);
		bcallayerdata().setE_L2(bcalshowers[i]->E_L2);
		bcallayerdata().setE_L3(bcalshowers[i]->E_L3);
		bcallayerdata().setE_L4(bcalshowers[i]->E_L4);
		bcallayerdata().setRmsTime(bcalshowers[i]->rmsTime);
	}

    // push any DCCALShower objects to the output record                                                                                         
    for (size_t i=0; i < ccalshowers.size(); i++)                                                                                                
    {                                                                                                                                            
        hddm_r::CcalShowerList ccal = res().addCcalShowers(1);                                                                               
        ccal().setX(ccalshowers[i]->x);                                                                                                      
        ccal().setY(ccalshowers[i]->y);                                                                                                      
        ccal().setZ(ccalshowers[i]->z);                                                                                                      
        ccal().setT(ccalshowers[i]->time);                                                                                                   
        ccal().setE(ccalshowers[i]->E);                                                                                                      
        ccal().setEmax(ccalshowers[i]->Emax);
        ccal().setTerr(ccalshowers[i]->sigma_t);                                                                                             
        ccal().setEerr(ccalshowers[i]->sigma_E);                                                                                             
        ccal().setChi2(ccalshowers[i]->chi2);
        ccal().setX1(ccalshowers[i]->x1);
        ccal().setY1(ccalshowers[i]->y1);
                                                                                                     
        ccal().setType(ccalshowers[i]->type);                                                                                                
        ccal().setDime(ccalshowers[i]->dime);                                                                                             
        ccal().setId(ccalshowers[i]->id);                                                                                                
        ccal().setIdmax(ccalshowers[i]->idmax);                                                                                                
    }                                                                                                                                            

	// push any DTOFPoint objects to the output record
	for (size_t i=0; i < tofpoints.size(); i++)
	{
		hddm_r::TofPointList tof = res().addTofPoints(1);
		tof().setX(tofpoints[i]->pos(0));
		tof().setY(tofpoints[i]->pos(1));
		tof().setZ(tofpoints[i]->pos(2));
		tof().setT(tofpoints[i]->t);
		tof().setDE(tofpoints[i]->dE);

		//Status //Assume compiler optimizes multiplication
		hddm_r::TofStatusList tofstatus = tof().addTofStatuses(1);
		int locStatus = tofpoints[i]->dHorizontalBar + 45*tofpoints[i]->dVerticalBar;
		locStatus += 45*45*tofpoints[i]->dHorizontalBarStatus + 45*45*4*tofpoints[i]->dVerticalBarStatus;
		tofstatus().setStatus(locStatus);
		// Energy deposition for each plane
		hddm_r::TofEnergyDepositionList tofEnergyDeposition = tof().addTofEnergyDepositions(1);
		tofEnergyDeposition().setDE1(tofpoints[i]->dE1);
		tofEnergyDeposition().setDE2(tofpoints[i]->dE2);
	}

	// push any DSCHit objects to the output record
	for (size_t i=0; i < starthits.size(); i++)
	{
		hddm_r::StartHitList hit = res().addStartHits(1);
		hit().setSector(starthits[i]->sector);
		hit().setT(starthits[i]->t);
		hit().setDE(starthits[i]->dE);
	}

	if(REST_WRITE_DIRC_HITS) {
		// push any DDIRCPmtHit objects to the output record
		for (size_t i=0; i < locDIRCPmtHits.size(); i++)
		{
			hddm_r::DircHitList hit = res().addDircHits(1);
			hit().setCh(locDIRCPmtHits[i]->ch);
			hit().setT(locDIRCPmtHits[i]->t);
			hit().setTot(locDIRCPmtHits[i]->tot);
		}
	}

	// push any DTrackTimeBased objects to the output record
	for (size_t i=0; i < tracks.size(); ++i)
	{
	

		hddm_r::ChargedTrackList tra = res().addChargedTracks(1);
		tra().setCandidateId(tracks[i]->candidateid);
		tra().setPtype(tracks[i]->PID());

		hddm_r::TrackFitList fit = tra().addTrackFits(1);
		fit().setNdof(tracks[i]->Ndof);
		fit().setChisq(tracks[i]->chisq);
		fit().setX0(tracks[i]->x());
		fit().setY0(tracks[i]->y());
		fit().setZ0(tracks[i]->z());
		fit().setPx(tracks[i]->px());
		fit().setPy(tracks[i]->py());
		fit().setPz(tracks[i]->pz());
		fit().setT0(tracks[i]->time());
		fit().setT0err(0.0);
		fit().setT0det(SYS_CDC);

		const TMatrixFSym& errors = *(tracks[i]->TrackingErrorMatrix().get());
		fit().setE11(errors(0,0));
		fit().setE12(errors(0,1));
		fit().setE13(errors(0,2));
		fit().setE14(errors(0,3));
		fit().setE15(errors(0,4));
		fit().setE22(errors(1,1));
		fit().setE23(errors(1,2));
		fit().setE24(errors(1,3));
		fit().setE25(errors(1,4));
		fit().setE33(errors(2,2));
		fit().setE34(errors(2,3));
		fit().setE35(errors(2,4));
		fit().setE44(errors(3,3));
		fit().setE45(errors(3,4));
		fit().setE55(errors(4,4));	

		hddm_r::TrackFlagsList myflags = tra().addTrackFlagses(1);
		myflags().setFlags(tracks[i]->flags);

		hddm_r::HitlayersList locHitLayers = tra().addHitlayerses(1);
		locHitLayers().setCDCrings(tracks[i]->dCDCRings);
		locHitLayers().setFDCplanes(tracks[i]->dFDCPlanes);

		vector<const DCDCTrackHit*> locCDCHits;
		tracks[i]->Get(locCDCHits);
		vector<const DFDCPseudo*> locFDCHits;
		tracks[i]->Get(locFDCHits);

		hddm_r::ExpectedhitsList locExpectedHits = tra().addExpectedhitses(1);
		//locExpectedHits().setMeasuredCDChits(locCDCHits.size());
		//locExpectedHits().setMeasuredFDChits(locFDCHits.size());
		locExpectedHits().setMeasuredCDChits(tracks[i]->measured_cdc_hits_on_track);
		locExpectedHits().setMeasuredFDChits(tracks[i]->measured_fdc_hits_on_track);
		//locExpectedHits().setMeasuredCDChits(tracks[i]->cdc_hit_usage.total_hits);
		//locExpectedHits().setMeasuredFDChits(tracks[i]->fdc_hit_usage.total_hits);
		locExpectedHits().setExpectedCDChits(tracks[i]->potential_cdc_hits_on_track);
		locExpectedHits().setExpectedFDChits(tracks[i]->potential_fdc_hits_on_track);

		hddm_r::McmatchList locMCMatches = tra().addMcmatchs(1);
		locMCMatches().setIthrown(tracks[i]->dMCThrownMatchMyID);
		locMCMatches().setNumhitsmatch(tracks[i]->dNumHitsMatchedToThrown);

		if (tracks[i]->dNumHitsUsedFordEdx_FDC + tracks[i]->dNumHitsUsedFordEdx_CDC > 0)
		{
			hddm_r::DEdxDCList elo = tra().addDEdxDCs(1);
			elo().setNsampleFDC(tracks[i]->dNumHitsUsedFordEdx_FDC);
			elo().setNsampleCDC(tracks[i]->dNumHitsUsedFordEdx_CDC);
			elo().setDxFDC(tracks[i]->ddx_FDC);
			elo().setDxCDC(tracks[i]->ddx_CDC);
			elo().setDEdxFDC(tracks[i]->ddEdx_FDC);
			elo().setDEdxCDC(tracks[i]->ddEdx_CDC);
			hddm_r::CDCAmpdEdxList elo2 = elo().addCDCAmpdEdxs(1);
			elo2().setDxCDCAmp(tracks[i]->ddx_CDC_amp);
			elo2().setDEdxCDCAmp(tracks[i]->ddEdx_CDC_amp);
            for (int it=0; it < (int)tracks[i]->ddx_CDC_trunc.size() &&
                           it < (int)tracks[i]->ddx_CDC_amp_trunc.size(); ++it)
            {
               hddm_r::CDCdEdxTruncList elo3 = elo().addCDCdEdxTruncs(1);
               elo3().setNtrunc(it);
               elo3().setDx(tracks[i]->ddx_CDC_trunc[it]);
               elo3().setDEdx(tracks[i]->ddEdx_CDC_trunc[it]);
               elo3().setDxAmp(tracks[i]->ddx_CDC_amp_trunc[it]);
               elo3().setDEdxAmp(tracks[i]->ddEdx_CDC_amp_trunc[it]);
            }
            for (int it=0; it < (int)tracks[i]->ddx_FDC_trunc.size() &&
                           it < (int)tracks[i]->ddx_FDC_amp_trunc.size(); ++it)
            {
               hddm_r::FDCdEdxTruncList elo3 = elo().addFDCdEdxTruncs(1);
               elo3().setNtrunc(it);
               elo3().setDx(tracks[i]->ddx_FDC_trunc[it]);
               elo3().setDEdx(tracks[i]->ddEdx_FDC_trunc[it]);
               elo3().setDxAmp(tracks[i]->ddx_FDC_amp_trunc[it]);
               elo3().setDEdxAmp(tracks[i]->ddEdx_FDC_amp_trunc[it]);
            }
		}
		if (REST_WRITE_TRACK_EXIT_PARAMS){
		  if (tracks[i]->extrapolations.find(SYS_NULL) != tracks[i]->extrapolations.end()) {
		    vector<DTrackFitter::Extrapolation_t>extraps=tracks[i]->extrapolations.at(SYS_NULL);
		    if (extraps.size()>0){
		      hddm_r::ExitParamsList locExitParams = tra().addExitParamses(1);
		      DVector3 pos=extraps[0].position;
		      DVector3 mom=extraps[0].momentum;
		      locExitParams().setX1(pos.X());
		      locExitParams().setY1(pos.Y());
		      locExitParams().setZ1(pos.Z()); 
		      locExitParams().setPx1(mom.X());
		      locExitParams().setPy1(mom.Y());
		      locExitParams().setPz1(mom.Z());
		      locExitParams().setT1(extraps[0].t);
		    }
		  }
		}
		
	}

	// push any DEventHitStatistics objects to the output record
	if (hitStats.size() > 0)
	{
		hddm_r::HitStatisticsList stats = res().addHitStatisticses(1);
		hddm_r::StartCountersList starts = stats().addStartCounterses(1);
		starts().setCount(hitStats[0]->start_counters);
		hddm_r::CdcStrawsList straws = stats().addCdcStrawses(1);
		straws().setCount(hitStats[0]->cdc_straws);
		hddm_r::FdcPseudosList pseudos = stats().addFdcPseudoses(1);
		pseudos().setCount(hitStats[0]->fdc_pseudos);
		hddm_r::BcalCellsList cells = stats().addBcalCellses(1);
		cells().setCount(hitStats[0]->bcal_cells);
		hddm_r::FcalBlocksList blocks = stats().addFcalBlockses(1);
		blocks().setCount(hitStats[0]->fcal_blocks);
		hddm_r::CcalBlocksList bloccs = stats().addCcalBlockses(1);
		bloccs().setCount(hitStats[0]->ccal_blocks);
		hddm_r::DircPMTsList pmts = stats().addDircPMTses(1);
		pmts().setCount(hitStats[0]->dirc_PMTs);
	}

	// push any DTrigger objects to the output record
	for (size_t i=0; i < locTriggers.size(); ++i)
	{
		hddm_r::TriggerList trigger = res().addTriggers(1);
		trigger().setL1_trig_bits(Convert_UnsignedIntToSigned(locTriggers[i]->Get_L1TriggerBits()));
		trigger().setL1_fp_trig_bits(Convert_UnsignedIntToSigned(locTriggers[i]->Get_L1FrontPanelTriggerBits()));
		
		// trigger energy sums
		hddm_r::TriggerEnergySumsList triggerEnergySum = trigger().addTriggerEnergySumses(1);
		triggerEnergySum().setBCALEnergySum(locTriggers[i]->Get_GTP_BCALEnergy());
		triggerEnergySum().setFCALEnergySum(locTriggers[i]->Get_GTP_FCALEnergy());

	}

	// push any DDetectorMatches objects to the output record
	for(size_t loc_i = 0; loc_i < locDetectorMatches.size(); ++loc_i)
	{
		hddm_r::DetectorMatchesList matches = res().addDetectorMatcheses(1);
		for(size_t loc_j = 0; loc_j < tracks.size(); ++loc_j)
		{
			vector<shared_ptr<const DBCALShowerMatchParams>> locBCALShowerMatchParamsVector;
			locDetectorMatches[loc_i]->Get_BCALMatchParams(tracks[loc_j], locBCALShowerMatchParamsVector);
			for(size_t loc_k = 0; loc_k < locBCALShowerMatchParamsVector.size(); ++loc_k)
			{
				hddm_r::BcalMatchParamsList bcalList = matches().addBcalMatchParamses(1);
				bcalList().setTrack(loc_j);

				const DBCALShower* locBCALShower = locBCALShowerMatchParamsVector[loc_k]->dBCALShower;
				size_t locBCALindex = 0;
				for(; locBCALindex < bcalshowers.size(); ++locBCALindex)
				{
					if(bcalshowers[locBCALindex] == locBCALShower)
						break;
				}
				bcalList().setShower(locBCALindex);

				bcalList().setDeltaphi(locBCALShowerMatchParamsVector[loc_k]->dDeltaPhiToShower);
				bcalList().setDeltaz(locBCALShowerMatchParamsVector[loc_k]->dDeltaZToShower);
				bcalList().setDx(locBCALShowerMatchParamsVector[loc_k]->dx);
				bcalList().setPathlength(locBCALShowerMatchParamsVector[loc_k]->dPathLength);
				bcalList().setTflight(locBCALShowerMatchParamsVector[loc_k]->dFlightTime);
				bcalList().setTflightvar(locBCALShowerMatchParamsVector[loc_k]->dFlightTimeVariance);
			}

			vector<shared_ptr<const DFCALShowerMatchParams>> locFCALShowerMatchParamsVector;
			locDetectorMatches[loc_i]->Get_FCALMatchParams(tracks[loc_j], locFCALShowerMatchParamsVector);
			for (size_t loc_k = 0; loc_k < locFCALShowerMatchParamsVector.size(); ++loc_k)
			{
				hddm_r::FcalMatchParamsList fcalList = matches().addFcalMatchParamses(1);
				fcalList().setTrack(loc_j);

				const DFCALShower* locFCALShower = locFCALShowerMatchParamsVector[loc_k]->dFCALShower;
				size_t locFCALindex = 0;
				for(; locFCALindex < fcalshowers.size(); ++locFCALindex)
				{
					if(fcalshowers[locFCALindex] == locFCALShower)
						break;
				}
				fcalList().setShower(locFCALindex);

				fcalList().setDoca(locFCALShowerMatchParamsVector[loc_k]->dDOCAToShower);
				fcalList().setDx(locFCALShowerMatchParamsVector[loc_k]->dx);
				fcalList().setPathlength(locFCALShowerMatchParamsVector[loc_k]->dPathLength);
				fcalList().setTflight(locFCALShowerMatchParamsVector[loc_k]->dFlightTime);
				fcalList().setTflightvar(locFCALShowerMatchParamsVector[loc_k]->dFlightTimeVariance);
			}

			vector<shared_ptr<const DFCALSingleHitMatchParams>> locFCALSingleHitMatchParamsVector;
			locDetectorMatches[loc_i]->Get_FCALSingleHitMatchParams(tracks[loc_j], locFCALSingleHitMatchParamsVector);
			for (size_t loc_k = 0; loc_k < locFCALSingleHitMatchParamsVector.size(); ++loc_k)
			{
				hddm_r::FcalSingleHitMatchParamsList fcalSingleHitList = matches().addFcalSingleHitMatchParamses(1);
				fcalSingleHitList().setTrack(loc_j);

				fcalSingleHitList().setEhit(locFCALSingleHitMatchParamsVector[loc_k]->dEHit);
				fcalSingleHitList().setThit(locFCALSingleHitMatchParamsVector[loc_k]->dTHit);
				fcalSingleHitList().setDoca(locFCALSingleHitMatchParamsVector[loc_k]->dDOCAToHit);
				fcalSingleHitList().setDx(locFCALSingleHitMatchParamsVector[loc_k]->dx);
				fcalSingleHitList().setPathlength(locFCALSingleHitMatchParamsVector[loc_k]->dPathLength);
				fcalSingleHitList().setTflight(locFCALSingleHitMatchParamsVector[loc_k]->dFlightTime);
				fcalSingleHitList().setTflightvar(locFCALSingleHitMatchParamsVector[loc_k]->dFlightTimeVariance);
			}

			vector<shared_ptr<const DTOFHitMatchParams>> locTOFHitMatchParamsVector;
			locDetectorMatches[loc_i]->Get_TOFMatchParams(tracks[loc_j], locTOFHitMatchParamsVector);
			for(size_t loc_k = 0; loc_k < locTOFHitMatchParamsVector.size(); ++loc_k)
			{
				hddm_r::TofMatchParamsList tofList = matches().addTofMatchParamses(1);
				tofList().setTrack(loc_j);

				size_t locTOFindex = 0;
				for(; locTOFindex < tofpoints.size(); ++locTOFindex)
				{
					if(tofpoints[locTOFindex] == locTOFHitMatchParamsVector[loc_k]->dTOFPoint)
						break;
				}
				tofList().setHit(locTOFindex);

				tofList().setThit(locTOFHitMatchParamsVector[loc_k]->dHitTime);
				tofList().setThitvar(locTOFHitMatchParamsVector[loc_k]->dHitTimeVariance);
				tofList().setEhit(locTOFHitMatchParamsVector[loc_k]->dHitEnergy);

				tofList().setDEdx(locTOFHitMatchParamsVector[loc_k]->dEdx);
				tofList().setPathlength(locTOFHitMatchParamsVector[loc_k]->dPathLength);
				tofList().setTflight(locTOFHitMatchParamsVector[loc_k]->dFlightTime);
				tofList().setTflightvar(locTOFHitMatchParamsVector[loc_k]->dFlightTimeVariance);

				tofList().setDeltax(locTOFHitMatchParamsVector[loc_k]->dDeltaXToHit);
				tofList().setDeltay(locTOFHitMatchParamsVector[loc_k]->dDeltaYToHit);
				// dEdx for each plane
				hddm_r::TofDedxList tofDedx = tofList().addTofDedxs(1);
				tofDedx().setDEdx1(locTOFHitMatchParamsVector[loc_k]->dEdx1);
				tofDedx().setDEdx2(locTOFHitMatchParamsVector[loc_k]->dEdx2);

			}

			vector<shared_ptr<const DSCHitMatchParams>> locSCHitMatchParamsVector;
			locDetectorMatches[loc_i]->Get_SCMatchParams(tracks[loc_j], locSCHitMatchParamsVector);
			for(size_t loc_k = 0; loc_k < locSCHitMatchParamsVector.size(); ++loc_k)
			{
				hddm_r::ScMatchParamsList scList = matches().addScMatchParamses(1);
				scList().setTrack(loc_j);

				size_t locSCindex = 0;
				for(; locSCindex < starthits.size(); ++locSCindex)
				{
					if(starthits[locSCindex] == locSCHitMatchParamsVector[loc_k]->dSCHit)
						break;
				}
				scList().setHit(locSCindex);

				scList().setDEdx(locSCHitMatchParamsVector[loc_k]->dEdx);
				scList().setDeltaphi(locSCHitMatchParamsVector[loc_k]->dDeltaPhiToHit);
				scList().setEhit(locSCHitMatchParamsVector[loc_k]->dHitEnergy);
				scList().setPathlength(locSCHitMatchParamsVector[loc_k]->dPathLength);
				scList().setTflight(locSCHitMatchParamsVector[loc_k]->dFlightTime);
				scList().setTflightvar(locSCHitMatchParamsVector[loc_k]->dFlightTimeVariance);
				scList().setThit(locSCHitMatchParamsVector[loc_k]->dHitTime);
				scList().setThitvar(locSCHitMatchParamsVector[loc_k]->dHitTimeVariance);
			}

			
			shared_ptr<const DDIRCMatchParams> locDIRCMatchParams;
			map<shared_ptr<const DDIRCMatchParams>, vector<const DDIRCPmtHit*> > locDIRCTrackMatchParamsMap;
			DDetectorMatches *locDetectorMatch = (DDetectorMatches*)locDetectorMatches[loc_i];
                        locDetectorMatch->Get_DIRCMatchParams(tracks[loc_j], locDIRCMatchParams);
			locDetectorMatch->Get_DIRCTrackMatchParamsMap(locDIRCTrackMatchParamsMap);

			if(locDIRCMatchParams) {
				hddm_r::DircMatchParamsList dircList = matches().addDircMatchParamses(1);
        	                dircList().setTrack(loc_j);

				vector<const DDIRCPmtHit*> locDIRCHitTrackMatch = (vector<const DDIRCPmtHit*>)locDIRCTrackMatchParamsMap[locDIRCMatchParams];
				for(size_t loc_k = 0; loc_k < locDIRCPmtHits.size(); ++loc_k) {
					const DDIRCPmtHit* locDIRCPmtHit = (DDIRCPmtHit*)locDIRCPmtHits[loc_k];
					if(find(locDIRCHitTrackMatch.begin(), locDIRCHitTrackMatch.end(), locDIRCPmtHit) != locDIRCHitTrackMatch.end()) {
						hddm_r::DircMatchHitList dircHitList = matches().addDircMatchHits(1);
						dircHitList().setTrack(loc_j);
						dircHitList().setHit(loc_k);
					}
				}

				vector<DTrackFitter::Extrapolation_t> extrapolations=tracks[loc_j]->extrapolations.at(SYS_DIRC);
				DVector3 locProjPos = locDIRCMatchParams->dExtrapolatedPos;
				DVector3 locProjMom = locDIRCMatchParams->dExtrapolatedMom;
				double locFlightTime = locDIRCMatchParams->dExtrapolatedTime;
				dircList().setX(locProjPos.X());
				dircList().setY(locProjPos.Y());
				dircList().setZ(locProjPos.Z());
				dircList().setT(locFlightTime);
				dircList().setPx(locProjMom.X());
				dircList().setPy(locProjMom.Y());
				dircList().setPz(locProjMom.Z());
				dircList().setExpectthetac(locDIRCMatchParams->dExpectedThetaC);
				dircList().setThetac(locDIRCMatchParams->dThetaC);
				dircList().setDeltat(locDIRCMatchParams->dDeltaT);
				dircList().setLele(locDIRCMatchParams->dLikelihoodElectron);
				dircList().setLpi(locDIRCMatchParams->dLikelihoodPion);
				dircList().setLk(locDIRCMatchParams->dLikelihoodKaon);
				dircList().setLp(locDIRCMatchParams->dLikelihoodProton);
				dircList().setNphotons(locDIRCMatchParams->dNPhotons);
			}

			double locFlightTimePCorrelation = 0.0;
			if(locDetectorMatches[loc_i]->Get_FlightTimePCorrelation(tracks[loc_j], SYS_BCAL, locFlightTimePCorrelation))
			{
				hddm_r::TflightPCorrelationList correlationList = matches().addTflightPCorrelations(1);
				correlationList().setTrack(loc_j);
				correlationList().setSystem(SYS_BCAL);
				correlationList().setCorrelation(locFlightTimePCorrelation);
			}
			if(locDetectorMatches[loc_i]->Get_FlightTimePCorrelation(tracks[loc_j], SYS_FCAL, locFlightTimePCorrelation))
			{
				hddm_r::TflightPCorrelationList correlationList = matches().addTflightPCorrelations(1);
				correlationList().setTrack(loc_j);
				correlationList().setSystem(SYS_FCAL);
				correlationList().setCorrelation(locFlightTimePCorrelation);
			}
			if(locDetectorMatches[loc_i]->Get_FlightTimePCorrelation(tracks[loc_j], SYS_TOF, locFlightTimePCorrelation))
			{
				hddm_r::TflightPCorrelationList correlationList = matches().addTflightPCorrelations(1);
				correlationList().setTrack(loc_j);
				correlationList().setSystem(SYS_TOF);
				correlationList().setCorrelation(locFlightTimePCorrelation);
			}
			if(locDetectorMatches[loc_i]->Get_FlightTimePCorrelation(tracks[loc_j], SYS_START, locFlightTimePCorrelation))
			{
				hddm_r::TflightPCorrelationList correlationList = matches().addTflightPCorrelations(1);
				correlationList().setTrack(loc_j);
				correlationList().setSystem(SYS_START);
				correlationList().setCorrelation(locFlightTimePCorrelation);
			}
		}

		for(size_t loc_j = 0; loc_j < bcalshowers.size(); ++loc_j)
		{
			double locDeltaPhi = 0.0, locDeltaZ = 0.0;
			if(!locDetectorMatches[loc_i]->Get_DistanceToNearestTrack(bcalshowers[loc_j], locDeltaPhi, locDeltaZ))
				continue;

			hddm_r::BcalDOCAtoTrackList bcalDocaList = matches().addBcalDOCAtoTracks(1);
			bcalDocaList().setShower(loc_j);
			bcalDocaList().setDeltaphi(locDeltaPhi);
			bcalDocaList().setDeltaz(locDeltaZ);
		}

		for(size_t loc_j = 0; loc_j < fcalshowers.size(); ++loc_j)
		{
			double locDistance = 0.0;
			if(!locDetectorMatches[loc_i]->Get_DistanceToNearestTrack(fcalshowers[loc_j], locDistance))
				continue;

			hddm_r::FcalDOCAtoTrackList fcalDocaList = matches().addFcalDOCAtoTracks(1);
			fcalDocaList().setShower(loc_j);
			fcalDocaList().setDoca(locDistance);
		}
	}
	return locRecord;
}

int32_t JEventProcessor_BCAL_timing::Convert_UnsignedIntToSigned(uint32_t locUnsignedInt) const
{
	//Convert uint32_t to int32_t
	//Scheme:
		//If bit 32 is zero, then the int32_t is the same as the uint32_t: Positive or zero
		//If bit 32 is one, and at least one other bit is 1, then the int32_t is -1 * uint32_t (after stripping the top bit)
		//If bit 32 is one, and all other bits are zero, then the int32_t is the minimum int: -(2^31)
	if((locUnsignedInt & 0x80000000) == 0)
		return int32_t(locUnsignedInt); //bit 32 is zero: positive or zero

	//bit 32 is 1. see if there is another bit set
	int32_t locTopBitStripped = int32_t(locUnsignedInt & uint32_t(0x7FFFFFFF)); //strip the top bit
	if(locTopBitStripped == 0)
		return numeric_limits<int32_t>::min(); //no other bit is set: minimum int
	return -1*locTopBitStripped; //return the negative
}
