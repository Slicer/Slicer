#ifndef __itkMultiResolutionPDEDeformableRegistration2_h
#define __itkMultiResolutionPDEDeformableRegistration2_h

#include "itkImage.h"
#include "itkImageToImageFilter.h"
#include "itkPDEDeformableRegistrationFilter.h"
#include "itkDemonsRegistrationFilter.h"
#include "itkMultiResolutionPyramidImageFilter.h"
#include "itkVectorResampleImageFilter.h"

#include <vector>

namespace itk
{
/**
 * \class MultiResolutionPDEDeformableRegistration2
 * \brief Framework for performing multi-resolution PDE
 * deformable registration ( with linear interpolation and NN extrapolation )
 *
 * MultiResolutionPDEDeformableRegistration2 provides a generic framework
 * to peform multi-resolution deformable registration.
 *
 * At each resolution level a PDEDeformableRegistrationFilter is used
 * to register two images by computing the deformation field which will 
 * map a moving image onto a fixed image.
 *
 * A deformation field is represented as an image whose pixel type is some
 * vector type with at least N elements, where N is the dimension of
 * the fixed image. The vector type must support element access via operator
 * []. It is assumed that the vector elements behave like floating point
 * scalars.
 *
 * The internal PDEDeformationRegistrationFilter can be set using
 * SetRegistrationFilter. By default a DemonsRegistrationFilter is used.
 *
 * The input fixed and moving images are set via methods SetFixedImage
 * and SetMovingImage respectively. An initial deformation field maybe set via
 * SetInitialDeformationField or SetInput. If no initial field is set
 * a zero field is used as the initial condition.
 *
 * MultiResolutionPyramidImageFilters are used to downsample the fixed
 * and moving images. A VectorExpandImageFilter is used to upsample
 * the deformation as we move from a coarse to fine solution.
 *
 * This class is templated over the fixed image type, the moving image type,
 * and the Deformation Field type.
 *
 * \author Tom Vercauteren, INRIA & Mauna Kea Technologies
 *
 * \warning This class assumes that the fixed, moving and deformation
 * field image types all have the same number of dimensions.
 *
 * \warning The difference between this class and the initial
 * MultiResolutionPDEDeformableRegistration2 is the type of interpolator
 * we use. Here we also ask for NN extrapolation.
 *
 * \sa PDEDeformableRegistrationFilter
 * \sa DemonsRegistrationFilter
 * \sa MultiResolutionPyramidImageFilter
 * \sa VectorExpandImageFilter
 *
 * The current implementation of this class does not support streaming.
 *
 * \ingroup DeformableImageRegistration
 */
template <class TFixedImage, class TMovingImage, class TDeformationField, class  TRealType = float>
class ITK_EXPORT MultiResolutionPDEDeformableRegistration2 :
    public ImageToImageFilter <TDeformationField, TDeformationField>
{
public:
  /** Standard class typedefs */
  typedef MultiResolutionPDEDeformableRegistration2 Self;
  typedef ImageToImageFilter<TDeformationField, TDeformationField>
  Superclass;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( MultiResolutionPDEDeformableRegistration2, 
                ImageToImageFilter );

  /** Fixed image type. */
  typedef TFixedImage FixedImageType;
  typedef typename FixedImageType::Pointer FixedImagePointer;
  typedef typename FixedImageType::ConstPointer FixedImageConstPointer;

  /** Moving image type. */
  typedef TMovingImage MovingImageType;
  typedef typename MovingImageType::Pointer MovingImagePointer;
  typedef typename MovingImageType::ConstPointer MovingImageConstPointer;

  /** Deformation field image type. */
  typedef TDeformationField DeformationFieldType;
  typedef typename DeformationFieldType::Pointer DeformationFieldPointer;

  /** ImageDimension. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      FixedImageType::ImageDimension);

  /** Internal float image type. */
  typedef Image<TRealType,itkGetStaticConstMacro(ImageDimension)> FloatImageType;

  /** The internal registration type. */
  typedef PDEDeformableRegistrationFilter<
    FloatImageType, FloatImageType, DeformationFieldType > RegistrationType;
  typedef typename RegistrationType::Pointer RegistrationPointer;

  /** The default registration type. */
  typedef DemonsRegistrationFilter<
    FloatImageType, FloatImageType, DeformationFieldType > DefaultRegistrationType;

  /** The fixed multi-resolution image pyramid type. */
  typedef MultiResolutionPyramidImageFilter<
    FixedImageType, FloatImageType > FixedImagePyramidType;
  typedef typename FixedImagePyramidType::Pointer FixedImagePyramidPointer;

  /** The moving multi-resolution image pyramid type. */
  typedef MultiResolutionPyramidImageFilter<
    MovingImageType, FloatImageType > MovingImagePyramidType;
  typedef typename MovingImagePyramidType::Pointer MovingImagePyramidPointer;
   
  /** The deformation field expander type. */
  typedef VectorResampleImageFilter<
    DeformationFieldType, DeformationFieldType > FieldExpanderType;
  typedef typename FieldExpanderType::Pointer  FieldExpanderPointer;

  /** Set the fixed image. */
  virtual void SetFixedImage( const FixedImageType * ptr );

  /** Get the fixed image. */
  const FixedImageType * GetFixedImage(void) const;

  /** Set the moving image. */
  virtual void SetMovingImage( const MovingImageType * ptr );

  /** Get the moving image. */
  const MovingImageType * GetMovingImage(void) const;

  /** Set initial deformation field. */
  virtual void SetInitialDeformationField( DeformationFieldType * itkNotUsed(ptr) )
  {
    itkExceptionMacro( << "This feature not implemented yet"  );
    // this->SetInput( ptr ); 
  }

  /** Get output deformation field. */
  const DeformationFieldType * GetDeformationField(void)
  { return this->GetOutput(); }

  /** Get the number of valid inputs.  For
   * MultiResolutionPDEDeformableRegistration2, this checks whether the
   * fixed and moving images have been set. While
   * MultiResolutionPDEDeformableRegistration2 can take a third input
   * as an initial deformation field, this input is not a required input.
   */
  virtual std::vector<SmartPointer<DataObject> >::size_type GetNumberOfValidRequiredInputs() const;

  /** Set the internal registrator. */
  itkSetObjectMacro( RegistrationFilter, RegistrationType );

  /** Get the internal registrator. */
  itkGetObjectMacro( RegistrationFilter, RegistrationType );
  
  /** Set the fixed image pyramid. */
  itkSetObjectMacro( FixedImagePyramid, FixedImagePyramidType );

  /** Get the fixed image pyramid. */
  itkGetObjectMacro( FixedImagePyramid, FixedImagePyramidType );

  /** Set the moving image pyramid. */
  itkSetObjectMacro( MovingImagePyramid, MovingImagePyramidType );

  /** Get the moving image pyramid. */
  itkGetObjectMacro( MovingImagePyramid, MovingImagePyramidType );

  /** Set number of multi-resolution levels. */
  virtual void SetNumberOfLevels( unsigned int num );

  /** Get number of multi-resolution levels. */
  itkGetConstReferenceMacro( NumberOfLevels, unsigned int );

  /** Get the current resolution level being processed. */
  itkGetConstReferenceMacro( CurrentLevel, unsigned int );

  /** Set number of iterations per multi-resolution levels. */
  itkSetVectorMacro( NumberOfIterations, unsigned int, m_NumberOfLevels );

  /** Get number of iterations per multi-resolution levels. */
  virtual const unsigned int * GetNumberOfIterations() const
  { return &(m_NumberOfIterations[0]); }

  /** Stop the registration after the current iteration. */
  virtual void StopRegistration();

protected:
  MultiResolutionPDEDeformableRegistration2();
  ~MultiResolutionPDEDeformableRegistration2() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** Generate output data by performing the registration
   * at each resolution level. */
  virtual void GenerateData();

  /** The current implementation of this class does not support
   * streaming. As such it requires the largest possible region
   * for the moving, fixed and input deformation field. */
  virtual void GenerateInputRequestedRegion();

  /** By default, the output deformation field has the same
   * spacing, origin and LargestPossibleRegion as the input/initial
   * deformation field.
   *
   * If the initial deformation field is not set, the output
   * information is copied from the fixed image. */
  virtual void GenerateOutputInformation();

  /** The current implementation of this class does not supprot
   * streaming. As such it produces the output for the largest
   * possible region. */
  virtual void EnlargeOutputRequestedRegion( DataObject *ptr );

  /** This method returns true to indicate that the registration should
   * terminate at the current resolution level. */
  virtual bool Halt();

private:
  MultiResolutionPDEDeformableRegistration2(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  
  RegistrationPointer        m_RegistrationFilter;
  FixedImagePyramidPointer   m_FixedImagePyramid;
  MovingImagePyramidPointer  m_MovingImagePyramid;
  FieldExpanderPointer       m_FieldExpander;

  unsigned int               m_NumberOfLevels;
  unsigned int               m_CurrentLevel;
  std::vector<unsigned int>  m_NumberOfIterations;

  /** Flag to indicate user stop registration request. */
  bool                      m_StopRegistrationFlag;

};


} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiResolutionPDEDeformableRegistration2.txx"
#endif


#endif
