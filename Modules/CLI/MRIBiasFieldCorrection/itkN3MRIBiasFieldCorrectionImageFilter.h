/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkN3MRIBiasFieldCorrectionImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009/06/09 16:22:05 $
  Version:   $Revision: 1.6 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkN3MRIBiasFieldCorrectionImageFilter_h
#define __itkN3MRIBiasFieldCorrectionImageFilter_h

#include "itkImageToImageFilter.h"

#include "itkBSplineScatteredDataPointSetToImageFilter.h"
#include "itkPointSet.h"
#include "itkSingleValuedCostFunction.h"
#include "itkVector.h"

#include "vnl/vnl_vector.h"

namespace itk
{

/** \class N3MRIBiasFieldCorrectionImageFilter
 * \brief Implementation of the N3 MRI bias field correction algorithm.
 *
 * The nonparametric nonuniform intensity normalization (N3) algorithm
 * is a method for correcting nonuniformity associated with MR images.
 * The algorithm assumes a simple parametric model (Gaussian) for the bias field
 * but does not require tissue class segmentation.  In addition, there are
 * only a couple of parameters to tune with the default values performing
 * quite well.
 *
 * N3 has been publicly available as a set of perl scripts
 * (http://www.bic.mni.mcgill.ca/software/N3/) but, with this class, has been
 * reimplemented for the ITK library with only one minor variation involving
 * the b-spline fitting routine.  We replaced the original fitting approach
 * with the itkBSplineScatteredDataPointSetToImageFilter which is not
 * susceptible to ill-conditioned matrix calculation as is the original proposed
 * fitting component.
 *
 * Notes for the user:
 *  0. Based on our experience with the filter, sometimes the scale of the bias
 *     field is too small particularly for large mesh element sizes.  Therefore,
 *     we added an option (which is true by default) for finding the optimal
 *     scaling of the bias field which is based on minimizing the coefficient
 *     of variation of the corrected image.  This cost function is given below
 *     in N3BiasFieldScaleCostFunction.
 *  1. Since much of the image manipulation is done in the log space of the
 *     intensities, input images with negative and small values (< 1) are
 *     discouraged.
 *  2. The original authors recommend performing the bias field correction
 *      on a downsampled version of the original image.
 *  3. A mask and/or confidence image can be supplied.
 *  4. The filter returns the corrected image.  If the bias field is wanted, one
 *     can reconstruct it using the class itkBSplineControlPointImageFilter.
 *     See the IJ article and the test file for an example.
 *  5. The 'Z' parameter in Sled's 1998 paper is the square root
 *     of the class variable 'm_WeinerFilterNoise'.
 *
 * \author Nicholas J. Tustison
 *
 * Contributed by Nicholas J. Tustison, James C. Gee
 * in the Insight Journal paper:
 *
 * \par REFERENCE
 * J.G. Sled, P. Zijdenbos and A.C. Evans, "A comparison of retrospective
 * intensity non-uniformity correction methods for MRI".  Information
 * Processing Medical Imaging Proc, 15th Int. Conf. IMPI'97, vol.1230,
 * pp 459-464,1997.
 *
 * J.G. Sled, A.P. Zijdenbos and A.C. Evans.  "A Nonparametric Method for
 * Automatic Correction of Intensity Nonuniformity in MRI Data"
 * IEEE Transactions on Medical Imaging, Vol 17, No 1. Feb 1998.
 *
 */

/**
 * Class definition for N3BiasFieldScaleCostFunction
 */

template <class TInputImage, class TBiasFieldImage, class TMaskImage,
          class TConfidenceImage>
class N3BiasFieldScaleCostFunction
  : public SingleValuedCostFunction
{
public:
  typedef N3BiasFieldScaleCostFunction Self;
  typedef SingleValuedCostFunction     Superclass;
  typedef SmartPointer<Self>           Pointer;
  typedef SmartPointer<const Self>     ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro( N3BiasFieldScaleCostFunction, SingleValuedCostFunction );

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  typedef Superclass::MeasureType    MeasureType;
  typedef Superclass::DerivativeType DerivativeType;
  typedef Superclass::ParametersType ParametersType;

  itkSetObjectMacro( InputImage, TInputImage );
  itkSetObjectMacro( BiasFieldImage, TBiasFieldImage );
  itkSetObjectMacro( MaskImage, TMaskImage );
  itkSetObjectMacro( ConfidenceImage, TConfidenceImage );

  itkSetMacro( MaskLabel, typename TMaskImage::PixelType );
  itkGetConstMacro( MaskLabel, typename TMaskImage::PixelType );

  virtual MeasureType GetValue( const ParametersType & parameters ) const;

  virtual void GetDerivative( const ParametersType & parameters, DerivativeType & derivative ) const;

  virtual unsigned int GetNumberOfParameters() const;

protected:
  N3BiasFieldScaleCostFunction();
  virtual ~N3BiasFieldScaleCostFunction();
private:
  N3BiasFieldScaleCostFunction(const Self &); // purposely not implemented
  void operator=(const Self &);               // purposely not implemented

  typename TInputImage::Pointer                  m_InputImage;
  typename TBiasFieldImage::Pointer              m_BiasFieldImage;
  typename TMaskImage::Pointer                   m_MaskImage;
  typename TConfidenceImage::Pointer             m_ConfidenceImage;

  typename TMaskImage::PixelType                 m_MaskLabel;
};

/**
 * Class definition for N3MRIBiasFieldCorrectionImageFilter
 */
template <class TInputImage, class TMaskImage = Image<unsigned char,
                                                      ::itk::GetImageDimension<TInputImage>::ImageDimension>,
          class TOutputImage = TInputImage>
class N3MRIBiasFieldCorrectionImageFilter :
  public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef N3MRIBiasFieldCorrectionImageFilter           Self;
  typedef ImageToImageFilter<TInputImage, TOutputImage> Superclass;
  typedef SmartPointer<Self>                            Pointer;
  typedef SmartPointer<const Self>                      ConstPointer;

  /** Runtime information support. */
  itkTypeMacro( N3MRIBiasFieldCorrectionImageFilter, ImageToImageFilter );

  /** Standard New method. */
  itkNewMacro( Self );

  /** ImageDimension constants */
  itkStaticConstMacro( ImageDimension, unsigned int,
                       TInputImage::ImageDimension );

  /** Some convenient typedefs. */
  typedef TInputImage                       InputImageType;
  typedef TOutputImage                      OutputImageType;
  typedef TMaskImage                        MaskImageType;
  typedef typename MaskImageType::PixelType MaskPixelType;

  typedef float                           RealType;
  typedef Image<RealType, ImageDimension> RealImageType;

  /** B-spline smoothing filter typedefs */
  typedef Vector<RealType, 1> ScalarType;
  typedef PointSet<ScalarType,
                   itkGetStaticConstMacro( ImageDimension )>        PointSetType;
  typedef Image<ScalarType,
                itkGetStaticConstMacro( ImageDimension )>        ScalarImageType;
  typedef BSplineScatteredDataPointSetToImageFilter
  <PointSetType, ScalarImageType>                  BSplineFilterType;
  typedef typename
  BSplineFilterType::PointDataImageType            BiasFieldControlPointLatticeType;
  typedef typename BSplineFilterType::ArrayType ArrayType;

  void SetMaskImage( const MaskImageType *mask )
  {
    this->SetNthInput( 1, const_cast<MaskImageType *>( mask ) );
  }

  const MaskImageType * GetMaskImage() const
  {
    return static_cast<MaskImageType *>( const_cast<DataObject *>
                                         ( this->ProcessObject::GetInput( 1 ) ) );
  }

  void SetConfidenceImage( const RealImageType *image )
  {
    this->SetNthInput( 2, const_cast<RealImageType *>( image ) );
  }

  const RealImageType * GetConfidenceImage() const
  {
    return static_cast<RealImageType *>( const_cast<DataObject *>
                                         ( this->ProcessObject::GetInput( 2 ) ) );
  }

  void SetInput1( const TInputImage *input )
  {
    this->SetInput( input );
  }

  void SetInput2( const TMaskImage *mask )
  {
    this->SetMaskImage( mask );
  }

  void SetInput3( const RealImageType *image )
  {
    this->SetConfidenceImage( image );
  }

  itkSetMacro( MaskLabel, MaskPixelType );
  itkGetConstMacro( MaskLabel, MaskPixelType );

  itkSetMacro( NumberOfHistogramBins, unsigned int );
  itkGetConstMacro( NumberOfHistogramBins, unsigned int );

  itkSetMacro( WeinerFilterNoise, RealType );
  itkGetConstMacro( WeinerFilterNoise, RealType );

  itkSetMacro( BiasFieldFullWidthAtHalfMaximum, RealType );
  itkGetConstMacro( BiasFieldFullWidthAtHalfMaximum, RealType );

  itkSetMacro( MaximumNumberOfIterations, unsigned int );
  itkGetConstMacro( MaximumNumberOfIterations, unsigned int );

  itkSetMacro( ConvergenceThreshold, RealType );
  itkGetConstMacro( ConvergenceThreshold, RealType );

  itkSetMacro( SplineOrder, unsigned int );
  itkGetConstMacro( SplineOrder, unsigned int );

  itkSetMacro( NumberOfFittingLevels, ArrayType );
  itkGetConstMacro( NumberOfFittingLevels, ArrayType );
  void SetNumberOfFittingLevels( unsigned int n )
  {
    ArrayType nlevels;

    nlevels.Fill( n );
    this->SetNumberOfFittingLevels( nlevels );
  }

  itkSetMacro( NumberOfControlPoints, ArrayType );
  itkGetConstMacro( NumberOfControlPoints, ArrayType );

  itkGetConstMacro( LogBiasFieldControlPointLattice,
                    typename BiasFieldControlPointLatticeType::Pointer );

  itkSetMacro( UseOptimalBiasFieldScaling, bool );
  itkGetConstMacro( UseOptimalBiasFieldScaling, bool );
  itkBooleanMacro( UseOptimalBiasFieldScaling );

  itkGetConstMacro( BiasFieldScaling, RealType );

  itkGetConstMacro( ElapsedIterations, unsigned int );
  itkGetConstMacro( CurrentConvergenceMeasurement, RealType );
protected:
  N3MRIBiasFieldCorrectionImageFilter();
  ~N3MRIBiasFieldCorrectionImageFilter()
  {
  };
  void PrintSelf( std::ostream& os, Indent indent ) const;

  void GenerateData();

private:
  N3MRIBiasFieldCorrectionImageFilter( const Self & ); // purposely not implemented
  void operator=( const Self & );                      // purposely not implemented

  typename RealImageType::Pointer SharpenImage(
    typename RealImageType::Pointer );
  typename RealImageType::Pointer SmoothField(
    typename RealImageType::Pointer );
  RealType CalculateConvergenceMeasurement(
    typename RealImageType::Pointer,
    typename RealImageType::Pointer );
  RealType CalculateOptimalBiasFieldScaling(
    typename RealImageType::Pointer );

  MaskPixelType m_MaskLabel;

  /**
   * Parameters for deconvolution with Weiner filter
   */
  unsigned int m_NumberOfHistogramBins;
  RealType     m_WeinerFilterNoise;
  RealType     m_BiasFieldFullWidthAtHalfMaximum;

  /**
   * Convergence parameters
   */
  unsigned int m_MaximumNumberOfIterations;
  unsigned int m_ElapsedIterations;
  RealType     m_ConvergenceThreshold;
  RealType     m_CurrentConvergenceMeasurement;

  /**
   * B-spline fitting parameters
   */
  typename
  BiasFieldControlPointLatticeType::Pointer m_LogBiasFieldControlPointLattice;
  unsigned int m_SplineOrder;
  ArrayType    m_NumberOfControlPoints;
  ArrayType    m_NumberOfFittingLevels;

  /**
   * other parameters
   */
  RealType m_BiasFieldScaling;
  bool     m_UseOptimalBiasFieldScaling;

}; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkN3MRIBiasFieldCorrectionImageFilter.txx"
#endif

#endif
