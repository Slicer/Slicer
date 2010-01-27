/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkImageToVTKImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2006-09-06 20:58:41 $
  Version:   $Revision: 1.1 $

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkImageToVTKImageFilter_txx
#define _itkImageToVTKImageFilter_txx

#include "itkImageToVTKImageFilter.h"

namespace itk
{



/**
 * Constructor
 */
template <class TInputImage>
ImageToVTKImageFilter<TInputImage>
::ImageToVTKImageFilter()
{

  m_Importer = vtkImageImport::New();

  m_Exporter = ExporterFilterType::New();

  m_Importer->SetUpdateInformationCallback(m_Exporter->GetUpdateInformationCallback());
  m_Importer->SetPipelineModifiedCallback(m_Exporter->GetPipelineModifiedCallback());
  m_Importer->SetWholeExtentCallback(m_Exporter->GetWholeExtentCallback());
  m_Importer->SetSpacingCallback(m_Exporter->GetSpacingCallback());
  m_Importer->SetOriginCallback(m_Exporter->GetOriginCallback());
  m_Importer->SetScalarTypeCallback(m_Exporter->GetScalarTypeCallback());
  m_Importer->SetNumberOfComponentsCallback(m_Exporter->GetNumberOfComponentsCallback());
  m_Importer->SetPropagateUpdateExtentCallback(m_Exporter->GetPropagateUpdateExtentCallback());
  m_Importer->SetUpdateDataCallback(m_Exporter->GetUpdateDataCallback());
  m_Importer->SetDataExtentCallback(m_Exporter->GetDataExtentCallback());
  m_Importer->SetBufferPointerCallback(m_Exporter->GetBufferPointerCallback());
  m_Importer->SetCallbackUserData(m_Exporter->GetCallbackUserData());

}




/**
 * Destructor
 */
template <class TInputImage>
ImageToVTKImageFilter<TInputImage>
::~ImageToVTKImageFilter()
{
  if( m_Importer )
    {
    m_Importer->Delete();
    m_Importer = 0;
    }
}



/**
 * Set an itk::Image as input 
 */
template <class TInputImage>
void
ImageToVTKImageFilter<TInputImage>
::SetInput( const InputImageType * inputImage )
{
  m_Exporter->SetInput( inputImage );
}



/**
 * Get a vtkImage as output
 */
template <class TInputImage>
vtkImageData *
ImageToVTKImageFilter<TInputImage>
::GetOutput() const
{
  return m_Importer->GetOutput();
}




/**
 * Get the importer filter
 */
template <class TInputImage>
vtkImageImport *
ImageToVTKImageFilter<TInputImage>
::GetImporter() const
{
  return m_Importer;
}



/**
 * Get the exporter filter
 */
template <class TInputImage>
typename ImageToVTKImageFilter<TInputImage>::ExporterFilterType *
ImageToVTKImageFilter<TInputImage>
::GetExporter() const
{
  return m_Exporter.GetPointer();
}



/**
 * Delegate the Update to the importer
 */
template <class TInputImage>
void
ImageToVTKImageFilter<TInputImage>
::Update()
{
  m_Importer->Update();
}





} // end namespace itk

#endif

