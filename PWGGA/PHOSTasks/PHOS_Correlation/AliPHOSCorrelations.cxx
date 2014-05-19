/**************************************************************************
 * Copyright(c) 1998-2014, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
 
// Analysis task for identified PHOS cluster from pi0 and take korrelation betwen hadron-pi0 angel's.
// Authors: 	Daniil Ponomarenko (Daniil.Ponomarenko@cern.ch)
// 		Dmitry Blau
// 07-Feb-2014

#include <Riostream.h>
#include "THashList.h"
#include "TObjArray.h"
#include "TClonesArray.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH2I.h"
#include "TH3F.h"
#include "TH3D.h"
#include "TMath.h"
#include "TVector3.h"
#include "TProfile.h"

#include "AliAnalysisManager.h"
#include "AliAnalysisTaskSE.h"
#include "AliPHOSCorrelations.h"
#include "AliPHOSGeometry.h"
#include "AliESDEvent.h"
#include "AliESDCaloCluster.h"
#include "AliESDVertex.h"
#include "AliESDtrackCuts.h"
#include "AliESDtrack.h"
#include "AliAODTrack.h"
#include "AliVTrack.h"
#include "AliPID.h"
#include "AliTriggerAnalysis.h"
#include "AliPIDResponse.h"
#include "AliPHOSEsdCluster.h"
#include "AliCDBManager.h"
#include "AliPHOSCalibData.h"
#include "AliCentrality.h"
#include "AliAnalysisTaskSE.h"
#include "AliEventplane.h"
#include "AliOADBContainer.h"
#include "AliAODEvent.h"
#include "AliAODCaloCells.h"
#include "AliAODCaloCluster.h"
#include "AliCaloPhoton.h"
#include "AliAODVertex.h"

using std::cout;
using std::endl;

ClassImp(AliPHOSCorrelations)

//_______________________________________________________________________________
AliPHOSCorrelations::AliPHOSCorrelations()
:AliAnalysisTaskSE(),
        fPHOSGeo(0x0),
	fOutputContainer(0x0),
	fMinClusterEnergy(0.3),
	fMinBCDistance(0),
	fMinNCells(3),
	fMinM02(0.2),
	fTOFCutEnabled(1),
	fTOFCut(100.e-9),
	fNVtxZBins(1),
	fCentEdges(10),
	fCentNMixed(),
	fNEMRPBins(9),
	fAssocBins(),	
	fCheckHibridGlobal(kOnlyHibridTracks),
	fPeriod(kUndefinedPeriod),
	fInternalTriggerSelection(kNoSelection),
	fMaxAbsVertexZ(10.),
	fManualV0EPCalc(false),
	fCentCutoffDown(0.),
	fCentCutoffUp(90),
	fMassInvMean(0.135),
	fMassInvSigma(0.01),
	fSigmaWidth(0.),
	fEvent(0x0),
	fEventESD(0x0),
	fEventAOD(0x0),
	fESDtrackCuts(0x0),
	fRunNumber(-999),
	fInternalRunNumber(0),
	fMultV0(0x0),
	fV0Cpol(0.),fV0Apol(0.),
	fEPcalibFileName("$ALICE_ROOT/OADB/PHOS/PHOSflat.root"),
	fVertexVector(),
        	fVtxBin(0),
	fCentralityEstimator("V0M"),
	fCentrality(0.),
	fCentBin(0),
	fHaveTPCRP(0),
	fRP(0.),
	fEMRPBin(0),
	fCaloPhotonsPHOS(0x0),
	fTracksTPC(0x0),
	fCaloPhotonsPHOSLists(0x0),
  	fTracksTPCLists(0x0)
{
  //Deafult constructor, no memory allocations here
}

//_______________________________________________________________________________
AliPHOSCorrelations::AliPHOSCorrelations(const char *name, Period period)
:AliAnalysisTaskSE(name),
        fPHOSGeo(0x0),
	fOutputContainer(0x0),
	fMinClusterEnergy(0.3),
	fMinBCDistance(0),
	fMinNCells(3),
	fMinM02(0.2),
	fTOFCutEnabled(1),
	fTOFCut(100.e-9),
	fNVtxZBins(1),
	fCentEdges(10),
	fCentNMixed(),
	fNEMRPBins(9),
	fAssocBins(),	
	fCheckHibridGlobal(kOnlyHibridTracks),
	fPeriod(period),
	fInternalTriggerSelection(kNoSelection),
	fMaxAbsVertexZ(10.),
	fManualV0EPCalc(false),
	fCentCutoffDown(0.),
	fCentCutoffUp(90),
	fMassInvMean(0.135),
	fMassInvSigma(0.01),
	fSigmaWidth(0.),
	fEvent(0x0),
	fEventESD(0x0),
	fEventAOD(0x0),
	fESDtrackCuts(0x0),
	fRunNumber(-999),
	fInternalRunNumber(0),
	fMultV0(0x0),
	fV0Cpol(0.),fV0Apol(0.),
	fEPcalibFileName("$ALICE_ROOT/OADB/PHOS/PHOSflat.root"),
	fVertexVector(),
        	fVtxBin(0),
	fCentralityEstimator("V0M"),
	fCentrality(0.),
	fCentBin(0),
	fHaveTPCRP(0),
	fRP(0.),
	fEMRPBin(0),
	fCaloPhotonsPHOS(0x0),
	fTracksTPC(0x0),
	fCaloPhotonsPHOSLists(0x0),
  	fTracksTPCLists(0x0)
{
	// Constructor
	// Output slots #0 write into a TH1 container
	DefineOutput(1,THashList::Class());

 	const Int_t nPtAssoc=10 ;
 	Double_t ptAssocBins[nPtAssoc]={0.,0.5,1.0,1.5,2.0,3.,5.,7.,10.,16} ;
 	fAssocBins.Set(nPtAssoc,ptAssocBins) ;
		
	const int nbins = 9;
	Double_t edges[nbins+1] = {0., 5., 10., 20., 30., 40., 50., 60., 70., 80.};
	TArrayD centEdges(nbins+1, edges);
	Int_t nMixed[nbins] = {4,4,6,10,20,30,50,100,100};
	//Int_t nMixed[nbins] = {100,100,100,100,100,100,100,100,100};
	TArrayI centNMixed(nbins, nMixed);
	SetCentralityBinning(centEdges, centNMixed);

	fVertex[0]=0; fVertex[1]=0; fVertex[2]=0; 

	fPHOSGeo = AliPHOSGeometry::GetInstance("IHEP");
}
//_______________________________________________________________________________
AliPHOSCorrelations::~AliPHOSCorrelations()
{
	if(fCaloPhotonsPHOS){ 
	  delete fCaloPhotonsPHOS;
	  fCaloPhotonsPHOS=0x0;
	}
	
	if(fTracksTPC){
	  delete fTracksTPC;
	  fTracksTPC=0x0;
	}

	if(fCaloPhotonsPHOSLists){
	  fCaloPhotonsPHOSLists->SetOwner() ;
	  delete fCaloPhotonsPHOSLists;
	  fCaloPhotonsPHOSLists=0x0;
	}
	
	if(fTracksTPCLists){
	  fTracksTPCLists->SetOwner() ;
	  delete fTracksTPCLists;
	  fTracksTPCLists=0x0 ;
	}
	 
	if( fESDtrackCuts){	  
	  delete fESDtrackCuts;
	  fESDtrackCuts=0x0 ;
	}
		  
	if(fOutputContainer){
	  delete fOutputContainer;
	  fOutputContainer=0x0;
	}	  
}
//_______________________________________________________________________________
void AliPHOSCorrelations::UserCreateOutputObjects()
{
	// Create histograms
  	// Called once
	const Int_t nRuns=200 ;

	// Create histograms
	if(fOutputContainer != NULL) { delete fOutputContainer; }
	fOutputContainer = new THashList();
	fOutputContainer->SetOwner(kTRUE);
	
	//Event selection
  	fOutputContainer->Add(new TH1F("hTriggerPassedEvents","Event selection passed Cuts", 20, 0., 20.) );

  	fOutputContainer->Add(new TH1F("hTotSelEvents","Event selection", kTotalSelected+3, 0., double(kTotalSelected+3))) ;
	
	fOutputContainer->Add(new TH2F("hSelEvents","Event selection", kTotalSelected+1, 0., double(kTotalSelected+1), nRuns,0.,float(nRuns))) ;
	fOutputContainer->Add(new TH2F("hCentrality","Event centrality", 100,0.,100.,nRuns,0.,float(nRuns))) ;
 	fOutputContainer->Add(new TH2F("phiRPflat","RP distribution with TPC flat", 100, 0., 2.*TMath::Pi(),20,0.,100.)) ;
 	fOutputContainer->Add(new TH2F("massWindow","mean & sigma", 100,0.1,0.18,100,0.,0.5));
  	

  	// Set hists, with track's and cluster's angle distributions.
	SetHistEtaPhi();
	SetHistPHOSClusterMap();
	SetHistCutDistribution();
	SetHistPtAssoc();

	// Setup photon lists
	Int_t kapacity = fNVtxZBins * GetNumberOfCentralityBins() * fNEMRPBins;
	fCaloPhotonsPHOSLists = new TObjArray(kapacity);
	fCaloPhotonsPHOSLists->SetOwner();

	fTracksTPCLists = new TObjArray(kapacity);
	fTracksTPCLists->SetOwner();

	PostData(1, fOutputContainer);
}
//_______________________________________________________________________________
void AliPHOSCorrelations::SetHistEtaPhi() 
{
	// Set hists, with track's and cluster's angle distributions.

	Float_t pi = TMath::Pi();

	//===
	fOutputContainer->Add(new TH2F("clu_phieta","Cluster's #phi & #eta distribution", 300, double(-1.8), double(-0.6), 300, double(-0.2), double(0.2) ) );
	TH2F * h = static_cast<TH2F*>(fOutputContainer->Last()) ;
    	h->GetXaxis()->SetTitle("#phi [rad]");
	h->GetYaxis()->SetTitle("#eta");

 	//===
       	fOutputContainer->Add(new TH2F("clusingle_phieta","Cluster's  #phi & #eta distribution", 300, double(-1.8), double(-0.6), 300, double(-0.2), double(0.2) ) );
	h = static_cast<TH2F*>(fOutputContainer->Last()) ;
    	h->GetXaxis()->SetTitle("#phi [rad]");
	h->GetYaxis()->SetTitle("#eta");
 	
 	//===
 	fOutputContainer->Add(new TH2F("track_phieta","TPC track's  #phi & #eta distribution", 200, double(-pi-0.3), double(pi+0.3), 200, double(-0.9), double(0.9) ) );
	h = static_cast<TH2F*>(fOutputContainer->FindObject("track_phieta")) ;
    	h->GetXaxis()->SetTitle("#phi [rad]");
	h->GetYaxis()->SetTitle("#eta");
} 
//_______________________________________________________________________________
void AliPHOSCorrelations::SetHistCutDistribution() 
{
	// Set other histograms.
	// cout<<"\nSetting output SetHist_CutDistribution...";

	Int_t  PtMult = 100;
	Double_t PtMin = 0.;
	Double_t PtMax = 20.;
	Double_t massMin = fMassInvMean-fMassInvSigma;
	Double_t massMax = fMassInvMean+fMassInvSigma;


	// Real ++++++++++++++++++++++++++++++

	fOutputContainer->Add(new TH2F("all_mpt"," Only standard cut's ", 100, massMin, massMax, PtMult, PtMin, PtMax ) );
	TH2F * h = static_cast<TH2F*>(fOutputContainer->Last()) ;
    	h->GetXaxis()->SetTitle("Mass [GeV]");
	h->GetYaxis()->SetTitle("Pt [GEV]");

	fOutputContainer->Add(new TH2F("cpv_mpt"," CPV cut ", 100, massMin, massMax, PtMult, PtMin, PtMax ) );
	h = static_cast<TH2F*>(fOutputContainer->Last()) ;
    	h->GetXaxis()->SetTitle("Mass [GeV]");
	h->GetYaxis()->SetTitle("Pt [GEV]");

	fOutputContainer->Add(new TH2F("disp_mpt"," Disp cut ", 100, massMin, massMax, PtMult, PtMin, PtMax ) );
	h = static_cast<TH2F*>(fOutputContainer->Last()) ;
    	h->GetXaxis()->SetTitle("Mass [GeV]");
	h->GetYaxis()->SetTitle("Pt [GEV]");

	fOutputContainer->Add(new TH2F("both_mpt"," Both cuts (CPV + Disp) ", 100, massMin, massMax, PtMult, PtMin, PtMax ) );
	h = static_cast<TH2F*>(fOutputContainer->Last()) ;
    	h->GetXaxis()->SetTitle("Mass [GeV]");
	h->GetYaxis()->SetTitle("Pt [GEV]");


	// MIX +++++++++++++++++++++++++

	fOutputContainer->Add(new TH2F("mix_all_mpt"," Only standard cut's (mix)", 100, massMin, massMax, PtMult, PtMin, PtMax ) );
	h = static_cast<TH2F*>(fOutputContainer->Last()) ;
    	h->GetXaxis()->SetTitle("Mass [GeV]");
	h->GetYaxis()->SetTitle("Pt [GEV]");

	fOutputContainer->Add(new TH2F("mix_cpv_mpt"," CPV cut (mix)", 100, massMin, massMax, PtMult, PtMin, PtMax ) );
	h = static_cast<TH2F*>(fOutputContainer->Last()) ;
    	h->GetXaxis()->SetTitle("Mass [GeV]");
	h->GetYaxis()->SetTitle("Pt [GEV]");

	fOutputContainer->Add(new TH2F("mix_disp_mpt"," Disp cut (mix)", 100, massMin, massMax, PtMult, PtMin, PtMax ) );
	h = static_cast<TH2F*>(fOutputContainer->Last()) ;
    	h->GetXaxis()->SetTitle("Mass [GeV]");
	h->GetYaxis()->SetTitle("Pt [GEV]");

	fOutputContainer->Add(new TH2F("mix_both_mpt"," Both cuts (CPV + Disp) (mix)", 100, massMin, massMax, PtMult, PtMin, PtMax ) );
	h = static_cast<TH2F*>(fOutputContainer->Last()) ;
    	h->GetXaxis()->SetTitle("Mass [GeV]");
	h->GetYaxis()->SetTitle("Pt [GEV]");


	// Calibration Pi0peak {REAL}
	for(Int_t mod=1; mod<4; mod++){
	  fOutputContainer->Add(new TH2F(Form("both%d_mpt",mod),Form("Both cuts (CPV + Disp) mod[%d]",mod), 100, massMin, massMax, PtMult, PtMin, PtMax ) );
	  h = static_cast<TH2F*>(fOutputContainer->Last()) ;
    	  h->GetXaxis()->SetTitle("Mass [GeV]");
	  h->GetYaxis()->SetTitle("Pt [GEV]");

 	  // Calibration Pi0peak {MIX}
	  fOutputContainer->Add(new TH2F(Form("mix_both%d_mpt",mod),Form(" Both cuts (CPV + Disp) mod[%d]",mod), 100, massMin, massMax, PtMult, PtMin, PtMax ) );
	  h = static_cast<TH2F*>(fOutputContainer->FindObject("mix_both1_mpt")) ;
    	  h->GetXaxis()->SetTitle("Mass [GeV]");
	  h->GetYaxis()->SetTitle("Pt [GEV]");
	  
	}

	// cout<<"  OK!"<<endl;
}
//_______________________________________________________________________________
void AliPHOSCorrelations::SetHistPtAssoc()
{
	Double_t pi = TMath::Pi();
	
	Int_t PhiMult  =  100;
	Float_t PhiMin =  -0.5*pi;
	Float_t PhiMax =  1.5*pi;
	Int_t EtaMult  =  20; 
	Float_t EtaMin = -1.;
	Float_t EtaMax =  1.;
	Int_t PtTrigMult = 100;
	Float_t PtTrigMin = 0.;
	Float_t PtTrigMax = 20.;

	TString spid[4]={"all","cpv","disp","both"} ;
	
	for (int i = 0; i<fAssocBins.GetSize()-1; i++){
	  for(Int_t ipid=0; ipid<4; ipid++){
		fOutputContainer->Add(new TH3F(Form("%s_ptphieta_ptAssoc_%3.1f",spid[ipid].Data(),fAssocBins.At(i+1)),
					       Form("%s_ptphieta_ptAssoc_%3.1f",spid[ipid].Data(),fAssocBins.At(i+1)), 
					       PtTrigMult, PtTrigMin, PtTrigMax,  PhiMult, PhiMin, PhiMax, EtaMult, EtaMin, EtaMax ) );
		TH3F * h = static_cast<TH3F*>(fOutputContainer->Last()) ;
    		h->GetXaxis()->SetTitle("Pt_{triger} [GEV]");
		h->GetYaxis()->SetTitle("#phi [rad]");
		h->GetZaxis()->SetTitle("#eta");

		fOutputContainer->Add(new TH3F(Form("mix_%s_ptphieta_ptAssoc_%3.1f",spid[ipid].Data(),fAssocBins.At(i+1)),
					       Form("Mixed %s_ptphieta_ptAssoc_%3.1f",spid[ipid].Data(),fAssocBins.At(i+1)),
					       PtTrigMult, PtTrigMin, PtTrigMax,  PhiMult, PhiMin, PhiMax, EtaMult, EtaMin, EtaMax ) );
		h = static_cast<TH3F*>(fOutputContainer->Last()) ;
    		h->GetXaxis()->SetTitle("Pt_{triger} [GEV]");
		h->GetYaxis()->SetTitle("#phi [rad]");
		h->GetZaxis()->SetTitle("#eta");

	  }
	}
}

void AliPHOSCorrelations::SetHistPHOSClusterMap()
{
	for(int i =  0; i<3; i++)
	{
		//  Cluster X/Z/E distribution.
		fOutputContainer->Add(new TH3F(Form("QA_cluXZE_mod%i", i+1),Form("PHOS Clusters XZE distribution of module %i", i+1), 100, 0, 100, 100, 0, 100, 100, 0, 10 ) );
		TH3F *h = static_cast<TH3F*>(fOutputContainer->Last()) ;
	    	h->GetXaxis()->SetTitle("X");
		h->GetYaxis()->SetTitle("Z");
		h->GetZaxis()->SetTitle("E");
	}	
}
//_______________________________________________________________________________
void AliPHOSCorrelations::UserExec(Option_t *) 
{
	// Main loop, called for each event analyze ESD/AOD 
	// Step 0: Event Objects
	LogProgress(0);
	fEvent = InputEvent();
	if( ! fEvent ) 
	{
		AliError("Event could not be retrieved");
		PostData(1, fOutputContainer);
		return ;
	}
        
	fEventESD = dynamic_cast<AliESDEvent*>(fEvent);
	fEventAOD = dynamic_cast<AliAODEvent*>(fEvent);

	{
		FillHistogram("hTriggerPassedEvents",  0);

		Bool_t isMB = (fEvent->GetTriggerMask() & (ULong64_t(1)<<1));
		Bool_t isCentral = (fEvent->GetTriggerMask() & (ULong64_t(1)<<4));
		Bool_t isSemiCentral = (fEvent->GetTriggerMask() & (ULong64_t(1)<<7));

		if (isMB) FillHistogram("hTriggerPassedEvents",  2.);
		if (isCentral) FillHistogram("hTriggerPassedEvents",  3.);
		if (isSemiCentral) FillHistogram("hTriggerPassedEvents",  4.);
	}

	// For first event from data only:
	if( fRunNumber<0)
	{
		if (fDebug >= 1) cout<<"Mean: "<< fMassInvMean << " Sigma: "<< fMassInvSigma
					<<" Sigma Width: " <<fSigmaWidth <<endl;
		if (!fSigmaWidth) FillHistogram("massWindow",  fMassInvMean, fMassInvSigma);
		else FillHistogram("massWindow",  fMassInvMean, fMassInvSigma*fSigmaWidth);
	}

  	// Step 1(done once):  
	if( fRunNumber != fEvent->GetRunNumber() )
	{
		fRunNumber = fEvent->GetRunNumber();
		fInternalRunNumber = ConvertToInternalRunNumber(fRunNumber);
		SetESDTrackCuts();
	}
	LogProgress(1);

	if( RejectTriggerMaskSelection() ) 
	{
		PostData(1, fOutputContainer);
		return; // Reject!
	}
  	LogProgress(2);

	// Step 2: Vertex
  	// fVertex, fVertexVector, fVtxBin
	SetVertex();
	if( RejectEventVertex() ) 
	{
    		PostData(1, fOutputContainer);
    		return; // Reject!
  	}
  	LogProgress(3);

  	// Step 3: Centrality
  	// fCentrality, fCentBin
	SetCentrality(); 
	if( RejectEventCentrality() ) 
	{
    		PostData(1, fOutputContainer);
    		return; // Reject!
  	}
  	FillHistogram("hCentrality",fCentrality,fInternalRunNumber-0.5) ;
	LogProgress(4);

	// Step 4: Reaction Plane
  	// fHaveTPCRP, fRP, fRPV0A, fRPV0C, fRPBin
	EvalReactionPlane();  
  	fEMRPBin = GetRPBin(); 
  	LogProgress(5);
  	
	// Step 5: Event Photons (PHOS Clusters) selectionMakeFlat
	SelectPhotonClusters();
	if( ! fCaloPhotonsPHOS->GetEntriesFast() )	LogSelection(kHasPHOSClusters, fInternalRunNumber);
	LogProgress(6);

	// Step 6: Event Associated particles (TPC Tracks) selection
	SelectAccosiatedTracks();
	
	if( ! fTracksTPC->GetEntriesFast() )	
	  LogSelection(kHasTPCTracks, fInternalRunNumber);
	LogSelection(kTotalSelected, fInternalRunNumber);
	LogProgress(7);

	// Step 7: Consider pi0 (photon/cluster) pairs.
	ConsiderPi0s();
	
	// Step 8; Mixing
	ConsiderPi0sMix();

	ConsiderTracksMix();
	//this->ConsiderPi0sTracksMix(); // Read how make mix events!
	LogProgress(8);

	// Step 9: Make TPC's mask
	FillTrackEtaPhi();
	LogProgress(9);

	// Step 10: Update lists
	UpdatePhotonLists();
    	UpdateTrackLists();
  
	LogProgress(10);

	// Post output data.
	PostData(1, fOutputContainer);
}
//_______________________________________________________________________________
void AliPHOSCorrelations::SetESDTrackCuts()
{
  if( fEventESD ) {
    // Create ESD track cut
    fESDtrackCuts = AliESDtrackCuts::GetStandardTPCOnlyTrackCuts() ;
    //fESDtrackCuts = AliESDtrackCuts::GetStandardITSTPCTrackCuts2010();
    fESDtrackCuts->SetRequireTPCRefit(kTRUE);
  }
}
//_______________________________________________________________________________
Int_t AliPHOSCorrelations::ConvertToInternalRunNumber(Int_t run){
  if(fPeriod== kLHC11h){
				switch(run)
				{
					case  170593 : return 179 ;
					case  170572 : return 178 ;
					case  170556 : return 177 ;
					case  170552 : return 176 ;
					case  170546 : return 175 ;
					case  170390 : return 174 ;
					case  170389 : return 173 ;
					case  170388 : return 172 ;
					case  170387 : return 171 ;
					case  170315 : return 170 ;
					case  170313 : return 169 ;
					case  170312 : return 168 ;
					case  170311 : return 167 ;
					case  170309 : return 166 ;
					case  170308 : return 165 ;
					case  170306 : return 164 ;
					case  170270 : return 163 ;
					case  170269 : return 162 ;
					case  170268 : return 161 ;
					case  170267 : return 160 ;
					case  170264 : return 159 ;
					case  170230 : return 158 ;
					case  170228 : return 157 ;
					case  170208 : return 156 ;
					case  170207 : return 155 ;
					case  170205 : return 154 ;
					case  170204 : return 153 ;
					case  170203 : return 152 ;
					case  170195 : return 151 ;
					case  170193 : return 150 ;
					case  170163 : return 149 ;
					case  170162 : return 148 ;
					case  170159 : return 147 ;
					case  170155 : return 146 ;
					case  170152 : return 145 ;
					case  170091 : return 144 ;
					case  170089 : return 143 ;
					case  170088 : return 142 ;
					case  170085 : return 141 ;
					case  170084 : return 140 ;
					case  170083 : return 139 ;
					case  170081 : return 138 ;
					case  170040 : return 137 ;
					case  170038 : return 136 ;
					case  170036 : return 135 ;
					case  170027 : return 134 ;
					case  169981 : return 133 ;
					case  169975 : return 132 ;
					case  169969 : return 131 ;
					case  169965 : return 130 ;
					case  169961 : return 129 ;
					case  169956 : return 128 ;
					case  169926 : return 127 ;
					case  169924 : return 126 ;
					case  169923 : return 125 ;
					case  169922 : return 124 ;
					case  169919 : return 123 ;
					case  169918 : return 122 ;
					case  169914 : return 121 ;
					case  169859 : return 120 ;
					case  169858 : return 119 ;
					case  169855 : return 118 ;
					case  169846 : return 117 ;
					case  169838 : return 116 ;
					case  169837 : return 115 ;
					case  169835 : return 114 ;
					case  169683 : return 113 ;
					case  169628 : return 112 ;
					case  169591 : return 111 ;
					case  169590 : return 110 ;
					case  169588 : return 109 ;
					case  169587 : return 108 ;
					case  169586 : return 107 ;
					case  169584 : return 106 ;
					case  169557 : return 105 ;
					case  169555 : return 104 ;
					case  169554 : return 103 ;
					case  169553 : return 102 ;
					case  169550 : return 101 ;
					case  169515 : return 100 ;
					case  169512 : return 99 ;
					case  169506 : return 98 ;
					case  169504 : return 97 ;
					case  169498 : return 96 ;
					case  169475 : return 95 ;
					case  169420 : return 94 ;
					case  169419 : return 93 ;
					case  169418 : return 92 ;
					case  169417 : return 91 ;
					case  169415 : return 90 ;
					case  169411 : return 89 ;
					case  169238 : return 88 ;
					case  169236 : return 87 ;
					case  169167 : return 86 ;
					case  169160 : return 85 ;
					case  169156 : return 84 ;
					case  169148 : return 83 ;
					case  169145 : return 82 ;
					case  169144 : return 81 ;
					case  169143 : return 80 ;
					case  169138 : return 79 ;
					case  169099 : return 78 ;
					case  169094 : return 77 ;
					case  169091 : return 76 ;
					case  169045 : return 75 ;
					case  169044 : return 74 ;
					case  169040 : return 73 ;
					case  169035 : return 72 ;
					case  168992 : return 71 ;
					case  168988 : return 70 ;
					case  168984 : return 69 ;
					case  168826 : return 68 ;
					case  168777 : return 67 ;
					case  168514 : return 66 ;
					case  168512 : return 65 ;
					case  168511 : return 64 ;
					case  168467 : return 63 ;
					case  168464 : return 62 ;
					case  168461 : return 61 ;
					case  168460 : return 60 ;
					case  168458 : return 59 ;
					case  168362 : return 58 ;
					case  168361 : return 57 ;
					case  168356 : return 56 ;
					case  168342 : return 55 ;
					case  168341 : return 54 ;
					case  168325 : return 53 ;
					case  168322 : return 52 ;
					case  168318 : return 51 ;
					case  168311 : return 50 ;
					case  168310 : return 49 ;
					case  168213 : return 48 ;
					case  168212 : return 47 ;
					case  168208 : return 46 ;
					case  168207 : return 45 ;
					case  168206 : return 44 ;
					case  168205 : return 43 ;
					case  168204 : return 42 ;
					case  168203 : return 41 ;
					case  168181 : return 40 ;
					case  168177 : return 39 ;
					case  168175 : return 38 ;
					case  168173 : return 37 ;
					case  168172 : return 36 ;
					case  168171 : return 35 ;
					case  168115 : return 34 ;
					case  168108 : return 33 ;
					case  168107 : return 32 ;
					case  168105 : return 31 ;
					case  168104 : return 30 ;
					case  168103 : return 29 ;
					case  168076 : return 28 ;
					case  168069 : return 27 ;
					case  168068 : return 26 ;
					case  168066 : return 25 ;
					case  167988 : return 24 ;
					case  167987 : return 23 ;
					case  167986 : return 22 ;
					case  167985 : return 21 ;
					case  167921 : return 20 ;
					case  167920 : return 19 ;
					case  167915 : return 18 ;
					case  167909 : return 17 ;
					case  167903 : return 16 ;
					case  167902 : return 15 ;
					case  167818 : return 14 ;
					case  167814 : return 13 ;
					case  167813 : return 12 ;
					case  167808 : return 11 ;
					case  167807 : return 10 ;
					case  167806 : return 9 ;
					case  167713 : return 8 ;
					case  167712 : return 7 ;
					case  167711 : return 6 ;
					case  167706 : return 5 ;
					case  167693 : return 4 ;
					case  166532 : return 3 ;
					case  166530 : return 2 ;
					case  166529 : return 1 ;

					default : return 199;
				}
			}
	if(fPeriod== kLHC10h){
		switch(run){
						case  139517 : return 137;
						case  139514 : return 136;
						case  139513 : return 135;
						case  139511 : return 134;
						case  139510 : return 133;
						case  139507 : return 132;
						case  139505 : return 131;
						case  139504 : return 130;
						case  139503 : return 129;
						case  139470 : return 128;
						case  139467 : return 127;
						case  139466 : return 126;
						case  139465 : return 125;
						case  139440 : return 124;
						case  139439 : return 123;
						case  139438 : return 122;
						case  139437 : return 121;
						case  139360 : return 120;
						case  139329 : return 119;
						case  139328 : return 118;
						case  139314 : return 117;
						case  139311 : return 116;
						case  139310 : return 115;
						case  139309 : return 114;
						case  139308 : return 113;
						case  139173 : return 112;
						case  139172 : return 111;
						case  139110 : return 110;
						case  139107 : return 109;
						case  139105 : return 108;
						case  139104 : return 107;
						case  139042 : return 106;
						case  139038 : return 105;
						case  139037 : return 104;
						case  139036 : return 103;
						case  139029 : return 102;
						case  139028 : return 101;
						case  138983 : return 100;
						case  138982 : return 99;
						case  138980 : return 98;
						case  138979 : return 97;
						case  138978 : return 96;
						case  138977 : return 95;
						case  138976 : return 94;
						case  138973 : return 93;
						case  138972 : return 92;
						case  138965 : return 91;
						case  138924 : return 90;
						case  138872 : return 89;
						case  138871 : return 88;
						case  138870 : return 87;
						case  138837 : return 86;
						case  138830 : return 85;
						case  138828 : return 84;
						case  138826 : return 83;
						case  138796 : return 82;
						case  138795 : return 81;
						case  138742 : return 80;
						case  138732 : return 79;
						case  138730 : return 78;
						case  138666 : return 77;
						case  138662 : return 76;
						case  138653 : return 75;
						case  138652 : return 74;
						case  138638 : return 73;
						case  138624 : return 72;
						case  138621 : return 71;
						case  138583 : return 70;
						case  138582 : return 69;
						case  138579 : return 68;
						case  138578 : return 67;
						case  138534 : return 66;
						case  138469 : return 65;
						case  138442 : return 64;
						case  138439 : return 63;
						case  138438 : return 62;
						case  138396 : return 61;
						case  138364 : return 60;
						case  138359 : return 59;
						case  138275 : return 58;
						case  138225 : return 57;
						case  138201 : return 56;
						case  138200 : return 55;
						case  138197 : return 54;
						case  138192 : return 53;
						case  138190 : return 52;
						case  138154 : return 51;
						case  138153 : return 50;
						case  138151 : return 49;
						case  138150 : return 48;
						case  138126 : return 47;
						case  138125 : return 46;
						case  137848 : return 45;
						case  137847 : return 44;
						case  137844 : return 43;
						case  137843 : return 42;
						case  137752 : return 41;
						case  137751 : return 40;
						case  137748 : return 39;
						case  137724 : return 38;
						case  137722 : return 37;
						case  137718 : return 36;
						case  137704 : return 35;
						case  137693 : return 34;
						case  137692 : return 33;
						case  137691 : return 32;
						case  137689 : return 31;
						case  137686 : return 30;
						case  137685 : return 29;
						case  137639 : return 28;
						case  137638 : return 27;
						case  137608 : return 26;
						case  137595 : return 25;
						case  137549 : return 24;
						case  137546 : return 23;
						case  137544 : return 22;
						case  137541 : return 21;
						case  137539 : return 20;
						case  137531 : return 19;
						case  137530 : return 18;
						case  137443 : return 17;
						case  137441 : return 16;
						case  137440 : return 15;
						case  137439 : return 14;
						case  137434 : return 13;
						case  137432 : return 12;
						case  137431 : return 11;
						case  137430 : return 10;
						case  137366 : return 9;
						case  137243 : return 8;
						case  137236 : return 7;
						case  137235 : return 6;
						case  137232 : return 5;
						case  137231 : return 4;
						case  137165 : return 3;
						case  137162 : return 2;
						case  137161 : return 1;
						default : return 199;
					}
				}
				if( kLHC13 == fPeriod ) 
				{
					switch(run)
					{
						case  195344 : return 1;
						case  195346 : return 2;
						case  195351 : return 3;
						case  195389 : return 4;
						case  195390 : return 5;
						case  195391 : return 6;
						case  195478 : return 7;
						case  195479 : return 8;
						case  195480 : return 9;
						case  195481 : return 10;
						case  195482 : return 11;
						case  195483 : return 12;
						case  195529 : return 13;
						case  195531 : return 14;
						case  195532 : return 15;
						case  195566 : return 16;
						case  195567 : return 17;
						case  195568 : return 18;
						case  195592 : return 19;
						case  195593 : return 20;
						case  195596 : return 21;
						case  195633 : return 22;
						case  195635 : return 23;
						case  195644 : return 24;
						case  195673 : return 25;
						case  195675 : return 26;
						case  195676 : return 27;
						case  195677 : return 28;
						case  195681 : return 29;
						case  195682 : return 30;
						case  195720 : return 31;
						case  195721 : return 32;
						case  195722 : return 33;
						case  195724 : return 34;
						case  195725 : return 34;
						case  195726 : return 35;
						case  195727 : return 36;
						case  195760 : return 37;
						case  195761 : return 38;
						case  195765 : return 39;
						case  195767 : return 40;
						case  195783 : return 41;
						case  195787 : return 42;
						case  195826 : return 43;
						case  195827 : return 44;
						case  195829 : return 45;
						case  195830 : return 46;
						case  195831 : return 47;
						case  195867 : return 48;
						case  195869 : return 49;
						case  195871 : return 50;
						case  195872 : return 51;
						case  195873 : return 52;
						case  195935 : return 53;
						case  195949 : return 54;
						case  195950 : return 55;
						case  195954 : return 56;
						case  195955 : return 57;
						case  195958 : return 58;
						case  195989 : return 59;
						case  195994 : return 60;
						case  195998 : return 61;
						case  196000 : return 62;
						case  196006 : return 63;
						case  196085 : return 64;
						case  196089 : return 65;
						case  196090 : return 66;
						case  196091 : return 67;
						case  196099 : return 68;
						case  196105 : return 69;
						case  196107 : return 70;
						case  196185 : return 71;
						case  196187 : return 72;
						case  196194 : return 73;
						case  196197 : return 74;
						case  196199 : return 75;
						case  196200 : return 76;
						case  196201 : return 77;
						case  196203 : return 78;
						case  196208 : return 79;
						case  196214 : return 80;
						case  196308 : return 81;
						case  196309 : return 82;
						case  196310 : return 83;
						case  196311 : return 84;
						case  196433 : return 85;
						case  196474 : return 86;
						case  196475 : return 87;
						case  196477 : return 88;
						case  196528 : return 89;
						case  196533 : return 90;
						case  196535 : return 91;
						case  196563 : return 92;
						case  196564 : return 93;
						case  196566 : return 94;
						case  196568 : return 95;
						case  196601 : return 96;
						case  196605 : return 97;
						case  196608 : return 98;
						case  196646 : return 99;
						case  196648 : return 100;
						case  196701 : return 101;
						case  196702 : return 102;
						case  196703 : return 103;
						case  196706 : return 104;
						case  196714 : return 105;
						case  196720 : return 106;
						case  196721 : return 107;
						case  196722 : return 108;
						case  196772 : return 109;
						case  196773 : return 110;
						case  196774 : return 111;
						case  196869 : return 112;
						case  196870 : return 113;
						case  196874 : return 114;
						case  196876 : return 115;
						case  196965 : return 116;
						case  196967 : return 117;
						case  196972 : return 118;
						case  196973 : return 119;
						case  196974 : return 120;
						case  197003 : return 121;
						case  197011 : return 122;
						case  197012 : return 123;
						case  197015 : return 124;
						case  197027 : return 125;
						case  197031 : return 126;
						case  197089 : return 127;
						case  197090 : return 128;
						case  197091 : return 129;
						case  197092 : return 130;
						case  197094 : return 131;
						case  197098 : return 132;
						case  197099 : return 133;
						case  197138 : return 134;
						case  197139 : return 135;
						case  197142 : return 136;
						case  197143 : return 137;
						case  197144 : return 138;
						case  197145 : return 139;
						case  197146 : return 140;
						case  197147 : return 140;
						case  197148 : return 141;
						case  197149 : return 142;
						case  197150 : return 143;
						case  197152 : return 144;
						case  197153 : return 145;
						case  197184 : return 146;
						case  197189 : return 147;
						case  197247 : return 148;
						case  197248 : return 149;
						case  197254 : return 150;
						case  197255 : return 151;
						case  197256 : return 152;
						case  197258 : return 153;
						case  197260 : return 154;
						case  197296 : return 155;
						case  197297 : return 156;
						case  197298 : return 157;
						case  197299 : return 158;
						case  197300 : return 159;
						case  197302 : return 160;
						case  197341 : return 161;
						case  197342 : return 162;
						case  197348 : return 163;
						case  197349 : return 164;
						case  197351 : return 165;
						case  197386 : return 166;
						case  197387 : return 167;
						case  197388 : return 168;
						default : return 199;
					}
				}
				if((fPeriod == kUndefinedPeriod) && (fDebug >= 1) ) 
				{
					AliWarning("Period not defined");
				}
				return 1;
			}

//_______________________________________________________________________________
Bool_t AliPHOSCorrelations::RejectTriggerMaskSelection()
{
	const Bool_t REJECT = true;
	const Bool_t ACCEPT = false;

	// No need to check trigger mask if no selection is done
	if( kNoSelection == fInternalTriggerSelection )
	return ACCEPT;

	Bool_t reject = REJECT;

	Bool_t isMB = (fEvent->GetTriggerMask() & (ULong64_t(1)<<1));
	Bool_t isCentral = (fEvent->GetTriggerMask() & (ULong64_t(1)<<4));
	Bool_t isSemiCentral = (fEvent->GetTriggerMask() & (ULong64_t(1)<<7));


	if( kCentralInclusive == fInternalTriggerSelection
	&& isCentral ) reject = ACCEPT; // accept event.
	else if( kCentralExclusive == fInternalTriggerSelection
	&& isCentral && !isSemiCentral && !isMB ) reject = ACCEPT; // accept event.

	else if( kSemiCentralInclusive == fInternalTriggerSelection
	&& isSemiCentral ) reject = ACCEPT; // accept event
	else if( kSemiCentralExclusive == fInternalTriggerSelection
	&& isSemiCentral && !isCentral && !isMB ) reject = ACCEPT; // accept event.

	else if( kMBInclusive == fInternalTriggerSelection
	&& isMB ) reject = ACCEPT; // accept event.
	else if( kMBExclusive == fInternalTriggerSelection
	&& isMB && !isCentral && !isSemiCentral ) reject = ACCEPT; // accept event.

	if( REJECT == reject )
	return REJECT;
	else {
	LogSelection(kInternalTriggerMaskSelection, fInternalRunNumber);
	return ACCEPT;
	}
}
//_______________________________________________________________________________
void AliPHOSCorrelations::SetVertex()
{
	const AliVVertex *primaryVertex = fEvent->GetPrimaryVertex();
	if( primaryVertex ) 
	{
		fVertex[0] = primaryVertex->GetX();
		fVertex[1] = primaryVertex->GetY();
		fVertex[2] = primaryVertex->GetZ();
	}
	else
	{
//		AliError("Event has 0x0 Primary Vertex, defaulting to origo");
		fVertex[0] = 0;
		fVertex[1] = 0;
		fVertex[2] = 0;
	}
	fVertexVector = TVector3(fVertex);

	fVtxBin=0 ;// No support for vtx binning implemented.
}
//_______________________________________________________________________________
Bool_t AliPHOSCorrelations::RejectEventVertex()
{
  if( ! fEvent->GetPrimaryVertex() )
    return true; // reject
  LogSelection(kHasVertex, fInternalRunNumber);
 
  if ( TMath::Abs(fVertexVector.z()) > fMaxAbsVertexZ )
    return true; // reject
  LogSelection(kHasAbsVertex, fInternalRunNumber);
 
  return false; // accept event.
}
//_______________________________________________________________________________
void AliPHOSCorrelations::SetCentrality()
{
	AliCentrality *centrality = fEvent->GetCentrality();
	if( centrality ) 
		fCentrality=centrality->GetCentralityPercentile(fCentralityEstimator);
	else 
	{
		AliError("Event has 0x0 centrality");
		fCentrality = -1.;
	}

	//cout<<"fCentrality: "<<fCentrality<<endl;
	//FillHistogram("hCentrality",fCentrality,fInternalRunNumber-0.5) ;
	fCentBin = GetCentralityBin(fCentrality);
}
//_______________________________________________________________________________
Bool_t AliPHOSCorrelations::RejectEventCentrality()
{
	if (fCentrality<fCentCutoffDown)
		return true; //reject
	if(fCentrality>fCentCutoffUp)
		return true;

	return false;  // accept event.
}
//_______________________________________________________________________________
void AliPHOSCorrelations::SetCentralityBinning(const TArrayD& edges, const TArrayI& nMixed){
// Define centrality bins by their edges
  for(int i=0; i<edges.GetSize()-1; ++i)
    if(edges.At(i) > edges.At(i+1)) AliFatal("edges are not sorted");
  if( edges.GetSize() != nMixed.GetSize()+1) AliFatal("edges and nMixed don't have appropriate relative sizes");
		  
  fCentEdges = edges;
  fCentNMixed = nMixed;
}
//_______________________________________________________________________________
Int_t AliPHOSCorrelations::GetCentralityBin(Float_t centralityV0M){
  int lastBinUpperIndex = fCentEdges.GetSize() -1;
  if( centralityV0M > fCentEdges[lastBinUpperIndex] ) {
    if( fDebug >= 1 )
      AliWarning( Form("centrality (%f) larger then upper edge of last centrality bin (%f)!", centralityV0M, fCentEdges[lastBinUpperIndex]) );
    return lastBinUpperIndex-1;
  }
  if( centralityV0M < fCentEdges[0] ) {
    if( fDebug >= 1 )
      AliWarning( Form("centrality (%f) smaller then lower edge of first bin (%f)!", centralityV0M, fCentEdges[0]) );
    return 0;
  }
		  
  fCentBin = TMath::BinarySearch<Double_t> ( GetNumberOfCentralityBins(), fCentEdges.GetArray(), centralityV0M );
  return fCentBin;
}
//_______________________________________________________________________________
void AliPHOSCorrelations::SetCentralityBorders (double down , double up ){
  if (down < 0. || up > 100 || up<=down)
     AliError( Form("Warning. Bad value of centrality borders. Setting as default: fCentCutoffDown=%2.f, fCentCutoffUp=%2.f",fCentCutoffDown,fCentCutoffUp) );
  else{
    fCentCutoffDown = down; 
    fCentCutoffUp = up;
    AliInfo( Form("Centrality border was set as fCentCutoffDown=%2.f, fCentCutoffUp=%2.f",fCentCutoffDown,fCentCutoffUp) );
  }
}

//_______________________________________________________________________________
void AliPHOSCorrelations::EvalReactionPlane()
{
	// assigns: fHaveTPCRP and fRP
	// also does a few histogram fills

	AliEventplane *eventPlane = fEvent->GetEventplane();
	if( ! eventPlane ) { AliError("Event has no event plane"); return; }

	Double_t reactionPlaneQ = eventPlane->GetEventplane("Q");

	if(reactionPlaneQ>=999 || reactionPlaneQ < 0.)
	{ 
		//reaction plain was not defined
		fHaveTPCRP = kFALSE;
	}
	else
	{
		fHaveTPCRP = kTRUE;
	}

	if(fHaveTPCRP)
		fRP = reactionPlaneQ;
	else
		fRP = 0.;
	
	FillHistogram("phiRPflat",fRP,fCentrality) ;
}
//_______________________________________________________________________________
Int_t AliPHOSCorrelations::GetRPBin()
{
	Double_t averageRP;
	averageRP = fRP ; 	// If possible, it is better to have EP bin from TPC
				// to have similar events for miximng (including jets etc)   (fRPV0A+fRPV0C+fRP) /3.;

	fEMRPBin = Int_t(fNEMRPBins*(averageRP)/TMath::Pi());

	if( fEMRPBin > (Int_t)fNEMRPBins-1 ) 
		fEMRPBin = fNEMRPBins-1 ;
	else 
	if(fEMRPBin < 0) fEMRPBin=0;

	return fEMRPBin;
}
//_______________________________________________________________________________
void AliPHOSCorrelations::SelectPhotonClusters()
{
	//Selects PHOS clusters

	// clear (or create) array for holding events photons/clusters
	if(fCaloPhotonsPHOS)
		fCaloPhotonsPHOS->Clear();
	else{
		fCaloPhotonsPHOS = new TClonesArray("AliCaloPhoton",200);
	}

	Int_t nclu = fEvent->GetNumberOfCaloClusters() ;
	Int_t inPHOS=0 ;
	for (Int_t i=0;  i<nclu;  i++) {
		AliVCluster *clu = fEvent->GetCaloCluster(i);	
		if ( !clu->IsPHOS() ) continue ;
		if( clu->E()< fMinClusterEnergy) continue; // reject cluster


		Double_t distBC=clu->GetDistanceToBadChannel();
		if(distBC<fMinBCDistance)
			continue ;

		if(clu->GetNCells() < fMinNCells) continue ;
		if(clu->GetM02() < fMinM02)   continue ;

		if(fTOFCutEnabled){
			Double_t tof = clu->GetTOF();
			if(TMath::Abs(tof) > fTOFCut ) continue ;
		}
		TLorentzVector lorentzMomentum;
		Double_t ecore = clu->GetMCEnergyFraction();

		clu->GetMomentum(lorentzMomentum, fVertex);
		lorentzMomentum*=ecore/lorentzMomentum.E() ;

		if(inPHOS>=fCaloPhotonsPHOS->GetSize()){
			fCaloPhotonsPHOS->Expand(inPHOS+50) ;
		}
        
		AliCaloPhoton * ph =new((*fCaloPhotonsPHOS)[inPHOS]) AliCaloPhoton(lorentzMomentum.X(),lorentzMomentum.Py(),lorentzMomentum.Z(),lorentzMomentum.E());
		inPHOS++ ;
		ph->SetCluster(clu);

		Float_t cellId=clu->GetCellAbsId(0) ;
		Int_t mod = (Int_t)TMath:: Ceil(cellId/(56*64) ) ; 
		ph->SetModule(mod) ;

		ph->SetNCells(clu->GetNCells());
		ph->SetDispBit(clu->GetDispersion()<2.5) ;
		ph->SetCPVBit(clu->GetEmcCpvDistance()>2.) ;

		Float_t  position[3];
		clu->GetPosition(position);
		TVector3 global(position) ;
		Int_t relId[4] ;
		fPHOSGeo->GlobalPos2RelId(global,relId) ;
		Int_t modPHOS  = relId[0] ;
		Int_t cellXPHOS = relId[2];
		Int_t cellZPHOS = relId[3] ;

		FillHistogram(Form("QA_cluXZE_mod%i", modPHOS), cellXPHOS, cellZPHOS, lorentzMomentum.E() ) ;
	}
}
//_______________________________________________________________________________
void AliPHOSCorrelations::SelectAccosiatedTracks()
{
	// clear (or create) array for holding events tracks
	if(fTracksTPC)
		fTracksTPC->Clear();
	else 
	{
		fTracksTPC = new TClonesArray("TLorentzVector",12000);
	}
	Int_t iTracks=0 ;
	for (Int_t i=0; i<fEvent->GetNumberOfTracks(); i++) 
	{
	  
		AliVParticle *track = fEvent->GetTrack(i);
	        	if(fEventESD){
  			if(!SelectESDTrack((AliESDtrack*)track)) continue ;
		}
		else{
  			if(!SelectAODTrack((AliAODTrack*)track)) continue ;		  
		}
		Double_t px = track->Px();
		Double_t py = track->Py();
		Double_t pz = track->Pz() ;
		Double_t e = track->E() ;
		
		if(iTracks>=fTracksTPC->GetSize())
                  		fTracksTPC->Expand(iTracks+50) ;
		
		new((*fTracksTPC)[iTracks]) TLorentzVector(px, py, pz,e);
		iTracks++ ;
	}
}
//_______________________________________________________________________________
void AliPHOSCorrelations::ConsiderPi0s()
{

  const Int_t nPHOS=fCaloPhotonsPHOS->GetEntriesFast() ;
  for(Int_t i1=0; i1 < nPHOS-1; i1++){
     AliCaloPhoton * ph1=(AliCaloPhoton*)fCaloPhotonsPHOS->At(i1) ;
     for (Int_t i2=i1+1; i2<nPHOS; i2++){
	AliCaloPhoton * ph2=(AliCaloPhoton*)fCaloPhotonsPHOS->At(i2) ;
	TLorentzVector p12  = *ph1  + *ph2;

	Double_t phiTrigger=p12.Phi() ;
	Double_t etaTrigger=p12.Eta() ;

	Double_t m=p12.M() ;
	Double_t pt=p12.Pt() ;
	int mod1 = ph1->Module() ;
	int mod2 = ph2->Module() ;
	

	FillHistogram("clu_phieta",phiTrigger,etaTrigger);
	FillHistogram("clusingle_phieta",ph1->Phi(), ph1->Eta());
	FillHistogram("clusingle_phieta",ph2->Phi(), ph2->Eta());


	FillHistogram("all_mpt",m, pt);
				
 	if ( ph1->IsCPVOK() && ph2->IsCPVOK() ) 
			FillHistogram("cpv_mpt",m, pt);

	if ( ph1->IsDispOK() && ph2->IsDispOK() ){
           FillHistogram("disp_mpt",m, pt);
	   if ( ph1->IsCPVOK() && ph2->IsCPVOK() ) {
     	      FillHistogram("both_mpt",m, pt);
	      if(mod1 == mod2) FillHistogram(Form("both%d_mpt",mod1),m, pt);
    	   }
	}
		  	
	if(!TestMass(m,pt)) continue;

     	// Take track's angles and compare with cluster's angles.
	for(Int_t i3=0; i3<fTracksTPC->GetEntriesFast(); i3++){
	   TLorentzVector * track = (TLorentzVector*)fTracksTPC->At(i3);

	   Double_t phiAssoc = track->Phi();
	   Double_t etaAssoc = track->Eta();
	   Double_t ptAssoc = track->Pt();

	   Double_t dPhi = phiAssoc - phiTrigger;
	   while (dPhi > 1.5*TMath::Pi()) dPhi-=2*TMath::Pi();
                while (dPhi < -.5*TMath::Pi()) dPhi+=2*TMath::Pi();

   	   Double_t dEta = etaAssoc - etaTrigger; 			
	   
	   Double_t ptAssocBin=GetAssocBin(ptAssoc) ;
	   FillHistogram(Form("all_ptphieta_ptAssoc_%3.1f",ptAssocBin), pt, dPhi, dEta);			
	   if ( ph1->IsCPVOK() && ph2->IsCPVOK() ) 
	     FillHistogram(Form("cpv_ptphieta_ptAssoc_%3.1f",ptAssocBin), pt, dPhi, dEta);			

 	   if ( ph1->IsDispOK() && ph2->IsDispOK() ){
	     FillHistogram(Form("disp_ptphieta_ptAssoc_%3.1f",ptAssocBin), pt, dPhi, dEta);			
	     if ( ph1->IsCPVOK() && ph2->IsCPVOK() ) 
	       FillHistogram(Form("both_ptphieta_ptAssoc_%3.1f",ptAssocBin), pt, dPhi, dEta);			
   	   }
	} 
     }
   }
}

//_______________________________________________________________________________
void AliPHOSCorrelations::ConsiderPi0sMix()
{
  TList * arrayList = GetCaloPhotonsPHOSList(fVtxBin, fCentBin, fEMRPBin);
  for(Int_t evi=0; evi<arrayList->GetEntries();evi++){
     TClonesArray * mixPHOS = static_cast<TClonesArray*>(arrayList->At(evi));
     for (Int_t i1=0; i1 < fCaloPhotonsPHOS->GetEntriesFast(); i1++){
	AliCaloPhoton * ph1 = (AliCaloPhoton*)fCaloPhotonsPHOS->At(i1) ;
	for(Int_t i2=0; i2<mixPHOS->GetEntriesFast(); i2++){
	  AliCaloPhoton * ph2 = (AliCaloPhoton*)mixPHOS->At(i2) ;
          TLorentzVector p12  = *ph1  + *ph2;
	  Double_t m=p12.M() ;
 	  Double_t pt=p12.Pt() ;
	  int mod1 = ph1->Module() ;
	  int mod2 = ph2->Module() ;

          FillHistogram("mix_all_mpt", m, pt);
	  if ( ph1->IsCPVOK() && ph2->IsCPVOK() ) 
	    FillHistogram("mix_cpv_mpt",m, pt);
	    if ( ph1->IsDispOK() && ph2->IsDispOK() ){
	      FillHistogram("mix_disp_mpt",m, pt);
	      if ( ph1->IsCPVOK() && ph2->IsCPVOK() ){
		 FillHistogram("mix_both_mpt",m, pt);
		 if (mod1 == mod2) FillHistogram(Form("mix_both%d_mpt",mod1),m, pt);
	      }
	    }
	}
     }
  }
}
//_______________________________________________________________________________
void AliPHOSCorrelations::ConsiderTracksMix()
{
  TList * arrayList = GetTracksTPCList(fVtxBin, fCentBin, fEMRPBin);
  for (Int_t i1=0; i1 < fCaloPhotonsPHOS->GetEntriesFast(); i1++) {
    AliCaloPhoton * ph1=(AliCaloPhoton*)fCaloPhotonsPHOS->At(i1) ;
    for (Int_t i2=0; i2<fCaloPhotonsPHOS->GetEntriesFast(); i2++){
      AliCaloPhoton * ph2=(AliCaloPhoton*)fCaloPhotonsPHOS->At(i2) ;
      TLorentzVector p12  = *ph1  + *ph2;
      Double_t phiTrigger=p12.Phi() ;
      Double_t etaTrigger=p12.Eta() ;

      Double_t m=p12.M() ;
      Double_t pt=p12.Pt() ;

      if(!TestMass(m,pt)) continue;
      for(Int_t evi=0; evi<arrayList->GetEntries();evi++){
 	TClonesArray * mixTracks = static_cast<TClonesArray*>(arrayList->At(evi));
	for(Int_t i3=0; i3<mixTracks->GetEntriesFast(); i3++){
  	  TLorentzVector * track = (TLorentzVector*)mixTracks->At(i3);		

  	  Double_t phiAssoc = track->Phi();
 	  Double_t etaAssoc = track->Eta();
	  Double_t ptAssoc =  track->Pt();

          Double_t ptAssocBin=GetAssocBin(ptAssoc) ;
	
	  Double_t dPhi = phiAssoc - phiTrigger;
	  while (dPhi > 1.5*TMath::Pi()) dPhi-=2*TMath::Pi();
	  while (dPhi < -.5*TMath::Pi()) dPhi+=2*TMath::Pi();

  	  Double_t dEta = etaAssoc - etaTrigger; 			
	  
	  FillHistogram(Form("mix_all_ptphieta_ptAssoc_%3.1f",ptAssocBin), pt, dPhi, dEta);				
 	  if ( ph1->IsCPVOK() && ph2->IsCPVOK() )
	    FillHistogram(Form("mix_cpv_ptphieta_ptAssoc_%3.1f",ptAssocBin), pt, dPhi, dEta);				

	  if ( ph1->IsDispOK() && ph2->IsDispOK() ){
	    FillHistogram(Form("mix_disp_ptphieta_ptAssoc_%3.1f",ptAssocBin), pt, dPhi, dEta);				
	    if ( ph1->IsCPVOK() && ph2->IsCPVOK() )
	      FillHistogram(Form("mix_both_ptphieta_ptAssoc_%3.1f",ptAssocBin), pt, dPhi, dEta);				
	  }
	}
     } 
   }
  }
}
//_______________________________________________________________________________
TList* AliPHOSCorrelations::GetCaloPhotonsPHOSList(UInt_t vtxBin, UInt_t centBin, UInt_t rpBin){

  int offset = vtxBin * GetNumberOfCentralityBins() * fNEMRPBins + centBin * fNEMRPBins + rpBin;
  if( fCaloPhotonsPHOSLists->At(offset) ) {
    TList* list = dynamic_cast<TList*> (fCaloPhotonsPHOSLists->At(offset));
    return list;
  }
  else{ // no list for this bin has been created, yet
    TList* list = new TList();
    fCaloPhotonsPHOSLists->AddAt(list, offset);
    return list;
  }
}
//_______________________________________________________________________________
TList* AliPHOSCorrelations::GetTracksTPCList(UInt_t vtxBin, UInt_t centBin, UInt_t rpBin){
		
  int offset = vtxBin * GetNumberOfCentralityBins() * fNEMRPBins + centBin * fNEMRPBins + rpBin;
  if( fTracksTPCLists->At(offset) ) { // list exists
     TList* list = dynamic_cast<TList*> (fTracksTPCLists->At(offset));
     return list;
  }
  else { // no list for this bin has been created, yet
    TList* list = new TList();
    fTracksTPCLists->AddAt(list, offset);
    return list;
  }
}
//_______________________________________________________________________________
Double_t AliPHOSCorrelations::GetAssocBin(Double_t pt){
  //Calculates bin 
  for(Int_t i=1; i<fAssocBins.GetSize(); i++){
    if(pt>fAssocBins.At(i-1) && pt<fAssocBins.At(i))
      return fAssocBins.At(i) ;
  }
  return fAssocBins.At(fAssocBins.GetSize()-1) ;
}
//_______________________________________________________________________________
void AliPHOSCorrelations::FillTrackEtaPhi()
{
	// Distribution TPC's tracks by angles.
	for (Int_t i1=0; i1<fTracksTPC->GetEntriesFast(); i1++){
		TLorentzVector * track = (TLorentzVector*)fTracksTPC->At(i1);
		FillHistogram( "track_phieta", track->Phi(), track->Eta() );
	}
}

//_______________________________________________________________________________
void AliPHOSCorrelations::UpdatePhotonLists()
{
  //Now we either add current events to stack or remove
  //If no photons in current event - no need to add it to mixed

  TList * arrayList = GetCaloPhotonsPHOSList(fVtxBin, fCentBin, fEMRPBin);
  if( fDebug >= 2 )
    AliInfo( Form("fCentBin=%d, fCentNMixed[]=%d",fCentBin,fCentNMixed[fCentBin]) );
  if(fCaloPhotonsPHOS->GetEntriesFast()>0)
  {
    arrayList->AddFirst(fCaloPhotonsPHOS) ;
    fCaloPhotonsPHOS=0x0;
    if(arrayList->GetEntries() > fCentNMixed[fCentBin])
    { // Remove redundant events
      TClonesArray * tmp = static_cast<TClonesArray*>(arrayList->Last()) ;
      arrayList->RemoveLast() ;
      delete tmp; 
    }
  }
}
//_______________________________________________________________________________
void AliPHOSCorrelations::UpdateTrackLists()
{
  //Now we either add current events to stack or remove
  //If no photons in current event - no need to add it to mixed

  TList * arrayList = GetTracksTPCList(fVtxBin, fCentBin, fEMRPBin);

  if( fDebug >= 2 )
    AliInfo( Form("fCentBin=%d, fCentNMixed[]=%d",fCentBin,fCentNMixed[fCentBin]) );
  if(fTracksTPC->GetEntriesFast()>0)
  {

    arrayList->AddFirst(fTracksTPC) ;
    fTracksTPC=0x0;
    if(arrayList->GetEntries() > fCentNMixed[fCentBin])
    { // Remove redundant events
      TClonesArray * tmp = static_cast<TClonesArray*>(arrayList->Last()) ;
      arrayList->RemoveLast() ;
      delete tmp; 
    }
  }
}
//_______________________________________________________________________________
Bool_t AliPHOSCorrelations::SelectESDTrack(AliESDtrack * t) const
// Estimate if this track can be used for the RP calculation. If all right - return "TRUE"
{
	Float_t pt=t->Pt();
	if(pt<0.5 || pt>10.) return kFALSE ;
	if(fabs( t->Eta() )>0.8) return kFALSE;
	if(!fESDtrackCuts->AcceptTrack(t)) return kFALSE ;
	return kTRUE ;
}
//_______________________________________________________________________________
Bool_t AliPHOSCorrelations::SelectAODTrack(AliAODTrack * t) const
// Estimate if this track can be used for the RP calculation. If all right - return "TRUE"
{
	Float_t pt=t->Pt();
	if(pt<0.5 || pt>10.) return kFALSE ;
	if(fabs( t->Eta() )>0.8) return kFALSE;
	if(fCheckHibridGlobal == kOnlyHibridTracks)
	{
		if(!t->IsHybridGlobalConstrainedGlobal()) 
			return kFALSE ;
	}

	if (fCheckHibridGlobal == kWithOutHibridTracks)
	{
		if(t->IsHybridGlobalConstrainedGlobal()) 
			return kFALSE ;
	}

	return kTRUE ;
}
//_______________________________________________________________________________
void AliPHOSCorrelations::SetPeriod(Period period)
{
	fPeriod = period;
}

//_______________________________________________________________________________
void AliPHOSCorrelations::LogProgress(int step)
// Fill "step by step" hist
{
  //FillHistogram("hSelEvents", step+0.5, internalRunNumber-0.5);
  FillHistogram("hTotSelEvents", step+0.5);
}
//_______________________________________________________________________________
void AliPHOSCorrelations::LogSelection(int step, int internalRunNumber)
{
  // the +0.5 is not realy neccisarry, but oh well... -henrik
  FillHistogram("hSelEvents", step+0.5, internalRunNumber-0.5);
  //FillHistogram("hTotSelEvents", step+0.5);
}
//_______________________________________________________________________________
Bool_t AliPHOSCorrelations::TestMass(Double_t m, Double_t /*pt*/)
{
	//Check if mair in pi0 peak window
	//To make pT-dependent 
	if (fSigmaWidth == 0.)
		return (fMassInvMean-fMassInvSigma<m && m<fMassInvMean+fMassInvSigma) ; 
	else
		return (fMassInvMean-fMassInvSigma*fSigmaWidth<m && m<fMassInvMean+fMassInvSigma*fSigmaWidth) ; 
} 
//_______________________________________________________________________________
void AliPHOSCorrelations::FillHistogram(const char * key,Double_t x)const
{
  //FillHistogram
  TH1 * hist = dynamic_cast<TH1*>(fOutputContainer->FindObject(key)) ;
  if(hist)
    hist->Fill(x) ;
  else
    AliError(Form("can not find histogram (of instance TH1) <%s> ",key)) ;
}
//_______________________________________________________________________________
void AliPHOSCorrelations::FillHistogram(const char * key, Double_t x, Double_t y) const
{
  //Fills 2D histograms with key
  TObject * obj = fOutputContainer->FindObject(key);
 
  TH2 * th2 = dynamic_cast<TH2*> (obj);
  if(th2) {
    th2->Fill(x, y) ;
    return;
  }

  AliError(Form("can not find histogram (of instance TH2) <%s> ",key)) ;
}
//_______________________________________________________________________________
void AliPHOSCorrelations::FillHistogram(const char * key,Double_t x, Double_t y, Double_t z) const
{
  //Fills 3D histograms with key
  TObject * obj = fOutputContainer->FindObject(key);
 
  TH3 * th3 = dynamic_cast<TH3*> (obj);
  if(th3) {
    th3->Fill(x, y, z) ;
    return;
  }
 
  AliError(Form("can not find histogram (of instance TH3) <%s> ",key)) ;
}
//_____________________________________________________________________________
void AliPHOSCorrelations::SetGeometry()
{
  // Initialize the PHOS geometry
  //Init geometry
  if(!fPHOSGeo){
     AliOADBContainer geomContainer("phosGeo");
     geomContainer.InitFromFile("$ALICE_ROOT/OADB/PHOS/PHOSGeometry.root","PHOSRotationMatrixes");
     TObjArray *matrixes = (TObjArray*)geomContainer.GetObject(fRunNumber,"PHOSRotationMatrixes");
     fPHOSGeo =  AliPHOSGeometry::GetInstance("IHEP") ;
     for(Int_t mod=0; mod<5; mod++) {
        if(!matrixes->At(mod)) {
          if( fDebug )
            AliInfo(Form("No PHOS Matrix for mod:%d, geo=%p\n", mod, fPHOSGeo));
          continue;
        }
        else {
             fPHOSGeo->SetMisalMatrix(((TGeoHMatrix*)matrixes->At(mod)),mod) ;
             if( fDebug >1 )
               AliInfo(Form("Adding PHOS Matrix for mod:%d, geo=%p\n", mod, fPHOSGeo));
        }
     }
  } 
}
 
