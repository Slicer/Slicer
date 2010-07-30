#ifndef __itkSymmetricLogDomainDemonsRegistrationFilter_h
#define __itkSymmetricLogDomainDemonsRegistrationFilter_h

#include "itkLogDomainDeformableRegistrationFilter.h"
#include "itkESMDemonsRegistrationFunction.h"

#include "itkMultiplyByConstantImageFilter.h"

namespace itk
{
/**
  * \class SymmetricLogDomainDemonsRegistrationFilter
  * \brief Deformably register two images using a diffeomorphic demons algorithm
  * and a symmetrized optimization scheme.
  *
  * See T. Vercauteren, X. Pennec, A. Perchant and N. Ayache,
  * "Symmetric Log-Domain Diffeomorphic Registration: A Demons-based Approach",
  * Proc. of MICCAI 2008.
  *
  * Velocity and deformation fields are represented as images whose pixel type
  *are
  * some vector type with at least N elements, where N is the dimension of
  * the fixed image. The vector type must support element access via operator
  * []. It is assumed that the vector elements behave like floating point
  * scalars.
  *
  * This class is templated over the fixed image type, moving image type
  * and the velocity/deformation field type.
  *
  * The input fixed and moving images are set via methods SetFixedImage
  * and SetMovingImage respectively. An initial velocity field maybe set via
  * SetInitialVelocityField or SetInput. If no initial field is set,
  * a zero field is used as the initial condition.
  *
  * The output velocity field can be obtained via methods GetOutput
  * or GetVelocityField.
  *
  * The output deformation field can be obtained via method GetDeformationField.
  *
  * This class make use of the finite difference solver hierarchy. Update
  * for each iteration is computed using a PDEDeformableRegistrationFunction.
  *
  * \warning This filter assumes that the fixed image type, moving image type
  * and velocity field type all have the same number of dimensions.
  *
  * \sa DemonsRegistrationFilter
  * \sa DemonsRegistrationFunction
  * \ingroup DeformableImageRegistration MultiThreaded
  * \author Florence Dru, INRIA and Tom Vercauteren, MKT
  */
template< class TFixedImage, class TMovingImage, class TField >
class ITK_EXPORT SymmetricLogDomainDemonsRegistrationFilter:
  public LogDomainDeformableRegistrationFilter< TFixedImage, TMovingImage, TField >
{
public:
  /** Standard class typedefs. */
  typedef SymmetricLogDomainDemonsRegistrationFilter                                 Self;
  typedef LogDomainDeformableRegistrationFilter< TFixedImage, TMovingImage, TField > Superclass;
  typedef SmartPointer< Self >                                                       Pointer;
  typedef SmartPointer< const Self >                                                 ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods) */
  itkTypeMacro(SymmetricLogDomainDemonsRegistrationFilter, LogDomainDeformableRegistrationFilter);

  /** FixedImage image type. */
  typedef typename Superclass::FixedImageType    FixedImageType;
  typedef typename Superclass::FixedImagePointer FixedImagePointer;

  /** MovingImage image type. */
  typedef typename Superclass::MovingImageType    MovingImageType;
  typedef typename Superclass::MovingImagePointer MovingImagePointer;

  /** Velocity field type. */
  typedef TField                              VelocityFieldType;
  typedef typename VelocityFieldType::Pointer VelocityFieldPointer;

  /** Deformation field type. */
  typedef typename Superclass::DeformationFieldType    DeformationFieldType;
  typedef typename Superclass::DeformationFieldPointer DeformationFieldPointer;

  /** Types inherithed from the superclass */
  typedef typename Superclass::OutputImageType OutputImageType;

  /** FiniteDifferenceFunction type. */
  typedef typename Superclass::FiniteDifferenceFunctionType FiniteDifferenceFunctionType;

  /** Take timestep type from the FiniteDifferenceFunction. */
  typedef typename
  FiniteDifferenceFunctionType::TimeStepType TimeStepType;

  /** DemonsRegistrationFilterFunction type. */
  typedef ESMDemonsRegistrationFunction< FixedImageType,
                                         MovingImageType,
                                         DeformationFieldType >                          DemonsRegistrationFunctionType;
  typedef typename DemonsRegistrationFunctionType::Pointer      DemonsRegistrationFunctionPointer;
  typedef typename DemonsRegistrationFunctionType::GradientType GradientType;

  /** Get the metric value. The metric value is the mean square difference
    * in intensity between the fixed image and transforming moving image
    * computed over the the overlapping region between the two images.
    * This value is calculated for the current iteration */
  virtual double GetMetric() const;

  virtual void SetUseGradientType(GradientType gtype);

  virtual GradientType GetUseGradientType() const;

  /** Set/Get the threshold below which the absolute difference of
    * intensity yields a match. When the intensities match between a
    * moving and fixed image pixel, the update vector (for that
    * iteration) will be the zero vector. Default is 0.001. */
  virtual void SetIntensityDifferenceThreshold(double);

  virtual double GetIntensityDifferenceThreshold() const;

  /** Set/Get the maximum length in terms of pixels of
    *  the vectors in the update buffer. */
  virtual void SetMaximumUpdateStepLength(double);

  virtual double GetMaximumUpdateStepLength() const;

  /** Set/Get the number of terms used in the Baker-Campbell-Hausdorff
    * approximation. */
  itkSetMacro(NumberOfBCHApproximationTerms, unsigned int);
  itkGetConstMacro(NumberOfBCHApproximationTerms, unsigned int);
protected:
  SymmetricLogDomainDemonsRegistrationFilter();
  ~SymmetricLogDomainDemonsRegistrationFilter() {}
  void PrintSelf(std::ostream & os, Indent indent) const;

  /** Initialize the state of filter and equation before each iteration. */
  virtual void InitializeIteration();

  /** This method allocates storage in m_UpdateBuffer.  It is called from
    * FiniteDifferenceFilter::GenerateData(). */
  virtual void AllocateUpdateBuffer();

  /** Method to allow subclasses to get direct access to the backward update
    * buffer */
  virtual VelocityFieldType * GetBackwardUpdateBuffer()
  { return m_BackwardUpdateBuffer; }

  /** This method allocates storage in m_BackwardUpdateBuffer. */
  virtual void AllocateBackwardUpdateBuffer();

  /** Utility to smooth the BackwardUpdateBuffer using a Gaussian operator.
    * The amount of smoothing is specified by the UpdateFieldStandardDeviations.
    */
  virtual void SmoothBackwardUpdateField();

  typedef typename VelocityFieldType::RegionType ThreadRegionType;
  /** Does the actual work of calculating change over a region supplied by
    * the multithreading mechanism. */
  virtual TimeStepType ThreadedCalculateChange(
    const ThreadRegionType & regionToProcess, int threadId);

  /** Apply update. */
  virtual void ApplyUpdate(TimeStepType dt);

  /** This method returns a pointer to a FiniteDifferenceFunction object that
    * will be used by the filter to calculate updates at image pixels.
    * \returns A FiniteDifferenceObject pointer. */
  itkGetConstReferenceObjectMacro(BackwardDifferenceFunction,
                                  FiniteDifferenceFunctionType);

  /** This method sets the pointer to a FiniteDifferenceFunction object that
    * will be used by the filter to calculate updates at image pixels.
    * \returns A FiniteDifferenceObject pointer. */
  itkSetObjectMacro(BackwardDifferenceFunction, FiniteDifferenceFunctionType);
private:
  //purposefully not implemented
  SymmetricLogDomainDemonsRegistrationFilter(const Self &);
  void operator=(const Self &);

  // implemented

  /** Downcast the DifferenceFunction using a dynamic_cast to ensure that it is
    * of the correct type.
    * this method will throw an exception if the function is not of the expected
    *type. */
  DemonsRegistrationFunctionType *  GetForwardRegistrationFunctionType();

  const DemonsRegistrationFunctionType *  GetForwardRegistrationFunctionType() const;

  DemonsRegistrationFunctionType *  GetBackwardRegistrationFunctionType();

  const DemonsRegistrationFunctionType *  GetBackwardRegistrationFunctionType() const;

  /** Exp and composition typedefs */
  typedef MultiplyByConstantImageFilter<
    VelocityFieldType,
    TimeStepType, VelocityFieldType >                   MultiplyByConstantType;

  typedef AddImageFilter<
    VelocityFieldType, VelocityFieldType >                AdderType;

  typedef typename MultiplyByConstantType::Pointer MultiplyByConstantPointer;
  typedef typename AdderType::Pointer              AdderPointer;

  typename FiniteDifferenceFunctionType::Pointer m_BackwardDifferenceFunction;

  MultiplyByConstantPointer m_Multiplier;
  AdderPointer              m_Adder;
  unsigned int              m_NumberOfBCHApproximationTerms;

  /** The buffer that holds the updates for an iteration of the algorithm. */
  VelocityFieldPointer m_BackwardUpdateBuffer;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkSymmetricLogDomainDemonsRegistrationFilter.txx"
#endif

#endif
