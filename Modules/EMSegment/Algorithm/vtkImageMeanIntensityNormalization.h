/*=auto=========================================================================

(c) Copyright 2003 Massachusetts Institute of Technology (MIT) All Rights Reserved.

This software ("3D Slicer") is provided by The Brigham and Women's 
Hospital, Inc. on behalf of the copyright holders and contributors.
Permission is hereby granted, without payment, to copy, modify, display 
and distribute this software and its documentation, if any, for  
research purposes only, provided that (1) the above copyright notice and 
the following four paragraphs appear on all copies of this software, and 
(2) that source code to any modifications to this software be made 
publicly available under terms no more restrictive than those in this 
License Agreement. Use of this software constitutes acceptance of these 
terms and conditions.

3D Slicer Software has not been reviewed or approved by the Food and 
Drug Administration, and is for non-clinical, IRB-approved Research Use 
Only.  In no event shall data or images generated through the use of 3D 
Slicer Software be used in the provision of patient care.

IN NO EVENT SHALL THE COPYRIGHT HOLDERS AND CONTRIBUTORS BE LIABLE TO 
ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
EVEN IF THE COPYRIGHT HOLDERS AND CONTRIBUTORS HAVE BEEN ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.

THE COPYRIGHT HOLDERS AND CONTRIBUTORS SPECIFICALLY DISCLAIM ANY EXPRESS 
OR IMPLIED WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND 
NON-INFRINGEMENT.

THE SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS." THE COPYRIGHT HOLDERS AND CONTRIBUTORS HAVE NO OBLIGATION TO 
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================auto=*/
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

protected:

  vtkImageMeanIntensityNormalization();
  vtkImageMeanIntensityNormalization(const vtkImageMeanIntensityNormalization&) {};
  ~vtkImageMeanIntensityNormalization();

  void operator=(const vtkImageMeanIntensityNormalization&) {};
  
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

};
#endif



 







