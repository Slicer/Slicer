/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://www.na-mic.org/svn/Slicer3/trunk/Libs/vtkITK/vtkITKArchetypeImageSeriesReader.cxx $
  Date:      $Date: 2008-04-03 07:35:41 -0500 (Thu, 03 Apr 2008) $
  Version:   $Revision: 6383 $

==========================================================================*/
#include "vtkITKTimeSeriesDatabase.h"

#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"

#include "itkExceptionObject.h"

vtkCxxRevisionMacro(vtkITKTimeSeriesDatabase, "$Revision: 6383 $");
vtkStandardNewMacro(vtkITKTimeSeriesDatabase);
void vtkITKTimeSeriesDatabase::ExecuteInformation()
  {
  vtkImageData *output = this->GetOutput();
  double spacing[3];
  double origin[3];
  for ( int i = 0; i < 3; i++ )
    {
    spacing[i] = this->m_Filter->GetOutputSpacing()[i];
    origin[i] = this->m_Filter->GetOutputOrigin()[i];
    }
  output->SetSpacing(spacing);
  output->SetOrigin(origin);
  int extent[6];
  SourceType::OutputImageType::RegionType region = m_Filter->GetOutputRegion();
  extent[0] = region.GetIndex()[0];
  extent[1] = region.GetIndex()[0] + region.GetSize()[0] - 1;
  extent[2] = region.GetIndex()[1];
  extent[3] = region.GetIndex()[1] + region.GetSize()[1] - 1;
  extent[4] = region.GetIndex()[2];
  extent[5] = region.GetIndex()[2] + region.GetSize()[2] - 1;
  output->SetWholeExtent(extent);
  output->SetScalarType(VTK_SHORT);
  output->SetNumberOfScalarComponents(1);
  };


  // defined in the subclasses
void  vtkITKTimeSeriesDatabase::ExecuteData(vtkDataObject *output) 
  {
    vtkImageData *data = vtkImageData::SafeDownCast(output);
    data->SetExtent(0,0,0,0,0,0);
    data->AllocateScalars();
    data->SetExtent(data->GetWholeExtent());
    itk::ImportImageContainer<unsigned long, OutputImagePixelType>::Pointer PixelContainerShort;
    PixelContainerShort = this->m_Filter->GetOutput()->GetPixelContainer();
    void *ptr = static_cast<void *> (PixelContainerShort->GetBufferPointer());
    (dynamic_cast<vtkImageData *>( output))->GetPointData()->GetScalars()->SetVoidArray(ptr, PixelContainerShort->Size(), 0);
    PixelContainerShort->ContainerManageMemoryOff();
  };
