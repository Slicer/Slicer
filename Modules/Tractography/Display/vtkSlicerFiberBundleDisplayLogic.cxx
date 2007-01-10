/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerFiberBundleDisplayLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <itksys/SystemTools.hxx> 
#include <itksys/Directory.hxx> 

#include "vtkSlicerFiberBundleDisplayLogic.h"

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"

#include <sstream>

vtkCxxRevisionMacro(vtkSlicerFiberBundleDisplayLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerFiberBundleDisplayLogic);

//----------------------------------------------------------------------------
vtkSlicerFiberBundleDisplayLogic::vtkSlicerFiberBundleDisplayLogic()
{
  vtkDebugWithObjectMacro(this,"Making it real");
  this->LineModelNode = NULL;
  this->TubeModelNode = NULL;
  this->GlyphModelNode = NULL;

  this->LineModelDisplayNode = NULL;
  this->TubeModelDisplayNode = NULL;
  this->GlyphModelDisplayNode = NULL;

  this->FiberBundleNode = NULL;

}

//----------------------------------------------------------------------------
vtkSlicerFiberBundleDisplayLogic::~vtkSlicerFiberBundleDisplayLogic()
{

  if ( this->FiberBundleNode ) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->FiberBundleNode, NULL );
    }

  this->DeleteLineModelNode();  
}


//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::SetAndObserveFiberBundleNode( vtkMRMLFiberBundleNode *fiberBundleNode )
{

  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLFiberBundleNode::DisplayModifiedEvent);
  events->InsertNextValue(vtkMRMLFiberBundleNode::PolyDataModifiedEvent);
  vtkSetAndObserveMRMLNodeEventsMacro(this->FiberBundleNode, fiberBundleNode, events );
  events->Delete();

  // Now that we have a fiber bundle node, display it.
  // Lauren TO DO: this caused a crash
  this->CreateLineModelNode();
}

void vtkSlicerFiberBundleDisplayLogic::CreateLineModelNode ( )
{

  // if we have no MRML scene yet 
  if (this->MRMLScene == NULL)
    {
    vtkErrorMacro("Cannot create line model node, no MRMLScene set yet.");
    return;
    }

  // if the modelNode exists and is not in the scene, delete it and then redo it
  if (this->LineModelNode != NULL && this->MRMLScene->GetNodeByID( this->LineModelNode->GetID() ) == NULL )
    {
    this->DeleteLineModelNode();
    }


  // if the modelNode does not exist set it up again totally
  if ( this->LineModelNode == NULL) 
    {
  
    this->LineModelNode = vtkMRMLModelNode::New();
    this->LineModelNode->SetScene(this->GetMRMLScene());
    this->LineModelNode->SetHideFromEditors(1);
    this->LineModelNode->SetSaveWithScene(0);
    
    // create display node
    this->LineModelDisplayNode = vtkMRMLModelDisplayNode::New();
    this->LineModelDisplayNode->SetScene(this->GetMRMLScene());
    this->LineModelDisplayNode->SetSaveWithScene(0);

    // give it a name
    std::stringstream ss;
    char name[256];
    ss << this->FiberBundleNode->GetName() << " " << "Line Model Node";
    ss.getline(name,256);
    this->LineModelNode->SetName(name);


    this->LineModelNode->SetAndObserveDisplayNodeID(this->LineModelDisplayNode->GetID());
    }

  // update the polydata and display parameters:
  if (this->FiberBundleNode != NULL) 
    {
    // get polylines from the fiber bundle node
    this->LineModelNode->SetAndObservePolyData(this->FiberBundleNode->GetPolyData());

    // update the polydata and display parameters:
    // set properties according to the fiber bundle's display node
    vtkMRMLFiberBundleDisplayNode * fiberBundleDisplayNode = this->FiberBundleNode->GetDisplayNode();
    if (fiberBundleDisplayNode != NULL)
      {
      this->LineModelDisplayNode->SetVisibility( fiberBundleDisplayNode->GetVisibility ( ) );
      this->LineModelDisplayNode->SetOpacity( fiberBundleDisplayNode->GetOpacity ( ) );
      this->LineModelDisplayNode->SetColor( fiberBundleDisplayNode->GetColor ( ) );
      this->LineModelDisplayNode->SetAmbient( fiberBundleDisplayNode->GetAmbient ( ) );
      this->LineModelDisplayNode->SetDiffuse( fiberBundleDisplayNode->GetDiffuse ( ) );
      }
    }

  // if the modelNode exists but is not in the scene, put it there
  if (this->LineModelNode != NULL && this->MRMLScene->GetNodeByID( this->LineModelNode->GetID() ) == NULL )
    {
    this->MRMLScene->AddNode(this->LineModelDisplayNode);
    this->MRMLScene->AddNode(this->LineModelNode);
    this->LineModelNode->SetAndObserveDisplayNodeID(this->LineModelDisplayNode->GetID());
    }

} 

void vtkSlicerFiberBundleDisplayLogic::DeleteLineModelNode ( )
{

  if (this->LineModelNode != NULL)
    {
    this->LineModelNode->SetAndObserveDisplayNodeID(NULL);
    this->LineModelNode->SetAndObservePolyData(NULL);
    this->LineModelNode->Delete();
    this->LineModelNode = NULL;
    }
  if (this->LineModelDisplayNode != NULL)
    {
    this->LineModelDisplayNode->Delete();
    this->LineModelDisplayNode = NULL;
    }
} 

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::ProcessMRMLEvents( vtkObject * caller, 
                                            unsigned long event, 
                                            void * callData )
{


  if (vtkMRMLFiberBundleNode::SafeDownCast(caller) != NULL
      && (event == vtkMRMLFiberBundleNode::DisplayModifiedEvent || event == vtkMRMLFiberBundleNode::PolyDataModifiedEvent))
    {
    
    vtkDebugWithObjectMacro(this,"Hooray caught a display modified event");
    vtkErrorWithObjectMacro(this,"Hooray caught a display modified event from fiber bundle node");
    this->CreateLineModelNode();
    vtkErrorWithObjectMacro(this,"Done creating line model!!!!");
    }

  vtkDebugWithObjectMacro(this,"Process MRML Events " << event );
  vtkErrorWithObjectMacro(this,"Process MRML Events " << event );
}


//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerFiberBundleDisplayLogic:             " << this->GetClassName() << "\n";

}


