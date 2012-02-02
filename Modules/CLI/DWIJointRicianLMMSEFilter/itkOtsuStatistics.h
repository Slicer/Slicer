/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkOtsuStatistics.h,v $
  Language:  C++
  Date:      $Date: 2008/02/7 14:28:51 $
  Version:   $Revision: 0.0 $
=========================================================================*/
#ifndef __itkOtsuStatistics_h
#define __itkOtsuStatistics_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"

namespace itk
{
/** \class OtsuStatistics
 * \brief Applies a LMMSE filtering for Rician noise removal
 */

template <class TInputImage, class TOutputImage>
class ITK_EXPORT OtsuStatistics : public ImageToImageFilter<TInputImage, TOutputImage>
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
  typedef OtsuStatistics                                      Self;
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( OtsuStatistics, ImageToImageFilter );

  /** Image typedef support. */
  typedef typename InputImageType::PixelType  InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;

  typedef itk::Array<unsigned int> IndicatorType;

  /** Set and get the radius of the neighborhood used to compute the statistics. */
  itkSetMacro( Radius, InputSizeType );
  itkGetConstReferenceMacro( Radius, InputSizeType );
  void SetRadius( unsigned int rad )
  {
    m_Radius.Fill( rad );
  }

  /** Set and get the number of DWI channels. */
  itkSetMacro( Channels, unsigned int );
  itkGetConstReferenceMacro( Channels, unsigned int );
  typedef enum OperationMode { USE_AVERAGED_BASELINES, USE_AVERAGED_GRADIENTS, USE_NEIGHBORHOOD_BASELINES,
                               USE_NEIGHBORHOOD_GRADIENTS } OperationMode;
  void SetUseAveragedBaselines(void)
  {
    m_Mode = USE_AVERAGED_BASELINES;
  }
  void SetUseAveragedGradients(void)
  {
    m_Mode = USE_AVERAGED_GRADIENTS;
  }
  void SetUseNeighborhoodBaselines(void)
  {
    m_Mode = USE_NEIGHBORHOOD_BASELINES;
  }
  void SetUseNeighborhoodGradients(void)
  {
    m_Mode = USE_NEIGHBORHOOD_GRADIENTS;
  }

  itkGetMacro( Min, double );
  itkGetMacro( Max, double );
  itkSetMacro( Indicator, IndicatorType );
  itkGetMacro( Indicator, IndicatorType );
protected:
  OtsuStatistics();
  virtual ~OtsuStatistics()
  {
  }
  void PrintSelf(std::ostream& os, Indent indent) const;

  // Threaded filter!
  void BeforeThreadedGenerateData( void );

#if ITK_VERSION_MAJOR < 4
  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, int threadId );

#else
  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId );

#endif
  void AfterThreadedGenerateData( void );

  virtual void GenerateInputRequestedRegion()
  throw (InvalidRequestedRegionError);

private:
  OtsuStatistics(const Self &);  // purposely not implemented
  void operator=(const Self &);  // purposely not implemented

  // The size of the nieghbourhood to compute the statistics:
  InputSizeType m_Radius;
  // The number of DWI-channels to filter; we externaly set this parameter in
  // order to achieve a more general functionality with different vector types
  // than the one for VectorImage class:
  unsigned int m_Channels;
  // Min and max values to compute:
  double m_Min;
  double m_Max;
  // The mode of ooperation:
  OperationMode m_Mode;
  // Per thread max and min values:
  itk::Array<double> m_ThreadMin;
  itk::Array<double> m_ThreadMax;
  // The indicator to compute the baselines:
  IndicatorType m_Indicator;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOtsuStatistics.txx"
#endif

#endif
