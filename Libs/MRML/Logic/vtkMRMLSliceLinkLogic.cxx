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
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLApplicationLogic.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

// STD includes
#include <cassert>


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLSliceLinkLogic);

//----------------------------------------------------------------------------
vtkMRMLSliceLinkLogic::vtkMRMLSliceLinkLogic()
{
  this->BroadcastingEvents = 0;
}

//----------------------------------------------------------------------------
vtkMRMLSliceLinkLogic::~vtkMRMLSliceLinkLogic() = default;

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::BroadcastingEventsOn()
{
  this->BroadcastingEvents++;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::BroadcastingEventsOff()
{
  this->BroadcastingEvents--;
}

//----------------------------------------------------------------------------
int vtkMRMLSliceLinkLogic::GetBroadcastingEvents()
{
  return this->BroadcastingEvents;
}


//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  // List of events the slice logics should listen
  vtkNew<vtkIntArray> events;
  vtkNew<vtkFloatArray> priorities;

  float normalPriority = 0.0;
  float lowPriority = -0.5;
  // float highPriority = 0.5;

  // Events that use the default priority.  Don't care the order they
  // are triggered
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  priorities->InsertNextValue(normalPriority);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  priorities->InsertNextValue(normalPriority);
  events->InsertNextValue(vtkMRMLScene::StartBatchProcessEvent);
  priorities->InsertNextValue(normalPriority);
  events->InsertNextValue(vtkMRMLScene::StartImportEvent);
  priorities->InsertNextValue(normalPriority);
  events->InsertNextValue(vtkMRMLScene::StartRestoreEvent);
  priorities->InsertNextValue(normalPriority);

  // Events that need to a lower priority than normal, in other words,
  // guaranteed to be be called after other triggers
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  priorities->InsertNextValue(lowPriority);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  priorities->InsertNextValue(lowPriority);
  events->InsertNextValue(vtkMRMLScene::EndRestoreEvent);
  priorities->InsertNextValue(lowPriority);

  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer(), priorities.GetPointer());

  this->ProcessMRMLSceneEvents(newScene, vtkCommand::ModifiedEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node)
    {
    return;
    }
  if (node->IsA("vtkMRMLSliceCompositeNode")
      || node->IsA("vtkMRMLSliceNode"))
    {
    vtkEventBroker::GetInstance()->AddObservation(
      node, vtkCommand::ModifiedEvent, this, this->GetMRMLNodesCallbackCommand());

    // If sliceNode we insert in our map the current status of the node
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
    SliceNodeStatusMap::iterator it = this->SliceNodeInteractionStatus.find(node->GetID());
    if (sliceNode && it == this->SliceNodeInteractionStatus.end())
      {
      this->SliceNodeInteractionStatus.insert(std::pair<std::string, SliceNodeInfos>
        (sliceNode->GetID(), SliceNodeInfos(sliceNode->GetInteracting())));
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (!node)
    {
    return;
    }
  if (node->IsA("vtkMRMLSliceCompositeNode")
      || node->IsA("vtkMRMLSliceNode"))
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      node, vtkCommand::ModifiedEvent, this, this->GetMRMLNodesCallbackCommand());

    // Update the map
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
    if (sliceNode)
      {
      SliceNodeStatusMap::iterator it = this->SliceNodeInteractionStatus.find(node->GetID());
      if(it != this->SliceNodeInteractionStatus.end())
        {
        this->SliceNodeInteractionStatus.erase(it);
        }
      }
  }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::OnMRMLNodeModified(vtkMRMLNode* node)
{
  // Update from SliceNode
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if ( sliceNode && sliceNode->GetID()
    && this->GetMRMLScene() && !this->GetMRMLScene()->IsBatchProcessing() )
    {
    SliceNodeStatusMap::iterator it = this->SliceNodeInteractionStatus.find(sliceNode->GetID());
    // if this is not the node that we are interacting with, short circuit

    if (!sliceNode->GetInteracting() || !sliceNode->GetInteractionFlags())
      {
      // We end up an interaction on the sliceNode
      if (it != this->SliceNodeInteractionStatus.end() && it->second.Interacting)
        {
        vtkMRMLSliceCompositeNode* compositeNode = this->GetCompositeNode(sliceNode);
        if (!compositeNode->GetHotLinkedControl() &&
            sliceNode->GetInteractionFlags() == vtkMRMLSliceNode::MultiplanarReformatFlag)
          {
          this->BroadcastSliceNodeEvent(sliceNode);
          }
        this->SliceNodeInteractionStatus.find(sliceNode->GetID())->second.Interacting =
          sliceNode->GetInteracting();
        }
      return;
      }

    // SliceNode was modified. Need to find the corresponding
    // SliceCompositeNode to check whether operations are linked
    vtkMRMLSliceCompositeNode* compositeNode = this->GetCompositeNode(sliceNode);

    if (compositeNode && compositeNode->GetLinkedControl())
      {
      // Slice node changed and slices are linked. Broadcast.
      //std::cout << "Slice node changed and slices are linked!" << std::endl;

      if (it != this->SliceNodeInteractionStatus.end() && !it->second.Interacting )
        {
        it->second.Interacting = sliceNode->GetInteracting();
        // Start Interaction event : we update the current sliceNodeNormal
        this->UpdateSliceNodeInteractionStatus(sliceNode);
        }

      if (compositeNode->GetHotLinkedControl() ||
          sliceNode->GetInteractionFlags() != vtkMRMLSliceNode::MultiplanarReformatFlag)
        {
        this->BroadcastSliceNodeEvent(sliceNode);
        }
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
    = vtkMRMLSliceCompositeNode::SafeDownCast(node);
  if (compositeNode && this->GetMRMLScene() && !this->GetMRMLScene()->IsBatchProcessing())
    {

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
void vtkMRMLSliceLinkLogic::OnMRMLSceneStartBatchProcess()
{
  // Note the sense. Turning "on" tells the link logic that we are
  // already broadcasting an event, so don't rebroadcast.
  //std::cerr << "OnMRMLSceneStartBatchProcess" << std::endl;
  this->BroadcastingEventsOn();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::OnMRMLSceneEndBatchProcess()
{
  // Note the sense. Turning "off" tells the link logic that we are
  // not already broadcasting an event, so future events can be broadcast
  //std::cerr << "OnMRMLSceneEndBatchProcess" << std::endl;
  this->BroadcastingEventsOff();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::OnMRMLSceneStartImport()
{
  // Note the sense. Turning "on" tells the link logic that we are
  // already broadcasting an event, so don't rebroadcast.
  //std::cerr << "OnMRMLSceneStartImport" << std::endl;
  this->BroadcastingEventsOn();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::OnMRMLSceneEndImport()
{
  // Note the sense. Turning "off" tells the link logic that we are
  // not already broadcasting an event, so future events can be broadcast
  //std::cerr << "OnMRMLSceneEndImport" << std::endl;
  this->BroadcastingEventsOff();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::OnMRMLSceneStartRestore()
{
  // Note the sense. Turning "on" tells the link logic that we are
  // already broadcasting an event, so don't rebroadcast.
  //std::cerr << "OnMRMLSceneStartRestore" << std::endl;
  this->BroadcastingEventsOn();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::OnMRMLSceneEndRestore()
{
  // Note the sense. Turning "off" tells the link logic that we are
  // not already broadcasting an event, so future events can be broadcast
  //std::cerr << "OnMRMLSceneEndRestore" << std::endl;
  this->BroadcastingEventsOff();
}


//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  vtkIndent nextIndent;
  nextIndent = indent.GetNextIndent();
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceLinkLogic::IsOrientationMatching(vtkMRMLSliceNode *sliceNode1, vtkMRMLSliceNode *sliceNode2,
  double comparisonTolerance /* = 0.001 */)
{
  if (sliceNode1 == nullptr || sliceNode2 == nullptr)
    {
    vtkErrorMacro("vtkMRMLSliceLinkLogic::IsOrientationMatching failed: invalid input");
    return false;
    }
  vtkMatrix4x4* sliceToRAS1 = sliceNode1->GetSliceToRAS();
  vtkMatrix4x4* sliceToRAS2 = sliceNode2->GetSliceToRAS();
  for (int axisIndex = 0; axisIndex < 3; axisIndex++)
  {
    double axisVector1[3] = { sliceToRAS1->Element[0][axisIndex], sliceToRAS1->Element[1][axisIndex], sliceToRAS1->Element[2][axisIndex] };
    double axisVector2[3] = { sliceToRAS2->Element[0][axisIndex], sliceToRAS2->Element[1][axisIndex], sliceToRAS2->Element[2][axisIndex] };
    vtkMath::Normalize(axisVector1);
    vtkMath::Normalize(axisVector2);
    if ((fabs(axisVector1[0] - axisVector2[0]) > comparisonTolerance)
      || (fabs(axisVector1[1] - axisVector2[1]) > comparisonTolerance)
      || (fabs(axisVector1[2] - axisVector2[2]) > comparisonTolerance))
      {
      return false;
      }
  }
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::BroadcastSliceNodeEvent(vtkMRMLSliceNode *sliceNode)
{
  // only broadcast a slice node event if we are not already actively
  // broadcasting events and we are actively interacting with the node
  // std::cout << "BroadcastingEvents: " << this->GetBroadcastingEvents()
  //           << ", Interacting: " << sliceNode->GetInteracting()
  //           << std::endl;
  if (!sliceNode)
    {
    return;
    }
  if (!this->GetBroadcastingEvents())
    {
    this->BroadcastingEventsOn();

    int requiredViewGroup = sliceNode->GetViewGroup();
    vtkMRMLSliceNode* sNode;
    vtkCollectionSimpleIterator it;
    vtkSmartPointer<vtkCollection> nodes;
    nodes.TakeReference(this->GetMRMLScene()->GetNodesByClass("vtkMRMLSliceNode"));
    for (nodes->InitTraversal(it);
        (sNode=vtkMRMLSliceNode::SafeDownCast(nodes->GetNextItemAsObject(it)));)
      {
      if (sNode == sliceNode || !sNode)
        {
        continue;
        }
      if (sNode->GetViewGroup() != requiredViewGroup)
        {
        continue;
        }

      // Link slice parameters whenever the reformation is consistent
      if (vtkMRMLSliceLinkLogic::IsOrientationMatching(sliceNode, sNode))
        {
        // std::cout << "Orientation match, flags = " << sliceNode->GetInteractionFlags() << std::endl;
        // std::cout << "Broadcasting SliceToRAS, SliceOrigin, and FieldOfView to "
        //            << sNode->GetName() << std::endl;
        //

        // Copy the slice to RAS information
        if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
          & vtkMRMLSliceNode::SliceToRASFlag)
          {
          sNode->GetSliceToRAS()->DeepCopy( sliceNode->GetSliceToRAS() );
          }

        // Copy the slice origin information
        if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
          & vtkMRMLSliceNode::XYZOriginFlag)
          {
          // Need to copy the SliceOrigin.
          double *xyzOrigin = sliceNode->GetXYZOrigin();
          sNode->SetXYZOrigin( xyzOrigin[0], xyzOrigin[1], xyzOrigin[2] );
          }

        // Forces the internal matrices to be updated which results
        // in this being modified so a Render can occur
        sNode->UpdateMatrices();
        }

      //
      // Some parameters and commands do not require the
      // orientations to match. These are handled here.
      //

      // Keeping zoom factor the same among all views (regardless of orientation)
      // is useful for reviewing a volume in multiple views.
      // Copy the field of view information. Use the new
      // prescribed x fov, aspect corrected y fov, and keep z fov
      // constant
      if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
        & vtkMRMLSliceNode::FieldOfViewFlag)
      {
        sNode->SetFieldOfView(sliceNode->GetFieldOfView()[0],
          sliceNode->GetFieldOfView()[0]
          * sNode->GetFieldOfView()[1]
          / sNode->GetFieldOfView()[0],
          sNode->GetFieldOfView()[2]);
      }

      // need to manage prescribed spacing here as well?

      // Setting the orientation of the slice plane does not
      // require that the orientations initially match.
      if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
        & vtkMRMLSliceNode::OrientationFlag)
        {
        // We could copy the orientation strings, but we really
        // want the slice to ras to match, so copy that
        sNode->GetSliceToRAS()->DeepCopy( sliceNode->GetSliceToRAS() );

        // Forces the internal matrices to be updated which results
        // in this being modified so a Render can occur
        sNode->UpdateMatrices();
        }

      // Resetting the field of view does not require the
      // orientations to match
      if ((sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
          & vtkMRMLSliceNode::ResetFieldOfViewFlag)
          && this->GetMRMLApplicationLogic()->GetSliceLogics())
        {
        // need the logic for this slice (sNode)
        vtkMRMLSliceLogic* logic;
        vtkCollectionSimpleIterator it;
        vtkCollection* logics = this->GetMRMLApplicationLogic()->GetSliceLogics();
        for (logics->InitTraversal(it);
              (logic=vtkMRMLSliceLogic::SafeDownCast(logics->GetNextItemAsObject(it)));)
          {
          if (logic->GetSliceNode() == sNode)
            {
            logic->FitSliceToAll();
            sNode->UpdateMatrices();
            break;
            }
          }
        }

      // Resetting the orientations to default does not require the
      // orientations to match
      if ((sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
          & vtkMRMLSliceNode::ResetOrientationFlag)
          && this->GetMRMLApplicationLogic()->GetSliceLogics())
        {
        sNode->SetOrientationToDefault();
        }

      // Broadcasting the rotation from a ReformatWidget
      if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
        & vtkMRMLSliceNode::MultiplanarReformatFlag)
        {
        this->BroadcastLastRotation(sliceNode,sNode);
        }

      // Setting the label outline mode
      if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
        & vtkMRMLSliceNode::LabelOutlineFlag)
        {
        sNode->SetUseLabelOutline( sliceNode->GetUseLabelOutline() );
        }

      // Broadcasting the visibility of slice in 3D
      if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
        & vtkMRMLSliceNode::SliceVisibleFlag)
        {
        std::string layoutName(sliceNode->GetLayoutName() ? sliceNode->GetLayoutName() : "");
        std::string lname(sNode->GetLayoutName() ? sNode->GetLayoutName() : "");
        if (layoutName.find("Compare") == 0)
          {
          // Compare view, only broadcast to compare views
          if (lname.find("Compare") == 0)
            {
            // Compare view, broadcast
            sNode->SetSliceVisible(sliceNode->GetSliceVisible());
            }
          }
        else
          {
          // Not a compare view, only broadcast to non compare views
          if (lname.find("Compare") != 0)
            {
            // not a Compare view, broadcast
            sNode->SetSliceVisible(sliceNode->GetSliceVisible());
            }
          }
        }

        // Setting the slice spacing
        if (sliceNode->GetInteractionFlags() & sliceNode->GetInteractionFlagsModifier()
          & vtkMRMLSliceNode::SliceSpacingFlag)
          {
          sNode->SetSliceSpacingMode( sliceNode->GetSliceSpacingMode() );
          sNode->SetPrescribedSliceSpacing( sliceNode->GetPrescribedSliceSpacing() );
          }
      //
      // End of the block for broadcasting parameters and commands
      // that do not require the orientation to match
      //
      }

    // Update SliceNodeInteractionStatus after MultiplanarReformat interaction
    this->UpdateSliceNodeInteractionStatus(sliceNode);
    this->BroadcastingEventsOff();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::BroadcastSliceCompositeNodeEvent(vtkMRMLSliceCompositeNode *sliceCompositeNode)
{
  // only broadcast a slice composite node event if we are not already actively
  // broadcasting events and we actively interacting with the node
  // std::cout << "BroadcastingEvents: " << this->GetBroadcastingEvents()
  //           << ", Interacting: " << sliceCompositeNode->GetInteracting()
  //           << std::endl;
  if (!sliceCompositeNode)
    {
    return;
    }
  if (!this->GetBroadcastingEvents() && sliceCompositeNode->GetInteracting())
    {
    this->BroadcastingEventsOn();

    int requiredViewGroup = -1;
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceLogic::GetSliceNode(sliceCompositeNode);
    if (sliceNode)
      {
      requiredViewGroup = sliceNode->GetViewGroup();
      }
    vtkMRMLSliceCompositeNode* cNode;
    vtkCollectionSimpleIterator it;
    vtkSmartPointer<vtkCollection> nodes;
    nodes.TakeReference(this->GetMRMLScene()->GetNodesByClass("vtkMRMLSliceCompositeNode"));

    for (nodes->InitTraversal(it);
        (cNode=vtkMRMLSliceCompositeNode::SafeDownCast(nodes->GetNextItemAsObject(it)));)
      {
      if (cNode == sliceCompositeNode || !cNode)
        {
        continue;
        }
      if (requiredViewGroup >= 0)
        {
        vtkMRMLSliceNode* sNode = vtkMRMLSliceLogic::GetSliceNode(cNode);
        if (sNode && sNode->GetViewGroup() != requiredViewGroup)
          {
          continue;
          }
        }
      // Foreground selection
      if (sliceCompositeNode->GetInteractionFlags() & sliceCompositeNode->GetInteractionFlagsModifier()
          & vtkMRMLSliceCompositeNode::ForegroundVolumeFlag)
        {
        //std::cerr << "Broadcasting Foreground Volume " << sliceCompositeNode->GetForegroundVolumeID() << std::endl;
        cNode->SetForegroundVolumeID(sliceCompositeNode->GetForegroundVolumeID());
        }

      // Background selection
      if (sliceCompositeNode->GetInteractionFlags() & sliceCompositeNode->GetInteractionFlagsModifier()
          & vtkMRMLSliceCompositeNode::BackgroundVolumeFlag)
        {
        cNode->SetBackgroundVolumeID(sliceCompositeNode->GetBackgroundVolumeID());
        }

      // Labelmap selection
      if (sliceCompositeNode->GetInteractionFlags() & sliceCompositeNode->GetInteractionFlagsModifier()
          & vtkMRMLSliceCompositeNode::LabelVolumeFlag)
        {
        cNode->SetLabelVolumeID(sliceCompositeNode->GetLabelVolumeID());
        }

      // Foreground opacity
      if (sliceCompositeNode->GetInteractionFlags() & sliceCompositeNode->GetInteractionFlagsModifier()
          & vtkMRMLSliceCompositeNode::ForegroundOpacityFlag)
        {
        cNode->SetForegroundOpacity(sliceCompositeNode->GetForegroundOpacity());
        }

      // Labelmap opacity
      if (sliceCompositeNode->GetInteractionFlags() & sliceCompositeNode->GetInteractionFlagsModifier()
          & vtkMRMLSliceCompositeNode::LabelOpacityFlag)
        {
        cNode->SetLabelOpacity(sliceCompositeNode->GetLabelOpacity());
        }

      }

    this->BroadcastingEventsOff();
    }
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* vtkMRMLSliceLinkLogic::GetCompositeNode(vtkMRMLSliceNode* sliceNode)
{
  vtkMRMLSliceCompositeNode* compositeNode = nullptr;

  vtkCollectionSimpleIterator it;
  vtkSmartPointer<vtkCollection> nodes;
  nodes.TakeReference(this->GetMRMLScene()->GetNodesByClass("vtkMRMLSliceCompositeNode"));

  for (nodes->InitTraversal(it);
      (compositeNode=vtkMRMLSliceCompositeNode::SafeDownCast(nodes->GetNextItemAsObject(it)));)
    {
    if (compositeNode->GetLayoutName()
        && !strcmp(compositeNode->GetLayoutName(), sliceNode->GetName()))
      {
      break;
      }

    compositeNode = nullptr;
    }

  return compositeNode;
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::BroadcastLastRotation(vtkMRMLSliceNode* sliceNode,
                                                  vtkMRMLSliceNode* sNode)
{
  SliceNodeStatusMap::iterator it = this->SliceNodeInteractionStatus.find(sliceNode->GetID());
  if (it == this->SliceNodeInteractionStatus.end())
    {
    return;
    }

  // Calculate the rotation applied to the sliceNode
  double cross[3], dot, rotation;
  vtkNew<vtkTransform> transform;
  vtkMatrix4x4* sNodeToRAS = sNode->GetSliceToRAS();
  double sliceNormal[3] = {sliceNode->GetSliceToRAS()->GetElement(0,2),
                           sliceNode->GetSliceToRAS()->GetElement(1,2),
                           sliceNode->GetSliceToRAS()->GetElement(2,2)};

  // Rotate the sliceNode to match the planeWidget normal
  vtkMath::Cross(it->second.LastNormal,sliceNormal, cross);
  dot = vtkMath::Dot(it->second.LastNormal, sliceNormal);
  // Clamp the dot product
  dot = (dot < -1.0) ? -1.0 : (dot > 1.0 ? 1.0 : dot);
  rotation = vtkMath::DegreesFromRadians(acos(dot));

  // Apply the rotation
  transform->PostMultiply();
  transform->SetMatrix(sNodeToRAS);
  transform->RotateWXYZ(rotation,cross);
  transform->GetMatrix(sNodeToRAS); // Update the changes

  sNode->UpdateMatrices();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceLinkLogic::UpdateSliceNodeInteractionStatus(vtkMRMLSliceNode* sliceNode)
{
  SliceNodeStatusMap::iterator it = this->SliceNodeInteractionStatus.find(sliceNode->GetID());

  if (it != SliceNodeInteractionStatus.end())
    {
    it->second.LastNormal[0] = sliceNode->GetSliceToRAS()->GetElement(0,2);
    it->second.LastNormal[1] = sliceNode->GetSliceToRAS()->GetElement(1,2);
    it->second.LastNormal[2] = sliceNode->GetSliceToRAS()->GetElement(2,2);
    }
}

