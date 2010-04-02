/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerTransformLogic.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>


#include "vtkSlicerTransformLogic.h"

#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformStorageNode.h"

#include "vtkStringArray.h"

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


//----------------------------------------------------------------------------
vtkMRMLTransformNode* vtkSlicerTransformLogic::AddTransform (const char* filename, vtkMRMLScene *scene)
{
  vtkMRMLTransformStorageNode *storageNode = vtkMRMLTransformStorageNode::New();

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
      std::string uname( this->MRMLScene->GetUniqueNameByString(name.c_str()));
      tnode->SetName(uname.c_str());
      scene->AddNodeNoNotify(storageNode);
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
