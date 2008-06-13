/*=auto=========================================================================

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
      if (transforms->size() != 1)
        {
        vtkWarningMacro(<< "More than one transform in the file: "
                        << fullName.c_str()
                        << ". Using only the first transform.");
        }
    
      TransformListType::iterator it = (*transforms).begin();
      transform = (*it);
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
        }

      // Identity transform of floats, dimension 3
      FloatIdentityTransformType::Pointer fit
        = dynamic_cast<FloatIdentityTransformType*>( transform.GetPointer() );
      if (fit)
        {
        // nothing to do, matrix is already the identity
        }
      
      // Scale transform of doubles, dimension 3
      DoubleScaleTransformType::Pointer dst
        = dynamic_cast<DoubleScaleTransformType*>( transform.GetPointer() );
      if (dst)
        {
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

      vtkMRMLGridTransformNode *gtn
        = vtkMRMLGridTransformNode::SafeDownCast(refNode);
      
      static const int D = 3;
      typedef itk::BSplineDeformableTransform<double,D,D> DoubleBSplineTransformType;
      typedef itk::BSplineDeformableTransform<float,D,D> FloatBSplineTransformType;

      vtkGridTransform* vtkgrid = vtkGridTransform::New();
      vtkgrid->SetInterpolationModeToCubic();

      vtkImageData *vtkgridimage = vtkImageData::New();
      
      // B-spline transform of doubles, dimension 3
      DoubleBSplineTransformType::Pointer dbt
        = dynamic_cast<DoubleBSplineTransformType*>( transform.GetPointer() );
      if (dbt)
        {
        DoubleBSplineTransformType::ImagePointer
          *grids = dbt->GetCoefficientImage();

        itk::ImageRegionIterator<DoubleBSplineTransformType::ImageType>
          xit( grids[0], grids[0]->GetBufferedRegion() );
        itk::ImageRegionIterator<DoubleBSplineTransformType::ImageType>
          yit( grids[1], grids[0]->GetBufferedRegion() );
        itk::ImageRegionIterator<DoubleBSplineTransformType::ImageType>
          zit( grids[2], grids[0]->GetBufferedRegion() );
        
        vtkDoubleArray *values = vtkDoubleArray::New();
        values->SetNumberOfComponents( 3 );
        values->SetNumberOfTuples( grids[0]->GetBufferedRegion().GetNumberOfPixels() );

        double in[4], out[4];
        in[3] = out[3] = 1.0;
        
        for (vtkIdType id=0;
             id < (vtkIdType)(grids[0]->GetBufferedRegion().GetNumberOfPixels()) ; ++id)
          {
          // convert each control point of the B-spline grid to RAS
          in[0] = xit.Get();
          in[1] = yit.Get();
          in[2] = zit.Get();

          lps2ras->MultiplyPoint(in, out);
          
          values->SetComponent(id, 0, out[0]);
          values->SetComponent(id, 1, out[1]);
          values->SetComponent(id, 2, out[2]);
          
          ++xit;
          ++yit;
          ++zit;
          }

        vtkgridimage->GetPointData()->SetScalars( values );
        values->Delete();

        vtkgridimage->SetOrigin( grids[0]->GetOrigin()[0],
                                 grids[0]->GetOrigin()[1],
                                 grids[0]->GetOrigin()[2] );
        vtkgridimage->SetSpacing( grids[0]->GetSpacing()[0],
                                  grids[0]->GetSpacing()[1],
                                  grids[0]->GetSpacing()[2]);

        DoubleBSplineTransformType::ImageType::IndexType index
          = grids[0]->GetBufferedRegion().GetIndex();
        DoubleBSplineTransformType::ImageType::SizeType size
          = grids[0]->GetBufferedRegion().GetSize();
        
        vtkgridimage->SetExtent( index[0], index[0]+size[0]-1,
                                 index[1], index[1]+size[1]-1,
                                 index[2], index[2]+size[2]-1 );

        vtkgrid->SetDisplacementGrid( vtkgridimage );

        // Set the matrix on the node
        gtn->SetAndObserveWarpTransformToParent( vtkgrid );
        vtkgrid->Delete();

        // What about the bulk transform?
        }
      
      // B-spline transform of floats, dimension 3
      FloatBSplineTransformType::Pointer fbt
        = dynamic_cast<FloatBSplineTransformType*>( transform.GetPointer() );
      if (fbt)
        {
        }
      
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

        // Convert from LPS (ITK) to RAS (Slicer)
        vtkBSpline->SetSwitchCoordinateSystem( true );

        // Set the transform on the node
        btn->SetAndObserveWarpTransformToParent( vtkBSpline );
        vtkBSpline->Delete();
        }
      }

      // B-spline transform of floats, dimension 3
      FloatBSplineTransformType::Pointer fbt
        = dynamic_cast<FloatBSplineTransformType*>( transform.GetPointer() );
      if (fbt)
        {
        vtkErrorMacro( "BSpline transform storage not yet implemented for float" )
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

      GridImageType::SizeType imageSize = 
        gridImage->GetLargestPossibleRegion().GetSize();
      unsigned const int Ni = imageSize[0];
      unsigned const int Nj = imageSize[1];
      unsigned const int Nk = imageSize[2];
      unsigned const int Nc = gridImage->GetVectorLength();

      vtkImageData *vtkgridimage = vtkImageData::New();

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
      //      = Diag(s2) * p + o2
      // where
      //    s2 = [ -s(1), -s(2), s(3) ]
      //    o2 = [ -o(1), -o(2), o(3) ].
      //
      // Thus, by changing the origin and spacing, the pixel indexing
      // does not need to change.
      //
      // However, the value at each grid point is a displacement in
      // the LPS coordinate system, and needs to be converted too.
      // E.g. a displacement d takes a physical point x1 to a physical
      // point x2, so that
      //      x2 = x1 + d
      // We require the corresponding displacement d2 that takes
      // y1 = C*x1 to y2 = C*x2, giving us
      //      d2 = y2-y1 = C*(x2-x1) = C*d
      // Thus
      //     d2 = [ -d(1), -d(2), d(3) ]

      vtkgridimage->SetOrigin( -gridImage->GetOrigin()[0],
                               -gridImage->GetOrigin()[1],
                                gridImage->GetOrigin()[2] );
      vtkgridimage->SetSpacing( -gridImage->GetSpacing()[0],
                                 gridImage->GetSpacing()[1],
                                 gridImage->GetSpacing()[2]);

      
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

      // As far as I know, both the itk::VectorImage and the vtkImage
      // both store the pixel data in contiguous memory, ordered the
      // same way, so we can do this memcpy.  If they don't (I don't
      // know of a way to check), we would need instead to do a loop
      // (implemented but commented out below).
      std::memcpy( vtkgridimage->GetScalarPointer(),
                   gridImage->GetPixelContainer()->GetBufferPointer(),
                   gridImage->GetPixelContainer()->Size() * sizeof(double) );

      // We need to convert from LPS to RAS
      vtkIdType incX, incY, incZ;
      vtkgridimage->GetIncrements( incX, incY, incZ );
      double* dataPtr = reinterpret_cast<double*>(vtkgridimage->GetScalarPointer());
      for( int k = 0; k < Nk; ++k, dataPtr += incZ )
        {
          for( int j = 0; j < Nj; ++j, dataPtr += incY )
          {
            for( int i = 0; i < Ni; ++i, dataPtr += incX )
            {
            // negate the first two components
            dataPtr[0] = -dataPtr[0];
            dataPtr[1] = -dataPtr[1];
            dataPtr += 3;
            }
          }
        }


      //GridImageType::IndexType idx;
      //for( int k = 0; k < Nk; ++k )
      //  {
      //  idx[2] = k;
      //  for( int j = 0; j < Nj; ++j )
      //    {
      //    idx[1] = j;
      //    for( int i = 0; i < Ni; ++i )
      //      {
      //      idx[0] = i;
      //      GridImageType::PixelType const& v = gridImage->GetPixel(idx);
      //      double *comp = reinterpret_cast<double*>(vtkgridImage->GetScalarPointer(i,j,k));
      //      comp[0] = -v[0];
      //      comp[1] = -v[1];
      //      comp[2] =  v[2];
      //      }
      //    }
      //  }
      //}


      vtkgrid->SetDisplacementGrid( vtkgridimage );
      vtkgridimage->Delete();

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
  
  vtkMatrix4x4 *lps2ras = vtkMatrix4x4::New();
  lps2ras->Identity();
  (*lps2ras)[0][0] = (*lps2ras)[1][1] = -1.0;
    
  vtkMatrix4x4 *ras2lps = vtkMatrix4x4::New();
  ras2lps->Identity();
  (*ras2lps)[0][0] = (*ras2lps)[1][1] = -1.0;


  vtkMRMLLinearTransformNode *ln
    = vtkMRMLLinearTransformNode::SafeDownCast(refNode);

  if (ln != 0)
    {
    // Linear transform
    static const int VTKDimension = 3;
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
    }

  lps2ras->Delete();
  ras2lps->Delete();

  return result;
}
