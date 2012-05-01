// $Id: AliEmcalIsolatedPhotonsTask.cxx  $

#include <TChain.h>
#include <TClonesArray.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TList.h>
#include <TLorentzVector.h>
#include <TParticle.h>

#include "AliAnalysisManager.h"
#include "AliCentrality.h"
#include "AliVCluster.h"
#include "AliESDtrack.h"
#include "AliEmcalJet.h"
#include "AliAODTrack.h"
#include "AliEmcalJet.h"
#include "AliVEventHandler.h"
#include "AliPicoTrack.h"

#include "AliEmcalIsolatedPhotonsTask.h"

ClassImp(AliEmcalIsolatedPhotonsTask)

//________________________________________________________________________
AliEmcalIsolatedPhotonsTask::AliEmcalIsolatedPhotonsTask() : 
  AliAnalysisTaskSE("AliEmcalIsolatedPhotonsTask"),
  fOutput(0),
  fTracksName("Tracks"),
  fCaloName("CaloClusters"),
  fJetsName("Jets"),
  fTrgClusName("ClustersL1GAMMAFEE"),
  fTracks(0),
  fCaloClusters(0),
  fJets(0),
  fTrgClusters(0),
  fCent(0),
  fHistCentrality(0),
  Ptbins(400),
  Ptlow(0),
  Ptup(200),
  Ebins(400),
  Elow(0),
  Eup(200)
{
  // Default constructor.
  fBranchNames="ESD:AliESDRun.,AliESDHeader.,PrimaryVertex.";

  for (Int_t i = 0; i < 4; i++) {
    fHistJetsE[i] = 0;
    fHistJetsNE[i] = 0;
    fHistJetsNEF[i] = 0;
    fHistJetsZ[i] = 0;
    fHistLeadingJetE[i] = 0;
    fHistTracksPtLJ[i] = 0;
    fHistClusELJ[i] = 0;
    fHistTracksPtBkg[i] = 0;
    fHistClusEBkg[i] = 0;
  }
}

//________________________________________________________________________
AliEmcalIsolatedPhotonsTask::AliEmcalIsolatedPhotonsTask(const char *name) : 
  AliAnalysisTaskSE("AliEmcalIsolatedPhotonsTask"),
  fOutput(0),
  fTracksName("Tracks"),
  fCaloName("CaloClusters"),
  fJetsName("Jets"),
  fTrgClusName("ClustersL1GAMMAFEE"),
  fTracks(0),
  fCaloClusters(0),
  fJets(0),
  fTrgClusters(0),
  fCent(0),
  fHistCentrality(0),
  Ptbins(400),
  Ptlow(0),
  Ptup(200),
  Ebins(400),
  Elow(0),
  Eup(200)
{
  // Standard constructor.

  if (!name)
    return;

  SetName(name);
  fBranchNames="ESD:AliESDRun.,AliESDHeader.,PrimaryVertex.";

  for (Int_t i = 0; i < 4; i++) {
    fHistJetsE[i] = 0;
    fHistJetsNE[i] = 0;
    fHistJetsNEF[i] = 0;
    fHistJetsZ[i] = 0;
    fHistLeadingJetE[i] = 0;
    fHistTracksPtLJ[i] = 0;
    fHistClusELJ[i] = 0;
    fHistTracksPtBkg[i] = 0;
    fHistClusEBkg[i] = 0;
  }

  DefineInput(0,TChain::Class());
  DefineOutput(1,TList::Class());
}

//________________________________________________________________________
AliEmcalIsolatedPhotonsTask::~AliEmcalIsolatedPhotonsTask()
{
  // Destructor
}

//________________________________________________________________________
void AliEmcalIsolatedPhotonsTask::UserCreateOutputObjects()
{
  // Create histograms
  
  fOutput = new TList();
  fOutput->SetOwner();  // IMPORTANT!
  
  fHistCentrality = new TH1F("fHistCentrality","Event centrality distribution", Ebins, 0, 100);
  fHistCentrality->GetXaxis()->SetTitle("Centrality (%)");
  fHistCentrality->GetYaxis()->SetTitle("counts");
  fOutput->Add(fHistCentrality);

  TString histname;

  for (Int_t i = 0; i < 4; i++) {
    histname = "fHistJetsE_";
    histname += i;
    fHistJetsE[i] = new TH1F(histname.Data(), histname.Data(), Ebins, Elow, Eup);
    fHistJetsE[i]->GetXaxis()->SetTitle("E [GeV]");
    fHistJetsE[i]->GetYaxis()->SetTitle("counts");
    fOutput->Add(fHistJetsE[i]);
    
    histname = "fHistJetsNE_";
    histname += i;
    fHistJetsNE[i] = new TH1F(histname.Data(), histname.Data(), Ebins, Elow, Eup);
    fHistJetsNE[i]->GetXaxis()->SetTitle("E [GeV]");
    fHistJetsNE[i]->GetYaxis()->SetTitle("counts");
    fOutput->Add(fHistJetsNE[i]);
    
    histname = "fHistJetsNEF_";
    histname += i;
    fHistJetsNEF[i] = new TH1F(histname.Data(), histname.Data(), Ebins, 0, 1.2);
    fHistJetsNEF[i]->GetXaxis()->SetTitle("NEF");
    fHistJetsNEF[i]->GetYaxis()->SetTitle("counts");
    fOutput->Add(fHistJetsNEF[i]);

    histname = "fHistJetsZ_";
    histname += i;
    fHistJetsZ[i] = new TH1F(histname.Data(), histname.Data(), Ebins, 0, 1.2);
    fHistJetsZ[i]->GetXaxis()->SetTitle("Z");
    fHistJetsZ[i]->GetYaxis()->SetTitle("counts");
    fOutput->Add(fHistJetsZ[i]);

    histname = "fHistLeadingJetE_";
    histname += i;
    fHistLeadingJetE[i] = new TH1F(histname.Data(), histname.Data(), Ebins, Elow, Eup);
    fHistLeadingJetE[i]->GetXaxis()->SetTitle("E [GeV]");
    fHistLeadingJetE[i]->GetYaxis()->SetTitle("counts");
    fOutput->Add(fHistLeadingJetE[i]);
    
    histname = "fHistTracksPtLJ_";
    histname += i;
    fHistTracksPtLJ[i] = new TH1F(histname.Data(), histname.Data(), Ptbins, Ptlow, Ptup);
    fHistTracksPtLJ[i]->GetXaxis()->SetTitle("P_{T} [GeV/c]");
    fHistTracksPtLJ[i]->GetYaxis()->SetTitle("counts");
    fOutput->Add(fHistTracksPtLJ[i]);
    
    histname = "fHistClusELJ_";
    histname += i;
    fHistClusELJ[i] = new TH1F(histname.Data(), histname.Data(), Ebins, Elow, Eup);
    fHistClusELJ[i]->GetXaxis()->SetTitle("E [GeV]");
    fHistClusELJ[i]->GetYaxis()->SetTitle("counts");
    fOutput->Add(fHistClusELJ[i]);

    histname = "fHistTracksPtBkg_";
    histname += i;
    fHistTracksPtBkg[i] = new TH1F(histname.Data(), histname.Data(), Ptbins, Ptlow, Ptup);
    fHistTracksPtBkg[i]->GetXaxis()->SetTitle("P_{T} [GeV/c]");
    fHistTracksPtBkg[i]->GetYaxis()->SetTitle("counts");
    fOutput->Add(fHistTracksPtBkg[i]);
    
    histname = "fHistClusEBkg_";
    histname += i;
    fHistClusEBkg[i] = new TH1F(histname.Data(), histname.Data(), Ebins, Elow, Eup);
    fHistClusEBkg[i]->GetXaxis()->SetTitle("E [GeV]");
    fHistClusEBkg[i]->GetYaxis()->SetTitle("counts");
    fOutput->Add(fHistClusEBkg[i]);
  }

  PostData(1, fOutput); // Post data for ALL output slots >0 here, to get at least an empty histogram
}

void AliEmcalIsolatedPhotonsTask::RetrieveEventObjects()
{
  fCaloClusters =  dynamic_cast<TClonesArray*>(InputEvent()->FindListObject(fCaloName));
  if (!fCaloClusters) {
    AliWarning(Form("Could not retrieve clusters!")); 
  }

  fTracks = dynamic_cast<TClonesArray*>(InputEvent()->FindListObject(fTracksName));
  if (!fTracks) {
    AliWarning(Form("Could not retrieve tracks!")); 
  }

  fJets = dynamic_cast<TClonesArray*>(InputEvent()->FindListObject(fJetsName));
  if (!fJets) {
    AliWarning(Form("Could not retrieve jets!")); 
  }

  if (strcmp(fTrgClusName,"")) {
    fTrgClusters =  dynamic_cast<TClonesArray*>(InputEvent()->FindListObject(fTrgClusName));
    if (!fTrgClusters) {
      AliWarning(Form("Could not retrieve trigger clusters!")); 
    }
  }

  fCent = InputEvent()->GetCentrality();
}

AliVTrack* AliEmcalIsolatedPhotonsTask::GetTrack(const Int_t i) const
{
  if (fTracks)
    return dynamic_cast<AliVTrack*>(fTracks->At(i));
  else
    return 0;
}

Int_t AliEmcalIsolatedPhotonsTask::GetNumberOfTracks() const
{
  if (fTracks)
    return fTracks->GetEntriesFast();
  else
    return 0;
}

AliVCluster* AliEmcalIsolatedPhotonsTask::GetCaloCluster(const Int_t i) const
{ 
  if (fCaloClusters)
    return dynamic_cast<AliVCluster*>(fCaloClusters->At(i));
  else
    return 0;
}

Int_t AliEmcalIsolatedPhotonsTask::GetNumberOfCaloClusters() const
{ 
  if (fCaloClusters)
    return fCaloClusters->GetEntriesFast();
  else
    return 0;
}

AliEmcalJet* AliEmcalIsolatedPhotonsTask::GetJet(const Int_t i) const
{
  if (fJets)
    return dynamic_cast<AliEmcalJet*>(fJets->At(i));
  else
    return 0;
}

Int_t AliEmcalIsolatedPhotonsTask::GetNumberOfJets() const
{
  if (fJets)
    return fJets->GetEntriesFast();
  else
    return 0;
}

AliVCluster* AliEmcalIsolatedPhotonsTask::GetTrgCluster(const Int_t i) const
{
  if (fTrgClusters)
    return dynamic_cast<AliVCluster*>(fTrgClusters->At(i));
  else
    return 0;
}

Int_t AliEmcalIsolatedPhotonsTask::GetNumberOfTrgClusters() const
{
  if (fTrgClusters)
    return fTrgClusters->GetEntriesFast();
  else
    return 0;
}

void AliEmcalIsolatedPhotonsTask::FillHistograms()
{
  Float_t cent = fCent->GetCentralityPercentile("V0M");

  fHistCentrality->Fill(cent);

  Int_t centbin=-1;
  if(cent>=0 && cent<10) centbin=0;
  else if(cent>=10 && cent<30) centbin=1;
  else if(cent>=30 && cent<50) centbin=2;
  else if(cent>=50 && cent<=100) centbin=3;

  // Jet loop
  Int_t njets =  GetNumberOfJets();
  //cout << njets << " jets" << endl;
  Float_t maxJetEnergy = 0;
  Int_t maxJetIndex = -1;
  for (Int_t ij = 0; ij < njets; ij++) {

    AliEmcalJet* jet = GetJet(ij);

    if (!jet) {
      printf("ERROR: Could not receive jet %d\n", ij);
      continue;
    }  
    
    if (jet->E() <= 0)
      continue;

    fHistJetsE[centbin]->Fill(jet->E());
    fHistJetsNEF[centbin]->Fill(jet->NEF());
    fHistJetsNE[centbin]->Fill(jet->E() * jet->NEF());

    //cout << "****** jet id = " << ij << " ******" << endl;

    for (Int_t it = 0; it < jet->GetNumberOfTracks(); it++) {
      Int_t trackid = jet->TrackAt(it);
      AliVTrack *track = GetTrack(trackid);
      //cout << "track id = " << trackid << endl;
      if (track)
	fHistJetsZ[centbin]->Fill(track->Pt() / jet->E());
    }
    for (Int_t ic = 0; ic < jet->GetNumberOfClusters(); ic++) {
      Int_t clusterid = jet->ClusterAt(ic);
      //cout << "cluster id = " << clusterid << endl;
      AliVCluster *cluster = GetCaloCluster(clusterid);
      if (cluster)
	fHistJetsZ[centbin]->Fill(cluster->E() / jet->E());
    }

    if (maxJetEnergy < jet->E()) {
      maxJetEnergy = jet->E();
      maxJetIndex = ij;
    }
  } //jet loop 

  
  if (!(maxJetEnergy > 0) || maxJetIndex < 0) 
    return;

  fHistLeadingJetE[centbin]->Fill(maxJetEnergy);
  
  AliEmcalJet* jet = GetJet(maxJetIndex);
  if (!jet)
    return;
  
  jet->SortConstituents();
  
  // Cluster loop
  Int_t clusJetId = 0;
  Int_t nclusters =  GetNumberOfCaloClusters();
  for (Int_t iClusters = 0; iClusters < nclusters; iClusters++) {
    AliVCluster* cluster = GetCaloCluster(iClusters);
    if (!cluster) {
      printf("ERROR: Could not receive cluster %d\n", iClusters);
      continue;
    }  
    
    if (!(cluster->IsEMCAL())) continue;
    
    if (jet->GetNumberOfClusters() > 0) {
      if (jet->ClusterAt(clusJetId) < iClusters) {
	if (clusJetId < jet->GetNumberOfClusters() - 1)
	  clusJetId++;
      }
      
      if (jet->ClusterAt(clusJetId) == iClusters) {
	fHistClusELJ[centbin]->Fill(cluster->E());
      }
      else {
	fHistClusEBkg[centbin]->Fill(cluster->E());
      }
    }
    else {
      fHistClusEBkg[centbin]->Fill(cluster->E());
    }
  } //cluster loop 
  
  
  // Track loop 
  Int_t trackJetId = 0;
  Int_t ntracks = GetNumberOfTracks();
  for(Int_t iTracks = 0; iTracks < ntracks; iTracks++) {
    AliVTrack* track = GetTrack(iTracks);         
    if(!track) {
      AliError(Form("ERROR: Could not retrieve track %d",iTracks)); 
      continue; 
    }
    
    if (!AcceptTrack(track)) continue;
    
    if (jet->GetNumberOfTracks() > 0) {
      if (jet->TrackAt(trackJetId) < iTracks) {
	if (trackJetId < jet->GetNumberOfTracks() - 1)
	  trackJetId++;
      }
      
      if (jet->TrackAt(trackJetId) == iTracks) {
	fHistTracksPtLJ[centbin]->Fill(track->Pt());
      }
      else {
	fHistTracksPtBkg[centbin]->Fill(track->Pt());
      }
    }
    else {
      fHistTracksPtBkg[centbin]->Fill(track->Pt());
    }
  }
}


//________________________________________________________________________
Bool_t AliEmcalIsolatedPhotonsTask::AcceptTrack(AliVTrack* /*track*/)
{
  return kTRUE;
}

//________________________________________________________________________
void AliEmcalIsolatedPhotonsTask::UserExec(Option_t *) 
{
  // Main loop, called for each event.
  // Add jets to event if not yet there

  RetrieveEventObjects();

  FillHistograms();
    
  // information for this iteration of the UserExec in the container
  PostData(1, fOutput);
}

//________________________________________________________________________
void AliEmcalIsolatedPhotonsTask::Terminate(Option_t *) 
{
  // Called once at the end of the analysis.
}
