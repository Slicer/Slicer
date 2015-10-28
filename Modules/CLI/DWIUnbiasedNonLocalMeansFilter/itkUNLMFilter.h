/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkUNLMFilter.h,v $
  Language:  C++
  Date:      $Date: 2006/03/27 17:01:10 $
  Version:   $Revision: 1.15 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkUNLMFilter_h
#define __itkUNLMFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include <vector>
#include "itkVector.h"
#include "itkArray.h"
#include "itkArray2D.h"
#include "itkFixedArray.h"

namespace itk
{

/* *************************************************************************** */
// We use tis piece of code to sort the gradients with respect to a given one
// in an ascending order with respect to the angular distance
typedef itk::FixedArray<double, 2> OrderType;
// To use with the sort method of std::vector
bool UNLM_gradientDistance_smaller( OrderType e1, OrderType e2 )
{
  return e1[1] < e2[1];
}
/* *************************************************************************** */

/** \class UNLMFilter
 *
 * DO NOT assume a particular image or pixel type, which is, the input image
 * may be a VectorImage as well as an Image obeject with vectorial pixel type.
 *
 * \sa Image
 */
template <class TInputImage, class TOutputImage>
class UNLMFilter : public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef UNLMFilter Self;
  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage                           InputImageType;
  typedef typename InputImageType::Pointer      InputImagePointer;
  typedef typename InputImageType::ConstPointer InputImageConstPointer;
  typedef TOutputImage                          OutputImageType;
  typedef typename OutputImageType::Pointer     OutputImagePointer;

  /** Standard class typedefs. */
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( UNLMFilter, ImageToImageFilter );

  /** Image typedef support. */
  typedef typename InputImageType::PixelType   InputPixelType;
  typedef typename OutputImageType::PixelType  OutputPixelType;
  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename InputImageType::SizeType    InputImageSizeType;
  typedef typename InputImageType::IndexType   InputImageIndexType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename OutputPixelType::ValueType  ScalarType;

  /** Typedefs for matrix computations; we have a variable number of gradients,
    * so we do need growing arrays; but gradients are always represented by
    * means of 3-component vectors; in general, we use fixed-size arrays where
    * possible, and dynamic size arrays only if needed
   */
  // Type of the gradients direction:
  typedef itk::Vector<double, 3> GradientType;
  // Type to store all the gradient directions
  typedef std::vector<GradientType> GradientListType;
  // Indicator type:
  typedef itk::Array<unsigned int> IndicatorType;
  // The type to store the closest gradient directions to a given one.
  typedef itk::Array2D<unsigned int> NeighboursIndType;

  /** Set and get the parameters */
  itkSetMacro( NDWI,       unsigned int );
  itkGetMacro( NDWI,       unsigned int );
  itkSetMacro( NBaselines, unsigned int );
  itkGetMacro( NBaselines, unsigned int );
  itkSetMacro( Sigma,      float        );
  itkGetMacro( Sigma,      float        );
  itkSetMacro( Neighbours, unsigned int );
  itkGetMacro( Neighbours, unsigned int );
  itkSetMacro( H,          float        );
  itkGetMacro( H,          float        );
  itkSetMacro( RSearch,    InputImageSizeType );
  itkGetMacro( RSearch,    InputImageSizeType );
  itkSetMacro( RComp,      InputImageSizeType );
  itkGetMacro( RComp,      InputImageSizeType );

  /** Add a new gradient direction: */
  void AddGradientDirection( GradientType grad )
  {
    m_GradientList.push_back( grad );
    return;
  }

  /** Set the vector with the DWI channels that are going to be used: */
  void SetDWI( IndicatorType ind )
  {
    m_DWI = ind;
  }

  void SetBaselines( IndicatorType ind )
  {
    m_Baselines = ind;
  }

  /** Set the vector of DWI channels using c-style vector. The user must set
   *  m_Channels before */
  void SetDWI( unsigned int* ind )
  {
    m_DWI.SetSize( m_NDWI );
    for( unsigned int k = 0; k < m_NDWI; ++k )
      {
      m_DWI[k] = ind[k];
      }
  }

  void SetBaselines( unsigned int* ind )
  {
    m_Baselines.SetSize( m_NBaselines );
    for( unsigned int k = 0; k < m_NBaselines; ++k )
      {
      m_Baselines[k] = ind[k];
      }
  }

  IndicatorType GetDWI(void)
  {
    return m_DWI;
  }
  IndicatorType GetBaselines(void)
  {
    return m_Baselines;
  }
protected:
  UNLMFilter();
  virtual ~UNLMFilter()
  {
  }
  // Threaded filter!
  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId ) ITK_OVERRIDE;

  void BeforeThreadedGenerateData() ITK_OVERRIDE;

  void GenerateInputRequestedRegion() ITK_OVERRIDE;

private:
  UNLMFilter(const Self &);        // purposely not implemented
  void operator=(const Self &);    // purposely not implemented

  // The list of gradient directions:
  GradientListType m_GradientList;
  // The number of DWI and baselines to use:
  unsigned int m_NDWI;
  unsigned int m_NBaselines;
  // Which channels correspond either to DWI or baselines:
  IndicatorType m_DWI;
  IndicatorType m_Baselines;
  // The standard deviation of noise (in the complex domain)
  float m_Sigma;
  // The number of neighbouring gradients to use:
  unsigned int m_Neighbours;
  // The neighbouring gradients to use:
  NeighboursIndType m_NeighboursInd;
  // The true parameteres of NLM:
  float              m_H;
  InputImageSizeType m_RSearch;
  InputImageSizeType m_RComp;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkUNLMFilter.txx"
#endif

#endif
