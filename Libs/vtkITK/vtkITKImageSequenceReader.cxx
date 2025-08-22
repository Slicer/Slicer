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

// VTKITK includes
#include "vtkITKImageSequenceReader.h"
#include "vtkITKArchetypeImageSeriesReader.h"

// VTK includes
#include <vtkErrorCode.h>
#include "vtkImageExtractComponents.h"
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>

// ITK includes
#include "itkExtractImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageToVTKImageFilter.h"
#include "itkNrrdImageIO.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

#define KEY_PREFIX "NRRD_"

vtkStandardNewMacro(vtkITKImageSequenceReader);

//----------------------------------------------------------------------------
vtkITKImageSequenceReader::vtkITKImageSequenceReader() {}

//----------------------------------------------------------------------------
vtkITKImageSequenceReader::~vtkITKImageSequenceReader()
{
  if (RasToIjkMatrix)
  {
    this->RasToIjkMatrix->Delete();
    this->RasToIjkMatrix = nullptr;
  }
}

//----------------------------------------------------------------------------
void vtkITKImageSequenceReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Header key/value pairs:\n";
  for (std::map<std::string, std::string>::iterator it = HeaderKeyValueMap.begin(); it != HeaderKeyValueMap.end(); ++it)
  {
    os << indent.GetNextIndent() << it->first << ": " << it->second << "\n";
  }
  os << indent << "Axis labels:\n";
  for (std::map<unsigned int, std::string>::iterator it = AxisLabels.begin(); it != AxisLabels.end(); ++it)
  {
    os << indent.GetNextIndent() << "Axis " << it->first << ": " << it->second << "\n";
  }
  os << indent << "Axis units:\n";
  for (std::map<unsigned int, std::string>::iterator it = AxisUnits.begin(); it != AxisUnits.end(); ++it)
  {
    os << indent.GetNextIndent() << "Axis " << it->first << ": " << it->second << "\n";
  }
  os << indent << "RasToIjkMatrix:\n";
  if (this->RasToIjkMatrix)
  {
    this->RasToIjkMatrix->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << indent.GetNextIndent() << "(none)\n";
  }
}

//----------------------------------------------------------------------------
const std::map<std::string, std::string> vtkITKImageSequenceReader::GetHeaderKeysMap()
{
  return this->HeaderKeyValueMap;
}

//----------------------------------------------------------------------------
const std::vector<std::string> vtkITKImageSequenceReader::GetHeaderKeysVector()
{
  std::vector<std::string> keys;
  for (std::map<std::string, std::string>::iterator it = HeaderKeyValueMap.begin(); it != HeaderKeyValueMap.end(); ++it)
  {
    keys.push_back(it->first);
  }
  return keys;
}

//----------------------------------------------------------------------------
const char* vtkITKImageSequenceReader::GetHeaderValue(const char* key)
{
  std::map<std::string, std::string>::iterator it = HeaderKeyValueMap.find(key);
  if (it != HeaderKeyValueMap.end())
  {
    return (it->second.c_str());
  }
  else
  {
    return nullptr;
  }
}

//----------------------------------------------------------------------------
const char* vtkITKImageSequenceReader::GetAxisLabel(unsigned int axis)
{
  if (this->AxisLabels.find(axis) == this->AxisLabels.end())
  {
    return nullptr;
  }
  return this->AxisLabels[axis].c_str();
}

//----------------------------------------------------------------------------
const char* vtkITKImageSequenceReader::GetAxisUnit(unsigned int axis)
{
  if (this->AxisUnits.find(axis) == this->AxisUnits.end())
  {
    return nullptr;
  }
  return this->AxisUnits[axis].c_str();
}

//----------------------------------------------------------------------------
template <class TPixelType, int Dimension>
void vtkITKExecuteDataFromFile(vtkITKImageSequenceReader* self, std::vector<vtkSmartPointer<vtkImageData>>& images, int listDimIdx, int voxelVectorType)
{
  using PixelType = TPixelType;
  constexpr unsigned int ImageDimension = Dimension;
  using ImageType = itk::Image<PixelType, ImageDimension>;

  using ReaderType = itk::ImageFileReader<ImageType>;
  typename ReaderType::Pointer reader = ReaderType::New();

  using ImageIOType = itk::NrrdImageIO;
  ImageIOType::Pointer imageIO = ImageIOType::New();
  imageIO->SetAxesReorderToUseNonListRangeAxisAsPixel();
  reader->SetImageIO(imageIO);

  reader->SetFileName(self->GetFileName());
  reader->Update();
  typename ImageType::ConstPointer image = reader->GetOutput();

  // Get IJK to LPS matrix
  vtkNew<vtkMatrix4x4> ijkToLpsMatrix;
  for (int i = 0; i < 3; i++)
  {
    double spacing = image->GetSpacing()[i];
    double origin = image->GetOrigin()[i];
    for (unsigned int j = 0; j < 3; j++)
    {
      ijkToLpsMatrix->SetElement(j, i, spacing * image->GetDirection()[j][i]);
    }
    ijkToLpsMatrix->SetElement(i, 3, origin);
  }

  // Transform from LPS to RAS
  vtkNew<vtkMatrix4x4> lpsToRasMatrix;
  lpsToRasMatrix->SetElement(0, 0, -1);
  lpsToRasMatrix->SetElement(1, 1, -1);

  vtkNew<vtkMatrix4x4> ijkToRasMatrix;
  vtkMatrix4x4::Multiply4x4(lpsToRasMatrix, ijkToLpsMatrix, ijkToRasMatrix);

  vtkNew<vtkMatrix4x4> rasToIjkMatrix;
  vtkMatrix4x4::Invert(ijkToRasMatrix, rasToIjkMatrix);
  self->SetRasToIjkMatrix(rasToIjkMatrix);

  // Extract requested frame from image
  using FrameImageType = itk::Image<PixelType, ImageDimension - 1>;
  using ExtractImageFilterType = itk::ExtractImageFilter<ImageType, FrameImageType>;
  typename ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();

  extractImageFilter->SetInput(image);

  typename ImageType::RegionType fullRegion = image->GetLargestPossibleRegion();
  typename ImageType::SizeType extractionSize = fullRegion.GetSize();
  self->SetNumberOfFrames(extractionSize[listDimIdx]);

  extractionSize[listDimIdx] = 0; // Collapse sequence dimension when extracting frame

  typename ImageType::RegionType extractionRegion;
  typename ImageType::IndexType extractionIndex = extractionRegion.GetIndex();
  extractionRegion.SetSize(extractionSize);
  extractImageFilter->SetDirectionCollapseToSubmatrix();

  using VTKExporterFilterType = itk::ImageToVTKImageFilter<FrameImageType>;
  typename VTKExporterFilterType::Pointer vtkExportFilter = VTKExporterFilterType::New();

  images.clear();
  for (unsigned int frameIndex = 0; frameIndex < self->GetNumberOfFrames(); frameIndex++)
  {
    extractionIndex[listDimIdx] = frameIndex;
    extractionRegion.SetIndex(extractionIndex);
    extractImageFilter->SetExtractionRegion(extractionRegion);
    extractImageFilter->Update();

    typename FrameImageType::Pointer frameImage = extractImageFilter->GetOutput();
    typename FrameImageType::RegionType frameRegion = frameImage->GetLargestPossibleRegion();

    // Convert extracted frame to VTK image
    vtkExportFilter->SetInput(frameImage);
    vtkExportFilter->Update();

    vtkSmartPointer<vtkImageData> cachedImage = vtkSmartPointer<vtkImageData>::New();
    // Copy to output. Note: crashes if ShallowCopy is used
    cachedImage->DeepCopy(vtkExportFilter->GetOutput());

    if (voxelVectorType == vtkITKImageWriter::VoxelVectorTypeSpatial || voxelVectorType == vtkITKImageWriter::VoxelVectorTypeSpatialCovariant)
    {
      vtkITKImageWriter::ConvertSpatialVectorVoxelsBetweenRasLps(cachedImage);
    }

    images.push_back(cachedImage);
  }
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The data extent/axes
// are assumed to be the same as the file extent/order.
void vtkITKImageSequenceReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{
  this->HeaderKeyValueMap.clear();
  this->AxisLabels.clear();
  this->AxisUnits.clear();
  this->SequenceAxisLabel.clear();
  this->SequenceAxisUnit.clear();

  if (this->FileName == nullptr)
  {
    vtkErrorMacro("A file name must be specified.");
    this->SetErrorCode(vtkErrorCode::NoFileNameError);
    return;
  }

  vtkImageData* data = vtkImageData::SafeDownCast(output);
  data->SetExtent(0, -1, 0, -1, 0, -1);
  data->AllocateScalars(outInfo);
  data->SetExtent(outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));

  bool measurementFrameMatrixExplicitlySpecified = false;

  try
  {
    // Read header to decide dimensions, pixel type, and scalar type

    // Create an NRRD image IO object
    using ImageIOType = itk::NrrdImageIO;
    ImageIOType::Pointer imageIO = ImageIOType::New();
    imageIO->SetAxesReorderToUseNonListRangeAxisAsPixel();
    // Read metadata
    if (!imageIO->CanReadFile(this->GetFileName()))
    {
      vtkErrorMacro("Cannot read the NRRD file: " << this->GetFileName());
      this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
      return;
    }
    imageIO->SetFileName(this->GetFileName());
    imageIO->ReadImageInformation(); // Read only the header information

    // Read the relevant header information
    itk::MetaDataDictionary thisDic = imageIO->GetMetaDataDictionary();
    std::vector<std::string> keys = thisDic.GetKeys();
    std::vector<std::string>::const_iterator keyIt;
    const char* keyField;
    unsigned int axi;
    unsigned int baseDim = (imageIO->GetNumberOfComponents() > 1 ? 1 : 0);
    int listDim{ -1 }; // List dimension is the (first) dimension of the list of frames, if any
    for (keyIt = keys.begin(); keyIt != keys.end(); ++keyIt)
    {
      // Check for NRRD specific keys
      std::string value;
      if (!strncmp(KEY_PREFIX, keyIt->c_str(), strlen(KEY_PREFIX)))
      {
        keyField = keyIt->c_str() + strlen(KEY_PREFIX);
        if (!strncmp(keyField, "kinds", strlen("kinds")))
        {
          if (1 == sscanf(keyField + strlen("kinds"), "[%u]", &axi) && axi < imageIO->GetNumberOfDimensions() + baseDim && listDim == -1)
          {
            itk::ExposeMetaData<std::string>(thisDic, *keyIt, value);
            if (value == "list")
            {
              listDim = axi; // Save first list dimension index
            }
          }
        }
        else if (!strncmp(keyField, "labels", strlen("labels")))
        {
          if (1 == sscanf(keyField + strlen("labels"), "[%u]", &axi) && axi < imageIO->GetNumberOfDimensions() + baseDim)
          {
            itk::ExposeMetaData<std::string>(thisDic, *keyIt, value);
            this->AxisLabels[axi] = value;
          }
        }
        else if (!strncmp(keyField, "units", strlen("units")))
        {
          if (1 == sscanf(keyField + strlen("units"), "[%u]", &axi) && axi < imageIO->GetNumberOfDimensions() + baseDim)
          {
            itk::ExposeMetaData<std::string>(thisDic, *keyIt, value);
            this->AxisUnits[axi] = value;
          }
        }
      }
      else
      {
        // Read pre-defined keys for NRRDs in ITK
        itk::ExposeMetaData<std::string>(thisDic, *keyIt, value);
        if (!(*keyIt).compare("RangeAxisLabel"))
        {
          this->SequenceAxisLabel = value;
        }
        else if (!(*keyIt).compare("RangeAxisUnit"))
        {
          this->SequenceAxisUnit = value;
        }
        else if (!(*keyIt).compare("ITK_InputFilterName"))
        {
          continue; // There is always a key "ITK_InputFilterName" in the header, skip it
        }
        else // Free-form key/value pairs
        {
          this->HeaderKeyValueMap[keyIt->c_str()] = value;
        }
      }
    } // For all keys in the metadata dictionary

    // Set SequenceAxisLabel and SequenceAxisUnit from AxisLabels and AxisUnits, respectively,
    // if the frames are stored in the dimensions instead of the components
    if (listDim >= 0)
    {
      this->SequenceAxisLabel = this->AxisLabels[listDim];
      this->SequenceAxisUnit = this->AxisUnits[listDim];
    }

    if (this->Debug)
    {
      // Print relevant image metadata (kept for debugging when adding support for new data types)
      unsigned int dimension = imageIO->GetNumberOfDimensions();
      std::cerr << "Dimensions: " << dimension << std::endl;
      for (unsigned int i = 0; i < dimension; ++i)
      {
        std::cerr << "Size of dimension " << i << ": " << imageIO->GetDimensions(i) << std::endl;
      }
      std::cerr << "Pixel Type: " << imageIO->GetPixelTypeAsString(imageIO->GetPixelType()) << std::endl;
      std::cerr << "Component Type: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()) << std::endl;
    }

    // Measurement frame
    vtkNew<vtkMatrix4x4> measurementFrameMatrix;
    if (vtkITKArchetypeImageSeriesReader::ReadMeasurementFrameMatrixFromMetaDataDictionary(thisDic, measurementFrameMatrix))
    {
      measurementFrameMatrixExplicitlySpecified = true;
    }

    bool isPixelAxisListKind = vtkITKArchetypeImageSeriesReader::IsListPixelComponentTypeInMetaDataDictionary(thisDic);

    // Load image from file
    switch (imageIO->GetNumberOfDimensions())
    {
      case 3:
        switch (imageIO->GetPixelType())
        {
          case itk::CommonEnums::IOPixel::RGB:
            this->SetVoxelVectorType(vtkITKImageWriter::VoxelVectorTypeColorRGB);
            if (imageIO->GetComponentType() != itk::ImageIOBase::IOComponentEnum::UCHAR)
            {
              vtkErrorMacro("Unexpected component type for RGB voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
              this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
              return;
            }
            vtkITKExecuteDataFromFile<itk::RGBPixel<unsigned char>, 3>(this, this->CachedImages, listDim, this->VoxelVectorType);
            break;
          case itk::CommonEnums::IOPixel::RGBA:
            this->SetVoxelVectorType(vtkITKImageWriter::VoxelVectorTypeColorRGBA);
            if (imageIO->GetComponentType() != itk::ImageIOBase::IOComponentEnum::UCHAR)
            {
              vtkErrorMacro("Unexpected component type for RGBA voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
              this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
              return;
            }
            vtkITKExecuteDataFromFile<itk::RGBAPixel<unsigned char>, 3>(this, this->CachedImages, listDim, this->VoxelVectorType);
            break;
          case itk::CommonEnums::IOPixel::VECTOR:
            if (measurementFrameMatrixExplicitlySpecified && !isPixelAxisListKind)
            {
              this->VoxelVectorType = vtkITKImageWriter::VoxelVectorTypeSpatial;
            }
            else
            {
              // If measurement frame was not explicitly specified, then we cannot be sure the vector is spatial
              this->VoxelVectorType = vtkITKImageWriter::VoxelVectorTypeUndefined;
            }
            switch (imageIO->GetComponentType())
            {
              case itk::ImageIOBase::IOComponentEnum::UCHAR: //
                vtkITKExecuteDataFromFile<itk::Vector<unsigned char>, 3>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::USHORT: //
                vtkITKExecuteDataFromFile<itk::Vector<unsigned short>, 3>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::INT: //
                vtkITKExecuteDataFromFile<itk::Vector<int>, 3>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::FLOAT: //
                vtkITKExecuteDataFromFile<itk::Vector<float>, 3>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::DOUBLE: //
                vtkITKExecuteDataFromFile<itk::Vector<double>, 3>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              default:
                vtkErrorMacro("Unexpected component type for 4 or less component vector voxel: " //
                              << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
                this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
                return;
            }
            break;
        }
        break;
      case 4:
        switch (imageIO->GetPixelType())
        {
          case itk::CommonEnums::IOPixel::SCALAR:
            switch (imageIO->GetComponentType())
            {
              case itk::ImageIOBase::IOComponentEnum::UCHAR: //
                vtkITKExecuteDataFromFile<unsigned char, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::CHAR: //
                vtkITKExecuteDataFromFile<char, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::USHORT: //
                vtkITKExecuteDataFromFile<unsigned short, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::SHORT: //
                vtkITKExecuteDataFromFile<short, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::INT: //
                vtkITKExecuteDataFromFile<int, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::UINT: //
                vtkITKExecuteDataFromFile<unsigned int, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::FLOAT: //
                vtkITKExecuteDataFromFile<float, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::DOUBLE: //
                vtkITKExecuteDataFromFile<double, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              default:
                vtkErrorMacro("Unexpected component type for scalar voxel: " //
                              << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
                this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
                return;
            }
            break;
          case itk::CommonEnums::IOPixel::RGB:
            this->SetVoxelVectorType(vtkITKImageWriter::VoxelVectorTypeColorRGB);
            if (imageIO->GetComponentType() != itk::ImageIOBase::IOComponentEnum::UCHAR)
            {
              vtkErrorMacro("Unexpected component type for RGB voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
              this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
              return;
            }
            vtkITKExecuteDataFromFile<itk::RGBPixel<unsigned char>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
            break;
          case itk::CommonEnums::IOPixel::RGBA:
            this->SetVoxelVectorType(vtkITKImageWriter::VoxelVectorTypeColorRGBA);
            if (imageIO->GetComponentType() != itk::ImageIOBase::IOComponentEnum::UCHAR)
            {
              vtkErrorMacro("Unexpected component type for RGBA voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
              this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
              return;
            }
            vtkITKExecuteDataFromFile<itk::RGBAPixel<unsigned char>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
            break;
          case itk::CommonEnums::IOPixel::VECTOR:
            if (measurementFrameMatrixExplicitlySpecified)
            {
              this->VoxelVectorType = vtkITKImageWriter::VoxelVectorTypeSpatial;
            }
            else
            {
              // If measurement frame was not explicitly specified, then we cannot be sure the vector is spatial
              this->VoxelVectorType = vtkITKImageWriter::VoxelVectorTypeUndefined;
            }
            switch (imageIO->GetNumberOfComponents())
            {
              case 3:
                switch (imageIO->GetComponentType())
                {
                  case itk::ImageIOBase::IOComponentEnum::UCHAR: //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned char>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::USHORT: //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned short>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::INT: //
                    vtkITKExecuteDataFromFile<itk::Vector<int>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::FLOAT: //
                    vtkITKExecuteDataFromFile<itk::Vector<float>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::DOUBLE: //
                    vtkITKExecuteDataFromFile<itk::Vector<double>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                    break;
                  default:
                    vtkErrorMacro("Unexpected component type for vector voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
                    this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
                    return;
                }
                break;
              case 4:
                this->SetVoxelVectorType(vtkITKImageWriter::VoxelVectorTypeUndefined);
                switch (imageIO->GetComponentType())
                {
                  case itk::ImageIOBase::IOComponentEnum::UCHAR: //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned char, 4>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::USHORT: //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned short, 4>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::INT: //
                    vtkITKExecuteDataFromFile<itk::Vector<int, 4>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::FLOAT: //
                    vtkITKExecuteDataFromFile<itk::Vector<float, 4>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::DOUBLE: //
                    vtkITKExecuteDataFromFile<itk::Vector<double, 4>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                    break;
                  default:
                    vtkErrorMacro("Unexpected component type for vector voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
                    this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
                    return;
                }
                break;
            }
            break;
          case itk::CommonEnums::IOPixel::COVARIANTVECTOR:
            this->SetVoxelVectorType(vtkITKImageWriter::VoxelVectorTypeSpatialCovariant);
            switch (imageIO->GetComponentType())
            {
              case itk::ImageIOBase::IOComponentEnum::UCHAR: //
                vtkITKExecuteDataFromFile<itk::CovariantVector<unsigned char>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::USHORT: //
                vtkITKExecuteDataFromFile<itk::CovariantVector<unsigned short>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::INT: //
                vtkITKExecuteDataFromFile<itk::CovariantVector<int>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::FLOAT: //
                vtkITKExecuteDataFromFile<itk::CovariantVector<float>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::DOUBLE: //
                vtkITKExecuteDataFromFile<itk::CovariantVector<double>, 4>(this, this->CachedImages, listDim, this->VoxelVectorType);
                break;
              default:
                vtkErrorMacro("Unexpected component type for covariant vector voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
                this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
                return;
            }
            break;
        }
        break;
    } // Load image from file the sequence frames either in the last dimension or as components, depending on the pixel type
  }
  catch (itk::InvalidArgumentError& e)
  {
    vtkDebugMacro(<< "Could not read file as sequence" << e);
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    // return successful read, because this is an expected error when the user
    // has selected a file that doesn't happen to be a sequence.  So it's a file
    // format error, but not something that should trigger a VTK pipeline error
    // (at least not as used in vtkMRMLStorageNodes).
    return;
  }
  catch (itk::ExceptionObject& e)
  {
    vtkErrorMacro(<< "Exception from vtkITK when reading image sequence: " << e << "\n");
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    return;
  }

  vtkImageData* loadedImage = this->GetCachedImage(this->GetCurrentFrameIndex());
  if (loadedImage && data)
  {
    data->DeepCopy(loadedImage);
  }
}

//----------------------------------------------------------------------------
unsigned int vtkITKImageSequenceReader::GetNumberOfCachedImages()
{
  return this->CachedImages.size();
}

//----------------------------------------------------------------------------
vtkImageData* vtkITKImageSequenceReader::GetCachedImage(unsigned int index)
{
  if (index >= this->CachedImages.size())
  {
    return nullptr;
  }
  return this->CachedImages[index];
}

//----------------------------------------------------------------------------
void vtkITKImageSequenceReader::ClearCachedImages()
{
  this->CachedImages.clear();
}
