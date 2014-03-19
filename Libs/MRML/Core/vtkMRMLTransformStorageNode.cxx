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

// VTKITK includes
#include <vtkITKBSplineTransform.h>

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkGridTransform.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>

// ITK includes
#include <itkTransformFileWriter.h>
#include <itkTransformFileReader.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkTranslationTransform.h>
#include <itkScaleTransform.h>

static const int VTKDimension = 3;

typedef itk::TransformFileReader TransformReader;
typedef TransformReader::TransformListType TransformListType;
typedef TransformReader::TransformType TransformType;

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
  TransformReader::Pointer reader = itk::TransformFileReader::New();
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

  static const int D = VTKDimension;
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
    for (int i=0; i < D; i++)
      {
      for (int j=0; j < D; j++)
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
    for (int i=0; i < D; i++)
      {
      for (int j=0; j < D; j++)
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
    for (int i=0; i < D; i++)
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
    for (int i=0; i < D; i++)
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
    for (int i=0; i < D; i++)
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
    for (int i=0; i < D; i++)
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

  // Set the matrix on the node
  if (ltn->GetReadWriteAsTransformToParent())
    {
    // Convert the sense of the transform (from an ITK resampling
    // transform to a Slicer modeling transform)
    //
    vtkmat->Invert();
    ltn->SetMatrixTransformToParent( vtkmat );
    }
  else
    {
    ltn->SetMatrixTransformFromParent( vtkmat );
    }

  return 1;
}

//----------------------------------------------------------------------------
template <typename T> bool SetVTKBSplineFromITK(vtkObject* self, vtkITKBSplineTransform* bsplineVtk, TransformType::Pointer bsplineItk, TransformType::Pointer bulkItk)
{
  typedef itk::BSplineDeformableTransform< T,VTKDimension,VTKDimension > BSplineTransformType;

  typename BSplineTransformType::Pointer bst = dynamic_cast< BSplineTransformType* >( bsplineItk.GetPointer() );
  if (!bst)
    {
    return false;
    }

  bsplineVtk->SetSplineOrder( 3 );
  typename BSplineTransformType::ParametersType const& fp = bst->GetFixedParameters();
  bsplineVtk->SetFixedParameters( fp.data_block(), fp.GetSize() );

  if( bst->GetNumberOfParameters() != bsplineVtk->GetNumberOfParameters() )
    {
    vtkErrorWithObjectMacro(self,"Mismatch in number of BSpline parameters in the transform file and the MRML node");
    return 0;
    }

  typename BSplineTransformType::ParametersType const& param = bst->GetParameters();
  bsplineVtk->SetParameters( param.data_block() );

  if( bulkItk )
    {
    typedef vtkITKBSplineTransform::BulkTransformType BulkTransformType;
    BulkTransformType* bulkItkAffine = dynamic_cast<BulkTransformType*> (bulkItk.GetPointer());
    if (!bulkItkAffine)
      {
      vtkErrorWithObjectMacro(self,"Cannot read the 2nd transform in BSplineTransform (expected AffineTransform_double_3_3)" );
      return false;
      }
    double linear[VTKDimension][VTKDimension];
    double offset[VTKDimension];
    for (int i=0; i < VTKDimension; i++)
      {
      for (int j=0; j < VTKDimension; j++)
        {
        linear[i][j] = bulkItkAffine->GetMatrix()[i][j];
        }
      offset[i] = bulkItkAffine->GetOffset()[i];
      }
    bsplineVtk->SetBulkTransform( linear, offset );
    }

  // Convert from LPS (in ITK tansformation file) to RAS (Slicer)
  bsplineVtk->SetSwitchCoordinateSystem( true );

  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::ReadBSplineTransform(vtkMRMLNode *refNode)
{
  TransformReader::Pointer reader = itk::TransformFileReader::New();
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

  vtkNew<vtkITKBSplineTransform> bsplineVtk;

  if (SetVTKBSplineFromITK<double>(this, bsplineVtk.GetPointer(), transform, transform2)
    || SetVTKBSplineFromITK<float>(this, bsplineVtk.GetPointer(), transform, transform2) )
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
int vtkMRMLTransformStorageNode::ReadGridTransform(vtkMRMLNode *refNode)
{
  // Grid transforms are not currently supported as ITK transforms but
  // rather as vector images. This is subject to change whereby an ITK transform
  // for displacement fields will provide a standard transform API
  // but will reference a vector image to store the displacements.

  // As a grid transform is not a itk::Transform, we do not read it
  // by using itk::TransformFileReader (as it is done for other transforms)
  // It is instead transferred as an itk::VectorImage.

  typedef itk::VectorImage< double, 3 >   GridImageType;
  GridImageType::Pointer gridImage = 0;

  typedef itk::ImageFileReader< GridImageType >  ReaderType;
  std::string fullName =  this->GetFullNameFromFileName();
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( fullName );
  try
    {
    reader->Update();
    gridImage = reader->GetOutput();

    if( gridImage->GetVectorLength() != 3 )
      {
      vtkErrorMacro( "The deformable vector field must contain 3-D vectors;"
                     " instead, it contains " << gridImage->GetVectorLength()
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
    vtkDebugMacro("ITK exception caught reading grid transform image file: "
                  << fullName.c_str() << "\n" << exc);

    return 0;
    }
  catch (...)
    {
    vtkErrorMacro("Unknown exception caught while reading grid transform image file: "
                  << fullName.c_str());
    return 0;
    }

  if (!gridImage)
    {
      vtkErrorMacro("Failed to read image as a grid transform from file: " << fullName.c_str());
      return 0;
    }

  // Convert the grid image to the appropriate type of VTK
  // transform

  vtkMRMLGridTransformNode *gtn = vtkMRMLGridTransformNode::SafeDownCast(refNode);

  vtkNew<vtkGridTransform> vtkgrid;
  vtkNew<vtkImageData> vtkgridimage;

  vtkgrid->SetInterpolationModeToCubic();

  //GridImageType::IndexType index
  //  = gridImage->GetBufferedRegion().GetIndex();
  GridImageType::SizeType size
    = gridImage->GetBufferedRegion().GetSize();

  unsigned const int Ni = size[0];
  unsigned const int Nj = size[1];
  unsigned const int Nk = size[2];
  unsigned const int Nc = gridImage->GetVectorLength();

  vtkgridimage->Initialize();

  // Convert from LPS (ITK) to RAS (Slicer)
  //
  // The conversion logic is as follows.
  //
  // The LPS to RAS (and back) conversion is the linear transform
  // given by the matrix
  //   C = [ -1 0 0; 0 -1 0; 0 0 1 ]
  //
  // Let o be the origin of the ITK grid transform, and s be the
  // spacing of the ITK grid transform.  Then a pixel coordinate p
  // on the ITK grid represents the physical point
  //   x =  Diag(s) * p + o,
  // where Diag(v) is the diagonal matrix with the vector v on the
  // diagonal.  Since x is an ITK physical point, it is in the LPS
  // coordinate system.  The corresponding point, y, in the RAS
  // system is given by
  //    y = C * x
  // This gives
  //    y = C * Diag(s) * p + C * o
  //      = Diag(s) * C * p + C * o, since C is also a diagonal matrix
  //      = Diag(s) * ( C * p + p0 ) + ( C * o - Diag(s) * p0 )
  //        ( this converts RAS's pixel coordinate range from
  //          [ -Ni + 1, -Nj + 1, 0] ~ [ 0, 0, Nk - 1 ] to
  //          [ 0, 0, 0 ] ~ [ Ni - 1, Nj - 1, Nk - 1 ] )
  //        ( p0 = [ Ni - 1, Nj - 1, 0 ]' )
  //      = Diag(s2) * p2 + o2
  // Therefore,
  //    new spacing:
  //        s2 = s
  //    new pixel coordinate:
  //        p2 = C * p + p0 = [ -p(1) + Ni - 1, -p(2) + Nj - 1, p(3) ]'
  //    new origin:
  //        o2 = [ -o(1) - s(1)*(Ni-1), -o(2) - s(2)*(Nj-1), o(3) ]'
  //
  // Also, the value at each grid point is a displacement in
  // the LPS coordinate system, and needs to be converted too.
  // E.g. a displacement d takes a physical point x1 to a physical
  // point x2, so that
  //      x2 = x1 + d
  // We require the corresponding displacement d2 that takes
  // y1 = C*x1 to y2 = C*x2, giving us
  //      d2 = y2-y1 = C*(x2-x1) = C*d
  //
  // Thus
  //     d2 = [ -d(1), -d(2), d(3) ]

  GridImageType::SpacingType spacing = gridImage->GetSpacing();
  vtkgridimage->SetOrigin( -gridImage->GetOrigin()[0] - spacing[0] * (Ni-1),
                           -gridImage->GetOrigin()[1] - spacing[1] * (Nj-1),
                            gridImage->GetOrigin()[2] );
  vtkgridimage->SetSpacing( spacing.GetDataPointer() );

  if (! (gridImage->GetDirection()(0,0) == 1 &&
         gridImage->GetDirection()(0,1) == 0 &&
         gridImage->GetDirection()(0,2) == 0 &&
         gridImage->GetDirection()(1,0) == 0 &&
         gridImage->GetDirection()(1,1) == 1 &&
         gridImage->GetDirection()(1,2) == 0 &&
         gridImage->GetDirection()(2,0) == 0 &&
         gridImage->GetDirection()(2,1) == 0 &&
         gridImage->GetDirection()(2,2) == 1) )
    {
    vtkErrorMacro( "Grid transform with a non-identity orientation matrix is not yet implemented" );
    // TODO: implement support for direction. Two options:
    // Option A: create a new "vtkOrientedGridTransform" class that contains an additional linear component
    // Option B: create a vtkGeneralTransform that contains a linear and a grid transform concatenated
    return 0;
    }

  vtkgridimage->SetDimensions( Ni, Nj, Nk );
  vtkgridimage->SetNumberOfScalarComponents( Nc );
  vtkgridimage->SetScalarTypeToDouble();
  vtkgridimage->AllocateScalars();

  // convert each vector in the displacement field from LPS to RAS
  double* dataPtr = reinterpret_cast<double*>(vtkgridimage->GetScalarPointer());
  GridImageType::IndexType ijk;
  for( int k = 0; k < (int)Nk; ++k )
    {
    ijk[2] = k;
    for( int j = 0; j < (int)Nj; ++j )
      {
      ijk[1] = Nj -j - 1;
      for( int i = 0; i < (int)Ni; ++i, dataPtr += 3 )
        {
        ijk[0] = Ni -i - 1;
        GridImageType::PixelType pixel = gridImage->GetPixel( ijk );
        // negate the first two components
        dataPtr[0] = -pixel[0];
        dataPtr[1] = -pixel[1];
        dataPtr[2] = pixel[2];
        }
      }
    }

  vtkgrid->SetDisplacementGrid( vtkgridimage.GetPointer() );

  // Set the matrix on the node
  if (gtn->GetReadWriteAsTransformToParent())
    {
    // Convert the sense of the transform (from an ITK resampling
    // transform to a Slicer modeling transform)
    gtn->SetAndObserveTransformToParent( vtkgrid.GetPointer() );
    }
  else
    {
    gtn->SetAndObserveTransformFromParent( vtkgrid.GetPointer() );
    }

  return 1;
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
    return ReadBSplineTransform(refNode);
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

  for (int i=0; i < VTKDimension; i++)
    {
    for (int j=0; j < VTKDimension; j++)
      {
      itkmat[i][j] = (*vtkmat)[i][j];
      }
    itkoffset[i] = (*vtkmat)[i][VTKDimension];
    }

  affine->SetMatrix(itkmat);
  affine->SetOffset(itkoffset);

  itk::TransformFileWriter::Pointer writer = itk::TransformFileWriter::New();
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
int vtkMRMLTransformStorageNode::WriteBSplineTransform(vtkMRMLBSplineTransformNode *bs)
{
  vtkITKBSplineTransform* vtkTrans = NULL;

  if (bs->GetReadWriteAsTransformToParent())
    {
    vtkTrans=vtkITKBSplineTransform::SafeDownCast(bs->GetTransformToParentAs("vtkITKBSplineTransform"));
    }
  else
    {
    vtkTrans=vtkITKBSplineTransform::SafeDownCast(bs->GetTransformFromParentAs("vtkITKBSplineTransform"));
    }

  if (vtkTrans==NULL)
    {
    vtkErrorMacro("Cannot retrieve BSpline transform from node");
    return 0;
    }

  // Update is needed bacause it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
  vtkTrans->Update();
  if (vtkTrans->GetInverseFlag())
    {
    vtkErrorMacro("Cannot write an inverse BSpline transform to file");
    return 0;
    }

  // get the itkBSplineDeformableTransform directly. No need to
  // convert the coordinate from RAS to LPS.
  typedef itk::Transform<double, VTKDimension, VTKDimension > ITKTransformType;

  ITKTransformType::Pointer itkTrans = vtkTrans->GetITKTransform();
  vtkITKBSplineTransform::BulkTransformType const* bulk = vtkTrans->GetBulkTransform();
  itk::TransformFileWriter::Pointer writer = itk::TransformFileWriter::New();
  writer->SetInput( itkTrans );
  if( bulk )
    {
    writer->AddTransform( bulk );
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
  vtkGridTransform* vtkTrans = NULL;

  if (gd->GetReadWriteAsTransformToParent())
    {
    vtkTrans=vtkGridTransform::SafeDownCast(gd->GetTransformToParentAs("vtkGridTransform"));
    }
  else
    {
    vtkTrans=vtkGridTransform::SafeDownCast(gd->GetTransformFromParentAs("vtkGridTransform"));
    }

  if (vtkTrans==NULL)
    {
    vtkErrorMacro("Cannot retrieve grid transform from node");
    return 0;
    }

  // Update is needed bacause it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
  vtkTrans->Update();
  if (vtkTrans->GetInverseFlag())
    {
    vtkErrorMacro("Cannot write an inverse grid transform to file");
    return 0;
    }

  vtkImageData* vtkgridimage = vtkTrans->GetDisplacementGrid();

  // initialize the vector image
  typedef itk::VectorImage< double, VTKDimension > GridType;
  GridType::Pointer gridImage = GridType::New();
  gridImage->SetVectorLength( VTKDimension );
  GridType::IndexType start;
  start[0] = start[1] = start[2] = 0;
  int* Nijk = vtkgridimage->GetDimensions();
  GridType::SizeType size;
  size[0] = Nijk[0]; size[1] = Nijk[1]; size[2] = Nijk[2];
  GridType::RegionType region;
  region.SetSize( size );
  region.SetIndex( start );
  gridImage->SetRegions( region );
  gridImage->SetVectorLength( VTKDimension );

  // convert the coordinate from RAS to LPS.
  GridType::SpacingType spacing( vtkgridimage->GetSpacing() );
  gridImage->SetSpacing( spacing );
  double* origin = vtkgridimage->GetOrigin();
  origin[0] = -origin[0] - spacing[0] * (Nijk[0]-1);
  origin[1] = -origin[1] - spacing[1] * (Nijk[1]-1);
  gridImage->SetOrigin( origin );
  gridImage->Allocate();

  double* dataPtr = reinterpret_cast<double*>(vtkgridimage->GetScalarPointer());
  GridType::IndexType ijk;
  GridType::PixelType pixel(3);
  for( int k = 0; k < Nijk[2]; ++k )
    {
    ijk[2] = k;
    for( int j = 0; j < Nijk[1]; ++j )
      {
      ijk[1] = -j + Nijk[1] - 1;
      for( int i = 0; i < Nijk[0]; ++i, dataPtr += 3 )
        {
        ijk[0] = -i + Nijk[0] - 1;
        // negate the first two components
        pixel[0] = -dataPtr[0];
        pixel[1] = -dataPtr[1];
        pixel[2] = dataPtr[2];
        gridImage->SetPixel( ijk, pixel );
        }
      }
    }
  itk::ImageFileWriter<GridType>::Pointer writer = itk::ImageFileWriter<GridType>::New();
  writer->SetInput( gridImage );
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
  this->SupportedWriteFileTypes->InsertNextValue("Transform (.tfm)");
  this->SupportedWriteFileTypes->InsertNextValue("Transform (.mat)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
  this->SupportedWriteFileTypes->InsertNextValue("Transform (.*)");
  this->SupportedWriteFileTypes->InsertNextValue("Deformation field (.nrrd)");
}
//----------------------------------------------------------------------------
const char* vtkMRMLTransformStorageNode::GetDefaultWriteFileExtension()
{
  return "tfm";
}
