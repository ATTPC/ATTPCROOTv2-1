#include "ATProtoEventAna.hh"

ClassImp(ATProtoEventAna);

ATProtoEventAna::ATProtoEventAna()
:TNamed("ATProtoEventAna", "Proto Event Analysis")
{

}

ATProtoEventAna::~ATProtoEventAna()
{

}


std::vector<Double_t>* ATProtoEventAna::GetAngleFit()                                                          {return &fAngle_fit;}
std::vector<Double_t>* ATProtoEventAna::GetAngle()                                                             {return &fAngle;}
std::vector<Double_t>* ATProtoEventAna::GetPar0()                                                              {return &fPar0_fit;}
std::vector<Double_t>* ATProtoEventAna::GetPar1()                                                              {return &fPar1_fit;}
std::vector<Double_t>* ATProtoEventAna::GetRange()                                                             {return &fRange;}
std::vector<std::pair<Double_t,Double_t>>* ATProtoEventAna::GetHoughPar()                                      {return &fHoughPar;}
std::vector<std::pair<Double_t,Double_t>>* ATProtoEventAna::GetELossHitPattern()                               {return &fELossHitPattern;}
std::vector<std::vector<std::pair<Double_t,Double_t>>>* ATProtoEventAna::GetQELossHitPattern()                 {return &fQELossHitPattern;}

//Setters
void ATProtoEventAna::SetAngleFit(std::vector<Double_t> Angle_fit)                                             {fAngle_fit = Angle_fit;}
void ATProtoEventAna::SetAngle(std::vector<Double_t> Angle)                                                    {fAngle = Angle;}
void ATProtoEventAna::SetPar0(std::vector<Double_t> Par0_fit)                                                  {fPar0_fit = Par0_fit;}
void ATProtoEventAna::SetPar1(std::vector<Double_t> Par1_fit)                                                  {fPar1_fit = Par1_fit;}
void ATProtoEventAna::SetRange(std::vector<Double_t> Range)                                                    {fRange = Range;}
void ATProtoEventAna::SetHoughPar(std::vector<std::pair<Double_t,Double_t>> HoughPar )                         {fHoughPar = HoughPar;}
void ATProtoEventAna::SetELHitPattern(std::vector<std::pair<Double_t,Double_t>> ELHitPattern )                 {fELossHitPattern = ELHitPattern; }
void ATProtoEventAna::SetQELHitPattern(std::vector<std::vector<std::pair<Double_t,Double_t>>> QELHitPattern )  {fQELossHitPattern = QELHitPattern; }
