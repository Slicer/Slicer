/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkITKTransformInverse.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/// \brief Simplified inverse ITK transforms
///
/// These are simple implementatations of ITK inverse transforms that
/// cannot actually compute any transformations, but only store all transform
/// parameters. This is used for reading/writing transformToParent
/// (resampling transform), even if only if its inverse (transformFromParent)
/// is known.
/// Eventually ITK may support inverse transforms, then these classes can be
/// replaced by those ITK classes.
///

#ifndef __vtkITKTransformInverse_h
#define __vtkITKTransformInverse_h

namespace itk
{

  //----------------------------------------------------------------------------
  template <typename TScalar = double, unsigned int NDimensions = 3, unsigned int VSplineOrder = 3>
  class InverseBSplineTransform :
    public BSplineTransform<TScalar, NDimensions, VSplineOrder>
  {
  public:
    /** Standard class typedefs. */
    using Self = InverseBSplineTransform<TScalar, NDimensions, VSplineOrder>;
    using Superclass = BSplineTransform<TScalar, NDimensions, VSplineOrder>;
    using Pointer = SmartPointer<Self>;
    using ConstPointer = SmartPointer<const Self>;

      /** Run-time type information (and related methods). */
    itkTypeMacro( InverseBSplineTransform, BSplineTransform );

    /** New macro for creation of through a Smart Pointer */
    itkNewMacro( Self );

    /** Define all computation methods as unimplemented to make sure they are not used
    (the inherited implementations are for the forward transform, so they would not give correct results) */
    using Superclass::TransformPoint;
    typename Superclass::OutputPointType TransformPoint(
        const typename Superclass::InputPointType&) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    void ComputeJacobianWithRespectToParameters(
        const typename Superclass::InputPointType &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    // Deprecated in ITKv5. It should be removed when ITK_LEGACY_REMOVE is set to ON.
    void ComputeJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    void ComputeJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::JacobianPositionType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    // Deprecated in ITKv5. It should be removed when ITK_LEGACY_REMOVE is set to ON.
    void ComputeInverseJacobianWithRespectToPosition(
        const typename Superclass::InputPointType &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    void ComputeInverseJacobianWithRespectToPosition(
        const typename Superclass::InputPointType &,
        typename Superclass::InverseJacobianPositionType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    void ComputeJacobianFromBSplineWeightsWithRespectToPosition(
        const typename Superclass::InputPointType &,
        typename Superclass::WeightsType &,
        typename Superclass::ParameterIndexArrayType &) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
  };

  //----------------------------------------------------------------------------
  template <typename TScalar = double, unsigned int NDimensions = 3, unsigned int VSplineOrder = 3>
  class InverseBSplineDeformableTransform :
    public BSplineDeformableTransform<TScalar, NDimensions, VSplineOrder>
  {
  public:
    /** Standard class typedefs. */
    using Self = InverseBSplineDeformableTransform<TScalar, NDimensions, VSplineOrder>;
    using Superclass = BSplineDeformableTransform<TScalar, NDimensions, VSplineOrder>;
    using Pointer = SmartPointer<Self>;
    using ConstPointer = SmartPointer<const Self>;

      /** Run-time type information (and related methods). */
    itkTypeMacro( InverseBSplineDeformableTransform, BSplineDeformableTransform );

    /** New macro for creation of through a Smart Pointer */
    itkNewMacro( Self );

    /** Define all computation methods as unimplemented to make sure they are not used
    (the inherited implementations are for the forward transform, so they would not give correct results) */
    using Superclass::TransformPoint;
    typename Superclass::OutputPointType TransformPoint(
        const typename Superclass::InputPointType&) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    void ComputeJacobianWithRespectToParameters(
        const typename Superclass::InputPointType &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    // Deprecated in ITKv5. It should be removed when ITK_LEGACY_REMOVE is set to ON.
    void ComputeJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    void ComputeJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::JacobianPositionType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    void ComputeInverseJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    void ComputeInverseJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::InverseJacobianPositionType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }

  };

  //----------------------------------------------------------------------------
  template <class TScalar, unsigned int NDimensions>
  class InverseDisplacementFieldTransform :
    public DisplacementFieldTransform<TScalar, NDimensions>
  {
  public:
    /** Standard class typedefs. */
    using Self = InverseDisplacementFieldTransform<TScalar, NDimensions>;
    using Superclass = DisplacementFieldTransform<TScalar, NDimensions>;
    using Pointer = SmartPointer<Self>;
    using ConstPointer = SmartPointer<const Self>;

    /** Run-time type information (and related methods). */
    itkTypeMacro( InverseDisplacementFieldTransform, DisplacementFieldTransform );

    /** New macro for creation of through a Smart Pointer */
    itkNewMacro( Self );

    /** Define all computation methods as unimplemented to make sure they are not used
    (the inherited implementations are for the forward transform, so they would not give correct results) */
    typename Superclass::OutputPointType TransformPoint(
        const typename Superclass::InputPointType&) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    void ComputeJacobianWithRespectToParameters(
        const typename Superclass::InputPointType &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    void ComputeJacobianWithRespectToParameters(
        const typename Superclass::IndexType &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    // Deprecated in ITKv5. It should be removed when ITK_LEGACY_REMOVE is set to ON.
    void ComputeJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    void ComputeJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::JacobianPositionType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    void ComputeJacobianWithRespectToPosition(
        const typename Superclass::IndexType  &,
        typename Superclass::JacobianPositionType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    void ComputeInverseJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    void ComputeInverseJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::InverseJacobianPositionType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    void GetInverseJacobianOfForwardFieldWithRespectToPosition(
        const typename Superclass::InputPointType &,
        typename Superclass::JacobianPositionType &,
        bool useSVD = false ) const override
      {
      (void)useSVD; // unused
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    void GetInverseJacobianOfForwardFieldWithRespectToPosition(
        const typename Superclass::IndexType &,
        typename Superclass::JacobianPositionType &,
        bool useSVD = false ) const override
      {
      (void)useSVD; // unused
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
  };

  //----------------------------------------------------------------------------
  template <class TScalar, unsigned int NDimensions>
  class InverseThinPlateSplineKernelTransform :
    public ThinPlateSplineKernelTransform<TScalar, NDimensions>
  {
  public:
    /** Standard class typedefs. */
    using Self = InverseThinPlateSplineKernelTransform<TScalar, NDimensions>;
    using Superclass = ThinPlateSplineKernelTransform<TScalar, NDimensions>;
    using Pointer = SmartPointer<Self>;
    using ConstPointer = SmartPointer<const Self>;

    /** Run-time type information (and related methods). */
    itkTypeMacro( InverseThinPlateSplineKernelTransform, ThinPlateSplineKernelTransform );

    /** New macro for creation of through a Smart Pointer */
    itkNewMacro( Self );

    /** Define all computation methods as unimplemented to make sure they are not used
    (the inherited implementations are for the forward transform, so they would not give correct results) */
    typename Superclass::OutputPointType TransformPoint(
        const typename Superclass::InputPointType&) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseThinPlateSplineKernelTransform" );
      }
    void ComputeDeformationContribution(const typename Superclass::InputPointType &,
                                              typename Superclass::OutputPointType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseThinPlateSplineKernelTransform" );
      }
    void ComputeJacobianWithRespectToParameters(
        const typename Superclass::InputPointType &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseThinPlateSplineKernelTransform" );
      }
    // Deprecated in ITKv5. It should be removed when ITK_LEGACY_REMOVE is set to ON.
    void ComputeJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseThinPlateSplineKernelTransform" );
      }
    void ComputeJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::JacobianPositionType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseThinPlateSplineKernelTransform" );
      }
    // Deprecated in ITKv5. It should be removed when ITK_LEGACY_REMOVE is set to ON.
    void ComputeInverseJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::JacobianType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseThinPlateSplineKernelTransform" );
      }
    void ComputeInverseJacobianWithRespectToPosition(
        const typename Superclass::InputPointType  &,
        typename Superclass::InverseJacobianPositionType &) const override
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseThinPlateSplineKernelTransform" );
      }
  };

} // end of ITK namespace

using InverseBSplineTransformFloatITKv3Type = itk::InverseBSplineDeformableTransform<float, VTKDimension, BSPLINE_TRANSFORM_ORDER>;
using InverseBSplineTransformDoubleITKv3Type = itk::InverseBSplineDeformableTransform<double, VTKDimension, BSPLINE_TRANSFORM_ORDER>;
using InverseBSplineTransformFloatITKv4Type = itk::InverseBSplineTransform<float, VTKDimension, BSPLINE_TRANSFORM_ORDER>;
using InverseBSplineTransformDoubleITKv4Type = itk::InverseBSplineTransform<double, VTKDimension, BSPLINE_TRANSFORM_ORDER>;

using InverseDisplacementFieldTransformFloatType = itk::InverseDisplacementFieldTransform<float, 3>;
using InverseDisplacementFieldTransformDoubleType = itk::InverseDisplacementFieldTransform<double, 3>;

using InverseThinPlateSplineTransformFloatType = itk::InverseThinPlateSplineKernelTransform<float, 3>;
using InverseThinPlateSplineTransformDoubleType = itk::InverseThinPlateSplineKernelTransform<double, 3>;

#endif // __vtkITKTransformInverse_h
