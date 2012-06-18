/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Libs/vtkITK/vtkITKArchetypeImageSeriesVectorReaderFile.cxx $
  Date:      $Date: 2007-08-15 17:04:27 -0400 (Wed, 15 Aug 2007) $
  Version:   $Revision: 4068 $

==========================================================================*/

// VTKITK includes
#include "vtkITKArchetypeImageSeriesVectorReaderFile.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

// ITK includes
#include <itkImageFileReader.h>
#include <itkOrientImageFilter.h>

vtkStandardNewMacro(vtkITKArchetypeImageSeriesVectorReaderFile);
vtkCxxRevisionMacro(vtkITKArchetypeImageSeriesVectorReaderFile, "$Revision: 4068 $");

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderFile::vtkITKArchetypeImageSeriesVectorReaderFile()
{
}

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderFile::~vtkITKArchetypeImageSeriesVectorReaderFile() 
{
}

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesVectorReaderFile::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "vtk ITK Archetype Image Series Vector Reader File\n";
}

//----------------------------------------------------------------------------
template <class T>
void vtkITKExecuteDataFromFileVector(
  vtkITKArchetypeImageSeriesVectorReaderFile* self,
  vtkImageData *data,
  vtkDataObject *output)
{
  typedef itk::Vector<T, 3> VectorPixelType;
  typedef itk::VectorImage<T,3> image2;
  typedef itk::ImageSource<image2> FilterType;
  FilterType::Pointer filter;
  typedef itk::ImageFileReader< image2 > ReaderType;
  ReaderType::Pointer reader2 = ReaderType::New();
  reader2->SetFileName(self->GetFileName(0));
  if (self->GetUseNativeCoordinateOrientation())
    {
    filter = reader2;
    }
  else
    {
    itk::OrientImageFilter<image2,image2>::Pointer orient2 =
      itk::OrientImageFilter<image2,image2>::New();
    orient2->SetDebug(self->GetDebug());
    orient2->SetInput(reader2->GetOutput());
    orient2->UseImageDirectionOn();
    orient2->SetDesiredCoordinateOrientation(
      self->GetDesiredCoordinateOrientation());
    filter = orient2;
    }
   filter->UpdateLargestPossibleRegion();
  itk::ImportImageContainer<unsigned long, T>::Pointer PixelContainer2;
  PixelContainer2 = filter->GetOutput()->GetPixelContainer();
  void *ptr = static_cast<void *> (PixelContainer2->GetBufferPointer());
  (dynamic_cast<vtkImageData *>( output))->GetPointData()->GetScalars()
    ->SetVoidArray(ptr, PixelContainer2->Size(), 0);
  PixelContainer2->ContainerManageMemoryOff();
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkITKArchetypeImageSeriesVectorReaderFile::ExecuteData(vtkDataObject *output)
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
    vtkDebugMacro("ImageSeriesVectorReaderFile: only one file: " << this->FileNames[0].c_str());
    vtkDebugMacro("DiffusionTensorImageReaderFile: only one file: " << this->FileNames[0].c_str());
    switch (this->OutputScalarType)
      {
      vtkTemplateMacroCase(VTK_DOUBLE, double, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data, output));
      vtkTemplateMacroCase(VTK_FLOAT, float, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data, output));
      vtkTemplateMacroCase(VTK_LONG, long, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data, output));
      vtkTemplateMacroCase(VTK_UNSIGNED_LONG, unsigned long, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data, output));
      vtkTemplateMacroCase(VTK_INT, int, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data, output));
      vtkTemplateMacroCase(VTK_UNSIGNED_INT, unsigned int, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data, output));
      vtkTemplateMacroCase(VTK_SHORT, short, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data, output));
      vtkTemplateMacroCase(VTK_UNSIGNED_SHORT, unsigned short, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data, output));
      vtkTemplateMacroCase(VTK_CHAR, char, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data, output));
      vtkTemplateMacroCase(VTK_SIGNED_CHAR, signed char, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data, output));
      vtkTemplateMacroCase(VTK_UNSIGNED_CHAR, unsigned char, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data, output));
    default:
        vtkErrorMacro(<< "UpdateFromFile: Unknown data type " << this->OutputScalarType);
      }
    }
  else
    {
    // ERROR - should have used the series reader
    vtkErrorMacro("There is more than one file, use the VectorReaderSeries instead");
    }
}


void vtkITKArchetypeImageSeriesVectorReaderFile::ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&,void* data)
{
  vtkITKArchetypeImageSeriesVectorReaderFile* me=reinterpret_cast<vtkITKArchetypeImageSeriesVectorReaderFile*>(data);
  me->Progress=obj->GetProgress();
  me->InvokeEvent(vtkCommand::ProgressEvent,&me->Progress);
}
