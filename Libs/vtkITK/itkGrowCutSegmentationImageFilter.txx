#ifndef itkGrowCutSegmentationImageFilter_txx
#define itkGrowCutSegmentationImageFilter_txx


#include "itkGrowCutSegmentationImageFilter.h"


#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkConstantBoundaryCondition.h"
#include "itkNumericTraits.h"
#include "itkImageFileWriter.h"
#include "itkLabelMap.h"
#include "itkShapeLabelObject.h"
#include "itkLabelImageToShapeLabelMapFilter.h"

#include "itkProgressReporter.h"
# include "itkIterationReporter.h"

#include <map>
#include <string>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <utility>

#include <iostream>
#include <ctime>


// #define LOG_OUTPUT
// #define DEBUG_LEVEL_0

#define USE_LABELSHAPEFILTER

namespace itk
{

template< class TInputImage, class TOutputImage, class TWeightPixelType>
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::GrowCutSegmentationImageFilter()
{
  this->SetNumberOfRequiredInputs(3);

  m_RunOneIteration = false;

  m_SetStateImage = false;

  m_SetDistancesImage = false;

  m_SetMaxSaturationImage = false;

  m_ConfThresh = 0.2;

  m_MaxIterations = 500;
  m_ObjectRadius = 10;

  m_SeedStrength = 1.0;

  m_LabelImage = OutputImageType::New();

  m_WeightImage = WeightImageType::New();

  m_ObjectLabel = static_cast<OutputPixelType>( NumericTraits<OutputPixelType>::max(OutputPixelType() ));
  m_BackgroundLabel = static_cast<OutputPixelType>( 1 );
  m_UnknownLabel = static_cast<OutputPixelType>( NumericTraits<OutputPixelType>::ZeroValue() );

  m_Radius.Fill(1);

  // Explicitly disable MultiThreading to workaround issue https://issues.slicer.org/view.php?id=4679
  // TODO: This class could be updated to use the ITKv5 dynamic threading system in the future
  // See https://github.com/InsightSoftwareConsortium/ITK/blob/master/Documentation/ITK5MigrationGuide.md
  this->DynamicMultiThreadingOff();
}


/**
 * Standard PrintSelf method.
 */
template <class TInputImage, class TOutputImage, class TWeightPixelType>
void
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::PrintSelf(std::ostream& os, Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "max iterations : " << m_MaxIterations<< std::endl;
  //   os << indent << "max enemies for attack T1 : " << m_T1<< std::endl;
  // os << indent << "min enemies for submit T2 : " << m_T2<< std::endl;
  os << indent << "starting seed strength :" <<m_SeedStrength<< std::endl;
  //os << indent << "use Algorithm Speed Slow : " << m_UseSlow<< std::endl;
}

template <class TInputImage, class TOutputImage, class TWeightPixelType>
void
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();
  if ( this->GetInput() )
    {
    InputImagePointer image = const_cast< InputImageType * >( this->GetInput() );
    image->SetRequestedRegionToLargestPossibleRegion();
    }
}

template <class TInputImage, class TOutputImage, class TWeightPixelType>
void GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::EnlargeOutputRequestedRegion(DataObject *output)
{
  Superclass::EnlargeOutputRequestedRegion(output);
  output->SetRequestedRegionToLargestPossibleRegion();
}

template <class TInputImage, class TOutputImage, class TWeightPixelType>
void
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::SetStateImage( const OutputImageType *l)
{
  this->ProcessObject::SetNthInput(3, const_cast< OutputImageType *>(l));

  this->SetStateImageOn();
  //m_StateImage->Graft(this->ProcessObject::GetInput(3));
}

template< class TInputImage, class TOutputImage, class TWeightPixelType >
const typename GrowCutSegmentationImageFilter< TInputImage, TOutputImage, TWeightPixelType >::OutputImagePointer
GrowCutSegmentationImageFilter< TInputImage, TOutputImage, TWeightPixelType >
::GetStateImage()
{
  typename OutputImageType::Pointer stateImage = OutputImageType::New();
  stateImage->Graft(static_cast< OutputImageType *>(this->ProcessObject::GetInput(3)));
  return stateImage;
}

template <class TInputImage, class TOutputImage, class TWeightPixelType>
void
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::SetDistancesImage( const WeightImageType* w)
{
  this->ProcessObject::SetNthInput(4, const_cast< WeightImageType*> (w));

  this->SetDistancesImageOn();
  //m_SetDistancesImage = true;
  //m_DistancesImage = static_cast< WeightImageType *>(this->ProcessObject::GetInput(4));
}

template< class TInputImage, class TOutputImage, class TWeightPixelType >
const typename GrowCutSegmentationImageFilter< TInputImage, TOutputImage, TWeightPixelType >::WeightImagePointer
GrowCutSegmentationImageFilter< TInputImage, TOutputImage, TWeightPixelType >
::GetDistancesImage()
{
  typename WeightImageType::Pointer distanceImage = WeightImageType::New();
  distanceImage->Graft(static_cast< WeightImageType *>(this->ProcessObject::GetInput(4)));
  return distanceImage;
  //return const_cast< WeightImageType*> (this->ProcessObject::GetInput(4));
}

template <class TInputImage, class TOutputImage, class TWeightPixelType>
void
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::SetMaxSaturationImage( const WeightImageType* s)
{
  this->ProcessObject::SetNthInput(5, const_cast< WeightImageType *>(s));

  this->SetMaxSaturationImageOn();
  //m_SetMaxSaturationImage = true;
  //m_MaxSaturationImage = static_cast< WeightImageType *>(this->ProcessObject::GetInput(5));
}


template< class TInputImage, class TOutputImage, class TWeightPixelType >
const typename GrowCutSegmentationImageFilter< TInputImage, TOutputImage, TWeightPixelType >::WeightImagePointer
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::GetMaxSaturationImage()
{
  typename WeightImageType::Pointer maxSaturationImage = WeightImageType::New();
  maxSaturationImage->Graft(static_cast< WeightImageType*>(this->ProcessObject::GetInput(5)));
  return maxSaturationImage;
  //return const_cast< WeightImageType*> (this->ProcessObject::GetInput(5));
}

template< class TInputImage, class TOutputImage, class TWeightPixelType >
const typename GrowCutSegmentationImageFilter< TInputImage, TOutputImage, TWeightPixelType >::OutputImagePointer
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::GetLabelImage()
{

  //typename OutputImageType::Pointer labelImage = OutputImageType::New();
  //labelImage->Graft(this->ProcessObject::GetInput(1));
  //return labelImage;
  return m_LabelImage;
  //return const_cast< const OutputImageType*> (this->ProcessObject::GetInput(1));
}

template< class TInputImage, class TOutputImage, class TWeightPixelType >
const typename GrowCutSegmentationImageFilter< TInputImage, TOutputImage, TWeightPixelType >::WeightImagePointer
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::GetStrengthImage()
{
  typename WeightImageType::Pointer weightImage = WeightImageType::New();
  weightImage->Graft(static_cast< WeightImageType*> (this->ProcessObject::GetInput(2)));
  return weightImage;
}


template< class TInputImage, class TOutputImage, class TWeightPixelType >
const typename GrowCutSegmentationImageFilter< TInputImage, TOutputImage, TWeightPixelType >::WeightImagePointer
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::GetUpdatedStrengthImage()
{
  return m_WeightImage;
}

template< class TInputImage, class TOutputImage, class TWeightPixelType >
const typename GrowCutSegmentationImageFilter< TInputImage, TOutputImage, TWeightPixelType >::InputImagePointer
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::GetInputImage()
{
  return const_cast< InputImageType*> (this->GetInput(0));
}

template <class TInputImage, class TOutputImage, class TWeightPixelType>
void
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::Initialize( OutputImageType *output)
{

  // is the output allocated already
  if(output->GetBufferedRegion().GetNumberOfPixels() == 0)
    {
    // allocate memory for the output buffer
    output->SetBufferedRegion( this->GetOutput()->GetRequestedRegion() );
    output->Allocate();
    output->FillBuffer ( m_UnknownLabel );
    }


  if(m_LabelImage->GetBufferedRegion().GetNumberOfPixels() == 0)
    {
    m_LabelImage->CopyInformation( output );
    m_LabelImage->SetBufferedRegion( output->GetBufferedRegion() );
    m_LabelImage->Allocate();
    m_LabelImage->FillBuffer( m_UnknownLabel );
    }


  if( m_WeightImage->GetBufferedRegion().GetNumberOfPixels() == 0)
    {
    m_WeightImage->CopyInformation( output );
    m_WeightImage->SetBufferedRegion( output->GetBufferedRegion() );
    m_WeightImage->Allocate();
    m_WeightImage->FillBuffer( 0.0 );
    }

}

template<class TInputImage, class TOutputImage, class TWeightPixelType>
void GrowCutSegmentationImageFilter< TInputImage, TOutputImage, TWeightPixelType>::
ComputeLabelVolumes(TOutputImage *outputImage, std::vector< unsigned > &volumes,
                    std::vector< unsigned > &physicalVolumes)
{
  //volumes.resize(3);

#ifndef USE_LABELSHAPEFILTER

  std::map<unsigned short, unsigned int>labelMap;
  std::vector< unsigned short > labels;
  unsigned int index = 0;
  ImageRegionConstIteratorWithIndex< OutputImageType > label( outputImage,
                                                              outputImage->GetBufferedRegion());

  for(label.GoToBegin(); !label.IsAtEnd(); ++label)
    {

    unsigned short pix = static_cast<unsigned short>(label.Get());
    std::map<unsigned short, unsigned int>::iterator it = labelMap.find(pix);
    if(it == labelMap.end())
      {
      labelMap.insert(std::pair<unsigned short, unsigned int>(pix, index));
      volumes.push_back(1);
      labels.push_back(pix);
      ++index;
      }
    else
      {
      int i = it->second;
      ++volumes[i];
      }
    }
  // std::cout<<" label \t "<<" volume "<<std::endl;
  // for (unsigned int i = 0; i < index; i++)
  //   {
  //   std::map<unsigned short, unsigned int>::iterator it = labelMap.find(labels[i]);
  //   std::cout<<labels[i]<<"\t"<<volumes[it->second]<<std::endl;
  //   }

#else
  typedef LabelImageToShapeLabelMapFilter< OutputImageType > LabelFilterType;
  typename LabelFilterType::Pointer labelFilter = LabelFilterType::New();

  labelFilter->SetInput(outputImage);
  labelFilter->SetBackgroundValue(0);
  labelFilter->Update();

  unsigned long numObjects = labelFilter->GetOutput()->GetNumberOfLabelObjects();
  volumes.resize((const int)numObjects);
  physicalVolumes.resize((const int)numObjects);
  for (unsigned n = 0; n < numObjects; n++)
    {
    volumes[n] = labelFilter->GetOutput()->
      GetNthLabelObject(n)->GetNumberOfPixels();
    physicalVolumes[n] = (unsigned int)
      (labelFilter->GetOutput()->
       GetNthLabelObject(n)->GetNumberOfPixels()/1000.0);
    }
#endif
}


template<class TInputImage, class TOutputImage, class TWeightPixelType>
bool GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>::
InitializeStateImage( OutputImageType *stateImage)
{
  bool setStart = false;
  //  bool setEnd = false;

  typename OutputImageType::Pointer labelImage = OutputImageType::New();
  labelImage->Graft(static_cast< OutputImageType*> (this->ProcessObject::GetInput(1)));

  typename OutputImageType::SizeType lsize =
    labelImage->GetBufferedRegion().GetSize();

  ImageRegionConstIteratorWithIndex< OutputImageType > label( labelImage,
                                                              labelImage->GetBufferedRegion());
  ImageRegionIterator< OutputImageType > state( stateImage,
                                                stateImage->GetBufferedRegion());

  for (unsigned i = 0; i < labelImage->GetImageDimension(); i++){
  m_RoiStart[i] = 0;
  m_RoiEnd[i] = 0;
  }

  bool foundLabels = false;

  for (label.GoToBegin(), state.GoToBegin(); !label.IsAtEnd();
       ++label, ++state)
    {
    if(label.Get() != m_UnknownLabel)
      {
      foundLabels = true;
      OutputIndexType idx = label.GetIndex();

      if(!setStart)
        {
        m_RoiStart = idx;
        m_RoiEnd = idx;
        setStart = true;
        }
      else
        {
        for (unsigned i = 0; i < labelImage->GetImageDimension(); i++)
          {
          if(idx[i] <= m_RoiStart[i])
            {
            m_RoiStart[i] = idx[i];
            }
          if(idx[i] >= m_RoiEnd[i])
            {
            m_RoiEnd[i] = idx[i];
            }
          }
        }

      state.Set(static_cast< OutputPixelType>(LABELED));
      }
    }

  for (unsigned i = 0; i < labelImage->GetImageDimension(); i++)
    {
    /* m_roiStart[i] = (midROI[i]-m_ObjectRadius >=0 && midROI[i]-m_ObjectRadius <= m_roiStart[i]) ?
   (midROI[i]-m_ObjectRadius) : 0; //m_roiStart[i];
   m_roiEnd[i] = (midROI[i]+m_ObjectRadius < (unsigned)lsize[i] && midROI[i]+m_ObjectRadius >= m_roiEnd[i]) ?
   (midROI[i]+m_ObjectRadius) : lsize[i]-1; //m_roiEnd[i];
      */

    m_RoiStart[i] = (m_RoiStart[i] - m_ObjectRadius >= 0)? (m_RoiStart[i] - m_ObjectRadius) : 0;
    m_RoiEnd[i] = (static_cast<unsigned int>(m_RoiEnd[i] + m_ObjectRadius) < lsize[i]) ?
      (m_RoiEnd[i] + m_ObjectRadius) : lsize[i]-1;
    }

  if(!foundLabels)
    return true;

  // std::cout<< " ROI start ";
  // for (unsigned int i = 0; i < labelImage->GetImageDimension(); i++)
  //   {
  //   std::cout<<m_roiStart[i]<<" ";
  //   }
  // std::cout<<" ROI end ";
  // for (unsigned int i = 0; i < labelImage->GetImageDimension(); i++)
  //   {
  //   std::cout<<m_roiEnd[i]<<" ";
  //   }
  // std::cout<<std::endl;

  return false;

}

template<class TInputImage, class TOutputImage, class TWeightPixelType>
void GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>::
InitializeDistancesImage( TInputImage *inputImage,
                          WeightImageType *distances)
{
  typedef ConstNeighborhoodIterator<InputImageType> InputNeighborhoodIteratorType;
  typedef typename InputNeighborhoodIteratorType::RadiusType InputRadiusType;
  InputRadiusType radiusIn = static_cast<InputRadiusType> (m_Radius);

  InputNeighborhoodIteratorType input(radiusIn,
                                      inputImage,
                                      inputImage->GetBufferedRegion());


  ImageRegionIterator< WeightImageType> dist(distances,
                                             distances->GetBufferedRegion());

  for(input.GoToBegin(), dist.GoToBegin(); !input.IsAtEnd();
      ++input, ++dist)
    {

    WeightPixelType center = static_cast< WeightPixelType>(input.GetCenterPixel());
    WeightPixelType maxDistance = 0.0;
    WeightPixelType distance;

    for (unsigned i = 0; i < input.Size(); i++)
      {
      WeightPixelType pix = static_cast< WeightPixelType>(input.GetPixel(i));

      distance = (pix - center)*(pix-center);
      maxDistance = (distance > maxDistance) ? distance :
        maxDistance;
      }

    dist.Set(maxDistance);
    }
}

template<class TInputImage, class TOutputImage, class TWeightPixelType>
void GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>::
MaskSegmentedImageByWeight( float confThresh)
{
  ImageRegionConstIterator< WeightImageType > weight( m_WeightImage,
                                                      m_WeightImage->GetBufferedRegion());

  ImageRegionIterator< OutputImageType > label( m_LabelImage,
                                                m_LabelImage->GetBufferedRegion());

  //ImageRegionIterator< OutputImageType > label( this->GetOutput(),
  //    this->GetOutput()->GetBufferedRegion());

  weight.GoToBegin();
  label.GoToBegin();
  //  unsigned int numInPixels = 0;
  //unsigned int numOutPixels = 0;

  for(;!weight.IsAtEnd(); ++weight, ++label)
    {
    if(weight.Get() < confThresh)
      {
      label.Set(static_cast< OutputPixelType > (0));
      //  ++numOutPixels;
      }
    //else
    // {
    //  ++numInPixels;
    // }
    }

  //return (numInPixels > numOutPixels);
}


// TODO: The class incorrectly overrides both: GenerateData and ThreadedGenerateData, this
// should be fixed. See https://issues.slicer.org/view.php?id=4679
template <class TInputImage, class TOutputImage, class TWeightPixelType>
void
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::GenerateData()
{
  IterationReporter iterate(this, 0, 1);

  // if the filter is configured to run a single iteration, use the superclass
  // GenerateData. This will engage the multi-threaded version of the GenerateData

  typename OutputImageType::Pointer output = this->GetOutput();
  this->Initialize(output);

  // TODO: RunOneIteration does the opposite of what is intended.
  // Superclass::GenerateData will call this class ThreadedGenerateData, so it will run multi-threaded.
  // If m_RunOneIteration is false, ThreadedGenerateData is NOT called.
  // To update to ITKv5, remove GenerateData, change the name of ThreadedGenerateData to
  // DynamicThreadedGenerateData and that's it.
  // Test if both implementations, GenerateData and ThreadedGenerateData and equivalent.
  if(m_RunOneIteration)
    {
    Superclass::GenerateData();
    iterate.CompletedStep();

    m_LabelImage = this->GetOutput();
    return;
    }


  unsigned int ndims = static_cast< unsigned int>(output->GetImageDimension());

  typename InputImageType::Pointer inputImage = InputImageType::New();
  inputImage->Graft( static_cast< InputImageType*>(this->ProcessObject::GetInput(0)));

  typename OutputImageType::Pointer pixelStateImage = OutputImageType::New();
  typename WeightImageType::Pointer maxDistancesImage = WeightImageType::New();
  typename WeightImageType::Pointer maxSaturationImage = WeightImageType::New();


  bool converged = false;

  // Initialize the state image
  if( !m_SetStateImage)
    {

    pixelStateImage->CopyInformation( this->GetLabelImage() );
    pixelStateImage->SetBufferedRegion( this->GetLabelImage()->GetBufferedRegion() );
    pixelStateImage->Allocate();
    pixelStateImage->FillBuffer(UNLABELED);

    converged = this->InitializeStateImage(pixelStateImage);
    }


  // Initialize the max distance for each pixel in the image
  if( !m_SetDistancesImage)
    {

    maxDistancesImage->CopyInformation(this->GetStrengthImage());
    maxDistancesImage->SetBufferedRegion( this->GetStrengthImage()->GetBufferedRegion() );
    maxDistancesImage->Allocate();
    maxDistancesImage->FillBuffer(UNLABELED);
    this->InitializeDistancesImage(inputImage, maxDistancesImage);
    }


  if( !m_SetMaxSaturationImage )
    {
    maxSaturationImage->CopyInformation( inputImage );
    maxSaturationImage->SetBufferedRegion( inputImage->GetBufferedRegion() );
    maxSaturationImage->Allocate();
    maxSaturationImage->FillBuffer( 0 );
    }

  /////////////////////////////////////////////////////////////////

  // Filter was configured to run until convergence. We need to delegate a different instance of the filter to run on each iteration.
  // std::cout<<" Running filter until convergence .... "<<std::endl;

  typename GrowCutSegmentationImageFilter< TInputImage, TOutputImage, TWeightPixelType >::Pointer
    singleIteration =
    GrowCutSegmentationImageFilter< TInputImage, TOutputImage, TWeightPixelType>::New();
  // set up the singleIteration Filter.
  singleIteration->RunOneIterationOn();
  singleIteration->SetInputImage( this->GetInputImage());
  singleIteration->SetLabelImage( this->GetLabelImage() );
  singleIteration->SetStrengthImage (this->GetStrengthImage() );

  if(m_SetStateImage)
    {
    singleIteration->SetStateImage( this->GetStateImage() );
    }
  else
    {
    singleIteration->SetStateImage(pixelStateImage);
    }

  if(m_SetDistancesImage)
    {
    singleIteration->SetDistancesImage( this->GetDistancesImage() );
    }
  else
    {
    singleIteration->SetDistancesImage(maxDistancesImage);
    }
  if(m_SetMaxSaturationImage)
    {
    singleIteration->SetMaxSaturationImage( this->GetMaxSaturationImage() );
    }
  else
    {
    singleIteration->SetMaxSaturationImage(maxSaturationImage);
    }

  // std::cout<<" Setting other filter parameters ... "<<std::endl;
  singleIteration->SetSeedStrength( this->GetSeedStrength());
  singleIteration->SetMaxIterations( this->GetMaxIterations());
  singleIteration->SetObjectRadius(this->GetObjectRadius() );

  singleIteration->SetROIStart( this->GetROIStart() );
  singleIteration->SetROIEnd( this->GetROIEnd() );

  singleIteration->GetOutput()->SetBufferedRegion(
    this->GetOutput()->GetBufferedRegion() );

  // Create Progress Accumulator for tracking the progress
  //ProgressAccumulator::Pointer progress = ProgressAccumulator::New();
  //progress->SetMiniPipelineFilter( this );
  //progress->RegisterInternalFilter( singleIterationFilter, 1.0f );

  // run until convergence

  unsigned int iter = 0;
  //time_t startTime;
  //time_t endTime;
  time_t start;
  time_t end;

  unsigned int maxRadius = 0;

  unsigned totalROIVolume = 1;

  for (unsigned n = 0; n < ndims; n++)
    {
    //prevUnlabeledPix *= (m_roiEnd[n]-m_roiStart[n]);
    totalROIVolume *= (m_RoiEnd[n]-m_RoiStart[n]);

    if(n == 0)
      {
      maxRadius = m_RoiEnd[n]-m_RoiStart[n];
      }
    else if(maxRadius < (m_RoiEnd[n]-m_RoiStart[n]))
      {
      maxRadius = m_RoiEnd[n]-m_RoiStart[n];
      }
    }

  maxRadius = static_cast< unsigned int>(std::ceil(maxRadius*0.5));

  // float threshSaturation = .96; //.999; // Determine the saturation according to the size of the object
  // float threshUnchanged = 0.05;
  // float threshUnlabeledLimit = 0;

  unsigned threshSaturation = 96; // .96*100
  unsigned threshUnchanged = 5;

  time(&start);

  // set up the grow cut update filter here...

  unsigned prevModifiedPix = 0;
  while (iter < m_MaxIterations && !converged)
    {
    //std::cout<<" running iteration ...... "<<iter<<std::endl;
    singleIteration->Update();
    //std::cout<<" DONE"<<std::endl;
    iterate.CompletedStep();

    // Check for convergence
    //pixelStateImage = singleIteration->GetStateImage();

    unsigned changeablePix =
      singleIteration->GetLabeled();//updateFilter->GetNumLabeledPixels();
    unsigned currLocallySaturatedPix =
      singleIteration->GetLocallySaturated();//updateFilter->GetNumLocallySaturatedPixels();
    unsigned currSaturatedPix =
      singleIteration->GetSaturated();//updateFilter->GetNumSaturatedPixels();

    unsigned unlabeled = (changeablePix + currLocallySaturatedPix + currSaturatedPix <= totalROIVolume) ?
      (static_cast<unsigned > (totalROIVolume) -(currLocallySaturatedPix +
                                                 currSaturatedPix + changeablePix)) : 0;

    unsigned currModified = currLocallySaturatedPix + currSaturatedPix;

    unsigned relUnlabeled = (unlabeled*100)/totalROIVolume;
    unsigned relSaturated = ((currSaturatedPix+currLocallySaturatedPix)*100)/totalROIVolume;

    // unsigned permodified = (currModified > prevModifiedPix) ?
    //  ((currModified-prevModifiedPix)*100)/(std::max(static_cast<unsigned int>(1), std::max(currModified, prevModifiedPix))) :
    //  ((prevModifiedPix - currModified)*100)/(std::max(static_cast<unsigned int>(1), std::max(currModified, prevModifiedPix)));

    converged = (prevModifiedPix == currModified) ||
      (relUnlabeled < threshUnchanged && relSaturated > threshSaturation);

    // if((iter % 10 == 0) || converged)
    //   {
      // std::cout<<" ITER "<<iter<<std::endl;
      // std::cout<<" saturated Pixels "<<currSaturatedPix<<"( "<<
      //   totalROIVolume<<") %"<<currSaturatedPix/(float)totalROIVolume<<std::endl;
      // std::cout<<" locally saturated Pixels "<<currLocallySaturatedPix<<" %"<<
      //   currLocallySaturatedPix/(float)totalROIVolume<<std::endl;
      // std::cout<<"Number of Labeled Pixels "<<changeablePix<<" %"<<
      //   changeablePix/(float)totalROIVolume<<std::endl;
      // std::cout<<"Curr Saturated "<<relSaturated<<" %"<<std::endl;
      // std::cout<<" Prev Modified "<<prevModifiedPix<<" Curr Modified "<<currModified<<std::endl;
      // std::cout<<"Unlabeled pixels "<<unlabeled<<" %"<<
      //   relUnlabeled<<" Per modified "<<permodified<<std::endl;
      // if(converged)
      //   std::cout<<" converged...."<<std::endl;
      // }

    prevModifiedPix = currModified;
    ++iter;
    m_WeightImage = singleIteration->GetUpdatedStrengthImage();

    if(!converged)
      {
      // assign the old output as input
      m_WeightImage->DisconnectPipeline();
      //m_LabelImage->DisconnectPipeline();

      singleIteration->SetStrengthImage(m_WeightImage);
      //singleIteration->SetLabelImage(m_LabelImage);
      singleIteration->GetOutput()->
        SetBufferedRegion(output->GetBufferedRegion() );
      }

    this->UpdateProgress((currSaturatedPix+currLocallySaturatedPix)/(float)totalROIVolume);
    }


  this->UpdateProgress(1.0);
  m_WeightImage = singleIteration->GetUpdatedStrengthImage();
  m_LabelImage = singleIteration->GetOutput();

  this->MaskSegmentedImageByWeight(m_ConfThresh);

  this->GraftOutput(m_LabelImage);

  time(&end);

  std::vector< unsigned > labelVolumes;
  std::vector< unsigned > physicalVolumes;

  this->ComputeLabelVolumes(m_LabelImage, labelVolumes, physicalVolumes);

  // std::cout<<"...................................................."<<std::endl;
  // for (unsigned n = 0; n < labelVolumes.size(); n++)
  //   {
  //   std::cout<<" Object "<<n<<" volume "<<labelVolumes[n]<<" Physical Volume "<<
  //     physicalVolumes[n]<<std::endl;
  //   }

  /*
  typedef ImageFileWriter < OutputImageType > WriterType;
  typename WriterType::Pointer writer = WriterType::New();

  std::string outfilename = "segmented_img.mhd";
  writer->SetFileName(outfilename.c_str());
  writer->SetInput(m_LabelImage);
  writer->Update();
  */
  // std::cout<<" Time taken for segmentation "<<iter<<
  //   " : "<<difftime(end, start)<<" secs"<<std::endl;

}


template <class TInputImage, class TOutputImage, class TWeightPixelType>
void GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
  ::ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread,
                          ThreadIdType itkNotUsed(threadId))
{
  // make a shallow copy of the image to break the pipeline from re-executing in each thread
  typename InputImageType::Pointer inputImage = InputImageType::New();
  inputImage->Graft( static_cast< InputImageType*>(this->ProcessObject::GetInput(0)));

  typename OutputImageType::Pointer labelImage = OutputImageType::New();
  labelImage->Graft(static_cast< OutputImageType*>(this->ProcessObject::GetInput(1)) );

  typename WeightImageType::Pointer weightImage = WeightImageType::New();
  weightImage->Graft(static_cast< WeightImageType*>(this->ProcessObject::GetInput(2)) );

  typename OutputImageType::Pointer stateImage = OutputImageType::New();
  stateImage->Graft(static_cast< OutputImageType*>(this->ProcessObject::GetInput(3)) );

  typename WeightImageType::Pointer distanceImage = WeightImageType::New();
  distanceImage->Graft(static_cast< WeightImageType*>(this->ProcessObject::GetInput(4)) );

  typename WeightImageType::Pointer maxSaturationImage = WeightImageType::New();
  maxSaturationImage->Graft(static_cast< WeightImageType*>(this->ProcessObject::GetInput(5)) );
  //maxSaturationImage->Graft(m_MaxSaturationImage );

  typename OutputImageType::Pointer outputImage = OutputImageType::New();
  outputImage->Graft(this->GetOutput() );


  //////////////////////////////////////////////////////////////////

  typename OutputImageType::IndexType regionStart = outputRegionForThread.GetIndex();
  typename OutputImageType::SizeType regionSize = outputRegionForThread.GetSize();

  unsigned int Dimension = inputImage->GetImageDimension();

  // Extract the image regions corresponding to the seed and ROI defined by the radius
  typename InputImageType::IndexType iIndex;
  typename InputImageType::SizeType iSize;


  for (unsigned d = 0; d < Dimension; d++)
    {
    unsigned long int x = m_RoiEnd[d]-m_RoiStart[d]+1; // > 0 ? m_roiEnd[d]-m_roiStart[d] : 0;
    iIndex[d] = m_RoiStart[d] > regionStart[d] ? m_RoiStart[d] : regionStart[d];
    iSize[d] = x < regionSize[d] ? (x > 0 ? x : 1) : regionSize[d];

    }

  typedef typename InputImageType::RegionType  InputRegionType;
  typedef typename OutputImageType::RegionType OutputRegionType;
  typedef typename WeightImageType::RegionType WeightRegionType;

  InputRegionType input;
  input.SetIndex( iIndex );
  input.SetSize( iSize );

  OutputRegionType label;
  label.SetIndex( iIndex);
  label.SetSize( iSize);

  WeightRegionType weight;
  weight.SetIndex( iIndex);
  weight.SetSize( iSize);

  typedef ConstNeighborhoodIterator<InputImageType> InputIteratorType;
  typedef typename InputIteratorType::RadiusType    InputRadiusType;

  typedef NeighborhoodIterator<OutputImageType>  LabelIteratorType;
  typedef typename LabelIteratorType::RadiusType LabelRadiusType;

  typedef NeighborhoodIterator<WeightImageType>   WeightIteratorType;
  typedef typename WeightIteratorType::RadiusType WeightRadiusType;


  InputRadiusType radiusIn; //= static_cast< InputRadiusType >(m_Radius);
  WeightRadiusType radiusWt; //= static_cast< WeightRadiusType >(m_Radius);
  LabelRadiusType radiusLab; //= static_cast< LabelRadiusType >(m_Radius);
  radiusIn.Fill(1);
  radiusWt.Fill(1);
  radiusLab.Fill(1);

  ConstantBoundaryCondition<InputImageType> cbci;
  cbci.SetConstant( NumericTraits< InputPixelType >::min(InputPixelType()) );

  ConstantBoundaryCondition<OutputImageType> cbcl;
  cbcl.SetConstant( NumericTraits< OutputPixelType >::min(OutputPixelType()) );

  ConstantBoundaryCondition<WeightImageType> cbcw;
  cbcw.SetConstant( NumericTraits< WeightPixelType >::min(WeightPixelType()) );


  InputPixelType minI = NumericTraits< InputPixelType > ::min(InputPixelType());
  WeightPixelType minW = NumericTraits< WeightPixelType > ::min(WeightPixelType());

  /////////////////////////////////////////////////////////////////////////////////////////

  // Boundary Faces Calculator
  typedef typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator< InputImageType > FaceCalculatorType;
  FaceCalculatorType faceCalculator;
  typename FaceCalculatorType::FaceListType faceList;

  faceList = faceCalculator( inputImage, input, radiusIn);

  // FaceList Iterators
  typename FaceCalculatorType::FaceListType::iterator fitIn;

  //std::cout<<" number faces "<<faceList.size()<<std::endl;
  //bool first = true;
  for( fitIn = faceList.begin(); fitIn != faceList.end(); ++fitIn)
    {

    //fitIn->Print(std::cout);
    //first = true;

    InputIteratorType inputIt; //(radiusIn, inputImage,input);

    LabelIteratorType oldLabelIt; //(radiusLab, labelImage, label);

    typedef ImageRegionIterator< OutputImageType > IteratorOut; // newLabelIt(this->GetOutput(), label);

    IteratorOut newLabelIt;

    WeightIteratorType oldWeightIt; //(radiusWt, weightImage, weight);

    typedef ImageRegionIterator< WeightImageType > IteratorWeight; //newWeightIt(m_WeightImage, weight);

    IteratorWeight newWeightIt;
    IteratorWeight distancesIt;

    WeightIteratorType maxSatIt;
    LabelIteratorType stateIt;


    inputIt = InputIteratorType(radiusIn, inputImage, *fitIn);

    //oldLabelIt = LabelIteratorType(radiusLab, labelImage, *fitIn);

    oldLabelIt = LabelIteratorType(radiusLab, m_LabelImage, *fitIn);

    oldWeightIt = WeightIteratorType(radiusWt, weightImage, *fitIn);

    maxSatIt = WeightIteratorType(radiusWt, maxSaturationImage, *fitIn);

    stateIt = LabelIteratorType(radiusLab, stateImage, *fitIn);


    inputIt.OverrideBoundaryCondition( &cbci );
    oldLabelIt.OverrideBoundaryCondition( &cbcl );
    stateIt.OverrideBoundaryCondition( &cbcl );
    oldWeightIt.OverrideBoundaryCondition( &cbcw );
    maxSatIt.OverrideBoundaryCondition( &cbcw );

    newLabelIt = IteratorOut(outputImage, *fitIn);
    newWeightIt = IteratorWeight(m_WeightImage, *fitIn);
    distancesIt = IteratorWeight(distanceImage, *fitIn);


    inputIt.GoToBegin(); oldLabelIt.GoToBegin(); newLabelIt.GoToBegin();
    oldWeightIt.GoToBegin(); newWeightIt.GoToBegin(); stateIt.GoToBegin();
    //newstateIt.GoToBegin();
    distancesIt.GoToBegin(); maxSatIt.GoToBegin();

    for (; !inputIt.IsAtEnd(); ++inputIt, ++oldLabelIt,
           ++newLabelIt, ++oldWeightIt, ++newWeightIt,
           ++stateIt, ++distancesIt, ++maxSatIt) // ++newstateIt;
      {

      OutputPixelType s_center = stateIt.GetCenterPixel();
      OutputPixelType l_center = oldLabelIt.GetCenterPixel();
      WeightPixelType w_center = oldWeightIt.GetCenterPixel();

      if(s_center == SATURATED)
        {
        //newstateIt.Set(s_center);
        newLabelIt.Set(l_center);
        newWeightIt.Set(w_center);
        continue;
        }

      InputPixelType f_center = inputIt.GetCenterPixel();

      OutputPixelType winnerLabel = l_center;
      WeightPixelType winnerWeight = w_center;

      WeightPixelType maxDist = distancesIt.Get();

      unsigned int countSaturatedLinks = 0;
      unsigned int countLocalSaturatedLinks = 0;

      bool modified = false;
      WeightPixelType maxWt = 0.0;

      unsigned int nlinks = 0;

      for (unsigned k = 0; k < inputIt.Size(); k++)
        {

        InputPixelType f = inputIt.GetPixel(k);
        WeightPixelType w = oldWeightIt.GetPixel(k);


        if(f == minI && w == minW)
          {
          continue;
          }

        OutputPixelType s = stateIt.GetPixel(k);
        OutputPixelType l = oldLabelIt.GetPixel(k);


        ++nlinks;

        WeightPixelType attackWeight = (f_center - f)*(f_center - f);
        attackWeight = (maxDist > 0) ? (1.0 - attackWeight/maxDist) : 1.0;


        WeightPixelType maxAttackWeight = 0.0; // attackWeight*maxSatIt.GetPixel(k);
        WeightPixelType msat = maxSatIt.GetPixel(k);

        maxAttackWeight = (s == UNLABELED) ? 0.0 :
          ((msat == 0.0) ? (attackWeight * this->GetSeedStrength()) :
           ((s == SATURATED) ? attackWeight * w : attackWeight*msat ) );

        attackWeight *= w;

        maxWt = (maxWt < maxAttackWeight) ? maxAttackWeight : maxWt;


        if(s_center != UNLABELED)
          {
          countSaturatedLinks += (maxAttackWeight <= w_center) ? 1 : 0;
          countLocalSaturatedLinks += (attackWeight <= w_center) ? 1 : 0;
          }

        if(s != UNLABELED && attackWeight > winnerWeight)
          {

          winnerWeight = attackWeight;
          winnerLabel = l;
          modified = true;

          stateIt.SetCenterPixel(LABELED);
          //newstateIt.Set(LABELED);
          }
        }

      if(nlinks > 0)
        {
        if(countSaturatedLinks == nlinks && winnerLabel != m_UnknownLabel)
          {
          stateIt.SetCenterPixel(SATURATED);
          }
        else{
        if (countLocalSaturatedLinks == nlinks && winnerLabel != m_UnknownLabel)
          {
          stateIt.SetCenterPixel(LOCALLY_SATURATED);
          }
        else if (stateIt.GetCenterPixel() != UNLABELED && !modified)
          {
          stateIt.SetCenterPixel(LOCALLY_SATURATED);
          }
        }
        }

      //if(nlinks > 0 && countSaturatedLinks == nlinks && winnerLabel != m_UnknownLabel)
      // {
      //   stateIt.SetCenterPixel(SATURATED);
      //   //newstateIt.Set(SATURATED);
      // }
      //else if((winnerLabel != m_UnknownLabel) &&
      //    ((nlinks > 0) &&
//       (countLocalSaturatedLinks == nlinks)) ||
//      ((stateIt.GetCenterPixel() != UNLABELED) &&
//       !modified))
//      {
//        stateIt.SetCenterPixel(LOCALLY_SATURATED);
//        //newstateIt.Set(LOCALLY_SATURATED);
//      }

      maxSatIt.SetCenterPixel(maxWt);
      newLabelIt.Set(winnerLabel);
      newWeightIt.Set(winnerWeight);
      }
    }


}


template <class TInputImage, class TOutputImage, class TWeightPixelType>
void
GrowCutSegmentationImageFilter<TInputImage, TOutputImage, TWeightPixelType>
::AfterThreadedGenerateData()
{
  m_Labeled = 0;
  m_LocallySaturated = 0;
  m_Saturated = 0;

  typename OutputImageType::Pointer stateImage = OutputImageType::New();
  stateImage->Graft(static_cast< OutputImageType*>(this->ProcessObject::GetInput(3)) );

  ImageRegionIterator< OutputImageType > state(stateImage, stateImage->GetBufferedRegion());
  ImageRegionIterator< WeightImageType > weight(m_WeightImage, m_WeightImage->GetBufferedRegion());


  for(state.GoToBegin(), weight.GoToBegin(); !state.IsAtEnd(); ++state, ++weight)
    {
    if(state.Get() == LABELED)
      {
      ++m_Labeled;
      }
    if(state.Get() == LOCALLY_SATURATED && weight.Get() >= m_ConfThresh)
      {
      ++m_LocallySaturated;
      }
    if(state.Get() == SATURATED && weight.Get() > m_ConfThresh)
      {
      ++m_Saturated;
      }

    }
}

}//namespace itk

#endif
