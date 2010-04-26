#ifndef __itkLogDomainDemonsRegistrationFilter_h
#define __itkLogDomainDemonsRegistrationFilter_h

#include "itkLogDomainDeformableRegistrationFilter.h"
#include "itkESMDemonsRegistrationFunction.h"

#include "itkMultiplyByConstantImageFilter.h"
#include "itkVelocityFieldBCHCompositionFilter.h"


namespace itk {

/**
 * \class LogDomainDemonsRegistrationFilter
 * \brief Deformably register two images using a diffeomorphic demons algorithm.
 * 
 * See T. Vercauteren, X. Pennec, A. Perchant and N. Ayache,
 * "Symmetric Log-Domain Diffeomorphic Registration: A Demons-based Approach",
 * Proc. of MICCAI 2008.
 *
 * Velocity and deformation fields are represented as images whose pixel type are
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
template<class TFixedImage, class TMovingImage, class TField>
class ITK_EXPORT LogDomainDemonsRegistrationFilter : 
   public LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
{
public:
  /** Standard class typedefs. */
  typedef LogDomainDemonsRegistrationFilter             Self;
  typedef LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>   Superclass;
  typedef SmartPointer<Self>                            Pointer;
  typedef SmartPointer<const Self>                      ConstPointer;
 
  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods) */
  itkTypeMacro( LogDomainDemonsRegistrationFilter,LogDomainDeformableRegistrationFilter );

  /** FixedImage image type. */
  typedef typename Superclass::FixedImageType           FixedImageType;
  typedef typename Superclass::FixedImagePointer        FixedImagePointer;

  /** MovingImage image type. */
  typedef typename Superclass::MovingImageType          MovingImageType;
  typedef typename Superclass::MovingImagePointer       MovingImagePointer;
  
  /** Velocity field type. */
  typedef TField                                        VelocityFieldType;
  typedef typename VelocityFieldType::Pointer           VelocityFieldPointer;
  
  /** Deformation field type. */
  typedef typename Superclass::DeformationFieldType     DeformationFieldType;
  typedef typename Superclass::DeformationFieldPointer  DeformationFieldPointer;

  /** Types inherithed from the superclass */
  typedef typename Superclass::OutputImageType          OutputImageType;

  /** FiniteDifferenceFunction type. */
  typedef typename Superclass::FiniteDifferenceFunctionType  FiniteDifferenceFunctionType;

  /** Take timestep type from the FiniteDifferenceFunction. */
  typedef typename 
    FiniteDifferenceFunctionType::TimeStepType          TimeStepType;
 
  /** DemonsRegistrationFilterFunction type. */
  typedef ESMDemonsRegistrationFunction<FixedImageType,
    MovingImageType, DeformationFieldType>                          DemonsRegistrationFunctionType;
  typedef typename DemonsRegistrationFunctionType::Pointer          DemonsRegistrationFunctionPointer;
  typedef typename DemonsRegistrationFunctionType::GradientType     GradientType;

  /** Get the metric value. The metric value is the mean square difference 
   * in intensity between the fixed image and transforming moving image 
   * computed over the the overlapping region between the two images. 
   * This value is calculated for the current iteration */
  virtual double GetMetric() const;
  virtual const double &GetRMSChange() const;

  virtual void SetUseGradientType( GradientType gtype );
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
 
  /** Set/Get the number of terms used in the Baker-Campbell-Hausdorff approximation. */
  virtual void SetNumberOfBCHApproximationTerms(unsigned int);
  virtual unsigned int GetNumberOfBCHApproximationTerms() const;

protected:
  LogDomainDemonsRegistrationFilter();
  ~LogDomainDemonsRegistrationFilter() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** Initialize the state of filter and equation before each iteration. */
  virtual void InitializeIteration();

  /** Apply update. */
  virtual void ApplyUpdate(TimeStepType dt);


private:
  LogDomainDemonsRegistrationFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  /** Downcast the DifferenceFunction using a dynamic_cast to ensure that it is of the correct type.
   * this method will throw an exception if the function is not of the expected type. */
  DemonsRegistrationFunctionType *  DownCastDifferenceFunctionType();
  const DemonsRegistrationFunctionType *  DownCastDifferenceFunctionType() const;

  /** Exp and composition typedefs */
  typedef MultiplyByConstantImageFilter<
    VelocityFieldType, 
    TimeStepType, VelocityFieldType >                   MultiplyByConstantType;

  typedef VelocityFieldBCHCompositionFilter<
     VelocityFieldType,
    VelocityFieldType>                                 BCHFilterType;

  typedef typename MultiplyByConstantType::Pointer      MultiplyByConstantPointer;
  typedef typename BCHFilterType::Pointer               BCHFilterPointer;


  MultiplyByConstantPointer                             m_Multiplier;
  BCHFilterPointer                                      m_BCHFilter;

};


} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLogDomainDemonsRegistrationFilter.txx"
#endif

#endif
