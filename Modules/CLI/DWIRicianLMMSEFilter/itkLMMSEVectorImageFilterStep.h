/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkLMMSEVectorImageFilterStep.h,v $
  Language:  C++
  Date:      $Date: 2008/02/7 14:28:51 $
  Version:   $Revision: 0.0 $
=========================================================================*/
#ifndef __itkLMMSEVectorImageFilterStep_h
#define __itkLMMSEVectorImageFilterStep_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"

namespace itk
{
/** \class LMMSEVectorImageFilterStep
 * \brief Applies a LMMSE filtering for Rician noise removal
 */

template <class TInputImage, class TOutputImage>
class LMMSEVectorImageFilterStep : public ImageToImageFilter<TInputImage, TOutputImage>
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
  typedef LMMSEVectorImageFilterStep                          Self;
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( LMMSEVectorImageFilterStep, ImageToImageFilter );

  /** Image typedef support. */
  typedef typename InputImageType::PixelType  InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;

  /** Set and get the radius of the neighborhood used to compute the statistics. */
  itkSetMacro( Radius, InputSizeType );
  itkGetConstReferenceMacro( Radius, InputSizeType );
  /** Set and get the number of DWI channels. */
  itkSetMacro( Channels, unsigned int );
  itkGetConstReferenceMacro( Channels, unsigned int );

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

  /** Set and get the externally estimated noise variance */
  itkGetMacro( NoiseVariance, double );
  itkSetMacro( NoiseVariance, double );

  /** It is necessary to override GenerateInputRequestedRegion(), since we need a larger
   region of the input than is the output */
  virtual void GenerateInputRequestedRegion()
  throw (InvalidRequestedRegionError) ITK_OVERRIDE;

protected:
  LMMSEVectorImageFilterStep();
  virtual ~LMMSEVectorImageFilterStep()
  {
  }
  void PrintSelf(std::ostream& os, Indent indent) const ITK_OVERRIDE;

  // Threaded filter!
  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId ) ITK_OVERRIDE;

private:
  LMMSEVectorImageFilterStep(const Self &); // purposely not implemented
  void operator=(const Self &);             // purposely not implemented

  // The size of the nieghbourhood to compute the statistics:
  InputSizeType m_Radius;
  // The number of DWI-channels to filter; we externaly set this parameter in
  // order to achieve a more general functionality with different vector types
  // than the one for VectorImage class:
  unsigned int m_Channels;
  // What should we do with negative values of the estimated square?
  bool m_UseAbsoluteValue;
  bool m_KeepValue;
  // The minimum number of voxels that we allow to compute local statistics and filter:
  unsigned int m_MinimumNumberOfUsedVoxelsFiltering;
  // The noise variance; this filter itself does not estimate this parameter, so
  // it should be supplied externally:
  double m_NoiseVariance;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLMMSEVectorImageFilterStep.txx"
#endif

#endif
