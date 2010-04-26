#ifndef __itkVelocityFieldBCHCompositionFilter_txx
#define __itkVelocityFieldBCHCompositionFilter_txx
#include "itkVelocityFieldBCHCompositionFilter.h"

#include <itkProgressAccumulator.h>

namespace itk
{

/**
 * Default constructor.
 */
template <class TInputImage, class TOutputImage>
VelocityFieldBCHCompositionFilter<TInputImage,TOutputImage>
::VelocityFieldBCHCompositionFilter()
{
  // Setup the number of required inputs
  this->SetNumberOfRequiredInputs( 2 );

  // By default we shouldn't be inplace
  this->InPlaceOff();

  // Set number of apprximation terms to default value
  m_NumberOfApproximationTerms = 2;

  // Declare sub filters
  m_Adder = AdderType::New();
  m_LieBracketFilter = LieBracketFilterType::New();
  m_LieBracketFilter2 = LieBracketFilterType::New();
  m_Multiplier = MultiplierType::New();
  m_Multiplier2 = MultiplierType::New();

  // Multipliers can always be inplace here
  m_Multiplier->InPlaceOn();
  m_Multiplier2->InPlaceOn();

  m_Multiplier->SetConstant( 0.5 );
  m_Multiplier2->SetConstant( 1.0/12.0 );
}


/**
 * Standard PrintSelf method.
 */
template <class TInputImage, class TOutputImage>
void
VelocityFieldBCHCompositionFilter<TInputImage,TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Adder: " << m_Adder << std::endl;
  os << indent << "LieBracketFilter: " << m_LieBracketFilter << std::endl;
  os << indent << "LieBracketFilter2: " << m_LieBracketFilter2 << std::endl;
  os << indent << "Multiplier: " << m_Multiplier << std::endl;
  os << indent << "Multiplier2: " << m_Multiplier2 << std::endl;
  os << indent << "NumberOfApproximationTerms: " << m_NumberOfApproximationTerms << std::endl;
} 


/** 
 * GenerateData()
 */
template <class TInputImage, class TOutputImage>
void
VelocityFieldBCHCompositionFilter<TInputImage,TOutputImage>
::GenerateData()
{
  InputFieldConstPointer leftField = this->GetInput(0);
  InputFieldConstPointer rightField = this->GetInput(1);

  // Create a progress accumulator for tracking the progress of minipipeline
  ProgressAccumulator::Pointer progress = ProgressAccumulator::New();
  progress->SetMiniPipelineFilter(this);
  
  switch ( m_NumberOfApproximationTerms )
    {
    case 2:
      {
      // lf + rf
      progress->RegisterInternalFilter(m_Adder, 1.0);
      
      m_Adder->SetInput( 0, leftField );
      m_Adder->SetInput( 1, rightField );
      m_Adder->SetInPlace( this->GetInPlace() );
      break;
      }
    case 3:
      {
      // lf + rf + 0.5*liebracket(lf,rf)
      progress->RegisterInternalFilter(m_LieBracketFilter, 0.5);
      progress->RegisterInternalFilter(m_Multiplier, 0.2);
      progress->RegisterInternalFilter(m_Adder, 0.3);
      
      m_LieBracketFilter->SetInput( 0, leftField );
      m_LieBracketFilter->SetInput( 1, rightField );
      
      m_Multiplier->SetInput( m_LieBracketFilter->GetOutput() );
      // constant set to 0.5 in constructor
      
      m_Adder->SetInput( 0, m_Multiplier->GetOutput() );
      m_Adder->SetInput( 1, leftField );
      m_Adder->SetInput( 2, rightField );
#if ( ITK_VERSION_MAJOR < 3 ) || ( ITK_VERSION_MAJOR == 3 && ITK_VERSION_MINOR < 13 )
      // Work-around for http://www.itk.org/Bug/view.php?id=8672
      m_Adder->InPlaceOff();
#else
      // Adder can be inplace since the 0th input is a temp field
      m_Adder->InPlaceOn();
#endif
      break;
      }
    case 4:
      {
      // lf + rf + 0.5*liebracket(lf,rf) + (1/12)*liebracket(lf,*liebracket(lf,rf))
      progress->RegisterInternalFilter(m_LieBracketFilter, 0.3);
      progress->RegisterInternalFilter(m_Multiplier, 0.15);
      progress->RegisterInternalFilter(m_LieBracketFilter2, 0.3);
      progress->RegisterInternalFilter(m_Multiplier2, 0.15);
      progress->RegisterInternalFilter(m_Adder, 0.1);
      
      m_LieBracketFilter->SetInput( 0, leftField );
      m_LieBracketFilter->SetInput( 1, rightField );

      m_LieBracketFilter2->SetInput( 0, leftField );
      m_LieBracketFilter2->SetInput( 1, m_LieBracketFilter->GetOutput() );
      
      m_Multiplier->SetInput( m_LieBracketFilter->GetOutput() );
      // constant set to 0.5 in constructor

      m_Multiplier2->SetInput( m_LieBracketFilter2->GetOutput() );
      // constant set to 1/12 in constructor

      m_Adder->SetInput( 0, m_Multiplier->GetOutput() );
      m_Adder->SetInput( 1, leftField );
      m_Adder->SetInput( 2, rightField );
      m_Adder->SetInput( 3, m_Multiplier2->GetOutput() );
#if ( ITK_VERSION_MAJOR < 3 ) || ( ITK_VERSION_MAJOR == 3 && ITK_VERSION_MINOR < 13 )
      // Work-around for http://www.itk.org/Bug/view.php?id=8672
      m_Adder->InPlaceOff();
#else
      // Adder can be inplace since the 0th input is a temp field
      m_Adder->InPlaceOn();
#endif
      break;
      }
    default:
      {
      itkExceptionMacro(<< "NumberOfApproximationTerms ("
                        << m_NumberOfApproximationTerms << ") not supported");
      }
    }

  m_Adder->GraftOutput( this->GetOutput() );
  m_Adder->Update();
  this->GraftOutput( m_Adder->GetOutput() );
}


} // end namespace itk


#endif
