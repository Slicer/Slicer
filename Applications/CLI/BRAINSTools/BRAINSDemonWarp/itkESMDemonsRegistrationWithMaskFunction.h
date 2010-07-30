/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile: itkESMDemonsRegistrationWithMaskFunction.h,v $
 *  Language:  C++
 *  Date:      $Date: 2008-11-12 13:59:47 $
 *  Version:   $Revision: 1.6 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/

#ifndef __itkESMDemonsRegistrationWithMaskFunction_h
#define __itkESMDemonsRegistrationWithMaskFunction_h

#include "itkPDEDeformableRegistrationFunction.h"
#include "itkPoint.h"
#include "itkCovariantVector.h"
#include "itkInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkCentralDifferenceImageFunction.h"
#include "itkWarpImageFilter.h"
#include "itkSpatialObject.h"

namespace itk
{
/**
  * \class ESMDemonsRegistrationWithMaskFunction
  *
  * \brief Fast implementation of the symmetric demons registration force
  *
  * This class provides a substantially faster implementation of the
  * symmetric demons registration force. Speed is improved by keeping
  * a deformed copy of the moving image for gradient evaluation.
  *
  * Symmetric forces simply means using the mean of the gradient
  * of the fixed image and the gradient of the warped moving
  * image.
  *
  * Note that this class also enables the use of fixed, mapped moving
  * and warped moving images forces by using a call to SetUseGradientType
  *
  * The moving image should not be saturated. We indeed use
  * NumericTraits<MovingPixelType>::Max() as a special value.
  *
  * \author Tom Vercauteren, INRIA & Mauna Kea Technologies
  *
  * This implementation was taken from the Insight Journal paper:
  * http://hdl.handle.net/1926/510
  *
  * \sa SymmetricForcesDemonsRegistrationFunction
  * \sa SymmetricForcesDemonsRegistrationFilter
  * \sa DemonsRegistrationFilter
  * \sa DemonsRegistrationFunction
  * \ingroup FiniteDifferenceFunctions
  *
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
class ITK_EXPORT ESMDemonsRegistrationWithMaskFunction:
  public PDEDeformableRegistrationFunction< TFixedImage,
                                            TMovingImage, TDeformationField >
{
public:
  /** Standard class typedefs. */
  typedef ESMDemonsRegistrationWithMaskFunction Self;
  typedef PDEDeformableRegistrationFunction<
    TFixedImage, TMovingImage, TDeformationField >    Superclass;

  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ESMDemonsRegistrationWithMaskFunction,
               PDEDeformableRegistrationFunction);

  /** MovingImage image type. */
  typedef typename Superclass::MovingImageType    MovingImageType;
  typedef typename Superclass::MovingImagePointer MovingImagePointer;
  typedef typename MovingImageType::PixelType     MovingPixelType;

  /** FixedImage image type. */
  typedef typename Superclass::FixedImageType    FixedImageType;
  typedef typename Superclass::FixedImagePointer FixedImagePointer;
  typedef typename FixedImageType::PixelType     FixedPixelType;
  typedef typename FixedImageType::IndexType     IndexType;
  typedef typename FixedImageType::SizeType      SizeType;
  typedef typename FixedImageType::SpacingType   SpacingType;
  typedef typename FixedImageType::DirectionType DirectionType;

  /** Deformation field type. */
  typedef typename Superclass::DeformationFieldType DeformationFieldType;
  typedef typename Superclass::DeformationFieldTypePointer
  DeformationFieldTypePointer;

  /** Inherit some enums from the superclass. */
  itkStaticConstMacro(ImageDimension, unsigned int, Superclass::ImageDimension);

  typedef SpatialObject< itkGetStaticConstMacro(ImageDimension) > MaskType;
  typedef typename MaskType::Pointer                              MaskPointer;

  /** Inherit some enums from the superclass. */
  typedef typename Superclass::PixelType        PixelType;
  typedef typename Superclass::RadiusType       RadiusType;
  typedef typename Superclass::NeighborhoodType NeighborhoodType;
  typedef typename Superclass::FloatOffsetType  FloatOffsetType;
  typedef typename Superclass::TimeStepType     TimeStepType;

  /** Interpolator type. */
  typedef double CoordRepType;
  typedef InterpolateImageFunction<
    MovingImageType, CoordRepType >                   InterpolatorType;
  typedef typename InterpolatorType::Pointer   InterpolatorPointer;
  typedef typename InterpolatorType::PointType PointType;
  typedef LinearInterpolateImageFunction<
    MovingImageType, CoordRepType >                   DefaultInterpolatorType;
  typedef Point< CoordRepType,
                 itkGetStaticConstMacro(ImageDimension) > MovingImagePointType;

  /** Warper type */
  typedef WarpImageFilter<
    MovingImageType,
    MovingImageType, DeformationFieldType >           WarperType;

  typedef typename WarperType::Pointer WarperPointer;

  /** Covariant vector type. */
  typedef CovariantVector< double,
                           itkGetStaticConstMacro(ImageDimension) >
  CovariantVectorType;

  /** Fixed image gradient calculator type. */
  typedef CentralDifferenceImageFunction< FixedImageType > GradientCalculatorType;
  typedef typename GradientCalculatorType::Pointer
  GradientCalculatorPointer;

  /** Moving image gradient (unwarped) calculator type. */
  typedef CentralDifferenceImageFunction< MovingImageType, CoordRepType >
  MovingImageGradientCalculatorType;
  typedef typename MovingImageGradientCalculatorType::Pointer
  MovingImageGradientCalculatorPointer;

  /** Set the moving image interpolator. */
  void SetMovingImageInterpolator(InterpolatorType *ptr)
  {
    m_MovingImageInterpolator = ptr; m_MovingImageWarper->SetInterpolator(ptr);
  }

  /** Get the moving image interpolator. */
  InterpolatorType * GetMovingImageInterpolator(void)
  {
    return m_MovingImageInterpolator;
  }

  /** This class uses a constant timestep of 1. */
  virtual TimeStepType ComputeGlobalTimeStep( void *itkNotUsed(GlobalData) )
  const
  {
    return m_TimeStep;
  }

  /** Return a pointer to a global data structure that is passed to
    * this object from the solver at each calculation.  */
  virtual void * GetGlobalDataPointer() const
  {
    GlobalDataStruct *global = new GlobalDataStruct();

    global->m_SumOfSquaredDifference  = 0.0;
    global->m_NumberOfPixelsProcessed = 0L;
    global->m_SumOfSquaredChange      = 0;
    return global;
  }

  /** Release memory for global data structure. */
  virtual void ReleaseGlobalDataPointer(void *GlobalData) const;

  /** Set the object's state before each iteration. */
  virtual void InitializeIteration();

  /** This method is called by a finite difference solver image filter at
    * each pixel that does not lie on a data set boundary */
  virtual PixelType  ComputeUpdate(
    const NeighborhoodType & neighborhood,
    void *globalData,
    const FloatOffsetType & offset =
      FloatOffsetType(0.0) );

  /** Get the metric value. The metric value is the mean square difference
    * in intensity between the fixed image and transforming moving image
    * computed over the the overlapping region between the two images. */
  virtual double GetMetric() const
  {
    return m_Metric;
  }

  /** Get the rms change in deformation field. */
  virtual const double & GetRMSChange() const
  {
    return m_RMSChange;
  }

  /** Set/Get the threshold below which the absolute difference of
    * intensity yields a match. When the intensities match between a
    * moving and fixed image pixel, the update vector (for that
    * iteration) will be the zero vector. Default is 0.001. */
  virtual void SetIntensityDifferenceThreshold(double);

  virtual double GetIntensityDifferenceThreshold() const;

  /** Set/Get the maximum update step length. In Thirion this is 0.5.
    *  Setting it to 0 implies no restriction (beware of numerical
    *  instability in this case. */
  virtual void SetMaximumUpdateStepLength(double sm)
  {
    this->m_MaximumUpdateStepLength = sm;
  }

  virtual double GetMaximumUpdateStepLength() const
  {
    return this->m_MaximumUpdateStepLength;
  }

  /** Type of available image forces */
  enum GradientType {
    Symmetric = 0,
    Fixed = 1,
    WarpedMoving = 2,
    MappedMoving = 3
    };

  /** Set/Get the type of used image forces */
  virtual void SetUseGradientType(GradientType gtype)
  {
    m_UseGradientType = gtype;
  }

  virtual GradientType GetUseGradientType() const
  {
    return m_UseGradientType;
  }

  virtual void SetMovingImageMask(MaskType *mask)
  {
    m_MovingMask = mask;
  }

  virtual const MaskType * GetMovingImageMask(void) const
  {
    return m_MovingMask;
  }

  virtual void SetFixedImageMask(MaskType *mask)
  {
    m_FixedMask = mask;
  }

  virtual const MaskType * GetFixedImageMask(void) const
  {
    return m_FixedMask;
  }

protected:
  ESMDemonsRegistrationWithMaskFunction();
  ~ESMDemonsRegistrationWithMaskFunction() {}
  void PrintSelf(std::ostream & os, Indent indent) const;

  /** FixedImage image neighborhood iterator type. */
  typedef ConstNeighborhoodIterator< FixedImageType >
  FixedImageNeighborhoodIteratorType;

  /** A global data type for this class of equation. Used to store
    * iterators for the fixed image. */
  struct GlobalDataStruct {
    double m_SumOfSquaredDifference;
    unsigned long m_NumberOfPixelsProcessed;
    double m_SumOfSquaredChange;
  };
private:
  // purposely notimplemented
  ESMDemonsRegistrationWithMaskFunction(const Self &);
  void operator=(const Self &);

  /** Cache fixed image information. */
  PointType     m_FixedImageOrigin;
  SpacingType   m_FixedImageSpacing;
  DirectionType m_FixedImageDirection;
  double        m_Normalizer;

  /** Function to compute derivatives of the fixed image. */
  GradientCalculatorPointer m_FixedImageGradientCalculator;

  /** Function to compute derivatives of the moving image (unwarped). */
  MovingImageGradientCalculatorPointer m_MappedMovingImageGradientCalculator;

  GradientType m_UseGradientType;

  /** Function to interpolate the moving image. */
  InterpolatorPointer m_MovingImageInterpolator;

  /** Filter to warp moving image for fast gradient computation. */
  WarperPointer m_MovingImageWarper;

  /** The global timestep. */
  TimeStepType m_TimeStep;

  /** Threshold below which the denominator term is considered zero. */
  double m_DenominatorThreshold;

  /** Threshold below which two intensity value are assumed to match. */
  double m_IntensityDifferenceThreshold;

  /** Maximum update step length in pixels (default is 0.5 as in Thirion). */
  double m_MaximumUpdateStepLength;

  /** The metric value is the mean square difference in intensity between
    * the fixed image and transforming moving image computed over the
    * the overlapping region between the two images. */
  mutable double        m_Metric;
  mutable double        m_SumOfSquaredDifference;
  mutable unsigned long m_NumberOfPixelsProcessed;
  mutable double        m_RMSChange;
  mutable double        m_SumOfSquaredChange;

  /** Mutex lock to protect modification to metric. */
  mutable SimpleFastMutexLock m_MetricCalculationLock;
  MaskPointer                 m_MovingMask;
  MaskPointer                 m_FixedMask;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkESMDemonsRegistrationWithMaskFunction.txx"
#endif

#endif
