/**********************************************************************
 * vtkItkGrowCutSegmentationImageFilter
 * Implements wrapper for the itkGrowCutSegmentationImageFilter
 * This implemnents n-class segmentation
 **********************************************************************/

#include "vtkITKGrowCutSegmentationImageFilter.h"

// VTK includes
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVersion.h>

// ITK includes
#include <itkGrowCutSegmentationImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkRegionOfInterestImageFilter.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkITKGrowCutSegmentationImageFilter);

//-----------------------------------------------------------------------------
////////// These types are not defined in itk ////////////
#ifdef vtkTemplateMacroCase_ui64
#undef vtkTemplateMacroCase_ui64
# define vtkTemplateMacroCase_ui64(typeN, type, call)
#endif
#ifdef vtkTemplateMacroCase_si64
#undef vtkTemplateMacroCase_si64
# define vtkTemplateMacroCase_si64(typeN, type, call)
#endif
#ifdef vtkTemplateMacroCase_ll
#undef vtkTemplateMacroCase_ll
# define vtkTemplateMacroCase_ll(typeN, type, call)
#endif

//-----------------------------------------------------------------------------
// Local Function: not method.
void vtkITKImageGrowCutHandleProgressEvent(itk::Object *caller,
                                           const itk::EventObject& vtkNotUsed(eventObject),
                                           void *clientdata)
{

  itk::ProcessObject *itkFilter = static_cast<itk::ProcessObject*>(caller);
  vtkAlgorithm *vtkFilter = static_cast<vtkAlgorithm*>(clientdata);
  if (itkFilter && vtkFilter )
    {
    vtkFilter->UpdateProgress( itkFilter->GetProgress() );
    }
};


//-----------------------------------------------------------------------------
//// 3D filter
template<class IT1, class OT>
void vtkITKImageGrowCutExecute3D(vtkImageData *inData,
  IT1 *inPtr1, OT *inPtr2, OT *inPtr3,
  OT *output, double &ObjectSize,
  double &contrastNoiseRatio,
  double &priorSegmentStrength,
  itk::CStyleCommand::Pointer progressCommand)
{
  typedef itk::Image<IT1, 3> InImageType;
  typename InImageType::Pointer image = InImageType::New();

  typedef itk::Image<OT, 3> OutImageType;

  typename OutImageType::Pointer labelImage = OutImageType::New();

  typename OutImageType::Pointer prevSegmentedImage = OutImageType::New();

  typename OutImageType::Pointer outputImageROI = OutImageType::New();

  typename OutImageType::Pointer outputImage = OutImageType::New();

  typedef itk::Image<float, 3> WeightImageType;
  typename WeightImageType::Pointer weightImage = WeightImageType::New();

  int dims[3];
  int extent[6];
  double spacing[3], origin[3];

  inData->GetDimensions(dims);
  inData->GetExtent(extent);
  inData->GetOrigin(origin);
  inData->GetSpacing(spacing);

  image->SetOrigin( origin );
  image->SetSpacing( spacing );

  typename InImageType::RegionType region;
  typename InImageType::IndexType index;
  typename InImageType::SizeType size;
  index[0] = extent[0];
  index[1] = extent[2];
  index[2] = extent[4];
  region.SetIndex( index );
  size[0] = extent[1] - extent[0] + 1;
  size[1] = extent[3] - extent[2] + 1;
  size[2] = extent[5] - extent[4] + 1;
  region.SetSize( size );
  image->SetRegions(region);

  image->GetPixelContainer()->SetImportPointer(inPtr1, dims[0]*dims[1]*dims[2], false);

  labelImage->SetOrigin( origin );
  labelImage->SetSpacing( spacing );
  labelImage->SetRegions( region );
  labelImage->GetPixelContainer()->SetImportPointer(inPtr2, dims[0]*dims[1]*dims[2], false);

  prevSegmentedImage->SetOrigin( origin );
  prevSegmentedImage->SetSpacing( spacing );
  prevSegmentedImage->SetRegions( region );
  prevSegmentedImage->GetPixelContainer()->SetImportPointer(inPtr3, dims[0]*dims[1]*dims[2], false);

  weightImage->CopyInformation(image);
  weightImage->SetBufferedRegion( image->GetBufferedRegion() );
  weightImage->Allocate();
  weightImage->FillBuffer( 0 );

  itk::ImageRegionIterator< WeightImageType > weight(weightImage, weightImage->GetBufferedRegion() );
  itk::ImageRegionIteratorWithIndex< OutImageType > label(labelImage, labelImage->GetBufferedRegion() );

  itk::ImageRegionConstIterator< OutImageType > plabel(prevSegmentedImage,
    prevSegmentedImage->GetBufferedRegion() );

  if(contrastNoiseRatio > 1.0)
    {
    contrastNoiseRatio /= 100.0;
    }

  if(priorSegmentStrength > 1.0)
    {
    priorSegmentStrength /= 100.0;
    }


  typename OutImageType::IndexType roiStart;
  typename OutImageType::IndexType roiEnd;


  roiStart[0] = 0; roiStart[1] = 0; roiStart[2] = 0;
  roiEnd[0] = 0; roiEnd[1] = 0; roiEnd[2] = 0;

  unsigned int ndims = image->GetImageDimension();

  bool foundLabel = false;

  for(weight.GoToBegin(), label.GoToBegin(); !weight.IsAtEnd();
      ++weight, ++label)
  {
    typename OutImageType::PixelType color = label.Get();
    if(color == 0)
      {
       weight.Set(0.0);
      }
    else
      {
      weight.Set( contrastNoiseRatio );

      typename OutImageType::IndexType idx = label.GetIndex();
      for (unsigned i = 0; i < ndims; i++)
        {
        if(!foundLabel)
          {
          roiStart[i] = idx[i];
          roiEnd[i] = idx[i];
          }
        else
          {
          if(idx[i] <= roiStart[i])
            {
            roiStart[i] = idx[i];
            }
          if(idx[i] >= roiEnd[i])
            {
            roiEnd[i] = idx[i];
            }
          }
        }
    foundLabel = true;
    }
  }


  for(weight.GoToBegin(), plabel.GoToBegin(), label.GoToBegin(); !weight.IsAtEnd();
      ++weight, ++plabel, ++label)
    {
    typename OutImageType::PixelType color = plabel.Get();
    if(color != 0 && weight.Get() == 0.0)
      {
      weight.Set( priorSegmentStrength );
      label.Set ( color );
      }
    }

  std::cout << " ObjectSize (radius) " << ObjectSize << std::endl;

  typename OutImageType::PixelType radius = static_cast< typename OutImageType::PixelType> (ObjectSize);

  for (unsigned i = 0; i < ndims; i++)
    {
    int diff = static_cast< int > (roiStart[i] - radius);
    if (diff >= index[i])
      {
      roiStart[i] -= radius;
      }
    else
      {
      roiStart[i] = index[i];
      }
    roiEnd[i] = (static_cast<unsigned int>(roiEnd[i] + radius) < size[i]) ?
(roiEnd[i] + radius) : size[i]-1;

    std::cout << " roi[ " << roiStart[i]<<" "<<roiEnd[i] << "] " << std::endl;
    }


  typedef itk::GrowCutSegmentationImageFilter<InImageType, OutImageType> FilterType;
  typename FilterType::Pointer filter = FilterType::New();

  filter->AddObserver(itk::ProgressEvent(), progressCommand );

  typename InImageType::IndexType istart;
  typename InImageType::SizeType isize;

  typename OutImageType::IndexType ostart;
  typename OutImageType::SizeType osize;

  typename WeightImageType::IndexType wstart;
  typename WeightImageType::SizeType wsize;

  for (unsigned n = 0; n < ndims; n++)
    {
    istart[n] = roiStart[n];
    isize[n] = roiEnd[n]-roiStart[n];

    std::cout << " istart " << istart[n] << " isize " << isize[n] << " " << std::endl;

    ostart[n] = istart[n];
    osize[n] = isize[n];

    wstart[n] = istart[n];
    wsize[n] = isize[n];

    }

  typename InImageType::RegionType iRegion;
  iRegion.SetSize( isize );
  iRegion.SetIndex( istart );

  typedef itk::RegionOfInterestImageFilter< InImageType, InImageType > iFilterType;
  typename iFilterType::Pointer fInput = iFilterType::New();
  fInput->SetRegionOfInterest( iRegion );

  fInput->SetInput( image );
  fInput->Update();

  typename OutImageType::RegionType oRegion;
  oRegion.SetSize(osize);
  oRegion.SetIndex(ostart);

  typedef itk::RegionOfInterestImageFilter< OutImageType, OutImageType > oFilterType;
  typename oFilterType::Pointer fOutput = oFilterType::New();
  fOutput->SetRegionOfInterest( oRegion );

  fOutput->SetInput( labelImage );
  fOutput->Update();

  typename WeightImageType::RegionType wRegion;
  wRegion.SetSize(wsize);
  wRegion.SetIndex(wstart);

  typedef itk::RegionOfInterestImageFilter< WeightImageType, WeightImageType > wFilterType;
  typename wFilterType::Pointer fWeight = wFilterType::New();
  fWeight->SetRegionOfInterest( wRegion );

  fWeight->SetInput( weightImage );
  fWeight->Update();

  typename InImageType::Pointer inImage = InImageType::New();
  inImage = fInput->GetOutput();

  typename OutImageType::Pointer labImage = OutImageType::New();
  labImage = fOutput->GetOutput();

  typename WeightImageType::Pointer wtImage = WeightImageType::New();
  wtImage = fWeight->GetOutput();

  filter->SetInput( inImage );
  filter->SetLabelImage( labImage );

  filter->SetStrengthImage( wtImage );

  filter->SetSeedStrength( contrastNoiseRatio );
  filter->SetObjectRadius((unsigned int)ObjectSize);

  filter->Update();
  outputImageROI = filter->GetOutput();

  std::cout << "Done running filter " << std::endl;

  // allocate outputImage first
  outputImage->CopyInformation( labelImage );
  outputImage->SetBufferedRegion( labelImage->GetBufferedRegion() );
  outputImage->Allocate();
  outputImage->FillBuffer(0);

  itk::ImageRegionIterator< OutImageType > filterOut(outputImageROI, outputImageROI->GetBufferedRegion());
  itk::ImageRegionIterator< OutImageType > out(outputImage, oRegion);

  for (filterOut.GoToBegin(), out.GoToBegin(); !filterOut.IsAtEnd(); ++filterOut, ++out)
    {
    out.Set(filterOut.Get());
    }

 memcpy(output, outputImage->GetBufferPointer(),
         outputImage->GetBufferedRegion().GetNumberOfPixels()*sizeof(OT) );
}

//-----------------------------------------------------------------------------
vtkITKGrowCutSegmentationImageFilter::vtkITKGrowCutSegmentationImageFilter()
{
  this->ObjectSize = 20;
  this->ContrastNoiseRatio = 1.0;
  this->PriorSegmentConfidence = 0.003;
  this->SetNumberOfInputPorts(3);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
template< class IT1>
void ExecuteGrowCut( vtkITKGrowCutSegmentationImageFilter *self,
          vtkImageData *input1,
          vtkImageData *input2,
          vtkImageData *input3,
          vtkImageData *outData,
          vtkInformation* outInfo,
          IT1 *)
{
  int outExt[6];
  int dims[3];
  double spacing[3], origin[3];

  input1->GetDimensions(dims);
  input1->GetOrigin(origin);
  input1->GetSpacing(spacing);
  input1->GetExtent(outExt);

  void *inPtr1 = input1->GetScalarPointerForExtent(outExt);
  void *inPtr2 =  input2->GetScalarPointerForExtent(outExt);
  void *inPtr3 = input3->GetScalarPointerForExtent(outExt);

  outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), outExt);
  outData->SetExtent(outExt);
  outData->SetOrigin(origin);
  outData->SetSpacing(spacing);
  outData->SetDimensions(dims);
  outData->AllocateScalars(outInfo);

  input1->GetExtent(outExt);

  void *outPtr = outData->GetScalarPointerForExtent(outExt);

  // set up progress callback
  itk::CStyleCommand::Pointer progressCommand = itk::CStyleCommand::New();
  progressCommand->SetClientData(static_cast<void *>(self));
  progressCommand->SetCallback(vtkITKImageGrowCutHandleProgressEvent );


  std::cout << " Input2 type is " <<input2->GetScalarType() << std::endl;

  if(input2->GetScalarType() != input3->GetScalarType() )
    {

    bool select2 = true;

    if( select2)
      {

      vtkImageCast *imageCaster = vtkImageCast::New();
      imageCaster->SetInputData( input3 );

      if((input2->GetScalarType() != VTK_UNSIGNED_SHORT) ||
      (input2->GetScalarType() != VTK_UNSIGNED_CHAR) ||
      (input2->GetScalarType() != VTK_UNSIGNED_LONG) ||
      (input2->GetScalarType() != VTK_SHORT) ||
      (input2->GetScalarType() != VTK_CHAR) ||
      (input2->GetScalarType() != VTK_LONG) )
        {

        std::cout<<" Setting to type "<<VTK_SHORT<<std::endl;

        vtkDataObject::SetPointDataActiveScalarInfo(
          outInfo, VTK_SHORT, vtkImageData::GetNumberOfScalarComponents(outInfo));

        imageCaster->SetOutputScalarTypeToShort();

        vtkImageCast *imageCaster1 = vtkImageCast::New();
        imageCaster1->SetInputData(input2);
        imageCaster1->SetOutputScalarTypeToShort();

        vtkITKImageGrowCutExecute3D(input1,
          (IT1*)(inPtr1), (short*)(inPtr2), (short*) (inPtr3),
          (short*)(outPtr),
          self->ObjectSize, self->ContrastNoiseRatio,
          self->PriorSegmentConfidence,
          progressCommand);
        imageCaster1->Delete();
        }
      else
        {
        std::cout<<" setting to type "<<input2->GetScalarType()<<std::endl;
        vtkDataObject::SetPointDataActiveScalarInfo(
          outInfo, input2->GetScalarType(),
          vtkImageData::GetNumberOfScalarComponents(outInfo));
        imageCaster->SetOutputScalarType(input2->GetScalarType() );

        if(input2->GetScalarType() == VTK_UNSIGNED_SHORT)
          {
          vtkITKImageGrowCutExecute3D(input1,
            (IT1*)(inPtr1), (unsigned short*)(inPtr2), (unsigned short*) (inPtr3),
            (unsigned short*)(outPtr),
            self->ObjectSize, self->ContrastNoiseRatio,
            self->PriorSegmentConfidence,
            progressCommand);
          }
        else if (input2->GetScalarType() == VTK_SHORT)
          {
          vtkITKImageGrowCutExecute3D(input1,
            (IT1*)(inPtr1), (short*)(inPtr2), (short*) (inPtr3),
            (short*)(outPtr),
            self->ObjectSize, self->ContrastNoiseRatio,
            self->PriorSegmentConfidence,
            progressCommand);
          }
        else if(input2->GetScalarType() == VTK_UNSIGNED_CHAR)
          {
          vtkITKImageGrowCutExecute3D(input1,
            (IT1*)(inPtr1), (unsigned char*)(inPtr2), (unsigned char*) (inPtr3),
            (unsigned char*)(outPtr),
            self->ObjectSize, self->ContrastNoiseRatio,
            self->PriorSegmentConfidence,
            progressCommand);
          }
        else if(input2->GetScalarType() == VTK_CHAR)
          {
          vtkITKImageGrowCutExecute3D(input1,
            (IT1*)(inPtr1), (char*)(inPtr2), (char*) (inPtr3),
            (char*)(outPtr),
            self->ObjectSize, self->ContrastNoiseRatio,
            self->PriorSegmentConfidence,
            progressCommand);
          }
        else if(input2->GetScalarType() == VTK_UNSIGNED_LONG)
          {
          vtkITKImageGrowCutExecute3D(input1,
            (IT1*)(inPtr1), (unsigned long*)(inPtr2), (unsigned long*) (inPtr3),
            (unsigned long*)(outPtr),
            self->ObjectSize, self->ContrastNoiseRatio,
            self->PriorSegmentConfidence,
            progressCommand);
          }
        else if(input2->GetScalarType() == VTK_LONG)
          {
          vtkITKImageGrowCutExecute3D(input1,
            (IT1*)(inPtr1), (long*)(inPtr2), (long*) (inPtr3),
            (long*)(outPtr),
            self->ObjectSize, self->ContrastNoiseRatio,
            self->PriorSegmentConfidence,
            progressCommand);
          }
        }
      imageCaster->Delete();
      }
    }
  else
    {
    if((input2->GetScalarType() != VTK_UNSIGNED_SHORT) ||
        (input2->GetScalarType() != VTK_UNSIGNED_CHAR) ||
        (input2->GetScalarType() != VTK_UNSIGNED_LONG) ||
        (input2->GetScalarType() != VTK_SHORT) ||
        (input2->GetScalarType() != VTK_CHAR) ||
        (input2->GetScalarType() != VTK_LONG) )
      {

        vtkDataObject::SetPointDataActiveScalarInfo(
          outInfo, VTK_SHORT, vtkImageData::GetNumberOfScalarComponents(outInfo));

      vtkImageCast *imageCaster = vtkImageCast::New();
      imageCaster->SetInputData( input2 );
      imageCaster->SetOutputScalarTypeToShort();

      vtkImageCast *imageCaster1 = vtkImageCast::New();
      imageCaster1->SetInputData(input3);
      imageCaster1->SetOutputScalarTypeToShort();

      vtkITKImageGrowCutExecute3D(input1,
        (IT1*)(inPtr1), (short*)(inPtr2), (short*) (inPtr3),
        (short*)(outPtr),
        self->ObjectSize, self->ContrastNoiseRatio,
        self->PriorSegmentConfidence,
        progressCommand);

      imageCaster1->Delete();
      imageCaster->Delete();
      }
    else
      {
      vtkDataObject::SetPointDataActiveScalarInfo(
        outInfo, input2->GetScalarType(),
        vtkImageData::GetNumberOfScalarComponents(outInfo));

      if(input2->GetScalarType() == VTK_UNSIGNED_SHORT)
        {
        vtkITKImageGrowCutExecute3D(input1,
          (IT1*)(inPtr1), (unsigned short*)(inPtr2), (unsigned short*) (inPtr3),
          (unsigned short*)(outPtr),
          self->ObjectSize, self->ContrastNoiseRatio,
          self->PriorSegmentConfidence,
          progressCommand);
        }
      else if (input2->GetScalarType() == VTK_SHORT)
        {
        vtkITKImageGrowCutExecute3D(input1,
          (IT1*)(inPtr1), (short*)(inPtr2), (short*) (inPtr3),
          (short*)(outPtr),
          self->ObjectSize, self->ContrastNoiseRatio,
          self->PriorSegmentConfidence,
          progressCommand);
        }
      else if(input2->GetScalarType() == VTK_UNSIGNED_CHAR)
        {
        vtkITKImageGrowCutExecute3D(input1,
          (IT1*)(inPtr1), (unsigned char*)(inPtr2), (unsigned char*) (inPtr3),
          (unsigned char*)(outPtr),
          self->ObjectSize, self->ContrastNoiseRatio,
          self->PriorSegmentConfidence,
          progressCommand);
        }
      else if(input2->GetScalarType() == VTK_CHAR)
        {
        vtkITKImageGrowCutExecute3D(input1,
          (IT1*)(inPtr1), (char*)(inPtr2), (char*) (inPtr3),
          (char*)(outPtr),
          self->ObjectSize, self->ContrastNoiseRatio,
          self->PriorSegmentConfidence,
          progressCommand);
        }
      else if(input2->GetScalarType() == VTK_UNSIGNED_LONG)
      {
      vtkITKImageGrowCutExecute3D(input1,
        (IT1*)(inPtr1), (unsigned long*)(inPtr2), (unsigned long*) (inPtr3),
        (unsigned long*)(outPtr),
        self->ObjectSize, self->ContrastNoiseRatio,
        self->PriorSegmentConfidence,
        progressCommand);
      }
      else if(input2->GetScalarType() == VTK_LONG)
        {
        vtkITKImageGrowCutExecute3D(input1,
          (IT1*)(inPtr1), (long*)(inPtr2), (long*) (inPtr3),
          (long*)(outPtr),
          self->ObjectSize, self->ContrastNoiseRatio,
          self->PriorSegmentConfidence,
          progressCommand);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkITKGrowCutSegmentationImageFilter::ExecuteDataWithInformation(
        vtkDataObject *outData, vtkInformation* outInfo)
{
  vtkImageData *input1 = vtkImageData::SafeDownCast(GetInput(0));
  vtkImageData *input2 = vtkImageData::SafeDownCast(GetInput(1));
  vtkImageData *input3 = vtkImageData::SafeDownCast(GetInput(2));

  vtkImageData * out = vtkImageData::SafeDownCast(outData);

  switch(input1->GetScalarType() ) {
    vtkTemplateMacro( ExecuteGrowCut(this, input1, input2,
             input3, out, outInfo,
             static_cast< VTK_TT*>(nullptr)));
    break;
  }
}

//-----------------------------------------------------------------------------
int vtkITKGrowCutSegmentationImageFilter::RequestInformation(
  vtkInformation * request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(1);

  if (inInfo != nullptr)
    {
    this->Superclass::RequestInformation(request, inputVector, outputVector);
    }
  return 1;
}

//-----------------------------------------------------------------------------
void vtkITKGrowCutSegmentationImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Object Size : " << this->ObjectSize << std::endl;
  os << indent << "ContrastNoiseRatio : " << this->ContrastNoiseRatio << std::endl;
}
