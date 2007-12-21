/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkITKNewOtsuThresholdImageFilter.h,v $
  Date:      $Date: 2006/03/16 20:00:18 $
  Version:   $Revision: 1.1.2.1 $

=========================================================================auto=*/
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkITKNewOtsuThresholdImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2006/03/16 20:00:18 $
  Version:   $Revision: 1.1.2.1 $
*/
// .NAME vtkITKNewOtsuThresholdImageFilter - Wrapper class around itk::NewOtsuThresholdImageFilter
// .SECTION Description
// vtkITKNewOtsuThresholdImageFilter


#ifndef __vtkITKNewOtsuThresholdImageFilter_h
#define __vtkITKNewOtsuThresholdImageFilter_h

#include "vtkITK.h"

#include "vtkITKImageToImageFilterSS.h"
#include "itkNewOtsuThresholdImageFilter.h"
#include "vtkObjectFactory.h"

class VTK_ITK_EXPORT vtkITKNewOtsuThresholdImageFilter : public vtkITKImageToImageFilterSS
{
 public:
  static vtkITKNewOtsuThresholdImageFilter *New();
  vtkTypeRevisionMacro(vtkITKNewOtsuThresholdImageFilter, vtkITKImageToImageFilterSS);
  
  void SetNumberOfHistogramBins( unsigned long value) 
  {
    DelegateITKInputMacro ( SetNumberOfHistogramBins, value );
  };
  unsigned long GetNumberOfHistogramBins ()
  { DelegateITKOutputMacro ( GetNumberOfHistogramBins ); };
  
  void SetOmega (double value)
  {
    DelegateITKInputMacro( SetOmega, value);
   }; 
  
  double GetOmega()
  {
    DelegateITKOutputMacro( GetOmega );
  }; 
  
  void SetInsideValue (short value)
  { 
    OutputImagePixelType d = static_cast<OutputImagePixelType> ( value );
    DelegateITKInputMacro (SetInsideValue,d);
  }
  
  void SetOutsideValue (short value)
  { 
    OutputImagePixelType d = static_cast<OutputImagePixelType> ( value );
    DelegateITKInputMacro (SetOutsideValue,d);
  }
  
  short GetInsideValue ()
  { DelegateITKOutputMacro ( GetInsideValue ); };
  
  short GetOutsideValue ()
  { DelegateITKOutputMacro ( GetOutsideValue ); };
 
  short GetThreshold()
  { DelegateITKOutputMacro ( GetThreshold ); };

protected:
  //BTX
  typedef itk::NewOtsuThresholdImageFilter<Superclass::InputImageType, Superclass::OutputImageType> ImageFilterType;
  vtkITKNewOtsuThresholdImageFilter() : Superclass ( ImageFilterType::New() ){};
  ~vtkITKNewOtsuThresholdImageFilter() {};
  ImageFilterType* GetImageFilterPointer() { return dynamic_cast<ImageFilterType*> ( m_Filter.GetPointer() ); }
  //ETX
  
private:
  vtkITKNewOtsuThresholdImageFilter(const vtkITKNewOtsuThresholdImageFilter&);  // Not implemented.
  void operator=(const vtkITKNewOtsuThresholdImageFilter&);  // Not implemented.
};

//vtkCxxRevisionMacro(vtkITKNewOtsuThresholdImageFilter, "$Revision: 1.1.2.1 $");
//vtkStandardNewMacro(vtkITKNewOtsuThresholdImageFilter);

#endif




