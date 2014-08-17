/*=Auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/


#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLBSplineTransformNode.h"
#include "vtkOrientedBSplineTransform.h"
#include "vtkOrientedGridTransform.h"

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkVersion.h>

// ITK includes
#include <itkAffineTransform.h>
#include <itkBSplineDeformableTransform.h> // ITKv3 style
#include <itkBSplineTransform.h> // ITKv4 style
#include <itkCompositeTransform.h>
#include <itkCompositeTransformIOHelper.h>
#include <itkIdentityTransform.h>
#include <itkTransformFileWriter.h>
#include <itkTransformFileReader.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkTranslationTransform.h>
#include <itkScaleTransform.h>

static const unsigned int VTKDimension = 3;

static const int BSPLINE_TRANSFORM_ORDER = 3;

typedef itk::TransformFileReader TransformReaderType;
typedef TransformReaderType::TransformListType TransformListType;
typedef TransformReaderType::TransformType TransformType;

typedef itk::TransformFileWriter TransformWriterType;

typedef itk::VectorImage< double, 3 > GridImageType;

typedef itk::CompositeTransform< double > CompositeTransformType;

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTransformStorageNode);

//----------------------------------------------------------------------------
vtkMRMLTransformStorageNode::vtkMRMLTransformStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLTransformStorageNode::~vtkMRMLTransformStorageNode()
{
}
//----------------------------------------------------------------------------
void vtkMRMLTransformStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLTransformNode");
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::ReadLinearTransform(vtkMRMLNode *refNode)
{
  TransformReaderType::Pointer reader = itk::TransformFileReader::New();
  std::string fullName =  this->GetFullNameFromFileName();
  reader->SetFileName( fullName );
  try
    {
    reader->Update();
    }
  catch (itk::ExceptionObject &exc)
    {
    vtkErrorMacro("ITK exception caught reading transform file: "<< fullName.c_str() << "\n" << exc);
    return 0;
    }
  catch (...)
    {
    vtkErrorMacro("Unknown exception caught while reading transform file: "<< fullName.c_str());
    return 0;
    }

  // For now, grab the first transform from the file.
  TransformListType *transforms = reader->GetTransformList();
  if (transforms->size() == 0)
    {
    vtkErrorMacro("Could not find a transform in file: " << fullName.c_str());
    return 0;
    }
  if (transforms->size() > 1)
    {
    vtkWarningMacro(<< "More than one transform in the file: "<< fullName.c_str()<< ". Using only the first transform.");
    }
  TransformListType::iterator it = (*transforms).begin();
  TransformType::Pointer transform = (*it);
  if (!transform)
    {
    vtkErrorMacro(<< "No transforms in the file: "<< fullName.c_str()<< ", (" << transforms->size() << ")");
    return 0;
    }

  vtkMRMLLinearTransformNode *ltn = vtkMRMLLinearTransformNode::SafeDownCast(refNode);

  static const unsigned int D = VTKDimension;
  typedef itk::MatrixOffsetTransformBase<double,D,D> DoubleLinearTransformType;
  typedef itk::MatrixOffsetTransformBase<float,D,D> FloatLinearTransformType;
  typedef itk::IdentityTransform<double, D> DoubleIdentityTransformType;
  typedef itk::IdentityTransform<float, D> FloatIdentityTransformType;
  typedef itk::ScaleTransform<double, D> DoubleScaleTransformType;
  typedef itk::ScaleTransform<float, D> FloatScaleTransformType;
  typedef itk::TranslationTransform<double, D> DoubleTranslateTransformType;
  typedef itk::TranslationTransform<float, D> FloatTranslateTransformType;

  vtkSmartPointer<vtkMatrix4x4> vtkmat = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkmat->Identity();

  bool convertedToVtkMatrix=false;

  // Linear transform of doubles, dimension 3
  DoubleLinearTransformType::Pointer dlt
    = dynamic_cast<DoubleLinearTransformType*>( transform.GetPointer() );
  if (dlt)
    {
    convertedToVtkMatrix=true;
    for (unsigned int i=0; i < D; i++)
      {
      for (unsigned int j=0; j < D; j++)
        {
        (*vtkmat)[i][j] = dlt->GetMatrix()[i][j];
        }
      (*vtkmat)[i][D] = dlt->GetOffset()[i];
      }
    }

  // Linear transform of floats, dimension 3
  FloatLinearTransformType::Pointer flt
    = dynamic_cast<FloatLinearTransformType*>( transform.GetPointer() );
  if (flt)
    {
    convertedToVtkMatrix=true;
    for (unsigned int i=0; i < D; i++)
      {
      for (unsigned int j=0; j < D; j++)
        {
        (*vtkmat)[i][j] = flt->GetMatrix()[i][j];
        }
      (*vtkmat)[i][D] = flt->GetOffset()[i];
      }
    }

  // Identity transform of doubles, dimension 3
  DoubleIdentityTransformType::Pointer dit
    = dynamic_cast<DoubleIdentityTransformType*>( transform.GetPointer() );
  if (dit)
    {
    // nothing to do, matrix is already the identity
    convertedToVtkMatrix=true;
    }

  // Identity transform of floats, dimension 3
  FloatIdentityTransformType::Pointer fit
    = dynamic_cast<FloatIdentityTransformType*>( transform.GetPointer() );
  if (fit)
    {
    // nothing to do, matrix is already the identity
    convertedToVtkMatrix=true;
    }

  // Scale transform of doubles, dimension 3
  DoubleScaleTransformType::Pointer dst
    = dynamic_cast<DoubleScaleTransformType*>( transform.GetPointer() );
  if (dst)
    {
    convertedToVtkMatrix=true;
    for (unsigned int i=0; i < D; i++)
      {
      (*vtkmat)[i][i] = dst->GetScale()[i];
      }
    }

  // Scale transform of floats, dimension 3
  FloatScaleTransformType::Pointer fst
    = dynamic_cast<FloatScaleTransformType*>( transform.GetPointer() );
  if (fst)
    {
    convertedToVtkMatrix=true;
    for (unsigned int i=0; i < D; i++)
      {
      (*vtkmat)[i][i] = fst->GetScale()[i];
      }
    }

  // Translate transform of doubles, dimension 3
  DoubleTranslateTransformType::Pointer dtt
    = dynamic_cast<DoubleTranslateTransformType*>( transform.GetPointer());
  if (dtt)
    {
    convertedToVtkMatrix=true;
    for (unsigned int i=0; i < D; i++)
      {
      (*vtkmat)[i][D] = dtt->GetOffset()[i];
      }
    }

  // Translate transform of floats, dimension 3
  FloatTranslateTransformType::Pointer ftt
    = dynamic_cast<FloatTranslateTransformType*>( transform.GetPointer() );
  if (ftt)
    {
    convertedToVtkMatrix=true;
    for (unsigned int i=0; i < D; i++)
      {
      (*vtkmat)[i][i] = ftt->GetOffset()[i];
      }
    }

  if (!convertedToVtkMatrix)
    {
    vtkErrorMacro(<< "Could not convert the transform in the file to a linear transform: "<< fullName.c_str());
    return 0;
    }

  // Convert from LPS (ITK) to RAS (Slicer)
  //
  // Tras = lps2ras * Tlps * ras2lps
  //

  vtkSmartPointer<vtkMatrix4x4> lps2ras = vtkSmartPointer<vtkMatrix4x4>::New();
  lps2ras->SetElement(0,0,-1);
  lps2ras->SetElement(1,1,-1);
  vtkMatrix4x4* ras2lps = lps2ras; // lps2ras is diagonal therefore the inverse is identical

  vtkMatrix4x4::Multiply4x4(lps2ras, vtkmat, vtkmat);
  vtkMatrix4x4::Multiply4x4(vtkmat, ras2lps, vtkmat);

  // Convert the sense of the transform (from an ITK resampling
  // transform to a Slicer modeling transform)
  //
  vtkmat->Invert();

  // Set the matrix on the node
  if (ltn->GetReadWriteAsTransformToParent())
    {
    ltn->SetMatrixTransformToParent( vtkmat );
    }
  else
    {
    ltn->SetMatrixTransformFromParent( vtkmat );
    }

  return 1;
}

//----------------------------------------------------------------------------
template <typename T> bool SetVTKBSplineFromITKv3(vtkObject* self,
  vtkOrientedBSplineTransform* bsplineVtk,
  TransformType::Pointer warpTransformItk, TransformType::Pointer bulkTransformItk)
{
  if (bsplineVtk==NULL)
    {
    vtkErrorWithObjectMacro(self, "vtkMRMLTransformStorageNode::SetVTKBSplineFromITK failed: bsplineVtk is invalid");
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
    vtkErrorWithObjectMacro(self,"Unsupported scalar type in BSpline transform file (only float and double are supported)");
    return false;
    }

  typedef itk::BSplineDeformableTransform< T,VTKDimension,BSPLINE_TRANSFORM_ORDER > BSplineTransformType;
  typename BSplineTransformType::Pointer bsplineItk = dynamic_cast< BSplineTransformType* >( warpTransformItk.GetPointer() );
  if (!bsplineItk.GetPointer())
    {
    return false;
    }

  typename BSplineTransformType::ParametersType const& transformFixedParamsItk = bsplineItk->GetFixedParameters();

  // Fixed parameters:
  // * mesh size X, Y, Z (including the BSPLINE_TRANSFORM_ORDER=3 boundary nodes, 1 before and 2 after the grid)
  // * mesh origin X, Y, Z (position of the boundary node before the grid)
  // * mesh spacing X, Y, Z
  // * mesh direction 3x3 matrix (first row, second row, third row)
  const unsigned int expectedNumberOfFixedParameters=VTKDimension*(VTKDimension+3);
  if (expectedNumberOfFixedParameters!=transformFixedParamsItk.size())
    {
    vtkErrorWithObjectMacro(self,"Mismatch in number of BSpline fixed parameters in the transform file and the MRML node");
    return false;
    }

  // Get grid parameters from the fixed parameters
  int gridSize[3]={0};
  gridSize[0]=static_cast<int>(transformFixedParamsItk[0]);
  gridSize[1]=static_cast<int>(transformFixedParamsItk[1]);
  gridSize[2]=static_cast<int>(transformFixedParamsItk[2]);
  double gridSpacing[3]={transformFixedParamsItk[6], transformFixedParamsItk[7], transformFixedParamsItk[8]};
  double gridOrigin_LPS[3]={transformFixedParamsItk[3], transformFixedParamsItk[4], transformFixedParamsItk[5]};
  vtkNew<vtkMatrix4x4> gridDirectionMatrix_LPS;
  int fpIndex=9;
  for (unsigned int row=0; row<VTKDimension; row++)
    {
    for (unsigned int column=0; column<VTKDimension; column++)
      {
      gridDirectionMatrix_LPS->SetElement(row,column,transformFixedParamsItk[fpIndex++]);
      }
    }

  // Set bspline grid and coefficients
  bsplineVtk->SetBorderModeToZero();

  vtkNew<vtkImageData> bsplineCoefficients;

  bsplineCoefficients->SetExtent(0, gridSize[0]-1, 0, gridSize[1]-1, 0, gridSize[2]-1);
  bsplineCoefficients->SetSpacing(gridSpacing);

  vtkNew<vtkMatrix4x4> lpsToRas;
  lpsToRas->SetElement(0,0,-1);
  lpsToRas->SetElement(1,1,-1);

  vtkNew<vtkMatrix4x4> rasToLps;
  rasToLps->SetElement(0,0,-1);
  rasToLps->SetElement(1,1,-1);

  vtkNew<vtkMatrix4x4> gridDirectionMatrix_RAS;

  vtkMatrix4x4::Multiply4x4(lpsToRas.GetPointer(), gridDirectionMatrix_LPS.GetPointer(), gridDirectionMatrix_RAS.GetPointer());
  bsplineVtk->SetGridDirectionMatrix(gridDirectionMatrix_RAS.GetPointer());

  double gridOrigin_RAS[3]={-gridOrigin_LPS[0], -gridOrigin_LPS[1], gridOrigin_LPS[2]};
  bsplineCoefficients->SetOrigin(gridOrigin_RAS);

  int bsplineCoefficientsScalarType=VTK_FLOAT;
  if (isDoublePrecisionInput)
    {
    bsplineCoefficientsScalarType=VTK_DOUBLE;
    }

#if (VTK_MAJOR_VERSION <= 5)
  bsplineCoefficients->SetScalarType(bsplineCoefficientsScalarType);
  bsplineCoefficients->SetNumberOfScalarComponents(3);
  bsplineCoefficients->AllocateScalars();
#else
  bsplineCoefficients->AllocateScalars(bsplineCoefficientsScalarType, 3);
#endif

  const unsigned int expectedNumberOfVectors = gridSize[0]*gridSize[1]*gridSize[2];
  const unsigned int expectedNumberOfParameters = expectedNumberOfVectors*VTKDimension;
  if( bsplineItk->GetNumberOfParameters() != expectedNumberOfParameters )
    {
    vtkErrorWithObjectMacro(self,"Mismatch in number of BSpline parameters in the transform file and the MRML node");
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
#if (VTK_MAJOR_VERSION <= 5)
  bsplineVtk->SetCoefficients(bsplineCoefficients.GetPointer());
#else
  bsplineVtk->SetCoefficientData(bsplineCoefficients.GetPointer());
#endif

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
      vtkErrorWithObjectMacro(self,"Cannot read the 2nd transform in BSplineTransform (expected AffineTransform_double_3_3 or IdentityTransform)" );
      return false;
      }
    }

  // Success
  return true;
}

//----------------------------------------------------------------------------
template <typename T> bool SetVTKBSplineFromITKv4(vtkObject* self,
  vtkOrientedBSplineTransform* bsplineVtk,
  TransformType::Pointer warpTransformItk, TransformType::Pointer bulkTransformItk)
{
  //
  // this version uses the itk::BSplineTransform not the itk::BSplineDeformableTransform
  //
  if (bsplineVtk==NULL)
    {
    vtkErrorWithObjectMacro(self, "vtkMRMLTransformStorageNode::SetVTKBSplineFromITKv4 failed: bsplineVtk is invalid");
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
    vtkErrorWithObjectMacro(self,"Unsupported scalar type in BSpline transform file (only float and double are supported)");
    return false;
    }

  typedef itk::BSplineTransform< T,VTKDimension,BSPLINE_TRANSFORM_ORDER > BSplineTransformType;
  typename BSplineTransformType::Pointer bsplineItk =
    dynamic_cast< BSplineTransformType* >( warpTransformItk.GetPointer() );
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
  int fpIndex=9;
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

#if (VTK_MAJOR_VERSION <= 5)
  bsplineCoefficients->SetScalarType(bsplineCoefficientsScalarType);
  bsplineCoefficients->SetNumberOfScalarComponents(3);
  bsplineCoefficients->AllocateScalars();
#else
  bsplineCoefficients->AllocateScalars(bsplineCoefficientsScalarType, 3);
#endif

  const unsigned int expectedNumberOfVectors = meshSize[0]*meshSize[1]*meshSize[2];
  const unsigned int expectedNumberOfParameters = expectedNumberOfVectors*VTKDimension;
  const unsigned int actualNumberOfParameters = bsplineItk->GetNumberOfParameters();

  if( actualNumberOfParameters != expectedNumberOfParameters )
    {
    vtkErrorWithObjectMacro(self,"Mismatch in number of BSpline parameters in the transform file and the MRML node");
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
#if (VTK_MAJOR_VERSION <= 5)
  bsplineVtk->SetCoefficients(bsplineCoefficients.GetPointer());
#else
  bsplineVtk->SetCoefficientData(bsplineCoefficients.GetPointer());
#endif

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
      vtkErrorWithObjectMacro(self,"Cannot read the 2nd transform in BSplineTransform (expected AffineTransform_double_3_3 or IdentityTransform)" );
      return false;
      }
    }

  // Success
  return true;
}
//----------------------------------------------------------------------------
template <typename T> bool SetITKBSplineFromVTK(vtkObject* self,
  typename itk::Transform<T,VTKDimension,VTKDimension>::Pointer& warpTransformItk,
  typename itk::Transform<T,VTKDimension,VTKDimension>::Pointer& bulkTransformItk,
  vtkOrientedBSplineTransform* bsplineVtk)
{
  if (bsplineVtk==NULL)
    {
    vtkErrorWithObjectMacro(self, "vtkMRMLTransformStorageNode::SetITKBSplineFromVTK failed: bsplineVtk is invalid");
    return false;
    }
#if (VTK_MAJOR_VERSION <= 5)
  vtkImageData* bsplineCoefficients_RAS=bsplineVtk->GetCoefficients();
#else
  vtkImageData* bsplineCoefficients_RAS=bsplineVtk->GetCoefficientData();
#endif
  if (bsplineCoefficients_RAS==NULL)
    {
    vtkErrorWithObjectMacro(self, "Cannot write an inverse BSpline transform to file: coefficients are not specified");
    return false;
    }

  typedef itk::BSplineDeformableTransform< T,VTKDimension,BSPLINE_TRANSFORM_ORDER > BSplineTransformType;
  typename BSplineTransformType::Pointer bsplineItk = BSplineTransformType::New();
  warpTransformItk = bsplineItk;

  // Fixed parameters:
  // * mesh size X, Y, Z (including the BSPLINE_TRANSFORM_ORDER=3 boundary nodes, 1 before and 2 after the grid)
  // * mesh origin X, Y, Z (position of the boundary node before the grid)
  // * mesh spacing X, Y, Z
  // * mesh direction 3x3 matrix (first row, second row, third row)
  typename BSplineTransformType::ParametersType transformFixedParamsItk;
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
    vtkErrorWithObjectMacro(self,"Mismatch in number of BSpline parameters in the ITK transform and the VTK transform");
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

  vtkMatrix4x4* bulkMatrix_RAS=bsplineVtk->GetBulkTransformMatrix();
  if (bulkMatrix_RAS)
    {
    vtkNew<vtkMatrix4x4> bulkMatrix_LPS; // bulk_LPS = rasToLps * bulk_RAS * lpsToRas
    vtkMatrix4x4::Multiply4x4(rasToLps.GetPointer(), bulkMatrix_RAS, bulkMatrix_LPS.GetPointer());
    vtkMatrix4x4::Multiply4x4(bulkMatrix_LPS.GetPointer(), lpsToRas.GetPointer(), bulkMatrix_LPS.GetPointer());

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
int vtkMRMLTransformStorageNode::ReadBSplineTransform(vtkMRMLNode *refNode)
{
  // Note: this method is hard coded to only be used with legacy ITKv3
  // BSpline files.  It creates a vtkOrientedBSpline with unfortunate
  // mathematical properties as described in the vtkOrientedBSpline
  // class description.
  TransformReaderType::Pointer reader = itk::TransformFileReader::New();
  std::string fullName =  this->GetFullNameFromFileName();
  reader->SetFileName( fullName );
  try
    {
    reader->Update();
    }
  catch (itk::ExceptionObject &exc)
    {
    vtkErrorMacro("ITK exception caught reading transform file: "<< fullName.c_str() << "\n" << exc);
    return 0;
    }
  catch (...)
    {
    vtkErrorMacro("Unknown exception caught while reading transform file: "<< fullName.c_str());
    return 0;
    }

  // For now, grab the first two transforms from the file.
  TransformListType *transforms = reader->GetTransformList();
  if (transforms->size() == 0)
    {
    vtkErrorMacro("Could not find a transform in file: " << fullName.c_str());
    return 0;
    }
  if (transforms->size() > 2)
    {
    vtkWarningMacro(<< "More than two transform in the file: "<< fullName.c_str()<< ". Using only the first two transforms.");
    }
  TransformListType::iterator it = (*transforms).begin();
  TransformType::Pointer transform = (*it);
  if (!transform)
    {
    vtkErrorMacro(<< "Invalid transform in the file: "<< fullName.c_str()<< ", (" << transforms->size() << ")");
    return 0;
    }
  ++it;
  TransformType::Pointer transform2=0;
  if( it != (*transforms).end() )
    {
    transform2 = (*it);
    if (!transform2)
      {
      vtkErrorMacro(<< "Invalid transform (2) in the file: "<< fullName.c_str()<< ", (" << transforms->size() << ")");
      return 0;
      }
    }

  vtkMRMLBSplineTransformNode *btn = vtkMRMLBSplineTransformNode::SafeDownCast(refNode);

  vtkNew<vtkOrientedBSplineTransform> bsplineVtk;
  if (SetVTKBSplineFromITKv3<double>(this, bsplineVtk.GetPointer(), transform, transform2)
    || SetVTKBSplineFromITKv3<float>(this, bsplineVtk.GetPointer(), transform, transform2) )
    {
    if (btn->GetReadWriteAsTransformToParent())
      {
      // Convert the sense of the transform (from an ITK resampling
      // transform to a Slicer modeling transform)
      btn->SetAndObserveTransformToParent( bsplineVtk.GetPointer() );
      }
    else
      {
      btn->SetAndObserveTransformFromParent( bsplineVtk.GetPointer() );
      }
    return 1;

    }
  else
    {
    // Log only at debug level because trial-and-error method is used for finding out what node can be retrieved
    // from a transform file
    vtkDebugMacro("Failed to retrieve BSpline transform from file: "<< fullName.c_str());
    return 0;
    }
}

//----------------------------------------------------------------------------
void SetVTKOrientedGridTransformFromITK(vtkObject* self, vtkOrientedGridTransform* grid_Ras, GridImageType::Pointer gridImage_Lps)
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
  unsigned int numberOfScalarComponents = gridImage_Lps->GetVectorLength();
  if (numberOfScalarComponents!=VTKDimension)
    {
    vtkErrorWithObjectMacro(self, "Cannot load grid transform: the input displacement field expected to contain "
      << VTKDimension << " components but it actually contains " << numberOfScalarComponents );
    return;
    }
#if (VTK_MAJOR_VERSION <= 5)
  gridImage_Ras->SetNumberOfScalarComponents( numberOfScalarComponents );
  gridImage_Ras->SetScalarTypeToDouble();
  gridImage_Ras->AllocateScalars();
#else
  gridImage_Ras->AllocateScalars(VTK_DOUBLE, 3);
#endif

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

#if (VTK_MAJOR_VERSION <= 5)
  grid_Ras->SetDisplacementGrid( gridImage_Ras.GetPointer() );
#else
  grid_Ras->SetDisplacementGridData( gridImage_Ras.GetPointer() );
#endif

  // Set the interpolation to cubic to have smooth derivatives
  grid_Ras->SetInterpolationModeToCubic();
}

//----------------------------------------------------------------------------
void SetITKOrientedGridTransformFromVTK(vtkObject* self, vtkOrientedGridTransform* grid_Ras, GridImageType::Pointer &gridImage_Lps)
{
  if (grid_Ras==NULL)
    {
    vtkErrorWithObjectMacro(self, "Cannot save grid transform: the input vtkOrientedGridTransform is invalid");
    return;
    }
  vtkImageData* gridImage_Ras = grid_Ras->GetDisplacementGrid();
  if (gridImage_Ras==NULL)
    {
    vtkErrorWithObjectMacro(self, "Cannot save grid transform: the input vtkOrientedGridTransform does not contain a valid displacement grid");
    return;
    }
  if (gridImage_Ras->GetNumberOfScalarComponents()!=VTKDimension)
    {
    vtkErrorWithObjectMacro(self, "Cannot save grid transform: the input vtkOrientedGridTransform expected to contain "
      << VTKDimension << " components but it actually contains " << gridImage_Ras->GetNumberOfScalarComponents() );
    return;
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
  gridImage_Lps->SetVectorLength( VTKDimension );
  GridImageType::IndexType start;
  start[0] = start[1] = start[2] = 0;
  int* Nijk = gridImage_Ras->GetDimensions();
  GridImageType::SizeType size;
  size[0] = Nijk[0]; size[1] = Nijk[1]; size[2] = Nijk[2];
  GridImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( start );
  gridImage_Lps->SetRegions( region );
  gridImage_Lps->SetVectorLength( VTKDimension );
  gridImage_Lps->Allocate();
  double* displacementVectors_Ras = reinterpret_cast<double*>(gridImage_Ras->GetScalarPointer());
  itk::ImageRegionIterator<GridImageType> gridImageIt_Lps(gridImage_Lps, region);
  gridImageIt_Lps.GoToBegin();
  GridImageType::PixelType displacementVectorLps;
  displacementVectorLps.SetSize(VTKDimension);
  double displacementScale = grid_Ras->GetDisplacementScale();
  double displacementShift = grid_Ras->GetDisplacementShift();
  while( !gridImageIt_Lps.IsAtEnd() )
    {
    displacementVectorLps[0] = -( displacementScale * (*(displacementVectors_Ras++)) + displacementShift );
    displacementVectorLps[1] = -( displacementScale * (*(displacementVectors_Ras++)) + displacementShift );
    displacementVectorLps[2] =  ( displacementScale * (*(displacementVectors_Ras++)) + displacementShift );
    gridImageIt_Lps.Set(displacementVectorLps);
    ++gridImageIt_Lps;
    }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::ReadGridTransform(vtkMRMLNode *refNode)
{
  vtkMRMLTransformNode *tn = vtkMRMLTransformNode::SafeDownCast(refNode);
  if (tn==NULL)
    {
    vtkErrorMacro("vtkMRMLTransformStorageNode::ReadGridTransform failed: expected a transform node as input");
    return 0;
    }

  // Grid transforms are not currently supported as ITK transforms but
  // rather as vector images. This is subject to change whereby an ITK transform
  // for displacement fields will provide a standard transform API
  // but will reference a vector image to store the displacements.

  // As a grid transform is not a itk::Transform, we do not read it
  // by using itk::TransformFileReader (as it is done for other transforms)
  // It is instead transferred as an itk::VectorImage.

  GridImageType::Pointer gridImage_Lps = 0;

  typedef itk::ImageFileReader< GridImageType >  ReaderType;
  std::string fullName =  this->GetFullNameFromFileName();
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( fullName );
  try
    {
    reader->Update();
    gridImage_Lps = reader->GetOutput();

    if( gridImage_Lps->GetVectorLength() != 3 )
      {
      vtkErrorMacro( "The deformable vector field must contain 3-D vectors;"
                     " instead, it contains " << gridImage_Lps->GetVectorLength()
                     << "-D vectors\n" );
      return 0;
      }
    }
  catch (itk::ExceptionObject &
#ifndef NDEBUG
         exc
#endif
        )
    {
    // File specified may not contain a grid image. Can we safely
    // error out quitely?
    vtkDebugMacro("ITK exception caught reading grid transform image file: " << fullName.c_str() << "\n" << exc);
    return 0;
    }
  catch (...)
    {
    vtkErrorMacro("Unknown exception caught while reading grid transform image file: " << fullName.c_str());
    return 0;
    }

  if (!gridImage_Lps)
    {
      vtkErrorMacro("Failed to read image as a grid transform from file: " << fullName.c_str());
      return 0;
    }

  vtkNew<vtkOrientedGridTransform> gridTransform_Ras;
  SetVTKOrientedGridTransformFromITK(this, gridTransform_Ras.GetPointer(), gridImage_Lps);
  // Set the matrix on the node
  if (tn->GetReadWriteAsTransformToParent())
    {
    tn->SetAndObserveTransformToParent( gridTransform_Ras.GetPointer() );
    }
  else
    {
    tn->SetAndObserveTransformFromParent( gridTransform_Ras.GetPointer() );
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::ReadCompositeTransform(vtkMRMLNode *refNode)
{
  // Note: this method reads composite transforms (as of Slicer 4.4, these will
  // come from .h5 files).  While the format of h5 files can be very general,
  // here we only deal with certain combinations that are used by existing
  // CLI code (BRAINSFit, for example)
  TransformReaderType::Pointer reader = TransformReaderType::New();
  std::string fullName =  this->GetFullNameFromFileName();
  reader->SetFileName( fullName );
  try
    {
    reader->Update();
    }
  catch (itk::ExceptionObject &exc)
    {
    vtkErrorMacro("ITK exception caught reading transform file: "<< fullName.c_str() << "\n" << exc);
    return 0;
    }
  catch (...)
    {
    vtkErrorMacro("Unknown exception caught while reading transform file: "<< fullName.c_str());
    return 0;
    }

  // Based on personal communication with the ITK developer group (Brad and Hans) the
  // transform list will contain only one transform, which is a composite transform.
  TransformListType *transforms = reader->GetTransformList();
  TransformListType::iterator transformsIT = (*transforms).begin();
  TransformType *compositeTransform = (*transformsIT);

  // the composite transform is itself a list of transforms.  There is a
  // helper class in ITK to convert the internal transform list into a
  // list that is possible to iterate over.  So we get this transformList.
  typedef const itk::CompositeTransformIOHelper::TransformType ComponentTransformType;
  itk::CompositeTransformIOHelper compositeTransformIOHelper;

  typedef itk::CompositeTransformIOHelper::ConstTransformListType ConstTransformListType;
  ConstTransformListType transformList =
    compositeTransformIOHelper.GetTransformList(compositeTransform);

  ConstTransformListType::iterator compositeIT = transformList.begin();
  ++compositeIT; // skip first element of list, which is duplicate of individual
                 // transforms that are included in the list


  // Now we have the iterator (compositeIT) pointing to the first real transform
  // in the list.  While in the general case this could itself be a composite transform
  // and create a hierarchy, for now the only existing use case that we will support
  // is a pair of transforms that a linear and a bspline, that are just like the
  // ones that had been stored in the tfm bspline files from ITKv3.

  TransformType::Pointer componentTransform0, componentTransform1;
  componentTransform0 = const_cast<TransformType *>((*compositeIT).GetPointer());
  ++compositeIT;
  componentTransform1 = const_cast<TransformType *>((*compositeIT).GetPointer());

  if( !componentTransform0 || !componentTransform1 )
    {
    vtkErrorMacro("Could not find valid transforms in composite file: "<< fullName.c_str());
    return 0;
    }

  vtkMRMLBSplineTransformNode *btn = vtkMRMLBSplineTransformNode::SafeDownCast(refNode);

  vtkNew<vtkOrientedBSplineTransform> bsplineVtk;
  if (SetVTKBSplineFromITKv4<double>(this, bsplineVtk.GetPointer(), componentTransform1, componentTransform0)
    || SetVTKBSplineFromITKv4<float>(this, bsplineVtk.GetPointer(), componentTransform1, componentTransform0) )
    {
    if (btn->GetReadWriteAsTransformToParent())
      {
      // Convert the sense of the transform (from an ITK resampling
      // transform to a Slicer modeling transform)
      btn->SetAndObserveTransformToParent( bsplineVtk.GetPointer() );
      }
    else
      {
      btn->SetAndObserveTransformFromParent( bsplineVtk.GetPointer() );
      }
    return 1;
    }
  else
    {
    // Log only at debug level because trial-and-error method is used for finding out
    // what node can be retrieved from a transform file
    vtkDebugMacro("Failed to retrieve BSpline transform from file: "<< fullName.c_str());
    return 0;
    }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName =  this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  if (refNode->IsA("vtkMRMLGridTransformNode"))
    {
    return ReadGridTransform(refNode);
    }
  else if (refNode->IsA("vtkMRMLBSplineTransformNode"))
    {
    int returnCode = ReadBSplineTransform(refNode);
    if (returnCode == 0)
      {
      return ReadCompositeTransform(refNode);
      }
    }
  else if (refNode->IsA("vtkMRMLLinearTransformNode"))
    {
    return ReadLinearTransform(refNode);
    }

  vtkErrorMacro("ReadData: failed, transform node type is not supported for reading");
  return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::WriteLinearTransform(vtkMRMLLinearTransformNode *ln)
{
  vtkSmartPointer<vtkMatrix4x4> lps2ras = vtkSmartPointer<vtkMatrix4x4>::New();
  lps2ras->SetElement(0,0,-1);
  lps2ras->SetElement(1,1,-1);
  vtkMatrix4x4* ras2lps = lps2ras; // lps2ras is diagonal therefore the inverse is identical

  typedef itk::AffineTransform<double, VTKDimension> AffineTransformType;
  AffineTransformType::Pointer affine = AffineTransformType::New();

  vtkSmartPointer<vtkMatrix4x4> mat2parent=vtkSmartPointer<vtkMatrix4x4>::New();
  ln->GetMatrixTransformToParent(mat2parent);

  // Convert from RAS (Slicer) to LPS (ITK)
  //
  // Tlps = ras2lps * Tras * lps2ras
  //
  vtkSmartPointer<vtkMatrix4x4> vtkmat = vtkSmartPointer<vtkMatrix4x4>::New();

  vtkMatrix4x4::Multiply4x4(ras2lps, mat2parent, vtkmat);
  vtkMatrix4x4::Multiply4x4(vtkmat, lps2ras, vtkmat);

  if (ln->GetReadWriteAsTransformToParent())
    {
    // Convert the sense of the transform (from a Slicer modeling
    // transform to an ITK resampling transform)
    //
    vtkmat->Invert();
    }

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

  affine->SetMatrix(itkmat);
  affine->SetOffset(itkoffset);

  TransformWriterType::Pointer writer = TransformWriterType::New();
  writer->SetInput( affine );
  std::string fullName =  this->GetFullNameFromFileName();
  writer->SetFileName( fullName );
  try
    {
    writer->Update();
    }
  catch (itk::ExceptionObject &exc)
    {
    vtkErrorMacro("ITK exception caught writing transform file: "
                  << fullName.c_str() << "\n" << exc);
    return 0;
    }
  catch (...)
    {
    vtkErrorMacro("Unknown exception caught while writing transform file: "
                  << fullName.c_str());
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::WriteBSplineTransform(vtkMRMLBSplineTransformNode *bsplineTransformNode)
{
  vtkOrientedBSplineTransform* bsplineVtk = NULL;

  if (bsplineTransformNode->GetReadWriteAsTransformToParent())
    {
    bsplineVtk=vtkOrientedBSplineTransform::SafeDownCast(bsplineTransformNode->GetTransformToParentAs("vtkOrientedBSplineTransform"));
    }
  else
    {
    bsplineVtk=vtkOrientedBSplineTransform::SafeDownCast(bsplineTransformNode->GetTransformFromParentAs("vtkOrientedBSplineTransform"));
    }

  if (bsplineVtk==NULL)
    {
    vtkErrorMacro("Cannot retrieve BSpline transform from node");
    return 0;
    }

  // Update is needed bacause it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
  bsplineVtk->Update();
  if (bsplineVtk->GetInverseFlag())
    {
    vtkErrorMacro("Cannot write an inverse BSpline transform to file");
    return 0;
    }

#if (VTK_MAJOR_VERSION <= 5)
  vtkImageData* bsplineCoefficients=bsplineVtk->GetCoefficients();
#else
  vtkImageData* bsplineCoefficients=bsplineVtk->GetCoefficientData();
#endif

  if (bsplineCoefficients==NULL)
    {
    vtkErrorMacro("Cannot write an inverse BSpline transform to file: coefficients are not specified");
    return 0;
    }

  TransformWriterType::Pointer writer = TransformWriterType::New();

  if (bsplineCoefficients->GetScalarType()==VTK_FLOAT)
    {
    typedef itk::Transform<float, VTKDimension, VTKDimension > ITKTransformType;
    ITKTransformType::Pointer warpTransformItk;
    ITKTransformType::Pointer bulkTransformItk;
    if (!SetITKBSplineFromVTK<float>(this, warpTransformItk, bulkTransformItk, bsplineVtk))
      {
      vtkErrorMacro("Cannot write an inverse BSpline transform to file");
      return 0;
      }
    writer->SetInput( warpTransformItk );
    if( bulkTransformItk )
      {
      writer->AddTransform( bulkTransformItk );
      }
    }
  else if (bsplineCoefficients->GetScalarType()==VTK_DOUBLE)
    {
    typedef itk::Transform<double, VTKDimension, VTKDimension > ITKTransformType;
    ITKTransformType::Pointer warpTransformItk;
    ITKTransformType::Pointer bulkTransformItk;
    if (!SetITKBSplineFromVTK<double>(this, warpTransformItk, bulkTransformItk, bsplineVtk))
      {
      vtkErrorMacro("Cannot write an inverse BSpline transform to file");
      return 0;
      }
    writer->SetInput( warpTransformItk );
    if( bulkTransformItk )
      {
      writer->AddTransform( bulkTransformItk );
      }
    }
  else
    {
    vtkErrorMacro("Cannot write an inverse BSpline transform to file: only float and double coefficient types are supported");
    return 0;
    }

  std::string fullName =  this->GetFullNameFromFileName();
  writer->SetFileName( fullName );
  try
    {
    writer->Update();
    }
  catch (itk::ExceptionObject &exc)
    {
    vtkErrorMacro("ITK exception caught writing transform file: "
                  << fullName.c_str() << "\n" << exc);
    return 0;
    }
  catch (...)
    {
    vtkErrorMacro("Unknown exception caught while writing transform file: "
                  << fullName.c_str());
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::WriteGridTransform(vtkMRMLGridTransformNode *gd)
{
  vtkOrientedGridTransform* gridTransform_Ras = NULL;

  if (gd->GetReadWriteAsTransformToParent())
    {
    gridTransform_Ras=vtkOrientedGridTransform::SafeDownCast(gd->GetTransformToParentAs("vtkOrientedGridTransform"));
    }
  else
    {
    gridTransform_Ras=vtkOrientedGridTransform::SafeDownCast(gd->GetTransformFromParentAs("vtkOrientedGridTransform"));
    }

  if (gridTransform_Ras==NULL)
    {
    vtkErrorMacro("Cannot retrieve grid transform from node");
    return 0;
    }

  // Update is needed bacause it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
  gridTransform_Ras->Update();
  if (gridTransform_Ras->GetInverseFlag())
    {
    vtkErrorMacro("Cannot write an inverse grid transform to file");
    return 0;
    }

  GridImageType::Pointer gridImage_Lps;
  SetITKOrientedGridTransformFromVTK(this, gridTransform_Ras, gridImage_Lps);

  itk::ImageFileWriter<GridImageType>::Pointer writer = itk::ImageFileWriter<GridImageType>::New();
  writer->SetInput( gridImage_Lps );
  std::string fullName =  this->GetFullNameFromFileName();
  writer->SetFileName( fullName );
  try
    {
    writer->Update();
    }
  catch (itk::ExceptionObject &exc)
    {
    vtkErrorMacro("Failed to save grid transform to file: " << fullName.c_str()
      << ". Make sure a 'Displacement field' format is selected for saving."
      << "ITK exception caught writing transform file: \n" << exc);
    return 0;
    }
  catch (...)
    {
    vtkErrorMacro("Unknown exception caught while writing transform file: "
                  << fullName.c_str());
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName =  this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLTransformNode: File name not specified");
    return 0;
    }

  vtkMRMLLinearTransformNode *ln = vtkMRMLLinearTransformNode::SafeDownCast(refNode);
  if (ln!=NULL)
  {
    return WriteLinearTransform(ln);
  }
  vtkMRMLBSplineTransformNode *bs = vtkMRMLBSplineTransformNode::SafeDownCast(refNode);
  if (bs!=NULL)
  {
    return WriteBSplineTransform(bs);
  }
  vtkMRMLGridTransformNode *gd = vtkMRMLGridTransformNode::SafeDownCast(refNode);
  if (gd!=NULL)
  {
    return WriteGridTransform(gd);
  }

  vtkErrorMacro("Writing of the transform node to file failed: unsupported node type");
  return 0;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Transform (.h5)");
  this->SupportedWriteFileTypes->InsertNextValue("Transform (.mat)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
  this->SupportedWriteFileTypes->InsertNextValue("Transform (.*)");
  this->SupportedWriteFileTypes->InsertNextValue("Displacement field (.nrrd)");
  this->SupportedWriteFileTypes->InsertNextValue("Displacement field (.nhdr)");
  this->SupportedWriteFileTypes->InsertNextValue("Displacement field (.mha)");
  this->SupportedWriteFileTypes->InsertNextValue("Displacement field (.mhd)");
  this->SupportedWriteFileTypes->InsertNextValue("Displacement field (.nii)");
  this->SupportedWriteFileTypes->InsertNextValue("Displacement field (.nii.gz)");
}
//----------------------------------------------------------------------------
const char* vtkMRMLTransformStorageNode::GetDefaultWriteFileExtension()
{
  return "h5";
}
