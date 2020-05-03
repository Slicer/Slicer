/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkDiffusionTensorMathematics.cxx,v $
  Date:      $Date: 2006/12/19 17:14:44 $
  Version:   $Revision: 1.43 $

=========================================================================auto=*/

#ifdef _WIN32
// to pick up M_SQRT2 and other nice things...
#define _USE_MATH_DEFINES
#endif

// But, if you are on VS6.0 you don't get the define...
#include "vtkDataArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkImageData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkDiffusionTensorMathematics.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkTransform.h"
#include "vtkPointData.h"
#ifndef M_SQRT2
#define M_SQRT2    1.41421356237309504880168872421      /* sqrt(2) */
#endif

extern "C" {
#include "teem/ten.h"
}

#include <ctime>
#include <limits>

#define VTK_EPS 1e-16
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MAX3(a,b,c) (MAX(a,MAX(b,c)))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MIN3(a,b,c) (MIN(a,MIN(b,c)))
#define OUT_OF_RANGE_TO_BOUNDRY(v, a, b) ( ((v) <= (a)) ? (a) : (((v) >= (b)) ? (b) : (v)))

vtkCxxSetObjectMacro(vtkDiffusionTensorMathematics,TensorRotationMatrix,vtkMatrix4x4);
vtkCxxSetObjectMacro(vtkDiffusionTensorMathematics,ScalarMask,vtkImageData);

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkDiffusionTensorMathematics);


//----------------------------------------------------------------------------
vtkDiffusionTensorMathematics::vtkDiffusionTensorMathematics()
{
  this->Operation = VTK_TENS_TRACE;

  this->ScaleFactor = 1.0;
  this->ExtractEigenvalues = 1;
  this->TensorRotationMatrix = nullptr;
  this->ScalarMask = nullptr;
  this->MaskWithScalars = 0;
  this->FixNegativeEigenvalues = 1;
  this->MaskLabelValue = 1;
}

//----------------------------------------------------------------------------
 vtkDiffusionTensorMathematics::~vtkDiffusionTensorMathematics()
 {
   if( this->TensorRotationMatrix )
     {
     this->TensorRotationMatrix->Delete();
     }
   if( this->ScalarMask )
     {
     this->ScalarMask->Delete();
     }
 }

//----------------------------------------------------------------------------
int vtkDiffusionTensorMathematics::RequestInformation (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  //vtkInformation *inInfo2 = inputVector[1]->GetInformationObject(0);

  int ext[6];

  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext);

  //int ext[6], *ext2, idx;


  // We want the whole extent of the tensors, and should
  // allocate the matching size image data.
  vtkDebugMacro(<<"Execute information extent: " <<
    ext[0] << " " << ext[1] << " " << ext[2] << " " <<
    ext[3] << " " << ext[4] << " " << ext[5]);


  // We always want to output float, unless it is color
  if (this->Operation == VTK_TENS_COLOR_ORIENTATION)
    {
    // output color (RGBA)
    vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_UNSIGNED_CHAR, 4);
    }
  else if (this->Operation == VTK_TENS_COLOR_MODE)
    {
    // output color (RGBA)
    vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_UNSIGNED_CHAR, 4);
    }
  else {
    vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_FLOAT, 1);
    }
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext, 6);
  return 1;
}



//----------------------------------------------------------------------------
int vtkDiffusionTensorMathematics::FillInputPortInformation(
  int port, vtkInformation* info)
{
  if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}


int vtkDiffusionTensorMathematics
::RequestData(vtkInformation* request, vtkInformationVector** inputVector,
              vtkInformationVector* outputVector)
{
  int res = this->Superclass::RequestData(request, inputVector, outputVector);
  for (int i = 0; i < this->GetNumberOfOutputPorts(); ++i)
    {
    vtkInformation* info = outputVector->GetInformationObject(i);
    vtkImageData *outData = static_cast<vtkImageData *>(
      info->Get(vtkDataObject::DATA_OBJECT()));
    outData->GetPointData()->SetTensors(nullptr);
    }
  return res;
}

//----------------------------------------------------------------------------
static void GetContinuousIncrements(vtkImageData* img, int extent[6], vtkIdType &incX,
                                    vtkIdType &incY, vtkIdType &incZ)
{
  int e0, e1, e2, e3;

  incX = 0;
  const int* selfExtent = img->GetExtent();

  e0 = extent[0];
  if (e0 < selfExtent[0])
    {
    e0 = selfExtent[0];
    }
  e1 = extent[1];
  if (e1 > selfExtent[1])
    {
    e1 = selfExtent[1];
    }
  e2 = extent[2];
  if (e2 < selfExtent[2])
    {
    e2 = selfExtent[2];
    }
  e3 = extent[3];
  if (e3 > selfExtent[3])
    {
    e3 = selfExtent[3];
    }

  // Make sure the increments are up to date
  vtkIdType inc[3];
  img->GetArrayIncrements(img->GetPointData()->GetTensors(), inc);
  //ComputeIncrements(img, inc);

  incY = inc[1] - (e1 - e0 + 1)*inc[0];
  incZ = inc[2] - (e3 - e2 + 1)*inc[1];
}

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
// Handles the one input operations.
// Handles the ops where eigensystems are not computed.
template <class T>
static void vtkDiffusionTensorMathematicsExecute1(vtkDiffusionTensorMathematics *self,
                    vtkImageData *in1Data,
                    vtkImageData *outData, T *outPtr,
                    int outExt[6], int id)
{
  // image variables
  int idxR, idxY, idxZ;
  int maxY, maxZ;
  vtkIdType outIncX, outIncY, outIncZ;
  vtkIdType inIncX, inIncY, inIncZ;
  int rowLength;
  // progress
  unsigned long count = 0;
  unsigned long target;
  // math operation
  int op = self->GetOperation();
  // tensor variables
  vtkDataArray *inTensors;
  double tensor[3][3];
  vtkPointData *pd;
  // time
#ifndef NDEBUG
  clock_t tStart, tEnd, tDiff;
  tStart = clock();
#endif
  // scaling
  double scaleFactor = self->GetScaleFactor();

  // find the input region to loop over
  pd = in1Data->GetPointData();
  inTensors = pd->GetTensors();

  if ( !inTensors || in1Data->GetNumberOfPoints() < 1 )
    {
    vtkGenericWarningMacro(<<"No input tensor data to filter!");
    return;
    }

  if (self->GetScalarMask() && self->GetScalarMask()->GetScalarType() != VTK_SHORT)
    {
    vtkGenericWarningMacro(<<"scalr type for mask must be short!");
    return;
    }

  // find the output region to loop over
  rowLength = (outExt[1] - outExt[0]+1);
  maxY = outExt[3] - outExt[2];
  maxZ = outExt[5] - outExt[4];
  target = (unsigned long)((maxZ+1)*(maxY+1)/50.0);
  target++;

  // Get increments to march through output data
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  // Call special version of GetContinuousIncrements that works for Tensors
  GetContinuousIncrements(in1Data, outExt, inIncX, inIncY, inIncZ);

  //Initialize ptId to walk through tensor volume
  // - these must be of type float - output type will be float or
  // unsigned char RGBA depending on the Operation selected.
  // See RequestInformation above.
  float* inPtr = reinterpret_cast<float*>(in1Data->GetArrayPointerForExtent(inTensors, outExt));

  bool doMasking = false;
  short * inMaskPtr = nullptr;
  vtkIdType maskIncX = 0;
  vtkIdType maskIncY = 0;
  vtkIdType maskIncZ = 0;
  if (self->GetMaskWithScalars() && self->GetScalarMask())
    {
    self->GetScalarMask()->GetContinuousIncrements(outExt, maskIncX, maskIncY, maskIncZ);
    inMaskPtr = reinterpret_cast<short *>(self->GetScalarMask()->GetScalarPointerForExtent(outExt));
    doMasking = self->GetScalarMask()->GetPointData()->GetScalars() != nullptr;
    }

  for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
    for (idxY = 0; idxY <= maxY; idxY++)
      {
      if (!id)
        {

        if (!(count%target))
          {
          self->UpdateProgress(count/(50.0*target));
          }
        count++;
        }

      for (idxR = 0; idxR < rowLength; idxR++)
        {

        if (doMasking && *inMaskPtr != self->GetMaskLabelValue())
          {
          *outPtr = 0;
          }
        else
          {

          // tensor at this voxel
          tensor[0][0] = static_cast<double>(inPtr[0]);
          tensor[0][1] = static_cast<double>(inPtr[1]);
          tensor[0][2] = static_cast<double>(inPtr[2]);
          tensor[1][0] = static_cast<double>(inPtr[3]);
          tensor[1][1] = static_cast<double>(inPtr[4]);
          tensor[1][2] = static_cast<double>(inPtr[5]);
          tensor[2][0] = static_cast<double>(inPtr[6]);
          tensor[2][1] = static_cast<double>(inPtr[7]);
          tensor[2][2] = static_cast<double>(inPtr[8]);

          // pixel operation
          switch (op)
            {
          case vtkDiffusionTensorMathematics::VTK_TENS_D11:
            *outPtr = (T)(scaleFactor*tensor[0][0]);
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_D22:
            *outPtr = (T)(scaleFactor*tensor[1][1]);
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_D33:
            *outPtr = (T)(scaleFactor*tensor[2][2]);
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_TRACE:
            /////////////////////*outPtr = (T)(scaleFactor*(tensor[0][0]
            //               +tensor[1][1]
            //               +tensor[2][2]));
            *outPtr = static_cast<T> (scaleFactor*vtkDiffusionTensorMathematics::Trace(tensor));
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_DETERMINANT:
            //*outPtr =
            //  (T)(scaleFactor*(vtkMath::Determinant3x3(tensor)));
            *outPtr = static_cast<T> (scaleFactor*vtkDiffusionTensorMathematics::Determinant(tensor));
            break;
            }
        }

        outPtr++;
        inPtr+=9;
        inMaskPtr++;
        }
      outPtr += outIncY;
      inPtr += inIncY;
      inMaskPtr += maskIncY;
      }
    outPtr += outIncZ;
    inPtr += inIncZ;
    inMaskPtr += maskIncZ;
    }

#ifndef NDEBUG
  tEnd = clock();
  tDiff = tEnd - tStart;
  vtkDebugWithObjectMacro(self, << "tDiff:" << tDiff);
#endif
}

// copied from
// vtkInteractorStyleUnicam.cxx
template <class Type>
inline Type tensor_math_clamp(const Type a,
                  const Type b,
                  const Type c) { return (a) > (b) ? ((a) < (c) ? (a) : (c)) : (b) ; }

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
// Handles the one input operations.
// Handles the ops where eigensystems are computed.
template <class T>
static void vtkDiffusionTensorMathematicsExecute1Eigen(vtkDiffusionTensorMathematics *self,
                          vtkImageData *in1Data,
                          vtkImageData *outData,
                          T *outPtr,
                          int outExt[6], int id)
{
  // image variables
  int idxR, idxY, idxZ;
  int maxY, maxZ;
  vtkIdType outIncX, outIncY, outIncZ;
  vtkIdType inIncX, inIncY, inIncZ;
  int rowLength;
  // progress
  unsigned long count = 0;
  unsigned long target;
  // math operation
  int op = self->GetOperation();
  // tensor variables
  vtkDataArray *inTensors;
  double tensor[3][3];
  vtkPointData *pd;
  int numPts;
  // time
#ifndef NDEBUG
  clock_t tStart, tEnd, tDiff;
  tStart = clock();
#endif
  // working matrices
  double *m[3], w[3], *v[3];
  double m0[3], m1[3], m2[3];
  double v0[3], v1[3], v2[3];
  double v_maj[3];
  m[0] = m0; m[1] = m1; m[2] = m2;
  v[0] = v0; v[1] = v1; v[2] = v2;
  int i, j;
  double r, g, b;
  int extractEigenvalues;
  double cl;
  // scaling
  double scaleFactor = self->GetScaleFactor();

  // map 0..1 values into the range a char takes on
  // but use scaleFactor so user can bump up the brightness
  const double rgb_scale = (double)VTK_UNSIGNED_CHAR_MAX * scaleFactor / 1000.;
  double rgb_temp = 0.;

  // find the input region to loop over
  pd = in1Data->GetPointData();
  inTensors = pd->GetTensors();
  numPts = in1Data->GetNumberOfPoints();

  if ( !inTensors || numPts < 1 )
    {
    vtkGenericWarningMacro(<<"No input tensor data to filter!");
    return;
    }
  if (self->GetScalarMask() && self->GetScalarMask()->GetScalarType() != VTK_SHORT)
    {
    vtkGenericWarningMacro(<<"scalr type for mask must be short!");
    return;
    }

  // find the output region to loop over
  rowLength = (outExt[1] - outExt[0]+1);
  maxY = outExt[3] - outExt[2];
  maxZ = outExt[5] - outExt[4];
  target = (unsigned long)((maxZ+1)*(maxY+1)/50.0);
  target++;

  // Get increments to march through output data
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  // Call special version of GetContinuousIncrements that works for Tensors
  GetContinuousIncrements(in1Data, outExt, inIncX, inIncY, inIncZ);

  //Initialize ptId to walk through tensor volume
  // - these must be of type float - output type will be float or
  // unsigned char RGBA depending on the Operation selected.
  // See RequestInformation above.
  float* inPtr = reinterpret_cast<float*>(in1Data->GetArrayPointerForExtent(inTensors, outExt));

  // decide whether to extract eigenfunctions or just use input cols
  extractEigenvalues = self->GetExtractEigenvalues();

  // transformation of tensor orientations for coloring
  vtkTransform *trans = vtkTransform::New();
  int useTransform = 0;

  // if the user has set this matrix grab it
  if (self->GetTensorRotationMatrix())
    {
    trans->SetMatrix(self->GetTensorRotationMatrix());
    useTransform = 1;
    }

  // Check for masking
  bool doMasking = false;
  short * inMaskPtr = nullptr;
  vtkIdType maskIncX = 0;
  vtkIdType maskIncY = 0;
  vtkIdType maskIncZ = 0;
  if (self->GetMaskWithScalars() && self->GetScalarMask())
    {
    self->GetScalarMask()->GetContinuousIncrements(outExt, maskIncX, maskIncY, maskIncZ);
    inMaskPtr = reinterpret_cast<short *>(self->GetScalarMask()->GetScalarPointerForExtent(outExt));
    doMasking = self->GetScalarMask()->GetPointData()->GetScalars() != nullptr;
    }

   //vtkGenericWarningMacro( "Do masking: " << doMasking );


  // Loop through output pixels and input points

  for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
    for (idxY = 0; idxY <= maxY; idxY++)
      {
      if (!id)
        {
        if (!(count%target))
          {
          self->UpdateProgress(count/(50.0*target));
          }
        count++;
        }

      for (idxR = 0; idxR < rowLength; idxR++)
        {
        if (doMasking && *inMaskPtr != self->GetMaskLabelValue())
          {
          *outPtr = 0;

          if (op ==  vtkDiffusionTensorMathematics::VTK_TENS_COLOR_MODE ||
            op ==  vtkDiffusionTensorMathematics::VTK_TENS_COLOR_ORIENTATION) {
            outPtr++;
            *outPtr = 0; // green
            outPtr++;
            *outPtr = 0; // blue
            outPtr++;
            *outPtr = VTK_UNSIGNED_CHAR_MAX ; // alpha
          }
        }
        else {

          // tensor at this voxel
          tensor[0][0] = static_cast<double>(inPtr[0]);
          tensor[0][1] = static_cast<double>(inPtr[1]);
          tensor[0][2] = static_cast<double>(inPtr[2]);
          tensor[1][0] = static_cast<double>(inPtr[3]);
          tensor[1][1] = static_cast<double>(inPtr[4]);
          tensor[1][2] = static_cast<double>(inPtr[5]);
          tensor[2][0] = static_cast<double>(inPtr[6]);
          tensor[2][1] = static_cast<double>(inPtr[7]);
          tensor[2][2] = static_cast<double>(inPtr[8]);

          // get eigenvalues and eigenvectors appropriately
          if (extractEigenvalues)
            {
            for (j=0; j<3; j++)
              {
              for (i=0; i<3; i++)
                {
                // transpose
                m[i][j] = tensor[j][i];
                }
              }
            // compute eigensystem
            //vtkMath::Jacobi(m, w, v);
            vtkDiffusionTensorMathematics::TeemEigenSolver(m,w,v);
            }
          else
            {
            // tensor columns are evectors scaled by evals
            for (i=0; i<3; i++)
              {
              v0[i] = tensor[i][0];
              v1[i] = tensor[i][1];
              v2[i] = tensor[i][2];
              }
            w[0] = vtkMath::Normalize(v0);
            w[1] = vtkMath::Normalize(v1);
            w[2] = vtkMath::Normalize(v2);
            }

          //Correct for negative eigenvalues. Three possible options:
          //  1. Round to zero
          //  2. Take absolute value
          //  3. Increase eigenvalues by negative part
          // The two first options have been problematic. Try 3
          if (self->GetFixNegativeEigenvalues()==1){
            const double min_eval = MIN3(w[0], w[1], w[2]);
            if (min_eval < 0)
              {
                const double add_to_eval = -min_eval + VTK_EPS;
                w[0] += add_to_eval;
                w[1] += add_to_eval;
                w[2] += add_to_eval;
              }
//            if (vtkDiffusionTensorMathematics::FixNegativeEigenvaluesMethod(w)) {
//              vtkGenericWarningMacro( "Warning: Eigenvalues are not properly sorted" );
//            }
            if ((w[0] < 0) || (w[1] < 0) || (w[2] < 0))
              vtkGenericWarningMacro( "Warning: Negative Eigenvalues after positivity fix" );
          } else {
            if (w[0] < 0)
              w[0] = 0;
            if (w[1] < 0)
              w[1] = 0;
            if (w[2] < 0)
              w[2] = 0;
          }

          // pixel operation
          switch (op)
            {
          case vtkDiffusionTensorMathematics::VTK_TENS_RELATIVE_ANISOTROPY:
            *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::RelativeAnisotropy(w));
            break;
          case vtkDiffusionTensorMathematics::VTK_TENS_FRACTIONAL_ANISOTROPY:
            *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::FractionalAnisotropy(w));
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_LINEAR_MEASURE:
            *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::LinearMeasure(w));
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_PLANAR_MEASURE:
            *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::PlanarMeasure(w));
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_SPHERICAL_MEASURE:
            *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::SphericalMeasure(w));
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_MAX_EIGENVALUE:
            *outPtr = (T)w[0];
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_MID_EIGENVALUE:
            *outPtr = (T)w[1];
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_MIN_EIGENVALUE:
            *outPtr = (T)w[2];
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_PARALLEL_DIFFUSIVITY:
            *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::ParallelDiffusivity(w));
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_PERPENDICULAR_DIFFUSIVITY:
            *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::PerpendicularDiffusivity(w));
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_MEAN_DIFFUSIVITY:
            *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::MeanDiffusivity(w));
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_MAX_EIGENVALUE_PROJX:
            *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::MaxEigenvalueProjectionX(v,w));
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_MAX_EIGENVALUE_PROJY:
            *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::MaxEigenvalueProjectionY(v,w));
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_MAX_EIGENVALUE_PROJZ:
            *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::MaxEigenvalueProjectionZ(v,w));
            break;

      case vtkDiffusionTensorMathematics::VTK_TENS_RAI_MAX_EIGENVEC_PROJX:
        *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::RAIMaxEigenvecX(v,w));
        break;

      case vtkDiffusionTensorMathematics::VTK_TENS_RAI_MAX_EIGENVEC_PROJY:
        *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::RAIMaxEigenvecY(v,w));
        break;

      case vtkDiffusionTensorMathematics::VTK_TENS_RAI_MAX_EIGENVEC_PROJZ:
        *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::RAIMaxEigenvecZ(v,w));
        break;

      case vtkDiffusionTensorMathematics::VTK_TENS_MAX_EIGENVEC_PROJX:
        *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::MaxEigenvecX(v,w));
        break;

      case vtkDiffusionTensorMathematics::VTK_TENS_MAX_EIGENVEC_PROJY:
        *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::MaxEigenvecY(v,w));
        break;

      case vtkDiffusionTensorMathematics::VTK_TENS_MAX_EIGENVEC_PROJZ:
        *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::MaxEigenvecZ(v,w));
        break;


      case vtkDiffusionTensorMathematics::VTK_TENS_MODE:
            *outPtr = static_cast<T> (vtkDiffusionTensorMathematics::Mode(w));
            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_COLOR_MODE:

            vtkDiffusionTensorMathematics::ColorByMode(w,r,g,b);
            // scale maps 0..1 values into the range a char takes on
            rgb_temp = (rgb_scale*r);
            *outPtr = (T)tensor_math_clamp(rgb_temp, (double)VTK_UNSIGNED_CHAR_MIN, (double)VTK_UNSIGNED_CHAR_MAX);
            outPtr++;
            rgb_temp = (rgb_scale*g);
            *outPtr = (T)tensor_math_clamp(rgb_temp, (double)VTK_UNSIGNED_CHAR_MIN, (double)VTK_UNSIGNED_CHAR_MAX);
            outPtr++;
            rgb_temp = (rgb_scale*b);
            *outPtr = (T)tensor_math_clamp(rgb_temp, (double)VTK_UNSIGNED_CHAR_MIN, (double)VTK_UNSIGNED_CHAR_MAX);
            outPtr++;
            *outPtr = (T)VTK_UNSIGNED_CHAR_MAX; //alpha

            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_COLOR_ORIENTATION:
            // If the user has set the rotation matrix
            // then transform the eigensystem first
            // This is used to rotate the vector into RAS space
            // for consistent anatomical coloring.
            v_maj[0]=v[0][0];
            v_maj[1]=v[1][0];
            v_maj[2]=v[2][0];
            if (useTransform)
              {
              trans->TransformPoint(v_maj,v_maj);
              }
            // Color R, G, B depending on max eigenvector
            // scale maps 0..1 values into the range a char takes on
            cl = vtkDiffusionTensorMathematics::LinearMeasure(w);
            rgb_temp = (rgb_scale*fabs(v_maj[0])*cl);
            *outPtr = (T)tensor_math_clamp(rgb_temp, (double)VTK_UNSIGNED_CHAR_MIN, (double)VTK_UNSIGNED_CHAR_MAX);
            outPtr++;
            rgb_temp = (rgb_scale*fabs(v_maj[1])*cl);
            *outPtr = (T)tensor_math_clamp(rgb_temp, (double)VTK_UNSIGNED_CHAR_MIN, (double)VTK_UNSIGNED_CHAR_MAX);
            outPtr++;
            rgb_temp = (rgb_scale*fabs(v_maj[2])*cl);
            *outPtr = (T)tensor_math_clamp(rgb_temp, (double)VTK_UNSIGNED_CHAR_MIN, (double)VTK_UNSIGNED_CHAR_MAX);
            outPtr++;
            *outPtr = (T)VTK_UNSIGNED_CHAR_MAX; //alpha

            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_COLOR_ORIENTATION_MIDDLE_EIGENVECTOR:
            // If the user has set the rotation matrix
            // then transform the eigensystem first
            // This is used to rotate the vector into RAS space
            // for consistent anatomical coloring.
            v_maj[0]=v[0][1];
            v_maj[1]=v[1][1];
            v_maj[2]=v[2][1];
            if (useTransform)
              {
              trans->TransformPoint(v_maj,v_maj);
              }
            // Color R, G, B depending on max eigenvector
            // scale maps 0..1 values into the range a char takes on
            cl = vtkDiffusionTensorMathematics::LinearMeasure(w);
            rgb_temp = (rgb_scale*fabs(v_maj[0])*cl);
            *outPtr = (T)tensor_math_clamp(rgb_temp, (double)VTK_UNSIGNED_CHAR_MIN, (double)VTK_UNSIGNED_CHAR_MAX);
            outPtr++;
            rgb_temp = (rgb_scale*fabs(v_maj[1])*cl);
            *outPtr = (T)tensor_math_clamp(rgb_temp, (double)VTK_UNSIGNED_CHAR_MIN, (double)VTK_UNSIGNED_CHAR_MAX);
            outPtr++;
            rgb_temp = (rgb_scale*fabs(v_maj[2])*cl);
            *outPtr = (T)tensor_math_clamp(rgb_temp, (double)VTK_UNSIGNED_CHAR_MIN, (double)VTK_UNSIGNED_CHAR_MAX);
            outPtr++;
            *outPtr = (T)VTK_UNSIGNED_CHAR_MAX; //alpha

            break;

          case vtkDiffusionTensorMathematics::VTK_TENS_COLOR_ORIENTATION_MIN_EIGENVECTOR:
            // If the user has set the rotation matrix
            // then transform the eigensystem first
            // This is used to rotate the vector into RAS space
            // for consistent anatomical coloring.
            v_maj[0]=v[0][2];
            v_maj[1]=v[1][2];
            v_maj[2]=v[2][2];
            if (useTransform)
              {
              trans->TransformPoint(v_maj,v_maj);
              }
            // Color R, G, B depending on max eigenvector
            // scale maps 0..1 values into the range a char takes on
            cl = vtkDiffusionTensorMathematics::LinearMeasure(w);
            rgb_temp = (rgb_scale*fabs(v_maj[0])*cl);
            *outPtr = (T)tensor_math_clamp(rgb_temp, (double)VTK_UNSIGNED_CHAR_MIN, (double)VTK_UNSIGNED_CHAR_MAX);
            outPtr++;
            rgb_temp = (rgb_scale*fabs(v_maj[1])*cl);
            *outPtr = (T)tensor_math_clamp(rgb_temp, (double)VTK_UNSIGNED_CHAR_MIN, (double)VTK_UNSIGNED_CHAR_MAX);
            outPtr++;
            rgb_temp = (rgb_scale*fabs(v_maj[2])*cl);
            *outPtr = (T)tensor_math_clamp(rgb_temp, (double)VTK_UNSIGNED_CHAR_MIN, (double)VTK_UNSIGNED_CHAR_MAX);
            outPtr++;
            *outPtr = (T)VTK_UNSIGNED_CHAR_MAX; //alpha

            break;


            }

          // scale double if the user requested this
          if (scaleFactor != 1 && op !=  vtkDiffusionTensorMathematics::VTK_TENS_COLOR_ORIENTATION
            && op !=  vtkDiffusionTensorMathematics::VTK_TENS_COLOR_MODE)
            {
            *outPtr = (T) ((*outPtr) * scaleFactor);
            }
          }


        outPtr++;
        inPtr+=9;
        inMaskPtr++;
        }
      outPtr += outIncY;
      inPtr += inIncY;
      inMaskPtr += maskIncY;
      }
    outPtr += outIncZ;
    inPtr += inIncZ;
    inMaskPtr += maskIncZ;
    }
  // Cleanup
  trans->Delete();

#ifndef NDEBUG
  tEnd = clock();
  tDiff = tEnd - tStart;
  vtkDebugWithObjectMacro(self, << "tDiff:" << tDiff);
#endif
}

//----------------------------------------------------------------------------
// This method computes the increments from the MemoryOrder and the extent.
void vtkDiffusionTensorMathematics::ComputeTensorIncrements(vtkImageData *imageData, vtkIdType incr[3])
{
  int idx;
  // make sure we have data before computing increments to traverse it
  if (!imageData->GetPointData()->GetTensors())
    {
    return;
    }
  vtkIdType inc = imageData->GetPointData()->GetTensors()->GetNumberOfComponents();
  const int* extent = imageData->GetExtent();

  for (idx = 0; idx < 3; ++idx)
    {
    //incr[idx] = inc;
    incr[idx] = 1;
    inc *= (extent[idx*2+1] - extent[idx*2] + 1);
    }
}

//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkDiffusionTensorMathematics::ThreadedRequestData(
  vtkInformation * vtkNotUsed( request ),
  vtkInformationVector ** vtkNotUsed( inputVector ),
  vtkInformationVector * vtkNotUsed( outputVector ),
  vtkImageData ***inData,
  vtkImageData **outData,
  int outExt[6], int id)
{
  void *outPtr;

  if (inData[0][0] == nullptr)
    {
    vtkErrorMacro(<< "Input " << 0 << " must be specified.");
    return;
    }
  if (inData[0][0]->GetPointData() == nullptr || inData[0][0]->GetPointData()->GetTensors() == nullptr)
    {
    vtkErrorMacro(<< "Input " << 0 << " must have tensors. PointData: " << inData[0][0]->GetPointData());
    return;
    }

  outPtr = outData[0]->GetScalarPointerForExtent(outExt);

  // single input only for now
  vtkDebugMacro ("In Threaded Execute. scalar type is " << inData[0][0]->GetScalarType() << "op is: " << this->Operation);

  switch (this->GetOperation())
    {

      // Operations where eigenvalues are not computed
    case VTK_TENS_D11:
    case VTK_TENS_D22:
    case VTK_TENS_D33:
    case VTK_TENS_TRACE:
    case VTK_TENS_DETERMINANT:
      switch (outData[0]->GetScalarType())
      {
      // we set the output data scalar type depending on the op
      // already.  And we only access the input tensors
      // which are float.  So this switch statement on output
      // scalar type is sufficient.
      vtkTemplateMacro(vtkDiffusionTensorMathematicsExecute1(
                this,inData[0][0], outData[0],
                static_cast<VTK_TT*>(outPtr), outExt, id));
      default:
        vtkErrorMacro(<< "Execute: Unknown ScalarType");
        return;
      }
      break;

    // Operations where eigenvalues are computed
    case VTK_TENS_RELATIVE_ANISOTROPY:
    case VTK_TENS_FRACTIONAL_ANISOTROPY:
    case VTK_TENS_LINEAR_MEASURE:
    case VTK_TENS_PLANAR_MEASURE:
    case VTK_TENS_SPHERICAL_MEASURE:
    case VTK_TENS_MAX_EIGENVALUE:
    case VTK_TENS_MID_EIGENVALUE:
    case VTK_TENS_MIN_EIGENVALUE:
    case VTK_TENS_MAX_EIGENVALUE_PROJX:
    case VTK_TENS_MAX_EIGENVALUE_PROJY:
    case VTK_TENS_MAX_EIGENVALUE_PROJZ:
    case VTK_TENS_RAI_MAX_EIGENVEC_PROJX:
    case VTK_TENS_RAI_MAX_EIGENVEC_PROJY:
    case VTK_TENS_RAI_MAX_EIGENVEC_PROJZ:
    case VTK_TENS_COLOR_ORIENTATION:
    case VTK_TENS_MODE:
    case VTK_TENS_COLOR_MODE:
    case VTK_TENS_PARALLEL_DIFFUSIVITY:
    case VTK_TENS_PERPENDICULAR_DIFFUSIVITY:
    case VTK_TENS_MEAN_DIFFUSIVITY:
      switch (outData[0]->GetScalarType())
      {
        vtkTemplateMacro(vtkDiffusionTensorMathematicsExecute1Eigen(
                this,inData[0][0], outData[0],
                static_cast<VTK_TT*>(outPtr), outExt, id));
        default:
        vtkErrorMacro(<< "Execute: Unknown ScalarType");
        return;
      }
    break;
  }
}


//Fix negative Eigen with a shift
/*
int  vtkDiffusionTensorMathematics::FixNegativeEigenvaluesMethod(double w[3])
{

  if(w[2]<0) {
     w[1] += (-w[2]);
     w[0] += (-w[2]);
     w[2] = 0;
  }

  if (!((w[0]>= w[1] && w[0]>= w[2]) &&
      (w[1]>=w[2]) ))  {
        return -1;
  }
  return 0;
}
*/

int  vtkDiffusionTensorMathematics::FixNegativeEigenvaluesMethod(double w[3])
{
 int cardinality;
 double wtmp[3];
 //Check for cardinality of negative eigenvalues
 if (w[0]<0 && w[1]<0 && w[2] <0)
   cardinality = 3;
 else if(w[1]<0 && w[2]<0)
   cardinality = 2;
 else if(w[2]<0)
   cardinality = 1;
 else
   cardinality = 0;

 switch(cardinality) {
    case 3:
        w[0]=0;
        w[1]=0;
        w[2]=0;
        break;
    case 2:
        w[0]= w[0]+0.5*(w[1]+w[2]);
        if (w[0]<0)
            w[0]=0;
        w[1]=0;
        break;
    case 1:
        wtmp[0]= w[0] + 1/3*w[2];
        wtmp[1]= w[1] + 1/3*w[2];
        w[2] =0;
        if (wtmp[0]>=0 && wtmp[1]>=0)
         {
            w[0]=wtmp[0];
            w[1]=wtmp[1];
         }
        else if(wtmp[1]< 0) {
            w[0] = w[0]+0.5*(w[1]+w[2]);
            if (w[0]<0)
              w[0]=0;
            w[1] = 0;

          }
        else {
             w[0] = 0;
             w[1] = 0;
          }
        break;

     case 0:
       //Keep everything (nothing is negative)
       break;
    }

  return 0;
}

double vtkDiffusionTensorMathematics::Trace(double D[3][3])
{
  return (D[0][0] + D[1][1] + D[2][2]);

}

double vtkDiffusionTensorMathematics::Trace(double w[3])
{
  return ( w[0] + w[1] + w[2] );
}

double vtkDiffusionTensorMathematics::Determinant(double D[3][3])
{
  return vtkMath::Determinant3x3(D);
}

double vtkDiffusionTensorMathematics::RelativeAnisotropy(double w[3])
{
  double trace = w[0]+w[1]+w[2];
  trace = MAX(trace, VTK_EPS);

  return ((0.70710678)*
                (sqrt((w[0]-w[1])*(w[0]-w[1]) +
                      (w[2]-w[1])*(w[2]-w[1]) +
                      (w[2]-w[0])*(w[2]-w[0])))/trace);
}

double vtkDiffusionTensorMathematics::FractionalAnisotropy(double w[3])
{
  double norm = sqrt(w[0]*w[0]+ w[1]*w[1] +  w[2]*w[2]);
  norm = MAX(norm, VTK_EPS);

  return ((0.70710678)*
                (sqrt((w[0]-w[1])*(w[0]-w[1]) +
                      (w[2]-w[1])*(w[2]-w[1]) +
                      (w[2]-w[0])*(w[2]-w[0])))/norm);
}

double vtkDiffusionTensorMathematics::LinearMeasure(double w[3])
{
  return OUT_OF_RANGE_TO_BOUNDRY((w[0] - w[1])/MAX(w[0], VTK_EPS), 0, 1);
}

double vtkDiffusionTensorMathematics::PlanarMeasure(double w[3])
{
  return OUT_OF_RANGE_TO_BOUNDRY((w[1] - w[2]) / MAX(w[0], VTK_EPS), 0, 1);
}

double vtkDiffusionTensorMathematics::SphericalMeasure(double w[3])
{
  return OUT_OF_RANGE_TO_BOUNDRY((w[2]) / MAX(w[0], VTK_EPS), 0, 1);
}


double vtkDiffusionTensorMathematics::MaxEigenvalue(double w[3])
{
  return w[0];
}

double vtkDiffusionTensorMathematics::MiddleEigenvalue(double w[3])
{
  return w[1];
}

double vtkDiffusionTensorMathematics::MinEigenvalue(double w[3])
{
  return w[2];
}

double vtkDiffusionTensorMathematics::ParallelDiffusivity(double w[3])
{
  return w[0];
}

double vtkDiffusionTensorMathematics::PerpendicularDiffusivity(double w[3])
{
  return ( ( w[1] + w[2] ) / 2 );
}

double vtkDiffusionTensorMathematics::MeanDiffusivity(double w[3])
{
  return ( ( w[0] + w[1] + w[2] ) / 3 );
}

double vtkDiffusionTensorMathematics::RAIMaxEigenvecX(double **v, double w[3])
{
  return (fabs(v[0][0])*vtkDiffusionTensorMathematics::RelativeAnisotropy(w));
}

double vtkDiffusionTensorMathematics::RAIMaxEigenvecY(double **v, double w[3])
{
  return (fabs(v[1][0])*vtkDiffusionTensorMathematics::RelativeAnisotropy(w));
}

double vtkDiffusionTensorMathematics::RAIMaxEigenvecZ(double **v, double w[3])
{
  return (fabs(v[2][0])*vtkDiffusionTensorMathematics::RelativeAnisotropy(w));
}

double vtkDiffusionTensorMathematics::MaxEigenvecX(double **v, double vtkNotUsed(w)[3])
{
  return (fabs(v[0][0]));
}

double vtkDiffusionTensorMathematics::MaxEigenvecY(double **v, double vtkNotUsed(w)[3])
{
  return (fabs(v[1][0]));
}

double vtkDiffusionTensorMathematics::MaxEigenvecZ(double **v, double vtkNotUsed(w)[3])
{
  return (fabs(v[2][0]));
}

double vtkDiffusionTensorMathematics::MaxEigenvalueProjectionX(double **v, double w[3])
{
  return (w[0]*fabs(v[0][0]));
}

double vtkDiffusionTensorMathematics::MaxEigenvalueProjectionY(double **v, double w[3])
{
  return (w[0]*fabs(v[1][0]));
}

double vtkDiffusionTensorMathematics::MaxEigenvalueProjectionZ(double **v, double w[3])
{
  return (w[0]*fabs(v[2][0]));
}

double vtkDiffusionTensorMathematics::Mode(double w[3])
{

  // see PhD thesis, Gordon Kindlmann
  double mean = (w[0] + w[1] + w[2])/3;
  double norm = ((w[0] - mean)*(w[0] - mean) +
                  (w[1] - mean)*(w[1] - mean) +
                  (w[2] - mean)*(w[2] - mean))/3;
  norm = sqrt(norm);
  norm = norm*norm*norm;
  norm = MAX(norm, VTK_EPS);

  // multiply by sqrt 2: range from -1 to 1
  return  (M_SQRT2*((w[0] + w[1] - 2*w[2]) *
                         (2*w[0] - w[1] - w[2]) *
                         (w[0] - 2*w[1] + w[2]))/(27*norm));
}

void vtkDiffusionTensorMathematics::ColorByMode(double w[3], double &R,
                                                       double &G, double &B)
{
  // see PhD thesis, Gordon Kindlmann
  // Compute FA for amount of gray
  double fa = vtkDiffusionTensorMathematics::FractionalAnisotropy(w);
  // Compute mode
  double mode = vtkDiffusionTensorMathematics::Mode(w);
  // Calculate RGB value for this mode and FA
  vtkDiffusionTensorMathematics::ModeToRGB(mode, fa, R, G, B);
}


void vtkDiffusionTensorMathematics::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Operation: " << this->Operation << "\n";
}

// Colormap: convert our mode value (-1..1) to RGB
void vtkDiffusionTensorMathematics::ModeToRGB(double Mode, double FA,
                                     double &R, double &G, double &B)
{

   double Hue, frac;
   int sextant;

   // Mode is clamped to [-1,1]
   Mode = (Mode < -0.9999
           ? -0.9999
           : (Mode > 0.9999
              ? 0.9999
              : Mode));
   // invert mode to get desired colormap effect
   // negative (blue) up to positive (red)
   Mode = -Mode;
   // Hue is in [0, 6)
   Hue = 3*(Mode+1);
   // to avoid using last two sextants
   Hue *= 4.0/6.0;
   sextant = (int) floor(Hue);
   frac = Hue - sextant;
   switch (sextant) {
   case 0: { R = 1;      G = frac;   B = 0;      break; }
   case 1: { R = 1-frac; G = 1;      B = 0;      break; }
   case 2: { R = 0;      G = 1;      B = frac;   break; }
   case 3: { R = 0;      G = 1-frac; B = 1;      break; }
   case 4: { R = frac;   G = 0;      B = 1;      break; }
   case 5: { R = 1;      G = 0;      B = 1-frac; break; }
   }
   // FA controls lerp between gray and color
   //R = FA*R + (1-FA)*0.5;
   //G = FA*G + (1-FA)*0.5;
   //B = FA*B + (1-FA)*0.5;
   R = FA*R;
   G = FA*G;
   B = FA*B;
}

// This is sort of the inverse of code from Gordon Kindlmann for mapping
// the mode (index value) to RGB. See vtkTensorMathematics for that code.
// There may be a simpler way to do this but this works.
// Note this expects a "0 1" Hue Range in the vtkLookupTable used to
// display the glyphs.
void vtkDiffusionTensorMathematics::RGBToIndex(double R, double G,
                                         double B, double &index)
{
  if (fabs(R-G) < 0.00001 &&
      fabs(R-B) < 0.00001)
    {
    index = 0;
    return;
    }

  // remove the gray part of the color.
  // this is so we can use the model where either R,G, or B is 0.
  // then we scale so that the max of the other two is one.
  double min = R;
  int minIdx = 0;
  if (G < min)
    {
      min = G;
      minIdx = 1;
    }
  if (B < min)
    {
      min = B;
      minIdx = 2;
    }

  // make the smallest of R,G,B equal 0
  R = R - min;
  G = G - min;
  B = B - min;

  // now take the max, and scale it to be 1.
  double max = R;
  int maxIdx = 0;
  if (G > max)
    {
      max = G;
      maxIdx = 1;
    }
  if (B > max)
    {
      max = B;
      maxIdx = 2;
    }

  R = R/max;
  G = G/max;
  B = B/max;


  // now using the inverse sextants, map this into an index.
  // switch (sextant) {
  //   case 0: { R = 1;      G = frac;   B = 0;      break; }
  //   case 1: { R = 1-frac; G = 1;      B = 0;      break; }
  //   case 2: { R = 0;      G = 1;      B = frac;   break; }
  //   case 3: { R = 0;      G = 1-frac; B = 1;      break; }
  //   case 4: { R = frac;   G = 0;      B = 1;      break; }
  //   case 5: { R = 1;      G = 0;      B = 1-frac; break; }
  // }
  int sextant = 0;
  if (maxIdx == 0 && minIdx == 2) sextant = 0;
  if (maxIdx == 1 && minIdx == 2) sextant = 1;
  if (maxIdx == 1 && minIdx == 0) sextant = 2;
  if (maxIdx == 2 && minIdx == 0) sextant = 3;
  if (maxIdx == 2 && minIdx == 1) sextant = 4;
  if (maxIdx == 0 && minIdx == 1) sextant = 5;

  double offset;
  offset = 256/6;

  switch (sextant)
    {
    case 0: { index =  G*offset;     break; }
    case 1: { index = offset + (1-R)*offset;      break; }
    case 2: { index = offset*2 + B*offset;   break; }
    case 3: { index = offset*3 + (1-G)*offset;      break; }
    case 4: { index = offset*4 + R*offset;      break; }
    case 5: { index = offset*5 + (1-B)*offset; break; }
    }

}


int vtkDiffusionTensorMathematics::TeemEigenSolver(double **m, double *w, double **v)
{
    double t[7];
    double evec[9];
    double eval[3];
    int res;
    //TEN_M2T(t,m);
     t[1] = m[0][0];
     t[2] = m[0][1];
     t[3] = m[0][2];
     t[4] = m[1][1];
     t[5] = m[1][2];
     t[6] = m[2][2];

    if (v == nullptr)
        res=tenEigensolve_d(eval,nullptr,t);
    else
        res=tenEigensolve_d(eval,evec,t);

    unsigned int eval_indices[3] = {0, 1, 2};

    if (eval[0] >= eval[1] && eval[0] >= eval[2])
    {
      eval_indices[0] = 0;
      if (eval[1] >= eval[2])
      {
        eval_indices[1] = 1;
        eval_indices[2] = 2;
      }
      else
      {
        eval_indices[1] = 2;
        eval_indices[2] = 1;
      }
    }
    else if (eval[1] >= eval[0] && eval[1] >= eval[2])
    {
      eval_indices[1] = 0;
      if (eval[2] >= eval[0])
      {
        eval_indices[0] = 2;
        eval_indices[2] = 1;
      }
      else
      {
        eval_indices[0] = 1;
        eval_indices[2] = 2;
      }
    }
    else if (eval[2] >= eval[0] && eval[2] >= eval[1])
    {
      eval_indices[2] = 0;
      if (eval[1] >= eval[0])
      {
        eval_indices[0] = 2;
        eval_indices[1] = 1;
      }
      else
      {
        eval_indices[0] = 1;
        eval_indices[1] = 2;
      }
    }
    //Assigned output eigenvalues
    if (v != nullptr) {
        v[eval_indices[0]][0] = evec[0]; v[eval_indices[1]][0] = evec[1]; v[eval_indices[2]][0] = evec[2];
        v[eval_indices[0]][1] = evec[3]; v[eval_indices[1]][1] = evec[4]; v[eval_indices[2]][1] = evec[5];
        v[eval_indices[0]][2] = evec[6]; v[eval_indices[1]][2] = evec[7]; v[eval_indices[2]][2] = evec[8];
    }
    w[eval_indices[0]]=eval[0];
    w[eval_indices[1]]=eval[1];
    w[eval_indices[2]]=eval[2];
    return res;

}
