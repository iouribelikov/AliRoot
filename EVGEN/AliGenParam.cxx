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

/* $Id$ */

// Class to generate particles from using paramtrized pT and y distributions.
// Distributions are obtained from pointer to object of type AliGenLib.
// (For example AliGenMUONlib)
// Decays are performed using Pythia.
// andreas.morsch@cern.ch

#include <TCanvas.h>
#include <TClonesArray.h>
#include <TDatabasePDG.h>
#include <TF1.h>
#include <TH1F.h>
#include <TLorentzVector.h>
#include <TMath.h>
#include <TParticle.h>
#include <TParticlePDG.h>
#include <TROOT.h>
#include <TVirtualMC.h>

#include "AliDecayer.h"
#include "AliGenMUONlib.h"
#include "AliGenParam.h"
#include "AliMC.h"
#include "AliRun.h"
#include "AliGenEventHeader.h"

ClassImp(AliGenParam)

//------------------------------------------------------------

//Begin_Html
/*
    <img src="picts/AliGenParam.gif">
*/
//End_Html

//____________________________________________________________
AliGenParam::AliGenParam()
: fPtParaFunc(0),
	fYParaFunc(0),
	fIpParaFunc(0),
  fV2ParaFunc(0),
	fPtPara(0),
	fYPara(0),
  fV2Para(0),
  fdNdPhi(0),
	fParam(0),
	fdNdy0(0.),
	fYWgt(0.),
	fPtWgt(0.),
	fBias(0.),
	fTrials(0),
	fDeltaPt(0.01),
	fSelectAll(kFALSE),
  fDecayer(0),
  fForceConv(kFALSE)
{
  // Default constructor
}
//____________________________________________________________
AliGenParam::AliGenParam(Int_t npart, const AliGenLib * Library,  Int_t param, const char* tname)
    :AliGenMC(npart),
     fPtParaFunc(Library->GetPt(param, tname)),
     fYParaFunc (Library->GetY (param, tname)),
     fIpParaFunc(Library->GetIp(param, tname)),
   fV2ParaFunc(Library->GetV2(param, tname)),
     fPtPara(0),
     fYPara(0),
   fV2Para(0),
   fdNdPhi(0),
     fParam(param),
     fdNdy0(0.),
     fYWgt(0.),
     fPtWgt(0.),
     fBias(0.),
     fTrials(0),
     fDeltaPt(0.01),
     fSelectAll(kFALSE),
   fDecayer(0),
   fForceConv(kFALSE)
{
  // Constructor using number of particles parameterisation id and library
    fName = "Param";
    fTitle= "Particle Generator using pT and y parameterisation";
    fAnalog = kAnalog;
    SetForceDecay();
}
//____________________________________________________________
AliGenParam::AliGenParam(Int_t npart, Int_t param, const char* tname, const char* name):
    AliGenMC(npart),
    fPtParaFunc(0),
    fYParaFunc (0),
    fIpParaFunc(0),
  fV2ParaFunc(0),
    fPtPara(0),
    fYPara(0),
  fV2Para(0),
  fdNdPhi(0),
    fParam(param),
    fdNdy0(0.),
    fYWgt(0.),
    fPtWgt(0.),
    fBias(0.),
    fTrials(0),
    fDeltaPt(0.01),
    fSelectAll(kFALSE),
  fDecayer(0),
  fForceConv(kFALSE)
{
  // Constructor using parameterisation id and number of particles
  //
    fName = name;
    fTitle= "Particle Generator using pT and y parameterisation";
      
    AliGenLib* pLibrary = new AliGenMUONlib();
    fPtParaFunc = pLibrary->GetPt(param, tname);
    fYParaFunc  = pLibrary->GetY (param, tname);
    fIpParaFunc = pLibrary->GetIp(param, tname);
  fV2ParaFunc = pLibrary->GetV2(param, tname);
    
    fAnalog = kAnalog;
    fChildSelect.Set(5);
    for (Int_t i=0; i<5; i++) fChildSelect[i]=0;
    SetForceDecay();
    SetCutOnChild();
    SetChildMomentumRange();
    SetChildPtRange();
    SetChildPhiRange();
    SetChildThetaRange(); 
}
//____________________________________________________________

AliGenParam::AliGenParam(Int_t npart, Int_t param,
                         Double_t (*PtPara) (const Double_t*, const Double_t*),
                         Double_t (*YPara ) (const Double_t* ,const Double_t*),
                         Double_t (*V2Para) (const Double_t* ,const Double_t*),
		         Int_t    (*IpPara) (TRandom *))		 
    :AliGenMC(npart),
     
     fPtParaFunc(PtPara),
     fYParaFunc(YPara),
     fIpParaFunc(IpPara),
   fV2ParaFunc(V2Para),
     fPtPara(0),
     fYPara(0),
   fV2Para(0),
   fdNdPhi(0),
     fParam(param),
     fdNdy0(0.),
     fYWgt(0.),
     fPtWgt(0.),
     fBias(0.),
     fTrials(0),
     fDeltaPt(0.01),
     fSelectAll(kFALSE),
  fDecayer(0),
  fForceConv(kFALSE)
{
  // Constructor
  // Gines Martinez 1/10/99 
    fName = "Param";
    fTitle= "Particle Generator using pT and y parameterisation";

    fAnalog = kAnalog;
    fChildSelect.Set(5);
    for (Int_t i=0; i<5; i++) fChildSelect[i]=0;
    SetForceDecay();
    SetCutOnChild();
    SetChildMomentumRange();
    SetChildPtRange();
    SetChildPhiRange();
    SetChildThetaRange();  
}

//____________________________________________________________
AliGenParam::~AliGenParam()
{
  // Destructor
    delete  fPtPara;
    delete  fYPara;
  delete  fV2Para;
  delete  fdNdPhi;
}

//-------------------------------------------------------------------
TVector3 AliGenParam::OrthogonalVector(TVector3 &inVec){
  double abc[]={inVec.x(), inVec.y(), inVec.z()}; 
  double xyz[]={1,1,1};
  int solvDim=0;
  double tmp=abc[0];
  for(int i=0; i<3; i++)
    if(abs(abc[i])>tmp){
      solvDim=i;
      tmp=abs(abc[i]);
    }
  xyz[solvDim]=(-abc[(1+solvDim)%3]-abc[(2+solvDim)%3])/abc[(0+solvDim)%3];
  
  TVector3 res(xyz[0],xyz[1],xyz[2]);
  return res;
}

double AliGenParam::ScreenFunc1(double d){
  if(d>1)
    return 21.12-4.184*log(d+0.952);
  else
    return 20.867-3.242*d+0.652*d*d;
}

double AliGenParam::ScreenFunc2(double d){
  if(d>1)
    return 21.12-4.184*log(d+0.952);
  else
    return 20.209-1.93*d-0.086*d*d;
}

double AliGenParam::EnergyFraction(double Z, double E){
  double e0=0.000511/E;
  double aZ=Z/137.036;
  double dmin=ScreenVar(Z,e0,0.5);
  double fcZ=(aZ*aZ)*(1/(1+aZ*aZ)+0.20206-0.0368*aZ*aZ+0.0083*aZ*aZ*aZ);
  double Fz=8*log(Z)/3;
  if(E>0.05)
    Fz+=8*fcZ;
  double dmax=exp((42.24-Fz)/8.368)-0.952;
  if(!(dmax>dmin))
    return fRandom->Uniform(e0,0.5);

  double e1=0.5-0.5*sqrt(1-dmin/dmax);
  double emin=TMath::Max(e0,e1);
  
  double normval=1/(0.5*(ScreenFunc1(dmin)-0.5*Fz)+0.1666667*(ScreenFunc2(dmin)-0.5*Fz));
  while(true){
    double y=fRandom->Uniform(emin,1);
    double eps=y*(0.38453+y*(0.10234+y*(0.026072+y*(0.014367-0.027313*y)))); //inverse to the enveloping cumulative probability distribution
    double val=fRandom->Uniform(0,2.12*(eps*eps-eps)+1.53); //enveloping probability density
    double d=ScreenVar(Z,e0,eps);
    double bh=((eps*eps)+(1-eps)*(1-eps))*(ScreenFunc1(d)-0.5*Fz)+0.6666667*eps*(1-eps)*(ScreenFunc2(d)-0.5*Fz);
    bh*=normval;
    if(val<bh)
      return eps;
  }
}

double AliGenParam::PolarAngle(double E){
  float rand[3];
  AliRndm rndm;
  rndm.Rndm(rand,3);
  double u=-8*log(rand[1]*rand[2])/5;
  if(!(rand[0]<9.0/36))
    u/=3;
  return u*0.000511/E;
}

Int_t AliGenParam::ForceGammaConversion(TClonesArray *particles, Int_t nPart)
{
  //based on: http://geant4.cern.ch/G4UsersDocuments/UsersGuides/PhysicsReferenceManual/html/node27.html
  //     and: http://geant4.cern.ch/G4UsersDocuments/UsersGuides/PhysicsReferenceManual/html/node58.html
  Int_t nPartNew=nPart;
  for(int iPart=0; iPart<nPart; iPart++){      
    TParticle *gamma = (TParticle *) particles->At(iPart);
    if(gamma->GetPdgCode()!=22) continue;

    TVector3 gammaV3(gamma->Px(),gamma->Py(),gamma->Pz());
    Float_t az=fRandom->Uniform(TMath::Pi()*2);
    double frac=EnergyFraction(1,gamma->Energy());
    double Ee1=frac*gamma->Energy();
    double Ee2=(1-frac)*gamma->Energy();
    double Pe1=Ee1;//sqrt(Ee1*Ee1-0.000511*0.000511);
    double Pe2=Ee2;//sqrt(Ee2*Ee2-0.000511*0.000511);
    
    TVector3 rotAxis(OrthogonalVector(gammaV3));
    rotAxis.Rotate(az,gammaV3);
    TVector3 e1V3(gammaV3);
    e1V3.Rotate(PolarAngle(Ee1),rotAxis);
    e1V3=e1V3.Unit();
    e1V3*=Pe1;
    TVector3 e2V3(gammaV3);
    e2V3.Rotate(-PolarAngle(Ee2),rotAxis);
    e2V3=e2V3.Unit();
    e2V3*=Pe2;
    // gamma = new TParticle(*gamma);
    // particles->RemoveAt(iPart);
    gamma->SetFirstDaughter(nPartNew+1);
    gamma->SetLastDaughter(nPartNew+2);
    // new((*particles)[iPart]) TParticle(*gamma);
    // delete gamma;

    TLorentzVector vtx;
    gamma->ProductionVertex(vtx);
    new((*particles)[nPartNew]) TParticle(11, gamma->GetStatusCode(), iPart+1, -1, 0, 0, TLorentzVector(e1V3,Ee1), vtx);
    nPartNew++;
    new((*particles)[nPartNew]) TParticle(-11, gamma->GetStatusCode(), iPart+1, -1, 0, 0, TLorentzVector(e2V3,Ee2), vtx);
    nPartNew++;
  }
  // particles->Compress();
  return particles->GetEntriesFast();
}

//____________________________________________________________
void AliGenParam::Init()
{
  // Initialisation

    if (gMC) fDecayer = gMC->GetDecayer();
  //Begin_Html
  /*
    <img src="picts/AliGenParam.gif">
  */
  //End_Html
    char name[256];
    snprintf(name, 256, "pt-parameterisation for %s", GetName());
    
    if (fPtPara) fPtPara->Delete();
    fPtPara = new TF1(name, fPtParaFunc, fPtMin, fPtMax,0);
    gROOT->GetListOfFunctions()->Remove(fPtPara);
  //  Set representation precision to 10 MeV
    Int_t npx= Int_t((fPtMax - fPtMin) / fDeltaPt);
    
    fPtPara->SetNpx(npx);

    snprintf(name, 256, "y-parameterisation  for %s", GetName());
    if (fYPara) fYPara->Delete();
    fYPara  = new TF1(name, fYParaFunc, fYMin, fYMax, 0);
    gROOT->GetListOfFunctions()->Remove(fYPara);

  snprintf(name, 256, "v2-parameterisation for %s", GetName());
  if (fV2Para) fV2Para->Delete();
  fV2Para  = new TF1(name, fV2ParaFunc, fPtMin, fPtMax, 0);
  // fV2Para  = new TF1(name, "2*[0]/(1+TMath::Exp([1]*([2]-x)))-[0]", fPtMin, fPtMax);
  // fV2Para->SetParameter(0, 0.236910);
  // fV2Para->SetParameter(1, 1.71122);
  // fV2Para->SetParameter(2, 0.0827617);
  //gROOT->GetListOfFunctions()->Remove(fV2Para);  //TR: necessary?
    
  snprintf(name, 256, "dNdPhi for %s", GetName());
  if (fdNdPhi) fdNdPhi->Delete();
  fdNdPhi  = new TF1(name, "1+2*[0]*TMath::Cos(2*(x-[1]))", fPhiMin, fPhiMax);
  //gROOT->GetListOfFunctions()->Remove(fdNdPhi);  //TR: necessary?
    
    snprintf(name, 256, "pt-for-%s", GetName());
    TF1 ptPara(name ,fPtParaFunc, 0, 15, 0);
    snprintf(name, 256, "y-for-%s", GetName());
    TF1 yPara(name, fYParaFunc, -6, 6, 0);

  //
  // dN/dy| y=0
    Double_t y1=0;
    Double_t y2=0;
    
    fdNdy0=fYParaFunc(&y1,&y2);
  //
  // Integral over generation region
#if ROOT_VERSION_CODE < ROOT_VERSION(5,99,0)
    Float_t intYS  = yPara.Integral(fYMin, fYMax,(Double_t*) 0x0,1.e-6);
    Float_t intPt0 = ptPara.Integral(0,15,(Double_t *) 0x0,1.e-6);
    Float_t intPtS = ptPara.Integral(fPtMin,fPtMax,(Double_t*) 0x0,1.e-6);
#else
    Float_t intYS  = yPara.Integral(fYMin, fYMax,1.e-6);
    Float_t intPt0 = ptPara.Integral(0,15,1.e-6);
    Float_t intPtS = ptPara.Integral(fPtMin,fPtMax,1.e-6);
#endif
  Float_t phiWgt=(fPhiMax-fPhiMin)/2./TMath::Pi();    //TR: should probably be done differently in case of anisotropic phi...
    if (fAnalog == kAnalog) {
	fYWgt  = intYS/fdNdy0;
	fPtWgt = intPtS/intPt0;
	fParentWeight = fYWgt*fPtWgt*phiWgt/fNpart;
    } else {
	fYWgt = intYS/fdNdy0;
	fPtWgt = (fPtMax-fPtMin)/intPt0;
	fParentWeight = fYWgt*fPtWgt*phiWgt/fNpart;
    }
  //
  // particle decay related initialization
    fDecayer->SetForceDecay(fForceDecay);
    fDecayer->Init();

  //
    AliGenMC::Init();
}

//____________________________________________________________
void AliGenParam::Generate()
{
  // 
  // Generate 1 event (see Generate(Int_t ntimes) for details
  //
  GenerateN(1);
}
//____________________________________________________________
void AliGenParam::GenerateN(Int_t ntimes)
{
  //
  // Generate ntimes*'npart' light and heavy mesons (J/Psi, upsilon or phi, Pion,
  // Kaons, Etas, Omegas) and Baryons (proton, antiprotons, neutrons and 
  // antineutrons in the the desired theta, phi and momentum windows; 
  // Gaussian smearing on the vertex is done if selected. 
  // The decay of heavy mesons is done using lujet, 
  //    and the childern particle are tracked by GEANT
  // However, light mesons are directly tracked by GEANT 
  // setting fForceDecay = nodecay (SetForceDecay(nodecay)) 
  //
  //
  //  Reinitialize decayer
  fDecayer->SetForceDecay(fForceDecay);
  fDecayer->Init();

  //
  Float_t polar[3]= {0,0,0};  // Polarisation of the parent particle (for GEANT tracking)
  Float_t origin0[3];         // Origin of the generated parent particle (for GEANT tracking)
  Float_t time0;              // Time0 of the generated parent particle
  Float_t pt, pl, ptot;       // Transverse, logitudinal and total momenta of the parent particle
  Float_t phi, theta;         // Phi and theta spherical angles of the parent particle momentum
  Float_t p[3], pc[3], 
          och[3];             // Momentum, polarisation and origin of the children particles from lujet
  Double_t ty, xmt;
  Int_t nt, i, j;
  Float_t  wgtp, wgtch;
  Double_t dummy;
  static TClonesArray *particles;
  //
  if(!particles) particles = new TClonesArray("TParticle",1000);
  
  TDatabasePDG *pDataBase = TDatabasePDG::Instance();
  //
  Float_t random[6];
 
  // Calculating vertex position per event
  for (j=0;j<3;j++) origin0[j]=fOrigin[j];
  time0 = fTimeOrigin;
  if(fVertexSmear==kPerEvent) {
      Vertex();
      for (j=0;j<3;j++) origin0[j]=fVertex[j];
      time0 = fTime;
  }
  
  Int_t ipa=0;
  
  // Generating fNpart particles
  fNprimaries = 0;
  
  Int_t nGen = fNpart*ntimes;
  while (ipa<nGen) {
      while(1) {
      //
      // particle type 
	  Int_t iPart = fIpParaFunc(fRandom);
	  fChildWeight=(fDecayer->GetPartialBranchingRatio(iPart))*fParentWeight;	   
	  TParticlePDG *particle = pDataBase->GetParticle(iPart);
	  Float_t am = particle->Mass();
	  
	  Rndm(random,2);
      //
      // y
	  ty = TMath::TanH(fYPara->GetRandom());
      //
      // pT
	  if (fAnalog == kAnalog) {
	      pt=fPtPara->GetRandom();
	      wgtp=fParentWeight;
	      wgtch=fChildWeight;
	  } else {
	      pt=fPtMin+random[1]*(fPtMax-fPtMin);
	      Double_t ptd=pt;
	      wgtp=fParentWeight*fPtParaFunc(& ptd, &dummy);
	      wgtch=fChildWeight*fPtParaFunc(& ptd, &dummy);
	  }
	  xmt=sqrt(pt*pt+am*am);
	  if (TMath::Abs(ty)==1.) {
	      ty=0.;
	      Fatal("AliGenParam", 
		    "Division by 0: Please check you rapidity range !");
	  }
      //
      // phi
	  //      if(!ipa)
	  //phi=fEvPlane; //align first particle of each event with event plane
	  //else{
	double v2 = fV2Para->Eval(pt);
	fdNdPhi->SetParameter(0,v2);
	fdNdPhi->SetParameter(1,fEvPlane);
	phi=fdNdPhi->GetRandom();
	//     }
	  
	  pl=xmt*ty/sqrt((1.-ty)*(1.+ty));
	  theta=TMath::ATan2(pt,pl);
      // Cut on theta
	  if(theta<fThetaMin || theta>fThetaMax) continue;
	  ptot=TMath::Sqrt(pt*pt+pl*pl);
      // Cut on momentum
	  if(ptot<fPMin || ptot>fPMax) continue;
      //
	  p[0]=pt*TMath::Cos(phi);
	  p[1]=pt*TMath::Sin(phi);
	  p[2]=pl;
	  if(fVertexSmear==kPerTrack) {
	      Rndm(random,6);
	      for (j=0;j<3;j++) {
		  origin0[j]=
		      fOrigin[j]+fOsigma[j]*TMath::Cos(2*random[2*j]*TMath::Pi())*
		      TMath::Sqrt(-2*TMath::Log(random[2*j+1]));
	      }
	      Rndm(random,2);
	      time0 = fTimeOrigin + fOsigma[2]/TMath::Ccgs()*
		TMath::Cos(2*random[0]*TMath::Pi())*
		TMath::Sqrt(-2*TMath::Log(random[1]));
	  }
	  
      // Looking at fForceDecay : 
      // if fForceDecay != none Primary particle decays using 
      // AliPythia and children are tracked by GEANT
      //
      // if fForceDecay == none Primary particle is tracked by GEANT 
      // (In the latest, make sure that GEANT actually does all the decays you want)	  
      //
	  Bool_t decayed = kFALSE;
	  

	  if (fForceDecay != kNoDecay) {
	// Using lujet to decay particle
	      Float_t energy=TMath::Sqrt(ptot*ptot+am*am);
	      TLorentzVector pmom(p[0], p[1], p[2], energy);
	      fDecayer->Decay(iPart,&pmom);
	//
	// select decay particles
	      Int_t np=fDecayer->ImportParticles(particles);

	if(fForceConv) np=ForceGammaConversion(particles,np);

	// for(int iPart=0; iPart<np; iPart++){
	//   TParticle *gamma = (TParticle *) particles->At(iPart);
	//   printf("%i %i:", iPart, gamma->GetPdgCode());
	//   printf("%i %i %i|",gamma->GetFirstMother(),gamma->GetFirstDaughter(),gamma->GetLastDaughter());
	// }

	      //  Selecting  GeometryAcceptance for particles fPdgCodeParticleforAcceptanceCut;
	      if (fGeometryAcceptance) 
		if (!CheckAcceptanceGeometry(np,particles)) continue;
	      Int_t ncsel=0;
	      Int_t* pFlag      = new Int_t[np];
	      Int_t* pParent    = new Int_t[np];
	      Int_t* pSelected  = new Int_t[np];
	      Int_t* trackIt    = new Int_t[np];

	      for (i=0; i<np; i++) {
		  pFlag[i]     =  0;
		  pSelected[i] =  0;
		  pParent[i]   = -1;
	      }
	      
	      if (np >1) {
		  decayed = kTRUE;
		  TParticle* iparticle =  0;
		  Int_t ipF, ipL;
		  for (i = 1; i<np ; i++) {
		      trackIt[i] = 1;
		      iparticle = (TParticle *) particles->At(i);
		      Int_t kf = iparticle->GetPdgCode();
		      Int_t ks = iparticle->GetStatusCode();
	    // flagged particle

		      if (pFlag[i] == 1) {
			  ipF = iparticle->GetFirstDaughter();
			  ipL = iparticle->GetLastDaughter();	
			  if (ipF > 0) for (j=ipF-1; j<ipL; j++) pFlag[j]=1;
			  continue;
		      }

	    // flag decay products of particles with long life-time (c tau > .3 mum)		      
		      
		      if (ks != 1) { 
	      //			  TParticlePDG *particle = pDataBase->GetParticle(kf);
			  
			  Double_t lifeTime = fDecayer->GetLifetime(kf);
	      //			  Double_t mass     = particle->Mass();
	      //			  Double_t width    = particle->Width();
			  if (lifeTime > (Double_t) fMaxLifeTime) {
			      ipF = iparticle->GetFirstDaughter();
			      ipL = iparticle->GetLastDaughter();	
			      if (ipF > 0) for (j=ipF-1; j<ipL; j++) pFlag[j]=1;
			  } else{
			      trackIt[i]     = 0;
			      pSelected[i]   = 1;
			  }
		      } // ks==1 ?
	    //
	    // children
		      
		      if ((ChildSelected(TMath::Abs(kf)) || fForceDecay == kAll || fSelectAll) && trackIt[i])
		      {
			  if (fCutOnChild) {
			      pc[0]=iparticle->Px();
			      pc[1]=iparticle->Py();
			      pc[2]=iparticle->Pz();
			      Bool_t  childok = KinematicSelection(iparticle, 1);
			      if(childok) {
				  pSelected[i]  = 1;
				  ncsel++;
			      } else {
				  ncsel=-1;
				  break;
			      } // child kine cuts
			  } else {
			      pSelected[i]  = 1;
			      ncsel++;
			  } // if child selection
		      } // select muon
		  } // decay particle loop
	      } // if decay products
	      
	      Int_t iparent;
	      if ((fCutOnChild && ncsel >0) || !fCutOnChild){
		  ipa++;
	  //
	  // Parent
		  
		  
		  PushTrack(0, -1, iPart, p, origin0, polar, time0, kPPrimary, nt, wgtp, ((decayed)? 11 : 1));
		  pParent[0] = nt;
		  KeepTrack(nt); 
		  fNprimaries++;
		  
	  //
	  // Decay Products
	  //		  
		  for (i = 1; i < np; i++) {
		      if (pSelected[i]) {
			  TParticle* iparticle = (TParticle *) particles->At(i);
			  Int_t kf   = iparticle->GetPdgCode();
			  Int_t ksc  = iparticle->GetStatusCode();
			  Int_t jpa  = iparticle->GetFirstMother()-1;
			  
			  och[0] = origin0[0]+iparticle->Vx();
			  och[1] = origin0[1]+iparticle->Vy();
			  och[2] = origin0[2]+iparticle->Vz();
			  pc[0]  = iparticle->Px();
			  pc[1]  = iparticle->Py();
			  pc[2]  = iparticle->Pz();
			  
			  if (jpa > -1) {
			      iparent = pParent[jpa];
			  } else {
			      iparent = -1;
			  }
			 
			  PushTrack(fTrackIt * trackIt[i], iparent, kf,
					   pc, och, polar,
				           time0 + iparticle->T(), kPDecay, nt, wgtch, ksc);
			  pParent[i] = nt;
			  KeepTrack(nt); 
			  fNprimaries++;
		      } // Selected
		  } // Particle loop 
	      }  // Decays by Lujet
	      particles->Clear();
	      if (pFlag)      delete[] pFlag;
	      if (pParent)    delete[] pParent;
	      if (pSelected)  delete[] pSelected;	   
	      if (trackIt)    delete[] trackIt;
	  } // kinematic selection
	  else  // nodecay option, so parent will be tracked by GEANT (pions, kaons, eta, omegas, baryons)
	  {
	    gAlice->GetMCApp()->
		PushTrack(fTrackIt,-1,iPart,p,origin0,polar,time0,kPPrimary,nt,wgtp, 1);
            ipa++; 
	    fNprimaries++;
	  }
	  break;
    } // while
  } // event loop
  
  SetHighWaterMark(nt);

  AliGenEventHeader* header = new AliGenEventHeader("PARAM");
  header->SetPrimaryVertex(fVertex);
  header->SetInteractionTime(fTime);
  header->SetNProduced(fNprimaries);
  AddHeader(header);
}
//____________________________________________________________________________________
Float_t AliGenParam::GetRelativeArea(Float_t ptMin, Float_t ptMax, Float_t yMin, Float_t yMax, Float_t phiMin, Float_t phiMax)
{
  //
  // Normalisation for selected kinematic region
  //
#if ROOT_VERSION_CODE < ROOT_VERSION(5,99,0)
  Float_t ratio =  
    fPtPara->Integral(ptMin,ptMax,(Double_t *)0,1.e-6) / fPtPara->Integral( fPtPara->GetXmin(), fPtPara->GetXmax(),(Double_t *)0,1.e-6) *
    fYPara->Integral(yMin,yMax,(Double_t *)0,1.e-6)/fYPara->Integral(fYPara->GetXmin(),fYPara->GetXmax(),(Double_t *)0,1.e-6)   *
    (phiMax-phiMin)/360.;
#else
  Float_t ratio =  
    fPtPara->Integral(ptMin,ptMax,1.e-6) / fPtPara->Integral( fPtPara->GetXmin(), fPtPara->GetXmax(),1.e-6) *
    fYPara->Integral(yMin,yMax,1.e-6)/fYPara->Integral(fYPara->GetXmin(),fYPara->GetXmax(),1.e-6)   *
    (phiMax-phiMin)/360.;
#endif
  return TMath::Abs(ratio);
}

//____________________________________________________________________________________

void AliGenParam::Draw( const char * /*opt*/)
{
    //
    // Draw the pT and y Distributions
    //
     TCanvas *c0 = new TCanvas("c0","Canvas 0",400,10,600,700);
     c0->Divide(2,1);
     c0->cd(1);
     fPtPara->Draw();
     fPtPara->GetHistogram()->SetXTitle("p_{T} (GeV)");     
     c0->cd(2);
     fYPara->Draw();
     fYPara->GetHistogram()->SetXTitle("y");     
}




