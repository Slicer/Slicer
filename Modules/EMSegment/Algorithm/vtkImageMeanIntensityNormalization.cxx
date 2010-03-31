/*=========================================================================

  Program:   Slicer3
  Language:  C++
  Module:    $HeadURL: http://svn.slicer.org/Slicer3/trunk/Applications/GUI/Slicer3.cxx $
  Date:      $Date: 2009-04-15 06:29:13 -0400 (Wed, 15 Apr 2009) $
  Version:   $Revision: 9206 $

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "vtkImageMeanIntensityNormalization.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkImageMathematics.h"
#include <assert.h>

//------------------------------------------------------------------------------
vtkImageMeanIntensityNormalization* vtkImageMeanIntensityNormalization::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageMeanIntensityNormalization");
  if(ret) return (vtkImageMeanIntensityNormalization*)ret;
  // If the factory was unable to create the object, then create it here.
  return new vtkImageMeanIntensityNormalization;
}

//----------------------------------------------------------------------------
vtkImageMeanIntensityNormalization::vtkImageMeanIntensityNormalization()
{
  this->NormValue = -1; 
  this->NormType  = INTENSITY_NORM_UNDEFINED; 
  this->InitialHistogramSmoothingWidth = 1;
  this->MaxHistogramSmoothingWidth     = -1;
  this->RelativeMaxVoxelNum            = -1;
  this->PrintInfo = 1;
  this->ErrorExecutionFlag = false;
}

vtkImageMeanIntensityNormalization::~vtkImageMeanIntensityNormalization(){ }

//----------------------------------------------------------------------------
void vtkImageMeanIntensityNormalization::ComputeInputUpdateExtent(int inExt[6], int vtkNotUsed(outExt)[6])
{
  this->GetInput()->GetWholeExtent(inExt);
}

//----------------------------------------------------------------------------
void vtkImageMeanIntensityNormalization::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

int* vtkImageMeanIntensityNormalization::InitializeHistogram(vtkImageAccumulate *HIST, vtkImageData *Input, int &HistMin, int &HistMax)
{
  // 1. Detect extrema 
  HIST->SetInput(Input);
  HIST->SetComponentSpacing(1,1,1);
  HIST->SetComponentOrigin(0,0,0);
  HIST->Update();
  
  // 2. Compute Histogram
  double EXTREMA[3];
  HIST->GetMin(EXTREMA);
  HistMin = int(EXTREMA[0]);
  HIST->GetMax(EXTREMA);
  HistMax = int(ceil(EXTREMA[0]));

  // Here is a change from the original 
  int EXTENT[6] = {HistMin,HistMax,0,0,0,0};
  HIST->SetComponentExtent(EXTENT);
  HIST->Update();
    
  HIST->GetOutput()->GetWholeExtent(EXTENT);
  return (int*)(HIST->GetOutput()->GetScalarPointerForExtent(EXTENT));
}

// Determines the index in the Histogram at which point the sum over the number of voxels with lower or equal the index is smaller than NumMaxVoxel 
// e.g. 99 % of all voxels  
int vtkImageMeanIntensityNormalization::DetermineFilterMax(int *HIST_Ptr, const int HIST_Length, const int FilterNumMaxVoxels)
{
  int VoxelCount =0;
  int FilterHistogramMax = 0;
    
  while ((FilterHistogramMax < HIST_Length) && (VoxelCount <  FilterNumMaxVoxels))
    {
    VoxelCount += (*HIST_Ptr ++); 
    FilterHistogramMax ++;
    }
  return FilterHistogramMax;
}

void vtkImageMeanIntensityNormalization::SmoothHistogram(const int* Input , const int InputLength, const int SmoothWidth, int &MaxIndex, int* Output)
{
  float InverseSmoothWidth = 1.0 / float(SmoothWidth);
  MaxIndex =  InputLength - SmoothWidth;
  memset(Output,0, sizeof(int)*MaxIndex);

  for (int k = 0; k <= SmoothWidth; k++ )
    {
    *Output += *Input;
    for (int i = 1; i <= k ; i++) Output[i] += *Input;
    Input ++;
    }
  *Output = int(*Output * InverseSmoothWidth);
  Output++;

  for (int x = 1; x < MaxIndex; x ++ )
    {
    for (int i = 0; i <= SmoothWidth ; i++) Output[i] += *Input;
    *Output = int(*Output * InverseSmoothWidth);
    Output++;
    Input++; 
    }
}

int vtkImageMeanIntensityNormalization::DetermineFilterMin(const int* HIST_PTR, const int HIST_Length)
{

  int *HIST_SMOOTH_Ptr    = new int[ HIST_Length];

  int iter = 1;
  int result = -1;
  int SmoothWidthPara = this->InitialHistogramSmoothingWidth; 
  while ((SmoothWidthPara <= this->MaxHistogramSmoothingWidth) && (result < 0))
    { 
    int SmoothWidth = HIST_Length / SmoothWidthPara;
    int HIST_SMOOTH_Length;
    if (this->PrintInfo)
      {
      std::cerr << "  " << iter << ". Histogram Smoothing" << endl;
      std::cerr << "     Absolute Width:         " << SmoothWidth  << endl;
      iter++;
      }
    
    this->SmoothHistogram(HIST_PTR,HIST_Length, SmoothWidth, HIST_SMOOTH_Length, HIST_SMOOTH_Ptr);
    result = this->DetermineFirstValey(HIST_SMOOTH_Ptr,HIST_SMOOTH_Length);
    SmoothWidthPara ++;
    }
  if (result < 0)
    { 
    vtkWarningMacro(<< "Lower bound of image could not properly derermined");
    result = 0;
    }

  delete[] HIST_SMOOTH_Ptr;
  return result;
}


int vtkImageMeanIntensityNormalization::DetermineFirstValey(const int *aSmoothHistogram,const int aSmoothHistogramLength)
{
  // Define the lower intensity value for calculating the mean of the historgram
  // - When through is set we reached the first minimum after the first peak which defines the lower bound of the intensity 
  //   value considered for calculating the Expected value of the histogram 

  int LowerBound = 0;    
  // Detect first peak
  // => We considere this area noise (or background) and therefore exclude it for the definition of the normalization factor  
  while ((LowerBound < aSmoothHistogramLength) && (aSmoothHistogram[LowerBound] <=  aSmoothHistogram[LowerBound + 1])) LowerBound++;
  if (LowerBound >= aSmoothHistogramLength -3)
    {
    vtkErrorMacro(<< "Lower bound of image could not properly derermined");
    return -1;
    }

  LowerBound++;
  // Detect next valey:
  // Sefines the lower bound of the intensity value considered for calculating the Expected value of the histogram 
  int MaxIndex = aSmoothHistogramLength - 2;
  while ((LowerBound < MaxIndex) && 
         (aSmoothHistogram[LowerBound] >=  aSmoothHistogram[LowerBound + 1] 
          || aSmoothHistogram[LowerBound+1] >= aSmoothHistogram[LowerBound+2] 
          || aSmoothHistogram[LowerBound+2] >= aSmoothHistogram[LowerBound+3])) 
    {
    LowerBound++;
    }

  if  (LowerBound == MaxIndex)
    {
    return -1;
    }
  return LowerBound;
}


void vtkImageMeanIntensityNormalization::MeanMRI(vtkImageData *Input, vtkImageData *Output)
{

  // -------------------------------------
  // Check if input is set correctly 
  // -------------------------------------
  if (this->InitialHistogramSmoothingWidth > this->MaxHistogramSmoothingWidth) 
    {
      vtkErrorMacro(<< "HistogramSmoothingWidth is not set correctly: Initial (" << this->InitialHistogramSmoothingWidth <<") is larger than Max (" <<  this->MaxHistogramSmoothingWidth<<")");
      this->ErrorExecutionFlag = true;
      return;
    }

  if ((this->RelativeMaxVoxelNum <= 0) || (this->RelativeMaxVoxelNum  > 1))
    {
    vtkErrorMacro(<< "RelativeMaxIntensityValue is not set correctly");
    this->ErrorExecutionFlag = true;
    return;
    }

  // -------------------------------------
  //  Initialize Variables
  // -------------------------------------

  // Image value
  int ImageIntensityMin;
  int ImageIntensityMax;
  double ImageIntensityMean;
  double ImageIntensityCorrectionRatio;

  int NumVoxels;

  // Filter Values
  int    FilterHistogramMax;
  int    FilterHistogramMin;
  vtkImageAccumulate *HIST = NULL;
  vtkImageMathematics *CORRECTED = NULL;
  int* HIST_PTR;

  {
  int INPUT_EXTENT[6];
  Input->GetExtent(INPUT_EXTENT);
  NumVoxels = (INPUT_EXTENT[1] - INPUT_EXTENT[0] + 1) * (INPUT_EXTENT[3] - INPUT_EXTENT[2] + 1) * (INPUT_EXTENT[5] - INPUT_EXTENT[4] + 1);
  }

  // Define Histogram
  HIST = vtkImageAccumulate::New();
  HIST_PTR = this->InitializeHistogram(HIST,Input, ImageIntensityMin, ImageIntensityMax);

  if (this->PrintInfo)
    {
    std::cerr << "vtkImageMeanIntensityNormalization::MeanMRI " << endl;
    std::cerr << "Histogram Parameters:" << endl;
    std::cerr << "  Image Intensity Min: " <<  ImageIntensityMin << " Max: " << ImageIntensityMax << endl;
    std::cerr << "  Initial Histogram Smoothig Width: " << this->InitialHistogramSmoothingWidth << endl;
    std::cerr << "  Maximum Histogram Smoothig Width: " << this->MaxHistogramSmoothingWidth << endl;
    }

  // Go through Histogram and detect intensity value which combines NumVoxels * this->RelativeMaxVoxelNum
  // => Cut of the tail of the the histogram
  FilterHistogramMax = this->DetermineFilterMax(HIST_PTR, ImageIntensityMax -  ImageIntensityMin +1 , int(NumVoxels * this->RelativeMaxVoxelNum));

  // Determine Lower Bound of filter based on Second peak
  // In some images the second peak is too close to the first so that the smoothing width is too large and smoothes over the drop 
  // => the peak is not found 
  FilterHistogramMin = this->DetermineFilterMin(HIST_PTR,FilterHistogramMax+1);

  // -------------------------------------
  //  Compute Mean
  // -------------------------------------
  ImageIntensityMean = 0;
  NumVoxels = 0;
  HIST_PTR += FilterHistogramMin;
  for (int x = FilterHistogramMin; x <= FilterHistogramMax; x++)
    {
    int val = *HIST_PTR ++;
    // we have to add ImageIntensityMin bc we do not start histogram anymore at 0 !
    ImageIntensityMean += double(x + ImageIntensityMin)*val;
    NumVoxels += val;
    }
  ImageIntensityMean = ImageIntensityMean / double(NumVoxels);
  assert(ImageIntensityMean);
  ImageIntensityCorrectionRatio = NormValue/ImageIntensityMean;
 
  // -------------------------------------
  //  Correct Image 
  // -------------------------------------
  CORRECTED = vtkImageMathematics::New();
  CORRECTED->SetInput(0,Input);
  CORRECTED->SetConstantK(ImageIntensityCorrectionRatio);
  CORRECTED->SetOperationToMultiplyByK();
  CORRECTED->Update();
  Output->DeepCopy(CORRECTED->GetOutput());

  if (this->PrintInfo)
    {
    std::cerr << "Bounds for Expected Value Calculation:" << endl;
    std::cerr << "  Lower Bound: " << FilterHistogramMin + ImageIntensityMin << endl;
    std::cerr << "  Upper Bound: " << FilterHistogramMax + ImageIntensityMin << endl;
    std::cerr << "Results of Filter:" << endl;
    std::cerr << "  Expect Image Intensity: " << ImageIntensityMean << endl;
    std::cerr << "  Normalization Factor:   " << ImageIntensityCorrectionRatio << endl;
    }

  // -------------------------------------
  //  Clean up  
  // -------------------------------------
  CORRECTED->Delete();    
  HIST->Delete();
}


// To chage anything about output us this executed before Thread
//----------------------------------------------------------------------------
void vtkImageMeanIntensityNormalization::ExecuteInformation(vtkImageData *inData, vtkImageData *outData) 
{
  outData->SetOrigin(inData->GetOrigin());
  outData->SetNumberOfScalarComponents(1);
  outData->SetWholeExtent(inData->GetWholeExtent());
  outData->SetSpacing(inData->GetSpacing());
  outData->SetScalarType(inData->GetScalarType());
}

//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.

void vtkImageMeanIntensityNormalization::ExecuteData(vtkDataObject *)
{
  int inExt[6];
  int outExt[6];
  // Necessary  for VTK
  this->ComputeInputUpdateExtent(inExt,outExt);
  // vtk4
  vtkImageData *inData  = this->GetInput();
  vtkImageData *outData = this->GetOutput();
  outData->SetExtent(this->GetOutput()->GetWholeExtent());
  outData->AllocateScalars();
  outData->GetWholeExtent(outExt);

  // vtk4
  vtkDebugMacro(<< "Execute: inData = " << inData << ", outData = " << outData);
 
  if (inData == NULL)
    {
    vtkErrorMacro(<< "Input " << 0 << " must be specified.");
    this->ErrorExecutionFlag = true;
    return;
    }

  if (inData->GetNumberOfScalarComponents() != 1)
    {
    vtkErrorMacro(<< "Number Of Scalar Components for Input has to be 1.");
    this->ErrorExecutionFlag = true;
    return;
    }

  switch (this->NormType)
    {
    case INTENSITY_NORM_MEAN_MRI :  this->MeanMRI(inData,outData); break;
    default:
      vtkErrorMacro(<< "Execute: Unknown Normalization Type");
      this->ErrorExecutionFlag = true;
      return;
    }
}
