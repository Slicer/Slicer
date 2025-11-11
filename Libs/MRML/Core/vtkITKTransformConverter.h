/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkITKTransformConverter.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or https://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkITKTransformConverter_h
#define __vtkITKTransformConverter_h

#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkOrientedBSplineTransform.h>
#include <vtkPoints.h>
#include <vtkThinPlateSplineTransform.h>
#include <vtkTransform.h>

// ITK includes
#include <itkAffineTransform.h>
#include <itkBSplineDeformableTransform.h> // ITKv3 style
#include <itkBSplineTransform.h>           // ITKv4 style
#include <itkCompositeTransform.h>
#include <itkCompositeTransformIOHelper.h>
#include <itkDisplacementFieldTransform.h>
#include <itkIdentityTransform.h>
#include <itkTransformFileWriter.h>
#include <itkTransformFileReader.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkTranslationTransform.h>
#include <itkScaleTransform.h>
#include <itkTransformFactory.h>
#include <itkThinPlateSplineKernelTransform.h>

// Constants and typedefs

static const unsigned int VTKDimension = 3;

static const int BSPLINE_TRANSFORM_ORDER = 3;

typedef itk::TransformFileWriter TransformWriterType;

typedef itk::DisplacementFieldTransform<double, 3> DisplacementFieldTransformDoubleType;
typedef DisplacementFieldTransformDoubleType::DisplacementFieldType GridImageDoubleType;

typedef itk::ThinPlateSplineKernelTransform<double, 3> ThinPlateSplineTransformDoubleType;

#include "vtkITKTransformInverse.h"

class vtkITKTransformConverter
{
public:
  static void RegisterInverseTransformTypes();

  template <typename T>
  static vtkAbstractTransform* CreateVTKTransformFromITK(vtkObject* loggerObject,
                                                         typename itk::TransformBaseTemplate<T>::Pointer transformItk,
                                                         double center_LocalRAS[3] = nullptr);

  ///
  /// Create an ITK transform from a VTK transform.
  /// secondaryTransformItk: Only for backward compatibility. For BSpline transforms that have an additive bulk component, the bulk component is saved
  ///   in the secondary transform.
  /// preferITKv3CompatibleTransforms: If true then the BSpline transform will be created as a BSplineDeformableTransform and additive bulk transform component
  ///  is always written in the secondary transform. If false then the BSpline transform is written as a BSplineTransform (multiplicative bulk component
  ///  is saved in a composite transform).
  /// If initialize is set to true then the transform is initialized to be readily usable.
  /// Initialization takes a long time for kernel transforms with many points,
  /// If a transform is created only to write it to file, initialization can be turned off to improve performance.
  static itk::Object::Pointer CreateITKTransformFromVTK(vtkObject* loggerObject,
                                                        vtkAbstractTransform* transformVtk,
                                                        itk::Object::Pointer& secondaryTransformItk,
                                                        int preferITKv3CompatibleTransforms,
                                                        bool initialize = true,
                                                        double center_LocalRAS[3] = nullptr);

  template <typename T>
  static bool SetVTKBSplineFromITKv3Generic(vtkObject* loggerObject,
                                            vtkOrientedBSplineTransform* bsplineVtk,
                                            typename itk::TransformBaseTemplate<T>::Pointer warpTransformItk,
                                            typename itk::TransformBaseTemplate<T>::Pointer bulkTransformItk);

  template <typename T>
  static bool SetVTKOrientedGridTransformFromITKImage(vtkObject* loggerObject,
                                                      vtkOrientedGridTransform* grid_Ras,
                                                      typename itk::DisplacementFieldTransform<T, 3>::DisplacementFieldType::Pointer gridImage_Lps);
  static bool SetITKImageFromVTKOrientedGridTransform(vtkObject* loggerObject, GridImageDoubleType::Pointer& gridImage_Lps, vtkOrientedGridTransform* grid_Ras);

  // Convert a 2D ITK matrix to a 3D ITK matrix.
  // Spacing for the third dimension is geometric mean of the other two dimensions.
  // This is chosen to minimize geometric distortion.
  // Sign of the third dimension is chosen to create a right-handed coordinate system.
  template <typename T>
  static itk::Matrix<T, 3, 3> Matrix2Dto3D(itk::Matrix<T, 2, 2> m2D);

protected:
  template <typename T>
  static typename itk::AffineTransform<T, 3>::Pointer ConvertITKLinearTransformFrom2Dto3D(typename itk::TransformBaseTemplate<T>::Pointer transformItk_LPS);
  template <typename T>
  static bool SetVTKLinearTransformFromITK(vtkObject* loggerObject,
                                           vtkMatrix4x4* transformVtk_RAS,
                                           typename itk::TransformBaseTemplate<T>::Pointer transformItk_LPS,
                                           double center_LocalRAS[3] = nullptr);
  static bool SetITKLinearTransformFromVTK(vtkObject* loggerObject, itk::Object::Pointer& transformItk_LPS, vtkMatrix4x4* transformVtk_RAS, double center_LocalRAS[3] = nullptr);

  template <typename T>
  static bool SetVTKOrientedGridTransformFrom2DITKImage(vtkObject* loggerObject,
                                                        vtkOrientedGridTransform* grid_Ras,
                                                        typename itk::DisplacementFieldTransform<T, 2>::DisplacementFieldType::Pointer gridImage_Lps);

  template <typename T, unsigned Dimension>
  static bool SetVTKOrientedGridTransformFromITK(vtkObject* loggerObject,
                                                 vtkOrientedGridTransform* transformVtk_RAS,
                                                 typename itk::TransformBaseTemplate<T>::Pointer transformItk_LPS);
  static bool SetITKOrientedGridTransformFromVTK(vtkObject* loggerObject, itk::Object::Pointer& transformItk_LPS, vtkOrientedGridTransform* transformVtk_RAS);

  static bool SetITKv3BSplineFromVTK(vtkObject* loggerObject,
                                     itk::Object::Pointer& warpTransformItk,
                                     itk::Object::Pointer& bulkTransformItk,
                                     vtkOrientedBSplineTransform* bsplineVtk,
                                     bool alwaysAddBulkTransform);
  static bool SetITKv4BSplineFromVTK(vtkObject* loggerObject, itk::Object::Pointer& warpTransformItk, vtkOrientedBSplineTransform* bsplineVtk);

  template <typename T, unsigned VDimension>
  static bool SetVTKThinPlateSplineTransformFromITK(vtkObject* loggerObject,
                                                    vtkThinPlateSplineTransform* transformVtk_RAS,
                                                    typename itk::TransformBaseTemplate<T>::Pointer transformItk_LPS);
  static bool SetITKThinPlateSplineTransformFromVTK(vtkObject* loggerObject,
                                                    itk::Object::Pointer& transformItk_LPS,
                                                    vtkThinPlateSplineTransform* transformVtk_RAS,
                                                    bool initialize = true);

  static bool IsIdentityMatrix(vtkMatrix4x4* matrix);

  template <typename BSplineTransformType>
  static bool SetVTKBSplineParametersFromITKGeneric(vtkObject* loggerObject,
                                                    vtkOrientedBSplineTransform* bsplineVtk,
                                                    typename itk::TransformBaseTemplate<typename BSplineTransformType::ScalarType>::Pointer warpTransformItk);
  template <typename T>
  static bool SetVTKBSplineFromITKv4Generic(vtkObject* loggerObject, vtkOrientedBSplineTransform* bsplineVtk, typename itk::TransformBaseTemplate<T>::Pointer warpTransformItk);

  template <typename BSplineTransformType>
  static bool SetITKBSplineParametersFromVTKGeneric(vtkObject* loggerObject,
                                                    typename itk::Transform<typename BSplineTransformType::ScalarType, VTKDimension, VTKDimension>::Pointer& warpTransformItk,
                                                    vtkOrientedBSplineTransform* bsplineVtk);
  template <typename T>
  static bool SetITKv3BSplineFromVTKGeneric(vtkObject* loggerObject,
                                            typename itk::Transform<T, VTKDimension, VTKDimension>::Pointer& warpTransformItk,
                                            typename itk::Transform<T, VTKDimension, VTKDimension>::Pointer& bulkTransformItk,
                                            vtkOrientedBSplineTransform* bsplineVtk,
                                            bool alwaysAddBulkTransform);
  template <typename T>
  static bool SetITKv4BSplineFromVTKGeneric(vtkObject* loggerObject,
                                            typename itk::Transform<T, VTKDimension, VTKDimension>::Pointer& warpTransformItk,
                                            vtkOrientedBSplineTransform* bsplineVtk);
};

//----------------------------------------------------------------------------
void vtkITKTransformConverter::RegisterInverseTransformTypes()
{
  itk::TransformFactory<InverseDisplacementFieldTransformFloatType>::RegisterTransform();
  itk::TransformFactory<InverseDisplacementFieldTransformDoubleType>::RegisterTransform();

  itk::TransformFactory<InverseBSplineTransformFloatITKv3Type>::RegisterTransform();
  itk::TransformFactory<InverseBSplineTransformFloatITKv4Type>::RegisterTransform();
  itk::TransformFactory<InverseBSplineTransformDoubleITKv3Type>::RegisterTransform();
  itk::TransformFactory<InverseBSplineTransformDoubleITKv4Type>::RegisterTransform();

  itk::TransformFactory<InverseThinPlateSplineTransformFloatType>::RegisterTransform();
  itk::TransformFactory<InverseThinPlateSplineTransformDoubleType>::RegisterTransform();

  itk::TransformFactory<itk::InverseThinPlateSplineKernelTransform<float, 2>>::RegisterTransform();
  itk::TransformFactory<itk::InverseThinPlateSplineKernelTransform<double, 2>>::RegisterTransform();

  typedef itk::ThinPlateSplineKernelTransform<float, 3> ThinPlateSplineTransformFloatType;
  typedef itk::ThinPlateSplineKernelTransform<double, 3> ThinPlateSplineTransformDoubleType;

  // by default they are not registered
  itk::TransformFactory<ThinPlateSplineTransformFloatType>::RegisterTransform();
  itk::TransformFactory<ThinPlateSplineTransformDoubleType>::RegisterTransform();

  itk::TransformFactory<itk::ThinPlateSplineKernelTransform<float, 2>>::RegisterTransform();
  itk::TransformFactory<itk::ThinPlateSplineKernelTransform<double, 2>>::RegisterTransform();
}

//----------------------------------------------------------------------------
template <typename T>
itk::Matrix<T, 3, 3> vtkITKTransformConverter::Matrix2Dto3D(itk::Matrix<T, 2, 2> m2D)
{
  itk::Matrix<T, 3, 3> direction3d;
  direction3d.SetIdentity();
  for (unsigned int i = 0; i < 2; i++)
  {
    for (unsigned int j = 0; j < 2; j++)
    {
      direction3d[i][j] = m2D(i, j);
    }
  }

  // compute scale for Z direction
  itk::Vector<T, VTKDimension> directionCosine{};
  directionCosine[0] = direction3d[0][0];
  directionCosine[1] = direction3d[0][1];
  auto scaleX = directionCosine.GetNorm();
  directionCosine[0] = direction3d[1][0];
  directionCosine[1] = direction3d[1][1];
  auto scaleY = directionCosine.GetNorm();
  T scaleZ = std::sqrt(scaleX * scaleY); // geometric mean
  auto det = vnl_determinant(m2D.GetVnlMatrix());
  direction3d(2, 2) = (det < 0.0) ? -scaleZ : scaleZ;

  return direction3d;
}

//----------------------------------------------------------------------------
template <typename T>
typename itk::AffineTransform<T, 3>::Pointer vtkITKTransformConverter::ConvertITKLinearTransformFrom2Dto3D(typename itk::TransformBaseTemplate<T>::Pointer transformItk_LPS)
{
  using Affine3D = itk::AffineTransform<T, 3>;
  using LinearTransformType = itk::MatrixOffsetTransformBase<T, 2, 2>;

  typename LinearTransformType::ConstPointer t2d = dynamic_cast<const LinearTransformType*>(transformItk_LPS.GetPointer());
  if (t2d == nullptr)
  {
    throw std::runtime_error("vtkITKTransformConverter::ConvertITKLinearTransformFrom2Dto3D: Unable to cast ITK transform to 2D linear transform");
  }

  // convert into 3D affine which Slicer can handle
  typename Affine3D::MatrixType m = vtkITKTransformConverter::Matrix2Dto3D<T>(t2d->GetMatrix());
  typename Affine3D::TranslationType t;
  t.Fill(0);
  typename Affine3D::InputPointType c;
  c.Fill(0);
  for (unsigned int i = 0; i < 2; i++)
  {
    t[i] = t2d->GetTranslation()[i];
    c[i] = t2d->GetCenter()[i];
  }

  typename Affine3D::Pointer aTr = Affine3D::New();
  aTr->SetCenter(c);
  aTr->SetMatrix(m);
  aTr->SetTranslation(t);
  return aTr;
}

//----------------------------------------------------------------------------
template <typename T>
bool vtkITKTransformConverter::SetVTKLinearTransformFromITK(vtkObject* /*loggerObject*/,
                                                            vtkMatrix4x4* transformVtk_RAS,
                                                            typename itk::TransformBaseTemplate<T>::Pointer transformItk_LPS,
                                                            double center_LocalRAS[3] /*=nullptr*/)
{
  static const unsigned int D = VTKDimension;
  typedef itk::MatrixOffsetTransformBase<T, D, D> LinearTransformType;
  typedef itk::TranslationTransform<T, D> TranslateTransformType;

  vtkSmartPointer<vtkMatrix4x4> transformVtk_LPS = vtkSmartPointer<vtkMatrix4x4>::New();
  const unsigned itkDim = transformItk_LPS->GetOutputSpaceDimension();

  bool convertedToVtkMatrix = false;

  std::string itkTransformClassName = transformItk_LPS->GetNameOfClass();

  // Linear transform of doubles or floats, dimension 2 or 3

  // ITKIO transform libraries are built as shared and dynamic_cast
  // can NOT be used with templated classes that are
  // instantiated in a translation unit different than the one where they are
  // defined. It will work only if the classes are explicitly instantiated
  // and exported.
  // To workaround the issue, instead of using dynamic_cast:
  // (1) to ensure the objects are of the right type string comparison is done
  // (2) static_cast is used instead of dynamic_cast.
  // See InsightSoftwareConsortium/ITK@d1e9fe2
  // and see https://stackoverflow.com/questions/8024010/why-do-template-class-functions-have-to-be-declared-in-the-same-translation-unit
  //
  // The disadvantage of this approach is that each supported class name has to be explicitly listed here and if the class hierarchy changes in ITK
  // then the static cast may produce invalid results. Also, even if the transform class name is matching,
  // we may cast the transform to a wrong type due to mismatch in dimensions (not 3) or data type (not double or float).

  if (itkTransformClassName.find("AffineTransform") != std::string::npos || //
      itkTransformClassName == "MatrixOffsetTransformBase" ||               //
      itkTransformClassName == "Rigid3DTransform" ||                        //
      itkTransformClassName == "Euler3DTransform" ||                        //
      itkTransformClassName == "CenteredEuler3DTransform" ||                //
      itkTransformClassName == "QuaternionRigidTransform" ||                //
      itkTransformClassName == "VersorTransform" ||                         //
      itkTransformClassName == "VersorRigid3DTransform" ||                  //
      itkTransformClassName == "ScaleSkewVersor3DTransform" ||              //
      itkTransformClassName == "ScaleVersor3DTransform" ||                  //
      itkTransformClassName == "Similarity3DTransform" ||                   //
      itkTransformClassName == "ScaleTransform" ||                          //
      itkTransformClassName == "ScaleLogarithmicTransform")
  {
    typename LinearTransformType::Pointer dlt;
    if (itkDim == 2)
    {
      dlt = ConvertITKLinearTransformFrom2Dto3D<T>(transformItk_LPS);
    }
    else
    {
      dlt = static_cast<LinearTransformType*>(transformItk_LPS.GetPointer());
    }

    convertedToVtkMatrix = true;
    for (unsigned int i = 0; i < D; i++)
    {
      for (unsigned int j = 0; j < D; j++)
      {
        transformVtk_LPS->SetElement(i, j, dlt->GetMatrix()[i][j]);
      }
      transformVtk_LPS->SetElement(i, D, dlt->GetOffset()[i]);
    }

    if (center_LocalRAS)
    {
      auto center_LocalLPS = dlt->GetCenter();
      center_LocalRAS[0] = -center_LocalLPS[0];
      center_LocalRAS[1] = -center_LocalLPS[1];
      center_LocalRAS[2] = center_LocalLPS[2];
    }
  }

  // Identity transform of doubles or floats, any dimension
  if (itkTransformClassName == "IdentityTransform")
  {
    // nothing to do, matrix is already the identity
    convertedToVtkMatrix = true;
  }

  // Translate transform of doubles or floats, dimension 2 or 3
  // Separated out as it does not inherit from MatrixOffsetTransformBase
  if (itkTransformClassName == "TranslationTransform")
  {
    if (itkDim == 2)
    {
      using Translate2DTransformType = itk::TranslationTransform<T, 2>;
      typename Translate2DTransformType::Pointer dtt = static_cast<Translate2DTransformType*>(transformItk_LPS.GetPointer());
      for (unsigned int i = 0; i < 2; i++)
      {
        transformVtk_LPS->SetElement(i, D, dtt->GetOffset()[i]);
      }
      // third element remains zero
    }
    else
    {
      typename TranslateTransformType::Pointer dtt = static_cast<TranslateTransformType*>(transformItk_LPS.GetPointer());
      for (unsigned int i = 0; i < D; i++)
      {
        transformVtk_LPS->SetElement(i, D, dtt->GetOffset()[i]);
      }
    }
    convertedToVtkMatrix = true;
  }

  // Convert from LPS (ITK) to RAS (Slicer)
  //
  // Tras = lps2ras * Tlps * ras2lps
  //

  vtkSmartPointer<vtkMatrix4x4> lps2ras = vtkSmartPointer<vtkMatrix4x4>::New();
  lps2ras->SetElement(0, 0, -1);
  lps2ras->SetElement(1, 1, -1);
  vtkMatrix4x4* ras2lps = lps2ras; // lps2ras is diagonal therefore the inverse is identical

  vtkMatrix4x4::Multiply4x4(lps2ras, transformVtk_LPS, transformVtk_LPS);
  vtkMatrix4x4::Multiply4x4(transformVtk_LPS, ras2lps, transformVtk_RAS);

  if (center_LocalRAS)
  {
    for (int i = 0; i < 3; ++i)
    {
      // Apply the offset to the center of transformation to account for the expected behavior of the ITK transform center.
      // For image registration, the center of the transform is the center of the fixed image.
      // See ITK software guide section 3.6.2.
      double offset_RAS = transformVtk_RAS->GetElement(i, 3);
      center_LocalRAS[i] = center_LocalRAS[i] + offset_RAS;
    }
  }

  return convertedToVtkMatrix;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetITKLinearTransformFromVTK(vtkObject* loggerObject,
                                                            itk::Object::Pointer& transformItk_LPS,
                                                            vtkMatrix4x4* transformVtk_RAS,
                                                            double center_LocalRAS[3] /*=nullptr*/)
{
  typedef itk::AffineTransform<double, VTKDimension> AffineTransformType;

  if (transformVtk_RAS == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "vtkITKTransformConverter::SetITKLinearTransformFromVTK failed: invalid input transform");
    return false;
  }

  vtkSmartPointer<vtkMatrix4x4> lps2ras = vtkSmartPointer<vtkMatrix4x4>::New();
  lps2ras->SetElement(0, 0, -1);
  lps2ras->SetElement(1, 1, -1);
  vtkMatrix4x4* ras2lps = lps2ras; // lps2ras is diagonal therefore the inverse is identical

  // Convert from RAS (Slicer) to LPS (ITK)
  //
  // Tlps = ras2lps * Tras * lps2ras
  //
  vtkSmartPointer<vtkMatrix4x4> vtkmat = vtkSmartPointer<vtkMatrix4x4>::New();

  vtkMatrix4x4::Multiply4x4(ras2lps, transformVtk_RAS, vtkmat);
  vtkMatrix4x4::Multiply4x4(vtkmat, lps2ras, vtkmat);

  typedef AffineTransformType::MatrixType MatrixType;
  typedef AffineTransformType::OutputVectorType OffsetType;

  MatrixType itkmat;
  OffsetType itkoffset;

  for (unsigned int i = 0; i < VTKDimension; i++)
  {
    for (unsigned int j = 0; j < VTKDimension; j++)
    {
      itkmat[i][j] = vtkmat->GetElement(i, j);
    }
    itkoffset[i] = vtkmat->GetElement(i, VTKDimension);
  }

  AffineTransformType::Pointer affine = AffineTransformType::New();
  // Matrix and offset are stored last as they represent desired transform of the VTK rotation and translation.
  if (center_LocalRAS)
  {
    // For itk::AffineTransform, the center is not normally specified with matrix/offset, however it can be set explicitly before setting the matrix/offset.
    // When using matrix/offset it is only safe to explicitly set the center of transformation before setting the matrix/offset. Setting the center after
    // the offset would change the contents of the transform.
    // Translation will be recomputed from the offset to represent the specified center of the transform.
    // For more information, see the documentation of itk::AffineTransform::SetCenter.

    // Convert the center from the local RAS to the parent RAS
    double center_LocalLPS[3] = { -center_LocalRAS[0], -center_LocalRAS[1], center_LocalRAS[2] };

    // Account for the offset to the center of transformation to account for the expected behavior of the ITK transform center.
    // For image registration, the center of the transform is the center of the fixed image.
    // See ITK software guide section 3.6.2.
    vtkMath::Subtract(center_LocalLPS, itkoffset, center_LocalLPS);
    affine->SetCenter(center_LocalLPS);
  }
  affine->SetMatrix(itkmat);
  affine->SetOffset(itkoffset);

  transformItk_LPS = affine;
  return true;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::IsIdentityMatrix(vtkMatrix4x4* matrix)
{
  static double identity[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
  int i, j;

  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < 4; j++)
    {
      if (matrix->GetElement(i, j) != identity[4 * i + j])
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------
template <typename BSplineTransformType>
bool vtkITKTransformConverter::SetVTKBSplineParametersFromITKGeneric(vtkObject* loggerObject,
                                                                     vtkOrientedBSplineTransform* bsplineVtk,
                                                                     typename itk::TransformBaseTemplate<typename BSplineTransformType::ScalarType>::Pointer warpTransformItk)
{
  //
  // this version uses the itk::BSplineTransform not the itk::BSplineDeformableTransform
  //
  typedef typename BSplineTransformType::ScalarType T;
  if (bsplineVtk == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "vtkMRMLTransformStorageNode::SetVTKBSplineFromITKv4 failed: bsplineVtk is invalid");
    return false;
  }
  bool isDoublePrecisionInput = true;
  if (sizeof(T) == sizeof(float))
  {
    isDoublePrecisionInput = false;
  }
  else if (sizeof(T) == sizeof(double))
  {
    isDoublePrecisionInput = true;
  }
  else
  {
    vtkErrorWithObjectMacro(loggerObject, "Unsupported scalar type in BSpline transform file (only float and double are supported)");
    return false;
  }

  typename BSplineTransformType::Pointer bsplineItk = BSplineTransformType::New();
  std::string warpTransformItkName = warpTransformItk->GetNameOfClass();
  std::string requestedWarpTransformItkName = bsplineItk->GetNameOfClass();
  if (warpTransformItkName != requestedWarpTransformItkName)
  {
    return false;
  }
  const unsigned itkDim = warpTransformItk->GetOutputSpaceDimension();
  constexpr unsigned instDim = BSplineTransformType::SpaceDimension;
  if (itkDim != instDim)
  {
    vtkErrorWithObjectMacro(loggerObject, "Actual and instantiated dimensionality differ: actual = " << itkDim << ", instantiated = " << instDim);
    return false;
  }
  bsplineItk = static_cast<BSplineTransformType*>(warpTransformItk.GetPointer());

  // now get the fixed parameters and map them to the vtk analogs

  // it turns out that for a BSplineTransform, the TransformDomain information
  // is not populated when reading from a file, so instead we access these
  // fields from one of the coefficient images (they are assumed to be
  // identical)
  const typename BSplineTransformType::CoefficientImageArray coefficientImages = bsplineItk->GetCoefficientImages();

  // * mesh size X, Y, Z (including the BSPLINE_TRANSFORM_ORDER=3 boundary nodes,
  //   1 before and 2 after the grid)
  typename BSplineTransformType::MeshSizeType meshSize = coefficientImages[0]->GetLargestPossibleRegion().GetSize();

  // * mesh origin X, Y, Z (position of the boundary node before the grid)
  typename BSplineTransformType::OriginType origin = coefficientImages[0]->GetOrigin();

  // * mesh spacing X, Y, Z
  typename BSplineTransformType::SpacingType spacing = coefficientImages[0]->GetSpacing();

  // * mesh direction 3x3 matrix (first row, second row, third row)
  typename BSplineTransformType::DirectionType direction = coefficientImages[0]->GetDirection();

  itk::Matrix<T, 3, 3> direction3d;
  if constexpr (instDim == 2)
  {
    direction3d = Matrix2Dto3D(direction);
  }
  else
  {
    direction3d = direction;
  }

  vtkNew<vtkMatrix4x4> gridDirectionMatrix_LPS;
  for (unsigned int row = 0; row < VTKDimension; row++)
  {
    for (unsigned int column = 0; column < VTKDimension; column++)
    {
      gridDirectionMatrix_LPS->SetElement(row, column, direction3d[row][column]);
    }
  }

  // Set bspline grid and coefficients
  bsplineVtk->SetBorderModeToZero();

  vtkNew<vtkImageData> bsplineCoefficients;

  if constexpr (instDim == 2)
  {
    bsplineCoefficients->SetExtent(0, meshSize[0] - 1, 0, meshSize[1] - 1, 0, 0);
    bsplineCoefficients->SetSpacing(spacing[0], spacing[1], std::sqrt(spacing[0] * spacing[1]));
  }
  else
  {
    bsplineCoefficients->SetExtent(0, meshSize[0] - 1, 0, meshSize[1] - 1, 0, meshSize[2] - 1);
    bsplineCoefficients->SetSpacing(spacing[0], spacing[1], spacing[2]);
  }

  // convert the direction matrix from LPS (itk) to RAS (slicer)
  vtkNew<vtkMatrix4x4> lpsToRas;
  lpsToRas->SetElement(0, 0, -1);
  lpsToRas->SetElement(1, 1, -1);

  vtkNew<vtkMatrix4x4> rasToLps;
  rasToLps->SetElement(0, 0, -1);
  rasToLps->SetElement(1, 1, -1);

  vtkNew<vtkMatrix4x4> gridDirectionMatrix_RAS;
  vtkMatrix4x4::Multiply4x4(lpsToRas.GetPointer(), gridDirectionMatrix_LPS.GetPointer(), gridDirectionMatrix_RAS.GetPointer());
  bsplineVtk->SetGridDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());

  // convert the origin from LPS (itk) to RAS (slicer)
  double gridOrigin_RAS[3] = { -origin[0], -origin[1], 0 };
  if constexpr (instDim == 3)
  {
    gridOrigin_RAS[2] = origin[2];
  }
  bsplineCoefficients->SetOrigin(gridOrigin_RAS);

  int bsplineCoefficientsScalarType = VTK_FLOAT;
  if (isDoublePrecisionInput)
  {
    bsplineCoefficientsScalarType = VTK_DOUBLE;
  }

  bsplineCoefficients->AllocateScalars(bsplineCoefficientsScalarType, 3);

  const unsigned int expectedNumberOfVectors = meshSize.CalculateProductOfElements();
  const unsigned int expectedNumberOfParameters = expectedNumberOfVectors * instDim;
  const unsigned int actualNumberOfParameters = bsplineItk->GetNumberOfParameters();

  if (actualNumberOfParameters != expectedNumberOfParameters)
  {
    vtkErrorWithObjectMacro(loggerObject, "Mismatch in number of BSpline parameters in the transform file and the MRML node");
    return false;
  }
  const T* itkBSplineParams_LPS = static_cast<const T*>(bsplineItk->GetParameters().data_block());
  T* vtkBSplineParams_RAS = static_cast<T*>(bsplineCoefficients->GetScalarPointer());
  for (unsigned int i = 0; i < expectedNumberOfVectors; i++)
  {
    *(vtkBSplineParams_RAS++) = -(*(itkBSplineParams_LPS));
    *(vtkBSplineParams_RAS++) = -(*(itkBSplineParams_LPS + expectedNumberOfVectors));
    if constexpr (instDim == 3)
    {
      *(vtkBSplineParams_RAS++) = (*(itkBSplineParams_LPS + expectedNumberOfVectors * 2));
    }
    else // 2D
    {
      *(vtkBSplineParams_RAS++) = 0; // Z component
    }
    itkBSplineParams_LPS++;
  }
  bsplineVtk->SetCoefficientData(bsplineCoefficients.GetPointer());

  // Success
  return true;
}

//----------------------------------------------------------------------------
template <typename T>
bool vtkITKTransformConverter::SetVTKBSplineFromITKv3Generic(vtkObject* loggerObject,
                                                             vtkOrientedBSplineTransform* bsplineVtk,
                                                             typename itk::TransformBaseTemplate<T>::Pointer warpTransformItk,
                                                             typename itk::TransformBaseTemplate<T>::Pointer bulkTransformItk)
{
  if (bsplineVtk == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "vtkMRMLTransformStorageNode::SetVTKBSplineFromITK failed: bsplineVtk is invalid");
    return false;
  }

  const unsigned int itkDim = warpTransformItk->GetOutputSpaceDimension();
  if (itkDim < 2 || itkDim > 3)
  {
    vtkErrorWithObjectMacro(loggerObject, "Unsupported number of dimensions in BSpline transform file (expected = 2 or 3, actual = " << itkDim << ")");
    return false;
  }
  bool inverse = false;
  if (itkDim == 2)
  {
    // inverse class is derived from forward class, so it has to be checked first
    if (SetVTKBSplineParametersFromITKGeneric<itk::InverseBSplineDeformableTransform<T, 2, BSPLINE_TRANSFORM_ORDER>>(loggerObject, bsplineVtk, warpTransformItk))
    {
      inverse = true;
    }
    else if (SetVTKBSplineParametersFromITKGeneric<itk::BSplineDeformableTransform<T, 2, BSPLINE_TRANSFORM_ORDER>>(loggerObject, bsplineVtk, warpTransformItk))
    {
      inverse = false;
    }
    else
    {
      vtkDebugWithObjectMacro(loggerObject, "Not an ITKv3 BSpline transform");
      return false;
    }
  }
  else
  {
    // inverse class is derived from forward class, so it has to be checked first
    if (SetVTKBSplineParametersFromITKGeneric<itk::InverseBSplineDeformableTransform<T, 3, BSPLINE_TRANSFORM_ORDER>>(loggerObject, bsplineVtk, warpTransformItk))
    {
      inverse = true;
    }
    else if (SetVTKBSplineParametersFromITKGeneric<itk::BSplineDeformableTransform<T, 3, BSPLINE_TRANSFORM_ORDER>>(loggerObject, bsplineVtk, warpTransformItk))
    {
      inverse = false;
    }
    else
    {
      vtkDebugWithObjectMacro(loggerObject, "Not an ITKv3 BSpline transform");
      return false;
    }
  }

  // Set the bulk transform
  if (bulkTransformItk)
  {
    std::string bulkTransformItkTransformName = bulkTransformItk->GetNameOfClass();

    if (bulkTransformItkTransformName == "AffineTransform")
    {
      vtkNew<vtkMatrix4x4> bulkMatrix_RAS;
      if (!SetVTKLinearTransformFromITK<T>(loggerObject, bulkMatrix_RAS, bulkTransformItk))
      {
        vtkErrorWithObjectMacro(loggerObject, "Cannot read the bulk transform in BSplineTransform");
        return false;
      }
      bsplineVtk->SetBulkTransformMatrix(bulkMatrix_RAS.GetPointer());
    }
    else if (bulkTransformItkTransformName == "IdentityTransform")
    {
      // bulk transform is identity, which is equivalent to no bulk transform
    }
    else
    {
      vtkErrorWithObjectMacro(loggerObject, "Cannot read the 2nd transform in BSplineTransform (expected AffineTransform_double_3_3 or IdentityTransform)");
      return false;
    }
  }

  if (inverse)
  {
    bsplineVtk->Inverse();
  }

  // Success
  return true;
}

//----------------------------------------------------------------------------
template <typename T>
bool vtkITKTransformConverter::SetVTKBSplineFromITKv4Generic(vtkObject* loggerObject,
                                                             vtkOrientedBSplineTransform* bsplineVtk,
                                                             typename itk::TransformBaseTemplate<T>::Pointer warpTransformItk)
{
  bool inverse = false;
  const unsigned int itkDim = warpTransformItk->GetOutputSpaceDimension();

  if (itkDim == 2)
  {
    // inverse class is derived from forward class, so it has to be checked first
    if (SetVTKBSplineParametersFromITKGeneric<itk::InverseBSplineTransform<T, 2, BSPLINE_TRANSFORM_ORDER>>(loggerObject, bsplineVtk, warpTransformItk))
    {
      inverse = true;
    }
    else if (SetVTKBSplineParametersFromITKGeneric<itk::BSplineTransform<T, 2, BSPLINE_TRANSFORM_ORDER>>(loggerObject, bsplineVtk, warpTransformItk))
    {
      inverse = false;
    }
    else
    {
      vtkDebugWithObjectMacro(loggerObject, "Not an ITKv4 BSpline transform");
      return false;
    }
  }
  else
  {
    // inverse class is derived from forward class, so it has to be checked first
    if (SetVTKBSplineParametersFromITKGeneric<itk::InverseBSplineTransform<T, 3, BSPLINE_TRANSFORM_ORDER>>(loggerObject, bsplineVtk, warpTransformItk))
    {
      inverse = true;
    }
    else if (SetVTKBSplineParametersFromITKGeneric<itk::BSplineTransform<T, 3, BSPLINE_TRANSFORM_ORDER>>(loggerObject, bsplineVtk, warpTransformItk))
    {
      inverse = false;
    }
    else
    {
      vtkDebugWithObjectMacro(loggerObject, "Not an ITKv4 BSpline transform");
      return false;
    }
  }

  if (inverse)
  {
    bsplineVtk->Inverse();
  }
  return true;
}

//----------------------------------------------------------------------------
template <typename BSplineTransformType>
bool vtkITKTransformConverter::SetITKBSplineParametersFromVTKGeneric(
  vtkObject* loggerObject,
  typename itk::Transform<typename BSplineTransformType::ScalarType, VTKDimension, VTKDimension>::Pointer& warpTransformItk,
  vtkOrientedBSplineTransform* bsplineVtk)
{
  typedef typename BSplineTransformType::ScalarType T;
  if (bsplineVtk == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "vtkMRMLTransformStorageNode::SetITKBSplineFromVTK failed: bsplineVtk is invalid");
    return false;
  }
  vtkImageData* bsplineCoefficients_RAS = bsplineVtk->GetCoefficientData();
  if (bsplineCoefficients_RAS == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot write BSpline transform to file: coefficients are not specified");
    return false;
  }

  typename BSplineTransformType::Pointer bsplineItk = BSplineTransformType::New();
  warpTransformItk = bsplineItk;

  // Fixed parameters:
  // * mesh size X, Y, Z (including the BSPLINE_TRANSFORM_ORDER=3 boundary nodes, 1 before and 2 after the grid)
  // * mesh origin X, Y, Z (position of the boundary node before the grid)
  // * mesh spacing X, Y, Z
  // * mesh direction 3x3 matrix (first row, second row, third row)
  typename BSplineTransformType::FixedParametersType transformFixedParamsItk;
  const unsigned int numberOfFixedParameters = VTKDimension * (VTKDimension + 3);
  transformFixedParamsItk.SetSize(numberOfFixedParameters);

  int* gridExtent = bsplineCoefficients_RAS->GetExtent();
  int gridSize[3] = { gridExtent[1] - gridExtent[0] + 1, gridExtent[3] - gridExtent[2] + 1, gridExtent[5] - gridExtent[4] + 1 };
  transformFixedParamsItk[0] = gridSize[0];
  transformFixedParamsItk[1] = gridSize[1];
  transformFixedParamsItk[2] = gridSize[2];

  double* gridOrigin_RAS = bsplineCoefficients_RAS->GetOrigin();
  double gridOrigin_LPS[3] = { -gridOrigin_RAS[0], -gridOrigin_RAS[1], gridOrigin_RAS[2] };
  transformFixedParamsItk[3] = gridOrigin_LPS[0];
  transformFixedParamsItk[4] = gridOrigin_LPS[1];
  transformFixedParamsItk[5] = gridOrigin_LPS[2];

  double* gridSpacing = bsplineCoefficients_RAS->GetSpacing();
  transformFixedParamsItk[6] = gridSpacing[0];
  transformFixedParamsItk[7] = gridSpacing[1];
  transformFixedParamsItk[8] = gridSpacing[2];

  vtkNew<vtkMatrix4x4> gridDirectionMatrix_RAS;
  if (bsplineVtk->GetGridDirectionMatrix() != nullptr)
  {
    gridDirectionMatrix_RAS->DeepCopy(bsplineVtk->GetGridDirectionMatrix());
  }
  vtkNew<vtkMatrix4x4> lpsToRas;
  lpsToRas->SetElement(0, 0, -1);
  lpsToRas->SetElement(1, 1, -1);
  vtkNew<vtkMatrix4x4> rasToLps;
  rasToLps->SetElement(0, 0, -1);
  rasToLps->SetElement(1, 1, -1);
  vtkNew<vtkMatrix4x4> gridDirectionMatrix_LPS;
  vtkMatrix4x4::Multiply4x4(rasToLps.GetPointer(), gridDirectionMatrix_RAS.GetPointer(), gridDirectionMatrix_LPS.GetPointer());
  int fpIndex = 9;
  for (unsigned int row = 0; row < VTKDimension; row++)
  {
    for (unsigned int column = 0; column < VTKDimension; column++)
    {
      transformFixedParamsItk[fpIndex++] = gridDirectionMatrix_LPS->GetElement(row, column);
    }
  }

  bsplineItk->SetFixedParameters(transformFixedParamsItk);

  // BSpline coefficients

  const unsigned int expectedNumberOfVectors = gridSize[0] * gridSize[1] * gridSize[2];
  const unsigned int expectedNumberOfParameters = expectedNumberOfVectors * VTKDimension;
  if (bsplineItk->GetNumberOfParameters() != expectedNumberOfParameters)
  {
    vtkErrorWithObjectMacro(loggerObject, "Mismatch in number of BSpline parameters in the ITK transform and the VTK transform");
    return false;
  }

  typename BSplineTransformType::ParametersType transformParamsItk(expectedNumberOfParameters);
  T* itkBSplineParams_LPS = static_cast<T*>(transformParamsItk.data_block());
  T* vtkBSplineParams_RAS = static_cast<T*>(bsplineCoefficients_RAS->GetScalarPointer());
  double coefficientScale = bsplineVtk->GetDisplacementScale();
  for (unsigned int i = 0; i < expectedNumberOfVectors; i++)
  {
    *(itkBSplineParams_LPS) = -coefficientScale * (*(vtkBSplineParams_RAS++));
    *(itkBSplineParams_LPS + expectedNumberOfVectors) = -coefficientScale * (*(vtkBSplineParams_RAS++));
    *(itkBSplineParams_LPS + expectedNumberOfVectors * 2) = coefficientScale * (*(vtkBSplineParams_RAS++));
    itkBSplineParams_LPS++;
  }

  bsplineItk->SetParameters(transformParamsItk);
  return true;
}

//----------------------------------------------------------------------------
template <typename T>
bool vtkITKTransformConverter::SetITKv3BSplineFromVTKGeneric(vtkObject* loggerObject,
                                                             typename itk::Transform<T, VTKDimension, VTKDimension>::Pointer& warpTransformItk,
                                                             typename itk::Transform<T, VTKDimension, VTKDimension>::Pointer& bulkTransformItk,
                                                             vtkOrientedBSplineTransform* bsplineVtk,
                                                             bool alwaysAddBulkTransform)
{
  if (bsplineVtk == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "vtkMRMLTransformStorageNode::SetITKBSplineFromVTK failed: bsplineVtk is invalid");
    return false;
  }
  // Update is needed because it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
  bsplineVtk->Update();
  bool itkTransformSetSuccessfully = false;
  if (bsplineVtk->GetInverseFlag())
  {
    itkTransformSetSuccessfully =
      SetITKBSplineParametersFromVTKGeneric<itk::InverseBSplineDeformableTransform<T, VTKDimension, VTKDimension>>(loggerObject, warpTransformItk, bsplineVtk);
  }
  else
  {
    itkTransformSetSuccessfully = SetITKBSplineParametersFromVTKGeneric<itk::BSplineDeformableTransform<T, VTKDimension, VTKDimension>>(loggerObject, warpTransformItk, bsplineVtk);
  }
  if (!itkTransformSetSuccessfully)
  {
    vtkErrorWithObjectMacro(loggerObject, "vtkMRMLTransformStorageNode::SetITKBSplineFromVTK failed: cannot determine BSpline parameters");
    return false;
  }

  vtkMatrix4x4* bulkMatrix_RAS = bsplineVtk->GetBulkTransformMatrix();
  if (bulkMatrix_RAS || alwaysAddBulkTransform)
  {
    vtkNew<vtkMatrix4x4> lpsToRas;
    lpsToRas->SetElement(0, 0, -1);
    lpsToRas->SetElement(1, 1, -1);
    vtkNew<vtkMatrix4x4> rasToLps;
    rasToLps->SetElement(0, 0, -1);
    rasToLps->SetElement(1, 1, -1);
    vtkNew<vtkMatrix4x4> bulkMatrix_LPS; // bulk_LPS = rasToLps * bulk_RAS * lpsToRas
    // If bulk transform is available then use it, otherwise just write an identity matrix (we just write it because
    // alwaysAddBulkTransform was requested, due to backward compatibility reasons)
    if (bulkMatrix_RAS != nullptr)
    {
      vtkMatrix4x4::Multiply4x4(rasToLps.GetPointer(), bulkMatrix_RAS, bulkMatrix_LPS.GetPointer());
      vtkMatrix4x4::Multiply4x4(bulkMatrix_LPS.GetPointer(), lpsToRas.GetPointer(), bulkMatrix_LPS.GetPointer());
    }
    typedef itk::AffineTransform<T, VTKDimension> BulkTransformType;
    typename BulkTransformType::Pointer affineItk = BulkTransformType::New();
    bulkTransformItk = affineItk;

    typename BulkTransformType::MatrixType affineMatrix;
    typename BulkTransformType::OffsetType affineOffset;
    for (unsigned int i = 0; i < VTKDimension; i++)
    {
      for (unsigned int j = 0; j < VTKDimension; j++)
      {
        affineMatrix[i][j] = bulkMatrix_LPS->GetElement(i, j);
      }
      affineOffset[i] = bulkMatrix_LPS->GetElement(i, VTKDimension);
    }

    affineItk->SetMatrix(affineMatrix);
    affineItk->SetOffset(affineOffset);
  }
  else
  {
    bulkTransformItk = nullptr;
  }

  return true;
}

//----------------------------------------------------------------------------
template <typename T>
bool vtkITKTransformConverter::SetITKv4BSplineFromVTKGeneric(vtkObject* loggerObject,
                                                             typename itk::Transform<T, VTKDimension, VTKDimension>::Pointer& warpTransformItk,
                                                             vtkOrientedBSplineTransform* bsplineVtk)
{
  // Update is needed because it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
  bsplineVtk->Update();
  bool itkTransformSetSuccessfully = false;
  if (bsplineVtk->GetInverseFlag())
  {
    itkTransformSetSuccessfully = SetITKBSplineParametersFromVTKGeneric<itk::InverseBSplineTransform<T, VTKDimension, VTKDimension>>(loggerObject, warpTransformItk, bsplineVtk);
  }
  else
  {
    itkTransformSetSuccessfully = SetITKBSplineParametersFromVTKGeneric<itk::BSplineTransform<T, VTKDimension, VTKDimension>>(loggerObject, warpTransformItk, bsplineVtk);
  }
  if (!itkTransformSetSuccessfully)
  {
    vtkErrorWithObjectMacro(loggerObject, "vtkMRMLTransformStorageNode::SetITKv4BSplineFromVTKGeneric failed: cannot determine BSpline parameters");
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetITKv3BSplineFromVTK(vtkObject* loggerObject,
                                                      itk::Object::Pointer& warpTransformItk,
                                                      itk::Object::Pointer& bulkTransformItk,
                                                      vtkOrientedBSplineTransform* bsplineVtk,
                                                      bool alwaysAddBulkTransform)
{
  if (bsplineVtk == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot retrieve BSpline transform from node");
    return false;
  }

  vtkImageData* bsplineCoefficients = bsplineVtk->GetCoefficientData();

  if (bsplineCoefficients == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot write BSpline transform to file: coefficients are not specified");
    return false;
  }

  if (bsplineCoefficients->GetScalarType() == VTK_FLOAT)
  {
    typedef itk::Transform<float, VTKDimension, VTKDimension> ITKTransformType;
    ITKTransformType::Pointer floatWarpTransformItk;
    ITKTransformType::Pointer floatBulkTransformItk;
    if (!SetITKv3BSplineFromVTKGeneric<float>(loggerObject, floatWarpTransformItk, floatBulkTransformItk, bsplineVtk, alwaysAddBulkTransform))
    {
      vtkErrorWithObjectMacro(loggerObject, "Cannot write BSpline transform to file");
      return false;
    }
    warpTransformItk = floatWarpTransformItk.GetPointer();
    bulkTransformItk = floatBulkTransformItk.GetPointer();
  }
  else if (bsplineCoefficients->GetScalarType() == VTK_DOUBLE)
  {
    typedef itk::Transform<double, VTKDimension, VTKDimension> ITKTransformType;
    ITKTransformType::Pointer doubleWarpTransformItk;
    ITKTransformType::Pointer doubleBulkTransformItk;
    if (!SetITKv3BSplineFromVTKGeneric<double>(loggerObject, doubleWarpTransformItk, doubleBulkTransformItk, bsplineVtk, alwaysAddBulkTransform))
    {
      vtkErrorWithObjectMacro(loggerObject, "Cannot write BSpline transform to file");
      return false;
    }
    warpTransformItk = doubleWarpTransformItk;
    bulkTransformItk = doubleBulkTransformItk;
  }
  else
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot write BSpline transform to file: only float and double coefficient types are supported");
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetITKv4BSplineFromVTK(vtkObject* loggerObject, itk::Object::Pointer& warpTransformItk, vtkOrientedBSplineTransform* bsplineVtk)
{
  if (bsplineVtk == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot retrieve BSpline transform from node");
    return false;
  }

  vtkImageData* bsplineCoefficients = bsplineVtk->GetCoefficientData();

  if (bsplineCoefficients == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot write BSpline transform to file: coefficients are not specified");
    return false;
  }

  if (bsplineCoefficients->GetScalarType() == VTK_FLOAT)
  {
    typedef itk::Transform<float, VTKDimension, VTKDimension> ITKTransformType;
    ITKTransformType::Pointer floatWarpTransformItk;
    if (!SetITKv4BSplineFromVTKGeneric<float>(loggerObject, floatWarpTransformItk, bsplineVtk))
    {
      vtkErrorWithObjectMacro(loggerObject, "Cannot write BSpline transform to file");
      return false;
    }
    warpTransformItk = floatWarpTransformItk.GetPointer();
  }
  else if (bsplineCoefficients->GetScalarType() == VTK_DOUBLE)
  {
    typedef itk::Transform<double, VTKDimension, VTKDimension> ITKTransformType;
    ITKTransformType::Pointer doubleWarpTransformItk;
    if (!SetITKv4BSplineFromVTKGeneric<double>(loggerObject, doubleWarpTransformItk, bsplineVtk))
    {
      vtkErrorWithObjectMacro(loggerObject, "Cannot write BSpline transform to file");
      return false;
    }
    warpTransformItk = doubleWarpTransformItk;
  }
  else
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot write BSpline transform to file: only float and double coefficient types are supported");
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
template <typename T, unsigned Dimension>
bool vtkITKTransformConverter::SetVTKOrientedGridTransformFromITK(vtkObject* loggerObject,
                                                                  vtkOrientedGridTransform* transformVtk_RAS,
                                                                  typename itk::TransformBaseTemplate<T>::Pointer transformItk_LPS)
{
  typedef itk::DisplacementFieldTransform<T, Dimension> DisplacementFieldTransformType;
  typedef itk::InverseDisplacementFieldTransform<T, Dimension> InverseDisplacementFieldTransformType;

  std::string transformItkClassName = transformItk_LPS->GetNameOfClass();

  bool inverse = false;
  typename DisplacementFieldTransformType::DisplacementFieldType* gridImageItk_Lps = nullptr;
  if (transformItkClassName == "InverseDisplacementFieldTransform") // inverse class is derived from forward class, so it has to be checked first
  {
    DisplacementFieldTransformType* inverseDisplacementFieldTransform = static_cast<InverseDisplacementFieldTransformType*>(transformItk_LPS.GetPointer());
    inverse = true;
    gridImageItk_Lps = inverseDisplacementFieldTransform->GetDisplacementField();
  }
  else if (transformItkClassName == "DisplacementFieldTransform")
  {
    DisplacementFieldTransformType* displacementFieldTransform = static_cast<DisplacementFieldTransformType*>(transformItk_LPS.GetPointer());
    inverse = false;
    gridImageItk_Lps = displacementFieldTransform->GetDisplacementField();
  }
  else
  {
    vtkDebugWithObjectMacro(loggerObject, "Not a grid transform");
    return false;
  }
  if constexpr (Dimension != VTKDimension)
  {
    if (!SetVTKOrientedGridTransformFrom2DITKImage<T>(loggerObject, transformVtk_RAS, gridImageItk_Lps))
    {
      return false;
    }
  }
  else
  {
    if (!SetVTKOrientedGridTransformFromITKImage<T>(loggerObject, transformVtk_RAS, gridImageItk_Lps))
    {
      return false;
    }
  }
  if (inverse)
  {
    transformVtk_RAS->Inverse();
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetITKOrientedGridTransformFromVTK(vtkObject* loggerObject, itk::Object::Pointer& transformItk_LPS, vtkOrientedGridTransform* transformVtk_RAS)
{
  GridImageDoubleType::Pointer gridImageItk_Lps;
  if (!SetITKImageFromVTKOrientedGridTransform(loggerObject, gridImageItk_Lps, transformVtk_RAS))
  {
    vtkErrorWithObjectMacro(loggerObject, "vtkITKTransformConverter::SetITKOrientedGridTransformFromVTK failed: input transform is invalid");
    return false;
  }
  // Update is needed because it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
  transformVtk_RAS->Update();
  if (transformVtk_RAS->GetInverseFlag())
  {
    InverseDisplacementFieldTransformDoubleType::Pointer gridTransformItk = InverseDisplacementFieldTransformDoubleType::New();
    gridTransformItk->SetDisplacementField(gridImageItk_Lps);
    transformItk_LPS = gridTransformItk;
  }
  else
  {
    DisplacementFieldTransformDoubleType::Pointer gridTransformItk = DisplacementFieldTransformDoubleType::New();
    gridTransformItk->SetDisplacementField(gridImageItk_Lps);
    transformItk_LPS = gridTransformItk;
  }
  return true;
}

//----------------------------------------------------------------------------
template <typename T>
bool vtkITKTransformConverter::SetVTKOrientedGridTransformFromITKImage(vtkObject* loggerObject,
                                                                       vtkOrientedGridTransform* grid_Ras,
                                                                       typename itk::DisplacementFieldTransform<T, 3>::DisplacementFieldType::Pointer gridImage_Lps)
{
  typedef itk::DisplacementFieldTransform<T, 3> DisplacementFieldTransformType;
  typedef typename DisplacementFieldTransformType::DisplacementFieldType GridImageType;

  vtkNew<vtkImageData> gridImage_Ras;

  // Origin
  gridImage_Ras->SetOrigin(-gridImage_Lps->GetOrigin()[0], -gridImage_Lps->GetOrigin()[1], gridImage_Lps->GetOrigin()[2]);

  // Spacing
  gridImage_Ras->SetSpacing(gridImage_Lps->GetSpacing()[0], gridImage_Lps->GetSpacing()[1], gridImage_Lps->GetSpacing()[2]);

  // Direction
  vtkNew<vtkMatrix4x4> gridDirectionMatrix_LPS;
  for (unsigned int row = 0; row < VTKDimension; row++)
  {
    for (unsigned int column = 0; column < VTKDimension; column++)
    {
      gridDirectionMatrix_LPS->SetElement(row, column, gridImage_Lps->GetDirection()(row, column));
    }
  }
  vtkNew<vtkMatrix4x4> lpsToRas;
  lpsToRas->SetElement(0, 0, -1);
  lpsToRas->SetElement(1, 1, -1);
  vtkNew<vtkMatrix4x4> gridDirectionMatrix_RAS;
  vtkMatrix4x4::Multiply4x4(lpsToRas.GetPointer(), gridDirectionMatrix_LPS.GetPointer(), gridDirectionMatrix_RAS.GetPointer());
  grid_Ras->SetGridDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());

  // Vectors
  typename GridImageType::SizeType size = gridImage_Lps->GetBufferedRegion().GetSize();
  gridImage_Ras->SetDimensions(size[0], size[1], size[2]);
  unsigned int numberOfScalarComponents = GridImageType::PixelType::Dimension;
  if (numberOfScalarComponents != VTKDimension)
  {
    vtkErrorWithObjectMacro(loggerObject,
                            "Cannot load grid transform: the input displacement field expected to contain " << VTKDimension << " components but it actually contains "
                                                                                                            << numberOfScalarComponents);
    return false;
  }
  gridImage_Ras->AllocateScalars(VTK_DOUBLE, 3);

  double* displacementVectors_Ras = reinterpret_cast<double*>(gridImage_Ras->GetScalarPointer());
  itk::ImageRegionConstIterator<GridImageType> inputIt(gridImage_Lps, gridImage_Lps->GetRequestedRegion());
  while (!inputIt.IsAtEnd())
  {
    typename GridImageType::PixelType displacementVectorLps = inputIt.Get();
    *(displacementVectors_Ras++) = -displacementVectorLps[0];
    *(displacementVectors_Ras++) = -displacementVectorLps[1];
    *(displacementVectors_Ras++) = displacementVectorLps[2];
    ++inputIt;
  }

  grid_Ras->SetDisplacementGridData(gridImage_Ras.GetPointer());

  // Set the interpolation to cubic to have smooth derivatives
  grid_Ras->SetInterpolationModeToCubic();

  return true;
}

//----------------------------------------------------------------------------
template <typename T>
bool vtkITKTransformConverter::SetVTKOrientedGridTransformFrom2DITKImage(vtkObject* loggerObject,
                                                                         vtkOrientedGridTransform* grid_Ras,
                                                                         typename itk::DisplacementFieldTransform<T, 2>::DisplacementFieldType::Pointer gridImage_Lps)
{
  typedef itk::DisplacementFieldTransform<T, 2> DisplacementFieldTransformType;
  typedef typename DisplacementFieldTransformType::DisplacementFieldType GridImageType;

  vtkNew<vtkImageData> gridImage_Ras;

  // Origin (convert LPS -> RAS for X and Y, set Z to 0)
  const auto& itkOrigin = gridImage_Lps->GetOrigin();
  double originRasX = -itkOrigin[0];
  double originRasY = -itkOrigin[1];
  double originRasZ = 0.0;
  gridImage_Ras->SetOrigin(originRasX, originRasY, originRasZ);

  // Spacing (X,Y from ITK, Z=geometric mean)
  const auto& itkSpacing = gridImage_Lps->GetSpacing();
  gridImage_Ras->SetSpacing(itkSpacing[0], itkSpacing[1], std::sqrt(itkSpacing[0] * itkSpacing[1]));

  itk::Matrix<itk::SpacePrecisionType, 3, 3> direction3d = vtkITKTransformConverter::Matrix2Dto3D(gridImage_Lps->GetDirection());

  vtkNew<vtkMatrix4x4> gridDirectionMatrix_LPS;
  for (unsigned int row = 0; row < 3; ++row)
  {
    for (unsigned int col = 0; col < 3; ++col)
    {
      gridDirectionMatrix_LPS->SetElement(row, col, direction3d(row, col));
    }
  }

  // Convert direction from LPS to RAS
  vtkNew<vtkMatrix4x4> lpsToRas;
  lpsToRas->SetElement(0, 0, -1);
  lpsToRas->SetElement(1, 1, -1);
  vtkNew<vtkMatrix4x4> gridDirectionMatrix_RAS;
  vtkMatrix4x4::Multiply4x4(lpsToRas.GetPointer(), gridDirectionMatrix_LPS.GetPointer(), gridDirectionMatrix_RAS.GetPointer());
  grid_Ras->SetGridDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());

  // Expand from 2->3 components
  typename GridImageType::SizeType size = gridImage_Lps->GetBufferedRegion().GetSize();
  int dimX = static_cast<int>(size[0]);
  int dimY = static_cast<int>(size[1]);
  gridImage_Ras->SetDimensions(dimX, dimY, 1);

  unsigned int numberOfScalarComponents = GridImageType::PixelType::Dimension;
  if (numberOfScalarComponents != 2)
  {
    vtkErrorWithObjectMacro(loggerObject,
                            "Cannot load 2D grid transform: the input displacement field expected to contain 2 components but it actually contains " << numberOfScalarComponents);
    return false;
  }

  if constexpr (std::is_same_v<T, float>)
  {
    gridImage_Ras->AllocateScalars(VTK_FLOAT, 3);
  }
  else if constexpr (std::is_same_v<T, double>)
  {
    gridImage_Ras->AllocateScalars(VTK_DOUBLE, 3);
  }
  else
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot load 2D grid transform: unsupported pixel type. Only float and double are supported.");
    return false;
  }

  // Vectors: copy displacement vectors, converting LPS -> RAS and expand from 2->3 components
  T* displacementVectors_Ras = reinterpret_cast<T*>(gridImage_Ras->GetScalarPointer());
  itk::ImageRegionConstIterator<GridImageType> inputIt(gridImage_Lps, gridImage_Lps->GetRequestedRegion());
  while (!inputIt.IsAtEnd())
  {
    typename GridImageType::PixelType displacementVectorLps = inputIt.Get();
    // Convert LPS -> RAS for X and Y, Z is zero for 2D
    *(displacementVectors_Ras++) = -static_cast<T>(displacementVectorLps[0]);
    *(displacementVectors_Ras++) = -static_cast<T>(displacementVectorLps[1]);
    *(displacementVectors_Ras++) = 0.0;
    ++inputIt;
  }

  grid_Ras->SetDisplacementGridData(gridImage_Ras.GetPointer());

  // Use cubic interpolation for smooth derivatives
  grid_Ras->SetInterpolationModeToCubic();

  return true;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetITKImageFromVTKOrientedGridTransform(vtkObject* loggerObject, GridImageDoubleType::Pointer& gridImage_Lps, vtkOrientedGridTransform* grid_Ras)
{
  if (grid_Ras == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot save grid transform: the input vtkOrientedGridTransform is invalid");
    return false;
  }

  // Update is needed because DisplacementGrid may be out-of-date if the transform depends on its inverse
  grid_Ras->Update();

  vtkImageData* gridImage_Ras = grid_Ras->GetDisplacementGrid();
  if (gridImage_Ras == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot save grid transform: the input vtkOrientedGridTransform does not contain a valid displacement grid");
    return false;
  }
  if (gridImage_Ras->GetNumberOfScalarComponents() != static_cast<int>(VTKDimension))
  {
    vtkErrorWithObjectMacro(loggerObject,
                            "Cannot save grid transform: the input vtkOrientedGridTransform expected to contain " << VTKDimension << " components but it actually contains "
                                                                                                                  << gridImage_Ras->GetNumberOfScalarComponents());
    return false;
  }

  gridImage_Lps = GridImageDoubleType::New();

  // Origin
  double* origin_Ras = gridImage_Ras->GetOrigin();
  double origin_Lps[3] = { -origin_Ras[0], -origin_Ras[1], origin_Ras[2] };
  gridImage_Lps->SetOrigin(origin_Lps);

  // Spacing
  double* spacing = gridImage_Ras->GetSpacing();
  // GridType::SpacingType spacing( spacing );
  gridImage_Lps->SetSpacing(spacing);

  // Direction
  vtkNew<vtkMatrix4x4> gridDirectionMatrix_Ras;
  if (grid_Ras->GetGridDirectionMatrix() != nullptr)
  {
    gridDirectionMatrix_Ras->DeepCopy(grid_Ras->GetGridDirectionMatrix());
  }
  vtkNew<vtkMatrix4x4> rasToLps;
  rasToLps->SetElement(0, 0, -1);
  rasToLps->SetElement(1, 1, -1);
  vtkNew<vtkMatrix4x4> gridDirectionMatrix_Lps;
  vtkMatrix4x4::Multiply4x4(rasToLps.GetPointer(), gridDirectionMatrix_Ras.GetPointer(), gridDirectionMatrix_Lps.GetPointer());
  GridImageDoubleType::DirectionType gridDirectionMatrixItk_Lps;
  for (unsigned int row = 0; row < VTKDimension; row++)
  {
    for (unsigned int column = 0; column < VTKDimension; column++)
    {
      gridDirectionMatrixItk_Lps(row, column) = gridDirectionMatrix_Lps->GetElement(row, column);
    }
  }
  gridImage_Lps->SetDirection(gridDirectionMatrixItk_Lps);

  // Vectors
  GridImageDoubleType::IndexType start;
  start[0] = start[1] = start[2] = 0;
  int* Nijk = gridImage_Ras->GetDimensions();
  GridImageDoubleType::SizeType size;
  size[0] = Nijk[0];
  size[1] = Nijk[1];
  size[2] = Nijk[2];
  GridImageDoubleType::RegionType region;
  region.SetSize(size);
  region.SetIndex(start);
  gridImage_Lps->SetRegions(region);
  gridImage_Lps->Allocate();
  itk::ImageRegionIterator<GridImageDoubleType> gridImageIt_Lps(gridImage_Lps, region);
  gridImageIt_Lps.GoToBegin();
  GridImageDoubleType::PixelType displacementVectorLps;
  double displacementScale = grid_Ras->GetDisplacementScale();
  double displacementShift = grid_Ras->GetDisplacementShift();

  if (gridImage_Ras->GetScalarType() == VTK_DOUBLE)
  {
    double* displacementVectors_Ras = reinterpret_cast<double*>(gridImage_Ras->GetScalarPointer());
    while (!gridImageIt_Lps.IsAtEnd())
    {
      displacementVectorLps[0] = -(displacementScale * (*(displacementVectors_Ras++)) + displacementShift);
      displacementVectorLps[1] = -(displacementScale * (*(displacementVectors_Ras++)) + displacementShift);
      displacementVectorLps[2] = (displacementScale * (*(displacementVectors_Ras++)) + displacementShift);
      gridImageIt_Lps.Set(displacementVectorLps);
      ++gridImageIt_Lps;
    }
  }
  else if (gridImage_Ras->GetScalarType() == VTK_FLOAT)
  {
    float* displacementVectors_Ras = reinterpret_cast<float*>(gridImage_Ras->GetScalarPointer());
    while (!gridImageIt_Lps.IsAtEnd())
    {
      displacementVectorLps[0] = -(displacementScale * (*(displacementVectors_Ras++)) + displacementShift);
      displacementVectorLps[1] = -(displacementScale * (*(displacementVectors_Ras++)) + displacementShift);
      displacementVectorLps[2] = (displacementScale * (*(displacementVectors_Ras++)) + displacementShift);
      gridImageIt_Lps.Set(displacementVectorLps);
      ++gridImageIt_Lps;
    }
  }
  else
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot save grid transform: only float and double scalar types are supported");
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
template <typename T, unsigned VDimension>
bool vtkITKTransformConverter::SetVTKThinPlateSplineTransformFromITK(vtkObject* loggerObject,
                                                                     vtkThinPlateSplineTransform* transformVtk_RAS,
                                                                     typename itk::TransformBaseTemplate<T>::Pointer transformItk_LPS)
{
  typedef itk::ThinPlateSplineKernelTransform<T, VDimension> ThinPlateSplineTransformType;
  typedef itk::InverseThinPlateSplineKernelTransform<T, VDimension> InverseThinPlateSplineTransformType;

  std::string transformItkClassName = transformItk_LPS->GetNameOfClass();

  bool inverse = false;
  typename ThinPlateSplineTransformType::PointSetType::Pointer sourceLandmarksItk_Lps;
  typename ThinPlateSplineTransformType::PointSetType::Pointer targetLandmarksItk_Lps;
  if (transformItkClassName == "InverseThinPlateSplineKernelTransform") // inverse class is derived from forward class, so it has to be checked first
  {
    ThinPlateSplineTransformType* inverseTpsTransform = static_cast<InverseThinPlateSplineTransformType*>(transformItk_LPS.GetPointer());
    inverse = true;
    sourceLandmarksItk_Lps = inverseTpsTransform->GetSourceLandmarks();
    targetLandmarksItk_Lps = inverseTpsTransform->GetTargetLandmarks();
  }
  else if (transformItkClassName == "ThinPlateSplineKernelTransform")
  {
    ThinPlateSplineTransformType* tpsTransform = static_cast<ThinPlateSplineTransformType*>(transformItk_LPS.GetPointer());
    inverse = false;
    sourceLandmarksItk_Lps = tpsTransform->GetSourceLandmarks();
    targetLandmarksItk_Lps = tpsTransform->GetTargetLandmarks();
  }
  else
  {
    vtkDebugWithObjectMacro(loggerObject, "Not a ThinPlateSpline transform");
    return false;
  }

  vtkNew<vtkPoints> sourceLandmarksVtk_Ras;
  unsigned int numberOfSourceLandmarks = sourceLandmarksItk_Lps->GetNumberOfPoints();
  vtkNew<vtkPoints> targetLandmarksVtk_Ras;
  unsigned int numberOfTargetLandmarks = targetLandmarksItk_Lps->GetNumberOfPoints();
  if constexpr (std::is_same_v<T, float>)
  {
    sourceLandmarksVtk_Ras->SetDataTypeToFloat();
    sourceLandmarksVtk_Ras->Allocate(numberOfSourceLandmarks);

    targetLandmarksVtk_Ras->SetDataTypeToFloat();
    targetLandmarksVtk_Ras->Allocate(numberOfTargetLandmarks);
  }
  else if constexpr (std::is_same_v<T, double>)
  {
    sourceLandmarksVtk_Ras->SetDataTypeToDouble();
    sourceLandmarksVtk_Ras->Allocate(numberOfSourceLandmarks);

    targetLandmarksVtk_Ras->SetDataTypeToDouble();
    targetLandmarksVtk_Ras->Allocate(numberOfTargetLandmarks);
  }
  else
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot load 2D TPS transform: unsupported pixel type. Only float and double are supported.");
    return false;
  }

  for (unsigned int i = 0; i < numberOfSourceLandmarks; i++)
  {
    typename ThinPlateSplineTransformType::InputPointType pointItk_Lps;
    bool pointExists = sourceLandmarksItk_Lps->GetPoint(i, &pointItk_Lps);
    if (!pointExists)
    {
      continue;
    }
    T pointVtk_Ras[3] = { 0 };
    pointVtk_Ras[0] = -pointItk_Lps[0];
    pointVtk_Ras[1] = -pointItk_Lps[1];
    if constexpr (VDimension == 2)
    {
      pointVtk_Ras[2] = 0.0;
    }
    else
    {
      pointVtk_Ras[2] = pointItk_Lps[2];
    }
    sourceLandmarksVtk_Ras->InsertNextPoint(pointVtk_Ras);
  }

  for (unsigned int i = 0; i < numberOfTargetLandmarks; i++)
  {
    typename ThinPlateSplineTransformType::InputPointType pointItk_Lps;
    bool pointExists = targetLandmarksItk_Lps->GetPoint(i, &pointItk_Lps);
    if (!pointExists)
    {
      continue;
    }
    T pointVtk_Ras[3] = { 0 };
    pointVtk_Ras[0] = -pointItk_Lps[0];
    pointVtk_Ras[1] = -pointItk_Lps[1];
    if constexpr (VDimension == 2)
    {
      pointVtk_Ras[2] = 0.0;
    }
    else
    {
      pointVtk_Ras[2] = pointItk_Lps[2];
    }
    targetLandmarksVtk_Ras->InsertNextPoint(pointVtk_Ras);
  }

  transformVtk_RAS->SetBasisToR(); // need to set this because data is 3D
  transformVtk_RAS->SetSourceLandmarks(sourceLandmarksVtk_Ras.GetPointer());
  transformVtk_RAS->SetTargetLandmarks(targetLandmarksVtk_Ras.GetPointer());

  if (inverse)
  {
    transformVtk_RAS->Inverse();
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetITKThinPlateSplineTransformFromVTK(vtkObject* loggerObject,
                                                                     itk::Object::Pointer& transformItk_LPS,
                                                                     vtkThinPlateSplineTransform* transformVtk_RAS,
                                                                     bool initialize /*= true*/)
{
  if (transformVtk_RAS == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "Cannot set ITK thin-plate spline transform from VTK: the input vtkThinPlateSplineTransform is invalid");
    return false;
  }

  // Update is needed because the Basis value and Inverse flag may be out-of-date if the transform depends on its inverse
  transformVtk_RAS->Update();

  if (transformVtk_RAS->GetBasis() != VTK_RBF_R)
  {
    vtkErrorWithObjectMacro(loggerObject,
                            "Cannot set ITK thin-plate spline transform from VTK: basis function must be R."
                            " Call SetBasisToR() method of the vtkThinPlateSplineTransform object before attempting to write it to file.");
    return false;
  }

  ThinPlateSplineTransformDoubleType::PointSetType::Pointer sourceLandmarksItk_Lps = ThinPlateSplineTransformDoubleType::PointSetType::New();
  vtkPoints* sourceLandmarksVtk_Ras = transformVtk_RAS->GetSourceLandmarks();
  if (sourceLandmarksVtk_Ras != nullptr)
  {
    for (int i = 0; i < sourceLandmarksVtk_Ras->GetNumberOfPoints(); i++)
    {
      double posVtk_Ras[3] = { 0 };
      sourceLandmarksVtk_Ras->GetPoint(i, posVtk_Ras);
      ThinPlateSplineTransformDoubleType::InputPointType posItk_Lps;
      posItk_Lps[0] = -posVtk_Ras[0];
      posItk_Lps[1] = -posVtk_Ras[1];
      posItk_Lps[2] = posVtk_Ras[2];
      sourceLandmarksItk_Lps->GetPoints()->InsertElement(i, posItk_Lps);
    }
  }
  ThinPlateSplineTransformDoubleType::PointSetType::Pointer targetLandmarksItk_Lps = ThinPlateSplineTransformDoubleType::PointSetType::New();
  vtkPoints* targetLandmarksVtk_Ras = transformVtk_RAS->GetTargetLandmarks();
  if (targetLandmarksVtk_Ras != nullptr)
  {
    for (int i = 0; i < targetLandmarksVtk_Ras->GetNumberOfPoints(); i++)
    {
      double posVtk_Ras[3] = { 0 };
      targetLandmarksVtk_Ras->GetPoint(i, posVtk_Ras);
      ThinPlateSplineTransformDoubleType::InputPointType posItk_Lps;
      posItk_Lps[0] = -posVtk_Ras[0];
      posItk_Lps[1] = -posVtk_Ras[1];
      posItk_Lps[2] = posVtk_Ras[2];
      targetLandmarksItk_Lps->GetPoints()->InsertElement(i, posItk_Lps);
    }
  }

  if (transformVtk_RAS->GetInverseFlag())
  {
    InverseThinPlateSplineTransformDoubleType::Pointer tpsTransformItk = InverseThinPlateSplineTransformDoubleType::New();
    tpsTransformItk->SetSourceLandmarks(sourceLandmarksItk_Lps);
    tpsTransformItk->SetTargetLandmarks(targetLandmarksItk_Lps);
    if (initialize)
    {
      tpsTransformItk->ComputeWMatrix();
    }
    transformItk_LPS = tpsTransformItk;
  }
  else
  {
    ThinPlateSplineTransformDoubleType::Pointer tpsTransformItk = ThinPlateSplineTransformDoubleType::New();
    tpsTransformItk->SetSourceLandmarks(sourceLandmarksItk_Lps);
    tpsTransformItk->SetTargetLandmarks(targetLandmarksItk_Lps);
    if (initialize)
    {
      tpsTransformItk->ComputeWMatrix();
    }
    transformItk_LPS = tpsTransformItk;
  }
  return true;
}

//----------------------------------------------------------------------------
template <typename T>
vtkAbstractTransform* vtkITKTransformConverter::CreateVTKTransformFromITK(vtkObject* loggerObject,
                                                                          typename itk::TransformBaseTemplate<T>::Pointer transformItk,
                                                                          double center_LocalRAS[3] /*=nullptr*/)
{
  bool conversionSuccess = false;

  // Perform these sanity checks early
  if (!transformItk)
  {
    vtkErrorWithObjectMacro(loggerObject, "CreateVTKTransformFromITK failed: nullptr provided");
    return nullptr;
  }
  const unsigned itkDim = transformItk->GetOutputSpaceDimension();
  if (itkDim != transformItk->GetInputSpaceDimension())
  {
    vtkErrorWithObjectMacro(loggerObject,
                            "We do not support transforms with different number of input and output dimensions, " << "input dimension = " << transformItk->GetInputSpaceDimension()
                                                                                                                  << ", output dimension = " << itkDim << ")");
    return nullptr;
  }
  if (itkDim < 2 || itkDim > 3) // do we need to support 4D transforms?
  {
    vtkErrorWithObjectMacro(loggerObject, "Unsupported transform dimension: " << itkDim);
    return nullptr;
  }

  std::string itkTransformClassName = transformItk->GetNameOfClass();

  // Composite Transform
  if (itkTransformClassName == "CompositeTransform")
  {
    // Composition is handled by the caller (vtkMRMLTransformStorageNode)
    return nullptr;
  }

  // Linear
  vtkNew<vtkMatrix4x4> transformMatrixVtk;
  conversionSuccess = SetVTKLinearTransformFromITK<T>(loggerObject, transformMatrixVtk.GetPointer(), transformItk, center_LocalRAS);
  if (conversionSuccess)
  {
    vtkNew<vtkTransform> linearTransformVtk;
    linearTransformVtk->SetMatrix(transformMatrixVtk.GetPointer());
    linearTransformVtk->Register(nullptr);
    return linearTransformVtk.GetPointer();
  }

  // Grid
  vtkNew<vtkOrientedGridTransform> gridTransformVtk;
  if (itkDim == 3)
  {
    conversionSuccess = SetVTKOrientedGridTransformFromITK<T, 3>(loggerObject, gridTransformVtk.GetPointer(), transformItk);
  }
  else if (itkDim == 2)
  {
    conversionSuccess = SetVTKOrientedGridTransformFromITK<T, 2>(loggerObject, gridTransformVtk.GetPointer(), transformItk);
  }
  if (conversionSuccess)
  {
    gridTransformVtk->Register(nullptr);
    return gridTransformVtk.GetPointer();
  }

  // BSpline
  vtkNew<vtkOrientedBSplineTransform> bsplineTransformVtk;
  conversionSuccess = SetVTKBSplineFromITKv4Generic<T>(loggerObject, bsplineTransformVtk.GetPointer(), transformItk);
  if (conversionSuccess)
  {
    bsplineTransformVtk->Register(nullptr);
    return bsplineTransformVtk.GetPointer();
  }

  // ThinPlateSpline
  vtkNew<vtkThinPlateSplineTransform> tpsTransformVtk;
  if (itkDim == 3)
  {
    conversionSuccess = SetVTKThinPlateSplineTransformFromITK<T, 3>(loggerObject, tpsTransformVtk.GetPointer(), transformItk);
  }
  else if (itkDim == 2)
  {
    conversionSuccess = SetVTKThinPlateSplineTransformFromITK<T, 2>(loggerObject, tpsTransformVtk.GetPointer(), transformItk);
  }
  if (conversionSuccess)
  {
    tpsTransformVtk->Register(nullptr);
    return tpsTransformVtk.GetPointer();
  }

  return nullptr;
}

//----------------------------------------------------------------------------
itk::Object::Pointer vtkITKTransformConverter::CreateITKTransformFromVTK(vtkObject* loggerObject,
                                                                         vtkAbstractTransform* transformVtk,
                                                                         itk::Object::Pointer& secondaryTransformItk,
                                                                         int preferITKv3CompatibleTransforms,
                                                                         bool initialize /*= true*/,
                                                                         double center_LocalRAS[3] /*=nullptr*/)
{
  typedef itk::CompositeTransform<double> CompositeTransformType;

  if (transformVtk == nullptr)
  {
    vtkErrorWithObjectMacro(loggerObject, "CreateITKTransformFromVTK failed: invalid VTK transform");
    return nullptr;
  }
  vtkNew<vtkCollection> transformList;
  vtkMRMLTransformNode::FlattenGeneralTransform(transformList.GetPointer(), transformVtk);
  if (transformList->GetNumberOfItems() == 0)
  {
    // no transformation means identity transform
    vtkNew<vtkTransform> identity;
    transformList->AddItem(identity.GetPointer());
  }

  itk::Object::Pointer primaryTransformItk;
  if (transformList->GetNumberOfItems() == 1)
  {
    // Simple, non-composite transform (one item in the input transform)
    vtkObject* singleTransformVtk = transformList->GetItemAsObject(0);
    // Linear
    if (vtkHomogeneousTransform::SafeDownCast(singleTransformVtk))
    {
      vtkHomogeneousTransform* linearTransformVtk = vtkHomogeneousTransform::SafeDownCast(singleTransformVtk);
      vtkMatrix4x4* transformMatrix = linearTransformVtk->GetMatrix();
      if (!SetITKLinearTransformFromVTK(loggerObject, primaryTransformItk, transformMatrix, center_LocalRAS))
      {
        // conversion failed
        return nullptr;
      }
      return primaryTransformItk;
    }
    // BSpline
    else if (vtkOrientedBSplineTransform::SafeDownCast(singleTransformVtk))
    {
      vtkOrientedBSplineTransform* bsplineTransformVtk = vtkOrientedBSplineTransform::SafeDownCast(singleTransformVtk);
      vtkMatrix4x4* bulkMatrix = bsplineTransformVtk->GetBulkTransformMatrix(); // non-zero for ITKv3 bspline transform only
      if (preferITKv3CompatibleTransforms || (bulkMatrix != nullptr && !IsIdentityMatrix(bulkMatrix)))
      {
        if (!SetITKv3BSplineFromVTK(loggerObject, primaryTransformItk, secondaryTransformItk, bsplineTransformVtk, preferITKv3CompatibleTransforms))
        {
          // conversion failed
          return nullptr;
        }
        return primaryTransformItk;
      }
      else
      {
        if (!SetITKv4BSplineFromVTK(loggerObject, primaryTransformItk, bsplineTransformVtk))
        {
          // conversion failed
          return nullptr;
        }
        return primaryTransformItk;
      }
    }
    // Grid
    else if (vtkOrientedGridTransform::SafeDownCast(singleTransformVtk))
    {
      vtkOrientedGridTransform* gridTransformVtk = vtkOrientedGridTransform::SafeDownCast(singleTransformVtk);
      if (!SetITKOrientedGridTransformFromVTK(loggerObject, primaryTransformItk, gridTransformVtk))
      {
        // conversion failed
        return nullptr;
      }
      return primaryTransformItk;
    }
    // ThinPlateSpline
    else if (vtkThinPlateSplineTransform::SafeDownCast(singleTransformVtk))
    {
      vtkThinPlateSplineTransform* tpsTransformVtk = vtkThinPlateSplineTransform::SafeDownCast(singleTransformVtk);
      if (!SetITKThinPlateSplineTransformFromVTK(loggerObject, primaryTransformItk, tpsTransformVtk, initialize))
      {
        // conversion failed
        return nullptr;
      }
      return primaryTransformItk;
    }
    else
    {
      if (singleTransformVtk == nullptr)
      {
        vtkErrorWithObjectMacro(loggerObject, "vtkITKTransformConverter::CreateITKTransformFromVTK failed: invalid input transform");
        return nullptr;
      }
      vtkErrorWithObjectMacro(
        loggerObject, "vtkITKTransformConverter::CreateITKTransformFromVTK failed: conversion of transform type " << singleTransformVtk->GetClassName() << " is not supported");
      return nullptr;
    }
  }
  else
  {
    // Composite transform (more than one items in the input transform)
    // Create an ITK composite transform from the VTK general transform
    CompositeTransformType::Pointer compositeTransformItk = CompositeTransformType::New();
    primaryTransformItk = compositeTransformItk;
    // We need to iterate through the list in reverse order, as ITK CompositeTransform can only append transform on one side
    for (int transformIndex = transformList->GetNumberOfItems() - 1; transformIndex >= 0; --transformIndex)
    {
      vtkAbstractTransform* singleTransformVtk = vtkAbstractTransform::SafeDownCast(transformList->GetItemAsObject(transformIndex));
      itk::Object::Pointer secondaryTransformItkTmp;
      // We use ITKv4 format (PreferITKv3Transform format is set to false), because
      // legacy ITKv3 code does not know how to interpret composite transforms,
      // and also ITKv3 bspline transform with bulk component cannot be saved in a composite transform
      itk::Object::Pointer singleTransformItk = CreateITKTransformFromVTK(loggerObject, singleTransformVtk, secondaryTransformItkTmp, false);
      if (secondaryTransformItkTmp.IsNotNull())
      {
        vtkErrorWithObjectMacro(loggerObject,
                                "vtkITKTransformConverter::CreateITKTransformFromVTK failed:"
                                " composite transforms cannot contain legacy transforms (that contains secondary transforms)."
                                " Do not harden transforms on legacy ITK transforms to avoid this error.");
        return nullptr;
      }

      if (singleTransformItk.IsNull() //
          || std::string(singleTransformItk->GetNameOfClass()).find("Transform") == std::string::npos)
      {
        vtkErrorWithObjectMacro(loggerObject,
                                "vtkITKTransformConverter::CreateITKTransformFromVTK failed:"
                                " invalid element found while trying to create a composite transform");
        return nullptr;
      }
      CompositeTransformType::TransformType::Pointer singleTransformItkTypeChecked = static_cast<CompositeTransformType::TransformType*>(singleTransformItk.GetPointer());
      compositeTransformItk->AddTransform(singleTransformItkTypeChecked.GetPointer());
    }
    return primaryTransformItk;
  }
  return nullptr;
}

#endif
