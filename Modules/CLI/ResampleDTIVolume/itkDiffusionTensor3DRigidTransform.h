/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DRigidTransform_h
#define itkDiffusionTensor3DRigidTransform_h

#include "itkDiffusionTensor3DMatrix3x3Transform.h"
#include <itkVersorRigid3DTransform.h>

#define PRECISION .001

namespace itk
{
/** \class DiffusionTensor3DRigidTransform
 *
 * This class implement a rigid transformation for diffusion tensors (rotation and translation).
 */
template <class TData>
class DiffusionTensor3DRigidTransform
  : public DiffusionTensor3DMatrix3x3Transform<TData>
{
public:
  using DataType = TData;
  using Self = DiffusionTensor3DRigidTransform<DataType>;
  using Superclass = DiffusionTensor3DMatrix3x3Transform<DataType>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using MatrixTransformType = typename Superclass::MatrixTransformType;
  using InternalMatrixTransformType = typename Superclass::InternalMatrixTransformType;
  using MatrixTransform4x4Type = Matrix<double, 4, 4>;
  using Rigid3DTransformType = Rigid3DTransform<double>;
  using VersorRigid3DTransformType = VersorRigid3DTransform<double>;
  // /Set the 4x4 Matrix (the last row is ignored and considered to be 0,0,0,1
  void SetMatrix4x4( MatrixTransform4x4Type matrix );

  // /Set the transformation matrix from an itk::RigidTransform< double > object
  void SetTransform( typename Rigid3DTransformType::Pointer transform );
  typename VersorRigid3DTransformType::Pointer GetRigidTransform();

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DRigidTransform, DiffusionTensor3DMatrix3x3Transform);

  itkNewMacro( Self );
  // /Set the 3x3 rotation matrix
  void SetMatrix3x3( MatrixTransformType & matrix ) override;

  void DisablePrecision();

  itkSetMacro( PrecisionChecking, bool );
protected:
  DiffusionTensor3DRigidTransform();
  bool m_PrecisionChecking;
  double GetDet( MatrixTransformType & matrix );

  void PreCompute() override;

};

} // end itk namespace

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DRigidTransform.txx"
#endif

#endif
