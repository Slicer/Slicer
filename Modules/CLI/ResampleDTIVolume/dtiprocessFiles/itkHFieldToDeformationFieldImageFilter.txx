// Copied from dtiprocess
// available there: http://www.nitrc.org/projects/dtiprocess/
/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkHFieldToDeformationFieldImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2010/03/09 18:01:02 $
  Version:   $Revision: 1.2 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef itkHFieldToDeformationFieldImageFilter_txx
#define itkHFieldToDeformationFieldImageFilter_txx

#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>

namespace itk
{

// const double eps = 1e-16;
template <typename TInputImage, typename TOutputImage>
void HFieldToDeformationFieldImageFilter<TInputImage, TOutputImage>::GenerateData()
{
//  Superclass::GenerateInputRequestedRegion();
//  outputImage->SetRequrestedRegion(inputImage
  this->AllocateOutputs();

  const typename InputImageType::ConstPointer input(this->GetInput() );

  typename OutputImageType::Pointer output(this->GetOutput() );

  typename InputImageType::RegionType inputRequestedRegion(input->GetRequestedRegion() );

  typename OutputImageType::RegionType outputRequestedRegion(output->GetRequestedRegion() );

  ImageRegionConstIteratorWithIndex<InputImageType> it = ImageRegionConstIteratorWithIndex<InputImageType>(
      input, inputRequestedRegion);

  ImageRegionIterator<OutputImageType> oit = ImageRegionIterator<OutputImageType>(
      output, outputRequestedRegion);
  //  typename InputImageType::SpacingType spacing = input->GetSpacing();
  for( it.GoToBegin(), oit.GoToBegin(); !it.IsAtEnd(); ++it, ++oit )
    {
    InputPixelType hvec = it.Get();
    typename OutputImageType::IndexType index = it.GetIndex();

    oit.Set(this->ComputeDisplacement(input, index, hvec) );
    }

}

template <typename TInputImage, typename TOutputImage>
typename TOutputImage::PixelType
HFieldToDeformationFieldImageFilter<TInputImage, TOutputImage>::ComputeDisplacement(
  typename InputImageType::ConstPointer input,
  typename InputImageType::IndexType
  ind,
  typename InputImageType::PixelType
  hvec)
{
  typedef typename InputPixelType::ValueType CoordRepType;
  const unsigned int Dimension = InputImageType::ImageDimension;

  typedef itk::Point<CoordRepType, Dimension> PointType;
  PointType ipt, hpt;

  typedef ContinuousIndex<CoordRepType, Dimension> ContinuousIndexType;
  ContinuousIndexType hind;
  for( unsigned int i = 0; i < Dimension; ++i )
    {
    hind[i] = hvec[i];
    }

  input->TransformIndexToPhysicalPoint(ind, ipt);
  input->TransformContinuousIndexToPhysicalPoint(hind, hpt);

  return hpt - ipt;
}

} // namespace itk

#endif
