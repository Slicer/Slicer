/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

// VTKITK includes
#include "vtkITKArchetypeImageSeriesVectorReaderSeries.h"

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
#include <itkImageSeriesReader.h>
#include <itkOrientImageFilter.h>
#ifdef VTKITK_BUILD_DICOM_SUPPORT
#include <itkDCMTKImageIO.h>
#include <itkGDCMImageIO.h>

// GDCM includes
#include "gdcmDict.h"           /// access to dictionary
#include "gdcmDictEntry.h"      /// access to dictionary
#endif

vtkStandardNewMacro(vtkITKArchetypeImageSeriesVectorReaderSeries);

namespace {

template <class T>
vtkAOSDataArrayTemplate<T>* DownCast(vtkAbstractArray* a)
{
  return vtkAOSDataArrayTemplate<T>::FastDownCast(a);
}

};

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderSeries::vtkITKArchetypeImageSeriesVectorReaderSeries() = default;

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesVectorReaderSeries::~vtkITKArchetypeImageSeriesVectorReaderSeries() = default;

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
  vtkImageData* data)
{
  typedef T VectorPixelType;
  typedef itk::VectorImage<VectorPixelType,3> image;
  typedef itk::ImageSource<image> FilterType;
  typename FilterType::Pointer filter;
  typename itk::ImageSeriesReader<image>::Pointer reader =
    itk::ImageSeriesReader<image>::New();
  itk::CStyleCommand::Pointer pcl=itk::CStyleCommand::New();
  pcl->SetCallback((itk::CStyleCommand::FunctionPointer)&self->ReadProgressCallback);
  pcl->SetClientData(self);
  reader->SetFileNames(self->GetFileNames());
  reader->ReleaseDataFlagOn();
  reader->GetOutput()->SetVectorLength(3);
#ifdef VTKITK_BUILD_DICOM_SUPPORT
  typedef itk::ImageIOBase ImageIOType;
  ImageIOType::Pointer imageIO;
  if (self->GetDICOMImageIOApproach() == vtkITKArchetypeImageSeriesReader::GDCM)
    {
    imageIO = itk::GDCMImageIO::New();
    }
  else if (self->GetDICOMImageIOApproach() == vtkITKArchetypeImageSeriesReader::DCMTK)
    {
    imageIO = itk::DCMTKImageIO::New();
    }
  else
#endif
    {
    vtkErrorWithObjectMacro(self, <<"vtkITKArchetypeImageSeriesVectorReaderSeries: Unsupported DICOMImageIOApproach: " << self->GetDICOMImageIOApproach());
    itkGenericExceptionMacro("UnrecognizedFileTypeError");
    }
  if (self->GetUseNativeCoordinateOrientation())
    {
    filter = reader;
    }
  else
    {
    typename itk::OrientImageFilter<image,image>::Pointer orient =
        itk::OrientImageFilter<image,image>::New();
    orient->SetDebug(self->GetDebug());
    orient->SetInput(reader->GetOutput());
    orient->UseImageDirectionOn();
    orient->SetDesiredCoordinateOrientation(
      self->GetDesiredCoordinateOrientation());
    filter = orient;
    }
  filter->UpdateLargestPossibleRegion();
  typename itk::ImportImageContainer<itk::SizeValueType, VectorPixelType>::Pointer PixelContainer;
  PixelContainer = filter->GetOutput()->GetPixelContainer();
  void *ptr = static_cast<void *> (PixelContainer->GetBufferPointer());
  DownCast<T>(data->GetPointData()->GetScalars())
    ->SetVoidArray(ptr, PixelContainer->Size(), 0,
                   vtkAOSDataArrayTemplate<T>::VTK_DATA_ARRAY_DELETE);
  PixelContainer->ContainerManageMemoryOff();
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkITKArchetypeImageSeriesVectorReaderSeries::ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo)
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
    vtkErrorMacro("ImageSeriesVectorReaderSeries: only one file: " << this->FileNames[0].c_str() << " use the VectorReaderFile instead!");
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    }
  else
    {
    // use the series reader
    try
      {
      switch (this->OutputScalarType)
        {
        vtkTemplateMacroCase(VTK_DOUBLE, double, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data));
        vtkTemplateMacroCase(VTK_FLOAT, float, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data));
        vtkTemplateMacroCase(VTK_LONG, long, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data));
        vtkTemplateMacroCase(VTK_UNSIGNED_LONG, unsigned long, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data));
        vtkTemplateMacroCase(VTK_INT, int, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data));
        vtkTemplateMacroCase(VTK_UNSIGNED_INT, unsigned int, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data));
        vtkTemplateMacroCase(VTK_SHORT, short, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data));
        vtkTemplateMacroCase(VTK_UNSIGNED_SHORT, unsigned short, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data));
        vtkTemplateMacroCase(VTK_CHAR, char, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data));
        vtkTemplateMacroCase(VTK_UNSIGNED_CHAR, unsigned char, vtkITKExecuteDataFromSeriesVector<VTK_TT>(this, data));
        default:
          vtkErrorMacro(<< "UpdateFromFile Series: Unknown data type " << this->OutputScalarType);
          this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
        }
      }
    catch (itk::ExceptionObject & e)
      {
      vtkErrorMacro(<< "Exception from vtkITK MegaMacro: " << e << "\n");
      this->SetErrorCode(vtkErrorCode::FileFormatError);
      }

    this->SetMetaDataScalarRangeToPointDataInfo(data);
    }
}


void vtkITKArchetypeImageSeriesVectorReaderSeries::ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&,void* data)
{
  vtkITKArchetypeImageSeriesVectorReaderSeries* me=reinterpret_cast<vtkITKArchetypeImageSeriesVectorReaderSeries*>(data);
  me->Progress=obj->GetProgress();
  me->InvokeEvent(vtkCommand::ProgressEvent,&me->Progress);
}
