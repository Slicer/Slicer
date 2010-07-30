#ifndef __CrossOverAffineSystem_h
#define __CrossOverAffineSystem_h

#include <itkLightProcessObject.h>
#include <itkMatrix.h>
#include <itkVector.h>
#include <itkAffineTransform.h>
#include <itkVersorTransform.h>
#include <itkVersorRigid3DTransform.h>
#include "itkScaleVersor3DTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkMacro.h"
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_inverse.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_vector.h>
#include <vnl/algo/vnl_svd.h>
#include <vcl_cmath.h>
#include <stdio.h>

/*
  *  The "Coordinate System" problem is closely related to generation
  *  of reliable choices for an affine transform.  AffineTransforms need
  *  to be generated in terms of a graphical vector space to map from,
  *  and a possibly different graphical vector space to map to.
  *
  *
  */

template< class TCoordinateType,
          unsigned int NDimensions = 3 >
class ITK_EXPORT CrossOverAffineSystem:public itk::LightProcessObject
{
public:
  /** Standard class typedefs. */
  typedef CrossOverAffineSystem           Self;
  typedef itk::LightProcessObject         Superclass;
  typedef itk::SmartPointer< Self >       Pointer;
  typedef itk::SmartPointer< const Self > ConstPointer;

  /** New method for creating an object using a factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(CrossOverAffineSystem, itk::LightProcessObject);

  /** Dimension of the domain space. */
  itkStaticConstMacro(SpaceDimension,  unsigned int, NDimensions);
  itkStaticConstMacro(AffineDimension, unsigned int, NDimensions + 1);

  /** Type of the scalar representing coordinate and vector elements. */
  typedef  TCoordinateType ScalarType;

  typedef vnl_matrix_fixed< TCoordinateType, NDimensions + 1, NDimensions
                            + 1 > VnlTransformMatrixType44;
  // typedef vnl_matrix_fixed<TCoordinateType, NDimensions+1, NDimensions+1>
  //  VnlTransformMatrixType33;

  /** Affine conversion type for this class */
  typedef itk::AffineTransform< TCoordinateType,
                                NDimensions >                                         AffineTransformType;
  typedef typename AffineTransformType::Pointer          AffineTransformPointer;
  typedef typename AffineTransformType::MatrixType       MatrixType;
  typedef typename AffineTransformType::InputPointType   PointType;
  typedef typename AffineTransformType::OutputVectorType VectorType;
  typedef typename VectorType::ValueType                 ValueType;

  /** Quaternion conversion types for this class */
  typedef itk::VersorTransform< TCoordinateType >      VersorTransformType;
  typedef typename VersorTransformType::Pointer        VersorTransformPointer;
  typedef typename VersorTransformType::ParametersType VersorParametersType;

  typedef itk::VersorRigid3DTransform< TCoordinateType >
  VersorRigid3DTransformType;
  typedef typename VersorRigid3DTransformType::Pointer
  VersorRigid3DTransformPointer;
  typedef typename VersorRigid3DTransformType::ParametersType
  VersorRigid3DParametersType;

  typedef itk::ScaleVersor3DTransform< TCoordinateType >
  ScaleVersor3DTransformType;
  typedef typename ScaleVersor3DTransformType::Pointer
  ScaleVersor3DTransformPointer;
  typedef typename ScaleVersor3DTransformType::ParametersType
  ScaleVersor3DParametersType;

  typedef itk::ScaleSkewVersor3DTransform< TCoordinateType >
  ScaleSkewVersor3DTransformType;
  typedef typename ScaleSkewVersor3DTransformType::Pointer
  ScaleSkewVersor3DTransformPointer;
  typedef typename ScaleSkewVersor3DTransformType::ParametersType
  ScaleSkewVersor3DParametersType;

  /** Get the four coordinated AffineTransform conversions. */
  itkGetMacro(InhaleEncodeConversion, AffineTransformPointer);
  itkGetMacro(InhaleDecodeConversion, AffineTransformPointer);
  itkGetMacro(ExhaleEncodeConversion, AffineTransformPointer);
  itkGetMacro(ExhaleDecodeConversion, AffineTransformPointer);

  /** Generate the four coordinated AffineTransform conversions. */
  void EncloseInScaling(const VectorType & EncodeScale,
                        const VectorType & DecodeScale);

  void EncloseInTranslation(const VectorType & EncodeShift,
                            const VectorType & DecodeShift);

  void EncloseInCentering(const PointType & EncodeCenter,
                          const PointType & DecodeCenter);

  void EncloseInAffineTransforms(AffineTransformPointer EncodeAffineTransform,
                                 AffineTransformPointer DecodeAffineTransform);

protected:

  /** Set the four coordinated AffineTransform conversions. */
  itkSetMacro(InhaleEncodeConversion, AffineTransformPointer);
  itkSetMacro(InhaleDecodeConversion, AffineTransformPointer);
  itkSetMacro(ExhaleEncodeConversion, AffineTransformPointer);
  itkSetMacro(ExhaleDecodeConversion, AffineTransformPointer);

  CrossOverAffineSystem();
  virtual ~CrossOverAffineSystem();

  mutable AffineTransformPointer m_InhaleEncodeConversion;
  mutable AffineTransformPointer m_InhaleDecodeConversion;
  mutable AffineTransformPointer m_ExhaleEncodeConversion;
  mutable AffineTransformPointer m_ExhaleDecodeConversion;
private:
  CrossOverAffineSystem(const Self &); // purposely not implemented
  void operator=(const Self &);        // purposely not implemented
};

#ifndef ITK_MANUAL_INSTANTIATION
#  include "CrossOverAffineSystem.txx"
#endif

#endif
