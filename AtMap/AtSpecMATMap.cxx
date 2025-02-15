#include "AtSpecMATMap.h"

#include <cassert>
#include <iostream>
#include "FairLogger.h"

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

AtSpecMATMap::AtSpecMATMap(Int_t numPads)
{
   AtPadCoord.resize(boost::extents[numPads][4][2]);
   kIsParsed = 0;
   kGUIMode = 0;
   kDebug = 0;
   std::fill(AtPadCoord.data(), AtPadCoord.data() + AtPadCoord.num_elements(), 0);
   std::cout << " SpecMAT Map initialized " << std::endl;
   std::cout << " SpecMAT Pad Coordinates container initialized " << std::endl;
   fPadInd = 0;
   fIniPads.clear();
   hPlane = new TH2Poly();
}

AtSpecMATMap::~AtSpecMATMap() {}

void AtSpecMATMap::Dump() {}

std::vector<double> TrianglesGenerator()
{
   std::vector<double> allCoordinates;
   int nRaws = 23;
   int firstElement = 1;
   int elementStep = 2;
   int nElementsInSegment = nRaws * ((2 * firstElement + (nRaws - 1) * elementStep) / 2);
   int nSegments = 6;
   int step = 8;
   int counter = 0;
   int a = 0;
   const int arrayLength = nElementsInSegment * nSegments;
   double x0[arrayLength], x1[arrayLength], x2[arrayLength], xCentre[arrayLength], x0next[arrayLength],
      x1next[arrayLength], x2next[arrayLength];
   double y0[arrayLength], y1[arrayLength], y2[arrayLength], yCentre[arrayLength], y0next[arrayLength],
      y1next[arrayLength], y2next[arrayLength];
   double h = 4.725;

   for (int i = 1; i < nRaws + 1; i++) {
      a = 1 + (i - 1) * 2;
      for (int j = 1; j < a + 1; j++) {
         if (i == 1) {
            x0[counter] = 0.;
            y0[counter] = 0.;

            x1[counter] = -h / sqrt(3);
            y1[counter] = h;

            x2[counter] = h / sqrt(3);
            y2[counter] = h;

            xCentre[counter] = 0.;
            yCentre[counter] = h / sqrt(3);
         } else {
            if (j % 2 == 0) {
               x0[counter] = x2[counter - 1];
               y0[counter] = y2[counter - 1];

               x1[counter] = x2[counter - a + 1];
               y1[counter] = y2[counter - a + 1];

               x2[counter] = x1[counter - a + 1];
               y2[counter] = y1[counter - a + 1];

               xCentre[counter] = x0[counter];
               yCentre[counter] = y0[counter] - h / sqrt(3);
            } else {
               x0[counter] = x0next[i - 2] + (j - 1) * (h / sqrt(3));
               y0[counter] = y0next[i - 2];

               x1[counter] = x1next[i - 2] + (j - 1) * (h / sqrt(3));
               y1[counter] = y1next[i - 2];

               x2[counter] = x2next[i - 2] + (j - 1) * (h / sqrt(3));
               y2[counter] = y2next[i - 2];

               xCentre[counter] = x0[counter];
               yCentre[counter] = y0[counter] + h / sqrt(3);
            }
         }
         counter += 1;
      }

      x0next[i - 1] = x0[counter - a] - h / sqrt(3);
      y0next[i - 1] = y0[counter - a] + h;

      x1next[i - 1] = x1[counter - a] - h / sqrt(3);
      y1next[i - 1] = y1[counter - a] + h;

      x2next[i - 1] = x2[counter - a] - h / sqrt(3);
      y2next[i - 1] = y2[counter - a] + h;
   }

   for (int i = 1; i < nSegments; i++) {
      for (int j = 0; j < nElementsInSegment; j++) {
         x0[i * nElementsInSegment + j] =
            0.5 * (x0[(i - 1) * nElementsInSegment + j] - 0) - sqrt(3) * 0.5 * (y0[(i - 1) * nElementsInSegment + j]);
         y0[i * nElementsInSegment + j] =
            sqrt(3) * 0.5 * (x0[(i - 1) * nElementsInSegment + j] - 0) + 0.5 * (y0[(i - 1) * nElementsInSegment + j]);

         x1[i * nElementsInSegment + j] =
            0.5 * (x1[(i - 1) * nElementsInSegment + j] - 0) - sqrt(3) * 0.5 * (y1[(i - 1) * nElementsInSegment + j]);
         y1[i * nElementsInSegment + j] =
            sqrt(3) * 0.5 * (x1[(i - 1) * nElementsInSegment + j] - 0) + 0.5 * (y1[(i - 1) * nElementsInSegment + j]);

         x2[i * nElementsInSegment + j] =
            0.5 * (x2[(i - 1) * nElementsInSegment + j] - 0) - sqrt(3) * 0.5 * (y2[(i - 1) * nElementsInSegment + j]);
         y2[i * nElementsInSegment + j] =
            sqrt(3) * 0.5 * (x2[(i - 1) * nElementsInSegment + j] - 0) + 0.5 * (y2[(i - 1) * nElementsInSegment + j]);

         xCentre[i * nElementsInSegment + j] = 0.5 * (xCentre[(i - 1) * nElementsInSegment + j] - 0) -
                                               sqrt(3) * 0.5 * (yCentre[(i - 1) * nElementsInSegment + j]);
         yCentre[i * nElementsInSegment + j] = sqrt(3) * 0.5 * (xCentre[(i - 1) * nElementsInSegment + j] - 0) +
                                               0.5 * (yCentre[(i - 1) * nElementsInSegment + j]);
      }
   }

   for (int i = 0; i < arrayLength; i++) {
      allCoordinates.push_back(x0[i]);
      allCoordinates.push_back(y0[i]);
      allCoordinates.push_back(x1[i]);
      allCoordinates.push_back(y1[i]);
      allCoordinates.push_back(x2[i]);
      allCoordinates.push_back(y2[i]);
      allCoordinates.push_back(xCentre[i]);
      allCoordinates.push_back(yCentre[i]);
   }
   return allCoordinates;
} // End TrianglesGenerator()

// TH2Poly *SpecMATHistoGenerator()
TH2Poly *AtSpecMATMap::GetAtTpcPlane()
{
   TH2Poly *SpecMAThisto = new TH2Poly();
   std::vector<double> arrayAllCoordinates = TrianglesGenerator();
   double x[3];
   double y[3];
   int arrayAllCoordinatesSize = arrayAllCoordinates.size();
   for (int i2 = 0; i2 < arrayAllCoordinatesSize / 8; i2++) {
      if (i2 != 0 && i2 != arrayAllCoordinatesSize * 1 / (6 * 8) && i2 != arrayAllCoordinatesSize * 2 / (6 * 8) &&
          i2 != arrayAllCoordinatesSize * 3 / (6 * 8) && i2 != arrayAllCoordinatesSize * 4 / (6 * 8) &&
          i2 != arrayAllCoordinatesSize * 5 / (6 * 8)) {
         x[0] = arrayAllCoordinates[8 * i2];
         y[0] = arrayAllCoordinates[8 * i2 + 1];
         x[1] = arrayAllCoordinates[8 * i2 + 2];
         y[1] = arrayAllCoordinates[8 * i2 + 3];
         x[2] = arrayAllCoordinates[8 * i2 + 4];
         y[2] = arrayAllCoordinates[8 * i2 + 5];
         SpecMAThisto->AddBin(3, x, y);
      }
   }
   return SpecMAThisto;
} // SpecMATHistoGenerator()

void AtSpecMATMap::SpecMATPadPlane()
{

   std::vector<double> arrayAllCoordinates2;
   arrayAllCoordinates2 = TrianglesGenerator();

   int nElementsInSegment3;
   nElementsInSegment3 = arrayAllCoordinates2.size() / (6 * 8);

   TH2Poly *XY;
   XY = GetAtTpcPlane();
   // XY = SpecMATHistoGenerator();
   XY->SetTitle("PadPlane X vs Y");
   XY->SetName("XY");
   XY->GetXaxis()->SetTitle("X (mm)");
   XY->GetYaxis()->SetTitle("Y (mm)");

   int currentBin;
   double InitialPoInt_X = 50;
   double InitialPoInt_Y = 50;
   double particleEnergy = 50;

   double binXcentroid;
   double binYcentroid;

   // Histogram filling example
   XY->Fill(InitialPoInt_X, InitialPoInt_Y, particleEnergy);
   XY->Draw("COLZ");

   // Obtain coordinates of the filled pad
   currentBin = XY->FindBin(InitialPoInt_X, InitialPoInt_Y, 0);

   if (currentBin < nElementsInSegment3) {
      binXcentroid = arrayAllCoordinates2[8 * (currentBin) + 6];
      binYcentroid = arrayAllCoordinates2[8 * (currentBin) + 7];
   } else if (currentBin >= nElementsInSegment3 && currentBin < nElementsInSegment3 * 2 - 1) {
      binXcentroid = arrayAllCoordinates2[8 * (currentBin + 1) + 6];
      binYcentroid = arrayAllCoordinates2[8 * (currentBin + 1) + 7];
   } else if (currentBin >= nElementsInSegment3 * 2 - 1 && currentBin < nElementsInSegment3 * 3 - 2) {
      binXcentroid = arrayAllCoordinates2[8 * (currentBin + 2) + 6];
      binYcentroid = arrayAllCoordinates2[8 * (currentBin + 2) + 7];
   } else if (currentBin >= nElementsInSegment3 * 3 - 2 && currentBin < nElementsInSegment3 * 4 - 3) {
      binXcentroid = arrayAllCoordinates2[8 * (currentBin + 3) + 6];
      binYcentroid = arrayAllCoordinates2[8 * (currentBin + 3) + 7];
   } else if (currentBin >= nElementsInSegment3 * 4 - 3 && currentBin < nElementsInSegment3 * 5 - 4) {
      binXcentroid = arrayAllCoordinates2[8 * (currentBin + 4) + 6];
      binYcentroid = arrayAllCoordinates2[8 * (currentBin + 4) + 7];
   } else if (currentBin >= nElementsInSegment3 * 5 - 4 && currentBin < nElementsInSegment3 * 6 - 5) {
      binXcentroid = arrayAllCoordinates2[8 * (currentBin + 5) + 6];
      binYcentroid = arrayAllCoordinates2[8 * (currentBin + 5) + 7];
   }

   std::cout << "Input" << std::endl;
   std::cout << "X: " << InitialPoInt_X << " Y: " << InitialPoInt_Y << std::endl;
   std::cout << "\nOutput" << std::endl;
   std::cout << "Bin: " << currentBin << " binCentroidX: " << binXcentroid << " binCentroidY: " << binYcentroid
             << std::endl;
} // SpecMATPadPlane

void AtSpecMATMap::GenerateAtTpc() {}

std::vector<Float_t> AtSpecMATMap::CalcPadCenter(Int_t PadRef)
{
   LOG(error) << "CalcPadCenter function for SpecMAT is not implemented!";
   return std::vector<Float_t>();
}

ClassImp(AtSpecMATMap)
