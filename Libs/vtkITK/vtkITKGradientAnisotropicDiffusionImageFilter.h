/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

// .NAME vtkITKGradientAnisotropicDiffusionImageFilter - Wrapper class around itk::GradientAnisotropicDiffusionImageFilterImageFilter
// .SECTION Description
// vtkITKGradientAnisotropicDiffusionImageFilter


#ifndef __vtkITKGradientAnisotropicDiffusionImageFilter_h
#define __vtkITKGradientAnisotropicDiffusionImageFilter_h


#include "vtkITKImageToImageFilterFF.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"

class VTK_ITK_EXPORT vtkITKGradientAnisotropicDiffusionImageFilter : public vtkITKImageToImageFilterFF
{
 public:
  static vtkITKGradientAnisotropicDiffusionImageFilter *New();
  vtkTypeRevisionMacro(vtkITKGradientAnisotropicDiffusionImageFilter, vtkITKImageToImageFilterFF);

  double GetTimeStep ()
  {
    DelegateITKOutputMacro(GetTimeStep) ;
  };

  double GetConductanceParameter ()
  {
    DelegateITKOutputMacro(GetConductanceParameter) ;
  };

  unsigned int GetNumberOfIterations ()
  {
#if (ITK_VERSION_MAJOR == 1 && ITK_VERSION_MINOR == 2 && ITK_VERSION_PATCH == 0)
    DelegateITKOutputMacro ( GetIterations );
#else
    DelegateITKOutputMacro ( GetNumberOfIterations );
#endif
  };

  void SetNumberOfIterations( unsigned int value )
  {
#if (ITK_VERSION_MAJOR == 1 && ITK_VERSION_MINOR == 2 && ITK_VERSION_PATCH == 0)
    DelegateITKInputMacro ( SetIterations, value );
#else
    DelegateITKInputMacro ( SetNumberOfIterations, value );
#endif
  };

  void SetTimeStep ( double value )
  {
    DelegateITKInputMacro ( SetTimeStep, value );
  };

  void SetConductanceParameter ( double value )
  {
    DelegateITKInputMacro ( SetConductanceParameter, value );
  };

protected:
  //BTX
  typedef itk::GradientAnisotropicDiffusionImageFilter<Superclass::InputImageType,Superclass::InputImageType> ImageFilterType;
  vtkITKGradientAnisotropicDiffusionImageFilter() : Superclass ( ImageFilterType::New() ){};
  ~vtkITKGradientAnisotropicDiffusionImageFilter() {};
  ImageFilterType* GetImageFilterPointer() { return dynamic_cast<ImageFilterType*> ( m_Filter.GetPointer() ); }

  //ETX
  
private:
  vtkITKGradientAnisotropicDiffusionImageFilter(const vtkITKGradientAnisotropicDiffusionImageFilter&);  // Not implemented.
  void operator=(const vtkITKGradientAnisotropicDiffusionImageFilter&);  // Not implemented.
};

#endif




