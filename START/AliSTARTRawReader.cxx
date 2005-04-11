#include "AliSTARTRawReader.h"
#include "AliSTARTRawData.h"
#include "AliSTARTdigit.h"

#include <Riostream.h>
#include "TMath.h"
#include "TH1F.h"
#include "TArrayI.h"
#include "AliLog.h"
 
ClassImp(AliSTARTRawReader)
  
  AliSTARTRawReader::AliSTARTRawReader (AliRawReader *rawReader, TTree* tree)
    :  TTask("STARTRawReader","read raw START data"),
       fTree(tree),
       fRawReader(rawReader)
{
  //
// create an object to read STARTraw digits
  AliDebug(1,"Start ");

 
}
 AliSTARTRawReader::~AliSTARTRawReader ()
{
  // 
}
//------------------------------------------------------------------------------------------------

UInt_t AliSTARTRawReader::UnpackWord(UInt_t packedWord, Int_t startBit, Int_t stopBit)
{
  //This method unpacks a words of StopBit-StartBit+1 bits starting from "StopBits"  
  UInt_t word;
  UInt_t offSet;
  Int_t length;
  length=stopBit-startBit+1;
  offSet=(UInt_t)TMath::Power(2,length)-1;
  offSet<<=startBit;
  word=packedWord&offSet;
  word>>=startBit;
  return word;
}
//---------------------------------------------------------------------------------------
void AliSTARTRawReader::NextThing()
{
// read the next raw digit
// returns kFALSE if there is no digit left
  UInt_t word, unpackword; 
  Int_t time, adc, pmt;
  TArrayI *timeTDC1 = new TArrayI(24);
  TArrayI * chargeTDC1 = new TArrayI(24);
  TArrayI *timeTDC2 = new TArrayI(24);
  TArrayI *chargeTDC2 = new TArrayI(24);
  TArrayI *sumMult = new TArrayI(6);

  fRawReader->Select(13);
 
  if (!fRawReader->ReadHeader()){
    Error("ReadSTARTRaw","Couldn't read header");
    return;
  }
  AliSTARTdigit *fDigits = new AliSTARTdigit(); 
  fTree->Branch("START","AliSTARTdigit",&fDigits,400,1);

  fRawReader->ReadNextInt(word);
   for (Int_t i=0; i<24; i++)
    {
      word=0;
      unpackword=0;
      fRawReader->ReadNextInt(word);
      unpackword=UnpackWord(word,0,5);
      pmt=unpackword; 
      word=0;
      unpackword=0;
      fRawReader->ReadNextInt(word);
      unpackword=UnpackWord(word,8,31);
      time=unpackword;
      timeTDC1->AddAt(time,pmt);

      word=0;
      unpackword=0;
      fRawReader->ReadNextInt(word);
      unpackword=UnpackWord(word,0,5);
      pmt=unpackword;
      word=0;
      unpackword=0;
      fRawReader->ReadNextInt(word);
      unpackword=UnpackWord(word,8,31);
      time=unpackword;
      timeTDC2->AddAt(time,pmt);
 
      //  QTC
      word=0;
      unpackword=0;
      fRawReader->ReadNextInt(word);
      unpackword=UnpackWord(word,0,5);
      pmt=unpackword;
      word=0;
      unpackword=0;
      fRawReader->ReadNextInt(word);
      unpackword= UnpackWord(word,8,31);
      time=unpackword; //T1
  
      word=0;
      unpackword=0;

      fRawReader->ReadNextInt(word);
      unpackword=UnpackWord(word,0,5);
      pmt=unpackword;
      word=0;
      unpackword=0;
      fRawReader->ReadNextInt(word);
      unpackword= UnpackWord(word,8,31);
      adc=unpackword-time;  // T1+ T2 (A)
      chargeTDC1->AddAt(adc,pmt);
      //QTC amplified

      word=0;
      unpackword=0;
      fRawReader->ReadNextInt(word);
      unpackword=UnpackWord(word,0,5);
      pmt=unpackword;
      word=0;
      unpackword=0;
      fRawReader->ReadNextInt(word);
      unpackword= UnpackWord(word,8,31);
      time=unpackword; //T1
  
      word=0;
      unpackword=0;

      fRawReader->ReadNextInt(word);
      unpackword=UnpackWord(word,0,5);
      pmt=unpackword;
      word=0;
      unpackword=0;
      fRawReader->ReadNextInt(word);
      unpackword= UnpackWord(word,8,31);
      adc=unpackword-time;  // T1+ T2 (A)
      chargeTDC2->AddAt(adc,pmt);
 

    }
   fDigits->SetTime(*timeTDC1);
   fDigits->SetADC(*chargeTDC1);
 

   word=0;
   unpackword=0;
    
   fRawReader->ReadNextInt(word);
   unpackword=UnpackWord(word,0,5);
   pmt=unpackword;

   word=0;
   unpackword=0;
   
   fRawReader->ReadNextInt(word);
   unpackword=UnpackWord(word,8,31);
   time=unpackword;
   fDigits->SetMeanTime(time);   
    
   // Best time right &left  
   word=0;
   unpackword=0;
   
   fRawReader->ReadNextInt(word);
   unpackword=UnpackWord(word,0,5);
   pmt=unpackword;

   word=0;
   unpackword=0;

   fRawReader->ReadNextInt(word);
   unpackword=UnpackWord(word,8,31);
   time=unpackword;
   fDigits->SetTimeBestRight(time);   
 

   // best time left 
   word=0;
   unpackword=0;
     
   fRawReader->ReadNextInt(word);
   unpackword=UnpackWord(word,0,5);
   pmt=unpackword;

   word=0;
   unpackword=0;
   
   fRawReader->ReadNextInt(word);
   unpackword=UnpackWord(word,8,31);
   time=unpackword;
   fDigits->SetTimeBestLeft(time);   
   

   // best time differece  
    word=0;
   unpackword=0;
   
   fRawReader->ReadNextInt(word);
   unpackword=UnpackWord(word,0,5);
   pmt=unpackword;

   word=0;
   unpackword=0;
   
   fRawReader->ReadNextInt(word);
   unpackword=UnpackWord(word,8,31);
   time=unpackword;
   fDigits->SetDiffTime(time);   
 
 //  multiplicity 
   for (Int_t im=0; im<6; im++)
     {
       word=0;
       unpackword=0;
       fRawReader->ReadNextInt(word);
       unpackword=UnpackWord(word,0,5);
       pmt=unpackword;
       word=0;
       unpackword=0;
       fRawReader->ReadNextInt(word);
       unpackword=UnpackWord(word,8,31);
       time=unpackword;
       sumMult->AddAt(time,im);
     }
        fDigits->SetSumMult(*sumMult);   

    fTree->Fill();

}
 
