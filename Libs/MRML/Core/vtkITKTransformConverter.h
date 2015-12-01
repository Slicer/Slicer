/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkITKTransformConverter.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkITKTransformConverter_h
#define __vtkITKTransformConverter_h

#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkPoints.h>
#include <vtkThinPlateSplineTransform.h>
#include <vtkTransform.h>

// ITK includes
#include <itkAffineTransform.h>
#include <itkBSplineDeformableTransform.h> // ITKv3 style
#include <itkBSplineTransform.h> // ITKv4 style
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

typedef itk::TransformFileReader TransformReaderType;
typedef TransformReaderType::TransformListType TransformListType;
typedef TransformReaderType::TransformType TransformType;

typedef itk::TransformFileWriter TransformWriterType;

typedef itk::DisplacementFieldTransform< double, 3 > DisplacementFieldTransformType;
typedef DisplacementFieldTransformType::DisplacementFieldType GridImageType;

typedef itk::BSplineDeformableTransform< float, VTKDimension, VTKDimension > BSplineTransformFloatITKv3Type;
typedef itk::BSplineDeformableTransform< double, VTKDimension, VTKDimension > BSplineTransformDoubleITKv3Type;
typedef itk::BSplineTransform< float, VTKDimension, VTKDimension > BSplineTransformFloatITKv4Type;
typedef itk::BSplineTransform< double, VTKDimension, VTKDimension > BSplineTransformDoubleITKv4Type;

typedef itk::ThinPlateSplineKernelTransform<double,3> ThinPlateSplineTransformType;

typedef itk::CompositeTransform< double > CompositeTransformType;

typedef itk::AffineTransform<double, VTKDimension> AffineTransformType;

#include "vtkITKTransformInverse.h"

class vtkITKTransformConverter
{
public:

  static void RegisterInverseTransformTypes();

  static vtkAbstractTransform* CreateVTKTransformFromITK(vtkObject* loggerObject, TransformType::Pointer transformItk);

  ///
  /// Create an ITK transform from a VTK transform.
  /// secondaryTransformItk: Only for backward compatibility. For BSpline transforms that have an additive bulk component, the bulk component is saved
  ///   in the secondary transform.
  /// preferITKv3CompatibleTransforms: If true then BSpline transform will created as BSplineDeformableTransform and additive bulk transform component is always written
  ///   in the secondary transform. If false then BSpline transform will be written as BSplineTransform (multiplicative bulk component is saved in a composite transform).
  static itk::Object::Pointer CreateITKTransformFromVTK(vtkObject* loggerObject, vtkAbstractTransform* transformVtk, itk::Object::Pointer& secondaryTransformItk, int preferITKv3CompatibleTransforms);

  static bool SetVTKBSplineFromITKv3(vtkObject* loggerObject, vtkOrientedBSplineTransform* bsplineVtk, TransformType::Pointer warpTransformItk, TransformType::Pointer bulkTransformItk);

  static bool SetVTKOrientedGridTransformFromITKImage(vtkObject* loggerObject, vtkOrientedGridTransform* grid_Ras, GridImageType::Pointer gridImage_Lps);
  static bool SetITKImageFromVTKOrientedGridTransform(vtkObject* loggerObject, GridImageType::Pointer &gridImage_Lps, vtkOrientedGridTransform* grid_Ras);

protected:

  static bool SetVTKLinearTransformFromITK(vtkObject* loggerObject, vtkMatrix4x4* transformVtk_RAS, TransformType::Pointer transformItk_LPS);
  static bool SetITKLinearTransformFromVTK(vtkObject* loggerObject, itk::Object::Pointer& transformItk_LPS, vtkMatrix4x4* transformVtk_RAS);

  static bool SetVTKOrientedGridTransformFromITK(vtkObject* loggerObject, vtkOrientedGridTransform* transformVtk_RAS, TransformType::Pointer transformItk_LPS);
  static bool SetITKOrientedGridTransformFromVTK(vtkObject* loggerObject, itk::Object::Pointer& transformItk_LPS, vtkOrientedGridTransform* transformVtk_RAS);

  static bool SetVTKBSplineFromITKv4(vtkObject* loggerObject, vtkOrientedBSplineTransform* bsplineVtk, TransformType::Pointer warpTransformItk);
  static bool SetITKv3BSplineFromVTK(vtkObject* loggerObject, itk::Object::Pointer& warpTransformItk, itk::Object::Pointer& bulkTransformItk, vtkOrientedBSplineTransform* bsplineVtk, bool alwaysAddBulkTransform);
  static bool SetITKv4BSplineFromVTK(vtkObject* loggerObject, itk::Object::Pointer& warpTransformItk, vtkOrientedBSplineTransform* bsplineVtk);

  static bool SetVTKThinPlateSplineTransformFromITK(vtkObject* loggerObject, vtkThinPlateSplineTransform* transformVtk_RAS, TransformType::Pointer transformItk_LPS);
  static bool SetITKThinPlateSplineTransformFromVTK(vtkObject* loggerObject, itk::Object::Pointer& transformItk_LPS, vtkThinPlateSplineTransform* transformVtk_RAS);

  static bool IsIdentityMatrix(vtkMatrix4x4 *matrix);

  template <typename BSplineTransformType> static bool SetVTKBSplineParametersFromITKGeneric(vtkObject* loggerObject, vtkOrientedBSplineTransform* bsplineVtk, TransformType::Pointer warpTransformItk);
  template <typename T> static bool SetVTKBSplineFromITKv3Generic(vtkObject* loggerObject, vtkOrientedBSplineTransform* bsplineVtk, TransformType::Pointer warpTransformItk, TransformType::Pointer bulkTransformItk);
  template <typename T> static bool SetVTKBSplineFromITKv4Generic(vtkObject* loggerObject, vtkOrientedBSplineTransform* bsplineVtk, TransformType::Pointer warpTransformItk);

  template <typename BSplineTransformType> static bool SetITKBSplineParametersFromVTKGeneric(vtkObject* loggerObject, typename itk::Transform< typename BSplineTransformType::ScalarType,VTKDimension,VTKDimension>::Pointer& warpTransformItk, vtkOrientedBSplineTransform* bsplineVtk);
  template <typename T> static bool SetITKv3BSplineFromVTKGeneric(vtkObject* loggerObject, typename itk::Transform<T,VTKDimension,VTKDimension>::Pointer& warpTransformItk, typename itk::Transform<T,VTKDimension,VTKDimension>::Pointer& bulkTransformItk, vtkOrientedBSplineTransform* bsplineVtk, bool alwaysAddBulkTransform);
  template <typename T> static bool SetITKv4BSplineFromVTKGeneric(vtkObject* loggerObject, typename itk::Transform<T,VTKDimension,VTKDimension>::Pointer& warpTransformItk, vtkOrientedBSplineTransform* bsplineVtk);

};

//----------------------------------------------------------------------------
void vtkITKTransformConverter::RegisterInverseTransformTypes()
{
  itk::TransformFactory<InverseDisplacementFieldTransformType>::RegisterTransform();
  itk::TransformFactory<InverseBSplineTransformFloatITKv3Type>::RegisterTransform();
  itk::TransformFactory<InverseBSplineTransformFloatITKv4Type>::RegisterTransform();
  itk::TransformFactory<InverseBSplineTransformDoubleITKv3Type>::RegisterTransform();
  itk::TransformFactory<InverseBSplineTransformDoubleITKv4Type>::RegisterTransform();
  itk::TransformFactory<InverseThinPlateSplineTransformType>::RegisterTransform();
  itk::TransformFactory<ThinPlateSplineTransformType>::RegisterTransform(); // by default it is not registered
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetVTKLinearTransformFromITK(vtkObject* /*loggerObject*/, vtkMatrix4x4* transformVtk_RAS, TransformType::Pointer transformItk_LPS)
{
  static const unsigned int D = VTKDimension;
  typedef itk::MatrixOffsetTransformBase<double,D,D> DoubleLinearTransformType;
  typedef itk::MatrixOffsetTransformBase<float,D,D> FloatLinearTransformType;
  typedef itk::IdentityTransform<double, D> DoubleIdentityTransformType;
  typedef itk::IdentityTransform<float, D> FloatIdentityTransformType;
  typedef itk::ScaleTransform<double, D> DoubleScaleTransformType;
  typedef itk::ScaleTransform<float, D> FloatScaleTransformType;
  typedef itk::TranslationTransform<double, D> DoubleTranslateTransformType;
  typedef itk::TranslationTransform<float, D> FloatTranslateTransformType;

  vtkSmartPointer<vtkMatrix4x4> transformVtk_LPS = vtkSmartPointer<vtkMatrix4x4>::New();

  bool convertedToVtkMatrix=false;

  // Linear transform of doubles, dimension 3
  DoubleLinearTransformType::Pointer dlt
    = dynamic_cast<DoubleLinearTransformType*>( transformItk_LPS.GetPointer() );
  if (dlt)
    {
    convertedToVtkMatrix=true;
    for (unsigned int i=0; i < D; i++)
      {
      for (unsigned int j=0; j < D; j++)
        {
        (*transformVtk_LPS)[i][j] = dlt->GetMatrix()[i][j];
        }
      (*transformVtk_LPS)[i][D] = dlt->GetOffset()[i];
      }
    }

  // Linear transform of floats, dimension 3
  FloatLinearTransformType::Pointer flt
    = dynamic_cast<FloatLinearTransformType*>( transformItk_LPS.GetPointer() );
  if (flt)
    {
    convertedToVtkMatrix=true;
    for (unsigned int i=0; i < D; i++)
      {
      for (unsigned int j=0; j < D; j++)
        {
        (*transformVtk_LPS)[i][j] = flt->GetMatrix()[i][j];
        }
      (*transformVtk_LPS)[i][D] = flt->GetOffset()[i];
      }
    }

  // Identity transform of doubles, dimension 3
  DoubleIdentityTransformType::Pointer dit
    = dynamic_cast<DoubleIdentityTransformType*>( transformItk_LPS.GetPointer() );
  if (dit)
    {
    // nothing to do, matrix is already the identity
    convertedToVtkMatrix=true;
    }

  // Identity transform of floats, dimension 3
  FloatIdentityTransformType::Pointer fit
    = dynamic_cast<FloatIdentityTransformType*>( transformItk_LPS.GetPointer() );
  if (fit)
    {
    // nothing to do, matrix is already the identity
    convertedToVtkMatrix=true;
    }

  // Scale transform of doubles, dimension 3
  DoubleScaleTransformType::Pointer dst
    = dynamic_cast<DoubleScaleTransformType*>( transformItk_LPS.GetPointer() );
  if (dst)
    {
    convertedToVtkMatrix=true;
    for (unsigned int i=0; i < D; i++)
      {
      (*transformVtk_LPS)[i][i] = dst->GetScale()[i];
      }
    }

  // Scale transform of floats, dimension 3
  FloatScaleTransformType::Pointer fst
    = dynamic_cast<FloatScaleTransformType*>( transformItk_LPS.GetPointer() );
  if (fst)
    {
    convertedToVtkMatrix=true;
    for (unsigned int i=0; i < D; i++)
      {
      (*transformVtk_LPS)[i][i] = fst->GetScale()[i];
      }
    }

  // Translate transform of doubles, dimension 3
  DoubleTranslateTransformType::Pointer dtt
    = dynamic_cast<DoubleTranslateTransformType*>( transformItk_LPS.GetPointer());
  if (dtt)
    {
    convertedToVtkMatrix=true;
    for (unsigned int i=0; i < D; i++)
      {
      (*transformVtk_LPS)[i][D] = dtt->GetOffset()[i];
      }
    }

  // Translate transform of floats, dimension 3
  FloatTranslateTransformType::Pointer ftt
    = dynamic_cast<FloatTranslateTransformType*>( transformItk_LPS.GetPointer() );
  if (ftt)
    {
    convertedToVtkMatrix=true;
    for (unsigned int i=0; i < D; i++)
      {
      (*transformVtk_LPS)[i][i] = ftt->GetOffset()[i];
      }
    }

  // Convert from LPS (ITK) to RAS (Slicer)
  //
  // Tras = lps2ras * Tlps * ras2lps
  //

  vtkSmartPointer<vtkMatrix4x4> lps2ras = vtkSmartPointer<vtkMatrix4x4>::New();
  lps2ras->SetElement(0,0,-1);
  lps2ras->SetElement(1,1,-1);
  vtkMatrix4x4* ras2lps = lps2ras; // lps2ras is diagonal therefore the inverse is identical

  vtkMatrix4x4::Multiply4x4(lps2ras, transformVtk_LPS, transformVtk_LPS);
  vtkMatrix4x4::Multiply4x4(transformVtk_LPS, ras2lps, transformVtk_RAS);

  return convertedToVtkMatrix;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetITKLinearTransformFromVTK(vtkObject* loggerObject, itk::Object::Pointer& transformItk_LPS, vtkMatrix4x4* transformVtk_RAS)
{
  if (transformVtk_RAS==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject,"vtkITKTransformConverter::SetITKLinearTransformFromVTK failed: invalid input transform");
    return false;
    }

  vtkSmartPointer<vtkMatrix4x4> lps2ras = vtkSmartPointer<vtkMatrix4x4>::New();
  lps2ras->SetElement(0,0,-1);
  lps2ras->SetElement(1,1,-1);
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

  for (unsigned int i=0; i < VTKDimension; i++)
    {
    for (unsigned int j=0; j < VTKDimension; j++)
      {
      itkmat[i][j] = (*vtkmat)[i][j];
      }
    itkoffset[i] = (*vtkmat)[i][VTKDimension];
    }

  AffineTransformType::Pointer affine = AffineTransformType::New();
  affine->SetMatrix(itkmat);
  affine->SetOffset(itkoffset);

  transformItk_LPS = affine;
  return true;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::IsIdentityMatrix(vtkMatrix4x4 *matrix)
{
  static double identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
  int i,j;

  for (i = 0; i < 4; i++)
    {
    for (j = 0; j < 4; j++)
      {
      if (matrix->GetElement(i,j) != identity[4*i+j])
        {
        return false;
        }
      }
    }
  return true;
}

//----------------------------------------------------------------------------
template <typename BSplineTransformType> bool vtkITKTransformConverter::SetVTKBSplineParametersFromITKGeneric(vtkObject* loggerObject,
  vtkOrientedBSplineTransform* bsplineVtk, TransformType::Pointer warpTransformItk)
{
  //
  // this version uses the itk::BSplineTransform not the itk::BSplineDeformableTransform
  //
  typedef typename BSplineTransformType::ScalarType T;
  if (bsplineVtk==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "vtkMRMLTransformStorageNode::SetVTKBSplineFromITKv4 failed: bsplineVtk is invalid");
    return false;
    }
  bool isDoublePrecisionInput=true;
  if (sizeof(T)==sizeof(float))
    {
    isDoublePrecisionInput=false;
    }
  else if (sizeof(T)==sizeof(double))
    {
    isDoublePrecisionInput=true;
    }
  else
    {
    vtkErrorWithObjectMacro(loggerObject,"Unsupported scalar type in BSpline transform file (only float and double are supported)");
    return false;
    }

  typename BSplineTransformType::Pointer bsplineItk = dynamic_cast< BSplineTransformType* >( warpTransformItk.GetPointer() );
  if (!bsplineItk.GetPointer())
    {
    return false;
    }

  // now get the fixed parameters and map them to the vtk analogs

  // it turns out that for a BSplineTransform, the TransformDomain information
  // is not populated when reading from a file, so instead we access these
  // fields from one of the coefficient images (they are assumed to be
  // identical)
  const typename BSplineTransformType::CoefficientImageArray coefficientImages =
    bsplineItk->GetCoefficientImages();

  // * mesh size X, Y, Z (including the BSPLINE_TRANSFORM_ORDER=3 boundary nodes,
  //   1 before and 2 after the grid)
  typename BSplineTransformType::MeshSizeType meshSize =
    coefficientImages[0]->GetLargestPossibleRegion().GetSize();

  // * mesh origin X, Y, Z (position of the boundary node before the grid)
  typename BSplineTransformType::OriginType origin =
    coefficientImages[0]->GetOrigin();

  // * mesh spacing X, Y, Z
  typename BSplineTransformType::SpacingType spacing =
    coefficientImages[0]->GetSpacing();

  // * mesh direction 3x3 matrix (first row, second row, third row)
  typename BSplineTransformType::DirectionType direction =
    coefficientImages[0]->GetDirection();

  vtkNew<vtkMatrix4x4> gridDirectionMatrix_LPS;
  for (unsigned int row=0; row<VTKDimension; row++)
    {
    for (unsigned int column=0; column<VTKDimension; column++)
      {
      gridDirectionMatrix_LPS->SetElement(row,column,direction[row][column]);
      }
    }

  // Set bspline grid and coefficients
  bsplineVtk->SetBorderModeToZero();

  vtkNew<vtkImageData> bsplineCoefficients;

  bsplineCoefficients->SetExtent(0, meshSize[0]-1, 0, meshSize[1]-1, 0, meshSize[2]-1);
  bsplineCoefficients->SetSpacing(spacing[0], spacing[1], spacing[2]);

  // convert the direction matrix from LPS (itk) to RAS (slicer)
  vtkNew<vtkMatrix4x4> lpsToRas;
  lpsToRas->SetElement(0,0,-1);
  lpsToRas->SetElement(1,1,-1);

  vtkNew<vtkMatrix4x4> rasToLps;
  rasToLps->SetElement(0,0,-1);
  rasToLps->SetElement(1,1,-1);

  vtkNew<vtkMatrix4x4> gridDirectionMatrix_RAS;
  vtkMatrix4x4::Multiply4x4(
    lpsToRas.GetPointer(),
    gridDirectionMatrix_LPS.GetPointer(),
    gridDirectionMatrix_RAS.GetPointer());
  bsplineVtk->SetGridDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());

  // convert the origin from LPS (itk) to RAS (slicer)
  double gridOrigin_RAS[3]={-origin[0], -origin[1], origin[2]};
  bsplineCoefficients->SetOrigin(gridOrigin_RAS);

  int bsplineCoefficientsScalarType=VTK_FLOAT;
  if (isDoublePrecisionInput)
    {
    bsplineCoefficientsScalarType=VTK_DOUBLE;
    }

  bsplineCoefficients->AllocateScalars(bsplineCoefficientsScalarType, 3);

  const unsigned int expectedNumberOfVectors = meshSize[0]*meshSize[1]*meshSize[2];
  const unsigned int expectedNumberOfParameters = expectedNumberOfVectors*VTKDimension;
  const unsigned int actualNumberOfParameters = bsplineItk->GetNumberOfParameters();

  if( actualNumberOfParameters != expectedNumberOfParameters )
    {
    vtkErrorWithObjectMacro(loggerObject,"Mismatch in number of BSpline parameters in the transform file and the MRML node");
    return false;
    }
  const T* itkBSplineParams_LPS = static_cast<const T*>(bsplineItk->GetParameters().data_block());
  T* vtkBSplineParams_RAS=static_cast<T*>(bsplineCoefficients->GetScalarPointer());
  for (unsigned int i=0; i<expectedNumberOfVectors; i++)
    {
    *(vtkBSplineParams_RAS++) =  - (*(itkBSplineParams_LPS                          ));
    *(vtkBSplineParams_RAS++) =  - (*(itkBSplineParams_LPS+expectedNumberOfVectors  ));
    *(vtkBSplineParams_RAS++) =    (*(itkBSplineParams_LPS+expectedNumberOfVectors*2));
    itkBSplineParams_LPS++;
    }
  bsplineVtk->SetCoefficientData(bsplineCoefficients.GetPointer());

  // Success
  return true;
}

//----------------------------------------------------------------------------
template <typename T> bool vtkITKTransformConverter::SetVTKBSplineFromITKv3Generic(vtkObject* loggerObject,
  vtkOrientedBSplineTransform* bsplineVtk,
  TransformType::Pointer warpTransformItk, TransformType::Pointer bulkTransformItk)
{
  if (bsplineVtk==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "vtkMRMLTransformStorageNode::SetVTKBSplineFromITK failed: bsplineVtk is invalid");
    return false;
    }

  bool inverse = false;
  // inverse class is derived from forward class, so it has to be checked first
  if (SetVTKBSplineParametersFromITKGeneric< itk::InverseBSplineDeformableTransform< T, VTKDimension, VTKDimension > >(loggerObject, bsplineVtk, warpTransformItk))
    {
    inverse = true;
    }
  else if (SetVTKBSplineParametersFromITKGeneric< itk::BSplineDeformableTransform< T, VTKDimension, VTKDimension > >(loggerObject, bsplineVtk, warpTransformItk))
    {
    inverse = false;
    }
  else
    {
    vtkDebugWithObjectMacro(loggerObject, "Not an ITKv3 BSpline transform");
    return false;
    }

  // Set the bulk transform
  if( bulkTransformItk )
    {
    typedef itk::AffineTransform<T,3> BulkTransformType;
    typedef itk::IdentityTransform<T,3> IdentityBulkTransformType;
    BulkTransformType* bulkItkAffine = dynamic_cast<BulkTransformType*> (bulkTransformItk.GetPointer());
    IdentityBulkTransformType* bulkItkIdentity = dynamic_cast<IdentityBulkTransformType*> (bulkTransformItk.GetPointer());
    if (bulkItkAffine)
      {
      vtkNew<vtkMatrix4x4> bulkMatrix_LPS;
      for (unsigned int i=0; i < VTKDimension; i++)
        {
        for (unsigned int j=0; j < VTKDimension; j++)
          {
          bulkMatrix_LPS->SetElement(i,j, bulkItkAffine->GetMatrix()[i][j]);
          }
        bulkMatrix_LPS->SetElement(i,VTKDimension, bulkItkAffine->GetOffset()[i]);
        }
      vtkNew<vtkMatrix4x4> lpsToRas;
      lpsToRas->SetElement(0,0,-1);
      lpsToRas->SetElement(1,1,-1);
      vtkNew<vtkMatrix4x4> rasToLps;
      rasToLps->SetElement(0,0,-1);
      rasToLps->SetElement(1,1,-1);
      vtkNew<vtkMatrix4x4> bulkMatrix_RAS; // bulk_RAS = lpsToRas * bulk_LPS * rasToLps
      vtkMatrix4x4::Multiply4x4(lpsToRas.GetPointer(), bulkMatrix_LPS.GetPointer(), bulkMatrix_RAS.GetPointer());
      vtkMatrix4x4::Multiply4x4(bulkMatrix_RAS.GetPointer(), rasToLps.GetPointer(), bulkMatrix_RAS.GetPointer());
      bsplineVtk->SetBulkTransformMatrix(bulkMatrix_RAS.GetPointer());
      }
    else if (bulkItkIdentity)
      {
      // bulk transform is identity, which is equivalent to no bulk transform
      }
    else
      {
      vtkErrorWithObjectMacro(loggerObject,"Cannot read the 2nd transform in BSplineTransform (expected AffineTransform_double_3_3 or IdentityTransform)" );
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
template <typename T> bool vtkITKTransformConverter::SetVTKBSplineFromITKv4Generic(vtkObject* loggerObject,
  vtkOrientedBSplineTransform* bsplineVtk, TransformType::Pointer warpTransformItk)
{
  bool inverse = false;
  // inverse class is derived from forward class, so it has to be checked first
  if (SetVTKBSplineParametersFromITKGeneric< itk::InverseBSplineTransform< T, VTKDimension, VTKDimension > >(loggerObject, bsplineVtk, warpTransformItk))
    {
    inverse = true;
    }
  else if (SetVTKBSplineParametersFromITKGeneric< itk::BSplineTransform< T, VTKDimension, VTKDimension > >(loggerObject, bsplineVtk, warpTransformItk))
    {
    inverse = false;
    }
  else
    {
    vtkDebugWithObjectMacro(loggerObject, "Not an ITKv4 BSpline transform");
    return false;
    }
  if (inverse)
    {
    bsplineVtk->Inverse();
    }
  return true;
}
//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetVTKBSplineFromITKv4(vtkObject* loggerObject, vtkOrientedBSplineTransform* bsplineVtk, TransformType::Pointer warpTransformItk)
{
  if (SetVTKBSplineFromITKv4Generic<double>(loggerObject, bsplineVtk, warpTransformItk))
  {
    return true;
  }
  if (SetVTKBSplineFromITKv4Generic<float>(loggerObject, bsplineVtk, warpTransformItk))
  {
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetVTKBSplineFromITKv3(vtkObject* loggerObject, vtkOrientedBSplineTransform* bsplineVtk, TransformType::Pointer warpTransformItk, TransformType::Pointer bulkTransformItk)
{
  if (SetVTKBSplineFromITKv3Generic<double>(loggerObject, bsplineVtk, warpTransformItk, bulkTransformItk))
  {
    return true;
  }
  if (SetVTKBSplineFromITKv3Generic<float>(loggerObject, bsplineVtk, warpTransformItk, bulkTransformItk))
  {
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
template <typename BSplineTransformType> bool vtkITKTransformConverter::SetITKBSplineParametersFromVTKGeneric(vtkObject* loggerObject,
  typename itk::Transform< typename BSplineTransformType::ScalarType,VTKDimension,VTKDimension>::Pointer& warpTransformItk, vtkOrientedBSplineTransform* bsplineVtk)
{
  typedef typename BSplineTransformType::ScalarType T;
  if (bsplineVtk==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "vtkMRMLTransformStorageNode::SetITKBSplineFromVTK failed: bsplineVtk is invalid");
    return false;
    }
  vtkImageData* bsplineCoefficients_RAS=bsplineVtk->GetCoefficientData();
  if (bsplineCoefficients_RAS==NULL)
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
#if defined(ITK_FIXED_PARAMETERS_ARE_DOUBLE) // After 4.8.1
  typename BSplineTransformType::FixedParametersType transformFixedParamsItk;
#else                                         //Pre 4.8.1
  typename BSplineTransformType::ParametersType transformFixedParamsItk;
#endif
  const unsigned int numberOfFixedParameters=VTKDimension*(VTKDimension+3);
  transformFixedParamsItk.SetSize(numberOfFixedParameters);

  int *gridExtent=bsplineCoefficients_RAS->GetExtent();
  int gridSize[3]={gridExtent[1]-gridExtent[0]+1, gridExtent[3]-gridExtent[2]+1, gridExtent[5]-gridExtent[4]+1};
  transformFixedParamsItk[0]=gridSize[0];
  transformFixedParamsItk[1]=gridSize[1];
  transformFixedParamsItk[2]=gridSize[2];

  double* gridOrigin_RAS=bsplineCoefficients_RAS->GetOrigin();
  double gridOrigin_LPS[3]={-gridOrigin_RAS[0], -gridOrigin_RAS[1], gridOrigin_RAS[2]};
  transformFixedParamsItk[3]=gridOrigin_LPS[0];
  transformFixedParamsItk[4]=gridOrigin_LPS[1];
  transformFixedParamsItk[5]=gridOrigin_LPS[2];

  double* gridSpacing=bsplineCoefficients_RAS->GetSpacing();
  transformFixedParamsItk[6]=gridSpacing[0];
  transformFixedParamsItk[7]=gridSpacing[1];
  transformFixedParamsItk[8]=gridSpacing[2];

  vtkNew<vtkMatrix4x4> gridDirectionMatrix_RAS;
  if (bsplineVtk->GetGridDirectionMatrix()!=NULL)
    {
    gridDirectionMatrix_RAS->DeepCopy(bsplineVtk->GetGridDirectionMatrix());
    }
  vtkNew<vtkMatrix4x4> lpsToRas;
  lpsToRas->SetElement(0,0,-1);
  lpsToRas->SetElement(1,1,-1);
  vtkNew<vtkMatrix4x4> rasToLps;
  rasToLps->SetElement(0,0,-1);
  rasToLps->SetElement(1,1,-1);
  vtkNew<vtkMatrix4x4> gridDirectionMatrix_LPS;
  vtkMatrix4x4::Multiply4x4(rasToLps.GetPointer(), gridDirectionMatrix_RAS.GetPointer(), gridDirectionMatrix_LPS.GetPointer());
  int fpIndex=9;
  for (unsigned int row=0; row<VTKDimension; row++)
    {
    for (unsigned int column=0; column<VTKDimension; column++)
      {
      transformFixedParamsItk[fpIndex++]=gridDirectionMatrix_LPS->GetElement(row,column);
      }
    }

  bsplineItk->SetFixedParameters(transformFixedParamsItk);

  // BSpline coefficients

  const unsigned int expectedNumberOfVectors = gridSize[0]*gridSize[1]*gridSize[2];
  const unsigned int expectedNumberOfParameters = expectedNumberOfVectors*VTKDimension;
  if( bsplineItk->GetNumberOfParameters() != expectedNumberOfParameters )
    {
    vtkErrorWithObjectMacro(loggerObject,"Mismatch in number of BSpline parameters in the ITK transform and the VTK transform");
    return false;
    }

  typename BSplineTransformType::ParametersType transformParamsItk(expectedNumberOfParameters);
  T* itkBSplineParams_LPS = static_cast<T*>(transformParamsItk.data_block());
  T* vtkBSplineParams_RAS=static_cast<T*>(bsplineCoefficients_RAS->GetScalarPointer());
  double coefficientScale = bsplineVtk->GetDisplacementScale();
  for (unsigned int i=0; i<expectedNumberOfVectors; i++)
    {
    *(itkBSplineParams_LPS                          ) = -coefficientScale * (*(vtkBSplineParams_RAS++));
    *(itkBSplineParams_LPS+expectedNumberOfVectors  ) = -coefficientScale * (*(vtkBSplineParams_RAS++));
    *(itkBSplineParams_LPS+expectedNumberOfVectors*2) =  coefficientScale * (*(vtkBSplineParams_RAS++));
    itkBSplineParams_LPS++;
    }

  bsplineItk->SetParameters(transformParamsItk);
  return true;
}

//----------------------------------------------------------------------------
template <typename T> bool vtkITKTransformConverter::SetITKv3BSplineFromVTKGeneric(vtkObject* loggerObject,
  typename itk::Transform<T,VTKDimension,VTKDimension>::Pointer& warpTransformItk,
  typename itk::Transform<T,VTKDimension,VTKDimension>::Pointer& bulkTransformItk,
  vtkOrientedBSplineTransform* bsplineVtk, bool alwaysAddBulkTransform)
{
  if (bsplineVtk==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "vtkMRMLTransformStorageNode::SetITKBSplineFromVTK failed: bsplineVtk is invalid");
    return false;
    }
  // Update is needed because it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
  bsplineVtk->Update();
  bool itkTransformSetSuccessfully = false;
  if (bsplineVtk->GetInverseFlag())
    {
    itkTransformSetSuccessfully = SetITKBSplineParametersFromVTKGeneric< itk::InverseBSplineDeformableTransform< T, VTKDimension, VTKDimension > >(loggerObject, warpTransformItk, bsplineVtk);
    }
  else
    {
    itkTransformSetSuccessfully = SetITKBSplineParametersFromVTKGeneric< itk::BSplineDeformableTransform< T, VTKDimension, VTKDimension > >(loggerObject, warpTransformItk, bsplineVtk);
    }
  if (!itkTransformSetSuccessfully)
    {
    vtkErrorWithObjectMacro(loggerObject, "vtkMRMLTransformStorageNode::SetITKBSplineFromVTK failed: cannot determine BSpline parameters");
    return false;
    }

  vtkMatrix4x4* bulkMatrix_RAS=bsplineVtk->GetBulkTransformMatrix();
  if (bulkMatrix_RAS || alwaysAddBulkTransform)
    {
    vtkNew<vtkMatrix4x4> lpsToRas;
    lpsToRas->SetElement(0,0,-1);
    lpsToRas->SetElement(1,1,-1);
    vtkNew<vtkMatrix4x4> rasToLps;
    rasToLps->SetElement(0,0,-1);
    rasToLps->SetElement(1,1,-1);
    vtkNew<vtkMatrix4x4> bulkMatrix_LPS; // bulk_LPS = rasToLps * bulk_RAS * lpsToRas
    // If bulk transform is available then use it, otherwise just write an identity matrix (we just write it because
    // alwaysAddBulkTransform was requested, due to backward compatibility reasons)
    if (bulkMatrix_RAS!=NULL)
      {
      vtkMatrix4x4::Multiply4x4(rasToLps.GetPointer(), bulkMatrix_RAS, bulkMatrix_LPS.GetPointer());
      vtkMatrix4x4::Multiply4x4(bulkMatrix_LPS.GetPointer(), lpsToRas.GetPointer(), bulkMatrix_LPS.GetPointer());
      }
    typedef itk::AffineTransform<T,VTKDimension> BulkTransformType;
    typename BulkTransformType::Pointer affineItk = BulkTransformType::New();
    bulkTransformItk = affineItk;

    typename BulkTransformType::MatrixType affineMatrix;
    typename BulkTransformType::OffsetType affineOffset;
    for (unsigned int i=0; i < VTKDimension; i++)
      {
      for (unsigned int j=0; j < VTKDimension; j++)
        {
        affineMatrix[i][j]=bulkMatrix_LPS->GetElement(i,j);
        }
      affineOffset[i]=bulkMatrix_LPS->GetElement(i,VTKDimension);
      }

    affineItk->SetMatrix(affineMatrix);
    affineItk->SetOffset(affineOffset);
    }
  else
    {
    bulkTransformItk=NULL;
    }

  return true;
}

//----------------------------------------------------------------------------
template <typename T> bool vtkITKTransformConverter::SetITKv4BSplineFromVTKGeneric(vtkObject* loggerObject,
  typename itk::Transform<T,VTKDimension,VTKDimension>::Pointer& warpTransformItk,
  vtkOrientedBSplineTransform* bsplineVtk)
{
  // Update is needed because it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
  bsplineVtk->Update();
  bool itkTransformSetSuccessfully = false;
  if (bsplineVtk->GetInverseFlag())
    {
    itkTransformSetSuccessfully = SetITKBSplineParametersFromVTKGeneric< itk::InverseBSplineTransform< T, VTKDimension, VTKDimension > >(loggerObject, warpTransformItk, bsplineVtk);
    }
  else
    {
    itkTransformSetSuccessfully = SetITKBSplineParametersFromVTKGeneric< itk::BSplineTransform< T, VTKDimension, VTKDimension > >(loggerObject, warpTransformItk, bsplineVtk);
    }
  if (!itkTransformSetSuccessfully)
    {
    vtkErrorWithObjectMacro(loggerObject, "vtkMRMLTransformStorageNode::SetITKv4BSplineFromVTKGeneric failed: cannot determine BSpline parameters");
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetITKv3BSplineFromVTK(vtkObject* loggerObject, itk::Object::Pointer& warpTransformItk,
  itk::Object::Pointer& bulkTransformItk, vtkOrientedBSplineTransform* bsplineVtk, bool alwaysAddBulkTransform)
{
  if (bsplineVtk==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "Cannot retrieve BSpline transform from node");
    return false;
    }

  vtkImageData* bsplineCoefficients=bsplineVtk->GetCoefficientData();

  if (bsplineCoefficients==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "Cannot write BSpline transform to file: coefficients are not specified");
    return false;
    }

  if (bsplineCoefficients->GetScalarType()==VTK_FLOAT)
    {
    typedef itk::Transform<float, VTKDimension, VTKDimension > ITKTransformType;
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
  else if (bsplineCoefficients->GetScalarType()==VTK_DOUBLE)
    {
    typedef itk::Transform<double, VTKDimension, VTKDimension > ITKTransformType;
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
  if (bsplineVtk==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "Cannot retrieve BSpline transform from node");
    return false;
    }

  vtkImageData* bsplineCoefficients=bsplineVtk->GetCoefficientData();

  if (bsplineCoefficients==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "Cannot write BSpline transform to file: coefficients are not specified");
    return false;
    }

  if (bsplineCoefficients->GetScalarType()==VTK_FLOAT)
    {
    typedef itk::Transform<float, VTKDimension, VTKDimension > ITKTransformType;
    ITKTransformType::Pointer floatWarpTransformItk;
    if (!SetITKv4BSplineFromVTKGeneric<float>(loggerObject, floatWarpTransformItk, bsplineVtk))
      {
      vtkErrorWithObjectMacro(loggerObject, "Cannot write BSpline transform to file");
      return false;
      }
    warpTransformItk = floatWarpTransformItk.GetPointer();
    }
  else if (bsplineCoefficients->GetScalarType()==VTK_DOUBLE)
    {
    typedef itk::Transform<double, VTKDimension, VTKDimension > ITKTransformType;
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
bool vtkITKTransformConverter::SetVTKOrientedGridTransformFromITK(vtkObject* loggerObject, vtkOrientedGridTransform* transformVtk_RAS, TransformType::Pointer transformItk_LPS)
{
  DisplacementFieldTransformType* displacementFieldTransform = dynamic_cast<DisplacementFieldTransformType*>( transformItk_LPS.GetPointer() );
  DisplacementFieldTransformType* inverseDisplacementFieldTransform = dynamic_cast<InverseDisplacementFieldTransformType*>( transformItk_LPS.GetPointer() );
  bool inverse = false;
  DisplacementFieldTransformType::DisplacementFieldType* gridImageItk_Lps = NULL;
  if (inverseDisplacementFieldTransform!=NULL) // inverse class is derived from forward class, so it has to be checked first
    {
    inverse = true;
    gridImageItk_Lps = inverseDisplacementFieldTransform->GetDisplacementField();
    }
  else if (displacementFieldTransform!=NULL)
    {
    inverse = false;
    gridImageItk_Lps = displacementFieldTransform->GetDisplacementField();
    }
  else
    {
    vtkDebugWithObjectMacro(loggerObject, "Not a grid transform");
    return false;
    }
  if (!SetVTKOrientedGridTransformFromITKImage(loggerObject, transformVtk_RAS, gridImageItk_Lps))
    {
    return false;
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
  GridImageType::Pointer gridImageItk_Lps;
  if (!SetITKImageFromVTKOrientedGridTransform(loggerObject, gridImageItk_Lps, transformVtk_RAS))
    {
    vtkErrorWithObjectMacro(loggerObject, "vtkITKTransformConverter::SetITKOrientedGridTransformFromVTK failed: input transform is invalid");
    return false;
    }
  // Update is needed because it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
  transformVtk_RAS->Update();
  if (transformVtk_RAS->GetInverseFlag())
    {
    InverseDisplacementFieldTransformType::Pointer gridTransformItk = InverseDisplacementFieldTransformType::New();
    gridTransformItk->SetDisplacementField(gridImageItk_Lps);
    transformItk_LPS = gridTransformItk;
    }
  else
    {
    DisplacementFieldTransformType::Pointer gridTransformItk = DisplacementFieldTransformType::New();
    gridTransformItk->SetDisplacementField(gridImageItk_Lps);
    transformItk_LPS = gridTransformItk;
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetVTKOrientedGridTransformFromITKImage(vtkObject* loggerObject, vtkOrientedGridTransform* grid_Ras, GridImageType::Pointer gridImage_Lps)
{
  vtkNew<vtkImageData> gridImage_Ras;

  // Origin
  gridImage_Ras->SetOrigin( -gridImage_Lps->GetOrigin()[0], -gridImage_Lps->GetOrigin()[1], gridImage_Lps->GetOrigin()[2] );

  // Spacing
  gridImage_Ras->SetSpacing( gridImage_Lps->GetSpacing()[0], gridImage_Lps->GetSpacing()[1], gridImage_Lps->GetSpacing()[2] );

  // Direction
  vtkNew<vtkMatrix4x4> gridDirectionMatrix_LPS;
  for (unsigned int row=0; row<VTKDimension; row++)
    {
    for (unsigned int column=0; column<VTKDimension; column++)
      {
      gridDirectionMatrix_LPS->SetElement(row,column,gridImage_Lps->GetDirection()(row,column));
      }
    }
  vtkNew<vtkMatrix4x4> lpsToRas;
  lpsToRas->SetElement(0,0,-1);
  lpsToRas->SetElement(1,1,-1);
  vtkNew<vtkMatrix4x4> gridDirectionMatrix_RAS;
  vtkMatrix4x4::Multiply4x4(lpsToRas.GetPointer(), gridDirectionMatrix_LPS.GetPointer(), gridDirectionMatrix_RAS.GetPointer());
  grid_Ras->SetGridDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());

  // Vectors
  GridImageType::SizeType size = gridImage_Lps->GetBufferedRegion().GetSize();
  gridImage_Ras->SetDimensions( size[0], size[1], size[2] );
  unsigned int numberOfScalarComponents = GridImageType::PixelType::Dimension;
  if (numberOfScalarComponents!=VTKDimension)
    {
    vtkErrorWithObjectMacro(loggerObject, "Cannot load grid transform: the input displacement field expected to contain "
      << VTKDimension << " components but it actually contains " << numberOfScalarComponents );
    return false;
    }
  gridImage_Ras->AllocateScalars(VTK_DOUBLE, 3);

  double* displacementVectors_Ras = reinterpret_cast<double*>(gridImage_Ras->GetScalarPointer());
  itk::ImageRegionConstIterator<GridImageType> inputIt(gridImage_Lps, gridImage_Lps->GetRequestedRegion());
  inputIt.GoToBegin();
  while( !inputIt.IsAtEnd() )
    {
    GridImageType::PixelType displacementVectorLps=inputIt.Get();
    *(displacementVectors_Ras++) = -displacementVectorLps[0];
    *(displacementVectors_Ras++) = -displacementVectorLps[1];
    *(displacementVectors_Ras++) =  displacementVectorLps[2];
    ++inputIt;
    }

  grid_Ras->SetDisplacementGridData( gridImage_Ras.GetPointer() );

  // Set the interpolation to cubic to have smooth derivatives
  grid_Ras->SetInterpolationModeToCubic();

  return true;
}

//----------------------------------------------------------------------------
bool vtkITKTransformConverter::SetITKImageFromVTKOrientedGridTransform(vtkObject* loggerObject, GridImageType::Pointer &gridImage_Lps, vtkOrientedGridTransform* grid_Ras)
{
  if (grid_Ras==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "Cannot save grid transform: the input vtkOrientedGridTransform is invalid");
    return false;
    }

  // Update is needed because DisplacementGrid may be out-of-date if the transform depends on its inverse
  grid_Ras->Update();

  vtkImageData* gridImage_Ras = grid_Ras->GetDisplacementGrid();
  if (gridImage_Ras==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "Cannot save grid transform: the input vtkOrientedGridTransform does not contain a valid displacement grid");
    return false;
    }
  if (gridImage_Ras->GetNumberOfScalarComponents()!=VTKDimension)
    {
    vtkErrorWithObjectMacro(loggerObject, "Cannot save grid transform: the input vtkOrientedGridTransform expected to contain "
      << VTKDimension << " components but it actually contains " << gridImage_Ras->GetNumberOfScalarComponents() );
    return false;
    }

  gridImage_Lps = GridImageType::New();

  // Origin
  double* origin_Ras = gridImage_Ras->GetOrigin();
  double origin_Lps[3] = { -origin_Ras[0], -origin_Ras[1], origin_Ras[2] };
  gridImage_Lps->SetOrigin( origin_Lps );

  // Spacing
  double* spacing = gridImage_Ras->GetSpacing();
  //GridType::SpacingType spacing( spacing );
  gridImage_Lps->SetSpacing( spacing );

  // Direction
  vtkNew<vtkMatrix4x4> gridDirectionMatrix_Ras;
  if (grid_Ras->GetGridDirectionMatrix()!=NULL)
    {
    gridDirectionMatrix_Ras->DeepCopy(grid_Ras->GetGridDirectionMatrix());
    }
  vtkNew<vtkMatrix4x4> rasToLps;
  rasToLps->SetElement(0,0,-1);
  rasToLps->SetElement(1,1,-1);
  vtkNew<vtkMatrix4x4> gridDirectionMatrix_Lps;
  vtkMatrix4x4::Multiply4x4(rasToLps.GetPointer(), gridDirectionMatrix_Ras.GetPointer(), gridDirectionMatrix_Lps.GetPointer());
  GridImageType::DirectionType gridDirectionMatrixItk_Lps;
  for (unsigned int row=0; row<VTKDimension; row++)
    {
    for (unsigned int column=0; column<VTKDimension; column++)
      {
      gridDirectionMatrixItk_Lps(row,column) = gridDirectionMatrix_Lps->GetElement(row,column);
      }
    }
  gridImage_Lps->SetDirection(gridDirectionMatrixItk_Lps);

  // Vectors
  GridImageType::IndexType start;
  start[0] = start[1] = start[2] = 0;
  int* Nijk = gridImage_Ras->GetDimensions();
  GridImageType::SizeType size;
  size[0] = Nijk[0]; size[1] = Nijk[1]; size[2] = Nijk[2];
  GridImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( start );
  gridImage_Lps->SetRegions( region );
  gridImage_Lps->Allocate();
  itk::ImageRegionIterator<GridImageType> gridImageIt_Lps(gridImage_Lps, region);
  gridImageIt_Lps.GoToBegin();
  GridImageType::PixelType displacementVectorLps;
  double displacementScale = grid_Ras->GetDisplacementScale();
  double displacementShift = grid_Ras->GetDisplacementShift();

  if (gridImage_Ras->GetScalarType()==VTK_DOUBLE)
    {
    double* displacementVectors_Ras = reinterpret_cast<double*>(gridImage_Ras->GetScalarPointer());
    while( !gridImageIt_Lps.IsAtEnd() )
      {
      displacementVectorLps[0] = -( displacementScale * (*(displacementVectors_Ras++)) + displacementShift );
      displacementVectorLps[1] = -( displacementScale * (*(displacementVectors_Ras++)) + displacementShift );
      displacementVectorLps[2] =  ( displacementScale * (*(displacementVectors_Ras++)) + displacementShift );
      gridImageIt_Lps.Set(displacementVectorLps);
      ++gridImageIt_Lps;
      }
    }
  else if (gridImage_Ras->GetScalarType()==VTK_FLOAT)
    {
    float* displacementVectors_Ras = reinterpret_cast<float*>(gridImage_Ras->GetScalarPointer());
  while( !gridImageIt_Lps.IsAtEnd() )
    {
    displacementVectorLps[0] = -( displacementScale * (*(displacementVectors_Ras++)) + displacementShift );
    displacementVectorLps[1] = -( displacementScale * (*(displacementVectors_Ras++)) + displacementShift );
    displacementVectorLps[2] =  ( displacementScale * (*(displacementVectors_Ras++)) + displacementShift );
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
bool vtkITKTransformConverter::SetVTKThinPlateSplineTransformFromITK(vtkObject* loggerObject, vtkThinPlateSplineTransform* transformVtk_RAS, TransformType::Pointer transformItk_LPS)
{
  if (transformVtk_RAS==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "Cannot set VTK thin-plate spline transform from ITK: the output vtkThinPlateSplineTransform is invalid");
    return false;
    }

  ThinPlateSplineTransformType* tpsTransform = dynamic_cast<ThinPlateSplineTransformType*>( transformItk_LPS.GetPointer() );
  ThinPlateSplineTransformType* inverseTpsTransform = dynamic_cast<InverseThinPlateSplineTransformType*>( transformItk_LPS.GetPointer() );
  bool inverse = false;
  ThinPlateSplineTransformType::PointSetType::Pointer sourceLandmarksItk_Lps;
  ThinPlateSplineTransformType::PointSetType::Pointer targetLandmarksItk_Lps;
  if (inverseTpsTransform!=NULL) // inverse class is derived from forward class, so it has to be checked first
    {
    inverse = true;
    sourceLandmarksItk_Lps = inverseTpsTransform->GetSourceLandmarks();
    targetLandmarksItk_Lps = inverseTpsTransform->GetTargetLandmarks();
    }
  else if (tpsTransform!=NULL)
    {
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
  for(unsigned int i = 0; i < numberOfSourceLandmarks; i++)
    {
    ThinPlateSplineTransformType::InputPointType pointItk_Lps;
    bool pointExists = sourceLandmarksItk_Lps->GetPoint(i, &pointItk_Lps);
    if (!pointExists)
      {
      continue;
      }
    double pointVtk_Ras[3]={0};
    pointVtk_Ras[0] = -pointItk_Lps[0];
    pointVtk_Ras[1] = -pointItk_Lps[1];
    pointVtk_Ras[2] =  pointItk_Lps[2];
    sourceLandmarksVtk_Ras->InsertNextPoint(pointVtk_Ras);
    }

  vtkNew<vtkPoints> targetLandmarksVtk_Ras;
  unsigned int numberOfTargetLandmarks = targetLandmarksItk_Lps->GetNumberOfPoints();
  for(unsigned int i = 0; i < numberOfTargetLandmarks; i++)
    {
    ThinPlateSplineTransformType::InputPointType pointItk_Lps;
    bool pointExists = targetLandmarksItk_Lps->GetPoint(i, &pointItk_Lps);
    if (!pointExists)
      {
      continue;
      }
    double pointVtk_Ras[3]={0};
    pointVtk_Ras[0] = -pointItk_Lps[0];
    pointVtk_Ras[1] = -pointItk_Lps[1];
    pointVtk_Ras[2] =  pointItk_Lps[2];
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
bool vtkITKTransformConverter::SetITKThinPlateSplineTransformFromVTK(vtkObject* loggerObject, itk::Object::Pointer& transformItk_LPS, vtkThinPlateSplineTransform* transformVtk_RAS)
{
  if (transformVtk_RAS==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "Cannot set ITK thin-plate spline transform from VTK: the intput vtkThinPlateSplineTransform is invalid");
    return false;
    }

  // Update is needed because the Basis value and Inverse flag may be out-of-date if the transform depends on its inverse
  transformVtk_RAS->Update();

  if (transformVtk_RAS->GetBasis()!=VTK_RBF_R)
    {
    vtkErrorWithObjectMacro(loggerObject, "Cannot set ITK thin-plate spline transform from VTK: basis function must be R");
    return false;
    }

  ThinPlateSplineTransformType::PointSetType::Pointer sourceLandmarksItk_Lps = ThinPlateSplineTransformType::PointSetType::New();
  vtkPoints* sourceLandmarksVtk_Ras=transformVtk_RAS->GetSourceLandmarks();
  if (sourceLandmarksVtk_Ras!=NULL)
    {
    for (int i=0; i<sourceLandmarksVtk_Ras->GetNumberOfPoints(); i++)
      {
      double posVtk_Ras[3]={0};
      sourceLandmarksVtk_Ras->GetPoint(i, posVtk_Ras);
      ThinPlateSplineTransformType::InputPointType posItk_Lps;
      posItk_Lps[0] = -posVtk_Ras[0];
      posItk_Lps[1] = -posVtk_Ras[1];
      posItk_Lps[2] =  posVtk_Ras[2];
      sourceLandmarksItk_Lps->GetPoints()->InsertElement(i,posItk_Lps);
      }
    }
  ThinPlateSplineTransformType::PointSetType::Pointer targetLandmarksItk_Lps = ThinPlateSplineTransformType::PointSetType::New();
  vtkPoints* targetLandmarksVtk_Ras=transformVtk_RAS->GetTargetLandmarks();
  if (targetLandmarksVtk_Ras!=NULL)
    {
    for (int i=0; i<targetLandmarksVtk_Ras->GetNumberOfPoints(); i++)
      {
      double posVtk_Ras[3]={0};
      targetLandmarksVtk_Ras->GetPoint(i, posVtk_Ras);
      ThinPlateSplineTransformType::InputPointType posItk_Lps;
      posItk_Lps[0] = -posVtk_Ras[0];
      posItk_Lps[1] = -posVtk_Ras[1];
      posItk_Lps[2] =  posVtk_Ras[2];
      targetLandmarksItk_Lps->GetPoints()->InsertElement(i,posItk_Lps);
      }
    }

  if (transformVtk_RAS->GetInverseFlag())
    {
    InverseThinPlateSplineTransformType::Pointer tpsTransformItk = InverseThinPlateSplineTransformType::New();
    tpsTransformItk->SetSourceLandmarks(sourceLandmarksItk_Lps);
    tpsTransformItk->SetTargetLandmarks(targetLandmarksItk_Lps);
    tpsTransformItk->ComputeWMatrix();
    transformItk_LPS = tpsTransformItk;
    }
  else
    {
    ThinPlateSplineTransformType::Pointer tpsTransformItk = ThinPlateSplineTransformType::New();
    tpsTransformItk->SetSourceLandmarks(sourceLandmarksItk_Lps);
    tpsTransformItk->SetTargetLandmarks(targetLandmarksItk_Lps);
    tpsTransformItk->ComputeWMatrix();
    transformItk_LPS = tpsTransformItk;
    }
  return true;
}


//----------------------------------------------------------------------------
vtkAbstractTransform* vtkITKTransformConverter::CreateVTKTransformFromITK(vtkObject* loggerObject, TransformType::Pointer transformItk)
{
  bool conversionSuccess = false;

  // Linear
  vtkNew<vtkMatrix4x4> transformMatrixVtk;
  conversionSuccess = SetVTKLinearTransformFromITK(loggerObject, transformMatrixVtk.GetPointer(), transformItk);
  if (conversionSuccess)
    {
    vtkNew<vtkTransform> linearTransformVtk;
    linearTransformVtk->SetMatrix(transformMatrixVtk.GetPointer());
    linearTransformVtk->Register(NULL);
    return linearTransformVtk.GetPointer();
    }
  // Grid
  vtkNew<vtkOrientedGridTransform> gridTransformVtk;
  conversionSuccess = SetVTKOrientedGridTransformFromITK(loggerObject, gridTransformVtk.GetPointer(), transformItk);
  if (conversionSuccess)
    {
    gridTransformVtk->Register(NULL);
    return gridTransformVtk.GetPointer();
    }
  // BSpline
  vtkNew<vtkOrientedBSplineTransform> bsplineTransformVtk;
  conversionSuccess = SetVTKBSplineFromITKv4(loggerObject, bsplineTransformVtk.GetPointer(), transformItk);
  if (conversionSuccess)
    {
    bsplineTransformVtk->Register(NULL);
    return bsplineTransformVtk.GetPointer();
    }
  // ThinPlateSpline
  vtkNew<vtkThinPlateSplineTransform> tpsTransformVtk;
  conversionSuccess = SetVTKThinPlateSplineTransformFromITK(loggerObject, tpsTransformVtk.GetPointer(), transformItk);
  if (conversionSuccess)
    {
    tpsTransformVtk->Register(NULL);
    return tpsTransformVtk.GetPointer();
    }

  return 0;
}

//----------------------------------------------------------------------------
itk::Object::Pointer vtkITKTransformConverter::CreateITKTransformFromVTK(vtkObject* loggerObject, vtkAbstractTransform* transformVtk, itk::Object::Pointer& secondaryTransformItk, int preferITKv3CompatibleTransforms)
{
  if (transformVtk==NULL)
    {
    vtkErrorWithObjectMacro(loggerObject, "CreateITKTransformFromVTK failed: invalid VTK transform");
    return 0;
    }
  vtkNew<vtkCollection> transformList;
  vtkMRMLTransformNode::FlattenGeneralTransform(transformList.GetPointer(), transformVtk);
  if (transformList->GetNumberOfItems()==0)
    {
    vtkErrorWithObjectMacro(loggerObject, "CreateITKTransformFromVTK failed: invalid VTK transform");
    return 0;
    }

  itk::Object::Pointer primaryTransformItk;
  if (transformList->GetNumberOfItems()==1)
    {
    // Simple, non-composite transform (one item in the input transform)
    vtkObject* singleTransformVtk = transformList->GetItemAsObject(0);
    // Linear
    if (vtkHomogeneousTransform::SafeDownCast(singleTransformVtk))
      {
      vtkHomogeneousTransform* linearTransformVtk = vtkHomogeneousTransform::SafeDownCast(singleTransformVtk);
      vtkMatrix4x4* transformMatrix = linearTransformVtk->GetMatrix();
      if (!SetITKLinearTransformFromVTK(loggerObject, primaryTransformItk, transformMatrix))
        {
        // conversion failed
        return 0;
        }
      return primaryTransformItk;
      }
    // BSpline
    else if (vtkOrientedBSplineTransform::SafeDownCast(singleTransformVtk))
      {
      vtkOrientedBSplineTransform* bsplineTransformVtk = vtkOrientedBSplineTransform::SafeDownCast(singleTransformVtk);
      vtkMatrix4x4* bulkMatrix = bsplineTransformVtk->GetBulkTransformMatrix(); // non-zero for ITKv3 bspline transform only
      if (preferITKv3CompatibleTransforms || (bulkMatrix!=NULL && !IsIdentityMatrix(bulkMatrix)))
        {
        if (!SetITKv3BSplineFromVTK(loggerObject, primaryTransformItk, secondaryTransformItk, bsplineTransformVtk, preferITKv3CompatibleTransforms))
          {
          // conversion failed
          return 0;
          }
        return primaryTransformItk;
        }
      else
        {
        if (!SetITKv4BSplineFromVTK(loggerObject, primaryTransformItk, bsplineTransformVtk))
          {
          // conversion failed
          return 0;
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
        return 0;
        }
      return primaryTransformItk;
      }
    // ThinPlateSpline
    else if (vtkThinPlateSplineTransform::SafeDownCast(singleTransformVtk))
      {
      vtkThinPlateSplineTransform* tpsTransformVtk = vtkThinPlateSplineTransform::SafeDownCast(singleTransformVtk);
      if (!SetITKThinPlateSplineTransformFromVTK(loggerObject, primaryTransformItk, tpsTransformVtk))
        {
        // conversion failed
        return 0;
        }
      return primaryTransformItk;
      }
    else
      {
      if (singleTransformVtk==NULL)
        {
        vtkErrorWithObjectMacro(loggerObject, "vtkITKTransformConverter::CreateITKTransformFromVTK failed: invalid input transform");
        return 0;
        }
      vtkErrorWithObjectMacro(loggerObject, "vtkITKTransformConverter::CreateITKTransformFromVTK failed: conversion of transform type "<<singleTransformVtk->GetClassName()<<" is not supported");
      return 0;
      }
    }
  else
    {
    // Composite transform (more than one items in the input transform)
    // Create an ITK composite transform from the VTK general transform
    CompositeTransformType::Pointer compositeTransformItk = CompositeTransformType::New();
    primaryTransformItk = compositeTransformItk;
    // We need to iterate through the list in reverse order, as ITK CompositeTransform can only append transform on one side
    for (int transformIndex = transformList->GetNumberOfItems()-1; transformIndex>=0; --transformIndex)
      {
      vtkAbstractTransform* singleTransformVtk = vtkAbstractTransform::SafeDownCast(transformList->GetItemAsObject(transformIndex));
      itk::Object::Pointer secondaryTransformItk;
      // We use ITKv4 format (PreferITKv3Transform format is set to false), because
      // legacy ITKv3 code does not know how to interpret composite transforms,
      // and also ITKv3 bspline transform with bulk component cannot be saved in a composite transform
      itk::Object::Pointer singleTransformItk = CreateITKTransformFromVTK(loggerObject, singleTransformVtk, secondaryTransformItk, false );
      if (secondaryTransformItk.IsNotNull())
        {
        vtkErrorWithObjectMacro(loggerObject, "vtkITKTransformConverter::CreateITKTransformFromVTK failed: composite transforms cannot contain legacy transforms (that contains secondary transforms). Do not harden transforms on legacy ITK transforms to avoid this error.");
        return 0;
        }

      CompositeTransformType::TransformType::Pointer singleTransformItkTypeChecked = dynamic_cast< CompositeTransformType::TransformType* >( singleTransformItk.GetPointer() );
      if (singleTransformItkTypeChecked.IsNull())
        {
        vtkErrorWithObjectMacro(loggerObject, "vtkITKTransformConverter::CreateITKTransformFromVTK failed: invalid element found while trying to create a composite transform");
        return 0;
        }
      compositeTransformItk->AddTransform(singleTransformItkTypeChecked.GetPointer());
      }
    return primaryTransformItk;
    }
  return 0;
}

#endif // __vtkITKTransformConverter_h
