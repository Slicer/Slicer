/*=Auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLNonlinearTransformNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLBSplineTransformNode.h"

#include "vtkMatrix4x4.h"
#include "vtkGridTransform.h"
#include "vtkImageData.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"

#include "vtkITKBSplineTransform.h"

#include "itksys/SystemTools.hxx"

#include "itkTransformFileWriter.h"
#include "itkTransformFileReader.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkTranslationTransform.h"
#include "itkScaleTransform.h"
#include "itkBSplineDeformableTransform.h"

#include "itkImageRegionIterator.h"


//------------------------------------------------------------------------------
vtkMRMLTransformStorageNode* vtkMRMLTransformStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLTransformStorageNode");
  if(ret)
    {
    return (vtkMRMLTransformStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTransformStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLTransformStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLTransformStorageNode");
  if(ret)
    {
    return (vtkMRMLTransformStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTransformStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLTransformStorageNode::vtkMRMLTransformStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLTransformStorageNode::~vtkMRMLTransformStorageNode()
{
}

void vtkMRMLTransformStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformStorageNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLStorageNode::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLTransformStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << indent << "FileName: " <<
    (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLTransformStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::ReadData(vtkMRMLNode *refNode)
{
  // do not read if if we are not in the scene (for example inside snapshot)
  if ( !this->GetAddToScene() || !refNode->GetAddToScene() )
    {
    return 1;
    }

  if (!refNode->IsA("vtkMRMLTransformNode") ) 
    {
    //vtkErrorMacro("Reference node is not a vtkMRMLTransformNode");
    return 0;
    }

  Superclass::StageReadData(refNode);
  if ( this->GetReadState() != this->TransferDone )
    {
    // remote file download hasn't finished
    return 0;
    }
  
  vtkMRMLTransformNode *transformNode = dynamic_cast <vtkMRMLTransformNode *> (refNode);

  std::string fullName;
  if (this->SceneRootDir != NULL && this->Scene->IsFilePathRelative(this->GetFileName())) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileName());
    }
  else 
    {
    fullName = std::string(this->GetFileName());
    }
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  int result = 1;

  vtkMatrix4x4 *lps2ras = vtkMatrix4x4::New();
  lps2ras->Identity();
  (*lps2ras)[0][0] = (*lps2ras)[1][1] = -1.0;
  
  vtkMatrix4x4 *ras2lps = vtkMatrix4x4::New();
  ras2lps->Identity();
  (*ras2lps)[0][0] = (*ras2lps)[1][1] = -1.0;

  typedef itk::TransformFileReader TransformReader;
  typedef TransformReader::TransformListType TransformListType;
  typedef TransformReader::TransformType TransformType;
  TransformType::Pointer transform = 0;
  TransformType::Pointer transform2 = 0;

  typedef itk::VectorImage< double, 3 >   GridImageType;
  GridImageType::Pointer gridImage = 0;

  // A grid transform is not a itk::Transform.  It is instead
  // transferred as an itk::VectorImage.  As such, we need special
  // logic to parse it.  If we have something other than a grid
  // transform, we read it through the itk::TransformFileReader (in
  // the else part of this block).
  //
  if (refNode->IsA("vtkMRMLGridTransformNode"))
    {
    typedef itk::ImageFileReader< GridImageType >  ReaderType;

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
        gridImage = 0;
        result = 0;
        }
      }
    catch (itk::ExceptionObject &exc)
      {
      // File specified may not contain a grid image. Can we safely
      // error out quitely?
      vtkErrorMacro("ITK exception caught reading grid transform image file: "
                    << fullName.c_str() << "\n" << exc);

      result = 0;
      }
    catch (...)
      {
      vtkErrorMacro("Unknown exception caught while reading grid transform image file: "
                    << fullName.c_str());
      result = 0;
      }
    }
  else
    {
    TransformReader::Pointer reader = itk::TransformFileReader::New();
    reader->SetFileName( fullName );
    try
      {
      reader->Update();
      }
    catch (itk::ExceptionObject &exc)
      {
      vtkErrorMacro("ITK exception caught reading transform file: "
                    << fullName.c_str() << "\n" << exc);

      result = 0;
      }
    catch (...)
      {
      vtkErrorMacro("Unknown exception caught while writing transform file: "
                    << fullName.c_str());
      result = 0;
      }

    // For now, grab the first transform from the file.
    TransformListType *transforms = reader->GetTransformList();
    if (transforms->size() != 0)
      {
      // If the transform is a BSplineTransform, it can have a second
      // transform for the corresponding bulk transform
      if (! (transforms->size() == 1 || 
             (transforms->size() <= 2 && 
              refNode->IsA("vtkMRMLBSplineTransformNode"))))
        {
        vtkWarningMacro(<< "More than one transform in the file: "
                        << fullName.c_str()
                        << ". Using only the first transform.");
        }
    
      TransformListType::iterator it = (*transforms).begin();
      transform = (*it);
      ++it;
      if( it != (*transforms).end() )
        {
        transform2 = (*it);
        }
      }

    if (!transform)
      {
      vtkErrorMacro(<< "No transforms in the file: "
                    << fullName.c_str()
                    << ", (" << transforms->size() << ")");
      result = 0;
      }
    }  

  // Convert the ITK transform to the appropriate type of VTK
  // transform
  if (transform)
    {
    if (refNode->IsA("vtkMRMLLinearTransformNode"))
      {
      vtkMRMLLinearTransformNode *ltn
        = vtkMRMLLinearTransformNode::SafeDownCast(refNode);
      
      static const int D = 3;
      int i, j;

      
      typedef itk::MatrixOffsetTransformBase<double,D,D> DoubleLinearTransformType;
      typedef itk::MatrixOffsetTransformBase<float,D,D> FloatLinearTransformType;
      typedef itk::IdentityTransform<double, D> DoubleIdentityTransformType;
      typedef itk::IdentityTransform<float, D> FloatIdentityTransformType;
      typedef itk::ScaleTransform<double, D> DoubleScaleTransformType;
      typedef itk::ScaleTransform<float, D> FloatScaleTransformType;
      typedef itk::TranslationTransform<double, D> DoubleTranslateTransformType;
      typedef itk::TranslationTransform<float, D> FloatTranslateTransformType;

      vtkMatrix4x4* vtkmat = vtkMatrix4x4::New();
      vtkmat->Identity();

      // Linear transform of doubles, dimension 3
      DoubleLinearTransformType::Pointer dlt
        = dynamic_cast<DoubleLinearTransformType*>( transform.GetPointer() );
      if (dlt)
        {
        result = 1;
        for (i=0; i < D; i++)
          {
          for (j=0; j < D; j++)
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
        result = 1;
        for (i=0; i < D; i++)
          {
          for (j=0; j < D; j++)
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
        result = 1;
        }

      // Identity transform of floats, dimension 3
      FloatIdentityTransformType::Pointer fit
        = dynamic_cast<FloatIdentityTransformType*>( transform.GetPointer() );
      if (fit)
        {
        // nothing to do, matrix is already the identity
        result = 1;
        }
      
      // Scale transform of doubles, dimension 3
      DoubleScaleTransformType::Pointer dst
        = dynamic_cast<DoubleScaleTransformType*>( transform.GetPointer() );
      if (dst)
        {
        result = 1;
        for (i=0; i < D; i++)
          {
          (*vtkmat)[i][i] = dst->GetScale()[i];
          }
        }

      // Scale transform of floats, dimension 3
      FloatScaleTransformType::Pointer fst
        = dynamic_cast<FloatScaleTransformType*>( transform.GetPointer() );
      if (fst)
        {
        result = 1;
        for (i=0; i < D; i++)
          {
          (*vtkmat)[i][i] = fst->GetScale()[i];
          }
        }

      // Translate transform of doubles, dimension 3
      DoubleTranslateTransformType::Pointer dtt
        = dynamic_cast<DoubleTranslateTransformType*>( transform.GetPointer());
      if (dtt)
        {
        result = 1;
        for (i=0; i < D; i++)
          {
          (*vtkmat)[i][D] = dtt->GetOffset()[i];
          }
        }

      // Translate transform of floats, dimension 3
      FloatTranslateTransformType::Pointer ftt
        = dynamic_cast<FloatTranslateTransformType*>( transform.GetPointer() );
      if (ftt)
        {
        result = 1;
        for (i=0; i < D; i++)
          {
          (*vtkmat)[i][i] = ftt->GetOffset()[i];
          }
        }

      // Convert from LPS (ITK) to RAS (Slicer)
      //
      // Tras = lps2ras * Tlps * ras2lps
      //
      vtkMatrix4x4::Multiply4x4(lps2ras, vtkmat, vtkmat);
      vtkMatrix4x4::Multiply4x4(vtkmat, ras2lps, vtkmat);
      
      // Set the matrix on the node
      ltn->SetAndObserveMatrixTransformToParent( vtkmat );
      vtkmat->Delete();
      }
    else if (refNode->IsA("vtkMRMLGridTransformNode"))
      {
      // This use case is handled separately.  Grid transforms are not
      // currently supported as ITK transforms but rather as vector
      // images. This is subject to change whereby an ITK transform
      // for displacement fields will provide a standard transform API
      // but will reference a vector image to store the displacements.
      result = 0;
      }
    else if (refNode->IsA("vtkMRMLBSplineTransformNode"))
      {

      vtkMRMLBSplineTransformNode *btn
        = vtkMRMLBSplineTransformNode::SafeDownCast(refNode);
      

      static const int D = 3;
      typedef itk::BSplineDeformableTransform<double,D,D> DoubleBSplineTransformType;
      typedef itk::BSplineDeformableTransform<float,D,D> FloatBSplineTransformType;

      vtkITKBSplineTransform* vtkBSpline = vtkITKBSplineTransform::New();
      
      // B-spline transform of doubles, dimension 3
      {
      DoubleBSplineTransformType::Pointer bst
        = dynamic_cast<DoubleBSplineTransformType*>( transform.GetPointer() );
      if (bst)
        {
        typedef DoubleBSplineTransformType SplineType;
          
        vtkBSpline->SetSplineOrder( 3 );
        SplineType::ParametersType const& fp = bst->GetFixedParameters();
        vtkBSpline->SetFixedParameters( fp.data_block(), fp.GetSize() );

        if( bst->GetNumberOfParameters() != vtkBSpline->GetNumberOfParameters() )
          {
          vtkErrorMacro("Mismatch in number of BSpline parameters");
          return 0;
          }

        SplineType::ParametersType const& param = bst->GetParameters();
        vtkBSpline->SetParameters( param.data_block() );

        if( transform2 ) 
          {
          typedef vtkITKBSplineTransform::BulkTransformType BulkTransformType;          
          BulkTransformType* bulk =
            dynamic_cast<BulkTransformType*> (transform2.GetPointer());
          if (bulk)
            {
            double linear[D][D];
            double offset[D];
            for (int i=0; i < D; i++)
              {
              for (int j=0; j < D; j++)
                {
                linear[i][j] = bulk->GetMatrix()[i][j];
                }
              offset[i] = bulk->GetOffset()[i];
              }
            vtkBSpline->SetBulkTransform( linear, offset );            
            }
          else
            {
            vtkWarningMacro( "The type of the 2nd transform in BSplineTransform is not correct." );
            }
          }
        // Convert from LPS (ITK) to RAS (Slicer)
        vtkBSpline->SetSwitchCoordinateSystem( true );
        
        // Set the transform on the node
        btn->SetAndObserveWarpTransformToParent( vtkBSpline );
        vtkBSpline->Delete();

        result = 1;
        }
      else
        {
        result = 0;
        }
      }

      // B-spline transform of floats, dimension 3
      FloatBSplineTransformType::Pointer fbt
        = dynamic_cast<FloatBSplineTransformType*>( transform.GetPointer() );
      if (!result && fbt)
        {
        vtkErrorMacro( "BSpline transform storage not yet implemented for float" );
        result = 0;
        }
      
      }
    }


  // Convert the grid image to the appropriate type of VTK
  // transform
  if (gridImage)
    {
    if (refNode->IsA("vtkMRMLGridTransformNode"))
      {

      vtkMRMLGridTransformNode *gtn
        = vtkMRMLGridTransformNode::SafeDownCast(refNode);
      
      vtkGridTransform* vtkgrid = vtkGridTransform::New();
      vtkgrid->SetInterpolationModeToCubic();
      vtkImageData *vtkgridimage = vtkImageData::New();

      GridImageType::IndexType index
        = gridImage->GetBufferedRegion().GetIndex();
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
        result = 0;
        }

      vtkgridimage->SetDimensions( Ni, Nj, Nk );
      vtkgridimage->SetNumberOfScalarComponents( Nc );
      vtkgridimage->SetScalarTypeToDouble();
      vtkgridimage->AllocateScalars();

      // convert from LPS to RAS
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

      vtkgrid->SetDisplacementGrid( vtkgridimage );

      // Set the matrix on the node
      gtn->SetAndObserveWarpTransformToParent( vtkgrid );
      vtkgrid->Delete();
      }
    }


  if (transformNode->GetTransformToParent() != NULL) 
    {
    transformNode->GetTransformToParent()->Modified();
    }
  transformNode->SetModifiedSinceRead(0);

  lps2ras->Delete();
  ras2lps->Delete();

  this->SetReadStateIdle();
   
  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::WriteData(vtkMRMLNode *refNode)
{
  // test whether refNode is a valid node to hold a transform
  if (!refNode->IsA("vtkMRMLTransformNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLTransformNode");
    return 0;
    }
  
  //vtkMRMLTransformNode *transformNode = vtkMRMLTransformNode::SafeDownCast(refNode);
  
  std::string fullName;
  if (this->SceneRootDir != NULL && this->Scene->IsFilePathRelative(this->GetFileName())) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileName());
    }
  else 
    {
    fullName = std::string(this->GetFileName());
    }  
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLTransformNode: File name not specified");
    return 0;
    }

  // Get an ITK version of the transform and then use the TransformIO
  // fractory mechanism
  int result = 1;
  static const int VTKDimension = 3;
  
  vtkMRMLLinearTransformNode *ln
    = vtkMRMLLinearTransformNode::SafeDownCast(refNode);
  vtkMRMLBSplineTransformNode *bs
    = vtkMRMLBSplineTransformNode::SafeDownCast(refNode);
  vtkMRMLGridTransformNode *gd
    = vtkMRMLGridTransformNode::SafeDownCast(refNode);

  if (ln != 0)
    {
    // Linear transform
    vtkMatrix4x4 *lps2ras = vtkMatrix4x4::New();
    lps2ras->Identity();
    (*lps2ras)[0][0] = (*lps2ras)[1][1] = -1.0;
    
    vtkMatrix4x4 *ras2lps = vtkMatrix4x4::New();
    ras2lps->Identity();
    (*ras2lps)[0][0] = (*ras2lps)[1][1] = -1.0;

    typedef itk::AffineTransform<double, VTKDimension> AffineTransformType;
    AffineTransformType::Pointer affine = AffineTransformType::New();

    vtkMatrix4x4 *mat2parent = ln->GetMatrixTransformToParent();
    
    // Convert from RAS (Slicer) to LPS (ITK)
    //
    // Tlps = ras2lps * Tras * lps2ras
    //
    vtkMatrix4x4 *vtkmat = vtkMatrix4x4::New();
    
    vtkMatrix4x4::Multiply4x4(ras2lps, mat2parent, vtkmat);
    vtkMatrix4x4::Multiply4x4(vtkmat, lps2ras, vtkmat);
    
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

    vtkmat->Delete();
    
    itk::TransformFileWriter::Pointer writer = itk::TransformFileWriter::New();
    writer->SetInput( affine );
    writer->SetFileName( fullName );
    try
      {
      writer->Update();
      }
    catch (itk::ExceptionObject &exc)
      {
      vtkErrorMacro("ITK exception caught writing transform file: "
                    << fullName.c_str() << "\n" << exc);
      result = 0;
      }
    catch (...)
      {
      vtkErrorMacro("Unknown exception caught while writing transform file: "
                    << fullName.c_str());
      result = 0;
      }
    lps2ras->Delete();
    ras2lps->Delete();
    }
  else if (bs != 0)
    {
    // BSpline transform

    vtkITKBSplineTransform* vtkTrans = vtkITKBSplineTransform::SafeDownCast(bs->GetWarpTransformToParent());
    
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
    writer->SetFileName( fullName );
    try
      {
      writer->Update();
      }
    catch (itk::ExceptionObject &exc)
      {
      vtkErrorMacro("ITK exception caught writing transform file: "
                    << fullName.c_str() << "\n" << exc);
      result = 0;
      }
    catch (...)
      {
      vtkErrorMacro("Unknown exception caught while writing transform file: "
                    << fullName.c_str());
      result = 0;
      }
    }
  else if( gd )
    {
    // Grid Transform
    vtkGridTransform* vtkTrans = vtkGridTransform::SafeDownCast(gd->GetWarpTransformToParent());
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
    writer->SetFileName( fullName );
    try
      {
      writer->Update();
      }
    catch (itk::ExceptionObject &exc)
      {
      vtkErrorMacro("ITK exception caught writing transform file: "
                    << fullName.c_str() << "\n" << exc);
      result = 0;
      }
    catch (...)
      {
      vtkErrorMacro("Unknown exception caught while writing transform file: "
                    << fullName.c_str());
      result = 0;
      }
    }


  return result;
}
