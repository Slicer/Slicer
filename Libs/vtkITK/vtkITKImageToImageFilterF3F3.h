/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkITKImageToImageFilterF3F3_h
#define __vtkITKImageToImageFilterF3F3_h

#include "vtkITKImageToImageFilter.h"
#include "vtkImageAlgorithm.h"
#include "itkImageToImageFilter.h"
#include "itkVTKImageExport.h"
#include "itkVTKImageImport.h"
#include "vtkITKUtility.h"

class VTK_ITK_EXPORT vtkITKImageToImageFilterF3F3 : public vtkITKImageToImageFilter
{
public:
  vtkTypeMacro(vtkITKImageToImageFilterF3F3,vtkITKImageToImageFilter);
  static vtkITKImageToImageFilterF3F3* New() { return 0; };
  void PrintSelf(ostream& os, vtkIndent indent)
  {
    Superclass::PrintSelf ( os, indent );
    os << m_Filter;
  };

protected:

  /// To/from ITK
  typedef itk::Vector<float,3> InputImagePixelType;
  typedef itk::Vector<float,3> OutputImagePixelType;
  typedef itk::Image<InputImagePixelType, 3> InputImageType;
  typedef itk::Image<OutputImagePixelType, 3> OutputImageType;

  typedef itk::VTKImageImport<InputImageType> ImageImportType;
  typedef itk::VTKImageExport<OutputImageType> ImageExportType;
  ImageImportType::Pointer itkImporter;
  ImageExportType::Pointer itkExporter;

  typedef itk::ImageToImageFilter<InputImageType,OutputImageType> GenericFilterType;
  GenericFilterType::Pointer m_Filter;

  vtkITKImageToImageFilterF3F3 ( GenericFilterType* filter )
  {
    /// Need an import, export, and a ITK pipeline
    m_Filter = filter;
    this->itkImporter = ImageImportType::New();
    this->itkExporter = ImageExportType::New();
    ConnectPipelines(this->vtkExporter, this->itkImporter);
    ConnectPipelines(this->itkExporter, this->vtkImporter);
    this->LinkITKProgressToVTKProgress ( m_Filter );

    /// Set up the filter pipeline
    /// m_Filter->SetInput ( this->itkImporter->GetOutput() );
    this->itkExporter->SetInput ( m_Filter->GetOutput() );
  };

  ~vtkITKImageToImageFilterF3F3()
  {
  };

private:
  vtkITKImageToImageFilterF3F3(const vtkITKImageToImageFilterF3F3&);  /// Not implemented.
  void operator=(const vtkITKImageToImageFilterF3F3&);  /// Not implemented.
};

#endif
