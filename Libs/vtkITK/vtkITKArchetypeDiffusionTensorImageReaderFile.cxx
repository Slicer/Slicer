/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

// vtkITK includes
#include "vtkITKArchetypeDiffusionTensorImageReaderFile.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkErrorCode.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>

// ITK includes
#include <itkDiffusionTensor3D.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkOrientImageFilter.h>

vtkStandardNewMacro(vtkITKArchetypeDiffusionTensorImageReaderFile);

//----------------------------------------------------------------------------
vtkITKArchetypeDiffusionTensorImageReaderFile::vtkITKArchetypeDiffusionTensorImageReaderFile() = default;

//----------------------------------------------------------------------------
vtkITKArchetypeDiffusionTensorImageReaderFile::~vtkITKArchetypeDiffusionTensorImageReaderFile() = default;

//----------------------------------------------------------------------------
void vtkITKArchetypeDiffusionTensorImageReaderFile::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "vtk ITK Archetype Image Series DiffusionTensor3D Reader File\n";
}

//----------------------------------------------------------------------------
template <class T>
void vtkITKExecuteDataFromFileDiffusionTensor3D(
  vtkITKArchetypeDiffusionTensorImageReaderFile* self,
  vtkFloatArray* tensors,
  vtkImageData *data)
{
  typedef itk::DiffusionTensor3D<T> DiffusionTensor3DPixelType;
  typedef itk::Image<DiffusionTensor3DPixelType,3> ImageType;
  typedef itk::ImageSource<ImageType> FilterType;
  typename FilterType::Pointer filter;
  typedef itk::ImageFileReader<ImageType> ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(self->GetFileName(0));
  if (self->GetUseNativeCoordinateOrientation())
    {
    filter = reader;
    }
  else
    {
    typename itk::OrientImageFilter<ImageType,ImageType>::Pointer orient2 =
      itk::OrientImageFilter<ImageType,ImageType>::New();
    orient2->SetDebug(self->GetDebug());
    orient2->SetInput(reader->GetOutput());
    orient2->UseImageDirectionOn();
    orient2->SetDesiredCoordinateOrientation(self->GetDesiredCoordinateOrientation());
    filter = orient2;
    }
  // Check pixel type. It must be vector for DTI
  reader->UpdateOutputInformation();

  // This is a conservative test for dti. It filters out most non-dti
  // data without having to read the entire file
  unsigned int numberOfComponents = reader->GetImageIO()->GetNumberOfComponents();
  if (numberOfComponents != 9 && numberOfComponents != 6)
    {
    ::itk::InvalidArgumentError e_(__FILE__, __LINE__);
    std::ostringstream message;
    message << "number of components is: " << numberOfComponents << " but expected 6 or 9";
    e_.SetDescription(message.str());
    throw e_; /* Explicit naming to work around Intel compiler bug.  */
    }

  // pixel type and number of components are correct. OK to read image data
  filter->UpdateLargestPossibleRegion();
  itk::ImageRegionConstIteratorWithIndex< ImageType >
    it( reader->GetOutput(), reader->GetOutput()->GetLargestPossibleRegion() );
  tensors->SetNumberOfComponents(9);
  tensors->SetNumberOfTuples(data->GetNumberOfPoints());
  //tensors->Modified();
  for ( it.GoToBegin(); !it.IsAtEnd() ; ++it )
    {
    const itk::Index<3u> index = it.GetIndex();
    vtkIdType position = data->FindPoint(index[0], index[1], index[2]);
    if (position == static_cast<vtkIdType>(-1) ||
        position >= tensors->GetNumberOfTuples())
      {
      itkGenericExceptionMacro(<< "Can't find index " << index
                               << " in output image data.");
      continue;
      }
    float value[9];
    const DiffusionTensor3DPixelType tensor = it.Get();
    for (int i=0; i<3; i++)
      {
      for(int j=0; j<3; j++)
        {
        value[i + 3 * j] = float(tensor(i, j));
        }
      }
    tensors->SetTypedTuple(position, value);
    }
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
int vtkITKArchetypeDiffusionTensorImageReaderFile::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  if (!this->Superclass::Archetype)
    {
    vtkErrorMacro("An Archetype must be specified.");
    this->SetErrorCode(vtkErrorCode::NoFileNameError);
    return 0;
    }

  int extent[6] = {0,-1,0,-1,0,-1};
  vtkImageData* data = vtkImageData::GetData(outputVector);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  outInfo->Get
    (vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), extent);
  data->SetOrigin(0, 0, 0);
  data->SetSpacing(1, 1, 1);
  data->SetExtent(extent);
  this->SetMetaDataScalarRangeToPointDataInfo(data);
  vtkNew<vtkFloatArray> tensors;
  tensors->SetName("ArchetypeReader");

    // If there is only one file in the series, just use an image file reader
  try
    {
    if (this->FileNames.size() == 1)
      {
      vtkDebugMacro("DiffusionTensorImageReaderFile: only one file: " << this->FileNames[0].c_str());
      switch (this->OutputScalarType)
        {
        vtkTemplateMacro(vtkITKExecuteDataFromFileDiffusionTensor3D<VTK_TT>(
          this, tensors.GetPointer(), data));
        default:
          vtkErrorMacro(<< "UpdateFromFile: Unknown data type " << this->OutputScalarType);
          this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
        }
      data->GetPointData()->SetTensors(tensors.GetPointer());
      }
    else
      {
      // ERROR - should have used the series reader
      vtkErrorMacro("There is more than one file, use the vtkITKArchetypeImageSeriesReader instead");
      this->SetErrorCode(vtkErrorCode::FileFormatError);
      }
    }
  catch (itk::InvalidArgumentError & e)
    {
    vtkDebugMacro(<< "Could not read file as tensor" << e);
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    // return successful read, because this is an expected error when the user
    // has selected a file that doesn't happen to be a tensor.  So it's a file
    // format error, but not something that should trigger a VTK pipeline error
    // (at least not as used in vtkMRMLStorageNodes).
    return 1;
    }
  catch (itk::ExceptionObject & e)
    {
    vtkErrorMacro(<< "Exception from vtkITK MegaMacro: " << e << "\n");
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkITKArchetypeDiffusionTensorImageReaderFile
::ReadProgressCallback(itk::ProcessObject* obj,
                       const itk::ProgressEvent&,void* data)
{
  vtkITKArchetypeDiffusionTensorImageReaderFile* me =
    reinterpret_cast<vtkITKArchetypeDiffusionTensorImageReaderFile*>(data);
  me->Progress=obj->GetProgress();
  me->InvokeEvent(vtkCommand::ProgressEvent,&me->Progress);
}
