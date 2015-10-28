/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkLMMSEVectorImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2008/02/7 14:28:51 $
  Version:   $Revision: 0.0 $
=========================================================================*/
#ifndef __itkLMMSEVectorImageFilter_h
#define __itkLMMSEVectorImageFilter_h

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
class LMMSEVectorImageFilter : public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage                            InputImageType;
  typedef typename InputImageType::Pointer       InputImagePointer;
  typedef typename InputImageType::ConstPointer  InputImageConstPointer;
  typedef TOutputImage                           OutputImageType;
  typedef typename OutputImageType::Pointer      OutputImagePointer;
  typedef typename OutputImageType::ConstPointer OutputImageConstPointer;

  /** Standard class typedefs. */
  typedef LMMSEVectorImageFilter                              Self;
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( LMMSEVectorImageFilter, ImageToImageFilter );

  /** Image typedef support. */
  typedef typename InputImageType::PixelType   InputPixelType;
  typedef typename OutputImageType::PixelType  OutputPixelType;
  typedef typename OutputPixelType::ValueType  ScalarType;
  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;

  /* ##################################################################################################################
    */
  /* ##################################################################################################################
    */
  /* ##################################################################################################################
    */
  /** Set and get the radius of the neighborhood used to compute the statistics. */
  itkSetMacro(               Radius, InputSizeType );
  itkGetConstReferenceMacro( Radius, InputSizeType );
  /* ##################################################################################################################
    */
  /* ##################################################################################################################
    */
  /* ##################################################################################################################
    */
  /** Methods to fix the behaviour against negative values of the estimation of the square of the signal */
  itkGetMacro( UseAbsoluteValue, bool );
  itkSetMacro( UseAbsoluteValue, bool );
  itkBooleanMacro( UseAbsoluteValue );
  itkGetMacro( KeepValue, bool );
  itkSetMacro( KeepValue, bool );
  itkBooleanMacro( KeepValue );
  /** The minimum number of voxels that we allow to compute local statistics and filter: */
  itkGetMacro( MinimumNumberOfUsedVoxelsFiltering, unsigned int );
  itkSetMacro( MinimumNumberOfUsedVoxelsFiltering, unsigned int );
  /* ##################################################################################################################
    */
  /* ##################################################################################################################
    */
  /* ##################################################################################################################
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
  /* ##################################################################################################################
    */
  /* ##################################################################################################################
    */
  /* ##################################################################################################################
    */
protected:
  LMMSEVectorImageFilter();
  virtual ~LMMSEVectorImageFilter()
  {
  }
  void PrintSelf(std::ostream& os, Indent indent) const ITK_OVERRIDE;

  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId ) ITK_OVERRIDE;

  void BeforeThreadedGenerateData( void ) ITK_OVERRIDE;

  virtual void GenerateInputRequestedRegion() 
  throw (InvalidRequestedRegionError) ITK_OVERRIDE;

  bool ComputeInverseMatrix( const double *, const double *, double, double *, unsigned int ) const;

  void CMMInversion( const double *, const double *, double, double *, unsigned int, unsigned int) const;

private:
  // bool ComputeInverseMatrix( const double* measures, const double* squaredAverages, double normal, double* whitened )
  // const;
  LMMSEVectorImageFilter(const Self &); // purposely not implemented
  void operator=(const Self &);         // purposely not implemented

  // The size of the nieghbourhood to compute the statistics:
  InputSizeType m_Radius;
  // What should we do with negative values of the estimated square?
  bool m_UseAbsoluteValue;
  bool m_KeepValue;
  // The minimum number of voxels that we allow to compute local statistics and filter:
  unsigned int m_MinimumNumberOfUsedVoxelsFiltering;
  // The noise variance; this filter itself does not estimate this parameter, so
  // it should be supplied externally:
  float             m_Sigma;
  unsigned int      m_NDWI;
  unsigned int      m_NBaselines;
  IndicatorType     m_DWI;
  IndicatorType     m_Baselines;
  GradientListType  m_GradientList;
  unsigned int      m_Neighbours;
  NeighboursIndType m_NeighboursInd;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLMMSEVectorImageFilter.txx"
#endif

#endif
