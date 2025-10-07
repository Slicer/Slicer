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
void vtkITKExecuteDataFromFile(vtkITKImageSequenceReader* self,
                               std::vector<vtkSmartPointer<vtkImageData>>& images,
                               int listDimIdx,
                               itk::NrrdImageIOEnums::AxesReorder axesReorder,
                               int voxelVectorType)
{
  if (listDimIdx < 0)
  {
    vtkErrorWithObjectMacro(self, "vtkITKImageSequenceReader::vtkITKExecuteDataFromFile failed: invalid listDimIdx value " << listDimIdx);
    images.clear();
    return;
  }

  using PixelType = TPixelType;
  constexpr unsigned int ImageDimension = Dimension;
  using ImageType = itk::Image<PixelType, ImageDimension>;

  using ReaderType = itk::ImageFileReader<ImageType>;
  typename ReaderType::Pointer reader = ReaderType::New();

  using ImageIOType = itk::NrrdImageIO;
  ImageIOType::Pointer imageIO = ImageIOType::New();
  imageIO->SetAxesReorder(axesReorder);
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

    // Backward-compatibility for legacy NRRD writers:
    // Some 3D+t sequences were encoded with kinds = {vector, domain, domain, domain},
    // i.e., no "list" axis. In these files the leading "vector" axis does not represent
    // per-voxel components, but the temporal/range axis (frames).
    //
    // If no axis of kind "list" is present but a "vector" axis is present
    // (i.e., the image reports multiple scalar components), interpret the data
    // as a sequence of scalar volumes by forcing scalar pixels
    // (AxesReorderToUseScalarPixel) and re-reading the header. This preserves
    // the intended 3D+t semantics and avoids crashes.
    bool foundListAxis = false;
    {
      itk::MetaDataDictionary thisDic = imageIO->GetMetaDataDictionary();
      std::vector<std::string> keys = thisDic.GetKeys();
      const unsigned int baseDim = (imageIO->GetNumberOfComponents() > 1 ? 1 : 0);
      const std::string nrrdKindsPrefix = std::string(KEY_PREFIX) + "kinds";
      for (std::vector<std::string>::const_iterator keyIt = keys.begin(); keyIt != keys.end(); ++keyIt)
      {
        if (!strncmp(nrrdKindsPrefix.c_str(), keyIt->c_str(), nrrdKindsPrefix.size()))
        {
          // Key is one of NRRD_kinds[0], NRRD_kinds[1], ...
          unsigned int axi = -1;
          if ((sscanf(keyIt->c_str() + nrrdKindsPrefix.size(), "[%u]", &axi) == 1) && (axi < imageIO->GetNumberOfDimensions() + baseDim))
          {
            std::string value;
            itk::ExposeMetaData<std::string>(thisDic, *keyIt, value);
            if (value == "list")
            {
              foundListAxis = true;
              break;
            }
          }
        }
      }
      if (!foundListAxis && (imageIO->GetNumberOfComponents() > 1))
      {
        // No list axis found but there are multiple scalar components.
        // Let's interpret this image as a series of scalar volumes.
        imageIO->SetAxesReorderToUseScalarPixel();
        imageIO->ReadImageInformation(); // Read only the header information
      }
    }

    // Read the relevant header information
    itk::MetaDataDictionary thisDic = imageIO->GetMetaDataDictionary();
    std::vector<std::string> keys = thisDic.GetKeys();
    unsigned int baseDim = (imageIO->GetNumberOfComponents() > 1 ? 1 : 0);
    std::map<unsigned int, std::string> axisKinds;
    for (std::vector<std::string>::const_iterator keyIt = keys.begin(); keyIt != keys.end(); ++keyIt)
    {
      // Check for NRRD specific keys
      std::string value;
      unsigned int axi = -1;
      if (!strncmp(KEY_PREFIX, keyIt->c_str(), strlen(KEY_PREFIX)))
      {
        const char* keyField = keyIt->c_str() + strlen(KEY_PREFIX);
        if (!strncmp(keyField, "kinds", strlen("kinds")))
        {
          if ((sscanf(keyField + strlen("kinds"), "[%u]", &axi) == 1) && (axi < imageIO->GetNumberOfDimensions() + baseDim))
          {
            itk::ExposeMetaData<std::string>(thisDic, *keyIt, value);
            axisKinds[axi] = value;
          }
        }
        else if (!strncmp(keyField, "labels", strlen("labels")))
        {
          if ((sscanf(keyField + strlen("labels"), "[%u]", &axi) == 1) && (axi < imageIO->GetNumberOfDimensions() + baseDim))
          {
            itk::ExposeMetaData<std::string>(thisDic, *keyIt, value);
            this->AxisLabels[axi] = value;
          }
        }
        else if (!strncmp(keyField, "units", strlen("units")))
        {
          if ((sscanf(keyField + strlen("units"), "[%u]", &axi) == 1) && (axi < imageIO->GetNumberOfDimensions() + baseDim))
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

    // Identify the sequence (range) axis:
    // Prefer the first axis with kind "list". If none exists, fall back to the first
    // axis with kind "vector" (legacy files that mislabeled the range axis as "vector").
    // Note: this fallback only applies when a proper "list" axis is absent.
    int listDim{ -1 };
    for (unsigned int axisIndex = 0; axisIndex < imageIO->GetNumberOfDimensions(); ++axisIndex)
    {
      // "list" kind is always accepted, "vector" is only accepted if there is no "list" kind
      if (axisKinds[axisIndex] == "list" || (!foundListAxis && (axisKinds[axisIndex] == "vector")))
      {
        listDim = axisIndex;
        break;
      }
    }
    if (listDim < 0)
    {
      vtkErrorMacro("Could not find list kind axis in image file");
      this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
      return;
    }

    if (imageIO->GetNumberOfDimensions() > 4)
    {
      vtkErrorMacro("Image reading failed: images up to 5 dimensions (1 pixel component, 3 domain, 1 list) are supported."
                    << " This image has " << imageIO->GetNumberOfDimensions() + baseDim << " dimensions.");
      this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
      return;
    }

    // Set SequenceAxisLabel and SequenceAxisUnit from AxisLabels and AxisUnits, respectively,
    // if the frames are stored in the dimensions instead of the components
    this->SequenceAxisLabel = this->AxisLabels[listDim];
    this->SequenceAxisUnit = this->AxisUnits[listDim];

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
            vtkITKExecuteDataFromFile<itk::RGBPixel<unsigned char>, 3>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
            break;
          case itk::CommonEnums::IOPixel::RGBA:
            this->SetVoxelVectorType(vtkITKImageWriter::VoxelVectorTypeColorRGBA);
            if (imageIO->GetComponentType() != itk::ImageIOBase::IOComponentEnum::UCHAR)
            {
              vtkErrorMacro("Unexpected component type for RGBA voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
              this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
              return;
            }
            vtkITKExecuteDataFromFile<itk::RGBAPixel<unsigned char>, 3>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
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
              case itk::ImageIOBase::IOComponentEnum::DOUBLE: //
                vtkITKExecuteDataFromFile<itk::Vector<double>, 3>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::FLOAT: //
                vtkITKExecuteDataFromFile<itk::Vector<float>, 3>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::LONG: //
                vtkITKExecuteDataFromFile<itk::Vector<long>, 3>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::ULONG: //
                vtkITKExecuteDataFromFile<itk::Vector<unsigned long>, 3>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::INT: //
                vtkITKExecuteDataFromFile<itk::Vector<int>, 3>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::UINT: //
                vtkITKExecuteDataFromFile<itk::Vector<unsigned int>, 3>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::SHORT: //
                vtkITKExecuteDataFromFile<itk::Vector<short>, 3>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::USHORT: //
                vtkITKExecuteDataFromFile<itk::Vector<unsigned short>, 3>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::CHAR: //
                vtkITKExecuteDataFromFile<itk::Vector<char>, 3>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::UCHAR: //
                vtkITKExecuteDataFromFile<itk::Vector<unsigned char>, 3>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              default:
                vtkErrorMacro("Unexpected component type for 4 or less component vector voxel: " //
                              << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
                this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
                return;
            }
            break;
          case itk::CommonEnums::IOPixel::SCALAR:
          case itk::CommonEnums::IOPixel::COVARIANTVECTOR:
          case itk::CommonEnums::IOPixel::UNKNOWNPIXELTYPE:
          case itk::CommonEnums::IOPixel::OFFSET:
          case itk::CommonEnums::IOPixel::POINT:
          case itk::CommonEnums::IOPixel::SYMMETRICSECONDRANKTENSOR:
          case itk::CommonEnums::IOPixel::DIFFUSIONTENSOR3D:
          case itk::CommonEnums::IOPixel::COMPLEX:
          case itk::CommonEnums::IOPixel::FIXEDARRAY:
          case itk::CommonEnums::IOPixel::ARRAY:
          case itk::CommonEnums::IOPixel::MATRIX:
          case itk::CommonEnums::IOPixel::VARIABLELENGTHVECTOR:
          case itk::CommonEnums::IOPixel::VARIABLESIZEMATRIX:
          {
            vtkErrorMacro("Unexpected pixel type: " << imageIO->GetPixelType());
            this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
            return;
          }
          break;
          default:
            vtkErrorMacro("Unexpected pixel type code requested: " << imageIO->GetPixelType());
            this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
            return;
        }
        break;
      case 4:
        switch (imageIO->GetPixelType())
        {
          case itk::CommonEnums::IOPixel::SCALAR:
            switch (imageIO->GetComponentType())
            {
              case itk::ImageIOBase::IOComponentEnum::DOUBLE: //
                vtkITKExecuteDataFromFile<double, 4>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::FLOAT: //
                vtkITKExecuteDataFromFile<float, 4>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::LONG: //
                vtkITKExecuteDataFromFile<long, 4>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::ULONG: //
                vtkITKExecuteDataFromFile<unsigned long, 4>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::INT: //
                vtkITKExecuteDataFromFile<int, 4>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::UINT: //
                vtkITKExecuteDataFromFile<unsigned int, 4>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::SHORT: //
                vtkITKExecuteDataFromFile<short, 4>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::USHORT: //
                vtkITKExecuteDataFromFile<unsigned short, 4>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::CHAR: //
                vtkITKExecuteDataFromFile<char, 4>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::UCHAR: //
                vtkITKExecuteDataFromFile<unsigned char, 4>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
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
            vtkITKExecuteDataFromFile<itk::RGBPixel<unsigned char>, 4>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
            break;
          case itk::CommonEnums::IOPixel::RGBA:
            this->SetVoxelVectorType(vtkITKImageWriter::VoxelVectorTypeColorRGBA);
            if (imageIO->GetComponentType() != itk::ImageIOBase::IOComponentEnum::UCHAR)
            {
              vtkErrorMacro("Unexpected component type for RGBA voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
              this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
              return;
            }
            vtkITKExecuteDataFromFile<itk::RGBAPixel<unsigned char>, 4>(this, this->CachedImages, listDim, imageIO->GetAxesReorder(), this->VoxelVectorType);
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
              case 2:
                this->SetVoxelVectorType(vtkITKImageWriter::VoxelVectorTypeUndefined);
                switch (imageIO->GetComponentType())
                {
                  case itk::ImageIOBase::IOComponentEnum::DOUBLE:         //
                    vtkITKExecuteDataFromFile<itk::Vector<double, 2>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::FLOAT:         //
                    vtkITKExecuteDataFromFile<itk::Vector<float, 2>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::LONG:         //
                    vtkITKExecuteDataFromFile<itk::Vector<long, 2>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::ULONG:                 //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned long, 2>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::INT:         //
                    vtkITKExecuteDataFromFile<itk::Vector<int, 2>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::UINT:                 //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned int, 2>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::SHORT:         //
                    vtkITKExecuteDataFromFile<itk::Vector<short, 2>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::USHORT:                 //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned short, 2>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::CHAR:         //
                    vtkITKExecuteDataFromFile<itk::Vector<char, 2>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::UCHAR:                 //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned char, 2>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  default:
                    vtkErrorMacro("Unexpected component type for vector voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
                    this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
                    return;
                }
                break;
              case 3:
                switch (imageIO->GetComponentType())
                {
                  case itk::ImageIOBase::IOComponentEnum::DOUBLE:      //
                    vtkITKExecuteDataFromFile<itk::Vector<double>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::FLOAT:      //
                    vtkITKExecuteDataFromFile<itk::Vector<float>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::LONG:      //
                    vtkITKExecuteDataFromFile<itk::Vector<long>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::ULONG:              //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned long>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::INT:      //
                    vtkITKExecuteDataFromFile<itk::Vector<int>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::UINT:              //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned int>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::SHORT:      //
                    vtkITKExecuteDataFromFile<itk::Vector<short>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::USHORT:              //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned short>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::CHAR:      //
                    vtkITKExecuteDataFromFile<itk::Vector<char>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::UCHAR:              //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned char>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
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
                  case itk::ImageIOBase::IOComponentEnum::DOUBLE:         //
                    vtkITKExecuteDataFromFile<itk::Vector<double, 4>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::FLOAT:         //
                    vtkITKExecuteDataFromFile<itk::Vector<float, 4>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::LONG:         //
                    vtkITKExecuteDataFromFile<itk::Vector<long, 4>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::ULONG:                 //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned long, 4>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::INT:         //
                    vtkITKExecuteDataFromFile<itk::Vector<int, 4>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::UINT:                 //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned int, 4>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::SHORT:         //
                    vtkITKExecuteDataFromFile<itk::Vector<short, 4>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::USHORT:                 //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned short, 4>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::CHAR:         //
                    vtkITKExecuteDataFromFile<itk::Vector<char, 4>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  case itk::ImageIOBase::IOComponentEnum::UCHAR:                 //
                    vtkITKExecuteDataFromFile<itk::Vector<unsigned char, 4>, 4>( //
                      this,
                      this->CachedImages,
                      listDim,
                      imageIO->GetAxesReorder(),
                      this->VoxelVectorType);
                    break;
                  default:
                    vtkErrorMacro("Unexpected component type for vector voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
                    this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
                    return;
                }
                break;
              default:
                vtkErrorMacro("Unexpected number of components for vector voxel: " << imageIO->GetNumberOfComponents());
                this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
                return;
            }
            break;
          case itk::CommonEnums::IOPixel::COVARIANTVECTOR:
            this->SetVoxelVectorType(vtkITKImageWriter::VoxelVectorTypeSpatialCovariant);
            switch (imageIO->GetComponentType())
            {
              case itk::ImageIOBase::IOComponentEnum::DOUBLE:               //
                vtkITKExecuteDataFromFile<itk::CovariantVector<double>, 4>( //
                  this,
                  this->CachedImages,
                  listDim,
                  imageIO->GetAxesReorder(),
                  this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::FLOAT:               //
                vtkITKExecuteDataFromFile<itk::CovariantVector<float>, 4>( //
                  this,
                  this->CachedImages,
                  listDim,
                  imageIO->GetAxesReorder(),
                  this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::LONG:               //
                vtkITKExecuteDataFromFile<itk::CovariantVector<long>, 4>( //
                  this,
                  this->CachedImages,
                  listDim,
                  imageIO->GetAxesReorder(),
                  this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::ULONG:                       //
                vtkITKExecuteDataFromFile<itk::CovariantVector<unsigned long>, 4>( //
                  this,
                  this->CachedImages,
                  listDim,
                  imageIO->GetAxesReorder(),
                  this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::INT:               //
                vtkITKExecuteDataFromFile<itk::CovariantVector<int>, 4>( //
                  this,
                  this->CachedImages,
                  listDim,
                  imageIO->GetAxesReorder(),
                  this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::UINT:                       //
                vtkITKExecuteDataFromFile<itk::CovariantVector<unsigned int>, 4>( //
                  this,
                  this->CachedImages,
                  listDim,
                  imageIO->GetAxesReorder(),
                  this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::SHORT:               //
                vtkITKExecuteDataFromFile<itk::CovariantVector<short>, 4>( //
                  this,
                  this->CachedImages,
                  listDim,
                  imageIO->GetAxesReorder(),
                  this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::USHORT:                       //
                vtkITKExecuteDataFromFile<itk::CovariantVector<unsigned short>, 4>( //
                  this,
                  this->CachedImages,
                  listDim,
                  imageIO->GetAxesReorder(),
                  this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::CHAR:               //
                vtkITKExecuteDataFromFile<itk::CovariantVector<char>, 4>( //
                  this,
                  this->CachedImages,
                  listDim,
                  imageIO->GetAxesReorder(),
                  this->VoxelVectorType);
                break;
              case itk::ImageIOBase::IOComponentEnum::UCHAR:                       //
                vtkITKExecuteDataFromFile<itk::CovariantVector<unsigned char>, 4>( //
                  this,
                  this->CachedImages,
                  listDim,
                  imageIO->GetAxesReorder(),
                  this->VoxelVectorType);
                break;
              default:
                vtkErrorMacro("Unexpected component type for covariant vector voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
                this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
                return;
            }
            break;
          case itk::CommonEnums::IOPixel::UNKNOWNPIXELTYPE:
          case itk::CommonEnums::IOPixel::OFFSET:
          case itk::CommonEnums::IOPixel::POINT:
          case itk::CommonEnums::IOPixel::SYMMETRICSECONDRANKTENSOR:
          case itk::CommonEnums::IOPixel::DIFFUSIONTENSOR3D:
          case itk::CommonEnums::IOPixel::COMPLEX:
          case itk::CommonEnums::IOPixel::FIXEDARRAY:
          case itk::CommonEnums::IOPixel::ARRAY:
          case itk::CommonEnums::IOPixel::MATRIX:
          case itk::CommonEnums::IOPixel::VARIABLELENGTHVECTOR:
          case itk::CommonEnums::IOPixel::VARIABLESIZEMATRIX:
            vtkErrorMacro("Unexpected pixel type: " << imageIO->GetPixelType());
            this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
            return;
          default:
            vtkErrorMacro("Unexpected pixel type code requested: " << imageIO->GetPixelType());
            this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
            return;
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
