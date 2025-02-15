#include "AtClusterizeTask.h"

// Fair class header
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "AtTpcPoint.h"
#include "AtVertexPropagator.h"
#include "AtSimulatedPoint.h"

// STL class headers
#include <cmath>
#include <iostream>
#include <iomanip>

#include "TRandom.h"
#include "TMath.h"
#include "TF1.h"

AtClusterizeTask::AtClusterizeTask() : FairTask("AtClusterizeTask"), fEventID(0), fIsPersistent(kFALSE) {}

AtClusterizeTask::~AtClusterizeTask()
{
   LOG(debug) << "Destructor of AtClusterizeTask";
}

void AtClusterizeTask::SetParContainers()
{
   LOG(debug) << "SetParContainers of AtAvalancheTask";

   FairRunAna *ana = FairRunAna::Instance();
   FairRuntimeDb *rtdb = ana->GetRuntimeDb();
   fPar = (AtDigiPar *)rtdb->getContainer("AtDigiPar");
}

InitStatus AtClusterizeTask::Init()
{
   LOG(debug) << "Initilization of AtClusterizeTask";

   FairRootManager *ioman = FairRootManager::Instance();

   fMCPointArray = (TClonesArray *)ioman->GetObject("AtTpcPoint");
   if (fMCPointArray == 0) {
      LOG(error) << "Cannot find fMCPointArray array!";
      return kERROR;
   }

   fElectronNumberArray = new TClonesArray("AtSimulatedPoint");
   ioman->Register("AtSimulatedPoint", "cbmsim", fElectronNumberArray, fIsPersistent);

   fEIonize = fPar->GetEIonize() / 1000000; // [MeV]
   fFano = fPar->GetFano();
   fVelDrift = fPar->GetDriftVelocity();   // [cm/us]
   fCoefT = fPar->GetCoefDiffusionTrans(); // [cm^2/us]
   fCoefL = fPar->GetCoefDiffusionLong();  // [cm^2/us]
   fDetPadPlane = fPar->GetZPadPlane();    //[mm]

   std::cout << "  Ionization energy of gas: " << fEIonize << " MeV" << std::endl;
   std::cout << "  Fano factor of gas: " << fFano << std::endl;
   std::cout << "  Drift velocity: " << fVelDrift << std::endl;
   std::cout << "  Longitudal coefficient of diffusion: " << fCoefT << std::endl;
   std::cout << "  Transverse coefficient of diffusion: " << fCoefL << std::endl;
   std::cout << "  Position of the pad plane (Z): " << fDetPadPlane << std::endl;

   return kSUCCESS;
}

void AtClusterizeTask::Exec(Option_t *option)
{
   LOG(debug) << "Exec of AtClusterizeTask";
   Int_t nMCPoints = fMCPointArray->GetEntries();
   // std::cout<<"AtClusterizeTask: Number of MC Points "<<nMCPoints<<std::endl;
   /*if(nMCPoints<10){
     LOG(warning) << "Not enough hits for digitization! (<10)";
     return;
   }*///Commented to process all events

   /**
    * NOTE! that fMCPoint has unit of [cm] for length scale,
    * [GeV] for energy and [ns] for time.
    */
   fElectronNumberArray->Delete();

   // Double_t  energyLoss_sca=0.0;
   Double_t energyLoss_rec = 0.0;
   Double_t x = 0;
   Double_t y = 0;
   Double_t z = 0;
   Int_t nElectrons = 0;
   Int_t eFlux = 0;
   Int_t genElectrons = 0;
   // Double_t  eIonize      = 15.603/1000; //Ionization energy (MeV)
   TString VolName;
   Double_t tTime; //, entries;

   // Double_t zMesh          = 1000; //mm (No tilt)
   // Double_t coefDiffusion  = 0.01; //from AtMCQMinimization.cc
   // Double_t driftVelocity  = 2; //cm/us for hydrogen
   // Double_t coefT          = 0.010;
   // Double_t coefL          = 0.025;
   Double_t driftLength;
   Double_t driftTime;
   Double_t propX;
   Double_t propY;
   // Double_t sigmaDiffusion;
   Double_t sigstrtrans, sigstrlong, phi, r;
   Int_t electronNumber = 0;
   // TF1 *trans      = new TF1("trans", "x*pow(2.718,(-pow(x,2))/[0])", 0, 1);

   Double_t x_post = 0;
   Double_t y_post = 0;
   Double_t z_post = 0;
   Double_t x_pre = 0;
   Double_t y_pre = 0;
   Double_t z_pre = 0;
   Double_t stepX, stepY, stepZ;

   Int_t presentTrackID = -10; // control of the point trackID
   Bool_t readyToProject = kFALSE;

   for (Int_t i = 0; i < nMCPoints; i++) {

      fMCPoint = (AtTpcPoint *)fMCPointArray->At(i);

      VolName = fMCPoint->GetVolName();
      Int_t trackID = fMCPoint->GetTrackID();

      if (VolName == "drift_volume") {

         // THAt WOULD BE THE CODE IF THE ENERGY IS DISTRIBUTED FROM present Point position to next Point position
         // THAt IS MOST PROBABLY NOT CORRECT
         /*
      if (i+1>nMCPoints){ //all but the last MCPoint
      AtTpcPoint* nextPoint = (AtTpcPoint*) fMCPointArray-> At(i+1);
      if((nextPoint->GetTrackID()) == (fMCPoint->GetTrackID())){ //next MCPoint of same track
      x_post  = nextPoint->GetXIn()*10; //mm
      y_post  = nextPoint->GetYIn()*10; //mm
      z_post  = 1000-(nextPoint->GetZIn()*10); //mm
      }
      else{ // all MCPoints from TrackExiting, TrackStop and TrackDisappeared should have a GetXOut, ...
      x_post  = fMCPoint->GetXOut()*10; //mm
      y_post  = fMCPoint->GetYOut()*10; //mm
      z_post  = 1000-(fMCPoint->GetZOut()*10); //mm
      }
      }
      else{ //the last MCPoint should be also TrackExiting, TrackStop or TrackDisappeared
      x_post  = fMCPoint->GetXOut()*10; //mm
      y_post  = fMCPoint->GetYOut()*10; //mm
      z_post  = 1000-(fMCPoint->GetZOut()*10); //mm
      }
         */

         // THAt WOULD BE THE CODE IF THE ENERGY IS DISTRIBUTED FROM present Point position to previous Point position
         // THAt IS MOST PROBABLY  CORRECT
         if (fMCPoint->GetEnergyLoss() == 0) {                // a new track is entering the volume or created
            x_pre = fMCPoint->GetXIn() * 10;                  // mm
            y_pre = fMCPoint->GetYIn() * 10;                  // mm
            z_pre = fDetPadPlane - (fMCPoint->GetZIn() * 10); // mm
            presentTrackID = fMCPoint->GetTrackID();
            continue; // no energy deposited in this point, just taking in entrance coordinates
         }
         if (presentTrackID != fMCPoint->GetTrackID()) {      // new track  (but energy not zero!)
            x_pre = fMCPoint->GetXIn() * 10;                  // mm
            y_pre = fMCPoint->GetYIn() * 10;                  // mm
            z_pre = fDetPadPlane - (fMCPoint->GetZIn() * 10); // mm
            presentTrackID = fMCPoint->GetTrackID();
            std::cout << "Note in NEW DIGI: Energy not zero in first point for a track!" << std::endl;
            continue; // first point of a new track, just taking in entrance coordinates
         }

         tTime = fMCPoint->GetTime() / 1000;           // us
         x = fMCPoint->GetXIn() * 10;                  // mm
         y = fMCPoint->GetYIn() * 10;                  // mm
         z = fDetPadPlane - (fMCPoint->GetZIn() * 10); // mm

         // if ENERGY IS DISTRIBUTED FROM present Point position to next Point position
         // std::cout << "Init point: " << x << " " << y << " " << z << std::endl;
         // std::cout << "Final point: " << x_post << " "<< y_post << " " << z_post << std::endl;
         // if ENERGY IS DISTRIBUTED FROM present Point position to previous Point position
         // std::cout << "Init point: " << x_pre << " " << y_pre << " " << z_pre << std::endl;
         // std::cout << "Final point: " << x << " "<< y << " " << z << " " << fMCPoint->GetTrackID() <<std::endl;

         // std::cout<<" tTime : "<<tTime<<" - x : "<<x<<" - y : "<<y<<" - z : "<<z<<" Time "<<tTime<<"\n";
         energyLoss_rec = (fMCPoint->GetEnergyLoss()) * 1000; // MeV
         // std::cout<<" Energy Loss "<<energyLoss_rec<<" fEIonize"<<fEIonize<<" Track ID "<<presentTrackID<<"\n";
         nElectrons = energyLoss_rec / fEIonize;          // mean electrons generated
         eFlux = pow(fFano * nElectrons, 0.5);            // fluctuation of generated electrons
         genElectrons = gRandom->Gaus(nElectrons, eFlux); // generated electrons
         // std::cout<<" nElectrons "<<nElectrons<<" Gen Electrons "<<genElectrons<<"\n";

         // step in each direction for an homogeneous electron creation position along the track
         // stepX = (x_post-x) / genElectrons;
         // stepY = (y_post-y) / genElectrons;
         // stepZ = (z_post-z) / genElectrons;
         if (genElectrons > 0) {
            stepX = (x - x_pre) / genElectrons;
            stepY = (y - y_pre) / genElectrons;
            stepZ = (z - z_pre) / genElectrons;
         }

         driftLength = abs(z);                                            // mm  //coarse value of the driftLength
         sigstrtrans = sqrt(10.0 * fCoefT * 2 * driftLength / fVelDrift); // transverse diffusion coefficient in mm
         sigstrlong = sqrt(10.0 * fCoefL * 2 * driftLength / fVelDrift);  // longitudal diffusion coefficient in mm
         // trans->SetParameter(0, sigstrtrans);

         for (Int_t charge = 0; charge < genElectrons; charge++) { // for every electron in the cluster
            // r             = trans->GetRandom(); //non-Gaussian cloud
            driftLength = abs(z_pre + stepZ * charge); // fine value of the driftLength
            r = gRandom->Gaus(0, sigstrtrans);         // Gaussian cloud
            phi = gRandom->Uniform(0, TMath::TwoPi());
            propX = x_pre + stepX * charge + r * TMath::Cos(phi);
            propY = y_pre + stepY * charge + r * TMath::Sin(phi);
            driftLength = driftLength + (gRandom->Gaus(0, sigstrlong)); // mm
            driftTime = ((driftLength / 10) / fVelDrift);               // us
            // NB: tTime in the simulation is 0 for the first simulation point
            // std::cout<<i<<"  "<<charge<<"  "<<" Drift velocity "<<fVelDrift
            // <<" driftTime : "<<driftTime<<" tTime "<<tTime<<"\n";
            // std::cout<<" Position of electron "<<charge<<" : "<<propX<<" : "<<propY<<"\n";
            electronNumber += 1;

            // Fill container AtSimulatedPoint
            Int_t size = fElectronNumberArray->GetEntriesFast();
            AtSimulatedPoint *simpoint =
               new ((*fElectronNumberArray)[size]) AtSimulatedPoint(i, electronNumber, // electron #
                                                                    propX,             // X
                                                                    propY,             // Y
                                                                    driftTime);        // Z
            simpoint->SetMCEventID(fMCPoint->GetEventID());
         } // end producing e- and filling AtSimpoint

         x_pre = x;
         y_pre = y;
         z_pre = z;

      } // end if drift volume

   } // end through all interaction points

   return;
}

ClassImp(AtClusterizeTask);
