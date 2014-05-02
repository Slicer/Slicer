/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkITKImageToImageFilter2DFF_h
#define __vtkITKImageToImageFilter2DFF_h

#include "vtkITKImageToImageFilter.h"
#include "vtkImageAlgorithm.h"
#include "itkImageToImageFilter.h"
#include "itkVTKImageExport.h"
#include "itkVTKImageImport.h"
#include "vtkITKUtility.h"

class VTK_ITK_EXPORT vtkITKImageToImageFilter2DFF : public vtkITKImageToImageFilter
{
public:
  vtkTypeMacro(vtkITKImageToImageFilter2DFF,vtkITKImageToImageFilter);
  static vtkITKImageToImageFilter2DFF* New() { return 0; };
  void PrintSelf(ostream& os, vtkIndent indent)
  {
    Superclass::PrintSelf ( os, indent );
    os << m_Filter;
  };

protected:

  /// To/from ITK
  typedef float InputImagePixelType;
  typedef float OutputImagePixelType;
  typedef itk::Image<InputImagePixelType, 2> InputImageType;
  typedef itk::Image<OutputImagePixelType, 2> OutputImageType;

  typedef itk::VTKImageImport<InputImageType> ImageImportType;
  typedef itk::VTKImageExport<OutputImageType> ImageExportType;
  ImageImportType::Pointer itkImporter;
  ImageExportType::Pointer itkExporter;

  typedef itk::ImageToImageFilter<InputImageType,OutputImageType> GenericFilterType;
  GenericFilterType::Pointer m_Filter;

  vtkITKImageToImageFilter2DFF ( GenericFilterType* filter )
  {
    /// Need an import, export, and a ITK pipeline
    m_Filter = filter;
    this->itkImporter = ImageImportType::New();
    this->itkExporter = ImageExportType::New();
    ConnectPipelines(this->vtkExporter, this->itkImporter);
    ConnectPipelines(this->itkExporter, this->vtkImporter);
    this->LinkITKProgressToVTKProgress ( m_Filter );

    /// Set up the filter pipeline
    m_Filter->SetInput ( this->itkImporter->GetOutput() );
    this->itkExporter->SetInput ( m_Filter->GetOutput() );
    this->vtkCast->SetOutputScalarTypeToFloat ();
  };

  ~vtkITKImageToImageFilter2DFF()
  {
  };

private:
  vtkITKImageToImageFilter2DFF(const vtkITKImageToImageFilter2DFF&);  /// Not implemented.
  void operator=(const vtkITKImageToImageFilter2DFF&);  /// Not implemented.
};

#endif




