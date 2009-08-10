/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
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

//------------------------------------------------------------------------- 
// Jet AOD Reader 
// AOD reader for jet analysis
// This is the reader which must be used if the jet analysis task
// is executed after the ESD filter task, in order to read its output
//
// Author: Davide Perrino <davide.perrino@cern.ch>
//------------------------------------------------------------------------- 


#include <Riostream.h>
#include <TSystem.h>
#include <TLorentzVector.h>
#include <TVector3.h>
#include <TChain.h>
#include <TFile.h>
#include <TTask.h>
#include <TGeoManager.h>

#include "AliJetAODReader.h"
#include "AliJetAODReaderHeader.h"
#include "AliAODEvent.h"
#include "AliAODTrack.h"
#include "AliJetDummyGeo.h"
#include "AliJetAODFillUnitArrayTracks.h"
#include "AliJetAODFillUnitArrayEMCalDigits.h"
#include "AliJetHadronCorrection.h"
#include "AliJetUnitArray.h"

ClassImp(AliJetAODReader)

AliJetAODReader::AliJetAODReader():
    AliJetReader(),
    fAOD(0x0),
    fRef(new TRefArray),
    fDebug(0),
    fOpt(0),
    fGeom(0),
    fHadCorr(0x0),
    fTpcGrid(0x0),
    fEmcalGrid(0x0),
    fGrid0(0),
    fGrid1(0),
    fGrid2(0),
    fGrid3(0),
    fGrid4(0),
    fPtCut(0),
    fApplyElectronCorrection(kFALSE),
    fApplyMIPCorrection(kTRUE),
    fApplyFractionHadronicCorrection(kFALSE),
    fFractionHadronicCorrection(0.3),
    fNumUnits(0),
    fMass(0),
    fSign(0),
    fNIn(0),
    fDZ(0),
    fNeta(0),
    fNphi(0),
    fRefArray(0x0),
    fProcId(kFALSE)
{
  // Constructor    
}

//____________________________________________________________________________

AliJetAODReader::~AliJetAODReader()
{
  // Destructor
    delete fAOD;
    delete fRef;
    delete fTpcGrid;
    delete fEmcalGrid;
    if(fDZ)
      {
        delete fGrid0;
        delete fGrid1;
        delete fGrid2;
        delete fGrid3;
        delete fGrid4;
      }

}

//____________________________________________________________________________

void AliJetAODReader::OpenInputFiles()
{
    // Open the necessary input files
    // chain for the AODs
  fChain   = new TChain("aodTree");

  // get directory and pattern name from the header
   const char* dirName=fReaderHeader->GetDirectory();
   const char* pattern=fReaderHeader->GetPattern();

//   // Add files matching patters to the chain

   void *dir  = gSystem->OpenDirectory(dirName);
   const char *name = 0x0;
   int naod = ((AliJetAODReaderHeader*) fReaderHeader)->GetNaod();
   int a = 0;
   while ((name = gSystem->GetDirEntry(dir))){
       if (a>=naod) continue;
       
       if (strstr(name,pattern)){
	   char path[256];
	   sprintf(path,"%s/%s/aod.root",dirName,name);
	   fChain->AddFile(path);
	   a++;
       }
   }
  
  gSystem->FreeDirectory(dir);
  
  fAOD = 0;
  fChain->SetBranchAddress("AOD",&fAOD);
  
  int nMax = fChain->GetEntries(); 

  printf("\n AliJetAODReader: Total number of events in chain= %d \n",nMax);
  
  // set number of events in header
  if (fReaderHeader->GetLastEvent() == -1)
    fReaderHeader->SetLastEvent(nMax);
  else {
    Int_t nUsr = fReaderHeader->GetLastEvent();
    fReaderHeader->SetLastEvent(TMath::Min(nMax,nUsr));
  }
}

//____________________________________________________________________________

void AliJetAODReader::ConnectTree(TTree* tree, TObject* /*data*/) {
    // Connect the tree
    // For AOD reader it's needed only to set the number of events
     fChain = (TChain*)      tree;
     
     Int_t nMax = fChain->GetEntries(); 
     printf("\n AliJetAODReader: Total number of events in chain= %5d \n", nMax);
     // set number of events in header
     if (fReaderHeader->GetLastEvent() == -1)
	 fReaderHeader->SetLastEvent(nMax);
     else {
	 Int_t nUsr = fReaderHeader->GetLastEvent();
	 fReaderHeader->SetLastEvent(TMath::Min(nMax,nUsr));
     }
}

//____________________________________________________________________________

Bool_t AliJetAODReader::FillMomentumArray()
{
  // Clear momentum array
  ClearArray();
  fRef->Clear();
  fDebug = fReaderHeader->GetDebug();
  
  if (!fAOD) {
      return kFALSE;
  }
  
  // get number of tracks in event (for the loop)
  Int_t nt = fAOD->GetNTracks();
  printf("AOD tracks: %5d \t", nt);
  
  // temporary storage of signal and pt cut flag
  Int_t* sflag  = new Int_t[nt];
  Int_t* cflag  = new Int_t[nt];
  
  // get cuts set by user
  Float_t ptMin =  fReaderHeader->GetPtCut();
  Float_t etaMin = fReaderHeader->GetFiducialEtaMin();
  Float_t etaMax = fReaderHeader->GetFiducialEtaMax();  
  UInt_t  filterMask =  ((AliJetAODReaderHeader*)fReaderHeader)->GetTestFilterMask();

  //loop over tracks
  Int_t aodTrack = 0;
  Float_t pt, eta;
  TVector3 p3;

  for (Int_t it = 0; it < nt; it++) {
    AliAODTrack *track = fAOD->GetTrack(it);
    UInt_t status = track->GetStatus();
    
    Double_t mom[3] = {track->Px(),track->Py(),track->Pz()};
    p3.SetXYZ(mom[0],mom[1],mom[2]);
    pt = p3.Pt();
    eta = p3.Eta();
    if (status == 0) continue;
    if((filterMask>0)&&!(track->TestFilterBit(filterMask)))continue;
    if ( (eta > etaMax) || (eta < etaMin)) continue;      // checking eta cut

    new ((*fMomentumArray)[aodTrack]) TLorentzVector(p3,p3.Mag());
    sflag[aodTrack] = (TMath::Abs(track->GetLabel()) < 10000) ? 1 : 0;
    cflag[aodTrack] = ( pt > ptMin ) ? 1: 0;
    aodTrack++;
    fRef->Add(track);
  }
  printf("Used AOD tracks: %5d \n", aodTrack);
  // set the signal flags
  fSignalFlag.Set(aodTrack,sflag);
  fCutFlag.Set(aodTrack,cflag);

  delete [] sflag;
  delete [] cflag;
  
  return kTRUE;
}

//__________________________________________________________
void AliJetAODReader::SetApplyMIPCorrection(Bool_t val)
{
  //
  // Set flag to apply MIP correction fApplyMIPCorrection
  // - exclusive with fApplyFractionHadronicCorrection
  //

  fApplyMIPCorrection = val;
  if(fApplyMIPCorrection == kTRUE)
    {
      SetApplyFractionHadronicCorrection(kFALSE);
      printf("Enabling MIP Correction \n");
    }
  else
    {
      printf("Disabling MIP Correction \n");
    }
}

//__________________________________________________________
void AliJetAODReader::SetApplyFractionHadronicCorrection(Bool_t val)
{
  //
  // Set flag to apply EMC hadronic correction fApplyFractionHadronicCorrection
  // - exclusive with fApplyMIPCorrection
  //

  fApplyFractionHadronicCorrection = val;
  if(fApplyFractionHadronicCorrection == kTRUE)
    {
      SetApplyMIPCorrection(kFALSE);
      printf("Enabling Fraction Hadronic Correction \n");
    }
  else
    {
      printf("Disabling Fraction Hadronic Correction \n");
    }
}

//__________________________________________________________
void AliJetAODReader::SetFractionHadronicCorrection(Double_t val)
{
  //
  // Set value to fFractionHadronicCorrection (default is 0.3)
  // apply EMC hadronic correction fApplyFractionHadronicCorrection
  // - exclusive with fApplyMIPCorrection
  //

  fFractionHadronicCorrection = val;
  if(fFractionHadronicCorrection > 0.0 && fFractionHadronicCorrection <= 1.0)
    {
      SetApplyFractionHadronicCorrection(kTRUE);
      printf("Fraction Hadronic Correction %1.3f \n",fFractionHadronicCorrection);
    }
  else
    {
      SetApplyFractionHadronicCorrection(kFALSE);
    }
}

//____________________________________________________________________________
void AliJetAODReader::CreateTasks(TChain* tree)
{
  //
  // For reader task initialization
  //

  fDebug = fReaderHeader->GetDebug();
  fDZ = fReaderHeader->GetDZ();
  fTree = tree;

  // Init EMCAL geometry and create UnitArray object
  SetEMCALGeometry();
  //  cout << "In create task" << endl;
  InitParameters();
  InitUnitArray();

  fFillUnitArray = new TTask("fFillUnitArray","Fill unit array jet finder");
  fFillUAFromTracks = new AliJetAODFillUnitArrayTracks();
  fFillUAFromTracks->SetReaderHeader(fReaderHeader);
  fFillUAFromTracks->SetGeom(fGeom);
  fFillUAFromTracks->SetTPCGrid(fTpcGrid);
  fFillUAFromTracks->SetEMCalGrid(fEmcalGrid);

  if(fDZ)
    {
      fFillUAFromTracks->SetGrid0(fGrid0);
      fFillUAFromTracks->SetGrid1(fGrid1);
      fFillUAFromTracks->SetGrid2(fGrid2);
      fFillUAFromTracks->SetGrid3(fGrid3);
      fFillUAFromTracks->SetGrid4(fGrid4);
    }
  fFillUAFromTracks->SetApplyMIPCorrection(fApplyMIPCorrection);
  fFillUAFromTracks->SetHadCorrector(fHadCorr);
  fFillUAFromEMCalDigits = new AliJetAODFillUnitArrayEMCalDigits();
  fFillUAFromEMCalDigits->SetReaderHeader(fReaderHeader);
  fFillUAFromEMCalDigits->SetGeom(fGeom);
  fFillUAFromEMCalDigits->SetTPCGrid(fTpcGrid);
  fFillUAFromEMCalDigits->SetEMCalGrid(fEmcalGrid);
  fFillUAFromEMCalDigits->SetApplyFractionHadronicCorrection(fApplyFractionHadronicCorrection);
  fFillUAFromEMCalDigits->SetFractionHadronicCorrection(fFractionHadronicCorrection);
  fFillUAFromEMCalDigits->SetApplyElectronCorrection(fApplyElectronCorrection);

  fFillUnitArray->Add(fFillUAFromTracks);
  fFillUnitArray->Add(fFillUAFromEMCalDigits);
  fFillUAFromTracks->SetActive(kFALSE);
  fFillUAFromEMCalDigits->SetActive(kFALSE);

  cout << "Tasks instantiated at that stage ! " << endl;
  cout << "You can loop over events now ! " << endl;

}

//____________________________________________________________________________
Bool_t AliJetAODReader::ExecTasks(Bool_t procid, TRefArray* refArray)
{
  //
  // Main function
  // Fill the reader part
  //

  fProcId = procid;
  fRefArray = refArray;
//(not used ?)  Int_t nEntRef = fRefArray->GetEntries();
//(not used ?)  Int_t nEntUnit = fUnitArray->GetEntries();

  // clear momentum array
  ClearArray();

  fDebug = fReaderHeader->GetDebug();
  fOpt = fReaderHeader->GetDetector();

  if(!fAOD) {
    return kFALSE;
  }

  // TPC only or Digits+TPC or Clusters+TPC
  if(fOpt%2==!0 && fOpt!=0){
    fFillUAFromTracks->SetAOD(fAOD);
    fFillUAFromTracks->SetActive(kTRUE);
    fFillUAFromTracks->SetUnitArray(fUnitArray);
    fFillUAFromTracks->SetRefArray(fRefArray);
    fFillUAFromTracks->SetProcId(fProcId);
    //    fFillUAFromTracks->ExecuteTask("tpc"); // => Temporarily changed
    fFillUAFromTracks->Exec("tpc");
    if(fOpt==1){
      fNumCandidate = fFillUAFromTracks->GetMult();
      fNumCandidateCut = fFillUAFromTracks->GetMultCut();
    }
  }

  // Digits only or Digits+TPC
  if(fOpt>=2 && fOpt<=3){
    fFillUAFromEMCalDigits->SetAOD(fAOD);
    fFillUAFromEMCalDigits->SetActive(kTRUE);
    fFillUAFromEMCalDigits->SetUnitArray(fUnitArray);
    fFillUAFromEMCalDigits->SetRefArray(fRefArray);
    fFillUAFromEMCalDigits->SetProcId(fFillUAFromTracks->GetProcId());
    fFillUAFromEMCalDigits->SetInitMult(fFillUAFromTracks->GetMult());
    fFillUAFromEMCalDigits->SetInitMultCut(fFillUAFromTracks->GetMultCut());
    fFillUAFromEMCalDigits->Exec("digits"); // => Temporarily added
    fNumCandidate = fFillUAFromEMCalDigits->GetMult();
    fNumCandidateCut = fFillUAFromEMCalDigits->GetMultCut();
  }

  //  fFillUnitArray->ExecuteTask(); // => Temporarily commented

  return kTRUE;
}

//____________________________________________________________________________
Bool_t AliJetAODReader::SetEMCALGeometry()
{
  // 
  // Set the EMCal Geometry
  //
  
  if (!fTree->GetFile()) 
    return kFALSE;

  TString geomFile(fTree->GetFile()->GetName());
  geomFile.ReplaceAll("AliESDs", "geometry");
  
  // temporary workaround for PROOF bug #18505
  geomFile.ReplaceAll("#geometry.root#geometry.root", "#geometry.root");
  if(fDebug>1) printf("Current geometry file %s \n", geomFile.Data());

  // Define EMCAL geometry to be able to read ESDs
  fGeom = AliJetDummyGeo::GetInstance();
  if (fGeom == 0)
    fGeom = AliJetDummyGeo::GetInstance("EMCAL_COMPLETE","EMCAL");
  
  // To be setted to run some AliEMCALGeometry functions
  TGeoManager::Import(geomFile);
  fGeom->GetTransformationForSM();  
  printf("\n EMCal Geometry set ! \n");
  
  return kTRUE;
  
}

//____________________________________________________________________________
void AliJetAODReader::InitParameters()
{
  // Initialise parameters
  fOpt = fReaderHeader->GetDetector();
  //  fHCorrection    = 0;          // For hadron correction
  fHadCorr        = 0;          // For hadron correction
  if(fEFlag==kFALSE){
    if(fOpt==0 || fOpt==1)
      fECorrection    = 0;        // For electron correction
    else fECorrection = 1;        // For electron correction
  }
  fNumUnits       = fGeom->GetNCells();      // Number of cells in EMCAL
  if(fDebug>1) printf("\n EMCal parameters initiated ! \n");
}

//____________________________________________________________________________
void AliJetAODReader::InitUnitArray()
{
  //Initialises unit arrays
  Int_t nElements = fTpcGrid->GetNEntries();
  Float_t eta = 0., phi = 0., deltaEta = 0., deltaPhi = 0.;
  if(fArrayInitialised) fUnitArray->Delete();

  if(fTpcGrid->GetGridType()==0)
    { // Fill the following quantities :
      // Good track ID, (Eta,Phi) position ID, eta, phi, energy, px, py, pz, deltaEta, deltaPhi,
      // detector flag, in/out jet, pt cut, mass, cluster ID)
      for(Int_t nBin = 1; nBin < nElements+1; nBin++)
        {
          //      fTpcGrid->GetEtaPhiFromIndex2(nBin,eta,phi);
          fTpcGrid->GetEtaPhiFromIndex2(nBin,phi,eta);
          phi = ((phi < 0) ? phi + 2. * TMath::Pi() : phi);
          deltaEta = fTpcGrid->GetDeta();
          deltaPhi = fTpcGrid->GetDphi();
          new ((*fUnitArray)[nBin-1]) AliJetUnitArray(nBin-1,0,eta,phi,0.,deltaEta,deltaPhi,kTpc,kOutJet,kPtSmaller,kPtSmaller,kBad,0.,-1);
        }
    }

  if(fTpcGrid->GetGridType()==1)
    {
      Int_t nGaps = 0;
      Int_t n0 = 0, n1 = 0, n2 = 0, n3 = 0, n4 = 0;

      if(fDZ)
        {
          // Define a grid of cell for the gaps between SM
          Double_t phimin0 = 0., phimin1 = 0., phimin2 = 0., phimin3 = 0., phimin4 = 0.;
          Double_t phimax0 = 0., phimax1 = 0., phimax2 = 0., phimax3 = 0., phimax4 = 0.;
          fGeom->GetPhiBoundariesOfSMGap(0,phimin0,phimax0);
          fGrid0 = new AliJetGrid(0,95,phimin0,phimax0,-0.7,0.7); // 0.015 x 0.015
          fGrid0->SetGridType(0);
          fGrid0->SetMatrixIndexes();
          fGrid0->SetIndexIJ();
          n0 = fGrid0->GetNEntries();
          fGeom->GetPhiBoundariesOfSMGap(1,phimin1,phimax1);
          fGrid1 = new AliJetGrid(0,95,phimin1,phimax1,-0.7,0.7); // 0.015 x 0.015
          fGrid1->SetGridType(0);
          fGrid1->SetMatrixIndexes();
          fGrid1->SetIndexIJ();
          n1 = fGrid1->GetNEntries();
          fGeom->GetPhiBoundariesOfSMGap(2,phimin2,phimax2);
          fGrid2 = new AliJetGrid(0,95,phimin2,phimax2,-0.7,0.7); // 0.015 x 0.015
          fGrid2->SetGridType(0);
          fGrid2->SetMatrixIndexes();
          fGrid2->SetIndexIJ();
          n2 = fGrid2->GetNEntries();
          fGeom->GetPhiBoundariesOfSMGap(3,phimin3,phimax3);
          fGrid3 = new AliJetGrid(0,95,phimin3,phimax3,-0.7,0.7); // 0.015 x 0.015
          fGrid3->SetGridType(0);
          fGrid3->SetMatrixIndexes();
          fGrid3->SetIndexIJ();
          n3 = fGrid3->GetNEntries();
          fGeom->GetPhiBoundariesOfSMGap(4,phimin4,phimax4);
          fGrid4 = new AliJetGrid(0,95,phimin4,phimax4,-0.7,0.7); // 0.015 x 0.015
          fGrid4->SetGridType(0);
          fGrid4->SetMatrixIndexes();
          fGrid4->SetIndexIJ();
          n4 = fGrid4->GetNEntries();

          nGaps = n0+n1+n2+n3+n4;

        }

      for(Int_t nBin = 0; nBin < fNumUnits+nElements+nGaps; nBin++) 
	{
	  if(nBin<fNumUnits)
	    {
	      fGeom->EtaPhiFromIndex(nBin, eta, phi); // From EMCal geometry 
	      // fEmcalGrid->GetEtaPhiFromIndex2(nBin,phi,eta); // My function from Grid
	      phi = ((phi < 0) ? phi + 2. * TMath::Pi() : phi);
	      deltaEta = fEmcalGrid->GetDeta(); // Modify with the exact detector values
	      deltaPhi = fEmcalGrid->GetDphi(); // Modify with the exact detector values
	      new ((*fUnitArray)[nBin]) AliJetUnitArray(nBin,0,eta,phi,0.,deltaEta,deltaPhi,kTpc,kOutJet,kPtSmaller,kPtSmaller,kBad,0.,-1);
	    } 
	  else {
	    if(nBin>=fNumUnits && nBin<fNumUnits+nElements){
	      fTpcGrid->GetEtaPhiFromIndex2(nBin+1-fNumUnits,phi,eta);
	      phi = ((phi < 0) ? phi + 2. * TMath::Pi() : phi);
	      deltaEta = fTpcGrid->GetDeta();
	      deltaPhi = fTpcGrid->GetDphi();
	      new ((*fUnitArray)[nBin]) AliJetUnitArray(nBin,0,eta,phi,0.,deltaEta,deltaPhi,kTpc,kOutJet,kPtSmaller,kPtSmaller,kBad,0.,-1);
	    }
	    else {
	      if(fDZ) {
		if(nBin>=fNumUnits+nElements && nBin<fNumUnits+nElements+nGaps){
		  if(nBin<fNumUnits+nElements+n0)
		    {
		      phi = eta = 0.;
		      fGrid0->GetEtaPhiFromIndex2(nBin+1-(fNumUnits+nElements),phi,eta);
		      deltaEta = fGrid0->GetDeta(); 
		      deltaPhi = fGrid0->GetDphi(); 
		      new ((*fUnitArray)[nBin]) AliJetUnitArray(nBin,0,eta,phi,0.,deltaEta,deltaPhi,kTpc,kOutJet,kPtSmaller,kPtSmaller,kBad,0.,-1);
		    }
		  else if(nBin>=fNumUnits+nElements+n0 && nBin<fNumUnits+nElements+n0+n1)
		    {
		      phi = eta = 0.;
		      fGrid1->GetEtaPhiFromIndex2(nBin+1-(fNumUnits+nElements+n0),phi,eta);
		      deltaEta = fGrid1->GetDeta(); 
		      deltaPhi = fGrid1->GetDphi(); 
		      new ((*fUnitArray)[nBin]) AliJetUnitArray(nBin,0,eta,phi,0.,deltaEta,deltaPhi,kTpc,kOutJet,kPtSmaller,kPtSmaller,kBad,0.,-1);
		    }
		  else if(nBin>=fNumUnits+nElements+n0+n1 && nBin<fNumUnits+nElements+n0+n1+n2)
		    {
		      phi = eta = 0.;
		      fGrid2->GetEtaPhiFromIndex2(nBin+1-(fNumUnits+nElements+n0+n1),phi,eta);
		      deltaEta = fGrid2->GetDeta(); 
		      deltaPhi = fGrid2->GetDphi(); 
		      new ((*fUnitArray)[nBin]) AliJetUnitArray(nBin,0,eta,phi,0.,deltaEta,deltaPhi,kTpc,kOutJet,kPtSmaller,kPtSmaller,kBad,0.,-1);
		    }
		  else if(nBin>=fNumUnits+nElements+n0+n1+n2 && nBin<fNumUnits+nElements+n0+n1+n2+n3)
		    {
		      phi = eta = 0.;
		      fGrid3->GetEtaPhiFromIndex2(nBin+1-(fNumUnits+nElements+n0+n1+n2),phi,eta);
		      deltaEta = fGrid3->GetDeta(); 
		      deltaPhi = fGrid3->GetDphi(); 
		      new ((*fUnitArray)[nBin]) AliJetUnitArray(nBin,0,eta,phi,0.,deltaEta,deltaPhi,kTpc,kOutJet,kPtSmaller,kPtSmaller,kBad,0.,-1);
		    }
		  else if(nBin>=fNumUnits+nElements+n0+n1+n2+n3 && nBin<fNumUnits+nElements+nGaps)
		    {
		      phi = eta = 0.;
		      fGrid4->GetEtaPhiFromIndex2(nBin+1-(fNumUnits+nElements+n0+n1+n2+n3),phi,eta);
		      deltaEta = fGrid4->GetDeta(); 
		      deltaPhi = fGrid4->GetDphi(); 
		      new ((*fUnitArray)[nBin]) AliJetUnitArray(nBin,0,eta,phi,0.,deltaEta,deltaPhi,kTpc,kOutJet,kPtSmaller,kPtSmaller,kBad,0.,-1);
		    }
		}
	      } // end if(fDZ)
	    } // end else 2
	  } // end else 1
	} // end loop on nBin
    } // end grid type == 1
  fArrayInitialised = 1;
}
