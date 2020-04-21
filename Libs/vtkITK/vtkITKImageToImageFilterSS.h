/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkITKImageToImageFilterSS_h
#define __vtkITKImageToImageFilterSS_h

#include "vtkITKImageToImageFilter.h"
#include "vtkImageAlgorithm.h"
#include "itkImageToImageFilter.h"
#include "itkVTKImageExport.h"
#include "itkVTKImageImport.h"
#include "vtkITKUtility.h"

class VTK_ITK_EXPORT vtkITKImageToImageFilterSS : public vtkITKImageToImageFilter
{
public:
  vtkTypeMacro(vtkITKImageToImageFilterSS,vtkITKImageToImageFilter);
  static vtkITKImageToImageFilterSS* New() { return nullptr; }
  void PrintSelf(ostream& os, vtkIndent indent) override
  {
    Superclass::PrintSelf ( os, indent );
    os << m_Filter;
  }

  ///
  /// Portion of the SetReleaseDataFlag implementation can be
  /// implemented at this level of the hierarchy.
  void SetReleaseDataFlag(int f) override
    {
      Superclass::SetReleaseDataFlag(f);
      m_Filter->SetReleaseDataFlag(f);
    }

protected:

  /// To/from ITK
  typedef short InputImagePixelType;
  typedef short OutputImagePixelType;
  typedef itk::Image<InputImagePixelType, 3> InputImageType;
  typedef itk::Image<OutputImagePixelType, 3> OutputImageType;

  typedef itk::VTKImageImport<InputImageType> ImageImportType;
  typedef itk::VTKImageExport<OutputImageType> ImageExportType;
  ImageImportType::Pointer itkImporter;
  ImageExportType::Pointer itkExporter;

  typedef itk::ImageToImageFilter<InputImageType,OutputImageType> GenericFilterType;
  GenericFilterType::Pointer m_Filter;

  vtkITKImageToImageFilterSS ( GenericFilterType* filter )
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
    this->vtkCast->SetOutputScalarTypeToShort();
  };

  ~vtkITKImageToImageFilterSS() override = default;

private:
  vtkITKImageToImageFilterSS(const vtkITKImageToImageFilterSS&) = delete;
  void operator=(const vtkITKImageToImageFilterSS&) = delete;
};

#endif
