/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

// VTKITK includes
#include "vtkITKArchetypeImageSeriesVectorReaderFile.h"

// VTK includes
#include <vtkAOSDataArrayTemplate.h>
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkErrorCode.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

// ITK includes
#include <itkImageFileReader.h>
#include <itkOrientImageFilter.h>

vtkStandardNewMacro(vtkITKArchetypeImageSeriesVectorReaderFile);

namespace {

template <class T>
vtkAOSDataArrayTemplate<T>* DownCast(vtkAbstractArray* a)
{
  return vtkAOSDataArrayTemplate<T>::FastDownCast(a);
}

};

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderFile::vtkITKArchetypeImageSeriesVectorReaderFile() = default;

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderFile::~vtkITKArchetypeImageSeriesVectorReaderFile() = default;

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
  vtkImageData *data)
{
  typedef itk::VectorImage<T,3> image2;
  typedef itk::ImageSource<image2> FilterType;
  typename FilterType::Pointer filter;
  typedef itk::ImageFileReader< image2 > ReaderType;
  typename ReaderType::Pointer reader2 = ReaderType::New();
  reader2->SetFileName(self->GetFileName(0));
  if (self->GetUseNativeCoordinateOrientation())
    {
    filter = reader2;
    }
  else
    {
    typename itk::OrientImageFilter<image2,image2>::Pointer orient2 =
      itk::OrientImageFilter<image2,image2>::New();
    orient2->SetDebug(self->GetDebug());
    orient2->SetInput(reader2->GetOutput());
    orient2->UseImageDirectionOn();
    orient2->SetDesiredCoordinateOrientation(
      self->GetDesiredCoordinateOrientation());
    filter = orient2;
    }
  filter->UpdateLargestPossibleRegion();
  typename itk::ImportImageContainer<itk::SizeValueType, T>::Pointer PixelContainer2;
  PixelContainer2 = filter->GetOutput()->GetPixelContainer();
  void *ptr = static_cast<void *> (PixelContainer2->GetBufferPointer());
  DownCast<T>(data->GetPointData()->GetScalars())
    ->SetVoidArray(ptr, PixelContainer2->Size(), 0,
                   vtkAOSDataArrayTemplate<T>::VTK_DATA_ARRAY_DELETE);
  PixelContainer2->ContainerManageMemoryOff();
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkITKArchetypeImageSeriesVectorReaderFile::ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo)
{
    if (!this->Superclass::Archetype)
      {
        vtkErrorMacro("An Archetype must be specified.");
        this->SetErrorCode(vtkErrorCode::NoFileNameError);
        return;
      }
    vtkImageData *data = this->AllocateOutputData(output, outInfo);

    // If there is only one file in the series, just use an image file reader
  if (this->FileNames.size() == 1)
    {
    vtkDebugMacro("ImageSeriesVectorReaderFile: only one file: " << this->FileNames[0].c_str());
    vtkDebugMacro("DiffusionTensorImageReaderFile: only one file: " << this->FileNames[0].c_str());
    switch (this->OutputScalarType)
      {
      vtkTemplateMacroCase(VTK_DOUBLE, double, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
      vtkTemplateMacroCase(VTK_FLOAT, float, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
      vtkTemplateMacroCase(VTK_LONG, long, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
      vtkTemplateMacroCase(VTK_UNSIGNED_LONG, unsigned long, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
      vtkTemplateMacroCase(VTK_INT, int, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
      vtkTemplateMacroCase(VTK_UNSIGNED_INT, unsigned int, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
      vtkTemplateMacroCase(VTK_SHORT, short, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
      vtkTemplateMacroCase(VTK_UNSIGNED_SHORT, unsigned short, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
      vtkTemplateMacroCase(VTK_CHAR, char, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
      vtkTemplateMacroCase(VTK_SIGNED_CHAR, signed char, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
      vtkTemplateMacroCase(VTK_UNSIGNED_CHAR, unsigned char, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
    default:
        vtkErrorMacro(<< "UpdateFromFile: Unknown data type " << this->OutputScalarType);
        this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
      }

    this->SetMetaDataScalarRangeToPointDataInfo(data);
    }
  else
    {
    // ERROR - should have used the series reader
    vtkErrorMacro("There is more than one file, use the VectorReaderSeries instead");
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    }
}


void vtkITKArchetypeImageSeriesVectorReaderFile::ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&,void* data)
{
  vtkITKArchetypeImageSeriesVectorReaderFile* me=reinterpret_cast<vtkITKArchetypeImageSeriesVectorReaderFile*>(data);
  me->Progress=obj->GetProgress();
  me->InvokeEvent(vtkCommand::ProgressEvent,&me->Progress);
}
