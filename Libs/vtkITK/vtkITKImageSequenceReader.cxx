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
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>

// ITK includes
#include "itkExtractImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageToVTKImageFilter.h"

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
template <class T>
void vtkITKExecuteDataFromFile(vtkITKImageSequenceReader* self, vtkImageData* data)
{
  // Read the entire image
  constexpr unsigned int ImageDimension = 4;  //TODO: Constant
  using PixelType = itk::RGBAPixel<unsigned short>;  //TODO: Scalar type is constant
  using ImageType = itk::Image<PixelType, ImageDimension>;

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
  self->SetNumberOfFrames(extractionSize[3]);  //TODO: Constant dimension

  extractionSize[3] = 0;  // Collapse sequence dimension when extracting frame  //TODO: Constant dimension

  ImageType::RegionType extractionRegion;
  ImageType::IndexType extractionIndex = extractionRegion.GetIndex();
  extractionIndex[3] = self->GetCurrentFrameIndex();  //TODO: Constant dimension
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

  data->DeepCopy(vtkExportFilter->GetOutput());  // Note: crashes if ShallowCopy is used
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
    //switch (this->OutputScalarType)
    //{
    //  vtkTemplateMacro(vtkITKExecuteDataFromFileDiffusionTensor3D<VTK_TT>(
    //    this, tensors.GetPointer(), data));
    //  default:
    //    vtkErrorMacro(<< "UpdateFromFile: Unknown data type " << this->OutputScalarType);
    //    this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
    //}

    vtkITKExecuteDataFromFile<unsigned short>(this, data);
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
