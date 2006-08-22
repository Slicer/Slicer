/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerFiducialsLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <vtksys/SystemTools.hxx> 

#include "vtkSlicerFiducialsLogic.h"

#include "vtkMRMLFiducialNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLFiducialListDisplayNode.h"

vtkCxxRevisionMacro(vtkSlicerFiducialsLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerFiducialsLogic);

//----------------------------------------------------------------------------
vtkSlicerFiducialsLogic::vtkSlicerFiducialsLogic()
{
  this->ActiveFiducialListNode = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerFiducialsLogic::~vtkSlicerFiducialsLogic()
{
  if (this->ActiveFiducialListNode != NULL)
    {
        this->ActiveFiducialListNode->Delete();
        this->ActiveFiducialListNode = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerFiducialsLogic::ProcessMRMLEvents()
{
  // TODO: implement if needed
}

//----------------------------------------------------------------------------
void vtkSlicerFiducialsLogic::SetActiveFiducialListNode(vtkMRMLFiducialListNode *activeNode)
{
  this->SetMRML( vtkObjectPointer(&this->ActiveFiducialListNode), activeNode );
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLFiducialListNode* vtkSlicerFiducialsLogic::AddFiducials ()
{
    std::cout << "vtkSlicerFiducialsLogic::AddFiducials : making a new fiducial list node\n";
  vtkMRMLFiducialListNode *modelNode = vtkMRMLFiducialListNode::New();

  std::cout << "\tmaking a new display node\n";
// this creates an empty display node for now
  vtkMRMLFiducialListDisplayNode *displayNode = vtkMRMLFiducialListDisplayNode::New();

    /*
  storageNode->SetFileName(filename);
  if (storageNode->ReadData(modelNode) != 0)
    {
    const vtksys_stl::string fname(filename);
    vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
    modelNode->SetName(name.c_str());

    this->GetMRMLScene()->SaveStateForUndo();

    modelNode->SetScene(this->GetMRMLScene());
    storageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene());

    this->GetMRMLScene()->AddNode(storageNode);  
    this->GetMRMLScene()->AddNode(displayNode);
    modelNode->SetStorageNodeID(storageNode->GetID());
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());  

    this->GetMRMLScene()->AddNode(modelNode);  

    this->Modified();  
    }
  */

   this->GetMRMLScene()->SaveStateForUndo();

    modelNode->SetScene(this->GetMRMLScene());
   
    // now that the list node has a scene, give it a unique name
    if (modelNode != NULL && modelNode->GetScene() != NULL)
    {
        modelNode->SetName(modelNode->GetScene()->GetUniqueIDByClass("FiducialList"));
    } else {
        std::cerr << "modelNode Scene is null, not changing the model node name\n";
    }

    this->GetMRMLScene()->AddNode(displayNode);
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());
    displayNode->SetScene(this->GetMRMLScene());

    this->GetMRMLScene()->AddNode(modelNode);

    // set it to be active
    this->SetActiveFiducialListNode(modelNode);
        
  modelNode->Delete();
  displayNode->Delete();

  return modelNode;  
}

//----------------------------------------------------------------------------
vtkMRMLFiducialNode *vtkSlicerFiducialsLogic::AddFiducial()
{
    // get the Fiducials list
    //vtkMRMLFiducialListNode *listNode = this->GetActiveFiducialListNode();
    // make a new fiducial node
    vtkMRMLFiducialNode * modelNode = vtkMRMLFiducialNode::New();
    
    // add it to the list
    if (this->GetActiveFiducialListNode() != NULL)
    {
        this->GetActiveFiducialListNode()->AddFiducialNode(modelNode);
    }
    else
    {
        std::cerr << "Error: the active ficucials list node is NULL\n";
        return NULL;
    }
    this->GetMRMLScene()->SaveStateForUndo();

    modelNode->SetScene(this->GetMRMLScene());

    // now that the point is in the scene, give it a unique name
    modelNode->SetName(modelNode->GetScene()->GetUniqueIDByClass(this->GetActiveFiducialListNode()->GetName()));
    
    modelNode->Delete();
    return modelNode;
    

/*
    this->GetMRMLScene()->SaveStateForUndo();

    modelNode->SetScene(this->GetMRMLScene());
    this->GetMRMLScene()->AddNode(displayNode);
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());  

    this->GetMRMLScene()->AddNode(modelNode);  
  
    modelNode->Delete();
    displayNode->Delete();

    return modelNode;
*/
}

//----------------------------------------------------------------------------
void vtkSlicerFiducialsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerFiducialsLogic:             " << this->GetClassName() << "\n";

  os << indent << "ActiveFiducialListNode: " <<
    (this->ActiveFiducialListNode ? this->ActiveFiducialListNode->GetName() : "(none)") << "\n";
}
