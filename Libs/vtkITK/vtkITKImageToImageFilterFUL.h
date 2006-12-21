/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

// .NAME vtkITKImageToImageFilter - Abstract base class for connecting ITK and VTK
// .SECTION Description
// vtkITKImageToImageFilter provides a 

#ifndef __vtkITKImageToImageFilterFUL_h
#define __vtkITKImageToImageFilterFUL_h

#include "vtkITKImageToImageFilter.h"
#include "vtkImageToImageFilter.h"
#include "itkImageToImageFilter.h"
#include "itkVTKImageExport.h"
#include "itkVTKImageImport.h"
#include "vtkITKUtility.h"


class VTK_ITK_EXPORT vtkITKImageToImageFilterFUL : public vtkITKImageToImageFilter
{
public:
  vtkTypeMacro(vtkITKImageToImageFilterFUL,vtkITKImageToImageFilter);
  static vtkITKImageToImageFilterFUL* New() { return 0; };
  void PrintSelf(ostream& os, vtkIndent indent)
  {
    Superclass::PrintSelf ( os, indent );
    os << m_Filter << std::endl;
  };

protected:
  //BTX
  
  // To/from ITK
  typedef itk::Image<float, 3> InputImageType;
  typedef itk::Image<unsigned long, 3> OutputImageType;

  typedef itk::VTKImageImport<InputImageType> ImageImportType;
  typedef itk::VTKImageExport<OutputImageType> ImageExportType;
  ImageImportType::Pointer itkImporter;
  ImageExportType::Pointer itkExporter;

  typedef itk::ImageToImageFilter<InputImageType,OutputImageType> GenericFilterType;
  GenericFilterType::Pointer m_Filter;

  vtkITKImageToImageFilterFUL ( GenericFilterType* filter ) : vtkITKImageToImageFilter()
  {
    // Need an import, export, and a ITK pipeline
    m_Filter = filter;
    this->itkImporter = ImageImportType::New();
    this->itkExporter = ImageExportType::New();
    ConnectPipelines(this->vtkExporter, this->itkImporter);
    ConnectPipelines(this->itkExporter, this->vtkImporter);
    // Set up the filter pipeline
    m_Filter->SetInput ( this->itkImporter->GetOutput() );
    this->itkExporter->SetInput ( m_Filter->GetOutput() );
    this->LinkITKProgressToVTKProgress ( m_Filter );
    this->vtkCast->SetOutputScalarTypeToFloat();
  };

  ~vtkITKImageToImageFilterFUL()
  {
  };
  //ETX
  
private:
  vtkITKImageToImageFilterFUL(const vtkITKImageToImageFilterFUL&);  // Not implemented.
  void operator=(const vtkITKImageToImageFilterFUL&);  // Not implemented.
};

#endif





