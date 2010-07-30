#ifndef __itkMultiResolutionLogDomainDeformableRegistration_h
#define __itkMultiResolutionLogDomainDeformableRegistration_h

#include "itkExponentialDeformationFieldImageFilter2.h"
#include "itkImage.h"
#include "itkImageToImageFilter.h"
#include "itkLogDomainDeformableRegistrationFilter.h"
#include "itkLogDomainDemonsRegistrationFilter.h"
#include "itkMultiResolutionPyramidImageFilter.h"
#include "itkVectorResampleImageFilter.h"

#include <vector>

namespace itk
{
/**
  * \class MultiResolutionLogDomainDeformableRegistration
  * \brief Framework for performing multi-resolution log-domain
  * deformable registration.
  *
  * MultiResolutionLogDomainDeformableRegistration provides a generic framework
  * to peform multi-resolution deformable registration.
  *
  * At each resolution level a LogDomainDeformableRegistrationFilter is used
  * to register two images by computing the velocity field whose exponential
  *will
  * map a moving image onto a fixed image.
  *
  * See T. Vercauteren, X. Pennec, A. Perchant and N. Ayache,
  * "Symmetric Log-Domain Diffeomorphic Registration: A Demons-based Approach",
  * Proc. of MICCAI 2008.
  *
  * Velocity and deformation fields are represented as images whose pixel type
  *is some
  * vector type with at least N elements, where N is the dimension of
  * the fixed image. The vector type must support element access via operator
  * []. It is assumed that the vector elements behave like floating point
  * scalars.
  *
  * The internal LogDomainDeformableRegistrationFilter can be set using
  * SetRegistrationFilter. By default a LogDomainDemonsRegistrationFilter is
  *used.
  *
  * The input fixed and moving images are set via methods SetFixedImage
  * and SetMovingImage respectively. An initial velocity field maybe set via
  * SetInitialVelocityField if is matches the characteristics of the coarsest
  * pyramid level. If no such assumption can be made (e.g. the velocity field
  * has the same characteristics as the input images), an initial velocity
  * field can still be set via SetArbitraryInitialVelocityField or
  * SetInput. The filter will then take care of mathching the coarsest level
  * characteristics. If no initial field is set a zero field is used as the
  * initial condition.
  *
  * MultiResolutionPyramidImageFilters are used to downsample the fixed
  * and moving images. A VectorExpandImageFilter is used to upsample
  * the velocity field as we move from a coarse to fine solution.
  *
  * This class is templated over the fixed image type, the moving image type,
  * and the velocity/deformation Field type.
  *
  * \warning This class assumes that the fixed, moving and
  * field image types all have the same number of dimensions.
  *
  * \author Florence Dru, INRIA and Tom Vercauteren, MKT
  *
  * \sa LogDomainDeformableRegistrationFilter
  * \sa LogDomainDemonsRegistrationFilter
  * \sa MultiResolutionPyramidImageFilter
  * \sa VectorExpandImageFilter
  *
  * The current implementation of this class does not support streaming.
  *
  * \ingroup DeformableImageRegistration
  */
template< class TFixedImage, class TMovingImage, class TField, class TRealType = float >
class ITK_EXPORT MultiResolutionLogDomainDeformableRegistration:
  public ImageToImageFilter< TField, TField >
{
public:
  /** Standard class typedefs */
  typedef MultiResolutionLogDomainDeformableRegistration Self;
  typedef ImageToImageFilter< TField, TField >           Superclass;
  typedef SmartPointer< Self >                           Pointer;
  typedef SmartPointer< const Self >                     ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MultiResolutionLogDomainDeformableRegistration,
               ImageToImageFilter);

  /** Fixed image type. */
  typedef TFixedImage                           FixedImageType;
  typedef typename FixedImageType::Pointer      FixedImagePointer;
  typedef typename FixedImageType::ConstPointer FixedImageConstPointer;

  /** Moving image type. */
  typedef TMovingImage                           MovingImageType;
  typedef typename MovingImageType::Pointer      MovingImagePointer;
  typedef typename MovingImageType::ConstPointer MovingImageConstPointer;

  /** Velocity field type. */
  typedef TField                              VelocityFieldType;
  typedef typename VelocityFieldType::Pointer VelocityFieldPointer;

  /** Deformation field image type. */
  typedef TField                                 DeformationFieldType;
  typedef typename DeformationFieldType::Pointer DeformationFieldPointer;

  /** ImageDimension. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      FixedImageType::ImageDimension);

  /** Internal float image type. */
  typedef Image< TRealType, itkGetStaticConstMacro(ImageDimension) > FloatImageType;

  /** The internal registration type. */
  typedef LogDomainDeformableRegistrationFilter< FloatImageType, FloatImageType, VelocityFieldType >
  RegistrationType;

  typedef typename RegistrationType::Pointer RegistrationPointer;

  /** The default registration type. */
  typedef LogDomainDemonsRegistrationFilter<
    FloatImageType, FloatImageType, VelocityFieldType > DefaultRegistrationType;

  /** The fixed multi-resolution image pyramid type. */
  typedef MultiResolutionPyramidImageFilter< FixedImageType, FloatImageType >
  FixedImagePyramidType;

  typedef typename FixedImagePyramidType::Pointer FixedImagePyramidPointer;

  /** The moving multi-resolution image pyramid type. */
  typedef MultiResolutionPyramidImageFilter< MovingImageType, FloatImageType >
  MovingImagePyramidType;

  typedef typename MovingImagePyramidType::Pointer MovingImagePyramidPointer;

  /** The velocity field expander type. */
  typedef VectorResampleImageFilter< VelocityFieldType, VelocityFieldType >
  FieldExpanderType;
  typedef typename FieldExpanderType::Pointer FieldExpanderPointer;

  /** Set the fixed image. */
  virtual void SetFixedImage(const FixedImageType *ptr);

  /** Get the fixed image. */
  const FixedImageType * GetFixedImage(void) const;

  /** Set the moving image. */
  virtual void SetMovingImage(const MovingImageType *ptr);

  /** Get the moving image. */
  const MovingImageType * GetMovingImage(void) const;

  /** Set initial velocity field to be used as is (no smoothing, no
    *  subsampling at the coarsest level of the pyramid. */
  virtual void SetInitialVelocityField(VelocityFieldType *ptr)
  {
    this->m_InitialVelocityField = ptr;
  }

  /** Set initial velocity field. No assumption is made on the
    *  input. It will therefore be smoothed and resampled to match the
    *  images characteristics at the coarsest level of the pyramid. */
  virtual void SetArbitraryInitialVelocityField(VelocityFieldType *ptr)
  {
    this->SetInput(ptr);
  }

  /** Get output velocity field. */
  VelocityFieldType * GetVelocityField() { return this->GetOutput(); }

  /** Get output deformation field. */
  DeformationFieldPointer GetDeformationField();

  /** Get output inverse deformation field. */
  DeformationFieldPointer GetInverseDeformationField();

  /** Get the number of valid inputs.  For
    * MultiResolutionLogDomainDeformableRegistration, this checks whether the
    * fixed and moving images have been set. While
    * MultiResolutionLogDomainDeformableRegistration can take a third input
    * as an initial velocity field, this input is not a required input. */
  virtual std::vector< SmartPointer< DataObject > >::size_type GetNumberOfValidRequiredInputs() const;

  /** Set the internal registrator. */
  itkSetObjectMacro(RegistrationFilter, RegistrationType);

  /** Get the internal registrator. */
  itkGetObjectMacro(RegistrationFilter, RegistrationType);

  /** Set the fixed image pyramid. */
  itkSetObjectMacro(FixedImagePyramid, FixedImagePyramidType);

  /** Get the fixed image pyramid. */
  itkGetObjectMacro(FixedImagePyramid, FixedImagePyramidType);

  /** Set the moving image pyramid. */
  itkSetObjectMacro(MovingImagePyramid, MovingImagePyramidType);

  /** Get the moving image pyramid. */
  itkGetObjectMacro(MovingImagePyramid, MovingImagePyramidType);

  /** Set number of multi-resolution levels. */
  virtual void SetNumberOfLevels(unsigned int num);

  /** Get number of multi-resolution levels. */
  itkGetConstReferenceMacro(NumberOfLevels, unsigned int);

  /** Get the current resolution level being processed. */
  itkGetConstReferenceMacro(CurrentLevel, unsigned int);

  /** Set number of iterations per multi-resolution levels. */
  itkSetVectorMacro(NumberOfIterations, unsigned int, m_NumberOfLevels);

  /** Set the moving image pyramid. */
  itkSetObjectMacro(FieldExpander, FieldExpanderType);

  /** Get the moving image pyramid. */
  itkGetObjectMacro(FieldExpander, FieldExpanderType);

  /** Get number of iterations per multi-resolution levels. */
  virtual const unsigned int * GetNumberOfIterations() const
  { return &( m_NumberOfIterations[0] ); }

  /** Stop the registration after the current iteration. */
  virtual void StopRegistration();

protected:
  MultiResolutionLogDomainDeformableRegistration();
  ~MultiResolutionLogDomainDeformableRegistration() {}
  void PrintSelf(std::ostream & os, Indent indent) const;

  /** Exponential type */
  typedef ExponentialDeformationFieldImageFilter2<
    VelocityFieldType, DeformationFieldType >      FieldExponentiatorType;

  typedef typename FieldExponentiatorType::Pointer FieldExponentiatorPointer;

  itkSetObjectMacro(Exponentiator, FieldExponentiatorType);
  itkGetObjectMacro(Exponentiator, FieldExponentiatorType);

  /** Generate output data by performing the registration
    * at each resolution level. */
  virtual void GenerateData();

  /** The current implementation of this class does not support
    * streaming. As such it requires the largest possible region
    * for the moving, fixed and input velocity field. */
  virtual void GenerateInputRequestedRegion();

  /** By default, the output velocity field has the same
    * spacing, origin and LargestPossibleRegion as the input/initial
    * velocity field. If the initial velocity field is not set, the output
    * information is copied from the fixed image. */
  virtual void GenerateOutputInformation();

  /** The current implementation of this class does not supprot
    * streaming. As such it produces the output for the largest
    * possible region. */
  virtual void EnlargeOutputRequestedRegion(DataObject *ptr);

  /** This method returns true to indicate that the registration should
    * terminate at the current resolution level. */
  virtual bool Halt();

private:
  // purposely not implemented
  MultiResolutionLogDomainDeformableRegistration(const Self &);
  void operator=(const Self &);

  // implemented

  RegistrationPointer       m_RegistrationFilter;
  FixedImagePyramidPointer  m_FixedImagePyramid;
  MovingImagePyramidPointer m_MovingImagePyramid;
  FieldExpanderPointer      m_FieldExpander;
  VelocityFieldPointer      m_InitialVelocityField;

  unsigned int                m_NumberOfLevels;
  unsigned int                m_CurrentLevel;
  std::vector< unsigned int > m_NumberOfIterations;

  /** Flag to indicate user stop registration request. */
  bool m_StopRegistrationFlag;

  FieldExponentiatorPointer m_Exponentiator;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkMultiResolutionLogDomainDeformableRegistration.txx"
#endif

#endif
