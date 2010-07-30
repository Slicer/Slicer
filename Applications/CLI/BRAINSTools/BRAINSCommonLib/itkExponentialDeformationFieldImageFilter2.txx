#ifndef __itkExponentialDeformationFieldImageFilter2_txx
#define __itkExponentialDeformationFieldImageFilter2_txx

#include "itkExponentialDeformationFieldImageFilter2.h"
#include "itkProgressReporter.h"
#include "itkImageRegionConstIterator.h"

namespace itk
{
/**
  * Initialize new instance
  */
template< class TInputImage, class TOutputImage >
ExponentialDeformationFieldImageFilter2< TInputImage, TOutputImage >
::ExponentialDeformationFieldImageFilter2()
{
  m_AutomaticNumberOfIterations = true;
  m_MaximumNumberOfIterations = 20;
  m_ComputeInverse = false;
  m_Divider = DivideByConstantType::New();
  m_Caster = CasterType::New();
  m_Oppositer = OppositerType::New();
  m_Warper = VectorWarperType::New();

  FieldInterpolatorPointer VectorInterpolator =
    FieldInterpolatorType::New();
  m_Warper->SetInterpolator(VectorInterpolator);

  m_Adder = AdderType::New();
  m_Adder->InPlaceOn();
}

/**
  * Print out a description of self
  *
  * \todo Add details about this class
  */
template< class TInputImage, class TOutputImage >
void
ExponentialDeformationFieldImageFilter2< TInputImage, TOutputImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "AutomaticNumberOfIterations: "
     << m_AutomaticNumberOfIterations << std::endl;
  os << indent << "MaximumNumberOfIterations:   "
     << m_MaximumNumberOfIterations << std::endl;
  os << indent << "ComputeInverse:   "
     << ( m_ComputeInverse ? "On" : "Off" ) << std::endl;

  return;
}

/**
  * GenerateData
  */
template< class TInputImage, class TOutputImage >
void
ExponentialDeformationFieldImageFilter2< TInputImage, TOutputImage >
::GenerateData()
{
  itkDebugMacro(<< "Actually executing");

  InputImageConstPointer inputPtr = this->GetInput();

  unsigned int numiter = 0;

  if ( m_AutomaticNumberOfIterations )
    {
    // Compute a good number of iterations based on the rationale
    // that the initial first order approximation,
    // exp(Phi/2^N) = Phi/2^N,
    // needs to be diffeomorphic. For this we simply impose to have
    // max(norm(Phi)/2^N) < 0.5*pixelspacing

    InputPixelRealValueType maxnorm2 = 0.0;

    double minpixelspacing = inputPtr->GetSpacing()[0];
    for ( unsigned int i = 1; i < itkGetStaticConstMacro(ImageDimension); ++i )
      {
      if ( inputPtr->GetSpacing()[i] < minpixelspacing )
        {
        minpixelspacing = inputPtr->GetSpacing()[i];
        }
      }

    typedef ImageRegionConstIterator< InputImageType > InputConstIterator;
    InputConstIterator InputIt = InputConstIterator(
      inputPtr, inputPtr->GetRequestedRegion() );

    for ( InputIt.GoToBegin(); !InputIt.IsAtEnd(); ++InputIt )
      {
      InputPixelRealValueType norm2 = InputIt.Get().GetSquaredNorm();
      if ( norm2 > maxnorm2 ) { maxnorm2 = norm2; }
      }

    // Divide the norm by the minimum pixel spacing
    maxnorm2 /= vnl_math_sqr(minpixelspacing);

    InputPixelRealValueType numiterfloat = 2.0
                                           + 0.5 * vcl_log(maxnorm2) / vnl_math::ln2;

    if ( numiterfloat >= 0.0 )
      {
      // take the ceil and threshold
      numiter = vnl_math_min(
        static_cast< unsigned int >( numiterfloat + 1.0 ),
        m_MaximumNumberOfIterations);
      }
    else
      {
      // numiter will keep the zero to which it was initialized
      }
    }
  else
    {
    numiter = m_MaximumNumberOfIterations;
    }

  ProgressReporter progress(this, 0, numiter + 1, numiter + 1);

  if ( numiter == 0 )
    {
    if ( !this->m_ComputeInverse )
      {
      m_Caster->SetInput(inputPtr);
      m_Caster->GraftOutput( this->GetOutput() );
      m_Caster->Update();
      // Region passing stuff
      this->GraftOutput( m_Caster->GetOutput() );
      }
    else
      {
      m_Oppositer->SetInput(inputPtr);
      m_Oppositer->GraftOutput( this->GetOutput() );
      m_Oppositer->Update();
      // Region passing stuff
      this->GraftOutput( m_Oppositer->GetOutput() );
      }

    this->GetOutput()->Modified();

    progress.CompletedPixel();
    return;
    }

  // Get the first order approximation (division by 2^numiter)
  m_Divider->SetInput(inputPtr);
  m_Divider->GraftOutput( this->GetOutput() );
  if ( !this->m_ComputeInverse )
    {
    m_Divider->SetConstant( static_cast< InputPixelRealValueType >( 1 << numiter ) );
    }
  else
    {
    m_Divider->SetConstant( -static_cast< InputPixelRealValueType >( 1 << numiter ) );
    }

  m_Divider->Update();

  // Region passing stuff
  this->GraftOutput( m_Divider->GetOutput() );
  this->GetOutput()->Modified();

  progress.CompletedPixel();

  // Do the iterative composition of the vector field
  m_Warper->SetOutputOrigin( inputPtr->GetOrigin() );
  m_Warper->SetOutputSpacing( inputPtr->GetSpacing() );
  m_Warper->SetOutputDirection( inputPtr->GetDirection() );

  for ( unsigned int i = 0; i < numiter; i++ )
    {
    m_Warper->SetInput( this->GetOutput() );
    m_Warper->SetDeformationField( this->GetOutput() );

    m_Warper->GetOutput()->SetRequestedRegion(
      this->GetOutput()->GetRequestedRegion() );

    m_Warper->Update();

    OutputImagePointer warpedIm = m_Warper->GetOutput();
    warpedIm->DisconnectPipeline();

    // Remember we chose to use an inplace adder
    m_Adder->SetInput1( this->GetOutput() );

    m_Adder->SetInput2(warpedIm);
    m_Adder->GetOutput()->SetRequestedRegion(
      this->GetOutput()->GetRequestedRegion() );

    m_Adder->Update();

    // Region passing stuff
    this->GraftOutput( m_Adder->GetOutput() );

    // Make a call to modified. This seems only necessary for
    // a non-inplace adder but it doesn't hurt anyhow.
    this->GetOutput()->Modified();

    progress.CompletedPixel();
    }
}
} // end namespace itk

#endif
