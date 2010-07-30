#ifndef __TransformAdaptor_txx
#define __TransformAdaptor_txx

#include "TransformAdaptor.h"

namespace itk
{
template< typename TCoordinateType, unsigned int NDimensions,
          typename TInputImage >
TransformAdaptor< TCoordinateType, NDimensions, TInputImage >
::TransformAdaptor()
{
  m_FixedImage   = NULL;
  m_MovingImage  = NULL;

  m_CenterMovingAffineTransform = AffineTransformType::New();
  m_CenterFixedAffineTransform = AffineTransformType::New();

  m_DeCenterMovingAffineTransform = AffineTransformType::New();
  m_DeCenterFixedAffineTransform = AffineTransformType::New();

  m_InputAffineTransform = AffineTransformType::New();
  m_ITKAffineTransform = AffineTransformType::New();
  m_OutputAffineTransform = AffineTransformType::New();
}

template< typename TCoordinateType, unsigned int NDimensions,
          typename TInputImage >
void
TransformAdaptor< TCoordinateType, NDimensions, TInputImage >
::ExecuteInput()
{
  if ( GetInputAffineTransformFilename().size() != 0 )
    {
    std::cout << "Initial Transform :" << GetInputAffineTransformFilename()
              << std::endl;
    // Only read if InputAffineTransformFilename was set.

    if ( GetInputAffineTransformFilename().find(".air16") != std::string::npos )
      {
      SetInputAffineTransform(
        ReadAir16File( GetInputAffineTransformFilename().c_str() ) );
      EstablishCrossOverSystemForAir16();
      ConvertInputAffineToITKAffine();
      }
    else if ( GetInputAffineTransformFilename().find(".xfrm") !=
              std::string::npos )
      {
      SetInputAffineTransform(
        ReadB2AffineFile( GetInputAffineTransformFilename().c_str() ) );
      EstablishCrossOverSystemForB2xfrm();
      ConvertInputAffineToITKAffine();
      }
    else
      {
      itkExceptionMacro( << "Unsupported transform file type, "
                         << GetInputAffineTransformFilename() );
      }
    }
  else   // No InputAffineTransformFilename given by user:
    {
    std::cout << "Initial Transform Not Provided." << std::endl;
    // Compose the preprocess and initialization transforms
    // Get into centered space.
    // This must agree with decoding the 'overall transform', below.

    GetITKAffineTransform()->SetIdentity();
    // const bool ApplyUpstream = false;
    // GetITKAffineTransform()->Compose( GetDeCenterFixedAffineTransform(),
    //                                   ApplyUpstream );
    // GetITKAffineTransform()->Compose( GetCenterMovingAffineTransform(),
    //                                   ApplyUpstream );
    }
}

template< typename TCoordinateType, unsigned int NDimensions,
          typename TInputImage >
void
TransformAdaptor< TCoordinateType, NDimensions, TInputImage >
::ExecuteOutput()
{
  /***************************
    * Compute overall transform in the output side of the transform adaptor
    */

  std::cout << "Overall transform matrix: " << std::endl;
  std::cout << GetITKAffineTransform()->GetMatrix() << std::endl;
  std::cout << "Overall transform offset: " << std::endl;
  std::cout << GetITKAffineTransform()->GetOffset() << std::endl;

  if ( GetOutputAffineTransformFilename().size() != 0 )
    {
    // Only write if OutputAffineTransformFilename was set.

    if ( GetOutputAffineTransformFilename().find(".air16") != std::string::npos )
      {
      EstablishCrossOverSystemForAir16();
      ConvertITKAffineToOutputAffine();
      }
    else if ( GetOutputAffineTransformFilename().find(".xfrm") !=
              std::string::npos )
      {
      EstablishCrossOverSystemForB2xfrm();
      ConvertITKAffineToOutputAffine();
      }
    else
      {
      itkExceptionMacro( << "Unsupported transform file type, "
                         << GetInputAffineTransformFilename() );
      }
    }
}

template< typename TCoordinateType, unsigned int NDimensions,
          typename TInputImage >
void
TransformAdaptor< TCoordinateType, NDimensions, TInputImage >
::EstablishCrossOverSystemForAir16(void)
{
  /*
    *  IMPORTANT:  In the new design, conversion is accomplished by enclosing
    *the
    *              alien transform in an encoding-decoding pair of
    *AffineTransforms.
    *
    *  All the paired EncloseIn operators are expressed in their Inhale sense.
    */

  m_CrossOverAffineSystem = CrossOverAffineSystemType::New();   // established
                                                                // the 4
                                                                // identity
                                                                // Affines.

    { /*  This is code for flipping axes, specific to Air16 files.
        */
    VectorType Flip(1.0);

    const int flipIndex = 1;  // A Flip in Y gets the rotation components of
    // Air16 evidently right.
    Flip[flipIndex] = -1.0;

    GetCrossOverAffineSystem()->EncloseInScaling(Flip, Flip);
    }

  if ( false )  // Turning this off ...
    {
    /*  address voxel floor corners on the outside, address voxel centers on the
      * inside.
      */
    VectorType const MovingImageSpacing( GetMovingImage()->GetSpacing() );

    VectorType const FixedImageSpacing( GetFixedImage()->GetSpacing() );

    ValueType const down(-0.5);

    VectorType const HalfFixedVoxelDown( FixedImageSpacing.operator *(down) );

    ValueType const up(0.5);

    VectorType const HalfMovingVoxelUp( MovingImageSpacing.operator*(up) );

    GetCrossOverAffineSystem()->EncloseInTranslation(HalfMovingVoxelUp,
                                                     HalfFixedVoxelDown);
    }

    { /*  addressing voxels on the outside, addressing millimeters on the
        * inside.
        */
    VectorType const FixedImageScaleReciprocal(
      Reciprocal< TCoordinateType, NDimensions >( GetFixedImage()->GetSpacing() ) );

    VectorType const MovingImageScale( GetMovingImage()->GetSpacing() );

    GetCrossOverAffineSystem()->EncloseInScaling(FixedImageScaleReciprocal,
                                                 MovingImageScale);
    }

  if ( false )  // Turning this alternative off ...
    {
    /*  uncentered on the outside (like the b2 standard, RIP), centered on the
      * inside.
      */
    VectorType const MovingImageCenter(
      GetCenterMovingAffineTransform()->GetOffset() );

    VectorType const FixedImageCenter(
      GetCenterFixedAffineTransform()->GetOffset() );

    GetCrossOverAffineSystem()->EncloseInTranslation(FixedImageCenter,
                                                     -MovingImageCenter);
    }
  if ( false )  // Turning this off made the output image pass through
                // correctly;
    {
    VectorType const MovingImageCenter(
      GetCenterMovingAffineTransform()->GetOffset() );

    VectorType const FixedImageCenter(
      GetCenterFixedAffineTransform()->GetOffset() );

    /*  uncentered on the outside (like the b2 standard, RIP), centered on the
      * inside.
      */
    //  GetCrossOverAffineSystem()->EncloseInTranslation(FixedImageCenter,
    // -MovingImageCenter);

    PointType MovingCenter;
    PointType FixedCenter;

    for ( unsigned int i = 0; i < NDimensions; i++ )
      {
      MovingCenter[i] = MovingImageCenter[i];
      FixedCenter[i] = FixedImageCenter[i];
      }

    GetCrossOverAffineSystem()->EncloseInCentering(FixedCenter, MovingCenter);
    }
}

template< typename TCoordinateType, unsigned int NDimensions,
          typename TInputImage >
void
TransformAdaptor< TCoordinateType, NDimensions, TInputImage >
::EstablishCrossOverSystemForB2xfrm(void)
{
  /*
    *  IMPORTANT:  In the new design, conversion is accomplished by enclosing
    *the
    *              alien transform in an encoding-decoding pair of
    *AffineTransforms.
    *
    *  All the paired EncloseIn operators are expressed in their Inhale sense.
    */

  m_CrossOverAffineSystem = CrossOverAffineSystemType::New();   // established
                                                                // the 4
                                                                // identity
                                                                // Affines.

  if ( false )  // Turning this off ...
    {
    /*  address voxel floor corners on the outside, address voxel centers on the
      * inside.
      */
    VectorType const MovingImageSpacing( GetMovingImage()->GetSpacing() );

    VectorType const FixedImageSpacing( GetFixedImage()->GetSpacing() );

    ValueType const down(-0.5);

    VectorType const HalfFixedVoxelDown( FixedImageSpacing.operator *(down) );

    ValueType const up(0.5);

    VectorType const HalfMovingVoxelUp( MovingImageSpacing.operator*(up) );

    GetCrossOverAffineSystem()->EncloseInTranslation(HalfMovingVoxelUp,
                                                     HalfFixedVoxelDown);
    }

    { /*  addressing voxels on the outside, addressing millimeters on the
        * inside.
        */
    VectorType const FixedImageScaleReciprocal(
      Reciprocal< TCoordinateType, NDimensions >( GetFixedImage()->GetSpacing() ) );

    VectorType const MovingImageScale( GetMovingImage()->GetSpacing() );

    GetCrossOverAffineSystem()->EncloseInScaling(FixedImageScaleReciprocal,
                                                 MovingImageScale);
    }

  if ( false )  // Turning this off made the output image pass through
                // correctly;
    {
    VectorType const MovingImageCenter(
      GetCenterMovingAffineTransform()->GetOffset() );

    VectorType const FixedImageCenter(
      GetCenterFixedAffineTransform()->GetOffset() );

    /*  uncentered on the outside (like the b2 standard, RIP), centered on the
      * inside.
      */
    //  GetCrossOverAffineSystem()->EncloseInTranslation(FixedImageCenter,
    // -MovingImageCenter);

    PointType MovingCenter;
    PointType FixedCenter;

    for ( unsigned int i = 0; i < NDimensions; i++ )
      {
      MovingCenter[i] = MovingImageCenter[i];
      FixedCenter[i] = FixedImageCenter[i];
      }

    GetCrossOverAffineSystem()->EncloseInCentering(FixedCenter, MovingCenter);
    }
}

/*
  *  So much for conversion transform initialization.
  *  The affine convention converters below are general to
  *  any affine representation.  Just initialize the conversion
  *  logic with the appropriate EstablishCrossOverSystem call.
  *
  *  TODO:  Add other conventions besides b2/xfrm, Air16:  matlab/spm, nifti.
  */

template< typename TCoordinateType, unsigned int NDimensions,
          typename TInputImage >
void
TransformAdaptor< TCoordinateType, NDimensions, TInputImage >
::ConvertInputAffineToITKAffine(void)
{
#ifndef NDEBUG
  std::cout << "Inhaling Shift: " << GetInputAffineTransform()->GetOffset()
            << std::endl;
#endif

  GetITKAffineTransform()->SetIdentity();
  const bool ApplyUpstream = false;

  GetITKAffineTransform()->Compose(
    GetCrossOverAffineSystem()->GetInhaleEncodeConversion(),
    ApplyUpstream);
  GetITKAffineTransform()->Compose(GetInputAffineTransform(),
                                   ApplyUpstream);
  GetITKAffineTransform()->Compose(
    GetCrossOverAffineSystem()->GetInhaleDecodeConversion(),
    ApplyUpstream);
#ifndef NDEBUG
  std::cout << std::endl
            << " <]-- Inhaled Shift: "
            << GetITKAffineTransform()->GetOffset() << std::endl;
  std::cout << " <]-- Inhaled Rotation: " << std::endl
            << GetITKAffineTransform()->GetMatrix() << std::endl;
#endif
}

template< typename TCoordinateType, unsigned int NDimensions,
          typename TInputImage >
void
TransformAdaptor< TCoordinateType, NDimensions, TInputImage >
::ConvertITKAffineToOutputAffine(void)
{
  GetOutputAffineTransform()->SetIdentity();
  const bool ApplyUpstream = false;

#ifndef NDEBUG
  std::cout << "Exhaling Shift: " << GetITKAffineTransform()->GetOffset()
            << std::endl;
#endif

  GetOutputAffineTransform()->Compose(
    GetCrossOverAffineSystem()->GetExhaleEncodeConversion(),
    ApplyUpstream);
  GetOutputAffineTransform()->Compose(GetITKAffineTransform(),
                                      ApplyUpstream);
  GetOutputAffineTransform()->Compose(
    GetCrossOverAffineSystem()->GetExhaleDecodeConversion(),
    ApplyUpstream);

#ifndef NDEBUG
  std::cout << std::endl
            << " <]-- Exhaled Shift: "
            << GetOutputAffineTransform()->GetOffset() << std::endl;
  std::cout << " <]-- Exhaled Rotation: " << std::endl
            << GetOutputAffineTransform()->GetMatrix() << std::endl;
#endif
}
}   // namespace itk

#endif
