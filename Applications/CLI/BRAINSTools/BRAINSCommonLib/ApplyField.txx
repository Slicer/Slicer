#ifndef __ApplyField_txx
#define __ApplyField_txx
#include "ApplyField.h"
#include "itkImage.h"
#include "itkWarpImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkReinitializeLevelSetImageFilter.h"
#include "itkIO.h"

namespace itk
{
template< typename TDeformationField, typename TInputImage,
          typename TOutputImage >
ApplyField< TDeformationField, TInputImage,
            TOutputImage >::ApplyField():
  m_InputImage(0),
  m_OutputImage(0),
  m_DeformationField(0),
  m_DefaultPixelValue(0)
{}

template< typename TDeformationField, typename TInputImage,
          typename TOutputImage >
ApplyField< TDeformationField, TInputImage,
            TOutputImage >::~ApplyField()
{}

template< typename TDeformationField, typename TInputImage,
          typename TOutputImage >
void ApplyField< TDeformationField, TInputImage,
                 TOutputImage >::Execute()
{
  if ( m_InputImage.IsNull() )
    {
    std::cout << "ERROR:  No Input image give.! " << std::endl;
    }

  typedef WarpImageFilter< InputImageType, OutputImageType,
                           TDeformationField > WarperType;
  typename WarperType::Pointer warper = WarperType::New();
  warper->SetInput(m_InputImage);
  warper->SetDeformationField(m_DeformationField);
  warper->SetOutputParametersFromImage(m_DeformationField);
  warper->SetEdgePaddingValue(m_DefaultPixelValue);
  warper->Update();
  std::cout << "  Registration Applied" << std::endl;
  m_OutputImage = warper->GetOutput();
}

template< typename TDeformationField, typename TInputImage,
          typename TOutputImage >
void ApplyField< TDeformationField, TInputImage,
                 TOutputImage >::ReleaseDataFlagOn()
{
  m_InputImage->DisconnectPipeline();
  m_DeformationField->DisconnectPipeline();
}
}
#endif
