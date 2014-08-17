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
#include <vtkCollection.h>
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkMatrixToLinearTransform.h>
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
#include <itkDisplacementFieldTransform.h>
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

typedef itk::DisplacementFieldTransform< double, 3 > DisplacementFieldTransformType;
typedef DisplacementFieldTransformType::DisplacementFieldType GridImageType;

typedef itk::CompositeTransform< double > CompositeTransformType;

#include "vtkITKTransformConverter.h"

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

  vtkSmartPointer<vtkMatrix4x4> transformVtk_RAS = vtkSmartPointer<vtkMatrix4x4>::New();
  bool convertedToVtkMatrix = vtkITKTransformConverter::SetVTKLinearTransformFromITK(this, transformVtk_RAS, transform);
  if (!convertedToVtkMatrix)
    {
    vtkDebugMacro(<< "Could not convert the transform in the file to a linear transform: "<< fullName.c_str());
    return 0;
    }

  // Convert the sense of the transform (from an ITK resampling
  // transform to a Slicer modeling transform)
  //
  transformVtk_RAS->Invert();

  // Set the matrix on the node
  vtkMRMLLinearTransformNode *ltn = vtkMRMLLinearTransformNode::SafeDownCast(refNode);
  if (ltn->GetReadWriteAsTransformToParent())
    {
    ltn->SetMatrixTransformToParent( transformVtk_RAS );
    }
  else
    {
    ltn->SetMatrixTransformFromParent( transformVtk_RAS );
    }

  return 1;
}


//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::ReadBSplineTransformITKv3(vtkMRMLNode *refNode)
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
  if (vtkITKTransformConverter::SetVTKBSplineFromITKv3(this, bsplineVtk.GetPointer(), transform, transform2))
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
int vtkMRMLTransformStorageNode::ReadBSplineTransform(vtkMRMLNode *refNode)
{
  int success = this->ReadBSplineTransformITKv3(refNode);
  if (success)
    {
    return success;
    }
  // If a composite transform only contains a BSpline and a linear transform then consider it as a BSpline transform
  // (the vtkMRMLTransformNode base class could handle everything, specialized BSpline node is only created for backward
  // compatibility, e.g., to allow filtering of transform nodes based on node type)
  success = ReadCompositeTransform(refNode);
  if (success)
    {
    // check if it is really a BSpline transform (contains a single BSpline or composite of linear and BSpline)
    vtkMRMLTransformNode *tn = vtkMRMLTransformNode::SafeDownCast(refNode);
    vtkAbstractTransform* inputTransform = tn->GetTransformToParent();
    bool isBSpline = false;
    if (vtkOrientedBSplineTransform::SafeDownCast(inputTransform))
      {
      isBSpline = true;
      }
    else
      {
      vtkGeneralTransform* generalTransform = vtkGeneralTransform::SafeDownCast(inputTransform);
      if (generalTransform!=NULL)
        {
        vtkNew<vtkCollection> transformList;
        vtkMRMLTransformNode::FlattenGeneralTransform(transformList.GetPointer(), generalTransform);
        if (transformList->GetNumberOfItems()==1)
          {
          if (vtkOrientedBSplineTransform::SafeDownCast(transformList->GetItemAsObject(0)))
            {
            isBSpline = true;
            }
          }
        else if (transformList->GetNumberOfItems()==2)
          {
          if (vtkMatrixToLinearTransform::SafeDownCast(transformList->GetItemAsObject(0))
            && vtkOrientedBSplineTransform::SafeDownCast(transformList->GetItemAsObject(1)))
            {
            isBSpline = true;
            }
          }
        }
      }
    // return with failure if not a bspline
    return isBSpline ? 1 : 0;
    }
  return 0;
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
  vtkITKTransformConverter::SetVTKOrientedGridTransformFromITKImage(this, gridTransform_Ras.GetPointer(), gridImage_Lps);
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
  TransformType *compositeTransform = transforms->front();

  std::string compositeTransformType = compositeTransform->GetTransformTypeAsString();
  if( compositeTransformType.find("CompositeTransform") == std::string::npos )
    {
    // It is not a composite transform. It may be still a list of transforms, but
    // when a list of transforms is stored in a file then there is no rule how to interpret them.
    // It is not necessarily a compositing, for example: in ITKv3 the list was used to store additive
    // bulk transform for BSpline deformable transform. Therefore, if the file contains a transform list
    // then we do not interpret it as a composite/ transform.
    vtkDebugMacro("Cannot read composite transform from file: "<< fullName.c_str());
    return 0;
    }

  if (transforms->size()>1)
    {
    vtkWarningMacro("Additional transform(s) are defined after a composite transform. Only the composite transform will be used from file: "<< fullName.c_str());
    }

  // the composite transform is itself a list of transforms.  There is a
  // helper class in ITK to convert the internal transform list into a
  // list that is possible to iterate over.  So we get this transformList.
  typedef const itk::CompositeTransformIOHelper::TransformType ComponentTransformType;
  itk::CompositeTransformIOHelper compositeTransformIOHelper;

  // if the first transform in the list is a
  // composite transform, use its internal list
  // instead of the IO
  typedef itk::CompositeTransformIOHelper::ConstTransformListType ConstTransformListType;
  ConstTransformListType transformList =
    compositeTransformIOHelper.GetTransformList(compositeTransform);

  if (transformList.empty())
    {
    // Log only at debug level because trial-and-error method is used for finding out
    // what node can be retrieved from a transform file
    vtkDebugMacro("Failed to retrieve any transform transform from file: "<< fullName.c_str());
    return 0;
    }

  vtkSmartPointer<vtkAbstractTransform> transformVtk;

  ConstTransformListType::const_iterator end = transformList.end();
  if (transformList.size()==1)
    {
    // there is only one single transform, so we create a specific VTK transform type instead of a general transform
    TransformType::Pointer transformComponentItk = const_cast< TransformType* >(transformList.front().GetPointer());
    transformVtk = vtkSmartPointer<vtkAbstractTransform>::Take(vtkITKTransformConverter::CreateVTKTransformFromITK(this, transformComponentItk));
    }
  else
    {
    // we have multiple transforms, so we create a general transform that can hold a list of transforms
    vtkNew<vtkGeneralTransform> generalTransform;
    //generalTransform->PostMultiply();
    for( ConstTransformListType::const_iterator it = transformList.begin();
      it != end; ++it )
      {
      TransformType::Pointer transformComponentItk = const_cast< TransformType* >((*it).GetPointer());
      vtkAbstractTransform* transformComponent = vtkITKTransformConverter::CreateVTKTransformFromITK(this, transformComponentItk);
      if (transformComponent!=NULL)
        {
        generalTransform->Concatenate(transformComponent);
        transformComponent->Delete();
        }
      }
    transformVtk = generalTransform.GetPointer();
    }

  vtkMRMLTransformNode *transformNode = vtkMRMLTransformNode::SafeDownCast(refNode);
  if (transformNode == NULL)
    {
    vtkErrorMacro("Unexpected node type, cannot read transform from file: "<< fullName.c_str());
    return 0;
    }
  if (transformNode->GetReadWriteAsTransformToParent())
    {
    // Convert the sense of the transform (from an ITK resampling
    // transform to a Slicer modeling transform)
    transformNode->SetAndObserveTransformToParent( transformVtk.GetPointer() );
    }
  else
    {
    transformNode->SetAndObserveTransformFromParent( transformVtk.GetPointer() );
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
  else if (refNode->IsA("vtkMRMLTransformNode"))
    {
    return ReadCompositeTransform(refNode);
    }

  vtkErrorMacro("ReadData: failed, transform node type is not supported for reading");
  return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::WriteTransform(vtkMRMLTransformNode *transformNode)
{
  // Get VTK transform from the transform node
  vtkAbstractTransform* transformVtk = NULL;
  if (transformNode->GetReadWriteAsTransformToParent())
    {
    transformVtk = transformNode->GetTransformToParent();
    }
  else
    {
    transformVtk = transformNode->GetTransformFromParent();
    }
  if (transformVtk==NULL)
    {
    vtkErrorMacro("WriteTransform failed: cannot get VTK transform");
    return 0;
    }

  // Convert VTK transform to ITK transform
  itk::Object::Pointer secondaryTransformItk; // only used for ITKv3 compatibility
  itk::Object::Pointer transformItk = vtkITKTransformConverter::CreateITKTransformFromVTK(this, transformVtk, secondaryTransformItk);
  if (transformItk.IsNull())
    {
    vtkErrorMacro("WriteTransform failed: cannot to convert VTK transform to ITK transform");
    return 0;
    }

  TransformWriterType::Pointer writer = TransformWriterType::New();
  writer->SetInput( transformItk );

  // In ITKv3 bulk transform may be added as a second transform in the transform list
  if( secondaryTransformItk.IsNotNull() )
    {
    writer->AddTransform( secondaryTransformItk );
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
  vtkITKTransformConverter::SetITKImageFromVTKOrientedGridTransform(this, gridImage_Lps, gridTransform_Ras);

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
    vtkErrorMacro("vtkMRMLTransformNode write data failed: file name not specified");
    return 0;
    }
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(refNode);
  if (transformNode == NULL)
    {
    vtkErrorMacro("vtkMRMLTransformNode write data failed: invalid transform node");
    return 0;
    }
  return WriteTransform(transformNode);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Transform (.h5)");
  this->SupportedWriteFileTypes->InsertNextValue("Transform (.tfm)");
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
