/*******************************************************************
 * Daughter class for Linear Hough Space transformation             *
 * Log: Class started 28-04-2015                                    *
 * Author: Y. Ayyad (NSCL ayyadlim@nscl.msu.edu)                    *
 ********************************************************************/

#ifndef AtHOUGHSPACELINE_H
#define AtHOUGHSPACELINE_H

#include "AtHoughSpace.h"
#include "TH2F.h"
#include "TF1.h"
#include "TGraph2D.h"
#include <Math/Vector3D.h>
#include <Math/Functor.h>
#include <Math/Minimizer.h>
#include <TPolyLine3D.h>

// FairRoot classes
#include "FairRootManager.h"
#include "FairLogger.h"

// AtTPCROOT
#include "AtTrack.h"

class AtHit;

class AtHoughSpaceLine : public AtHoughSpace {

public:
   AtHoughSpaceLine();
   ~AtHoughSpaceLine();

   template <class GenHough, typename ProjPlane>
   std::vector<AtTrack *> CalcGenHoughSpace(GenHough event, ProjPlane plane);
   TH2F *GetHoughSpace(TString ProjPlane);
   void CalcHoughSpace(AtEvent *event, Bool_t YZplane, Bool_t XYplane, Bool_t XZplane);
   void CalcHoughSpace(AtEvent *event, TH2Poly *hPadPlane);
   void CalcHoughSpace(AtProtoEvent *protoevent, Bool_t q1, Bool_t q2, Bool_t q3, Bool_t q4);
   void CalcHoughSpace(AtEvent *event, TH2Poly *hPadPlane, const multiarray &PadCoord);
   void CalcMultiHoughSpace(AtEvent *event);
   void CalcHoughSpace(AtEvent *event);

   // TH2F* GetHoughQuadrant(Int_t index);
   std::pair<Double_t, Double_t> GetHoughParameters(TH2F *hist);
   std::vector<std::pair<Double_t, Double_t>> GetMultiHoughParameters(TH2F *hist);
   std::pair<Double_t, Double_t> GetHoughParameters(); // Overloaded version for std::map
   std::vector<std::pair<Double_t, Double_t>> GetHoughPar(TString opt = "Hist");
   std::vector<Double_t> GetHoughMax();
   TVector3 GetVertex1();
   TVector3 GetVertex2();
   Double_t GetMinimum(); // Distance of minumum approach between candidate lines (for the moment only 2)
   std::vector<AtTrack> GetTrackCand();

   void FillHoughMap(Double_t ang, Double_t dist);
   void SetRadiusThreshold(Float_t value);
   void SetLine(double t, const double *p, double &x, double &y, double &z);
   void FindVertex(std::vector<AtTrack *> HoughTracks);
   void SetTrackCand(AtTrack *track);
   // void SetTrackCandArray(std::vector<AtTrack> *TrackArray);

   Int_t MinimizeTrack(AtTrack *track);
   static double distance2(double x, double y, double z, const double *p);

   struct SumDistance2 {
      TGraph2D *fGraph;

      SumDistance2(TGraph2D *g) : fGraph(g) {}
      double operator()(const double *par)
      {
         assert(fGraph != 0);
         double *x = fGraph->GetX();
         double *y = fGraph->GetY();
         double *z = fGraph->GetZ();
         int npoints = fGraph->GetN();
         double sum = 0;
         for (int i = 0; i < npoints; ++i) {
            double d = distance2(x[i], y[i], z[i], par);
            sum += d;
         }
         /*#ifdef DEBUG
                  if (first)
                     std::cout << "point " << i << "\t" << x[i] << "\t" << y[i] << "\t" << z[i] << "\t" << std::sqrt(d)
                               << std::endl;
                     #end*/

         // if (first)
         // std::cout << "Total Initial distance square = " << sum << std::endl;
         // first = false;
         return sum;
      }
   };

protected:
   Int_t fThreshold;
   Int_t fRadThreshold;
   Double_t fDriftVelocity;
   Int_t fTBTime;
   Int_t fEntTB;
   Float_t fZk;
   Int_t fXbinRZ;
   Int_t fYbinRZ;
   std::vector<AtTrack *> fHoughTracks; //!
   // NB: This member wont be saved in to the root file. If the //! is uncommented the program will crash because the
   // pointers are destroyed after being used
   TVector3 fVertex_1;
   TVector3 fVertex_2;
   Double_t fMinimum;
   std::vector<AtTrack> fTrackCand; // Candidate tracks (debugging purposes)
   Int_t fLineDistThreshold;

   std::map<std::vector<Float_t>, Int_t> HoughMap_XZ;
   TH2F *HistHoughXZ;
   TH2F *HistHoughRZ;
   // TH2F *HistHoughRZ[4]; //One per quadrant
   std::vector<std::pair<Double_t, Double_t>> HoughPar;
   std::vector<std::pair<Double_t, Double_t>> HoughParSTD;
   std::map<ULong64_t, Int_t> HoughMap; // 8 byte for the key, unsigned, no negative distance in Linear Hough space is
                                        // expected for each quadrant (Radius and Z are the vairbales)
   std::vector<ULong64_t> HoughMapKey;
   std::vector<Double_t> HoughMax;

   Bool_t CheckTrackID(Int_t trackID, std::vector<AtTrack> trackArray); // Check if Track ID is in the list

   struct maxpersecond {
      template <typename Lhs, typename Rhs>
      bool operator()(const Lhs &lhs, const Rhs &rhs) const
      {
         return lhs.second < rhs.second;
      }
   };

   ClassDef(AtHoughSpaceLine, 1);
};

// Template that works for any At container with Hits inside. It works on the X-Z projection for the time being
#include "AtHit.h"
template <class GenHough, typename ProjPlane>
std::vector<AtTrack *> AtHoughSpaceLine::CalcGenHoughSpace(GenHough event, ProjPlane plane)
{

   Int_t nHits = event->GetNumHits();
   Double_t drift_cal = fDriftVelocity * fTBTime / 100.0; // mm
   // TH2F *rad_z = new TH2F("rad_z","rad_z",1000,-500,500,1000,0,1000);
   // TH2F *rad_z2 = new TH2F("rad_z2","rad_z2",1000,-500,500,1000,0,1000);
   // rad_z2->SetMarkerColor(kRed);
   // rad_z->SetMarkerSize(2.0);
   std::vector<AtTrack *> HoughTracks;

   std::vector<AtHit *> HitBuffer;
   std::vector<AtHit *> HitBuffer2;

   // Generic Hough Space coordinates
   Double_t coordX;
   Double_t coordY;

   for (Int_t iHit = 0; iHit < nHits; iHit++) {
      AtHit *hit = event->GetHit(iHit);
      Int_t PadNumHit = hit->GetHitPadNum();
      TVector3 position = hit->GetPosition();

      coordY = hit->GetTimeStamp();
      if (plane.EqualTo("XZ"))
         coordX = position.X();
      else if (plane.EqualTo("YZ"))
         coordX = position.Y();

      Float_t radius = TMath::Sqrt(TMath::Power(position.X(), 2) + TMath::Power(position.Y(), 2));

      Int_t itheta = 0;
      for (itheta = 0; itheta < 1023; itheta++) {
         Float_t angle = TMath::Pi() * (static_cast<Float_t>(itheta) / 1023);
         Float_t d0_RZ = (TMath::Cos(angle) * coordX) +
                         (TMath::Sin(angle) * coordY); // posZCal can be replaced anytime by position.Z()
         HistHoughRZ->Fill(angle, d0_RZ);

      } // Hough Angle loop
   }

   std::pair<Double_t, Double_t> HoughParBuff = GetHoughParameters(HistHoughRZ);
   AtTrack *track = new AtTrack();

   for (Int_t iHit = 0; iHit < nHits; iHit++) {
      AtHit *hit = event->GetHit(iHit);
      Int_t PadNumHit = hit->GetHitPadNum();
      TVector3 position = hit->GetPosition();

      coordY = hit->GetTimeStamp();
      if (plane.EqualTo("XZ"))
         coordX = position.X();
      else if (plane.EqualTo("YZ"))
         coordX = position.Y();

      Float_t radius = TMath::Sqrt(TMath::Power(position.X(), 2) + TMath::Power(position.Y(), 2));
      Double_t geo_dist = TMath::Abs(TMath::Cos(HoughParBuff.first) * coordX + TMath::Sin(HoughParBuff.first) * coordY -
                                     HoughParBuff.second);
      // rad_z2->Fill(hit->GetTimeStamp(),position.X());
      if (geo_dist > fHoughDist) {
         HitBuffer.push_back(hit);
         // rad_z->Fill(hit->GetTimeStamp(),position.X());
      } else {
         track->AddHit(hit);
      }
   }

   if (HoughMax.at(0) > fHoughMaxThreshold) {
      HoughTracks.push_back(track);
      HoughPar.push_back(HoughParBuff);
      // MinimizeTrack(track);
   }

   // delete track;

   // rad_z2->Draw();
   // rad_z->Draw("SAMES");

   //******************************************** Recursive Hough Space calculation ***********************************

   // std::cout<<HitBuffer.size()<<std::endl;

   for (Int_t i = 0; i < 5; i++) { // NB: Only 5 lines allowed right now (TEve Draw Taks must be changed if this number
                                   // changes otherwise we will have a pitfall)

      track = new AtTrack();
      HistHoughRZ->Reset();
      // rad_z->Reset();

      for (Int_t iHit = 0; iHit < HitBuffer.size(); iHit++) {
         AtHit *hit = HitBuffer.at(iHit);
         Int_t PadNumHit = hit->GetHitPadNum();
         TVector3 position = hit->GetPosition();

         coordY = hit->GetTimeStamp();
         if (plane.EqualTo("XZ"))
            coordX = position.X();
         else if (plane.EqualTo("YZ"))
            coordX = position.Y();

         Float_t radius = TMath::Sqrt(TMath::Power(position.X(), 2) + TMath::Power(position.Y(), 2));

         Int_t itheta = 0;
         //#pragma omp parallel for ordered schedule(dynamic) private(itheta) //TOOD: Check performance
         for (itheta = 0; itheta < 1023; itheta++) {
            Float_t angle = TMath::Pi() * (static_cast<Float_t>(itheta) / 1023);
            Float_t d0_RZ = (TMath::Cos(angle) * coordX) +
                            (TMath::Sin(angle) * coordY); // posZCal can be replaced anytime by position.Z()
            //#pragma omp ordered
            HistHoughRZ->Fill(angle, d0_RZ);
            // rad_z->Fill(hit->GetTimeStamp(),radius);

         } // Hough Angle loop
      }

      HoughParBuff = GetHoughParameters(HistHoughRZ);

      // std::cout<<"  Iterative Hough Space  "<<i<<std::endl;
      // std::cout<<"  Hough Parameter Angle : "<<HoughPar.first<<std::endl;
      // std::cout<<"  Hough Parameter Distance : "<<HoughPar.second<<std::endl;

      // fHoughLinearFit->SetParameter(0,TMath::Pi()/2.0-HoughPar.first);
      // fHoughLinearFit->SetParameter(1,HoughPar.second);

      for (Int_t iHit = 0; iHit < HitBuffer.size(); iHit++) {
         AtHit *hit = HitBuffer.at(iHit);
         Int_t PadNumHit = hit->GetHitPadNum();
         TVector3 position = hit->GetPosition();

         coordY = hit->GetTimeStamp();
         if (plane.EqualTo("XZ"))
            coordX = position.X();
         else if (plane.EqualTo("YZ"))
            coordX = position.Y();

         Float_t radius = TMath::Sqrt(TMath::Power(position.X(), 2) + TMath::Power(position.Y(), 2));
         Double_t geo_dist = TMath::Abs(TMath::Cos(HoughParBuff.first) * coordX +
                                        TMath::Sin(HoughParBuff.first) * coordY - HoughParBuff.second);
         if (geo_dist > fHoughDist) {
            // std::cout<<geo_dist<<" Time Bucket : "<<hit->GetTimeStamp()<<std::endl;
            HitBuffer2.push_back(hit);
            // rad_z->Fill(hit->GetTimeStamp(),radius);
         } else
            track->AddHit(hit);
      }

      if (HoughMax.at(i + 1) > fHoughMaxThreshold) {
         HoughTracks.push_back(track);
         HoughPar.push_back(HoughParBuff);
         // MinimizeTrack(track);
      }

      HitBuffer.clear();
      HitBuffer = HitBuffer2;
      HitBuffer2.clear();

      // delete track;
   }

   // rad_z->Draw();
   // fHoughLinearFit->Draw("SAME");

   for (Int_t iHit = 0; iHit < nHits; iHit++) {
      AtHit *hit = event->GetHit(iHit);
      Int_t PadNumHit = hit->GetHitPadNum();
      TVector3 position = hit->GetPosition();

      coordY = hit->GetTimeStamp();
      if (plane.EqualTo("XZ"))
         coordX = position.X();
      else if (plane.EqualTo("YZ"))
         coordX = position.Y();

      Float_t radius = TMath::Sqrt(TMath::Power(position.X(), 2) + TMath::Power(position.Y(), 2));

      Int_t itheta = 0;

      for (itheta = 0; itheta < 1023; itheta++) {
         Float_t angle = TMath::Pi() * (static_cast<Float_t>(itheta) / 1023);
         Float_t d0_RZ = (TMath::Cos(angle) * coordX) +
                         (TMath::Sin(angle) * coordY); // posZCal can be replaced anytime by position.Z()
         //#pragma omp ordered
         HistHoughRZ->Fill(angle, d0_RZ);

      } // Hough Angle loop
      //}
   }

   return HoughTracks;
}

#endif
