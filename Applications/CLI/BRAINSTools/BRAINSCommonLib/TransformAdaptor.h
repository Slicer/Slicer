#ifndef __TransformAdaptor_h
#define __TransformAdaptor_h

#include "CrossOverAffineSystem.h"

namespace itk
{
/** \class TransformAdaptor
  *
  * This component converts transform formats required for
  * input and output for use in registration.
  *
  * The preprocessing is activatived by method ExecuteInput().
  *
  * The postprocessing is activatived by method ExecuteOutput().
  *
  * Inputs:
  *    - pointer to original fixed image
  *    - pointer original moving image
  *
  * Outputs:
  *    - pointer to transform representing the pre-transform
  *    - pointer to transform representing the post-transform
  *
  * After registration, the overall transform is obtained by
  * composing pre-transform, the registration transform and
  * the post-transform.
  *
  */
template< typename TCoordinateType, unsigned int NDimensions,
          typename TInputImage >
class ITK_EXPORT TransformAdaptor:public LightProcessObject
{
public:

  /** Standard class typedefs. */
  typedef TransformAdaptor           Self;
  typedef Object                     Superclass;
  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(TransformAdaptor, Object);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Input Image Type. */
  typedef TInputImage                       InputImageType;
  typedef typename InputImageType::Pointer  InputImagePointer;
  typedef typename InputImageType::SizeType InputImageSizeType;

  /** Image dimension enumeration. */
  itkStaticConstMacro(ImageDimension, unsigned int, TInputImage::ImageDimension);

  /** Type of the scalar representing coordinate and vector elements. */
  typedef  TCoordinateType
  ScalarType;

  typedef vnl_matrix_fixed< TCoordinateType, NDimensions + 1, NDimensions
                            + 1 > VnlTransformMatrixType44;

  /** Affine transform type. */
  typedef AffineTransform< TCoordinateType,
                           itkGetStaticConstMacro(ImageDimension) > AffineTransformType;
  typedef typename AffineTransformType::Pointer
  AffineTransformPointer;
  typedef typename AffineTransformType::MatrixType
  MatrixType;
  typedef typename AffineTransformType::InputPointType
  PointType;
  typedef typename AffineTransformType::OutputVectorType
  VectorType;
  typedef typename VectorType::ValueType
  ValueType;

  /** CrossOverAffineSystem type. */
  typedef CrossOverAffineSystem< TCoordinateType,
                                 itkGetStaticConstMacro(ImageDimension) > CrossOverAffineSystemType;
  typedef typename CrossOverAffineSystemType::Pointer
  CrossOverAffineSystemPointer;

  /** Set the input fixed image. */
  iplSetMacro(FixedImage, InputImagePointer);
  iplGetMacro(FixedImage, InputImagePointer);

  /** Set the input moving image. */
  iplSetMacro(MovingImage, InputImagePointer);
  iplGetMacro(MovingImage, InputImagePointer);

  /** Methods to execute the transform processing. */
  void ExecuteInput();

  void ExecuteOutput();

  /** Methods to define and perform Air16 AffineTransform conversion. */
  void EstablishCrossOverSystemForAir16(void);

  void EstablishCrossOverSystemForB2xfrm(void);

  void ConvertInputAffineToITKAffine(void);

  void ConvertITKAffineToOutputAffine(void);

  iplSetMacro(CenterMovingAffineTransform, AffineTransformPointer);
  iplGetMacro(CenterMovingAffineTransform, AffineTransformPointer);
  iplSetMacro(DeCenterMovingAffineTransform, AffineTransformPointer);
  iplGetMacro(DeCenterMovingAffineTransform, AffineTransformPointer);
  iplSetMacro(CenterFixedAffineTransform, AffineTransformPointer);
  iplGetMacro(CenterFixedAffineTransform, AffineTransformPointer);
  iplSetMacro(DeCenterFixedAffineTransform, AffineTransformPointer);
  iplGetMacro(DeCenterFixedAffineTransform, AffineTransformPointer);

  iplGetMacro(InputAffineTransformFilename, std::string);
  iplSetMacro(InputAffineTransformFilename, std::string);
  iplGetMacro(OutputAffineTransformFilename, std::string);
  iplSetMacro(OutputAffineTransformFilename, std::string);

  iplSetMacro(InputAffineTransform, AffineTransformPointer);
  iplGetMacro(InputAffineTransform, AffineTransformPointer);
  iplSetMacro(ITKAffineTransform, AffineTransformPointer);
  iplGetMacro(ITKAffineTransform, AffineTransformPointer);
  iplSetMacro(OutputAffineTransform, AffineTransformPointer);
  iplGetMacro(OutputAffineTransform, AffineTransformPointer);

  iplSetMacro(CrossOverAffineSystem, CrossOverAffineSystemPointer);
  iplGetMacro(CrossOverAffineSystem, CrossOverAffineSystemPointer);
protected:
  TransformAdaptor();
  ~TransformAdaptor()
  {}
private:
  TransformAdaptor(const Self &);               // purposely not implemented
  void operator=(const Self &);                 // purposely not implemented

  InputImagePointer m_FixedImage;
  InputImagePointer m_MovingImage;

  AffineTransformPointer m_CenterFixedAffineTransform;
  AffineTransformPointer m_CenterMovingAffineTransform;

  AffineTransformPointer m_DeCenterFixedAffineTransform;
  AffineTransformPointer m_DeCenterMovingAffineTransform;

  std::string m_InputAffineTransformFilename;
  std::string m_OutputAffineTransformFilename;

  AffineTransformPointer m_ITKAffineTransform;
  AffineTransformPointer m_InputAffineTransform;
  AffineTransformPointer m_OutputAffineTransform;

  CrossOverAffineSystemPointer m_CrossOverAffineSystem;
};
}   // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "TransformAdaptor.txx"
#endif

#endif
