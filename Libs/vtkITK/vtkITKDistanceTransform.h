/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

// .NAME vtkITKDistanceTransform - Wrapper class around itk::SignedMaurerDistanceMapImageFilter
// .SECTION Description
// vtkITKDistanceTransform


#ifndef __vtkITKDistanceTransform_h
#define __vtkITKDistanceTransform_h


#include "vtkITKImageToImageFilterUSF.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"

class VTK_ITK_EXPORT vtkITKDistanceTransform : public vtkITKImageToImageFilterUSF
{
 public:
  static vtkITKDistanceTransform *New();
  vtkTypeRevisionMacro(vtkITKDistanceTransform, vtkITKImageToImageFilterUSF);

  bool GetSquaredDistance ()
  {
    DelegateITKOutputMacro(GetSquaredDistance) ;
  };

  void SetSquaredDistance ( bool value )
  {
    DelegateITKInputMacro ( SetSquaredDistance, value );
  };

  bool GetInsideIsPositive ()
  {
    DelegateITKOutputMacro(GetInsideIsPositive) ;
  };

  void SetInsideIsPositive ( bool value )
  {
    DelegateITKInputMacro ( SetInsideIsPositive, value );
  };

  bool GetUseImageSpacing ()
  {
    DelegateITKOutputMacro(GetUseImageSpacing) ;
  };

  void SetUseImageSpacing ( bool value )
  {
    DelegateITKInputMacro ( SetUseImageSpacing, value );
  };

  float GetBackgroundValue ()
  {
    DelegateITKOutputMacro(GetBackgroundValue) ;
  };

  void SetBackgroundValue ( bool value )
  {
    DelegateITKInputMacro ( SetBackgroundValue, value );
  };


protected:
  //BTX
  typedef itk::SignedMaurerDistanceMapImageFilter<Superclass::InputImageType,Superclass::OutputImageType> ImageFilterType;
  vtkITKDistanceTransform() : Superclass ( ImageFilterType::New() ){};
  ~vtkITKDistanceTransform() {};
  ImageFilterType* GetImageFilterPointer() { return dynamic_cast<ImageFilterType*> ( m_Filter.GetPointer() ); }

  //ETX
  
private:
  vtkITKDistanceTransform(const vtkITKDistanceTransform&);  // Not implemented.
  void operator=(const vtkITKDistanceTransform&);  // Not implemented.
};

#endif




