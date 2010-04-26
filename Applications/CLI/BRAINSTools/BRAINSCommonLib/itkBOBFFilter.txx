#ifndef _itkBOBFFilter_txx
#define _itkBOBFFilter_txx

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
template<class TInputImage, class TOutputImage>
BOBFFilter<TInputImage, TOutputImage>
  ::BOBFFilter()
  {
  this->SetNumberOfRequiredInputs( 2 );
  m_Seed.Fill(0);
  m_Lower = NumericTraits<InputPixelType>::NonpositiveMin();
  m_Upper = NumericTraits<InputPixelType>::max();
  m_ReplaceValue = NumericTraits<OutputPixelType>::One;
  m_Radius.Fill(1);
  }

/*
 *
 */
template<class TInputImage, class TOutputImage>
void
BOBFFilter<TInputImage, TOutputImage>
  ::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  // os << indent << "UpperGradient: ";
  // os << m_UpperGradient << std::endl;
}

/*
 *
 */
template<class TInputImage, class TOutputImage>
void
BOBFFilter<TInputImage, TOutputImage>::SetInputMask(
  const typename BOBFFilter<TInputImage, TOutputImage>
    ::InputImageType *image )
{
  this->SetNthInput( 1, const_cast<TInputImage *>( image ) );
}

template<class TInputImage, class TOutputImage>
const typename BOBFFilter<TInputImage, TOutputImage>
  ::InputImageType *
BOBFFilter<TInputImage, TOutputImage>
  ::GetInputMask()
{
  return static_cast<const TInputImage *>
                                 ( this->ProcessObject::GetInput(1) );
}

#if 0
template<class TInputImage, class TOutputImage>
void BOBFFilter<TInputImage, TOutputImage>::GenerateData()
{
  OutputImagePointer     OutputPtr = this->GetOutput();
  InputImageConstPointer InputImage  = this->GetInputImage();
  InputImageConstPointer InputMask  = this->GetInputMask();

  /*Allocate the output*/
  OutputPtr->SetRequestedRegion( InputImage->GetRequestedRegion() );
  OutputPtr->SetBufferedRegion( InputImage->GetBufferedRegion() );
  OutputPtr->SetLargestPossibleRegion( InputImage->GetLargestPossibleRegion() );
  OutputPtr->Allocate();

  typedef NeighborhoodConnectedImageFilter<InputImageType, InputImageType>
  ConnectedFilterType;
  typename ConnectedFilterType::Pointer Connectedfilter = ConnectedFilterType::
                                                            New();
  Connectedfilter->SetLower( m_Lower );
  Connectedfilter->SetUpper( m_Upper  );
  Connectedfilter->SetRadius( m_Radius );
  Connectedfilter->SetSeed( m_Seed );
  Connectedfilter->SetReplaceValue( m_ReplaceValue );
  Connectedfilter->SetInput( InputImage );

  try
    {
    Connectedfilter->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Exception Object caught: " << std::endl;
    std::cout << err << std::endl;
    exit (-1);
    }

  // OutputPtr=Connectedfilter->GetOutput();

  typedef ImageRegionConstIterator<TInputImage> InputIterator;
  typedef ImageRegionIterator<TOutputImage>     OutputIterator;

  OutputIterator outItr( OutputPtr, OutputPtr->GetLargestPossibleRegion() );

  InputIterator ImgItr( InputImage, InputImage->GetLargestPossibleRegion() );

  InputIterator MskItr( InputMask, InputMask->GetLargestPossibleRegion() );

  InputIterator NbdItr(
    Connectedfilter->GetOutput(),
    Connectedfilter->GetOutput()->GetLargestPossibleRegion() );

  for ( ImgItr.GoToBegin(),
        MskItr.GoToBegin(), NbdItr.GoToBegin(), outItr.GoToBegin();
        !ImgItr.IsAtEnd();
        ++ImgItr, ++MskItr, ++NbdItr, ++outItr )
    {
    outItr.Set( NbdItr.Get() );
    if ( outItr.Get() == 0 )
      {
      outItr.Set( ImgItr.Get() );
      }
    if ( MskItr.Get() == 0 )
      {
      outItr.Set(m_ReplaceValue);
      }
    }
}

#else
template<class TInputImage, class TOutputImage>
void BOBFFilter<TInputImage, TOutputImage>::GenerateData()
{
  OutputImagePointer     OutputPtr = this->GetOutput();
  InputImageConstPointer InputImage  = this->GetInputImage();
  InputImageConstPointer InputMask  = this->GetInputMask();

  /*Allocate the output*/
  OutputPtr->SetRequestedRegion( InputImage->GetRequestedRegion() );
  OutputPtr->SetBufferedRegion( InputImage->GetBufferedRegion() );
  OutputPtr->SetLargestPossibleRegion( InputImage->GetLargestPossibleRegion() );
  OutputPtr->CopyInformation( InputImage );
  OutputPtr->Allocate();

  typedef ImageRegionConstIterator<TInputImage> InputIterator;
  typedef ImageRegionIterator<TOutputImage>     OutputIterator;

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

#endif
}   // end namespace itk

#endif // _itkBOBFFilter_txx
