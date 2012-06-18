/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Libs/vtkITK/vtkITKArchetypeImageSeriesVectorReaderSeries.cxx $
  Date:      $Date: 2007-08-15 17:04:27 -0400 (Wed, 15 Aug 2007) $
  Version:   $Revision: 4068 $

==========================================================================*/

// VTKITK includes
#include "vtkITKArchetypeImageSeriesVectorReaderSeries.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

// ITK includes
#include <itkImageFileReader.h>
#include <itkImageSeriesReader.h>
#include <itkOrientImageFilter.h>

// GDCM includes
#include "gdcmDict.h"           /// access to dictionary
#include "gdcmDictEntry.h"      /// access to dictionary
#if ITK_VERSION_MAJOR < 4
#include "gdcmDictSet.h"        /// access to dictionary
#endif
#include "gdcmGlobal.h"         /// access to dictionary

vtkStandardNewMacro(vtkITKArchetypeImageSeriesVectorReaderSeries);
vtkCxxRevisionMacro(vtkITKArchetypeImageSeriesVectorReaderSeries, "$Revision: 4068 $");

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderSeries::vtkITKArchetypeImageSeriesVectorReaderSeries()
{
}

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderSeries::~vtkITKArchetypeImageSeriesVectorReaderSeries() 
{
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesVectorReaderSeries::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "vtk ITK Archetype Image Series Vector Reader\n";
}

//----------------------------------------------------------------------------
template <class T>
void vtkITKExecuteDataFromSeriesVector(
  vtkITKArchetypeImageSeriesVectorReaderSeries* self,
  vtkImageData* data,
  vtkDataObject* output)
{
  typedef T VectorPixelType;
  typedef itk::VectorImage<VectorPixelType,3> image;
  typedef itk::ImageSource<image> FilterType;
  FilterType::Pointer filter;
  itk::ImageSeriesReader<image>::Pointer reader =
  itk::ImageSeriesReader<image>::New();
  itk::CStyleCommand::Pointer pcl=itk::CStyleCommand::New();
  pcl->SetCallback((itk::CStyleCommand::FunctionPointer)&self->ReadProgressCallback);
  pcl->SetClientData(self);
  reader->SetFileNames(self->GetFileNames());
  reader->ReleaseDataFlagOn();
  reader->GetOutput()->SetVectorLength(3);
  if (self->GetUseNativeCoordinateOrientation())
    {
    filter = reader;
    }
  else
    {
    itk::OrientImageFilter<image,image>::Pointer orient =
        itk::OrientImageFilter<image,image>::New();
    orient->SetDebug(self->GetDebug());
    orient->SetInput(reader->GetOutput());
    orient->UseImageDirectionOn();
    orient->SetDesiredCoordinateOrientation(
      self->GetDesiredCoordinateOrientation());
    filter = orient;
    }
  filter->UpdateLargestPossibleRegion();
  itk::ImportImageContainer<unsigned long, VectorPixelType>::Pointer PixelContainer;
  PixelContainer = filter->GetOutput()->GetPixelContainer();
  void *ptr = static_cast<void *> (PixelContainer->GetBufferPointer());
  (dynamic_cast<vtkImageData *>(output))->GetPointData()->GetScalars()->SetVoidArray(ptr, PixelContainer->Size(), 0);
  PixelContainer->ContainerManageMemoryOff();
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkITKArchetypeImageSeriesVectorReaderSeries::ExecuteData(vtkDataObject *output)
{
  if (!this->Superclass::Archetype)
    {
      vtkErrorMacro("An Archetype must be specified.");
      return;
    }

  vtkImageData *data = vtkImageData::SafeDownCast(output);
  //data->UpdateInformation();
  data->SetExtent(0,0,0,0,0,0);
  data->AllocateScalars();
  data->SetExtent(data->GetWholeExtent());

    // If there is only one file in the series, just use an image file reader
  if (this->FileNames.size() == 1)
    {
    vtkErrorMacro("ImageSeriesVectorReaderSeries: only one file: " << this->FileNames[0].c_str() << " use the VectorReaderFile instead!");
    }    
  else
    {
    // use the series reader
    try 
      {
      switch (this->OutputScalarType)
        {
        vtkTemplateMacroCase(VTK_DOUBLE, double, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data, output));
        vtkTemplateMacroCase(VTK_FLOAT, float, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data, output));
        vtkTemplateMacroCase(VTK_LONG, long, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data, output));
        vtkTemplateMacroCase(VTK_UNSIGNED_LONG, unsigned long, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data, output));
        vtkTemplateMacroCase(VTK_INT, int, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data, output));
        vtkTemplateMacroCase(VTK_UNSIGNED_INT, unsigned int, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data, output));
        vtkTemplateMacroCase(VTK_SHORT, short, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data, output));
        vtkTemplateMacroCase(VTK_UNSIGNED_SHORT, unsigned short, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data, output));
        vtkTemplateMacroCase(VTK_CHAR, char, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data, output));
        vtkTemplateMacroCase(VTK_UNSIGNED_CHAR, unsigned char, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data, output));
        default:
          vtkErrorMacro(<< "UpdateFromFile Series: Unknown data type " << this->OutputScalarType);
        }
      }
    catch (itk::ExceptionObject & e)
      {
      vtkErrorMacro(<< "Exception from vtkITK MegaMacro: " << e << "\n");
      }
    }
}


void vtkITKArchetypeImageSeriesVectorReaderSeries::ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&,void* data)
{
  vtkITKArchetypeImageSeriesVectorReaderSeries* me=reinterpret_cast<vtkITKArchetypeImageSeriesVectorReaderSeries*>(data);
  me->Progress=obj->GetProgress();
  me->InvokeEvent(vtkCommand::ProgressEvent,&me->Progress);
}
