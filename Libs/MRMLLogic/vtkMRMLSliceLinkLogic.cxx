/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceLinkLogic.cxx,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLSliceLinkLogic.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>

// STD includes
#include <cassert>


//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMRMLSliceLinkLogic, "$Revision$");
vtkStandardNewMacro(vtkMRMLSliceLinkLogic);

//----------------------------------------------------------------------------
vtkMRMLSliceLinkLogic::vtkMRMLSliceLinkLogic()
{
  this->BroadcastingEvents = 0;
}

//----------------------------------------------------------------------------
vtkMRMLSliceLinkLogic::~vtkMRMLSliceLinkLogic()
{

}


//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  // List of events the slice logics should listen
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneAboutToBeClosedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneRestoredEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  this->SetAndObserveMRMLSceneEventsInternal(newScene, events);

  this->ProcessLogicEvents();
  this->ProcessMRMLEvents(newScene, vtkCommand::ModifiedEvent, 0);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::ProcessMRMLEvents(vtkObject * caller, 
                                            unsigned long event, 
                                            void * callData )
{
  // SliceLinkLogic needs to observe EVERY SliceNode and
  // SliceCompositeNode in the scene.
  if (vtkMRMLScene::SafeDownCast(caller) == this->GetMRMLScene())
    {
    if (event == vtkMRMLScene::NodeAddedEvent 
        || event == vtkMRMLScene::NodeRemovedEvent)
      {
      vtkMRMLNode *node =  reinterpret_cast<vtkMRMLNode*> (callData);
      if (!node)
        {
        return;
        }
      // Return if different from SliceCompositeNode or SliceNode 
      if (node->IsA("vtkMRMLSliceCompositeNode") 
          || node->IsA("vtkMRMLSliceNode"))
        {
        if (event == vtkMRMLScene::NodeAddedEvent)
          {
          vtkEventBroker::GetInstance()->AddObservation(node, vtkCommand::ModifiedEvent, this, this->GetMRMLCallbackCommand());
          }
        else if (event == vtkMRMLScene::NodeRemovedEvent)
          {
          vtkEventBroker::GetInstance()->RemoveObservations(node, vtkCommand::ModifiedEvent, this, this->GetMRMLCallbackCommand());
          }
        return;
        }
      }

    if (event == vtkMRMLScene::SceneAboutToBeClosedEvent ||
        caller == 0)
      {
      return;
      }
    }
  if (this->GetMRMLScene()->GetIsClosing())
    {
    // Do we need to remove the observers?
    return;
    }


  // Update from SliceNode
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(caller);
  if (sliceNode && !this->GetMRMLScene()->GetIsUpdating())
    {
    assert (event == vtkCommand::ModifiedEvent);

    // if this is not the node that we are interacting with, short circuit
    if (!sliceNode->GetInteracting() || !sliceNode->GetInteractionFlags())
      {
      return;
      }

    // SliceNode was modified. Need to find the corresponding
    // SliceCompositeNode to check whether operations are linked
    vtkMRMLSliceCompositeNode *compositeNode = 0;
    int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
    for (int n=0; n<nnodes; n++)
      {
      // should we cache this list of composite nodes?
      // or should we have special query methods in MRMLScene that
      // will be fast?????

      compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast (
        this->GetMRMLScene()->GetNthNodeByClass(n,"vtkMRMLSliceCompositeNode"));

      // Is the the composite node that goes with this slice node?
      if (compositeNode->GetLayoutName() 
          && !strcmp(compositeNode->GetLayoutName(), sliceNode->GetName()))
        {
        // Matching layout
        break;
        }

      compositeNode = 0;
      }

    if (compositeNode && compositeNode->GetLinkedControl())
      {
      // Slice node changed and slices are linked. Broadcast.
      //std::cout << "Slice node changed and slices are linked!" << std::endl;
      this->BroadcastSliceNodeEvent(sliceNode);
      }
    else
      {
      // Slice node changed and slices are not linked. Do not broadcast.
      //std::cout << "Slice node changed and slices are NOT linked!" << std::endl;
      return;
      }
    }

  // Update from SliceCompositeNode
  vtkMRMLSliceCompositeNode* compositeNode 
    = vtkMRMLSliceCompositeNode::SafeDownCast(caller);
  if (compositeNode && !this->GetMRMLScene()->GetIsUpdating())
    {
    assert (event == vtkCommand::ModifiedEvent);

    // if this is not the node that we are interacting with, short circuit
    if (!compositeNode->GetInteracting() 
        || !compositeNode->GetInteractionFlags())
      {
      return;
      }

    if (compositeNode && compositeNode->GetLinkedControl())
      {
      // Slice composite node changed and slices are linked. Broadcast.
      //std::cout << "SliceCompositeNode changed and slices are linked!" << std::endl;
      this->BroadcastSliceCompositeNodeEvent(compositeNode);
      }
    else
      {
      // Slice composite node changed and slices are not linked. Do
      // not broadcast.
      //std::cout << "SliceCompositeNode changed and slices are NOT linked!" << std::endl;
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::ProcessLogicEvents()
{
}



//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();

}


//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::BroadcastSliceNodeEvent(vtkMRMLSliceNode *sliceNode)
{
  // only broadcast a slice node event if we are not already actively
  // broadcasting events and we actively interacting with the node
  // std::cout << "BroadcastingEvents: " << this->GetBroadcastingEvents()
  //           << ", Interacting: " << sliceNode->GetInteracting()
  //           << std::endl;
  if (!this->GetBroadcastingEvents() && sliceNode->GetInteracting())
    {
    this->BroadcastingEventsOn();

    vtkMRMLSliceNode *sNode = 0;
    int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceNode");
    for (int n=0; n<nnodes; n++)
      {
      sNode = vtkMRMLSliceNode::SafeDownCast (
        this->GetMRMLScene()->GetNthNodeByClass(n,"vtkMRMLSliceNode"));
      
      if (sNode != sliceNode)
        {
        // Link slice parameters whenever the reformation is consistent
        if (!strcmp(sNode->GetOrientationString(), 
                    sliceNode->GetOrientationString()))
        // if (sliceNode->Matrix4x4AreEqual(sliceNode->GetSliceToRAS(), 
        //                                  sNode->GetSliceToRAS()))
          {
          //std::cout << "Orientation match, flags = " << sliceNode->GetInteractionFlags() << std::endl;
          // std::cout << "Broadcasting SliceToRAS and FieldOfView to "
          //           << sNode->GetName() << std::endl;
          // 

          // Copy the slice to RAS information
          if (sliceNode->GetInteractionFlags() & vtkMRMLSliceNode::SliceToRASFlag)
            {
            // Need to copy the SliceToRAS. SliceNode::SetSliceToRAS()
            // does a shallow copy. So we have to explictly call DeepCopy()
            sNode->GetSliceToRAS()->DeepCopy( sliceNode->GetSliceToRAS() );
            }

          // Copy the field of view information. Use the new
          // prescribed x fov, aspect corrected y fov, and keep z fov
          // constant
          if (sliceNode->GetInteractionFlags() & vtkMRMLSliceNode::FieldOfViewFlag)
            {
            sNode->SetFieldOfView( sliceNode->GetFieldOfView()[0], 
                                   sliceNode->GetFieldOfView()[0] 
                                   * sNode->GetFieldOfView()[1] 
                                   / sNode->GetFieldOfView()[0], 
                                   sNode->GetFieldOfView()[2] );
            }

          // need to manage prescribed spacing here as well?

          // Forces the internal matrices to be updated which results
          // in this being modified so a Render can occur
          sNode->UpdateMatrices();
          }
        else
          {
          // Setting the orientation of the slice plane does not
          // require that the orientations initially match.
          // std::cout << "Orientation mismatch, flags = " << sliceNode->GetInteractionFlags() << std::endl;
          if (sliceNode->GetInteractionFlags() & vtkMRMLSliceNode::OrientationFlag)
            {
            // We could copy the orientation strings, but we really
            // want the slice to ras to match, so copy that

            // Need to copy the SliceToRAS. SliceNode::SetSliceToRAS()
            // does a shallow copy. So we have to explictly call DeepCopy()
            sNode->GetSliceToRAS()->DeepCopy( sliceNode->GetSliceToRAS() );

            // Forces the internal matrices to be updated which results
            // in this being modified so a Render can occur
            sNode->UpdateMatrices();
            }
          }
        }
      }

    this->BroadcastingEventsOff();
    }
  //std::cout << "End Broadcast" << std::endl;
}


//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::BroadcastSliceCompositeNodeEvent(vtkMRMLSliceCompositeNode *sliceCompositeNode)
{
  // only broadcast a slice composite node event if we are not already actively
  // broadcasting events and we actively interacting with the node
  // std::cout << "BroadcastingEvents: " << this->GetBroadcastingEvents()
  //           << ", Interacting: " << sliceCompositeNode->GetInteracting()
  //           << std::endl;
  if (!this->GetBroadcastingEvents() && sliceCompositeNode->GetInteracting())
    {
    this->BroadcastingEventsOn();

    vtkMRMLSliceCompositeNode *cNode = 0;
    int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
    for (int n=0; n<nnodes; n++)
      {
      cNode = vtkMRMLSliceCompositeNode::SafeDownCast (
        this->GetMRMLScene()->GetNthNodeByClass(n,"vtkMRMLSliceCompositeNode"));
      
      if (cNode != sliceCompositeNode)
        {
        if (sliceCompositeNode->GetInteractionFlags() 
            & vtkMRMLSliceCompositeNode::ForegroundVolumeFlag)
          {
          cNode->SetForegroundVolumeID(sliceCompositeNode->GetForegroundVolumeID());
          }

        if (sliceCompositeNode->GetInteractionFlags() 
            & vtkMRMLSliceCompositeNode::BackgroundVolumeFlag)
          {
          cNode->SetBackgroundVolumeID(sliceCompositeNode->GetBackgroundVolumeID());
          }

        if (sliceCompositeNode->GetInteractionFlags() 
            & vtkMRMLSliceCompositeNode::ForegroundVolumeFlag)
          {
          cNode->SetLabelVolumeID(sliceCompositeNode->GetLabelVolumeID());
          }

        }
      }

    this->BroadcastingEventsOff();
    }
}
