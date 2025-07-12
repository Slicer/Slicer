/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

/// vtkITK includes
#include "itkImageToVTKImageFilter.h"
#include "itkVTKImageToImageFilter.h"
#include "vtkITKGrowCut.h"

/// VTK includes
#include "vtkAlgorithm.h"
#include "vtkArrayDispatch.h"
#include "vtkDataArray.h"
#include "vtkDataArrayMeta.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkStreamingDemandDrivenPipeline.h"

/// ITK includes
#include "itkFastGrowCut.h"

vtkStandardNewMacro(vtkITKGrowCut);

//----------------------------------------------------------------------------
class vtkITKGrowCut::vtkInternal
{
public:
  vtkInternal(vtkITKGrowCut* external)
    : External(external)
  {
  }
  virtual ~vtkInternal() = default;

  itk::ProcessObject::Pointer FGCFilterProcess{ nullptr };

  void Reset() { this->FGCFilterProcess = nullptr; }

  vtkITKGrowCut* External{ nullptr };

  template <typename IntensityType, typename SeedLabelType, typename MaskType>
  void RunGrowCut(vtkImageData* intensityVolume,
                  vtkImageData* seedLabelVolume,
                  vtkImageData* maskLabelVolume,
                  vtkImageData* resultLabelVolume);

  //----------------------------------------------------------------------------
  struct FastGrowCutWorker
  {
    template <typename IntensityVolumeArrayType, typename SeedLabelVolumeArrayType>
    void operator()(IntensityVolumeArrayType* vtkNotUsed(intensityVolumeArray),
                    SeedLabelVolumeArrayType* vtkNotUsed(seedLabelVolumeArray),
                    vtkImageData* intensityVolume,
                    vtkImageData* seedLabelVolume,
                    vtkImageData* resultLabelVolume,
                    vtkITKGrowCut* self)
    {
      using IntensityType = vtk::GetAPIType<IntensityVolumeArrayType>;
      using SeedLabelType = vtk::GetAPIType<SeedLabelVolumeArrayType>;
      using MaskType = char; // Mask image not specified. Use placeholder type char.
      self->Internal->RunGrowCut<IntensityType, SeedLabelType, MaskType>(
        intensityVolume, seedLabelVolume, nullptr, resultLabelVolume);
    }

    template <typename IntensityVolumeArrayType, typename SeedLabelVolumeArrayType, typename MaskLabelVolumeArrayType>
    void operator()(IntensityVolumeArrayType* vtkNotUsed(intensityVolumeArray),
                    SeedLabelVolumeArrayType* vtkNotUsed(seedLabelVolumeArray),
                    MaskLabelVolumeArrayType* vtkNotUsed(maskLabelVolumeArray),
                    vtkImageData* intensityVolume,
                    vtkImageData* seedLabelVolume,
                    vtkImageData* maskLabelVolume,
                    vtkImageData* resultLabelVolume,
                    vtkITKGrowCut* self)
    {
      using IntensityType = vtk::GetAPIType<IntensityVolumeArrayType>;
      using SeedLabelType = vtk::GetAPIType<SeedLabelVolumeArrayType>;
      using MaskType = vtk::GetAPIType<MaskLabelVolumeArrayType>;
      self->Internal->RunGrowCut<IntensityType, SeedLabelType, MaskType>(
        intensityVolume, seedLabelVolume, maskLabelVolume, resultLabelVolume);
    }
  };
};

//----------------------------------------------------------------------------
template <typename IntensityType, typename SeedLabelType, typename MaskType>
void vtkITKGrowCut::vtkInternal::RunGrowCut(vtkImageData* intensityVolume,
                                            vtkImageData* seedLabelVolume,
                                            vtkImageData* maskLabelVolume,
                                            vtkImageData* resultLabelVolume)
{
  typedef itk::Image<IntensityType, 3> IntensityImageType;
  typedef itk::Image<SeedLabelType, 3> SeedLabelImageType;
  typedef itk::Image<MaskType, 3> MaskImageType;
  using FGCType = itk::FastGrowCut<IntensityImageType, SeedLabelImageType, MaskImageType>;

  typename FGCType::Pointer fgcFilter = nullptr;
  if (this->FGCFilterProcess)
  {
    // Attempt to use the existing filter.
    fgcFilter = dynamic_cast<FGCType*>(this->FGCFilterProcess.GetPointer());
  }
  if (!fgcFilter)
  {
    fgcFilter = FGCType::New();
    this->FGCFilterProcess = fgcFilter;
  }

  // Wrap intensity VTK image into an ITK image.
  typename itk::VTKImageToImageFilter<IntensityImageType>::Pointer intensityVTKToITKFilter =
    itk::VTKImageToImageFilter<IntensityImageType>::New();
  intensityVTKToITKFilter->SetInput(intensityVolume);
  intensityVTKToITKFilter->Update();
  IntensityImageType* intensityImage = intensityVTKToITKFilter->GetOutput();
  fgcFilter->SetInput(intensityImage);

  // Wrap seed label VTK image into an ITK image.
  typename itk::VTKImageToImageFilter<SeedLabelImageType>::Pointer seedLabelVTKToITKFilter =
    itk::VTKImageToImageFilter<SeedLabelImageType>::New();
  seedLabelVTKToITKFilter->SetInput(seedLabelVolume);
  seedLabelVTKToITKFilter->Update();
  SeedLabelImageType* seedImage = seedLabelVTKToITKFilter->GetOutput();
  fgcFilter->SetSeedImage(seedImage);

  if (maskLabelVolume)
  {
    // Wrap mask label VTK image into an ITK image.
    typename itk::VTKImageToImageFilter<MaskImageType>::Pointer maskVTKToITKFilter =
      itk::VTKImageToImageFilter<MaskImageType>::New();
    maskVTKToITKFilter->SetInput(maskLabelVolume);
    maskVTKToITKFilter->Update();
    MaskImageType* maskImage = maskVTKToITKFilter->GetOutput();
    fgcFilter->SetMaskImage(maskImage);
  }

  fgcFilter->SetDistancePenalty(this->External->GetDistancePenalty());
  fgcFilter->Update();

  // Wrap output ITK image into an VTK image. Data type is the same as the seed label.
  typename itk::ImageToVTKImageFilter<SeedLabelImageType>::Pointer resultImageToVTKFilter =
    itk::ImageToVTKImageFilter<SeedLabelImageType>::New();
  resultImageToVTKFilter->SetInput(fgcFilter->GetOutput());
  resultImageToVTKFilter->Update();
  resultLabelVolume->ShallowCopy(resultImageToVTKFilter->GetOutput());
}

//----------------------------------------------------------------------------
vtkITKGrowCut::vtkITKGrowCut()
  : Internal(new vtkInternal(this))
{
  this->SetNumberOfInputPorts(3);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
vtkITKGrowCut::~vtkITKGrowCut()
{
  delete this->Internal;
  this->Internal = nullptr;
}

//----------------------------------------------------------------------------
void vtkITKGrowCut::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
int vtkITKGrowCut::FillInputPortInformation(int port, vtkInformation* info)
{
  vtkImageAlgorithm::FillInputPortInformation(port, info);
  if (port == 2)
  {
    // Mask port is optional.
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  }
  return 1;
}

//-----------------------------------------------------------------------------
void vtkITKGrowCut::ExecuteDataWithInformation(vtkDataObject* resultLabelVolumeDataObject,
                                               vtkInformation* vtkNotUsed(resultLabelVolumeInfo))
{
  vtkImageData* intensityVolume = vtkImageData::SafeDownCast(this->GetInput(0));
  vtkImageData* seedLabelVolume = vtkImageData::SafeDownCast(this->GetInput(1));
  vtkImageData* maskLabelVolume = vtkImageData::SafeDownCast(this->GetInput(2));
  vtkImageData* resultLabelVolume = vtkImageData::SafeDownCast(resultLabelVolumeDataObject);

  vtkDataArray* intensityScalars = intensityVolume ? intensityVolume->GetPointData()->GetScalars() : nullptr;
  if (!intensityScalars)
  {
    vtkErrorMacro("Invalid intensity image data");
    return;
  }

  int intensityVolumeDimensions[3] = { 0, 0, 0 };
  intensityVolume->GetDimensions(intensityVolumeDimensions);
  if (intensityVolumeDimensions[0] < 3 || intensityVolumeDimensions[1] < 3 || intensityVolumeDimensions[2] < 3)
  {
    // image is too small (there should be space for at least one voxel padding around the image)
    vtkErrorMacro("vtkITKGrowCut: image size is too small. Minimum size along each dimension is 3.");
    return;
  }

  vtkDataArray* seedScalars = seedLabelVolume ? seedLabelVolume->GetPointData()->GetScalars() : nullptr;
  if (!intensityVolume)
  {
    vtkErrorMacro("Invalid seed image data");
    return;
  }

  vtkInternal::FastGrowCutWorker worker;
  if (maskLabelVolume)
  {
    vtkDataArray* maskScalars = maskLabelVolume ? maskLabelVolume->GetPointData()->GetScalars() : nullptr;
    if (!maskScalars)
    {
      vtkErrorMacro("Invalid mask image data");
      return;
    }

    vtkArrayDispatch::Dispatch3::Execute(intensityScalars,
                                         seedScalars,
                                         maskScalars,
                                         worker,
                                         intensityVolume,
                                         seedLabelVolume,
                                         maskLabelVolume,
                                         resultLabelVolume,
                                         this);
  }
  else
  {
    vtkArrayDispatch::Dispatch2::Execute(
      intensityScalars, seedScalars, worker, intensityVolume, seedLabelVolume, resultLabelVolume, this);
  }
}

//-----------------------------------------------------------------------------
void vtkITKGrowCut::Reset()
{
  this->Internal->Reset();
}

//-----------------------------------------------------------------------------
void vtkITKGrowCut::SetDistancePenalty(double distancePenalty)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting DistancePenalty to " << distancePenalty);
  if (this->DistancePenalty != distancePenalty)
  {
    this->DistancePenalty = distancePenalty;
    // TODO: Due to a bug in ITK (https://github.com/InsightSoftwareConsortium/ITKGrowCut/issues/18),
    // we currently need to reset the filter when changing the distance penalty.
    // Otherwise the internal computed distance values will not be recomputed, and the change to distance penalty will
    // not be applied.
    this->Reset();
    this->Modified();
  }
}
