/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#include "vtkITKArchetypeImageSeriesScalarReader.h"

// VTK includes
#include <vtkAOSDataArrayTemplate.h>
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkErrorCode.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVersion.h>

// ITK includes
#include <itkOrientImageFilter.h>
#include <itkImageSeriesReader.h>
#ifdef VTKITK_BUILD_DICOM_SUPPORT
#include <itkDCMTKImageIO.h>
#include <itkGDCMImageIO.h>
#endif

vtkStandardNewMacro(vtkITKArchetypeImageSeriesScalarReader);

namespace {

template <class T>
vtkAOSDataArrayTemplate<T>* DownCast(vtkAbstractArray* a)
{
  return vtkAOSDataArrayTemplate<T>::FastDownCast(a);
}

};

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesScalarReader::vtkITKArchetypeImageSeriesScalarReader() = default;

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesScalarReader::~vtkITKArchetypeImageSeriesScalarReader() = default;

//----------------------------------------------------------------------------
void vtkITKArchetypeImageSeriesScalarReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "vtk ITK Archetype Image Series Scalar Reader\n";
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
int vtkITKArchetypeImageSeriesScalarReader::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
{
  if (!this->Superclass::Archetype)
    {
      vtkErrorMacro("An Archetype must be specified.");
      this->SetErrorCode(vtkErrorCode::NoFileNameError);
      return 0;
    }

  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkDataObject * output = outInfo->Get(vtkDataObject::DATA_OBJECT());
  vtkImageData *data = vtkImageData::SafeDownCast(output);
  // removed UpdateInformation: generates an error message
  //   from VTK and doesn't appear to be needed...
  //data->UpdateInformation();
  data->SetExtent(0,0,0,0,0,0);
  data->AllocateScalars(outInfo);
  data->SetExtent(outInfo->Get(
    vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));
  this->SetMetaDataScalarRangeToPointDataInfo(data);

#ifdef VTKITK_BUILD_DICOM_SUPPORT
#define vtkITKExecuteDataDeclareDICOMImageIO \
      typedef itk::ImageIOBase ImageIOType; \
      ImageIOType::Pointer imageIO; \
      if (this->DICOMImageIOApproach == vtkITKArchetypeImageSeriesReader::GDCM) \
        { \
        imageIO = itk::GDCMImageIO::New(); \
        } \
      else if (this->DICOMImageIOApproach == vtkITKArchetypeImageSeriesReader::DCMTK) \
        { \
        imageIO = itk::DCMTKImageIO::New(); \
        } \
      else \
        { \
        vtkErrorMacro(<<"vtkITKExecuteDataFromSeries: Unsupported DICOMImageIOApproach: " << this->GetDICOMImageIOApproach()); \
        this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError); \
        return 0; \
        }
#else
#define vtkITKExecuteDataDeclareDICOMImageIO \
  typedef itk::ImageIOBase ImageIOType; \
  ImageIOType::Pointer imageIO;
#endif

/// SCALAR MACRO
#define vtkITKExecuteDataFromSeries(typeN, type) \
    case typeN: \
    {\
      typedef itk::Image<type,3> image##typeN;\
      typedef itk::ImageSource<image##typeN> FilterType; \
      FilterType::Pointer filter; \
      itk::ImageSeriesReader<image##typeN>::Pointer reader##typeN = \
        itk::ImageSeriesReader<image##typeN>::New(); \
      vtkITKExecuteDataDeclareDICOMImageIO \
      if (this->ArchetypeIsDICOM) \
        { \
        reader##typeN->SetImageIO(imageIO); \
        } \
      itk::CStyleCommand::Pointer pcl=itk::CStyleCommand::New(); \
      pcl->SetCallback((itk::CStyleCommand::FunctionPointer)&ReadProgressCallback); \
      pcl->SetClientData(this); \
      reader##typeN->AddObserver(itk::ProgressEvent(),pcl); \
      reader##typeN->SetFileNames(this->FileNames); \
      reader##typeN->ReleaseDataFlagOn(); \
      if (this->UseNativeCoordinateOrientation) \
        { \
        filter = reader##typeN; \
        } \
      else \
        { \
        itk::OrientImageFilter<image##typeN,image##typeN>::Pointer orient##typeN = \
            itk::OrientImageFilter<image##typeN,image##typeN>::New(); \
        if (this->Debug) {orient##typeN->DebugOn();} \
        orient##typeN->SetInput(reader##typeN->GetOutput()); \
        orient##typeN->UseImageDirectionOn(); \
        orient##typeN->SetDesiredCoordinateOrientation(this->DesiredCoordinateOrientation); \
        filter = orient##typeN; \
        }\
      filter->UpdateLargestPossibleRegion(); \
      itk::ImportImageContainer<itk::SizeValueType, type>::Pointer PixelContainer##typeN;\
      PixelContainer##typeN = filter->GetOutput()->GetPixelContainer();\
      void *ptr = static_cast<void *> (PixelContainer##typeN->GetBufferPointer());\
      DownCast<type>(data->GetPointData()->GetScalars())                \
        ->SetVoidArray(ptr, PixelContainer##typeN->Size(), 0,\
                       vtkAOSDataArrayTemplate<type>::VTK_DATA_ARRAY_DELETE);\
      PixelContainer##typeN->ContainerManageMemoryOff();\
    }\
    break

#define vtkITKExecuteDataFromFile(typeN, type) \
    case typeN: \
    {\
      typedef itk::Image<type,3> image2##typeN;\
      typedef itk::ImageSource<image2##typeN> FilterType; \
      FilterType::Pointer filter; \
      itk::ImageFileReader<image2##typeN>::Pointer reader2##typeN = \
            itk::ImageFileReader<image2##typeN>::New(); \
      reader2##typeN->SetFileName(this->FileNames[0].c_str()); \
      vtkITKExecuteDataDeclareDICOMImageIO \
      if (this->ArchetypeIsDICOM) \
        { \
        reader2##typeN->SetImageIO(imageIO); \
        } \
      if (this->UseNativeCoordinateOrientation) \
        { \
        filter = reader2##typeN; \
        } \
      else \
        { \
        itk::OrientImageFilter<image2##typeN,image2##typeN>::Pointer orient2##typeN = \
              itk::OrientImageFilter<image2##typeN,image2##typeN>::New(); \
        if (this->Debug) {orient2##typeN->DebugOn();} \
        orient2##typeN->SetInput(reader2##typeN->GetOutput()); \
        orient2##typeN->UseImageDirectionOn(); \
        orient2##typeN->SetDesiredCoordinateOrientation(this->DesiredCoordinateOrientation); \
        filter = orient2##typeN; \
        } \
      filter->UpdateLargestPossibleRegion();\
      itk::ImportImageContainer<itk::SizeValueType, type>::Pointer PixelContainer2##typeN;\
      PixelContainer2##typeN = filter->GetOutput()->GetPixelContainer();\
      void *ptr = static_cast<void *> (PixelContainer2##typeN->GetBufferPointer());\
      DownCast<type>(data->GetPointData()->GetScalars())                \
        ->SetVoidArray(ptr, PixelContainer2##typeN->Size(), 0,\
                       vtkAOSDataArrayTemplate<type>::VTK_DATA_ARRAY_DELETE);\
      PixelContainer2##typeN->ContainerManageMemoryOff();\
    }\
    break
  /// END SCALAR MACRO

  try
    {
    // If there is only one file in the series, just use an image file reader
    if (this->FileNames.size() == 1)
      {
      if (this->GetNumberOfComponents() == 1)
        {
        switch (this->OutputScalarType)
          {
            vtkITKExecuteDataFromFile(VTK_DOUBLE, double);
            vtkITKExecuteDataFromFile(VTK_FLOAT, float);
            vtkITKExecuteDataFromFile(VTK_LONG, long);
            vtkITKExecuteDataFromFile(VTK_UNSIGNED_LONG, unsigned long);
            vtkITKExecuteDataFromFile(VTK_INT, int);
            vtkITKExecuteDataFromFile(VTK_UNSIGNED_INT, unsigned int);
            vtkITKExecuteDataFromFile(VTK_SHORT, short);
            vtkITKExecuteDataFromFile(VTK_UNSIGNED_SHORT, unsigned short);
            vtkITKExecuteDataFromFile(VTK_CHAR, char);
            vtkITKExecuteDataFromFile(VTK_UNSIGNED_CHAR, unsigned char);
          default:
            vtkErrorMacro(<< "UpdateFromFile: Unknown data type");
            this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
          }
        }
      else
        {
          vtkErrorMacro(<< "UpdateFromFile: Unsupported number of components (only 1 allowed): " << this->GetNumberOfComponents());
          this->SetErrorCode(vtkErrorCode::FileFormatError);
        }
      }
    else
      {
      if (this->GetNumberOfComponents() == 1)
        {
        switch (this->OutputScalarType)
          {
            vtkITKExecuteDataFromSeries(VTK_DOUBLE, double);
            vtkITKExecuteDataFromSeries(VTK_FLOAT, float);
            vtkITKExecuteDataFromSeries(VTK_LONG, long);
            vtkITKExecuteDataFromSeries(VTK_UNSIGNED_LONG, unsigned long);
            vtkITKExecuteDataFromSeries(VTK_INT, int);
            vtkITKExecuteDataFromSeries(VTK_UNSIGNED_INT, unsigned int);
            vtkITKExecuteDataFromSeries(VTK_SHORT, short);
            vtkITKExecuteDataFromSeries(VTK_UNSIGNED_SHORT, unsigned short);
            vtkITKExecuteDataFromSeries(VTK_CHAR, char);
            vtkITKExecuteDataFromSeries(VTK_UNSIGNED_CHAR, unsigned char);
          default:
            vtkErrorMacro(<< "UpdateFromFile: Unknown data type");
            this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
          }
        }
      else
        {
          vtkErrorMacro(<<"UpdateFromSeries: Unsupported number of components: 1 != " << this->GetNumberOfComponents());
          this->SetErrorCode(vtkErrorCode::FileFormatError);
        }
      }
    }
    catch (itk::ExceptionObject & e)
      {
      vtkErrorMacro(<< "Exception from vtkITK MegaMacro: " << e << "\n");
      this->SetErrorCode(vtkErrorCode::FileFormatError);
      return 0;
      }
  return 1;
}


void vtkITKArchetypeImageSeriesScalarReader::ReadProgressCallback(itk::ProcessObject* obj,const itk::ProgressEvent&,void* data)
{
  vtkITKArchetypeImageSeriesScalarReader* me=reinterpret_cast<vtkITKArchetypeImageSeriesScalarReader*>(data);
  me->Progress=obj->GetProgress();
  me->InvokeEvent(vtkCommand::ProgressEvent,&me->Progress);
}
