/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkITKUtility_h
#define __vtkITKUtility_h


#include "vtkObjectFactory.h"
#include "vtkSetGet.h"

/**
 * This function will connect the given itk::VTKImageExport filter to
 * the given vtkImageImport filter.
 */
template <typename ITK_Exporter, typename VTK_Importer>
void ConnectPipelines(ITK_Exporter exporter, VTK_Importer* importer)
{
  importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());
  importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
  importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
  importer->SetSpacingCallback(exporter->GetSpacingCallback());
  importer->SetOriginCallback(exporter->GetOriginCallback());
  importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());
  importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());
  importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
  importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
  importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
  importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
  importer->SetCallbackUserData(exporter->GetCallbackUserData());
}

/**
 * This function will connect the given vtkImageExport filter to
 * the given itk::VTKImageImport filter.
 */
template <typename VTK_Exporter, typename ITK_Importer>
void ConnectPipelines(VTK_Exporter* exporter, ITK_Importer importer)
{
  importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());
  importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
  importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
  importer->SetSpacingCallback(exporter->GetSpacingCallback());
  importer->SetOriginCallback(exporter->GetOriginCallback());
  importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());
  importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());
  importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
  importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
  importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
  importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
  importer->SetCallbackUserData(exporter->GetCallbackUserData());
}


#define DelegateSetMacro(name,arg) DelegateITKInputMacro(Set##name,arg)
#define DelegateITKInputMacro(name,arg) \
if ( 1 ) { \
  vtkDebugMacro( << this->GetClassName() << " (" << this << "): setting " #name " to " << #arg ); \
  ImageFilterType* tempFilter = dynamic_cast<ImageFilterType*> ( this->m_Filter.GetPointer() ); \
  if ( tempFilter ) \
    { \
    tempFilter->name ( arg ); \
    this->Modified(); \
    } \
  }

#define DelegateGetMacro(name) DelegateITKOutputMacro (Get##name)
#define DelegateITKOutputMacro(name) \
if ( 1 ) { \
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): returning " #name ); \
  ImageFilterType* tempFilter = dynamic_cast<ImageFilterType*> ( this->m_Filter.GetPointer() ); \
  if ( tempFilter ) \
    { \
    return tempFilter->name (); \
    } \
    else \
    { \
    vtkErrorMacro ( << this->GetClassName() << " Error getting " #name " Dynamic cast returned 0" ); \
    return 0; \
    } \
  }


// struct vtkITKProgressDisplay
// {
//   ProgressDisplay(vtkObject* obj, itk::ProcessObject* process): m_Process(process), m_Object(obj) {}
  
//   void Display()
//   {
//     m_Object->SetProgress ( m_Process->GetProgress() );
//     }
  
//   itk::ProcessObject::Pointer m_Process;
//   vtkObject* m_Object();
// };

//   // Add a progress observer for the itk::CurvatureFlowImageFilter.
//   // This will make it clear when this part of the ITK pipeline
//   // executes.
//   ProgressDisplay progressDisplay(denoiser);
//   itk::SimpleMemberCommand<ProgressDisplay>::Pointer progressEvent =
//     itk::SimpleMemberCommand<ProgressDisplay>::New();
//   progressEvent->SetCallbackFunction(&progressDisplay,
//                                      &ProgressDisplay::Display);
//   denoiser->AddObserver(itk::ProgressEvent(), progressEvent);



#endif
