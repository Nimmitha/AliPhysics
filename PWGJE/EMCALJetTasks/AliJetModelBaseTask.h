#ifndef ALIJETMODELBASETASK_H
#define ALIJETMODELBASETASK_H

// $Id$

class TClonesArray;
class AliEMCALGeometry;
class AliVCluster;
class AliPicoTrack;
class AliVCaloCells;
class AliAODMCParticle;
class AliNamedArrayI;
class TF2;
class AliStackPartonInfo;

#include <TH1F.h>
#include <TF1.h>

#include "AliAnalysisTaskSE.h"

class AliJetModelBaseTask : public AliAnalysisTaskSE {
 public:
  AliJetModelBaseTask();
  AliJetModelBaseTask(const char *name, Bool_t drawqa=kFALSE); 
  virtual ~AliJetModelBaseTask();

  void                   SetEtaRange(Float_t min, Float_t max) { fEtaMin       = min;  fEtaMax = max; }
  void                   SetPhiRange(Float_t min, Float_t max) { fPhiMin       = min;  fPhiMax = max; }
  void                   SetPtRange(Float_t min, Float_t max)  { fPtMin        = min;  fPtMax  = max; }
  void                   SetPtSpectrum(TH1F *f)                { fPtSpectrum   = f;    }
  void                   SetPtSpectrum(TF1 *f)                 { fPtSpectrum   = new TH1F("ptSpectrum","ptSpectrum",1000,f->GetXmin(),f->GetXmax()); 
                                                                 fPtSpectrum->Add(f); }
  void                   SetPtPhiEvPlDistribution(TF2 *f)      { fPtPhiEvPlDistribution   = f;    }
  void                   SetDensitySpectrum(TH1F *f)           { fDensitySpectrum = f;    }
  void                   SetDensitySpectrum(TF1 *f)            { fDensitySpectrum = new TH1F("densitypectrum","densitypectrum",1000,f->GetXmin(),f->GetXmax()); 
                                                                 fDensitySpectrum->Add(f); }
  void                   SetDifferentialV2(TF1* f)             { fDifferentialV2 = f;  }
  void                   SetAddV2(Bool_t b)                    { fAddV2 = b;           }
  void                   SetAddFlowFluctuations(Bool_t b)      { fFlowFluctuations = b;}
  void                   SetMC(Bool_t a)                       { fIsMC         = a   ; }
  void                   SetCopyArray(Bool_t copy)             { fCopyArray    = copy; }
  void                   SetTracksName(const char *n)          { fTracksName   = n;    }
  void                   SetClusName(const char *n)            { fCaloName     = n;    }
  void                   SetCellsName(const char *n)           { fCellsName    = n;    }
  void                   SetMCParticlesName(const char *n)     { fMCParticlesName = n; }
    void                 SetPartonInfoName(const char *n)      { fPartonInfoName = n; }
  void                   SetSuffix(const char *s)              { fSuffix       = s;    }
  void                   SetGeometryName(const char *n)        { fGeomName     = n;    }
  void                   SetMarkMC(Int_t m)                    { fMarkMC       = m;    }
  virtual void           SetNClusters(Int_t n)                 { fNClusters    = n;    }
  virtual void           SetNCells(Int_t n)                    { fNCells       = n;    }
  virtual void           SetNTracks(Int_t n)                   { fNTracks      = n;    }

 protected:
  void                   UserExec(Option_t* /*option*/);
  void                   UserCreateOutputObjects();
  Int_t                  SetNumberOfOutCells(Int_t n);                                          // set the number of cells
  Int_t                  AddCell(Double_t e = -1, Double_t eta = -999, Double_t phi = -1);      // add a cell; if values are -1 generate random parameters
  Int_t                  AddCell(Double_t e, Int_t absId, Double_t time = 0, Int_t label=0);    // add a cell with given energy, position and times
  AliVCluster           *AddCluster(Double_t e = -1, Double_t eta = -999, Double_t phi = -1, Int_t label=0); // add a cluster; if values are -1 generate random parameters
  AliVCluster           *AddCluster(Double_t e, Int_t absId, Int_t label=0);                    // add a cluster with given energy and position
  AliVCluster           *AddCluster(AliVCluster *oc);                                           // add a cluster (copy)
  AliPicoTrack          *AddTrack(Double_t pt = -1, Double_t eta = -999, Double_t phi = -1, Byte_t type=0, 
				  Double_t etaemc=0, Double_t phiemc=0, Double_t ptemc=0, Bool_t ise=kFALSE, 
				  Int_t label=0, Short_t charge=1, Double_t mass = 0.1396);          // add a track; if values are -1 generate random parameters
  AliAODMCParticle      *AddMCParticle(AliAODMCParticle *part, Int_t origIndex);                // add a MC particle
  void                   AddV2(Double_t &phi, Double_t &pt) const;
  void                   CopyCells();
  void                   CopyClusters();
  void                   CopyTracks();
  void                   CopyMCParticles();
  void                   GetRandomCell(Double_t &eta, Double_t &phi, Int_t &absId);             // generate a random cell in the calorimeter
  Double_t               GetRandomEta(Bool_t emcal=kFALSE);                                     // generate a random eta value in the given range
  Double_t               GetRandomPhi(Bool_t emcal=kFALSE);                                     // generate a random phi value in the given range
  Double_t               GetRandomPt();                                                         // generate a random pt value in the given range
  void                   GetRandomParticle(Double_t &pt, Double_t &eta, Double_t &phi, Bool_t emcal=kFALSE);  // generate a particle with random eta,phi,pt values
  virtual Bool_t         ExecOnce();                                                            // intialize task
  virtual void           Run();                                                                 // do jet model action

  TString                fGeomName;               // EMCal geometry name
  TString                fTracksName;             // name of track collection
  TString                fOutTracksName;          // name of output track collection
  TString                fCaloName;               // name of calo cluster collection
  TString                fOutCaloName;            // name of output cluster collection
  TString                fCellsName;              // name of calo cells collection
  TString                fOutCellsName;           // name of output cells collection
  TString                fMCParticlesName;        // name of MC particle collection
  TString                fOutMCParticlesName;     // name of output MC particle collection
  TString                fPartonInfoName;         // name of partons info
  Bool_t                 fIsMC;                   // whether the current event is MC or not
  TString                fSuffix;                 // suffix to add in the name of new collections
  Float_t                fEtaMin;                 // eta minimum value
  Float_t                fEtaMax;                 // eta maximum value
  Float_t                fPhiMin;                 // phi minimum value
  Float_t                fPhiMax;                 // phi maximum value
  Float_t                fPtMin;                  // pt minimum value
  Float_t                fPtMax;                  // pt maximum value
  Bool_t                 fCopyArray;              // whether or not the array will be copied to a new one before modelling
  Int_t                  fNClusters;              // how many clusters are being processed
  Int_t                  fNCells;                 // how many cells are being processed
  Int_t                  fNTracks;                // how many tracks are being processed
  Int_t                  fMarkMC;                 // which MC label is to be used (default=100)
  TH1F                  *fPtSpectrum;             // pt spectrum to extract random pt values
  TF2                   *fPtPhiEvPlDistribution;  // pt vs. (phi-psi) distribution to extract random pt/phi values
  TH1F                  *fDensitySpectrum;        // particle density spectrum to extract random density values
  TF1                   *fDifferentialV2;         // v2 as function of pt
  Bool_t                 fAddV2;                  // add v2 sampled from a tf1
  Bool_t                 fFlowFluctuations;       // introduce gaussian flow fluctuation 
  Bool_t                 fQAhistos;               // draw QA histograms
  Double_t               fPsi;                    //!simmetry plane for the elliptic flow
  Bool_t                 fIsInit;                 //!=true if initialized
  AliEMCALGeometry      *fGeom;                   //!pointer to EMCal geometry
  Double_t               fVertex[3];              //!event vertex
  TClonesArray          *fClusters;               //!cluster collection
  TClonesArray          *fOutClusters;            //!output cluster collection
  TClonesArray          *fTracks;                 //!track collection
  TClonesArray          *fOutTracks;              //!output track collection
  AliVCaloCells         *fCaloCells;              //!cells collection
  AliVCaloCells         *fOutCaloCells;           //!output cells collection
  Int_t                  fAddedCells;             //!number of added cells
  TClonesArray          *fMCParticles;            //!MC particles collection
  AliNamedArrayI        *fMCParticlesMap;         //!MC particles mapping
  TClonesArray          *fOutMCParticles;         //!output MC particles collection
  AliNamedArrayI        *fOutMCParticlesMap;      //!MC particles mapping
  Int_t                  fMCLabelShift;           //!MC label shift
  Bool_t                 fEsdMode;                //!ESD/AOD mode
  TList                 *fOutput;                 //!output list for QA histograms
  AliStackPartonInfo    *fStackPartonInfo;     //!Info on original partons:PDG,pt, eta, phi 

 private:
  AliJetModelBaseTask(const AliJetModelBaseTask&);            // not implemented
  AliJetModelBaseTask &operator=(const AliJetModelBaseTask&); // not implemented

  ClassDef(AliJetModelBaseTask, 12) // Jet modelling task
};
#endif
