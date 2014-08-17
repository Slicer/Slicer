/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageResliceMask.h,v $

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
    typedef InverseBSplineTransform                            Self;
    typedef BSplineTransform<TScalar,NDimensions,VSplineOrder> Superclass;
    typedef SmartPointer<Self>                                 Pointer;
    typedef SmartPointer<const Self>                           ConstPointer;

      /** Run-time type information (and related methods). */
    itkTypeMacro( InverseBSplineTransform, BSplineTransform );

    /** New macro for creation of through a Smart Pointer */
    itkNewMacro( Self );

    /** Define all computation methods as unimplemented to make sure they are not used
    (the inherited implementations are for the forward transform, so they would not give correct results) */
    virtual OutputPointType TransformPoint( const InputPointType& thisPoint ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    virtual void ComputeJacobianWithRespectToParameters(const InputPointType &, JacobianType & j) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    virtual void ComputeJacobianWithRespectToParameters(const IndexType &, JacobianType & j) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    virtual void ComputeJacobianWithRespectToPosition(const InputPointType  & x, JacobianType & j ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    virtual void ComputeInverseJacobianWithRespectToPosition(const InputPointType  & x, JacobianType & j ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    virtual void ComputeJacobianWithRespectToPosition(const IndexType  & x, JacobianType & j ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }

    void ComputeJacobianFromBSplineWeightsWithRespectToPosition(const InputPointType &, WeightsType &, ParameterIndexArrayType & ) const
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
    typedef InverseBSplineDeformableTransform                            Self;
    typedef BSplineDeformableTransform<TScalar,NDimensions,VSplineOrder> Superclass;
    typedef SmartPointer<Self>                                 Pointer;
    typedef SmartPointer<const Self>                           ConstPointer;

      /** Run-time type information (and related methods). */
    itkTypeMacro( InverseBSplineDeformableTransform, BSplineDeformableTransform );

    /** New macro for creation of through a Smart Pointer */
    itkNewMacro( Self );

    /** Define all computation methods as unimplemented to make sure they are not used
    (the inherited implementations are for the forward transform, so they would not give correct results) */
    virtual OutputPointType TransformPoint( const InputPointType& thisPoint ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    virtual void ComputeJacobianWithRespectToParameters(const InputPointType &, JacobianType & j) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    virtual void ComputeJacobianWithRespectToParameters(const IndexType &, JacobianType & j) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    virtual void ComputeJacobianWithRespectToPosition(const InputPointType  & x, JacobianType & j ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    virtual void ComputeInverseJacobianWithRespectToPosition(const InputPointType  & x, JacobianType & j ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }
    virtual void ComputeJacobianWithRespectToPosition(const IndexType  & x, JacobianType & j ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseBSplineTransform" );
      }

    void ComputeJacobianFromBSplineWeightsWithRespectToPosition(const InputPointType &, WeightsType &, ParameterIndexArrayType & ) const
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
    typedef InverseDisplacementFieldTransform            Self;
    typedef DisplacementFieldTransform<TScalar, NDimensions> Superclass;
    typedef SmartPointer<Self>                           Pointer;
    typedef SmartPointer<const Self>                     ConstPointer;

    /** Run-time type information (and related methods). */
    itkTypeMacro( InverseDisplacementFieldTransform, DisplacementFieldTransform );

    /** New macro for creation of through a Smart Pointer */
    itkNewMacro( Self );

    /** Define all computation methods as unimplemented to make sure they are not used
    (the inherited implementations are for the forward transform, so they would not give correct results) */
    virtual OutputPointType TransformPoint( const InputPointType& thisPoint ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    virtual void ComputeJacobianWithRespectToParameters(const InputPointType &, JacobianType & j) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    virtual void ComputeJacobianWithRespectToParameters(const IndexType &, JacobianType & j) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    virtual void ComputeJacobianWithRespectToPosition(const InputPointType  & x, JacobianType & j ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    virtual void ComputeInverseJacobianWithRespectToPosition(const InputPointType  & x, JacobianType & j ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    virtual void ComputeJacobianWithRespectToPosition(const IndexType  & x, JacobianType & j ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    virtual void GetInverseJacobianOfForwardFieldWithRespectToPosition(const InputPointType & point, JacobianType & jacobian,bool useSVD = false ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
    virtual void GetInverseJacobianOfForwardFieldWithRespectToPosition(const IndexType & index, JacobianType & jacobian, bool useSVD = false ) const
      {
      itkExceptionMacro( "Only storage methods are implemented for InverseDisplacementFieldTransform" );
      }
  };
} // end of ITK namespace

typedef itk::InverseBSplineDeformableTransform< float, VTKDimension, VTKDimension > InverseBSplineTransformFloatITKv3Type;
typedef itk::InverseBSplineDeformableTransform< double, VTKDimension, VTKDimension > InverseBSplineTransformDoubleITKv3Type;
typedef itk::InverseBSplineTransform< float, VTKDimension, VTKDimension > InverseBSplineTransformFloatITKv4Type;
typedef itk::InverseBSplineTransform< double, VTKDimension, VTKDimension > InverseBSplineTransformDoubleITKv4Type;

typedef itk::InverseDisplacementFieldTransform< double, 3 > InverseDisplacementFieldTransformType;

#endif // __vtkITKTransformInverse_h
