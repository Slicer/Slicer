#ifndef __itkBOBFFilter_txx
#define __itkBOBFFilter_txx

#include "itkBOBFFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkNumericTraits.h"
#include <vector>
#include "itkNeighborhoodConnectedImageFilter.h"

namespace itk
{
/*
  *
  */
template< class TInputImage, class TOutputImage >
BOBFFilter< TInputImage, TOutputImage >
::BOBFFilter()
{
  this->SetNumberOfRequiredInputs(2);
  m_Seed.Fill(0);
  m_Lower = NumericTraits< InputPixelType >::NonpositiveMin();
  m_Upper = NumericTraits< InputPixelType >::max();
  m_ReplaceValue = NumericTraits< OutputPixelType >::One;
  m_Radius.Fill(1);
}

/*
  *
  */
template< class TInputImage, class TOutputImage >
void
BOBFFilter< TInputImage, TOutputImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  // os << indent << "UpperGradient: ";
  // os << m_UpperGradient << std::endl;
}

/*
  *
  */
template< class TInputImage, class TOutputImage >
void
BOBFFilter< TInputImage, TOutputImage >::SetInputMask(
  const typename BOBFFilter< TInputImage, TOutputImage >
  ::InputImageType *image)
{
  this->SetNthInput( 1, const_cast< TInputImage * >( image ) );
}

template< class TInputImage, class TOutputImage >
const typename BOBFFilter< TInputImage, TOutputImage >
::InputImageType *
BOBFFilter< TInputImage, TOutputImage >
::GetInputMask()
{
  return static_cast< const TInputImage * >
         ( this->ProcessObject::GetInput(1) );
}

template< class TInputImage, class TOutputImage >
void BOBFFilter< TInputImage, TOutputImage >::GenerateData()
{
  OutputImagePointer     OutputPtr = this->GetOutput();
  InputImageConstPointer InputImage  = this->GetInputImage();
  InputImageConstPointer InputMask  = this->GetInputMask();

  /*Allocate the output*/
  OutputPtr->SetRequestedRegion( InputImage->GetRequestedRegion() );
  OutputPtr->SetBufferedRegion( InputImage->GetBufferedRegion() );
  OutputPtr->SetLargestPossibleRegion( InputImage->GetLargestPossibleRegion() );
  OutputPtr->CopyInformation(InputImage);
  OutputPtr->Allocate();

  typedef ImageRegionConstIterator< TInputImage > InputIterator;
  typedef ImageRegionIterator< TOutputImage >     OutputIterator;

  OutputIterator outItr( OutputPtr, OutputPtr->GetLargestPossibleRegion() );

  InputIterator ImgItr( InputImage, InputImage->GetLargestPossibleRegion() );

  InputIterator MskItr( InputMask, InputMask->GetLargestPossibleRegion() );

  for ( ImgItr.GoToBegin(), MskItr.GoToBegin(), outItr.GoToBegin();
        !ImgItr.IsAtEnd();
        ++ImgItr, ++MskItr, ++outItr )
    {
    if ( MskItr.Get() == 0 )
      {
      outItr.Set(m_ReplaceValue);
      }
    else
      {
      outItr.Set( ImgItr.Get() );
      }
    }
}
}   // end namespace itk

#endif // _itkBOBFFilter_txx
