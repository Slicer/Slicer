/*=auto=========================================================================

(c) Copyright 2001 Massachusetts Institute of Technology

Permission is hereby granted, without payment, to copy, modify, display 
and distribute this software and its documentation, if any, for any purpose, 
provided that the above copyright notice and the following three paragraphs 
appear on all copies of this software.  Use of this software constitutes 
acceptance of these terms and conditions.

IN NO EVENT SHALL MIT BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, 
INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE 
AND ITS DOCUMENTATION, EVEN IF MIT HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

MIT SPECIFICALLY DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTIES INCLUDING, 
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.

THE SOFTWARE IS PROVIDED "AS IS."  MIT HAS NO OBLIGATION TO PROVIDE 
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================auto=*/
#include "vtkImageLogOdds.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "assert.h"

//------------------------------------------------------------------------
vtkImageLogOdds* vtkImageLogOdds::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageLogOdds");
  if(ret)
  {
    return (vtkImageLogOdds*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageLogOdds;

}


//----------------------------------------------------------------------------
vtkImageLogOdds::vtkImageLogOdds() { 
  this->DimProbSpace = 2;
  this->DimOutput = 1;
  this->DimInput = 0;

  this->Mode = 0;
  this->XDim = this->YDim = this->ZDim = this->InputScalarType = -1;

  // Probability range [epsilon , 1]
  // otherwise get -inf
  this->epsilon = 1.0e-15;

  // It odes not matter how much higher you go bc everything is mapped to one anyway ! 
  this->omega = 1.0e15;

  this->LogOddsType = LOGODDS_INSIDE_POSTIVE;
  this->Alpha = 1.0;

  this->MapMinProb = 0.01;
 
  this->results.clear();

  this->LabelList.clear();
}

vtkImageLogOdds::~vtkImageLogOdds()
{
  if ( this->results.size()) {
    for (int i = 0 ; i < (int) this->results.size(); i++)
      {
         if (this->results[i])
         {
            this->results[i]->Delete();
          }
      }
    this->results.clear();
  }
 this->LabelList.clear();
}
//----------------------------------------------------------------------------
void vtkImageLogOdds::SetDimProbSpace(int init) {
  // First Define Mode before calling this function ! 
  assert((init > 1) && this->Mode); 
  int newDimOutput = ((this->Mode == LOGODDS_LOG2MAP) ? 1 : ((this->Mode != LOGODDS_LOG2PROB) ? init-1: init));
  this->DimInput  = ((this->Mode ==  LOGODDS_PROB2LOG) ? init: init-1); 
 
  int oldSize = (int) this->results .size();
  if (oldSize > newDimOutput) 
    {
       for (int i = oldSize -1 ; i >= newDimOutput; i--)
      {
          if (this->results[i])
           {
         this->results[i]->Delete();
          }
      }
    }
    this->results.resize(newDimOutput);
    if (oldSize < newDimOutput) 
    {
         for (int i = oldSize; i < newDimOutput; i++)
         {
             this->results[i] = vtkImageData::New();
         }
    }

   // This does not work anymore with VTK 5 - so I will just created the variable results ! 
  // Add new ones
  // for (int i = this->DimOutput ; i < newDimOutput; i++) {
  //  
  //    this->vtkSource::AddOutput(vtkImageData::New());
  //    // So Filter knows they are empty
  //    this->Outputs[i]->ReleaseData();
  //    this->Outputs[i]->Delete();
  // }
  // // Delete old ones     
  // for (int i = this->DimOutput -1; i >=  newDimOutput; i--) this->vtkSource::RemoveOutput(this->Outputs[i]);

  this->DimProbSpace = init;
  this->DimOutput  = newDimOutput;

  this->Modified();
}

// Only lower bound works due to numrical approximation errors 1- eps = 1 !  
#define vtkImageLogOddsProbabilityMin(a) (a < this->epsilon ? this->epsilon : a)

// Special case - in two DimProbSpace = 2 then logodds = log( p / (1-p))  
void  vtkImageLogOdds::LogOdds(float *probPtr, float *logPtr) {
  const int N_POINTS =  this->XDim * this->YDim * this->ZDim ;

  for (int x = 0; x <  N_POINTS; x++) {
    if (*probPtr < this->epsilon) *logPtr++ = this->LogOddsType*log(this->epsilon);  
    else {
      if (1-*probPtr < this->epsilon) *logPtr++ = this->LogOddsType*log(this->omega);              
      else *logPtr++ = this->LogOddsType*log(double(*probPtr)/ double((1 - *probPtr)));    
    }
    probPtr ++;
  }
}


// Prob -> LogOdds => inptr = Prob, outptr = Log

void  vtkImageLogOdds::MultiNomialLogOdds(float **inptr, float **outptr) {
  if (this->DimProbSpace == 2 ) { this->LogOdds(inptr[0], outptr[0]); return;}

  const int N_POINTS =  this->XDim * this->YDim * this->ZDim ;
  const int DimLogSpace =  this->DimProbSpace -1;

  for (int i = 0; i < DimLogSpace;i++ ) {
    float *NormPtr = inptr[DimLogSpace];
    float *ProbPtr = inptr[i];
    float *LogOddsPtr = outptr[i]; 
    for ( int x = 0; x <  N_POINTS; x++ ) {
      *LogOddsPtr++ = this->LogOddsType*log(vtkImageLogOddsProbabilityMin(*ProbPtr)/ vtkImageLogOddsProbabilityMin(*NormPtr));
      ProbPtr++;NormPtr++;
    }    
  }
}

// LogOdds -> Prob => inptr = LogOdds, outPtr = Prob 
#define vtkImageLogOddsMax(a) (a > this->omega ? this->omega : a)

void  vtkImageLogOdds::GeneralizedLogistic(float **inptr, float **outptr) {
  const int N_POINTS =  this->XDim * this->YDim * this->ZDim ;
  const int DimLogSpace =  this->DimProbSpace -1;

  const double alphaReal = double(this->Alpha * this->LogOddsType);

  float** ProbPtr    = new float*[this->DimProbSpace];
  float** LogPtr     = new float*[DimLogSpace];
  double* ProbDouble = new double[DimLogSpace];
   
  for (int i = 0; i < DimLogSpace;i++ ) {
     ProbPtr[i] = outptr[i] ;
     LogPtr[i] = inptr[i];
  }
  ProbPtr[DimLogSpace] = outptr[DimLogSpace];

  // A little bit more accurate but not a lot faster
  for (int x = 0; x <  N_POINTS; x++ ) {
    double Norm  = 1.0;
    for (int i = 0; i < DimLogSpace;i++ ) {
      Norm += ProbDouble[i] = vtkImageLogOddsMax(double(exp(alphaReal * double(*LogPtr[i]))));
      LogPtr[i] ++;
    }
    Norm = 1.0 / Norm;
    for (int i = 0; i < DimLogSpace;i++ ) *ProbPtr[i]++ = float(double(ProbDouble[i]*Norm));
    *ProbPtr[DimLogSpace]++ = float(Norm);
  }

  delete[] ProbDouble; 
  delete[] ProbPtr;
  delete[] LogPtr;
}



// Higher accuracy 

// void  vtkImageLogOdds::GeneralizedLogistic(float **inptr, float **outptr) {
//   const int N_POINTS =  this->XDim * this->YDim * this->ZDim ;
//   const int DimLogSpace =  this->DimProbSpace -1;
// 
//   const float alphaReal = this->Alpha * this->LogOddsType;
//   {
//     float *NormPtr = outptr[DimLogSpace];
//     for (int x = 0; x <  N_POINTS; x++ ) *NormPtr++ = 1.0;
//   }
// 
//   for (int i = 0; i < DimLogSpace;i++ ) {
//     float* NormPtr = outptr[DimLogSpace];
//     float* ProbPtr = outptr[i];
//     float* LogOddsPtr = inptr[i];
//  
//     for (int x = 0; x <  N_POINTS; x++ ) {
//       *NormPtr++ +=  *ProbPtr++ = vtkImageLogOddsMax(double(exp( double(alphaReal * (*LogOddsPtr)))));
//       LogOddsPtr ++;
//     }
//   }
// 
//   {
//     float *NormPtr = outptr[DimLogSpace];
//     for (int x = 0; x <  N_POINTS; x++ ) *NormPtr++ = double(1.0)/double((*NormPtr));
//   }
//   for (int i = 0; i < DimLogSpace;i++ ) {
//     float* NormPtr = outptr[DimLogSpace];
//     float* ProbPtr = outptr[i];
//     float* LogOddsPtr = inptr[i]; 
// 
//     for (int x = 0; x <  N_POINTS; x++) {
//       *ProbPtr *= double(*NormPtr++);
//       if (*ProbPtr == 1.0) {cout << *LogOddsPtr << " " ; }
//       ProbPtr ++;
//       LogOddsPtr ++;
//     }
// 
//   }
//   cout << " " << endl;
// }


void vtkImageLogOdds::LogOddsNorm(float **inptr, float **outptr) {
  const int N_POINTS =  this->XDim * this->YDim * this->ZDim ;
  const int DimLogSpace =  this->DimProbSpace -1;

  float** ProbPtr = new float*[this->DimProbSpace];
  for (int i = 0; i < this->DimProbSpace; i++ ) ProbPtr[i] = new float[N_POINTS];

  // Transfere into the Probability Space
  this->GeneralizedLogistic(inptr,ProbPtr);
  // Transfere back into LogOdds Space indepdentently from each other 
  for (int i = 0; i < DimLogSpace; i++ ) this->LogOdds(ProbPtr[i],outptr[i]);

  for (int i = 0; i < this->DimProbSpace; i++ ) delete[] ProbPtr[i]; 
  delete[] ProbPtr;
}

void vtkImageLogOdds::LogOddsMap(float **inptr, short **outptr) {
  const int N_POINTS =  this->XDim * this->YDim * this->ZDim ;
  const int DimLogSpace =  this->DimProbSpace -1;

  const float LogMin = log (this->MapMinProb / (1 - this->MapMinProb));
  // I did it this way for speed and memory efficiency
  // this->GeneralizedLogistic(inptr,ProbPtr);

  // If label list is not correctly defined - just create one 
  if (int(this->LabelList.size()) != this->DimProbSpace )
    {
      cout << "LabelList is not correctly defined - so we create the default" << endl; 
      this->LabelList.resize(this->DimProbSpace);
      for (int i = 0; i <  this->DimProbSpace ; i++)
    {
      this->LabelList[i] = (i+1) %  this->DimProbSpace;
    }
    }

  // Transfere back into LogOdds Space indepdentently from each other 
  for (int x = 0; x <  N_POINTS; x++) {
     int maxIndex = 0;
     float maxVal = this->LogOddsType * inptr[0][x]; 
      // Without background 
      for (int i = 1; i < DimLogSpace ; i++ ) {
        if (maxVal < this->LogOddsType * inptr[i][x]) {
          maxIndex = i;
          maxVal = this->LogOddsType * inptr[i][x]; 
        } 
      }
      // Background (id = DimLogSpace) gets assigned to label 0 ! 
      // Currently only do it if max prob is below a certain threshold - might want to change this
      if (maxVal <  LogMin) maxIndex = DimLogSpace;
      outptr[0][x] = this->LabelList[maxIndex];;
    }  
}


vtkImageData* vtkImageLogOdds::GetOutput(int index) {
  assert((index > -1) && (index < this->DimOutput));
  return this->results[index];
}

vtkImageData* vtkImageLogOdds::GetProbabilities(int index) {
  assert(this->Mode ==  LOGODDS_LOG2PROB);
  return this->GetOutput(index);
}

vtkImageData* vtkImageLogOdds::GetLogOdds(int index) {
  assert(this->Mode !=  LOGODDS_LOG2PROB);
  return this->GetOutput(index);
}

vtkImageData* vtkImageLogOdds::GetMap() {
  assert(this->Mode ==  LOGODDS_LOG2MAP);
  return this->GetOutput(0);
}



//----------------------------------------------------------------------------
void  vtkImageLogOdds::InitializeOutputs()
{
   int Ext[6];
  Ext[0] = Ext[2] = Ext[4] = 0;
  Ext[1] = this->XDim-1;
  Ext[3] = this->YDim-1;
  Ext[5] = this->ZDim-1;
  vtkIdType IncX, IncY, IncZ;
  for (int i=0; i < this->DimOutput; i++) {
    this->results[i]->SetWholeExtent(Ext);
    this->results[i]->SetExtent(Ext); 
    this->results[i]->SetNumberOfScalarComponents(1);
    // Can be easily changed if needed
    if (this->Mode != LOGODDS_LOG2MAP ) {
      this->results[i]->SetScalarType(VTK_FLOAT);
    } else {
      this->results[i]->SetScalarType(VTK_SHORT);
    }
    this->results[i]->AllocateScalars(); 

    this->results[i]->GetContinuousIncrements(Ext,IncX,IncY,IncZ);
    // Can be easily changed 
    if (IncY ||IncZ ) {
      vtkErrorMacro(<<"Currently only works if increments of Y and Z are 0");
      return; 
    }

  }  
  return;
}

int  vtkImageLogOdds::CheckInput(vtkImageData *InData) {
    int Ext[6];
    vtkIdType Inc[3];

    InData->GetWholeExtent(Ext);
    if ((this->XDim != Ext[1] -Ext[0] +1) || (this->YDim != Ext[3] -Ext[2] +1) || (this->ZDim != Ext[5] -Ext[4] +1)) {
      vtkErrorMacro(<< "Input does not have the correct dimensions - should: ("<< this->XDim <<"," << this->YDim <<"," << this->ZDim <<") has: ("<< Ext[1] -Ext[0] +1 <<"," <<Ext[3] -Ext[2] +1 <<","<< Ext[5] -Ext[4] +1 << ")" );
      return 1;
    }
    InData->GetContinuousIncrements(Ext, Inc[0],Inc[1],Inc[2]);
    // We can easily adjust the algorithm to take care of it 
    if (Inc[1] != 0 || Inc[2] != 0) {
      vtkErrorMacro(<< "Input does have increments unequal to zero!");
      return 1;
    }
    if (InData->GetScalarType() != this->InputScalarType) {
      vtkErrorMacro(<< "Inputs have different Scalar Types ! Should be Float ("<< this->InputScalarType <<") but is " << InData->GetScalarType());
      return 1;
    }
    // Could be easily adjusted if needed to -> Compare this filter to vtkPCAAnalysis
    if (InData->GetNumberOfScalarComponents() != 1) {
      vtkErrorMacro(<< "Number Of Scalar Components for all Input has to be 1");
      return 1;
    }
    return 0;
}


//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImageLogOdds::ExecuteData(vtkDataObject *)
{
   vtkDebugMacro(<<"ExecuteData()");
   // Initial Check
   assert(this->Mode);

   int NumInputs = this->vtkProcessObject::GetNumberOfInputs();

   if(!NumInputs) {
    vtkErrorMacro(<<"No input!");
    return;
  }
 
  // Special case bc we only need one input
   // cout << "fsdfsdf " << NumInputs << " " << this->DimInput << endl;
  assert((NumInputs == this->DimInput) || ((NumInputs == 1) && (this->DimInput == 2) && (this->Mode ==  LOGODDS_PROB2LOG)));


  // Redefine ImageRelatedClass Parameters   
  vtkImageData **inData  = (vtkImageData **) this->GetInputs();
  { 
    int Ext[6];
    inData[0]->GetWholeExtent(Ext);
    this->XDim= Ext[1] - Ext[0] + 1;
    this->YDim= Ext[3] - Ext[2] + 1;
    this->ZDim= Ext[5] - Ext[4] + 1;
    if (!(this->XDim * this->YDim * this->ZDim)) {
    vtkErrorMacro( << "Input has no points!" );
    return;
    }
    // Can be changed later
    this->InputScalarType = VTK_FLOAT; 
  }
  for (int i = 0; i < NumInputs; i++) {
    if (this->CheckInput(inData[i])) return;
  }

  // -----------------------------------------------
  // Set Ouput correctly
  this->InitializeOutputs();
 
  // Run Algorithm  
  void** outPtr = new void*[this->DimOutput];
  for (int i = 0 ; i < this->DimOutput; i++)
    outPtr[i] = (void*) this->results[i]->GetScalarPointerForExtent(this->results[i]->GetExtent());

  // Run Algorithm  
  float** inPtr = new float*[NumInputs];
  for (int i = 0 ; i <  NumInputs; i++) inPtr[i] = (float*) (inData[i]->GetScalarPointerForExtent(inData[i]->GetExtent()));
  switch (this->Mode) 
    {
    case LOGODDS_LOG2PROB : this->GeneralizedLogistic(inPtr,(float**)outPtr); break;
    case LOGODDS_PROB2LOG : this->MultiNomialLogOdds(inPtr,(float**)outPtr); break;
    case LOGODDS_LOGNORM  : this->LogOddsNorm(inPtr,(float**)outPtr);break;
    case LOGODDS_LOG2MAP  : this->LogOddsMap(inPtr,(short**) outPtr);break;
    default:  vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
  }
  // Clean up 
  delete[] outPtr;
  delete[] inPtr;
}


void vtkImageLogOdds::SetProbabilities(int index, vtkImageData *image) 
{
     assert(this->Mode ==  LOGODDS_PROB2LOG); 
     this->SetInput(index,image);
}

void vtkImageLogOdds::SetLogOdds(int index, vtkImageData *image)
{
      assert(this->Mode > LOGODDS_PROB2LOG);  
      this->SetInput(index,image);
}

 // See earlier explanations about different modes
void vtkImageLogOdds::SetMode_Log2Prob() {
    assert(!this->Mode); 
    this->Mode = LOGODDS_LOG2PROB;
}

void vtkImageLogOdds::SetMode_Prob2Log() {
   assert(!this->Mode); 
   this->Mode = LOGODDS_PROB2LOG;
}

void vtkImageLogOdds::SetMode_LogNorm()  {
   assert(!this->Mode); 
   this->Mode = LOGODDS_LOGNORM;
}

void vtkImageLogOdds::SetMode_Log2Map()  {
     assert(!this->Mode); 
     this->Mode = LOGODDS_LOG2MAP;
}

void vtkImageLogOdds::SetMapMinProb(float init) {
   assert(this->Mode == LOGODDS_LOG2MAP); 
   this->MapMinProb = init;
}

void vtkImageLogOdds::ThreadedExecute(vtkImageData **inData, vtkImageData *outData,int outExt[6], int id) 
{
   assert(0);
} 

