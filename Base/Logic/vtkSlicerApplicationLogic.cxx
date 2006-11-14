/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkSlicerApplicationLogic.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLModelNode.h"

#include "itksys/SystemTools.hxx"


vtkCxxRevisionMacro(vtkSlicerApplicationLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerApplicationLogic);

//----------------------------------------------------------------------------
vtkSlicerApplicationLogic::vtkSlicerApplicationLogic()
{
    this->Views = NULL;
    this->Slices = NULL;
    this->Modules = NULL;
    this->ActiveSlice = NULL;
    this->SelectionNode = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerApplicationLogic::~vtkSlicerApplicationLogic()
{
  if (this->Views)
    {
        this->Views->Delete();
        this->Views = NULL;
    }
  if (this->Slices)
    {
        this->Slices->Delete();
        this->Slices = NULL;
    }
  if (this->Modules)
    {
        this->Modules->Delete();
        this->Modules = NULL;
    }
  this->SetSelectionNode ( NULL );
  this->SetActiveSlice ( NULL );


  // TODO - unregister/delete ivars
}


//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::ClearCollections ( ) {
    if ( this->Views) {
        this->Views->RemoveAllItems ( );
    }
    if ( this->Slices ) {
        this->Slices->RemoveAllItems ( );
    }
    if ( this->Modules ) {
        this->Modules->RemoveAllItems ( );
    }
}


//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::ProcessMRMLEvents(vtkObject * /*caller*/, 
                                            unsigned long /*event*/, 
                                            void * /*callData*/ )
{


  //
  // Look for a selection node in the scene
  // - we always use the first one in the scene
  // - if it doesn't match the one we had, we switch
  // - if there isn't one, we create one
  // - we add it to the scene if needed
  //
  vtkMRMLSelectionNode *node;
  node = vtkMRMLSelectionNode::SafeDownCast (
          this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));

  if ( node == NULL )
    {
    node = vtkMRMLSelectionNode::New();
    this->SetSelectionNode (node);
    node->Delete();
    }

  if ( this->SelectionNode != node )
    {
      this->SetSelectionNode (node);
    }

  if (this->MRMLScene->GetNodeByID(this->SelectionNode->GetID()) == NULL)
    {
    this->SetMRMLScene(this->GetMRMLScene());
    this->MRMLScene->AddNode(this->SelectionNode);
    this->SetAndObserveMRMLScene(this->GetMRMLScene());
    }
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::PropagateVolumeSelection()
{
  if ( !this->SelectionNode || !this->MRMLScene )
    {
    return;
    }

  int i, nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  char *ID = this->SelectionNode->GetActiveVolumeID();
  char *labelID = this->SelectionNode->GetActiveLabelVolumeID();

  vtkMRMLSliceCompositeNode *cnode;
  for (i = 0; i < nnodes; i++)
    {
    cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
            this->MRMLScene->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
    cnode->SetBackgroundVolumeID( ID );
    cnode->SetLabelVolumeID( labelID );
    }
}

//----------------------------------------------------------------------------
// Create a new Slice with it's associated class instances
vtkSlicerSliceLogic *vtkSlicerApplicationLogic::CreateSlice ()
{
    // Create the logic instances
    vtkSlicerSliceLogic *sliceLogic = vtkSlicerSliceLogic::New();
    vtkSlicerSliceLayerLogic *bg = vtkSlicerSliceLayerLogic::New();
    vtkSlicerSliceLayerLogic *fg = vtkSlicerSliceLayerLogic::New();

    // Create the mrml nodes to store state
    vtkMRMLSliceNode *sliceNode = vtkMRMLSliceNode::New();
    this->MRMLScene->AddNode(sliceNode);

    // Configure the logic
    sliceLogic->SetBackgroundLayer(bg);
    sliceLogic->SetForegroundLayer(fg);
    sliceLogic->SetSliceNode(sliceNode);

    // Update internal state
    this->Slices->AddItem(sliceLogic);
    this->SetActiveSlice(sliceLogic);

    // Since they were New(), they should be Deleted(). If it crashes
    // then something is not ref-counted properly and should be fixed
    // (otherwise you are just leaking)

    sliceLogic->Delete();
    bg->Delete();
    fg->Delete();
    sliceNode->Delete();

    return (sliceLogic);
}

//----------------------------------------------------------------------------
void vtkSlicerApplicationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "SlicerApplicationLogic:             " << this->GetClassName() << "\n"; 
} 

