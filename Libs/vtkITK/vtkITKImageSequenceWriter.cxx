/*==========================================================================

  Copyright (c) Ebatinca S.L., Las Palmas de Gran Canaria, Spain

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, Ebatinca, funded
  by the grant GRT-00000485 of Children's Hospital of Philadelphia, USA.

==========================================================================*/

// vtkITK includes
#include "vtkITKImageSequenceWriter.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkFloatArray.h>
#include <vtkImageExport.h>
#include <vtkImageFlip.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkITKUtility.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVersion.h>

// VTKsys includes
#include <vtksys/SystemTools.hxx>

// ITK includes
#include <itkDiffusionTensor3D.h>
#include <itkImageFileWriter.h>
#include <itkJoinSeriesImageFilter.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <itkMetaDataObjectBase.h>
#include <itkVTKImageImport.h>


vtkStandardNewMacro(vtkITKImageSequenceWriter);

// helper function
template <class TPixelType, int Dimension>
void ITKWriteVTKImage(vtkITKImageSequenceWriter* self, vtkCollection* inputImageCollection, char* fileName, vtkMatrix4x4* rasToIjkMatrix)
{
  typedef itk::Image<TPixelType, Dimension-1> InImageType;
  typedef itk::Image<TPixelType, Dimension> OutImageType;

  vtkMatrix4x4* ijkToRasMatrix = vtkMatrix4x4::New();

  if (rasToIjkMatrix == nullptr)
  {
    std::cerr << "ITKWriteVTKImage: rasToIjkMatrix is null" << std::endl;
  }
  else
  {
    vtkMatrix4x4::Invert(rasToIjkMatrix, ijkToRasMatrix);
  }
  ijkToRasMatrix->Transpose();

  typename InImageType::DirectionType inDirection;
  typename InImageType::PointType inOrigin;
  typename OutImageType::DirectionType outDirection;
  typename OutImageType::PointType outOrigin;
  inDirection.SetIdentity();

  double mag[4] = {0.0};
  int i = 0;
  for (i=0; i<4; i++)
  {
    // normalize vectors
    mag[i] = 0;
    for (int j=0; j<4; j++)
    {
      mag[i] += ijkToRasMatrix->GetElement(i,j) * ijkToRasMatrix->GetElement(i,j);
    }
    if (mag[i] == 0.0)
    {
      mag[i] = 1;
    }
    mag[i] = sqrt(mag[i]);
  }

  for (i=0; i<4; i++)
  {
    for (int j=0; j<4; j++)
    {
      ijkToRasMatrix->SetElement(i, j, ijkToRasMatrix->GetElement(i,j) / mag[i]);
    }
  }

  // ITK image direction are in LPS space
  // convert from ijkToRas to ijkToLps
  vtkMatrix4x4* rasToLpsMatrix = vtkMatrix4x4::New();
  rasToLpsMatrix->Identity();
  rasToLpsMatrix->SetElement(0,0,-1);
  rasToLpsMatrix->SetElement(1,1,-1);

  vtkMatrix4x4* ijkToLpsMatrix = vtkMatrix4x4::New();
  vtkMatrix4x4::Multiply4x4(ijkToRasMatrix, rasToLpsMatrix, ijkToLpsMatrix);

  for (i=0; i<Dimension; i++)
  {
    if (i < Dimension-1)
    {
      inOrigin[i] = ijkToRasMatrix->GetElement(3,i);
    }
    outOrigin[i] = ijkToRasMatrix->GetElement(3,i);
    for (int j=0; j<Dimension; j++)
    {
      if (i < Dimension-1 && j < Dimension-1)
      {
        inDirection[j][i] = ijkToLpsMatrix->GetElement(i,j);
      }
      outDirection[j][i] = ijkToLpsMatrix->GetElement(i,j);
    }
  }

  rasToLpsMatrix->Delete();
  ijkToRasMatrix->Delete();
  ijkToLpsMatrix->Delete();

  inOrigin[0] *= -1;
  inOrigin[1] *= -1;

  typedef typename itk::VTKImageImport<InImageType> ImageImportType;

  typedef typename itk::JoinSeriesImageFilter<InImageType, OutImageType> JoinImageFilterType;
  typename JoinImageFilterType::Pointer joinImageFilter = JoinImageFilterType::New();

  for (int imgIdx=0; imgIdx<inputImageCollection->GetNumberOfItems(); ++imgIdx)
  {
    vtkImageData* inputImage = vtkImageData::SafeDownCast(inputImageCollection->GetItemAsObject(imgIdx));

    // itk import for input itk images
    typename ImageImportType::Pointer itkImporter = ImageImportType::New();

    // vtk export for vtk image
    vtkNew<vtkImageExport> vtkExporter;

    // set pipeline for the image
    vtkExporter->SetInputData(inputImage);

    ConnectPipelines(vtkExporter.GetPointer(), itkImporter);

    itkImporter->GetOutput()->SetDirection(inDirection);
    itkImporter->GetOutput()->Update();
    itkImporter->GetOutput()->SetOrigin(inOrigin);
    itkImporter->GetOutput()->SetSpacing(mag);

    joinImageFilter->PushBackInput(itkImporter->GetOutput());
  }

  // Set origin and spacing of the new dimension
  joinImageFilter->SetOrigin(0.0);
  joinImageFilter->SetSpacing(1.0);
  joinImageFilter->Update();

  // writer
  typedef typename itk::ImageFileWriter<OutImageType> ImageWriterType;
  typename ImageWriterType::Pointer itkImageWriter = ImageWriterType::New();

  if (self->GetUseCompression())
  {
    itkImageWriter->UseCompressionOn();
  }
  else
  {
    itkImageWriter->UseCompressionOff();
  }

  // setup image writer
  itk::ImageIOBase* imageIOType = nullptr;
  if (self->GetImageIOClassName())
  {
    itk::LightObject::Pointer objectType = itk::ObjectFactoryBase::CreateInstance(self->GetImageIOClassName());
    imageIOType = dynamic_cast<itk::ImageIOBase*>(objectType.GetPointer());
    if (imageIOType)
    {
      itkImageWriter->SetImageIO(imageIOType);
    }
  }
  OutImageType::Pointer joinedImage = joinImageFilter->GetOutput();
  itkImageWriter->SetInput(joinedImage);

  // Fill in axis kinds metadata
  itk::MetaDataDictionary &dictionary = joinedImage->GetMetaDataDictionary();
  std::string kindsKeyBase = "NRRD_kinds[";
  unsigned int axis = 0;
  for (int spatialAxis = 0; spatialAxis < 3; ++spatialAxis, ++axis)
  {
    itk::EncapsulateMetaData<std::string>(dictionary, kindsKeyBase + std::to_string(axis) + std::string("]"), "domain");
  }
  itk::EncapsulateMetaData<std::string>(dictionary, kindsKeyBase + std::to_string(axis) + std::string("]"), "list");
  // Set intent code indicating this is a transform (comes from Nifti heritage as a de facto standard)
  if (self->GetIntentCode())
  {
    itk::EncapsulateMetaData<std::string>(dictionary, "intent_code", self->GetIntentCode());
  }

  try
  {
    joinImageFilter->GetOutput()->SetDirection(outDirection);
    joinImageFilter->GetOutput()->Update();
    joinImageFilter->GetOutput()->SetOrigin(outOrigin);
    joinImageFilter->GetOutput()->SetSpacing(mag);
    itkImageWriter->SetFileName(fileName);
    itkImageWriter->Update();
  }
  catch (itk::ExceptionObject& exception)
  {
    exception.Print(std::cerr);
    throw exception;
  }
}

//----------------------------------------------------------------------------
template <class TPixelType>
void ITKWriteVTKImage(vtkITKImageSequenceWriter *self, vtkCollection *inputImageCollection, char *fileName, vtkMatrix4x4* rasToIjkMatrix)
{
  // Fix 4 dimensions: 3 spatial + 1 sequence. The fifth dimension for the scalar components is in the pixel type
  ITKWriteVTKImage<TPixelType, 4>(self, inputImageCollection, fileName, rasToIjkMatrix);
}

//----------------------------------------------------------------------------
vtkITKImageSequenceWriter::vtkITKImageSequenceWriter()
{
}

//----------------------------------------------------------------------------
vtkITKImageSequenceWriter::~vtkITKImageSequenceWriter()
{
  this->SetFileName(nullptr);
  this->SetImageIOClassName(nullptr);
  this->SetIntentCode(nullptr);
}

//----------------------------------------------------------------------------
void vtkITKImageSequenceWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "FileName: " << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "ImageIOClassName: " << (this->ImageIOClassName ? this->ImageIOClassName : "(none)") << "\n";
}

//------------------------------------------------------------------------------
int vtkITKImageSequenceWriter::FillInputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  return this->Superclass::FillInputPortInformation(port, info);
}

//----------------------------------------------------------------------------
// Writes all the data from the input.
void vtkITKImageSequenceWriter::Write()
{
  if (!this->FileName)
  {
    vtkErrorMacro(<<"vtkITKImageSequenceWriter: Please specify a FileName");
    return;
  }

  this->UpdateInformation();

  int numberOfImputImages = this->GetNumberOfInputConnections(0);
  int inputDataType = -1;
  int inputNumberOfScalarComponents = -1;
  vtkNew<vtkCollection> inputImageCollection;
  for (int i=0; i<numberOfImputImages; ++i)
  {
    vtkImageData* inputImage = vtkImageData::SafeDownCast(this->GetInputDataObject(0, i));
    if (inputImage == nullptr)
    {
      vtkErrorMacro(<<"vtkITKImageSequenceWriter: Invalid input object at connection " << i);
      continue;
    }
    vtkPointData* pointData = inputImage->GetPointData();
    if (pointData == nullptr)
    {
      vtkErrorMacro(<<"vtkITKImageSequenceWriter: No image to write at connection " << i);
      continue;
    }
    int currentInputDataType =
      pointData->GetScalars() ? pointData->GetScalars()->GetDataType() :
      pointData->GetTensors() ? pointData->GetTensors()->GetDataType() :
      pointData->GetVectors() ? pointData->GetVectors()->GetDataType() :
      pointData->GetNormals() ? pointData->GetNormals()->GetDataType() :
      0;
    if (inputDataType == -1)
    {
      inputDataType = currentInputDataType;
    }
    else if (currentInputDataType != inputDataType)
    {
      vtkErrorMacro(<<"vtkITKImageSequenceWriter: Data type mismatch at connection " << i << ". Skipping connection.");
      continue;
    }
    int currentInputNumberOfScalarComponents =
      pointData->GetScalars() ? pointData->GetScalars()->GetNumberOfComponents() :
      pointData->GetTensors() ? pointData->GetTensors()->GetNumberOfComponents() :
      pointData->GetVectors() ? pointData->GetVectors()->GetNumberOfComponents() :
      pointData->GetNormals() ? pointData->GetNormals()->GetNumberOfComponents() :
      0;
    if (inputNumberOfScalarComponents == -1)
    {
      inputNumberOfScalarComponents = currentInputNumberOfScalarComponents;
    }
    else if (currentInputNumberOfScalarComponents != inputNumberOfScalarComponents)
    {
      vtkErrorMacro(<<"vtkITKImageSequenceWriter: Number of components mismatch at connection " << i << ". Skipping connection.");
      continue;
    }

    inputImageCollection->AddItem(inputImage);
  }

  if (this->GetOutputInformation(0))
  {
    this->GetOutputInformation(0)->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
      this->GetOutputInformation(0)->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()), 6);
  }

  if (inputNumberOfScalarComponents == 1)
  {
    // take into consideration the scalar type
    switch (inputDataType)
    {
      case VTK_DOUBLE:
        ITKWriteVTKImage<double>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
        break;
      case VTK_FLOAT:
        ITKWriteVTKImage<float>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
        break;
      case VTK_LONG:
        ITKWriteVTKImage<long>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
        break;
      case VTK_UNSIGNED_LONG:
        ITKWriteVTKImage<unsigned long>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
        break;
      case VTK_INT:
        ITKWriteVTKImage<int>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
        break;
      case VTK_UNSIGNED_INT:
        ITKWriteVTKImage<unsigned int>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
        break;
      case VTK_SHORT:
        ITKWriteVTKImage<short>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
        break;
      case VTK_UNSIGNED_SHORT:
        ITKWriteVTKImage<unsigned short>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
        break;
      case VTK_CHAR:
        ITKWriteVTKImage<char>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
        break;
      case VTK_UNSIGNED_CHAR:
        ITKWriteVTKImage<unsigned char>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
        break;
      default:
        vtkErrorMacro(<< "Execute: Unknown output ScalarType");
        return;
    }
  } // scalar
  else if (inputNumberOfScalarComponents == 3)
  {
    if (this->VoxelVectorType == vtkITKImageSequenceWriter::VoxelVectorTypeColorRGB)
    {
      switch (inputDataType)
      {
        case VTK_DOUBLE:
        {
          typedef itk::RGBPixel<double> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_FLOAT:
        {
          typedef itk::RGBPixel<float> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_UNSIGNED_SHORT:
        {
          typedef itk::RGBPixel<unsigned short> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_UNSIGNED_CHAR:
        {
          typedef itk::RGBPixel<unsigned char> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        default:
          vtkErrorMacro(<< "Execute: Unknown output ScalarType");
          return;
      }
    }
    else if (this->VoxelVectorType == vtkITKImageSequenceWriter::VoxelVectorTypeSpatial)
    {
      switch (inputDataType)
      {
        case VTK_DOUBLE:
        {
          typedef itk::CovariantVector<double,3> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_FLOAT:
        {
          typedef itk::CovariantVector<float, 3> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_UNSIGNED_SHORT:
        {
          typedef itk::CovariantVector<unsigned short, 3> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_UNSIGNED_CHAR:
        {
          typedef itk::CovariantVector<unsigned char, 3> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        default:
          vtkErrorMacro(<< "Execute: Unknown output ScalarType");
          return;
      }
    }
    else
    {
      switch (inputDataType)
      {
        case VTK_DOUBLE:
        {
          typedef itk::Vector<double, 3> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_FLOAT:
        {
          typedef itk::Vector<float, 3> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_UNSIGNED_SHORT:
        {
          typedef itk::Vector<unsigned short, 3> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_UNSIGNED_CHAR:
        {
          typedef itk::Vector<unsigned char, 3> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        default:
          vtkErrorMacro(<< "Execute: Unknown output ScalarType");
          return;
      }
    }
  } // vector
  else if (inputNumberOfScalarComponents == 4)
  {
    if (this->VoxelVectorType == vtkITKImageSequenceWriter::VoxelVectorTypeColorRGBA)
    {
      switch (inputDataType)
      {
        case VTK_DOUBLE:
        {
          typedef itk::RGBAPixel<double> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_FLOAT:
        {
          typedef itk::RGBAPixel<float> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_UNSIGNED_SHORT:
        {
          typedef itk::RGBAPixel<unsigned short> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_UNSIGNED_CHAR:
        {
          typedef itk::RGBAPixel<unsigned char> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        default:
          vtkErrorMacro(<< "Execute: Unknown output ScalarType");
          return;
      }
    }
    else
    {
      switch (inputDataType)
      {
        case VTK_DOUBLE:
        {
          typedef itk::Vector<double, 4> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_FLOAT:
        {
          typedef itk::Vector<float, 4> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_UNSIGNED_SHORT:
        {
          typedef itk::Vector<unsigned short, 4> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        case VTK_UNSIGNED_CHAR:
        {
          typedef itk::Vector<unsigned char, 4> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
          break;
        }
        default:
          vtkErrorMacro(<< "Execute: Unknown output ScalarType");
          return;
      }
    }
  } // 4-vector
  else
  {
    vtkErrorMacro(<< "Can only export 1 or 3 component images, current image has " << inputNumberOfScalarComponents << " components");
    return;
  }
}
