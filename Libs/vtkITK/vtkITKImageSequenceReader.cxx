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
  by the grant GRT-00000485 of Children's Hospital of Philadeplhia, USA.

==========================================================================*/

// VTKITK includes
#include "vtkITKImageSequenceReader.h"

// VTK includes
//#include <vtkAOSDataArrayTemplate.h>
//#include <vtkCommand.h>
//#include <vtkDataArray.h>
//#include <vtkErrorCode.h>
//#include <vtkImageData.h>
#include <vtkObjectFactory.h>
//#include <vtkPointData.h>

// ITK includes
#include "itkImageFileReader.h"
#include "itkOrientImageFilter.h"
#include "itkNrrdImageIO.h" //TODO:
#include "itkRGBAPixel.h" //TODO:
#include "itkExtractImageFilter.h"

vtkStandardNewMacro(vtkITKImageSequenceReader);


//----------------------------------------------------------------------------
vtkITKImageSequenceReader::vtkITKImageSequenceReader()
{
  this->FileName = nullptr;
}

//----------------------------------------------------------------------------
vtkITKImageSequenceReader::~vtkITKImageSequenceReader() = default;

//----------------------------------------------------------------------------
void vtkITKImageSequenceReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "FileName: " << (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkITKImageSequenceReader::SetFileName(const char *name)
{
  //TODO: Simply copied from vtkITKImageWriter, but probably can be changed to VTK macro. After the class is done...
  if (this->FileName && name && (!strcmp(this->FileName,name)))
  {
    return;
  }
  if (!name && !this->FileName)
  {
    return;
  }
  if (this->FileName)
  {
    delete [] this->FileName;
  }

  this->FileName = new char[strlen(name) + 1];
  strcpy(this->FileName, name);
  this->Modified();
}

////----------------------------------------------------------------------------
//template <class T>
//void vtkITKExecuteDataFromFileVector(vtkITKImageSequenceReader* self, vtkImageData *data)
//{
//  typedef itk::VectorImage<T,3> image2;
//  typedef itk::ImageSource<image2> FilterType;
//  typename FilterType::Pointer filter;
//  typedef itk::ImageFileReader< image2 > ReaderType;
//  typename ReaderType::Pointer reader2 = ReaderType::New();
//  reader2->SetFileName(self->GetFileName(0));
//  if (self->GetUseNativeCoordinateOrientation())
//  {
//    filter = reader2;
//  }
//  else
//  {
//    typename itk::OrientImageFilter<image2,image2>::Pointer orient2 =
//      itk::OrientImageFilter<image2,image2>::New();
//    orient2->SetDebug(self->GetDebug());
//    orient2->SetInput(reader2->GetOutput());
//    orient2->UseImageDirectionOn();
//    orient2->SetDesiredCoordinateOrientation(
//      self->GetDesiredCoordinateOrientation());
//    filter = orient2;
//  }
//  filter->UpdateLargestPossibleRegion();
//  typename itk::ImportImageContainer<itk::SizeValueType, T>::Pointer PixelContainer2;
//  PixelContainer2 = filter->GetOutput()->GetPixelContainer();
//  void *ptr = static_cast<void *> (PixelContainer2->GetBufferPointer());
//  DownCast<T>(data->GetPointData()->GetScalars())
//    ->SetVoidArray(ptr, PixelContainer2->Size(), 0,
//                   vtkAOSDataArrayTemplate<T>::VTK_DATA_ARRAY_DELETE);
//  PixelContainer2->ContainerManageMemoryOff();
//}
//
////----------------------------------------------------------------------------
//// This function reads a data from a file.  The data extent/axes
//// are assumed to be the same as the file extent/order.
//void vtkITKImageSequenceReader::ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo)
//{
//  if (!this->Superclass::Archetype)
//  {
//    vtkErrorMacro("An Archetype must be specified.");
//    this->SetErrorCode(vtkErrorCode::NoFileNameError);
//    return;
//  }
//  vtkImageData *data = this->AllocateOutputData(output, outInfo);
//
//  // If there is only one file in the series, just use an image file reader
//  if (this->FileNames.size() == 1)
//  {
//    vtkDebugMacro("ImageSeriesVectorReaderFile: only one file: " << this->FileNames[0].c_str());
//    vtkDebugMacro("DiffusionTensorImageReaderFile: only one file: " << this->FileNames[0].c_str());
//    switch (this->OutputScalarType)
//    {
//      vtkTemplateMacroCase(VTK_DOUBLE, double, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
//      vtkTemplateMacroCase(VTK_FLOAT, float, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
//      vtkTemplateMacroCase(VTK_LONG, long, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
//      vtkTemplateMacroCase(VTK_UNSIGNED_LONG, unsigned long, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
//      vtkTemplateMacroCase(VTK_INT, int, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
//      vtkTemplateMacroCase(VTK_UNSIGNED_INT, unsigned int, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
//      vtkTemplateMacroCase(VTK_SHORT, short, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
//      vtkTemplateMacroCase(VTK_UNSIGNED_SHORT, unsigned short, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
//      vtkTemplateMacroCase(VTK_CHAR, char, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
//      vtkTemplateMacroCase(VTK_SIGNED_CHAR, signed char, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
//      vtkTemplateMacroCase(VTK_UNSIGNED_CHAR, unsigned char, vtkITKExecuteDataFromFileVector<VTK_TT>(this, data));
//    default:
//      vtkErrorMacro(<< "UpdateFromFile: Unknown data type " << this->OutputScalarType);
//      this->SetErrorCode(vtkErrorCode::UnrecognizedFileTypeError);
//    }
//
//    this->SetMetaDataScalarRangeToPointDataInfo(data);
//  }
//  else
//  {
//    // ERROR - should have used the series reader
//    vtkErrorMacro("There is more than one file, use the VectorReaderSeries instead");
//    this->SetErrorCode(vtkErrorCode::FileFormatError);
//  }
//}

////----------------------------------------------------------------------------
//void vtkITKImageSequenceReader::ReadProgressCallback(itk::Object* obj, const itk::EventObject&, void* data)
//{
//  itk::ProcessObject::Pointer p(dynamic_cast<itk::ProcessObject *>(obj));
//  if (p.IsNull())
//  {
//    return;
//  }
//  vtkITKImageSequenceReader* me=reinterpret_cast<vtkITKImageSequenceReader*>(data);
//  me->Progress = p->GetProgress();
//  me->InvokeEvent(vtkCommand::ProgressEvent,&me->Progress);
//}

//----------------------------------------------------------------------------
void vtkITKImageSequenceReader::Read()
{
  constexpr unsigned int ImageDimension = 4;  //TODO: Constant
  using PixelType = itk::RGBAPixel<unsigned short>;
  using ImageType = itk::Image< PixelType, ImageDimension >;

  using ReaderType = itk::ImageFileReader< ImageType >;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(this->FileName);
  reader->Update();
  ImageType::ConstPointer image = reader->GetOutput();

  // Split image by sequence frames
  using FrameImageType = itk::Image< PixelType, ImageDimension-1 >;
  using ExtractImageFilterType = itk::ExtractImageFilter<ImageType, FrameImageType>;
  ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();

  extractImageFilter->SetInput(image);

  ImageType::RegionType fullRegion = image->GetLargestPossibleRegion();
  ImageType::SizeType extractionSize = fullRegion.GetSize();
  unsigned int numberOfFrames = extractionSize[3];  //TODO: Constant
  extractionSize[3] = 0;  // Collapse sequence dimension when extracting frame  //TODO: Constant

  for (int frameIdx=0; frameIdx<numberOfFrames; ++frameIdx)
  {
    ImageType::RegionType extractionRegion;
    ImageType::IndexType extractionIndex = extractionRegion.GetIndex();
    extractionIndex[3] = frameIdx;  //TODO: Constant
    extractionRegion.SetIndex(extractionIndex);
    extractionRegion.SetSize(extractionSize);
    extractImageFilter->SetDirectionCollapseToSubmatrix();
    extractImageFilter->SetExtractionRegion(extractionRegion);
    extractImageFilter->Update();

    FrameImageType::Pointer frameImage = extractImageFilter->GetOutput();
    FrameImageType::RegionType frameRegion = frameImage->GetLargestPossibleRegion();
    //std::cerr << "ZZZ: Frame " << frameIdx << ": Largest possible region: " << frameRegion << std::endl;
  }


}
