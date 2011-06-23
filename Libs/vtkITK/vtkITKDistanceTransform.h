/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

///  vtkITKDistanceTransform - Wrapper class around itk::SignedMaurerDistanceMapImageFilter
/// 
/// vtkITKDistanceTransform


#ifndef __vtkITKDistanceTransform_h
#define __vtkITKDistanceTransform_h


#include "vtkITK.h"
#include "vtkSimpleImageToImageFilter.h"

class VTK_ITK_EXPORT vtkITKDistanceTransform : public vtkSimpleImageToImageFilter
{
 public:
  static vtkITKDistanceTransform *New();
  vtkTypeRevisionMacro(vtkITKDistanceTransform, vtkSimpleImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Output volume contains square of distance or actual distance
  vtkGetMacro(SquaredDistance, int);
  vtkSetMacro(SquaredDistance, int);

  /// Pixels inside region positive
  vtkGetMacro(InsideIsPositive, int);
  vtkSetMacro(InsideIsPositive, int);

  /// Use image spacing when calculating distances
  vtkGetMacro(UseImageSpacing, int);
  vtkSetMacro(UseImageSpacing, int);

  /// Value of background
  vtkGetMacro(BackgroundValue, double);
  vtkSetMacro(BackgroundValue, double);


protected:
  vtkITKDistanceTransform();
  ~vtkITKDistanceTransform();

  virtual void SimpleExecute(vtkImageData* input, vtkImageData* output);

  int SquaredDistance;
  int InsideIsPositive;
  int UseImageSpacing;
  double BackgroundValue;
  
private:
  vtkITKDistanceTransform(const vtkITKDistanceTransform&);  /// Not implemented.
  void operator=(const vtkITKDistanceTransform&);  /// Not implemented.
};

#endif




