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
#include <itkVTKImageImport.h>

#define NRRD_DIM_MAX 16

class AttributeMapType : public std::map<std::string, std::string>
{
};
class AxisInfoMapType : public std::map<unsigned int, std::string>
{
};

vtkStandardNewMacro(vtkITKImageSequenceWriter);

// helper function
template <class TPixelType, int Dimension>
void ITKWriteVTKImage(vtkITKImageSequenceWriter* self,
                      vtkCollection* inputImageCollection,
                      char* fileName,
                      vtkMatrix4x4* rasToIjkMatrix,
                      vtkMatrix4x4* measurementFrameMatrix = nullptr,
                      int voxelVectorType = vtkITKImageWriter::VoxelVectorTypeUndefined)
{
  typedef itk::Image<TPixelType, Dimension - 1> InImageType;
  typedef itk::Image<TPixelType, Dimension> OutImageType;

  vtkNew<vtkMatrix4x4> ijkToRasMatrix;

  if (rasToIjkMatrix == nullptr)
  {
    std::cerr << "ITKWriteVTKImage: rasToIjkMatrix is null" << std::endl;
  }
  else
  {
    vtkMatrix4x4::Invert(rasToIjkMatrix, ijkToRasMatrix);
  }
  ijkToRasMatrix->Transpose();

  typename OutImageType::SpacingType outSpacing;
  outSpacing.Fill(1.0);
  for (int i = 0; i < Dimension - 1; i++)
  {
    // compute spacing
    outSpacing[i] = 0;
    for (int j = 0; j < Dimension - 1; j++)
    {
      outSpacing[i] += ijkToRasMatrix->GetElement(i, j) * ijkToRasMatrix->GetElement(i, j);
    }
    if (outSpacing[i] == 0.0)
    {
      outSpacing[i] = 1;
    }
    outSpacing[i] = sqrt(outSpacing[i]);
  }

  // ITK image direction are in LPS space
  // convert from ijkToRas to ijkToLps
  vtkNew<vtkMatrix4x4> rasToLpsMatrix;
  rasToLpsMatrix->Identity();
  rasToLpsMatrix->SetElement(0, 0, -1);
  rasToLpsMatrix->SetElement(1, 1, -1);

  vtkNew<vtkMatrix4x4> ijkToLpsMatrix;
  vtkMatrix4x4::Multiply4x4(ijkToRasMatrix, rasToLpsMatrix, ijkToLpsMatrix);

  typename OutImageType::DirectionType outDirection;
  typename OutImageType::PointType outOrigin;
  outOrigin.Fill(0.0);
  outDirection.SetIdentity();
  for (int i = 0; i < Dimension - 1; i++)
  {
    outOrigin[i] = ijkToLpsMatrix->GetElement(3, i);
    for (int j = 0; j < Dimension - 1; j++)
    {
      outDirection[j][i] = ijkToLpsMatrix->GetElement(i, j) / outSpacing[i];
    }
  }

  typedef typename itk::VTKImageImport<InImageType> ImageImportType;

  typedef typename itk::JoinSeriesImageFilter<InImageType, OutImageType> JoinImageFilterType;
  typename JoinImageFilterType::Pointer joinImageFilter = JoinImageFilterType::New();

  // Temporarily switch image voxel values from RAS to LPS
  bool convertVectorVoxelsToLPS = (voxelVectorType == vtkITKImageWriter::VoxelVectorTypeSpatial //
                                   || voxelVectorType == vtkITKImageWriter::VoxelVectorTypeSpatialCovariant);

  for (int imgIdx = 0; imgIdx < inputImageCollection->GetNumberOfItems(); ++imgIdx)
  {
    vtkImageData* inputImage = vtkImageData::SafeDownCast(inputImageCollection->GetItemAsObject(imgIdx));
    if (convertVectorVoxelsToLPS)
    {
      vtkITKImageWriter::ConvertSpatialVectorVoxelsBetweenRasLps(inputImage);
    }

    // itk import for input itk images
    typename ImageImportType::Pointer itkImporter = ImageImportType::New();

    // vtk export for vtk image
    vtkNew<vtkImageExport> vtkExporter;

    // set pipeline for the image
    vtkExporter->SetInputData(inputImage);

    ConnectPipelines(vtkExporter.GetPointer(), itkImporter);

    itkImporter->GetOutput()->Update();

    joinImageFilter->PushBackInput(itkImporter->GetOutput());
  }

  // No need to set origin and spacing for the new dimension here,
  // because we will set the image geometry for the joinImageFilter output later.
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
  typename OutImageType::Pointer joinedImage = joinImageFilter->GetOutput();
  itkImageWriter->SetInput(joinedImage);

  // Fill in axis kinds metadata
  itk::MetaDataDictionary& dictionary = joinedImage->GetMetaDataDictionary();
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

  // Set axis metadata
  if (!self->GetAxisLabels()->empty())
  {
    std::string labelsKeyBase = "NRRD_labels[";
    const char* labels[NRRD_DIM_MAX] = { nullptr };
    for (unsigned int axi = 0; axi < NRRD_DIM_MAX; axi++)
    {
      if (self->GetAxisLabels()->find(axi) != self->GetAxisLabels()->end())
      {
        labels[axi] = (*self->GetAxisLabels())[axi].c_str();
        itk::EncapsulateMetaData<std::string>(dictionary, labelsKeyBase + std::to_string(axi) + std::string("]"), labels[axi]);
      }
    }
  }
  if (!self->GetAxisUnits()->empty())
  {
    std::string unitsKeyBase = "NRRD_units[";
    const char* units[NRRD_DIM_MAX] = { nullptr };
    for (unsigned int axi = 0; axi < NRRD_DIM_MAX; axi++)
    {
      if (self->GetAxisUnits()->find(axi) != self->GetAxisUnits()->end())
      {
        units[axi] = (*self->GetAxisUnits())[axi].c_str();
        itk::EncapsulateMetaData<std::string>(dictionary, unitsKeyBase + std::to_string(axi) + std::string("]"), units[axi]);
      }
    }
  }

  if (measurementFrameMatrix != nullptr)
  {
    vtkITKImageWriter::WriteMeasurementFrameMatrixToMetaDataDictionary(dictionary, measurementFrameMatrix);
  }

  // Set attributes
  AttributeMapType::iterator ait;
  for (ait = self->GetAttributes()->begin(); ait != self->GetAttributes()->end(); ++ait)
  {
    // Do not set "space" as k-v. it is handled separately, and needs to be a nrrd *field*.
    if (ait->first != "space")
    {
      itk::EncapsulateMetaData<std::string>(dictionary, ait->first, ait->second);
    }
  }

  try
  {
    joinImageFilter->GetOutput()->SetDirection(outDirection);
    joinImageFilter->GetOutput()->Update();
    joinImageFilter->GetOutput()->SetOrigin(outOrigin);
    joinImageFilter->GetOutput()->SetSpacing(outSpacing);
    itkImageWriter->SetFileName(fileName);
    itkImageWriter->Update();

    if (convertVectorVoxelsToLPS)
    {
      for (int imgIdx = 0; imgIdx < inputImageCollection->GetNumberOfItems(); ++imgIdx)
      {
        vtkImageData* inputImage = vtkImageData::SafeDownCast(inputImageCollection->GetItemAsObject(imgIdx));
        vtkITKImageWriter::ConvertSpatialVectorVoxelsBetweenRasLps(inputImage);
      }
    }
  }
  catch (itk::ExceptionObject& exception)
  {
    if (convertVectorVoxelsToLPS)
    {
      for (int imgIdx = 0; imgIdx < inputImageCollection->GetNumberOfItems(); ++imgIdx)
      {
        vtkImageData* inputImage = vtkImageData::SafeDownCast(inputImageCollection->GetItemAsObject(imgIdx));
        vtkITKImageWriter::ConvertSpatialVectorVoxelsBetweenRasLps(inputImage);
      }
    }
    throw exception;
  }
}

//----------------------------------------------------------------------------
template <class TPixelType>
void ITKWriteVTKImage(vtkITKImageSequenceWriter* self,
                      vtkCollection* inputImageCollection,
                      char* fileName,
                      vtkMatrix4x4* rasToIjkMatrix,
                      vtkMatrix4x4* measurementFrameMatrix = nullptr,
                      int voxelVectorType = vtkITKImageWriter::VoxelVectorTypeUndefined)
{
  // Fix 4 dimensions: 3 spatial + 1 sequence. The fifth dimension for the scalar components is in the pixel type
  ITKWriteVTKImage<TPixelType, 4>(self, inputImageCollection, fileName, rasToIjkMatrix, measurementFrameMatrix, voxelVectorType);
}

//----------------------------------------------------------------------------
vtkITKImageSequenceWriter::vtkITKImageSequenceWriter()
{
  this->Attributes = new AttributeMapType;
  this->AxisLabels = new AxisInfoMapType;
  this->AxisUnits = new AxisInfoMapType;
}

//----------------------------------------------------------------------------
vtkITKImageSequenceWriter::~vtkITKImageSequenceWriter()
{
  this->SetFileName(nullptr);
  this->SetImageIOClassName(nullptr);
  this->SetIntentCode(nullptr);

  delete this->Attributes;
  this->Attributes = nullptr;
  delete this->AxisLabels;
  this->AxisLabels = nullptr;
  delete this->AxisUnits;
  this->AxisUnits = nullptr;
}

//----------------------------------------------------------------------------
void vtkITKImageSequenceWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "FileName: " << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "ImageIOClassName: " << (this->ImageIOClassName ? this->ImageIOClassName : "(none)") << "\n";
}

//------------------------------------------------------------------------------
int vtkITKImageSequenceWriter::FillInputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  return this->Superclass::FillInputPortInformation(port, info);
}

//------------------------------------------------------------------------------
void vtkITKImageSequenceWriter::SetAttribute(const std::string& name, const std::string& value)
{
  if (!this->Attributes)
  {
    return;
  }

  (*this->Attributes)[name] = value;
}

//------------------------------------------------------------------------------
void vtkITKImageSequenceWriter::SetAxisLabel(unsigned int axis, const char* label)
{
  if (!this->AxisLabels)
  {
    return;
  }
  if (label)
  {
    (*this->AxisLabels)[axis] = label;
  }
  else
  {
    this->AxisLabels->erase(axis);
  }
}

//------------------------------------------------------------------------------
void vtkITKImageSequenceWriter::SetAxisUnit(unsigned int axis, const char* unit)
{
  if (!this->AxisUnits)
  {
    return;
  }
  if (unit)
  {
    (*this->AxisUnits)[axis] = unit;
  }
  else
  {
    this->AxisUnits->erase(axis);
  }
}

//------------------------------------------------------------------------------
// Writes all the data from the input.
void vtkITKImageSequenceWriter::Write()
{
  if (!this->FileName)
  {
    vtkErrorMacro(<< "vtkITKImageSequenceWriter: Please specify a FileName");
    return;
  }

  this->UpdateInformation();

  int numberOfImputImages = this->GetNumberOfInputConnections(0);
  int inputDataType = -1;
  int inputNumberOfScalarComponents = -1;
  vtkNew<vtkCollection> inputImageCollection;
  for (int i = 0; i < numberOfImputImages; ++i)
  {
    vtkImageData* inputImage = vtkImageData::SafeDownCast(this->GetInputDataObject(0, i));
    if (inputImage == nullptr)
    {
      vtkErrorMacro(<< "vtkITKImageSequenceWriter: Invalid input object at connection " << i);
      continue;
    }
    vtkPointData* pointData = inputImage->GetPointData();
    if (pointData == nullptr)
    {
      vtkErrorMacro(<< "vtkITKImageSequenceWriter: No image to write at connection " << i);
      continue;
    }
    int currentInputDataType = pointData->GetScalars()   ? pointData->GetScalars()->GetDataType()
                               : pointData->GetTensors() ? pointData->GetTensors()->GetDataType()
                               : pointData->GetVectors() ? pointData->GetVectors()->GetDataType()
                               : pointData->GetNormals() ? pointData->GetNormals()->GetDataType()
                                                         : 0;
    if (inputDataType == -1)
    {
      inputDataType = currentInputDataType;
    }
    else if (currentInputDataType != inputDataType)
    {
      vtkErrorMacro(<< "vtkITKImageSequenceWriter: Data type mismatch at connection " << i << ". Skipping connection.");
      continue;
    }
    int currentInputNumberOfScalarComponents = pointData->GetScalars()   ? pointData->GetScalars()->GetNumberOfComponents()
                                               : pointData->GetTensors() ? pointData->GetTensors()->GetNumberOfComponents()
                                               : pointData->GetVectors() ? pointData->GetVectors()->GetNumberOfComponents()
                                               : pointData->GetNormals() ? pointData->GetNormals()->GetNumberOfComponents()
                                                                         : 0;
    if (inputNumberOfScalarComponents == -1)
    {
      inputNumberOfScalarComponents = currentInputNumberOfScalarComponents;
    }
    else if (currentInputNumberOfScalarComponents != inputNumberOfScalarComponents)
    {
      vtkErrorMacro(<< "vtkITKImageSequenceWriter: Number of components mismatch at connection " << i << ". Skipping connection.");
      continue;
    }

    inputImageCollection->AddItem(inputImage);
  }

  if (this->GetOutputInformation(0))
  {
    this->GetOutputInformation(0)->Set( //
      vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
      this->GetOutputInformation(0)->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()),
      6);
  }

  int voxelVectorType = this->GetVoxelVectorType();
  if (voxelVectorType == vtkITKImageWriter::VoxelVectorTypeSpatial || voxelVectorType == vtkITKImageWriter::VoxelVectorTypeSpatialCovariant)
  {
    if (inputNumberOfScalarComponents != 3)
    {
      vtkWarningMacro(<< "vtkITKImageWriter: VoxelVectorType is set to Spatial or SpatialCovariant, but the input image does not have 3 scalar components.");
      voxelVectorType = vtkITKImageWriter::VoxelVectorTypeUndefined;
    }
  }
  vtkSmartPointer<vtkMatrix4x4> measurementFrameMatrix;
  if (voxelVectorType == vtkITKImageWriter::VoxelVectorTypeSpatial || voxelVectorType == vtkITKImageWriter::VoxelVectorTypeSpatialCovariant)
  {
    if (!measurementFrameMatrix)
    {
      // Set measurement frame matrix to indicate that the vector is spatial.
      measurementFrameMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    }
  }

  try
  {
    if (inputNumberOfScalarComponents == 1)
    {
      // Scalar image
      switch (inputDataType)
      {
        case VTK_DOUBLE: ITKWriteVTKImage<double>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix); break;
        case VTK_FLOAT: ITKWriteVTKImage<float>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix); break;
        case VTK_LONG: ITKWriteVTKImage<long>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix); break;
        case VTK_UNSIGNED_LONG: ITKWriteVTKImage<unsigned long>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix); break;
        case VTK_INT: ITKWriteVTKImage<int>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix); break;
        case VTK_UNSIGNED_INT: ITKWriteVTKImage<unsigned int>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix); break;
        case VTK_SHORT: ITKWriteVTKImage<short>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix); break;
        case VTK_UNSIGNED_SHORT: ITKWriteVTKImage<unsigned short>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix); break;
        case VTK_CHAR: ITKWriteVTKImage<char>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix); break;
        case VTK_UNSIGNED_CHAR: ITKWriteVTKImage<unsigned char>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix); break;
        default: vtkErrorMacro(<< "Execute: Unknown output ScalarType " << inputDataType); return;
      }
    }
    else if (inputNumberOfScalarComponents == 2)
    {
      // 2-component vector image
      switch (inputDataType)
      {
        case VTK_DOUBLE:
        {
          typedef itk::Vector<double, 2> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
          break;
        }
        case VTK_FLOAT:
        {
          typedef itk::Vector<float, 2> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
          break;
        }
        case VTK_LONG:
        {
          typedef itk::Vector<long, 2> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
          break;
        }
        case VTK_UNSIGNED_LONG:
        {
          typedef itk::Vector<unsigned long, 2> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
          break;
        }
        case VTK_INT:
        {
          typedef itk::Vector<int, 2> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
          break;
        }
        case VTK_UNSIGNED_INT:
        {
          typedef itk::Vector<unsigned int, 2> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
          break;
        }
        case VTK_SHORT:
        {
          typedef itk::Vector<short, 2> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
          break;
        }
        case VTK_UNSIGNED_SHORT:
        {
          typedef itk::Vector<unsigned short, 2> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
          break;
        }
        case VTK_CHAR:
        {
          typedef itk::Vector<char, 2> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
          break;
        }
        case VTK_UNSIGNED_CHAR:
        {
          typedef itk::Vector<unsigned char, 2> PixelType;
          ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
          break;
        }
        default: vtkErrorMacro(<< "Execute: Unknown output ScalarType " << inputDataType); return;
      }
    }
    else if (inputNumberOfScalarComponents == 3)
    {
      if (this->VoxelVectorType == vtkITKImageWriter::VoxelVectorTypeColorRGB)
      {
        // RGB image
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
          case VTK_LONG:
          {
            typedef itk::RGBPixel<long> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_UNSIGNED_LONG:
          {
            typedef itk::RGBPixel<unsigned long> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_INT:
          {
            typedef itk::RGBPixel<int> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_UNSIGNED_INT:
          {
            typedef itk::RGBPixel<unsigned int> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_SHORT:
          {
            typedef itk::RGBPixel<short> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_UNSIGNED_SHORT:
          {
            typedef itk::RGBPixel<unsigned short> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_CHAR:
          {
            typedef itk::RGBPixel<char> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_UNSIGNED_CHAR:
          {
            typedef itk::RGBPixel<unsigned char> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          default: vtkErrorMacro(<< "Execute: Unknown output ScalarType " << inputDataType); return;
        }
      }
      else if (this->VoxelVectorType == vtkITKImageWriter::VoxelVectorTypeSpatialCovariant)
      {
        // Covariant spatial vector (such as gradient field)
        switch (inputDataType)
        {
          case VTK_DOUBLE:
          {
            typedef itk::CovariantVector<double, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_FLOAT:
          {
            typedef itk::CovariantVector<float, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_LONG:
          {
            typedef itk::CovariantVector<long, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_UNSIGNED_LONG:
          {
            typedef itk::CovariantVector<unsigned long, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_INT:
          {
            typedef itk::CovariantVector<int, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_UNSIGNED_INT:
          {
            typedef itk::CovariantVector<unsigned int, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_SHORT:
          {
            typedef itk::CovariantVector<short, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_UNSIGNED_SHORT:
          {
            typedef itk::CovariantVector<unsigned short, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_CHAR:
          {
            typedef itk::CovariantVector<char, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_UNSIGNED_CHAR:
          {
            typedef itk::CovariantVector<unsigned char, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          default: vtkErrorMacro(<< "Execute: Unknown output ScalarType " << inputDataType); return;
        }
      }
      else
      {
        // Displacement field, velocity field, or 3-component contravariant vector image
        switch (inputDataType)
        {
          case VTK_DOUBLE:
          {
            typedef itk::Vector<double, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_FLOAT:
          {
            typedef itk::Vector<float, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_LONG:
          {
            typedef itk::Vector<long, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_UNSIGNED_LONG:
          {
            typedef itk::Vector<unsigned long, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_INT:
          {
            typedef itk::Vector<int, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_UNSIGNED_INT:
          {
            typedef itk::Vector<unsigned int, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_SHORT:
          {
            typedef itk::Vector<short, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_UNSIGNED_SHORT:
          {
            typedef itk::Vector<unsigned short, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_CHAR:
          {
            typedef itk::Vector<char, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          case VTK_UNSIGNED_CHAR:
          {
            typedef itk::Vector<unsigned char, 3> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix, measurementFrameMatrix, voxelVectorType);
            break;
          }
          default: vtkErrorMacro(<< "Execute: Unknown output ScalarType " << inputDataType); return;
        }
      }
    }
    else if (inputNumberOfScalarComponents == 4)
    {
      if (this->VoxelVectorType == vtkITKImageWriter::VoxelVectorTypeColorRGBA)
      {
        // RGBA image
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
          case VTK_LONG:
          {
            typedef itk::RGBAPixel<long> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_UNSIGNED_LONG:
          {
            typedef itk::RGBAPixel<unsigned long> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_INT:
          {
            typedef itk::RGBAPixel<int> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_UNSIGNED_INT:
          {
            typedef itk::RGBAPixel<unsigned int> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_SHORT:
          {
            typedef itk::RGBAPixel<short> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_UNSIGNED_SHORT:
          {
            typedef itk::RGBAPixel<unsigned short> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_CHAR:
          {
            typedef itk::RGBAPixel<char> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_UNSIGNED_CHAR:
          {
            typedef itk::RGBAPixel<unsigned char> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          default: vtkErrorMacro(<< "Execute: Unknown output ScalarType " << inputDataType); return;
        }
      }
      else
      {
        // Other 4-component vector image
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
          case VTK_LONG:
          {
            typedef itk::Vector<long, 4> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_UNSIGNED_LONG:
          {
            typedef itk::Vector<unsigned long, 4> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_INT:
          {
            typedef itk::Vector<int, 4> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_UNSIGNED_INT:
          {
            typedef itk::Vector<unsigned int, 4> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_SHORT:
          {
            typedef itk::Vector<short, 4> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_UNSIGNED_SHORT:
          {
            typedef itk::Vector<unsigned short, 4> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_CHAR:
          {
            typedef itk::Vector<char, 4> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          case VTK_UNSIGNED_CHAR:
          {
            typedef itk::Vector<unsigned char, 4> PixelType;
            ITKWriteVTKImage<PixelType>(this, inputImageCollection, this->GetFileName(), this->RasToIJKMatrix);
            break;
          }
          default: vtkErrorMacro(<< "Execute: Unknown output ScalarType " << inputDataType); return;
        }
      }
    }
    else
    {
      vtkErrorMacro(<< "Can only export 1, 3, or 4 component images, current image has " << inputNumberOfScalarComponents << " components");
      return;
    }
  }
  catch (itk::ExceptionObject& exception)
  {
    vtkErrorMacro("Failed to write image: " << exception.GetDescription());
    return;
  }
}
