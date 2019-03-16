/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkITKNewOtsuThresholdImageFilter.h,v $
  Date:      $Date: 2006/03/16 20:00:18 $
  Version:   $Revision: 1.1.2.1 $

=========================================================================auto=*/

#ifndef __vtkITKNewOtsuThresholdImageFilter_h
#define __vtkITKNewOtsuThresholdImageFilter_h

#include "vtkITK.h"

#include "vtkITKImageToImageFilterSS.h"
#include "itkNewOtsuThresholdImageFilter.h"
#include "vtkObjectFactory.h"

/// \brief Wrapper class around itk::NewOtsuThresholdImageFilter.
class VTK_ITK_EXPORT vtkITKNewOtsuThresholdImageFilter
  : public vtkITKImageToImageFilterSS
{
public:
  static vtkITKNewOtsuThresholdImageFilter *New();
  vtkTypeMacro(vtkITKNewOtsuThresholdImageFilter, vtkITKImageToImageFilterSS);

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
  typedef itk::NewOtsuThresholdImageFilter<Superclass::InputImageType, Superclass::OutputImageType> ImageFilterType;
  vtkITKNewOtsuThresholdImageFilter() : Superclass ( ImageFilterType::New() ){};
  ~vtkITKNewOtsuThresholdImageFilter() override  = default;
  ImageFilterType* GetImageFilterPointer() { return dynamic_cast<ImageFilterType*> ( m_Filter.GetPointer() ); }

private:
  vtkITKNewOtsuThresholdImageFilter(const vtkITKNewOtsuThresholdImageFilter&) = delete;
  void operator=(const vtkITKNewOtsuThresholdImageFilter&) = delete;
};

//vtkStandardNewMacro(vtkITKNewOtsuThresholdImageFilter);

#endif




