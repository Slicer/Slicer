/*=========================================================================

  Program:   Slicer3
  Language:  C++
  Module:    $HeadURL: http://svn.slicer.org/Slicer3/trunk/Applications/GUI/Slicer3.cxx $
  Date:      $Date: 2009-04-15 06:29:13 -0400 (Wed, 15 Apr 2009) $
  Version:   $Revision: 9206 $

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
// .NAME vtkImageMeanIntensityNormalization

#ifndef __vtkImageMeanIntensityNormalization_h
#define __vtkImageMeanIntensityNormalization_h


#include "vtkImageToImageFilter.h"
#include "vtkImageAccumulate.h"
#include "vtkEMSegment.h"

#define INTENSITY_NORM_UNDEFINED 0
#define INTENSITY_NORM_MEAN_MRI 1

class VTK_EMSEGMENT_EXPORT vtkImageMeanIntensityNormalization : public vtkImageToImageFilter
{
  public:
  static vtkImageMeanIntensityNormalization *New();
  vtkTypeMacro(vtkImageMeanIntensityNormalization,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetMacro(NormValue,double);
  vtkGetMacro(NormValue,double);

  vtkSetMacro(NormType,int);
  vtkGetMacro(NormType,int);
  void SetTypeToNormMeanMRI() {this->NormType = INTENSITY_NORM_MEAN_MRI; }

  // Description:
  // Define initial width for smoothing the intensity histogram, e.g. 2 => smoothing width = numober of voxels / 2;
  vtkSetMacro(InitialHistogramSmoothingWidth,int);
  vtkGetMacro(InitialHistogramSmoothingWidth,int);

  // Description:
  // Define maximum width for smoothing the intensity histogram, e.g. 5 => smoothing width = numober of voxels / 5;
  vtkSetMacro(MaxHistogramSmoothingWidth,int);
  vtkGetMacro(MaxHistogramSmoothingWidth,int);

  // Description:
  // Define percentage of voxels that should be included in intensity normalization
  // Traditionally the value is 0.99
  // However, some T2 have a large hump at the end so it should be set to 95%
  vtkSetMacro(RelativeMaxVoxelNum,float);
  vtkGetMacro(RelativeMaxVoxelNum,float);

  vtkSetMacro(PrintInfo,int);
  vtkGetMacro(PrintInfo,int);

  // Description:
  // If an Error is detected during execution the flag = true otherwise flag = false
  vtkGetMacro(ErrorExecutionFlag,bool);

protected:

  vtkImageMeanIntensityNormalization();
  ~vtkImageMeanIntensityNormalization();

  // When it works on parallel machines use : 
  //  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,int outExt[6], int id);
  // If you do not want to have it multi threaded 
  void ExecuteData(vtkDataObject *);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};
  void ExecuteInformation(vtkImageData *inData,vtkImageData *outData);
  void ComputeInputUpdateExtent(int inExt[6], int outExt[6]);

  // Core function
  void MeanMRI(vtkImageData *Input, vtkImageData *Output);
  // Functions called from MeanMRI
  int* InitializeHistogram(vtkImageAccumulate *HIST, vtkImageData *Input, int &HistMin, int &HistMax);
  int  DetermineFilterMax(int *HIST_Ptr, const int HIST_Length, const int FilterNumMaxVoxels);
  int  DetermineFilterMin(const int* HIST_PTR, const int HIST_Length);
  // Functions needed to determine lower bound of filter (DetermineFilterMin)
  int  DetermineFirstValey(const int *SmoothHistogram,const int SmoothHistogramLength);
  void SmoothHistogram(const int* Input , const int InputLength, const int SmoothWidth, int &MaxIndex, int* Output);

  double NormValue; 
  int NormType; 
  int InitialHistogramSmoothingWidth;
  int MaxHistogramSmoothingWidth;
  float RelativeMaxVoxelNum;
  int PrintInfo;
  bool ErrorExecutionFlag;

private:
         vtkImageMeanIntensityNormalization(const vtkImageMeanIntensityNormalization&); // Not implemented
         void operator=(const vtkImageMeanIntensityNormalization&); // Not implemented
       
};
#endif



 







