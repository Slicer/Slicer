/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkITKImageToImageFilterFF_h
#define __vtkITKImageToImageFilterFF_h

#include "vtkITKImageToImageFilter.h"
#include "vtkImageAlgorithm.h"
#include "itkImageToImageFilter.h"
#include "itkVTKImageExport.h"
#include "itkVTKImageImport.h"
#include "vtkITKUtility.h"

class VTK_ITK_EXPORT vtkITKImageToImageFilterFF : public vtkITKImageToImageFilter
{
public:
  vtkTypeMacro(vtkITKImageToImageFilterFF,vtkITKImageToImageFilter);
  static vtkITKImageToImageFilterFF* New() { return nullptr; };
  void PrintSelf(ostream& os, vtkIndent indent) override
  {
    Superclass::PrintSelf ( os, indent );
    os << m_Filter;
  };

protected:

  /// To/from ITK
  using InputImagePixelType = float;
  using OutputImagePixelType = float;
  using InputImageType = itk::Image<InputImagePixelType, 3>;
  using OutputImageType = itk::Image<OutputImagePixelType, 3>;

  using ImageImportType = itk::VTKImageImport<InputImageType>;
  using ImageExportType = itk::VTKImageExport<OutputImageType>;
  ImageImportType::Pointer itkImporter;
  ImageExportType::Pointer itkExporter;

  using GenericFilterType = itk::ImageToImageFilter<InputImageType, OutputImageType>;
  GenericFilterType::Pointer m_Filter;

  vtkITKImageToImageFilterFF ( GenericFilterType* filter )
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
    this->vtkCast->SetOutputScalarTypeToFloat();
  };

  ~vtkITKImageToImageFilterFF() override
   = default;

private:
  vtkITKImageToImageFilterFF(const vtkITKImageToImageFilterFF&) = delete;
  void operator=(const vtkITKImageToImageFilterFF&) = delete;
};

#endif
