/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkSlicerTransformLogic.h"

// MRML includes
#include "vtkCacheManager.h"
#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformStorageNode.h"

// VTKsys includes
#include <itksys/SystemTools.hxx>

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkSmartPointer.h>

// ITK includes
#include "itkBSplineDeformableTransform.h"
#include "itkCenteredAffineTransform.h"
#include "itkCenteredEuler3DTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkCenteredSimilarity2DTransform.h"
#include "itkEuler2DTransform.h"
#include "itkFixedCenterOfRotationAffineTransform.h"
#include "itkQuaternionRigidTransform.h"
#include "itkRigid3DPerspectiveTransform.h"
#include "itkScaleLogarithmicTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkScaleVersor3DTransform.h"
#include "itkTranslationTransform.h"
#include "itkTransformFactory.h"

vtkCxxRevisionMacro(vtkSlicerTransformLogic, "$Revision$");
vtkStandardNewMacro(vtkSlicerTransformLogic);

//----------------------------------------------------------------------------
vtkSlicerTransformLogic::vtkSlicerTransformLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerTransformLogic::~vtkSlicerTransformLogic()
{
}

//-----------------------------------------------------------------------------
bool vtkSlicerTransformLogic::hardenTransform(vtkMRMLTransformableNode* transformableNode)
{
  vtkMRMLTransformNode* transformNode =
    transformableNode ? transformableNode->GetParentTransformNode() : 0;
  if (!transformNode)
    {
    return false;
    }
  if (transformNode->IsTransformToWorldLinear())
    {
    vtkSmartPointer<vtkMatrix4x4> hardeningMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    transformNode->GetMatrixTransformToWorld(hardeningMatrix);
    transformableNode->ApplyTransformMatrix(hardeningMatrix);
    }
  else
    {
    vtkSmartPointer<vtkGeneralTransform> hardeningTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    transformNode->GetTransformToWorld(hardeningTransform);
    transformableNode->ApplyTransform(hardeningTransform);
    }

  transformableNode->SetAndObserveTransformNodeID(NULL);
  transformableNode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
  transformableNode->SetModifiedSinceRead(1);
  return true;
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode* vtkSlicerTransformLogic::AddTransform (const char* filename, vtkMRMLScene *scene)
{
  vtkMRMLTransformStorageNode *storageNode = vtkMRMLTransformStorageNode::New();

  if(scene == NULL){
    vtkErrorMacro("scene == NULL in vtkSlicerTransformLogic::AddTransform");
    return NULL;
  }

  // check for local or remote files
  int useURI = 0; // false;
  if (scene->GetCacheManager() != NULL)
    {
    useURI = scene->GetCacheManager()->IsRemoteReference(filename);
    }

  itksys_stl::string name;
  const char *localFile;
  if (useURI)
    {
    vtkDebugMacro("AddTransforn: file name is remote: " << filename);
    storageNode->SetURI(filename);
    // reset filename to the local file name
    localFile = ((scene)->GetCacheManager())->GetFilenameFromURI(filename);
    }
  else
    {
    storageNode->SetFileName(filename);
    localFile = filename;
    }

  const itksys_stl::string fname(localFile);
  // the model name is based on the file name (itksys call should work even if
  // file is not on disk yet)
  name = itksys::SystemTools::GetFilenameName(fname);

  // check to see which node can read this type of file
  vtkMRMLTransformNode *tnode = NULL;

  if (storageNode->SupportedFileType(name.c_str()))
    {
    scene->SaveStateForUndo();
    storageNode->SetScene(scene);

    // now set up the reading
    vtkMRMLGridTransformNode    *gridTfm = vtkMRMLGridTransformNode::New();
    vtkMRMLBSplineTransformNode *bsplineTfm = vtkMRMLBSplineTransformNode::New();
    vtkMRMLLinearTransformNode  *linearTfm = vtkMRMLLinearTransformNode::New();

    gridTfm->SetScene(scene);
    bsplineTfm->SetScene(scene);
    linearTfm->SetScene(scene);

    if (storageNode->ReadData(gridTfm))
      {
      tnode = gridTfm;
      }
    else if (storageNode->ReadData(bsplineTfm))
      {
      tnode = bsplineTfm;
      }
    else if (storageNode->ReadData(linearTfm))
      {
      tnode = linearTfm;
      }

    if (tnode)
      {
      const itksys_stl::string basename(
        itksys::SystemTools::GetFilenameWithoutExtension(fname));
      const std::string uname( scene->GetUniqueNameByString(basename.c_str()));
      tnode->SetName(uname.c_str());
      scene->AddNode(storageNode);
      scene->AddNode(tnode);

      tnode->SetAndObserveStorageNodeID(storageNode->GetID());
      }
    gridTfm->Delete();
    bsplineTfm->Delete();
    linearTfm->Delete();
    }
  else
    {
    vtkErrorMacro("Unsupported transform file format: " << filename);
    }
  storageNode->Delete();

  return tnode;
}

int vtkSlicerTransformLogic::SaveTransform (const char* vtkNotUsed(filename), 
                                            vtkMRMLTransformNode *vtkNotUsed(transformNode))
{
  return 1;
}
