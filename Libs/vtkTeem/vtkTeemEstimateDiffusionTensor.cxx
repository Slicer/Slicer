/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTeemEstimateDiffusionTensor.cxx,v $
  Date:      $Date: 2006/10/24 20:06:47 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include "vtkTeemEstimateDiffusionTensor.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"


#define VTKEPS 10e-12

//----------------------------------------------------------------------------
vtkTeemEstimateDiffusionTensor* vtkTeemEstimateDiffusionTensor::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkTeemEstimateDiffusionTensor");
  if(ret)
    {
      return (vtkTeemEstimateDiffusionTensor*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkTeemEstimateDiffusionTensor;
}


//----------------------------------------------------------------------------
vtkTeemEstimateDiffusionTensor::vtkTeemEstimateDiffusionTensor()
{
  // may be set by user
  this->Transform = NULL;

  this->NumberOfGradients = 7;

  // this is LeBihan's b factor for physical MR gradient parameters 
  // (same number as C-F uses)
  this->B = vtkDoubleArray::New();
  this->B->SetNumberOfComponents(1);
  this->B->SetNumberOfTuples(this->NumberOfGradients);
  for (int i=0; i<this->NumberOfGradients;i++)
    this->B->SetValue(i,1000);

  // Scalar Factor for the tensor values
  //this->ScaleFactor = 1;


  this->MinimumSignalValue = 1.0;

  this->Sigma = 0.0;

  this->EstimationMethod = tenEstimateMethodLLS;

  this->NumberOfWLSIterations = 1;

  this->knownB0 = 0;
 
  // defaults are from DT-MRI 
  // (from Processing and Visualization for 
  // Diffusion Tensor MRI, C-F Westin, pg 8)
  this->DiffusionGradient = vtkDoubleArray::New();
  this->DiffusionGradient->SetNumberOfComponents(3);
  this->DiffusionGradient->SetNumberOfTuples(this->NumberOfGradients);
  this->SetDiffusionGradient(0,0,0,0);
  this->SetDiffusionGradient(1,1,1,0);
  this->SetDiffusionGradient(2,0,1,1);
  this->SetDiffusionGradient(3,1,0,1);
  this->SetDiffusionGradient(4,0,1,-1);
  this->SetDiffusionGradient(5,1,-1,0);
  this->SetDiffusionGradient(6,-1,0,1);

}
vtkTeemEstimateDiffusionTensor::~vtkTeemEstimateDiffusionTensor()
{
  this->B->Delete();
  this->DiffusionGradient->Delete();
  if (this->Transform) 
    {
    this->Transform->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkTeemEstimateDiffusionTensor::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageToImageFilter::PrintSelf(os,indent);

  os << indent << "NumberOfGradients: " << this->NumberOfGradients << "\n";
  double g[3];
  // print all of the gradients
  for (int i = 0; i < this->NumberOfGradients; i++ ) 
    {
      this->GetDiffusionGradient(i,g);
      os << indent << "Gradient " << i << ": (" 
         << g[0] << ", "
         << g[1] << ", "
         << g[2] << ")" << "\n";
      
    }  
}

//----------------------------------------------------------------------------
void vtkTeemEstimateDiffusionTensor::TransformDiffusionGradients()
{
  vtkFloatingPointType gradient[3];
  double g[3];
  // if matrix has not been set by user don't use it
  if (this->Transform == NULL) 
    {
      return;
    }

  vtkDebugMacro("Transforming diffusion gradients");
  //this->Transform->Print(cout);


  // transform each gradient by this matrix
  for (int i = 0; i < this->NumberOfGradients; i++ ) 
    {
      this->GetDiffusionGradient(i,g);
      this->Transform->TransformPoint(g,gradient);

      // set the gradient to the transformed one 
      // (note this set function normalizes too)
      this->SetDiffusionGradient(i,gradient);
    }
}

//----------------------------------------------------------------------------
// The number of required inputs is one more than the number of
// diffusion gradients applied.  (Since image 0 is an image
// acquired without diffusion gradients).
void vtkTeemEstimateDiffusionTensor::SetNumberOfGradients(int num) 
{
  if (this->NumberOfGradients != num)
    {
      vtkDebugMacro ("setting num gradients to " << num);
      // internal array for storage of gradient vectors
      this->DiffusionGradient->SetNumberOfTuples(num);
      this->B->SetNumberOfTuples(num);
      // this class's info
      this->NumberOfGradients = num;
      //this->NumberOfRequiredInputs = num;
      this->Modified();
    }
}

void vtkTeemEstimateDiffusionTensor::GetDiffusionGradient(int num,double grad[3])
  {  if (num<this->DiffusionGradient->GetNumberOfTuples()) {
       grad[0]=this->DiffusionGradient->GetComponent(num,0);
       grad[1]=this->DiffusionGradient->GetComponent(num,1);
       grad[2]=this->DiffusionGradient->GetComponent(num,2);
     } else {
       vtkErrorMacro("Gradient number is out of range");
     }
  }

//----------------------------------------------------------------------------
//
void vtkTeemEstimateDiffusionTensor::ExecuteInformation(vtkImageData *inData, 
                                             vtkImageData *outData)
{
  // We always want to output input scalars Type
  outData->SetScalarType(inData->GetScalarType());
  // We output two scalar components: baseline and average of dwis
  outData->SetNumberOfScalarComponents(1);

}

//----------------------------------------------------------------------------
// Replace superclass Execute with a function that allocates tensors
// as well as scalars.  This gets called before multithreader starts
// (after which we can't allocate, for example in ThreadedExecute).
// Note we return to the regular pipeline at the end of this function.
void vtkTeemEstimateDiffusionTensor::ExecuteData(vtkDataObject *out)
{
  vtkImageData *output = vtkImageData::SafeDownCast(out);
  vtkImageData *inData = (vtkImageData *) this->GetInput();

  //Check inputs numbertenEstimateMethodLLS

  if (inData == NULL) {
    vtkErrorMacro("Input with DWIs has not been assigned");
    return;
  }  

  //Check if this input is multicomponent and match the number of  gradients
  int ncomp = this->GetInput()->GetPointData()->GetScalars()->GetNumberOfComponents();
  if (ncomp != this->NumberOfGradients) {
      vtkErrorMacro("The input has to have a number of components equal to the number of gradients");
      return;
    }  
  
  // set extent so we know how many tensors to allocate
  output->SetExtent(output->GetUpdateExtent());

  // allocate output tensors
  vtkFloatArray* data = vtkFloatArray::New();
  int* dims = output->GetDimensions();
  vtkDebugMacro("Allocating output tensors, dims " << dims[0] <<" " << dims[1] << " " << dims[2]);
  data->SetNumberOfComponents(9);
  data->SetNumberOfTuples(dims[0]*dims[1]*dims[2]);
  output->GetPointData()->SetTensors(data);
  data->Delete();


  // make sure our gradient matrix is up to date
  //This update is not thread safe and it has to be performed outside
  // the threaded part.
  // if the user has transformed the coordinate system
  this->TransformDiffusionGradients();
  
  //Allocate Internals
  int N = this->GetNumberOfGradients();

  // jump back into normal pipeline: call standard superclass method here
  //Do not jump to do the proper allocation of output data
  this->vtkImageToImageFilter::ExecuteData(out);

}



//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
static void vtkTeemEstimateDiffusionTensorExecute(vtkTeemEstimateDiffusionTensor *self,
                                           vtkImageData *inData, 
                                           T * inPtr,
                                           vtkImageData *outData, 
                                           T * outPtr,
                                           int outExt[6], int id)
{
  int idxX, idxY, idxZ;
  int maxX, maxY, maxZ;
  int inIncX, inIncY, inIncZ;
  int outIncX, outIncY, outIncZ;
  unsigned long count = 0;
  unsigned long target;
  int numInputs, k,i,j;
  double *dwi;
  double B0;
  vtkDataArray *outTensors;
  double D[3][3];
  float outT[3][3];
  int ptId;

  // values that we have to set up later on.
  double sigma, bValue;

  // Get information to march through output tensor data
  outTensors = self->GetOutput()->GetPointData()->GetTensors();


  // Set Ten Context
  tenEstimateContext *tec = tenEstimateContextNew();
  if (self->SetTenContext(tec)) {
    cout<<"TenContext cannot be set. Bailing out"<<endl;
    tenEstimateContextNix(tec);
    return;
  }

  // changed from arrays to pointers
  int *outInc,*outFullUpdateExt;
  outInc = self->GetOutput()->GetIncrements();
  outFullUpdateExt = self->GetOutput()->GetUpdateExtent(); //We are only working over the update extent
  ptId = ((outExt[0] - outFullUpdateExt[0]) * outInc[0]
         + (outExt[2] - outFullUpdateExt[2]) * outInc[1]
         + (outExt[4] - outFullUpdateExt[4]) * outInc[2]);


  // find the region to loop over
  maxX = outExt[1] - outExt[0];
  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  target = (unsigned long)(outData->GetNumberOfScalarComponents()*
                           (maxZ+1)*(maxY+1)/50.0);
  target++;

  // Get increments to march through image data 
  inData->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  numInputs = inData->GetNumberOfScalarComponents();
  dwi = new double[numInputs]; 

  double _ten[7];

  for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
      for (idxY = 0; !self->AbortExecute && idxY <= maxY; idxY++)
        {
          if (!id) 
            {
              if (!(count%target)) 
                {
                  self->UpdateProgress(count/(50.0*target) 
                                       + (maxZ+1)*(maxY+1));
                }
              count++;
            }
          for (idxX = 0; idxX <= maxX; idxX++)
            {
             // create tensor from combination of gradient inputs
             for (int i=0; i< numInputs; i++) 
             {
               dwi[i] = (double) inPtr[i];
             }
             // Set dwi to context
             //Main method
              tenEstimate1TensorSingle_d(tec,_ten, dwi);
              outT[0][0] = _ten[1];
              outT[0][1] = outT[1][0] = _ten[2];
              outT[0][2] = outT[2][0] = _ten[3];
              outT[1][1] = _ten[4];
              outT[1][2] = outT[2][1] = _ten[5];
              outT[2][2] = _ten[6];

              // Pixel operation              
              outTensors->SetTuple(ptId,(float *)outT);

              // copy no diffusion data through for scalars
              *outPtr = (T) B0;

              //*outPtr = (T) tec->errorDwi;

              //Pending: Compute the average tensor upon request.

              inPtr += inIncX;
              ptId ++;
              outPtr++;
            }
          outPtr += outIncY;
          ptId += outIncY;
          inPtr += inIncY;
         }
      outPtr += outIncZ;
      ptId += outIncZ;
      inPtr += inIncZ;
    }

  delete [] dwi;

  // Delete Context
  tenEstimateContextNix(tec);
}

int vtkTeemEstimateDiffusionTensor::SetGradientsToContext(tenEstimateContext *tec) 
{
  Nrrd *ngrad =NULL;
  Nrrd *nbmat =NULL;
  //Fill ngrad  from vtkArray
  ngrad  = nrrdNew();
  nbmat = nrrdNew();
  char *err;
  const int type = nrrdTypeDouble;
  size_t size[2];
  size[0]=3;
  size[1]=this->DiffusionGradient->GetNumberOfTuples();
  double *data = (double *) this->DiffusionGradient->GetVoidPointer(0);
  if(nrrdWrap_nva(ngrad ,data,type,2,size)) {
    biffAdd(NRRD, err);
    sprintf(err,"%s:",this->GetClassName());
    nrrdNuke(ngrad);
    nrrdNuke(nbmat);
    return 1;
  }
  
  // To accomodate different b-values we might have to rescale the gradients
  double maxB = this->B->GetRange()[1];
  data = (double *) (ngrad ->data);
  double factor;
  for (int i=0; i< size[1]; i++) {
   factor =  1/sqrt(maxB/this->B->GetValue(i));
   data[0] = data[0] * factor;
   data[1] = data[1] * factor;
   data[2] = data[2] * factor;
   data += 3;
  }

  //tenEstimateGradientsSet(tec,ngrad,maxB,!this->knownB0);

  if (tenBMatrixCalc(nbmat,ngrad) ) {
    biffAdd(NRRD, err);
    sprintf(err,"%s:",this->GetClassName());
    nrrdNuke(ngrad);
    nrrdNuke(nbmat);
    return 1;
  }  
  tenEstimateBMatricesSet(tec,nbmat,maxB,!this->knownB0);
  tec->knownB0 = this->knownB0;
  nrrdNuke(ngrad);
  return 0;
}

int vtkTeemEstimateDiffusionTensor::SetTenContext(  tenEstimateContext *tec)
{
    tec->progress = AIR_TRUE;

    // Set gradients
    if (this->SetGradientsToContext(tec)) {
      vtkErrorMacro("Error setting gradient into tenEstimateContext. Bailing out");
      return 1;
    }

    int EE = 0;
    int verbose = 0;
    char *err;

    if (!EE) tenEstimateVerboseSet(tec, verbose);
    if (!EE) EE |= tenEstimateMethodSet(tec, this->EstimationMethod);
    if (!EE) EE |= tenEstimateValueMinSet(tec, this->MinimumSignalValue);
 
    switch(this->EstimationMethod) {
    case tenEstimateMethodLLS:
        tec->recordErrorLogDwi = AIR_TRUE;
        /* tec->recordErrorDwi = AIR_TRUE; */
      break;
    case tenEstimateMethodNLS:
        tec->recordErrorDwi = AIR_TRUE;
      break;
    case tenEstimateMethodWLS:
      if (!EE) tec->WLSIterNum = this->NumberOfWLSIterations;
      tec->recordErrorDwi = AIR_TRUE;
      break;
    /*
    case tenEstimateMethodMLE:
      if (this->Sigma < 0.0)) {
        vtkErrorMacro("Noise sigma has to be positive >=0.0");
        return 1;
      }
      if (!EE) EE |= tenEstimateSigmaSet(tec, this->Sigma);
      tec->recordErrorDwi = AIR_TRUE;
      break;
     */
    }

    // Do not set any threshold for the mask. Do that later
    if (!EE) EE |= tenEstimateThresholdSet(tec, 0, 1);
    if (!EE) EE |= tenEstimateUpdate(tec);
 
    if (EE) {
      err=biffGetDone(TEN);
      fprintf(stderr, "%s: trouble setting up estimation:\n%s\n",
      this->GetClassName(), err);
      return 1;
    }

return 0;

}

//----------------------------------------------------------------------------
// This method is passed a input and output regions, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the regions data types.
void vtkTeemEstimateDiffusionTensor::ThreadedExecute(vtkImageData *inData, 
                                              vtkImageData *outData,
                                              int outExt[6], int id)
{
  int idx;
  void *inPtrs;
  void *outPtr = outData->GetScalarPointerForExtent(outExt);

  vtkDebugMacro("in threaded execute, " << this->GetNumberOfInputs() << " inputs ");


  inPtrs = new void*[this->NumberOfInputs];

  // Loop through to fill input pointer array
  inPtrs = inData->GetScalarPointerForExtent(outExt);


  // call Execute method to handle all data at the same time
  switch (inData->GetScalarType())
    {
      vtkTemplateMacro7(vtkTeemEstimateDiffusionTensorExecute, this, 
                        inData, (VTK_TT *)(inPtrs),
                        outData, (VTK_TT *)(outPtr), 
                        outExt, id);
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
  
}


