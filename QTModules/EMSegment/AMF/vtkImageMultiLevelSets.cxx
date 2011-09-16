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
#include "vtkImageMultiLevelSets.h" 
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkMultiThreader.h"

//---------------------------------------------------------------------------
VTK_THREAD_RETURN_TYPE vtkImageMultiLevelSets_ComputeLogOddsComponent_Threaded(void *arg )
{
    vtkImageMultiLevelSets* self;
    int             first,last;
    int             threadId;
    int             threadCount;

    threadId    = ((ThreadInfoStruct *)(arg))->ThreadID;
    threadCount = ((ThreadInfoStruct *)(arg))->NumberOfThreads;
    self = (vtkImageMultiLevelSets *)(((ThreadInfoStruct *)(arg))->UserData);
    self->DefineThread(first, last, threadId, threadCount);

    switch (self->GetMultiLevelVersion() ) {
      case vtkImageMultiLevelSets::IPMI_ORIG :
     self->ComputeLogOddsComponent(first,last);
     break;
      case vtkImageMultiLevelSets::IPMI_CORRECT:
     self->ComputeLogOddsComponent_IPMICorrect(first,last);
     break;
     //      case vtkImageMultiLevelSets::JOURNAL:
     // self->ComputeLogOddsComponent(first,last);
     // break;
      default: 
    std::cerr << "ERROR::vtkImageMultiLevelSets_ComputeLogOddsComponent_Threaded:: Do not know type " << self->GetMultiLevelVersion() << endl;
    }

    return VTK_THREAD_RETURN_VALUE;
} 

//------------------------------------------------------------------------
vtkImageMultiLevelSets* vtkImageMultiLevelSets::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageMultiLevelSets");
  if(ret)
  {
    return (vtkImageMultiLevelSets*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageMultiLevelSets;

}


//----------------------------------------------------------------------------
vtkImageMultiLevelSets::vtkImageMultiLevelSets() { 
  //  We do not have any outputs - but do not remove first output ever - otherwise seg fault!
  this->vtkSource::RemoveOutput(this->Outputs[0]);

  this->Curves= NULL;
  this->CurvesOutputWithoutLogOdds = NULL;
  this->CurvesOutputComplete       = NULL;
  this->CurvesOutputJustLogOdds    = NULL;

  this->logCondIntensityCoefficient = NULL;
  this->logCondIntensityInside = -1;
  // logCondIntensityImage is defined as input 
  this->logCurveCouplingCoefficient = NULL;

  this->probCondWeightMin = 0.0;
  this->NumberOfCurves = 0;
  this->XDim = this->YDim = this->ZDim = this->ImgDim = -1;

  this->Threader = NULL;

  this->MultiLevelVersion =  IPMI_ORIG; 
}

vtkImageMultiLevelSets::~vtkImageMultiLevelSets() { 
  if (this->Curves) {
    delete[] this->Curves;
    this->Curves = NULL;
  }

  if (this->CurvesOutputComplete) {
    delete[] this->CurvesOutputComplete;
    this->CurvesOutputComplete = NULL;
  }

  if (this->CurvesOutputWithoutLogOdds) {
    for (int i = 0 ; i < this->NumberOfCurves; i++) this->CurvesOutputWithoutLogOdds[i]->Delete();
    delete[] this->CurvesOutputWithoutLogOdds;
    this->CurvesOutputWithoutLogOdds = NULL;
  }

  if (this->CurvesOutputJustLogOdds) {
    delete[] this->CurvesOutputJustLogOdds;
    this->CurvesOutputJustLogOdds = NULL;
  }

  if (this->logCondIntensityCoefficient) {
    delete[] this->logCondIntensityCoefficient;
    this->logCondIntensityCoefficient = NULL;
  } 

  if (this->logCurveCouplingCoefficient) {
    delete[] this->logCurveCouplingCoefficient;
    this->logCurveCouplingCoefficient = NULL;
  } 


  if (this->Threader) this->Threader->Delete();
}

int  vtkImageMultiLevelSets::CheckInput(vtkImageData *InData) {
    int Ext[6];
    vtkIdType Inc[3];

    assert(InData);
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
    if (InData->GetScalarType() != VTK_FLOAT) {
      vtkErrorMacro(<< "Inputs have different Scalar Types!");
      return 1;
    }
    // Could be easily adjusted if needed to -> Compare this filter to vtkPCAAnalysis
    if (InData->GetNumberOfScalarComponents() != 1) {
      vtkErrorMacro(<< "Number Of Scalar Components for all Input has to be 1");
      return 1;
    }
    return 0;
}


void vtkImageMultiLevelSets::SetNumberOfCurves(int initNumCurves) {
  assert(initNumCurves > 0);

  if (this->Curves) delete[] this->Curves;
  this->Curves = new vtkImageLevelSets*[initNumCurves];

  if (this->CurvesOutputWithoutLogOdds) {
    for (int i = 0 ; i < this->NumberOfCurves; i++) this->CurvesOutputWithoutLogOdds[i]->Delete();
    delete[] this->CurvesOutputWithoutLogOdds;
  }
  this->CurvesOutputWithoutLogOdds = new vtkImageData*[initNumCurves];

  if (this->CurvesOutputJustLogOdds) delete[] this->CurvesOutputJustLogOdds;
  this->CurvesOutputJustLogOdds = new float*[initNumCurves];

  if (this->CurvesOutputComplete) delete[] this->CurvesOutputComplete;
  this->CurvesOutputComplete = new vtkImageData*[initNumCurves];


  if (this->logCondIntensityCoefficient) delete[] this->logCondIntensityCoefficient;
  this->logCondIntensityCoefficient = new float[initNumCurves];

  if (this->logCurveCouplingCoefficient) delete[] this->logCurveCouplingCoefficient;
  this->logCurveCouplingCoefficient = new float[initNumCurves];

  for (int i = 0; i < initNumCurves; i++) {
    this->Curves[i]                      = NULL;
    this->CurvesOutputComplete[i]        = NULL;
    this->CurvesOutputWithoutLogOdds[i]  = vtkImageData::New();
    this->CurvesOutputJustLogOdds[i]     = NULL; 
    this->logCondIntensityCoefficient[i] = 0.0;
    this->logCurveCouplingCoefficient[i] = 0.0;
  }
  this->NumberOfCurves = initNumCurves;
  this->Modified();
}

// Multi Level Set specific functions
void vtkImageMultiLevelSets::SetCurve(int initID, vtkImageLevelSets  *initWithoutLogOdds, vtkImageData  *initInitialCurve, 
                      vtkImageData *initLogCondIntImage,float initLogCondIntCoeff,  float initLogCurveCouplingCoeff, vtkImageData  *initFinalCurve) {
  // cout << "SetInitialCurve: initID " << initID << " NumberOfCurves " << NumberOfCurves << endl;

  assert((initID > -1) && (initID < this->NumberOfCurves));

  // Initialize each LevelSet representing a curve
  this->Curves[initID] = initWithoutLogOdds;

  // We set the data term to zero as each curve only knows about itself but not the others 
  // however to compute the weight function you need to know all the other curves too!
  this->Curves[initID]->SetlogCondIntensityCoefficient(0.0); 
  // Just a Dummy
  this->Curves[initID]->SetlogCondIntensityImage(initLogCondIntImage); 

  // COMMENT 1:
  // The same as for data term should be done for the smoothing term: The weight for the smoothing term should be disabled and computed later on in this software
  // Problem: if you do that  ut = min(max(StepDt*ut,-Band),Band); turns out to be different, i.e. the update of the parameters based on the curvature.
  // I do not have indepth knowledge about the implementation and so do not know the implicatins if ut is larger than the band ! So that we do not set it right now 
  // If you decide to uncomment the next few lines then please make sure to properly multiply the smoothing term when computing the curve evolution 
  // if (this->ultiLevelVersion != IPMI_ORIG)
  //  { 
  //    this->Curves[initID]->SetprobCondWeightMin(1.0); 
  //  }

  // this->CurvesOutputWithoutLogOdds is the output that Curves->Iter saves the results to
  this->Curves[initID]->InitParam(initInitialCurve,this->CurvesOutputWithoutLogOdds[initID]);

  this->CurvesOutputComplete[initID] = initFinalCurve;

  // Define parameters specific to MultiLevelSet   
  this->SetInput(initID,initLogCondIntImage);
  this->logCondIntensityCoefficient[initID] = initLogCondIntCoeff;
  this->logCurveCouplingCoefficient[initID] = initLogCurveCouplingCoeff;
}



//----------------------------------------------------------------------------
int vtkImageMultiLevelSets::InitParam()
{   
   vtkImageData **inData  = (vtkImageData **) this->GetInputs();
   int Ext[6];
 
   assert(inData[0]);
   inData[0]->GetWholeExtent(Ext);
   this->XDim= Ext[1] - Ext[0] + 1;
   this->YDim= Ext[3] - Ext[2] + 1;
   this->ZDim= Ext[5] - Ext[4] + 1;
   this->ImgDim = this->ZDim * this->YDim * this->XDim ;
   assert((Ext[1] >= Ext[0]) && (Ext[3] >= Ext[2]) && (Ext[5] >= Ext[4]));

   if (!(this->ImgDim)) {
     vtkErrorMacro( << "Input has no points!" );
     return 1;
   }

   vtkIdType  IncX, IncY, IncZ;
   for (int i=0; i < this->NumberOfCurves; i++) {
     this->CurvesOutputComplete[i]->SetWholeExtent(Ext);
     this->CurvesOutputComplete[i]->SetExtent(Ext); 
     this->CurvesOutputComplete[i]->SetNumberOfScalarComponents(1);
     this->CurvesOutputComplete[i]->SetScalarType(VTK_FLOAT);
     this->CurvesOutputComplete[i]->AllocateScalars();
     this->CurvesOutputComplete[i]->GetContinuousIncrements(Ext,IncX,IncY,IncZ);

    // Can be easily changed 
    if (IncY ||IncZ ) {
      vtkErrorMacro(<<"Currently only works if increments of Y and Z are 0");
      return 1; 
    }
   }  
   // Initialize Threading function
   if (this->Threader) this->Threader->Delete();
   this->Threader = vtkMultiThreader::New();
   int NumThreads = vtkMultiThreader::GetGlobalDefaultNumberOfThreads();
   this->Threader->SetNumberOfThreads(NumThreads);
   // cout << "vtkImageMultiLevelSets::InitParam: Run cost function on " <<  NumThreads << " threads." << endl; 
   this->Threader->SetSingleMethod(vtkImageMultiLevelSets_ComputeLogOddsComponent_Threaded, this);
   return 0;
}

int vtkImageMultiLevelSets::InitEvolution() {
  assert(this->vtkProcessObject::GetNumberOfInputs()== this->NumberOfCurves && this->NumberOfCurves);
   vtkImageData **inData  = (vtkImageData **) this->GetInputs();
  
  for (int i=0; i < this->NumberOfCurves; i++) {
    Curves[i]->InitEvolution();
    if (this->CheckInput(inData[i])) return 1;
    // sign can be flipped which is done in InitEvolution
    this->CurvesOutputComplete[i]->CopyAndCastFrom(this->CurvesOutputWithoutLogOdds[i],this->CurvesOutputWithoutLogOdds[i]->GetExtent());
  }
  // vtkIndent indent;
  // this->PrintSelf(cout,indent);
  // exit(0); 
  return 0;
} 

void vtkImageMultiLevelSets::DefineThread(int& first, int& last, const int id, const int total)
{
  assert(id < total);

  int NumVoxelsPerThread = this->ImgDim / total;
  first =  NumVoxelsPerThread *id;
  if (id < total - 1) last =  first + NumVoxelsPerThread;
  else  last =  this->ImgDim;
}

// Kilian 2010: This was the orginal used for IPMI but I believe it is wrong 
void vtkImageMultiLevelSets::ComputeLogOddsComponent(const int first, const int last) {
 
  // -----------------------------------------------
  // Initialize 

  float** previousIterationPtr       = new float*[this->NumberOfCurves];
  float** logCondIntensityImagePtr   = new float*[this->NumberOfCurves];
  float** currentIterationPtr        = new float*[this->NumberOfCurves];

  float* logOddsDiffTerm             = new float[this->NumberOfCurves];
  float* ProbTerm                    = new float[this->NumberOfCurves];
  vtkImageData** inData  = (vtkImageData**) this->GetInputs();

  for (int i = 0 ; i < this->NumberOfCurves; i++) { 
     previousIterationPtr[i]         = (float*) this->CurvesOutputComplete[i]->GetScalarPointer();
     logCondIntensityImagePtr[i]     = (float*) inData[i]->GetScalarPointerForExtent(inData[i]->GetExtent()); 
     // Result after smoothing - Here we took a short cut in the curve evolution as we only computed the weight for the smoothing term via the bimodal setting !
     currentIterationPtr[i]          = (float*) this->CurvesOutputWithoutLogOdds[i]->GetScalarPointer();
  }
 
  for (int x = first ; x < last; x++) {
    float NormTerm = 1.0;
   
    for(int i = 0; i < this->NumberOfCurves; i++) {  
      // ProbTerm = \SIG(\para)
      // Turn log odds into probabilities - remember that the LogOdds of the curve evolution define the inside with negative and the outside with positive numbers ! 
      // => \SIG(-previousIterationPtr) ! 
      NormTerm += ProbTerm[i] = exp(-previousIterationPtr[i][x]);

      // Likelihood Term
      // \para - L
      logOddsDiffTerm[i] = (previousIterationPtr[i][x] - this->logCondIntensityInside * logCondIntensityImagePtr[i][x]); 
    }
    for(int i = 0; i < this->NumberOfCurves; i++)  ProbTerm[i] = ProbTerm[i]/NormTerm;

    for(int i = 0; i < this->NumberOfCurves; i++) { 
      if (logCondIntensityCoefficient[i]) { 
    float logConditionalTerm = 0.0;
    float probWeight = 0.0;
    float logConditional ;
    for(int j = 0; j < this->NumberOfCurves; j++) {
      if (i == j) {
        float probCondWeight = ProbTerm[i] * (1.0 - ProbTerm[i]);
        probWeight = ((probCondWeight < this->probCondWeightMin ) ?  this->probCondWeightMin : probCondWeight ); 
      } else {
        probWeight = ProbTerm[i] * ProbTerm[j];
      }
      // I think this is not correct as it simply takes the intensity difference but not the difference between log odds value for i!=j 
          // it then should be previousIterationPtr[i][x] - previousIterationPtr[j][x] - see next version
      logConditionalTerm += probWeight * logOddsDiffTerm[j] ;
    }
    // Should not be the following way bc how currentIterationPtr is defined  
    // currentIterationPtr[i][x] = this->logConditionalTermMinusOne[i] * currentIterationPtr[i][x]  - this->logCondIntensityCoefficient[i] * logConditionalTerm;
        logConditional = -this->logCondIntensityCoefficient[i] * logConditionalTerm;

    currentIterationPtr[i][x] +=  logConditional;
        if (this->CurvesOutputJustLogOdds[i]) this->CurvesOutputJustLogOdds[i][x] = logConditional;

      }
    }
  }

  delete[] previousIterationPtr;
  delete[] currentIterationPtr;
  delete[] logCondIntensityImagePtr; 
  delete[] logOddsDiffTerm; 
  delete[] ProbTerm; 
}

// Kilian 2010: I modified the function according to what is written down at IPMI
// I am not sure if it works correctly or not 
void vtkImageMultiLevelSets::ComputeLogOddsComponent_IPMICorrect(const int first, const int last) {
 
  // -----------------------------------------------
  // Initialize 

  float** previousIterationPtr       = new float*[this->NumberOfCurves];
  float** logCondIntensityImagePtr   = new float*[this->NumberOfCurves];
  float** currentIterationPtr        = new float*[this->NumberOfCurves];

  float* probTerm                    = new float[this->NumberOfCurves];
  // LogOdds value from the previous iteration
  float* para                        = new float[this->NumberOfCurves];

  vtkImageData** inData  = (vtkImageData**) this->GetInputs();

  for (int i = 0 ; i < this->NumberOfCurves; i++) { 
     previousIterationPtr[i]         = (float*) this->CurvesOutputComplete[i]->GetScalarPointer();
     logCondIntensityImagePtr[i]     = (float*) inData[i]->GetScalarPointerForExtent(inData[i]->GetExtent()); 
     currentIterationPtr[i]          = (float*) this->CurvesOutputWithoutLogOdds[i]->GetScalarPointer();
  }
 
  for (int x = first ; x < last; x++) {
    float normTerm = 1.0 ;
    // Define Probabilities for Weight Function and Parameter
    for(int i = 0; i < this->NumberOfCurves; i++) {  
      // ProbTerm = \SIG(\para)
      // Turn log odds into probabilities - remember that the LogOdds of the curve evolution define the inside with negative and the outside with positive numbers ! 
      // => \SIG(-previousIterationPtr) !
      para[i]   = previousIterationPtr[i][x];
      normTerm += probTerm[i] = exp(- para[i]);
    }

    //
    // Normalize Weight and add intensity as well as to the curve evolution
    //
    for(int i = 0; i < this->NumberOfCurves; i++)  {
      probTerm[i] = probTerm[i]/normTerm;
       // Add intensity term to smoothing term
      if (this->logCondIntensityCoefficient[i] > 0.0) {
    //  the '-' is due to the fact that we do the para <-  \para - \lambda( smoothing  - coupling + intensity) 
        // unlike in our derivations \para is negative inside so that curvature is negative to one we define in our paper 
        // and thus the smoothing term is negative what we define in our paper -> 
        // so that in this implementation we have para <-  \para + \lambda( smoothing  + coupling - intensity) 

        // Note change due to COMMENT 1 in SetCurve 
    float probWeight = probTerm[i] * (1.0 - probTerm[i]);
    currentIterationPtr[i][x] -= ((probWeight < this->probCondWeightMin ) ?  this->probCondWeightMin : probWeight ) * this->logCondIntensityCoefficient[i] * (previousIterationPtr[i][x] - this->logCondIntensityInside * logCondIntensityImagePtr[i][x]); 
      }
      // Multply now with weighing function 
      // we do not do that due to  COMMENT 1 in function SetCurve 
      // float probWeight = probTerm[i] * (1.0 - probTerm[i]);
      //currentIterationPtr[i][x] *= ((probWeight < this->probCondWeightMin ) ?  this->probCondWeightMin : probWeight );        
    }

    //
    // Coupling term 
    //
    for(int i = 0; i < this->NumberOfCurves; i++) { 
      float logCouplingTerm = 0.0;
      if (this->logCurveCouplingCoefficient[i] > 0.0) { 
    for(int j = 0; j < this->NumberOfCurves; j++) {
      if (i == j) { continue; }
      float probWeight = probTerm[i] * probTerm[j];
      if (probWeight == 0.0) { continue; }      
      logCouplingTerm += (para[i] - para[j]);
    }
    // given that we subtract intensities I guess I have to add the coupling term 
    currentIterationPtr[i][x] +=  this->logCurveCouplingCoefficient[i]*logCouplingTerm;
      }
      if (this->CurvesOutputJustLogOdds[i]) this->CurvesOutputJustLogOdds[i][x] = logCouplingTerm; 
    }
  }

  delete[] previousIterationPtr;
  delete[] currentIterationPtr;
  delete[] logCondIntensityImagePtr; 
  delete[] para; 
  delete[] probTerm; 
}


// Kilian 2010: journal version 
void vtkImageMultiLevelSets::ComputeLogOddsComponent_journal(const int first, const int last) {
 
  // -----------------------------------------------
  // Initialize 

  float** previousIterationPtr       = new float*[this->NumberOfCurves];
  float** logCondIntensityImagePtr   = new float*[this->NumberOfCurves];
  float** currentIterationPtr        = new float*[this->NumberOfCurves];

  float* probTerm                    = new float[this->NumberOfCurves];
  // LogOdds value from the previous iteration
  float* para                        = new float[this->NumberOfCurves];

  vtkImageData** inData  = (vtkImageData**) this->GetInputs();

  for (int i = 0 ; i < this->NumberOfCurves; i++) { 
     previousIterationPtr[i]         = (float*) this->CurvesOutputComplete[i]->GetScalarPointer();
     logCondIntensityImagePtr[i]     = (float*) inData[i]->GetScalarPointerForExtent(inData[i]->GetExtent()); 
     currentIterationPtr[i]          = (float*) this->CurvesOutputWithoutLogOdds[i]->GetScalarPointer();
  }
 
  for (int x = first ; x < last; x++) {
    float normTerm = 1.0 ;
    // Define Probabilities for Weight Function and Parameter
    for(int i = 0; i < this->NumberOfCurves; i++) {  
      // ProbTerm = \SIG(\para)
      // Turn log odds into probabilities - remember that the LogOdds of the curve evolution define the inside with negative and the outside with positive numbers ! 
      // => \SIG(-previousIterationPtr) !
      para[i]   = previousIterationPtr[i][x];
      normTerm += probTerm[i] = exp(- para[i]);
    }

    //
    // Normalize Weight and add intensity as well as to the curve evolution
    //
    for(int i = 0; i < this->NumberOfCurves; i++)  {
      probTerm[i] = probTerm[i]/normTerm;
       // Add intensity term to smoothing term
      if (this->logCondIntensityCoefficient[i] > 0.0) {
    //  the '-' is due to the fact that we do the para <-  \para - \lambda( smoothing  - coupling + intensity) 
        // unlike in our derivations \para is negative inside so that curvature is negative to one we define in our paper 
        // and thus the smoothing term is negative what we define in our paper -> 
        // so that in this implementation we have para <-  \para + \lambda( smoothing  + coupling - intensity) 

        // Note change due to COMMENT 1 in SetCurve 
    float probWeight = probTerm[i] * (1.0 - probTerm[i]);
    currentIterationPtr[i][x] -= ((probWeight < this->probCondWeightMin ) ?  this->probCondWeightMin : probWeight ) * this->logCondIntensityCoefficient[i] * (previousIterationPtr[i][x] - this->logCondIntensityInside * logCondIntensityImagePtr[i][x]); 
      }
      // Multply now with weighing function 
      // we do not do that due to  COMMENT 1 in function SetCurve 
      // float probWeight = probTerm[i] * (1.0 - probTerm[i]);
      //currentIterationPtr[i][x] *= ((probWeight < this->probCondWeightMin ) ?  this->probCondWeightMin : probWeight );        
    }

    //
    // Coupling term 
    //
    for(int i = 0; i < this->NumberOfCurves; i++) { 
      float logCouplingTerm = 0.0;
      if (this->logCurveCouplingCoefficient[i] > 0.0) { 
    for(int j = 0; j < this->NumberOfCurves; j++) {
      if (i == j) { continue; }
      float probWeight = probTerm[i] * probTerm[j];
      if (probWeight == 0.0) { continue; }      
      logCouplingTerm += (para[i] - para[j]);
    }
    // given that we subtract intensities I guess I have to add the coupling term 
    currentIterationPtr[i][x] +=  this->logCurveCouplingCoefficient[i]*logCouplingTerm;
      }
      if (this->CurvesOutputJustLogOdds[i]) this->CurvesOutputJustLogOdds[i][x] = logCouplingTerm; 
    }
  }

  delete[] previousIterationPtr;
  delete[] currentIterationPtr;
  delete[] logCondIntensityImagePtr; 
  delete[] para; 
  delete[] probTerm; 
}

int vtkImageMultiLevelSets::Iterate() {
  // cout << "vtkImageMultiLevelSets::Iterate " << endl;

  // -------------------------------------------------------------
  // Initialize iteration by running curve evolution without LogOdds 
  for (int i = 0 ; i < this->NumberOfCurves; i++) this->Curves[i]->Iterate();

  // -----------------------------------------------
  // Add LogOdds Component
  Threader->SingleMethodExecute(); 

  // -----------------------------------------------
  // Update Results and clean up 
  for(int i = 0; i < this->NumberOfCurves; i++) {  
     float *previousIterationPtr         = (float*) this->CurvesOutputComplete[i]->GetScalarPointer();
     float *currentIterationPtr          = (float*) this->CurvesOutputWithoutLogOdds[i]->GetScalarPointer();
     memcpy(previousIterationPtr,currentIterationPtr,sizeof(float)*this->ImgDim);
  }

  return 1;
}

void vtkImageMultiLevelSets::EndEvolution() {
  for (int i=0; i < this->NumberOfCurves; i++) {
    Curves[i]->EndEvolution();
    this->CurvesOutputComplete[i]->CopyAndCastFrom(this->CurvesOutputWithoutLogOdds[i],this->CurvesOutputWithoutLogOdds[i]->GetExtent());
  }
}


//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImageMultiLevelSets::ExecuteData(vtkDataObject *)
{
  // Should not be used bc while(Iterate) goes on forever right now  
  assert(0);

  this->InitParam(); 
  this->InitEvolution();
  while (this->Iterate());
  this->EndEvolution();
}

void vtkImageMultiLevelSets::PrintSelf(ostream& os, vtkIndent indent)
{
   os << indent << "logCondIntensityInside: "      << this->logCondIntensityInside         << "\n";
   os << indent << "probCondWeightMin: "           << this->probCondWeightMin         << "\n";
   os << indent << "NumberOfCurves: "              << this->NumberOfCurves        << "\n";
   for (int i = 0; i < this->NumberOfCurves; i++) os << indent << "logCondIntensityCoefficient " << i <<  ": " << this->logCondIntensityCoefficient[i] << "\n";    
}

