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

vtkStandardNewMacro(vtkITKImageSequenceReader);


//----------------------------------------------------------------------------
vtkITKImageSequenceReader::vtkITKImageSequenceReader()
{
}

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
  this->Superclass::PrintSelf(os,indent);

  os << indent << "FileName: " << (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
template <class TPixelType, int Dimension>
void vtkITKExecuteDataFromFile_FramesInDimension(vtkITKImageSequenceReader* self, vtkImageData* data)
{
  using PixelType = TPixelType;
  constexpr unsigned int ImageDimension = Dimension;
  using ImageType = itk::Image<PixelType, ImageDimension>;

  // Debug information about the pixel type
  using ValueType = typename PixelType::ValueType;
  constexpr bool isVector = std::is_same<PixelType, itk::Vector<ValueType>>::value;
  constexpr bool isCovariantVector = std::is_same<PixelType, itk::CovariantVector<ValueType>>::value;
  constexpr bool isRGB = std::is_same<PixelType, itk::RGBPixel<ValueType>>::value;
  constexpr bool isRGBA = std::is_same<PixelType, itk::RGBAPixel<ValueType>>::value;

  std::cout << "PixelType: " << typeid(PixelType).name() << std::endl;
  std::cout << "ValueType: " << typeid(ValueType).name() << std::endl;
  std::cout << "Is Vector: " << isVector << std::endl;
  std::cout << "Is CovariantVector: " << isCovariantVector << std::endl;
  std::cout << "Is RGB: " << isRGB << std::endl;
  std::cout << "Is RGBA: " << isRGBA << std::endl;
  if (isVector || isCovariantVector)
  {
    std::cout << "Vector Length: " << PixelType::Dimension << std::endl;
  }

  using ReaderType = itk::ImageFileReader<ImageType>;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(self->GetFileName());
  reader->Update();
  ImageType::ConstPointer image = reader->GetOutput();

  // Get origin and spacing from ITK image
  ImageType::PointType itkOrigin = image->GetOrigin();
  ImageType::SpacingType itkSpacing = image->GetSpacing();
  double origin[3] = {itkOrigin[0], itkOrigin[1], itkOrigin[2]};
  double spacing[3] = {itkSpacing[0], itkSpacing[1], itkSpacing[2]};
  // Get directions from ITK image
  ImageType::DirectionType itkDirections = image->GetDirection();
  double directions[3][3] = { {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0} };
  for (unsigned int col=0; col<3; col++)
  {
    for (unsigned int row=0; row<3; row++)
    {
      directions[row][col] = itkDirections[row][col];
    }
  }
  // Make the pose matrix available in VTK
  if (self->GetRasToIjkMatrix())
  {
    self->GetRasToIjkMatrix()->Delete();
  }
  vtkMatrix4x4* rasToIjkMatrix = vtkMatrix4x4::New();
  rasToIjkMatrix->Identity();
  for (int row=0; row<3; row++)
  {
    for (int col=0; col<3; col++)
    {
      rasToIjkMatrix->SetElement(row, col, spacing[col] * directions[row][col]);
    }
    rasToIjkMatrix->SetElement(row, 3, origin[row]);
  }
  self->SetRasToIjkMatrix(rasToIjkMatrix);

  // Extract requested frame from image
  using FrameImageType = itk::Image<PixelType, ImageDimension-1>;
  using ExtractImageFilterType = itk::ExtractImageFilter<ImageType, FrameImageType>;
  ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();

  extractImageFilter->SetInput(image);

  ImageType::RegionType fullRegion = image->GetLargestPossibleRegion();
  ImageType::SizeType extractionSize = fullRegion.GetSize();
  self->SetNumberOfFrames(extractionSize[Dimension-1]);

  //TODO: Get list dimension from metadata
  extractionSize[Dimension-1] = 0;  // Collapse sequence dimension when extracting frame

  ImageType::RegionType extractionRegion;
  ImageType::IndexType extractionIndex = extractionRegion.GetIndex();
  extractionIndex[Dimension-1] = self->GetCurrentFrameIndex();
  extractionRegion.SetIndex(extractionIndex);
  extractionRegion.SetSize(extractionSize);
  extractImageFilter->SetDirectionCollapseToSubmatrix();
  extractImageFilter->SetExtractionRegion(extractionRegion);
  extractImageFilter->Update();

  FrameImageType::Pointer frameImage = extractImageFilter->GetOutput();
  FrameImageType::RegionType frameRegion = frameImage->GetLargestPossibleRegion();

  // Convert extracted frame to VTK image
  using VTKExporterFilterType = itk::ImageToVTKImageFilter<FrameImageType>;
  VTKExporterFilterType::Pointer vtkExportFilter = VTKExporterFilterType::New();
  vtkExportFilter->SetInput(frameImage);
  vtkExportFilter->Update();

  // Copy to output. Note: crashes if ShallowCopy is used
  data->DeepCopy(vtkExportFilter->GetOutput());
}

//----------------------------------------------------------------------------
// NOTE: This function needs to be separate as ImageToVTKImageFilter cannot be
//       used with dimensions higher than 3
template <class TPixelType>
void vtkITKExecuteDataFromFile_FramesInComponent(vtkITKImageSequenceReader* self, vtkImageData* data)
{
  using PixelType = TPixelType;
  constexpr unsigned int ImageDimension = 3;
  using ImageType = itk::VectorImage<PixelType, ImageDimension>;

  using ReaderType = itk::ImageFileReader<ImageType>;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(self->GetFileName());
  reader->Update();
  ImageType::ConstPointer image = reader->GetOutput();

  // Get origin and spacing from ITK image
  ImageType::PointType itkOrigin = image->GetOrigin();
  ImageType::SpacingType itkSpacing = image->GetSpacing();
  double origin[3] = {itkOrigin[0], itkOrigin[1], itkOrigin[2]};
  double spacing[3] = {itkSpacing[0], itkSpacing[1], itkSpacing[2]};
  // Get directions from ITK image
  ImageType::DirectionType itkDirections = image->GetDirection();
  double directions[3][3] = { {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0} };
  for (unsigned int col=0; col<3; col++)
  {
    for (unsigned int row=0; row<3; row++)
    {
      directions[row][col] = itkDirections[row][col];
    }
  }
  // Make the pose matrix available in VTK
  if (self->GetRasToIjkMatrix())
  {
    self->GetRasToIjkMatrix()->Delete();
  }
  vtkMatrix4x4* rasToIjkMatrix = vtkMatrix4x4::New();
  rasToIjkMatrix->Identity();
  for (int row=0; row<3; row++)
  {
    for (int col=0; col<3; col++)
    {
      rasToIjkMatrix->SetElement(row, col, spacing[col] * directions[row][col]);
    }
    rasToIjkMatrix->SetElement(row, 3, origin[row]);
  }
  self->SetRasToIjkMatrix(rasToIjkMatrix);

  // Set number of frames from loaded vector image
  self->SetNumberOfFrames(image->GetNumberOfComponentsPerPixel());

  // Set up component extractor to get the frame
  using FrameImageType = itk::Image<PixelType, ImageDimension>;
  using ComponentExtractorType = itk::VectorIndexSelectionCastImageFilter<ImageType, FrameImageType>;
  typename ComponentExtractorType::Pointer componentExtractor = ComponentExtractorType::New();
  componentExtractor->SetInput(image);
  componentExtractor->SetIndex(self->GetCurrentFrameIndex());
  componentExtractor->Update();
  // Get the extracted component image
  FrameImageType::Pointer frameImage = componentExtractor->GetOutput();
  FrameImageType::RegionType region = frameImage->GetLargestPossibleRegion();
  const PixelType* inputBuffer = frameImage->GetBufferPointer();

  // Create VTK image with same properties
  vtkNew<vtkImageData> outputImage;
  outputImage->SetDimensions(region.GetSize(0), region.GetSize(1), region.GetSize(2));

  // Set up the VTK image type
  int vtkType;
  if (std::is_same<PixelType, unsigned short>::value)
  {
    vtkType = VTK_UNSIGNED_SHORT;
  }
  else if (std::is_same<PixelType, int>::value)
  {
    vtkType = VTK_INT;
  }
  else if (std::is_same<PixelType, float>::value)
  {
    vtkType = VTK_FLOAT;
  }
  else
  {
    vtkType = VTK_FLOAT;  // Default to float for unknown types
  }

  outputImage->AllocateScalars(vtkType, 1);

  // Copy the buffer directly, handling the vector pixel type
  const size_t numPixels = region.GetNumberOfPixels();
  void* outputBuffer = outputImage->GetScalarPointer();

  // Copy each pixel value, extracting the scalar component.
  // This is needed because ImageToVTKImageFilter cannot handle variable length vectors.
  for (size_t i = 0; i < numPixels; ++i)
  {
    switch (vtkType)
    {
      case VTK_UNSIGNED_SHORT:
        static_cast<unsigned short*>(outputBuffer)[i] = inputBuffer[i][0];
        break;
      case VTK_INT:
        static_cast<int*>(outputBuffer)[i] = inputBuffer[i][0];
        break;
      case VTK_FLOAT:
        static_cast<float*>(outputBuffer)[i] = inputBuffer[i][0];
        break;
    }
  }

  // Set spacing and origin
  FrameImageType::SpacingType itkFrameSpacing = frameImage->GetSpacing();
  FrameImageType::PointType itkFrameOrigin = frameImage->GetOrigin();
  double frameSpacing[3] = {itkFrameSpacing[0], itkFrameSpacing[1], itkFrameSpacing[2]};
  double frameOrigin[3] = {itkFrameOrigin[0], itkFrameOrigin[1], itkFrameOrigin[2]};
  outputImage->SetSpacing(frameSpacing);
  outputImage->SetOrigin(frameOrigin);

  // Copy to output. Note: crashes if ShallowCopy is used
  data->DeepCopy(outputImage);
}

//----------------------------------------------------------------------------
// This function reads a data from a file.  The data extent/axes
// are assumed to be the same as the file extent/order.
void vtkITKImageSequenceReader::ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo)
{
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

  try
  {
    // Read header to decide dimensions, pixel type, and scalar type

    // Create an NRRD image IO object
    using ImageIOType = itk::NrrdImageIO;
    ImageIOType::Pointer imageIO = ImageIOType::New();
    // Read metadata
    if (!imageIO->CanReadFile(this->GetFileName()))
    {
      vtkErrorMacro("Cannot read the NRRD file: " << this->GetFileName());
      this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
      return;
    }
    imageIO->SetFileName(this->GetFileName());
    imageIO->ReadImageInformation(); // Read only the header information

    //// Print relevant metadata (kept for debugging when adding support for new data types)
    //unsigned int dimension = imageIO->GetNumberOfDimensions();
    //std::cerr << "Dimensions: " << dimension << std::endl;
    //for (unsigned int i = 0; i < dimension; ++i)
    //{
    //  std::cerr << "Size of dimension " << i << ": " << imageIO->GetDimensions(i) << std::endl;
    //}
    //std::cerr << "Pixel Type: " << imageIO->GetPixelTypeAsString(imageIO->GetPixelType()) << std::endl;
    //std::cerr << "Component Type: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()) << std::endl;

    // Load image from file
    switch (imageIO->GetNumberOfDimensions())
    {
    case 3:
      switch (imageIO->GetPixelType())
      {
        case itk::CommonEnums::IOPixel::RGB:
          if (imageIO->GetComponentType() != itk::ImageIOBase::IOComponentEnum::UCHAR)
          {
            vtkErrorMacro("Unexpected component type for RGB voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
            this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
            return;
          }
          vtkITKExecuteDataFromFile_FramesInDimension<itk::RGBPixel<unsigned char>, 3>(this, data);
          break;
        case itk::CommonEnums::IOPixel::RGBA:
          if (imageIO->GetComponentType() != itk::ImageIOBase::IOComponentEnum::UCHAR)
          {
            vtkErrorMacro("Unexpected component type for RGBA voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
            this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
            return;
          }
          vtkITKExecuteDataFromFile_FramesInDimension<itk::RGBAPixel<unsigned char>, 3>(this, data);
          break;
        case itk::CommonEnums::IOPixel::VECTOR:
          // For 3D images with vector type, we suppose that the sequence information is stored as scalar components
          // if number of components is larger than 4
          if (imageIO->GetNumberOfComponents() > 4)
          {
            switch (imageIO->GetComponentType())
            {
            case itk::ImageIOBase::IOComponentEnum::USHORT:
              vtkITKExecuteDataFromFile_FramesInComponent<itk::Vector<unsigned short>>(this, data);
              break;
            case itk::ImageIOBase::IOComponentEnum::INT:
              vtkITKExecuteDataFromFile_FramesInComponent<itk::Vector<int>>(this, data);
              break;
            case itk::ImageIOBase::IOComponentEnum::FLOAT:
              vtkITKExecuteDataFromFile_FramesInComponent<itk::Vector<float>>(this, data);
              break;
            default:
              vtkErrorMacro("Unexpected component type for 5+ component vector voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
              this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
              return;
            }
          }
          else
          {
            switch (imageIO->GetComponentType())
            {
            case itk::ImageIOBase::IOComponentEnum::USHORT:
              vtkITKExecuteDataFromFile_FramesInDimension<itk::Vector<unsigned short>, 3>(this, data);
              break;
            case itk::ImageIOBase::IOComponentEnum::INT:
              vtkITKExecuteDataFromFile_FramesInDimension<itk::Vector<int>, 3>(this, data);
              break;
            case itk::ImageIOBase::IOComponentEnum::FLOAT:
              vtkITKExecuteDataFromFile_FramesInDimension<itk::Vector<float>, 3>(this, data);
              break;
            default:
              vtkErrorMacro("Unexpected component type for 4 or less component vector voxel: "
                << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
              this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
              return;
            }
          }
          break;
      }
      break;
    case 4:
      switch (imageIO->GetPixelType())
      {
        case itk::CommonEnums::IOPixel::RGB:
          if (imageIO->GetComponentType() != itk::ImageIOBase::IOComponentEnum::UCHAR)
          {
            vtkErrorMacro("Unexpected component type for RGB voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
            this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
            return;
          }
          vtkITKExecuteDataFromFile_FramesInDimension<itk::RGBPixel<unsigned char>, 4>(this, data);
          break;
        case itk::CommonEnums::IOPixel::RGBA:
          if (imageIO->GetComponentType() != itk::ImageIOBase::IOComponentEnum::UCHAR)
          {
            vtkErrorMacro("Unexpected component type for RGBA voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
            this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
            return;
          }
          vtkITKExecuteDataFromFile_FramesInDimension<itk::RGBAPixel<unsigned char>, 4>(this, data);
          break;
        case itk::CommonEnums::IOPixel::VECTOR:
          switch (imageIO->GetComponentType())
          {
          case itk::ImageIOBase::IOComponentEnum::USHORT:
            vtkITKExecuteDataFromFile_FramesInDimension<itk::Vector<unsigned short>, 4>(this, data);
            break;
          case itk::ImageIOBase::IOComponentEnum::FLOAT:
            vtkITKExecuteDataFromFile_FramesInDimension<itk::Vector<float>, 4>(this, data);
            break;
          case itk::ImageIOBase::IOComponentEnum::DOUBLE:
            vtkITKExecuteDataFromFile_FramesInDimension<itk::Vector<double>, 4>(this, data);
            break;
          default:
            vtkErrorMacro("Unexpected component type for vector voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
            this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
            return;
          }
          break;
        case itk::CommonEnums::IOPixel::COVARIANTVECTOR:
          switch (imageIO->GetComponentType())
          {
          case itk::ImageIOBase::IOComponentEnum::USHORT:
            vtkITKExecuteDataFromFile_FramesInDimension<itk::CovariantVector<unsigned short>, 4>(this, data);
            break;
          case itk::ImageIOBase::IOComponentEnum::FLOAT:
            vtkITKExecuteDataFromFile_FramesInDimension<itk::CovariantVector<float>, 4>(this, data);
            break;
          case itk::ImageIOBase::IOComponentEnum::DOUBLE:
            vtkITKExecuteDataFromFile_FramesInDimension<itk::CovariantVector<double>, 4>(this, data);
            break;
          default:
            vtkErrorMacro("Unexpected component type for covariant vector voxel: " << imageIO->GetComponentTypeAsString(imageIO->GetComponentType()));
            this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
            return;
          }
          break;
      }
      break;
    }
  }
  catch (itk::InvalidArgumentError & e)
  {
    vtkDebugMacro(<< "Could not read file as sequence" << e);
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    // return successful read, because this is an expected error when the user
    // has selected a file that doesn't happen to be a sequence.  So it's a file
    // format error, but not something that should trigger a VTK pipeline error
    // (at least not as used in vtkMRMLStorageNodes).
    return;
  }
  catch (itk::ExceptionObject & e)
  {
    vtkErrorMacro(<< "Exception from vtkITK when reading image sequence: " << e << "\n");
    this->SetErrorCode(vtkErrorCode::FileFormatError);
    return;
  }
  return;
}
