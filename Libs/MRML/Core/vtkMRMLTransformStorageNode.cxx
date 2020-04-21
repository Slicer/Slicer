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
#include "vtkOrientedBSplineTransform.h"
#include "vtkOrientedGridTransform.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkGeneralTransform.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkVersion.h>
#include "vtksys/SystemTools.hxx"

#include "vtkITKTransformConverter.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTransformStorageNode);

bool vtkMRMLTransformStorageNode::RegisterInverseTransformTypesCompleted = false;

//----------------------------------------------------------------------------
vtkMRMLTransformStorageNode::vtkMRMLTransformStorageNode()
{
  this->PreferITKv3CompatibleTransforms = 0;
  this->DefaultWriteFileExtension = "h5";

  // Ensure custom ITK inverse transform classes are registered.
  // Register them only once to improve performance when many transform nodes
  // are instantiated.
  if (!vtkMRMLTransformStorageNode::RegisterInverseTransformTypesCompleted)
    {
    vtkITKTransformConverter::RegisterInverseTransformTypes();
    vtkMRMLTransformStorageNode::RegisterInverseTransformTypesCompleted = true;
    }
}

//----------------------------------------------------------------------------
vtkMRMLTransformStorageNode::~vtkMRMLTransformStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLTransformStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " preferITKv3CompatibleTransforms=\"" << (this->PreferITKv3CompatibleTransforms ? "true" : "false") << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLTransformStorageNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "preferITKv3CompatibleTransforms"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->PreferITKv3CompatibleTransforms = 1;
        }
      else
        {
        this->PreferITKv3CompatibleTransforms = 0;
        }
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLTransformStorageNode::Copy(vtkMRMLNode *anode)
{
  if (!anode)
    {
    return;
    }
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLTransformStorageNode *node = vtkMRMLTransformStorageNode::SafeDownCast(anode);

  this->SetPreferITKv3CompatibleTransforms(node->GetPreferITKv3CompatibleTransforms());

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLTransformStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "PreferITKv3CompatibleTransforms: " <<
    (this->PreferITKv3CompatibleTransforms ? "true" : "false") << "\n";
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLTransformNode");
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::ReadFromITKv3BSplineTransformFile(vtkMRMLNode *refNode)
{
  typedef itk::TransformFileReaderTemplate<double> TransformReaderType;
  typedef TransformReaderType::TransformListType TransformListType;
  typedef TransformReaderType::TransformType TransformType;

  vtkMRMLTransformNode *transformNode = vtkMRMLTransformNode::SafeDownCast(refNode);
  if (transformNode==nullptr)
    {
    vtkErrorMacro("vtkMRMLTransformStorageNode::ReadFromITKv3BSplineTransformFile failed: expected a transform node as input");
    return 0;
    }

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
  TransformType::Pointer transform2=nullptr;
  if( it != (*transforms).end() )
    {
    transform2 = (*it);
    if (!transform2)
      {
      vtkErrorMacro(<< "Invalid transform (2) in the file: "<< fullName.c_str()<< ", (" << transforms->size() << ")");
      return 0;
      }
    }

  vtkNew<vtkOrientedBSplineTransform> bsplineVtk;
  if (!vtkITKTransformConverter::SetVTKBSplineFromITKv3Generic<double>(this, bsplineVtk.GetPointer(), transform, transform2))
    {
    // Log only at debug level because trial-and-error method is used for finding out what node can be retrieved
    // from a transform file
    vtkDebugMacro("Failed to retrieve BSpline transform from file: "<< fullName.c_str());
    return 0;
    }

  // Backward compatibility
  if (transformNode->GetReadAsTransformToParent())
    {
    // For backward compatibility only (now all the transforms are saved as TransformFromParent)
    // Convert the sense of the transform (from an ITK resampling
    // transform to a Slicer modeling transform)
    bsplineVtk->Inverse();
    transformNode->SetReadAsTransformToParent(0);
    }

  SetAndObserveTransformFromParentAutoInvert(transformNode, bsplineVtk.GetPointer());
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::ReadFromImageFile(vtkMRMLNode *refNode)
{
  vtkMRMLTransformNode *tn = vtkMRMLTransformNode::SafeDownCast(refNode);
  if (tn==nullptr)
    {
    vtkErrorMacro("vtkMRMLTransformStorageNode::ReadGridTransform failed: expected a transform node as input");
    return 0;
    }

  GridImageDoubleType::Pointer gridImage_Lps = nullptr;

  typedef itk::ImageFileReader< GridImageDoubleType >  ReaderType;
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
    // error out quietely?
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
  vtkITKTransformConverter::SetVTKOrientedGridTransformFromITKImage<double>(
        this, gridTransform_Ras.GetPointer(), gridImage_Lps);

  // Backward compatibility
  if (tn->GetReadAsTransformToParent())
    {
    // For backward compatibility only (now all the transforms are saved as TransformFromParent)
    // Convert the sense of the transform (from an ITK resampling
    // transform to a Slicer modeling transform)
    gridTransform_Ras->Inverse();
    tn->SetReadAsTransformToParent(0);
    }

  SetAndObserveTransformFromParentAutoInvert(tn, gridTransform_Ras.GetPointer());
  return 1;
}

//----------------------------------------------------------------------------
template<typename T>
vtkAbstractTransform* ReadFromTransformFile(vtkObject* loggerObject, const std::string& fullName)
{
  typedef itk::TransformFileReaderTemplate<T> TransformReaderType;
  typedef typename TransformReaderType::TransformListType TransformListType;
  typedef typename TransformReaderType::TransformType TransformType;

  typename TransformReaderType::Pointer reader = TransformReaderType::New();
  reader->SetFileName( fullName );
  try
    {
    reader->Update();
    }
  catch (itk::ExceptionObject &exc)
    {
    vtkErrorWithObjectMacro(loggerObject, "ITK exception caught reading transform file: "<< fullName.c_str() << "\n" << exc);
    return nullptr;
    }
  catch (...)
    {
    vtkErrorWithObjectMacro(loggerObject, "Unknown exception caught while reading transform file: "<< fullName.c_str());
    return nullptr;
    }

  TransformListType *transforms = reader->GetTransformList();
  if (transforms==nullptr || transforms->empty())
    {
    vtkErrorWithObjectMacro(loggerObject, "Transforms not found in transform file: "<< fullName.c_str());
    return nullptr;
    }
  if (transforms->size()>1)
    {
    // When a list of transforms is stored in a file then there is no rule how to interpret them.
    // It is not necessarily a compositing, for example: in ITKv3 the list was used to store additive
    // bulk transform for BSpline deformable transform. Therefore, if the file contains a transform list
    // then we do not interpret it as a composite/ transform.
    vtkErrorWithObjectMacro(loggerObject, "Multiple transforms are defined in the transform file but only one is allowed (composite transforms has to be stored as a single CompositeTransform). In file: "<< fullName.c_str());
    return nullptr;
    }
  TransformType *firstTransform = transforms->front();
  if (firstTransform==nullptr)
    {
    vtkErrorWithObjectMacro(loggerObject, "Transforms not found in transform file: "<< fullName.c_str());
    return nullptr;
    }

  vtkSmartPointer<vtkAbstractTransform> transformVtk;
  std::string firstTransformType = firstTransform->GetTransformTypeAsString();
  if( firstTransformType.find("CompositeTransform") == std::string::npos )
    {
    // just a single transform
    transformVtk = vtkSmartPointer<vtkAbstractTransform>::Take(
          vtkITKTransformConverter::CreateVTKTransformFromITK<T>(loggerObject, firstTransform));
    }
  else
    {
    typedef itk::CompositeTransformIOHelperTemplate<T> CompositeTransformIOHelper;

    // The composite transform is itself a list of transforms.  There is a
    // helper class in ITK to convert the internal transform list into a
    // list that is possible to iterate over.  So we get this transformList.
    CompositeTransformIOHelper compositeTransformIOHelper;

    // if the first transform in the list is a
    // composite transform, use its internal list
    // instead of the IO
    typedef typename CompositeTransformIOHelper::ConstTransformListType ConstTransformListType;
    ConstTransformListType transformList =
      compositeTransformIOHelper.GetTransformList(firstTransform);

    if (transformList.empty())
      {
      // Log only at debug level because trial-and-error method is used for finding out
      // what node can be retrieved from a transform file
      vtkDebugWithObjectMacro(loggerObject, "Failed to retrieve any transform transform from file: "<< fullName.c_str());
      return nullptr;
      }

    typename ConstTransformListType::const_iterator end = transformList.end();
    if (transformList.size()==1)
      {
      // there is only one single transform, so we create a specific VTK transform type instead of a general transform
      typename TransformType::Pointer transformComponentItk = const_cast< TransformType* >(transformList.front().GetPointer());
      transformVtk = vtkSmartPointer<vtkAbstractTransform>::Take(
            vtkITKTransformConverter::CreateVTKTransformFromITK<T>(loggerObject, transformComponentItk));
      }
    else
      {
      // we have multiple transforms, so we create a general transform that can hold a list of transforms
      vtkNew<vtkGeneralTransform> generalTransform;
      //generalTransform->PostMultiply();
      for( typename ConstTransformListType::const_iterator it = transformList.begin();
        it != end; ++it )
        {
        typename TransformType::Pointer transformComponentItk = const_cast< TransformType* >((*it).GetPointer());
        vtkAbstractTransform* transformComponent = vtkITKTransformConverter::CreateVTKTransformFromITK<T>(loggerObject, transformComponentItk);
        if (transformComponent!=nullptr)
          {
          generalTransform->Concatenate(transformComponent);
          transformComponent->Delete();
          }
        }
      transformVtk = generalTransform.GetPointer();
      }
    }

  if (transformVtk)
    {
    transformVtk->Register(nullptr);
    }
  return transformVtk;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::ReadFromTransformFile(vtkMRMLNode *refNode)
{
  vtkMRMLTransformNode *transformNode = vtkMRMLTransformNode::SafeDownCast(refNode);
  if (transformNode == nullptr)
    {
    vtkErrorMacro("Unexpected node type, cannot read transform from file");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();

  vtkSmartPointer<vtkAbstractTransform> transformVtk;

  transformVtk = vtkSmartPointer<vtkAbstractTransform>::Take(
        ::ReadFromTransformFile<double>(this, fullName));

  if (transformVtk.GetPointer()==nullptr)
    {
    transformVtk = vtkSmartPointer<vtkAbstractTransform>::Take(
          ::ReadFromTransformFile<float>(this, fullName));
    }

  if (transformVtk.GetPointer()==nullptr)
      {
      vtkErrorMacro("Failed to read transform from file: "<< fullName.c_str());
      return 0;
      }

  // Backward compatibility
  if (transformNode->GetReadAsTransformToParent())
    {
    // For backward compatibility only (now all the transforms are saved as TransformFromParent)
    // Convert the sense of the transform (from an ITK resampling
    // transform to a Slicer modeling transform)
    transformVtk->Inverse();
    transformNode->SetReadAsTransformToParent(0);
    }

  SetAndObserveTransformFromParentAutoInvert(transformNode, transformVtk.GetPointer());

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

  // Skip reading if write state indicates that no file has been saved due to empty dataset
  if (this->GetWriteState() == SkippedNoData)
    {
    vtkDebugMacro("ReadDataInternal: empty transform file was not saved, ignore loading");
    return 1;
    }

  // Check that the file exists
  if (vtksys::SystemTools::FileExists(fullName.c_str()) == false)
    {
    vtkErrorMacro("ReadDataInternal: transform file '" << fullName.c_str() << "' not found.");
    return 0;
    }

  // We support reading of grid transforms directly from a vector image
  if (IsImageFile(fullName))
  {
    return ReadFromImageFile(refNode);
  }

  // For ITKv3 backward compatibility
  int success = this->ReadFromITKv3BSplineTransformFile(refNode);
  if (success)
    {
    return success;
    }

  return ReadFromTransformFile(refNode);
}

//----------------------------------------------------------------------------
int vtkMRMLTransformStorageNode::WriteToTransformFile(vtkMRMLNode *refNode)
{
  vtkMRMLTransformNode *transformNode = vtkMRMLTransformNode::SafeDownCast(refNode);
  if (transformNode == nullptr)
    {
    vtkErrorMacro("Unexpected node type, cannot read transform from file");
    return 0;
    }

  // Get VTK transform from the transform node
  vtkAbstractTransform* transformVtk = transformNode->GetTransformFromParent();
  if (transformVtk==nullptr)
    {
    this->SetWriteStateSkippedNoData();
    return 1;
    }

  // Convert VTK transform to ITK transform
  itk::Object::Pointer secondaryTransformItk; // only used for ITKv3 compatibility
  // ITK transform is created without initialization, because initialization may take a long time for certain transform types
  // which would slow down saving. Initialization is only needed for computing transformations, not necessary for file writing.
  itk::Object::Pointer transformItk = vtkITKTransformConverter::CreateITKTransformFromVTK(
    this, transformVtk, secondaryTransformItk, this->PreferITKv3CompatibleTransforms, false);
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
int vtkMRMLTransformStorageNode::WriteToImageFile(vtkMRMLNode *refNode)
{
  vtkMRMLTransformNode *transformNode = vtkMRMLTransformNode::SafeDownCast(refNode);
  if (transformNode == nullptr)
    {
    vtkErrorMacro("Unexpected node type, cannot read transform from file");
    return 0;
    }

  vtkOrientedGridTransform* gridTransform_Ras =  vtkOrientedGridTransform::SafeDownCast(transformNode->GetTransformFromParentAs("vtkOrientedGridTransform"));
  if (gridTransform_Ras==nullptr)
    {
    vtkErrorMacro("Cannot retrieve grid transform from node");
    return 0;
    }

  // Update is needed because it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
  gridTransform_Ras->Update();
  if (gridTransform_Ras->GetInverseFlag())
    {
    vtkErrorMacro("Cannot write inverse grid transform to image file. Either save the transform in a transform file (.h5) or invert the transform before saving it into an image file.");
    return 0;
    }

  GridImageDoubleType::Pointer gridImage_Lps;
  vtkITKTransformConverter::SetITKImageFromVTKOrientedGridTransform(this, gridImage_Lps, gridTransform_Ras);

  itk::ImageFileWriter<GridImageDoubleType>::Pointer writer = itk::ImageFileWriter<GridImageDoubleType>::New();
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
  if (transformNode == nullptr)
    {
    vtkErrorMacro("vtkMRMLTransformNode write data failed: invalid transform node");
    return 0;
    }
  if (IsImageFile(fullName))
    {
    return WriteToImageFile(transformNode);
    }
  else
    {
    return WriteToTransformFile(transformNode);
    }
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
bool vtkMRMLTransformStorageNode::IsImageFile(const std::string &filename)
{
  // determine file type
  std::string extension = this->GetSupportedFileExtension(filename.c_str());
  if( extension.empty() )
    {
    vtkErrorMacro("ReadData: no file extension specified: " << filename.c_str());
    return false;
    }
  if ( !extension.compare(".nrrd")
      || !extension.compare(".nhdr")
      || !extension.compare(".mha")
      || !extension.compare(".mhd")
      || !extension.compare(".nii")
      || !extension.compare(".nii.gz"))
    {
    return true;
    }

  // All other file extensions are transforms
  return false;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformStorageNode::SetAndObserveTransformFromParentAutoInvert(vtkMRMLTransformNode* transformNode, vtkAbstractTransform *transform)
{
  bool allInvertedTransforms = true;

  // Flatten the transform list to make the interpretation simpler
  vtkNew<vtkCollection> sourceTransformList;
  vtkMRMLTransformNode::FlattenGeneralTransform(sourceTransformList.GetPointer(), transform);
  // Check if they are all inverse, if they are, then it indicates that this transform is computed from its inverse
  vtkCollectionSimpleIterator it;
  vtkWarpTransform* concatenatedTransform = nullptr;
  for (sourceTransformList->InitTraversal(it); (concatenatedTransform = vtkWarpTransform::SafeDownCast(sourceTransformList->GetNextItemAsObject(it))) ;)
    {
    if (concatenatedTransform)
      {
      // Update is needed because it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
      concatenatedTransform->Update();
      if (!concatenatedTransform->GetInverseFlag())
        {
        // found a non-inverse transform, so it's not allInvertedTransforms
        allInvertedTransforms = false;
        break;
        }
      }
    }

  if (allInvertedTransforms)
    {
    // we invert the transform to store the forward transform
    transform->Inverse();
    transformNode->SetAndObserveTransformToParent(transform);
    }
  else
    {
    // this is a forward (or mixed) transform already so store it as is
    transformNode->SetAndObserveTransformFromParent(transform);
    }
}
