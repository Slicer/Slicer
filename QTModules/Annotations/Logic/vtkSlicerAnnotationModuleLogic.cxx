// Annotation includes
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// Annotation/MRML includes
#include "vtkMRMLAnnotationRulerStorageNode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationRulerStorageNode.h"
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLAnnotationAngleStorageNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationStickyNode.h"
#include "vtkMRMLAnnotationTextNode.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLAnnotationBidimensionalNode.h"
#include "vtkMRMLAnnotationSplineNode.h"
#include "vtkMRMLAnnotationSnapshotNode.h"
#include "vtkMRMLAnnotationSnapshotStorageNode.h"

#include "qSlicerCoreApplication.h"

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPNGWriter.h>

// STD includes
#include <string>
#include <iostream>
#include <sstream>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//-----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerAnnotationModuleLogic, "$Revision: 2.0$")
vtkStandardNewMacro(vtkSlicerAnnotationModuleLogic)

//-----------------------------------------------------------------------------
// vtkSlicerAnnotationModuleLogic methods
//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic::vtkSlicerAnnotationModuleLogic()
{
  this->m_Widget = 0;
  this->m_LastAddedAnnotationNode = 0;
  this->ActiveHierarchyNodeID = NULL;

  this->m_MeasurementFormat = new char[8];
  sprintf(this->m_MeasurementFormat, "%s", "%.1f");

  this->m_CoordinateFormat = new char[8];
  sprintf(this->m_CoordinateFormat, "%s", "%.0f");

}

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic::~vtkSlicerAnnotationModuleLogic()
{

  if (this->m_Widget)
    {
    this->m_Widget = 0;
    }

  if (this->m_LastAddedAnnotationNode)
    {
    this->m_LastAddedAnnotationNode->Delete();
    this->m_LastAddedAnnotationNode = 0;
    }

  if (this->ActiveHierarchyNodeID)
    {
    delete [] this->ActiveHierarchyNodeID;
    this->ActiveHierarchyNodeID = NULL;
    }

  if (this->m_MeasurementFormat)
    {
    delete[] this->m_MeasurementFormat;
    this->m_MeasurementFormat = NULL;
    }

  if (this->m_CoordinateFormat)
    {
    delete[] this->m_CoordinateFormat;
    this->m_CoordinateFormat = NULL;
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "MeasurementFormat = " << (this->m_MeasurementFormat ? this->m_MeasurementFormat : "NULL") << std::endl;
  os << indent << "CoordinateFormat = " << (this->m_CoordinateFormat ? this->m_CoordinateFormat : "NULL") << std::endl;
  os << indent << "ActiveHierarchyNodeID = " << (this->ActiveHierarchyNodeID ? this->ActiveHierarchyNodeID : "NULL") << std::endl;
if (this->m_LastAddedAnnotationNode)
  {
  os << indent << "LastAddedAnnotationNode: " << (this->m_LastAddedAnnotationNode->GetID() ? this->m_LastAddedAnnotationNode->GetID() : "NULL ID") << std::endl;
  }
}

//-----------------------------------------------------------------------------
/// Set and observe the GUI widget
//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAndObserveWidget(qSlicerAnnotationModuleWidget* widget)
{
  if (!widget)
    {
    return;
    }

  this->m_Widget = widget;
}

//-----------------------------------------------------------------------------
//
//
// MRML event handling
//
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  vtkDebugMacro("ProcessMRMLEvents: Event "<< event);

  vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*> (callData);

  if (event == vtkMRMLScene::SceneClosedEvent)
    {
    this->OnMRMLSceneClosedEvent();
    return;
    }

  vtkMRMLInteractionNode *interactionNode =
      vtkMRMLInteractionNode::SafeDownCast(caller);
  if (interactionNode)
    {
    switch (event)
      {
      case vtkMRMLInteractionNode::InteractionModeChangedEvent:
        this->OnInteractionModeChangedEvent(interactionNode);
        break;
      case vtkMRMLInteractionNode::InteractionModePersistenceChangedEvent:
        this->OnInteractionModePersistenceChangedEvent(interactionNode);
        break;
      default:
        vtkWarningMacro("ProcessMRMLEvents: unhandled event on interaction node: " << event)
        ;
      }
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);
  if (annotationNode)
    {
    switch (event)
      {
      case vtkMRMLScene::NodeAddedEvent:
        this->OnMRMLSceneNodeAddedEvent(annotationNode);
        break;
      case vtkCommand::ModifiedEvent:
        this->OnMRMLAnnotationNodeModifiedEvent(annotationNode);
        break;
      case vtkMRMLAnnotationControlPointsNode::ControlPointModifiedEvent:
        this->OnMRMLAnnotationNodeModifiedEvent(annotationNode);
        break;
      }
    return;
    }

}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLSceneNodeAddedEvent");
  vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);
  if (!annotationNode)
    {
    return;
    }

  bool retval = this->AddHierarchyNodeForAnnotation(annotationNode);
  if (!retval)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: No hierarchyNode added.")
    return;
    }

  // we pass the hierarchy node along - it includes the pointer to the actual annotationNode
  this->AddNodeCompleted(annotationNode);
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::OnMRMLAnnotationNodeModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationNodeModifiedEvent " << node->GetID());

  vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);
  if (!annotationNode)
    {
    return;
    }

  if (this->m_Widget)
    {
    // refresh the hierarchy tree
    this->m_Widget->refreshTree();
    }

}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::OnMRMLSceneClosedEvent()
{
  if (this->m_LastAddedAnnotationNode)
    {
    this->m_LastAddedAnnotationNode = 0;
    }

  if (this->GetActiveHierarchyNodeID())
    {
    this->SetActiveHierarchyNodeID(NULL);
    }

  if (this->m_Widget)
    {
    //    this->m_Widget->refreshTree();
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::OnInteractionModeChangedEvent(vtkMRMLInteractionNode *interactionNode)
{
  vtkDebugMacro("OnInteractionModeChangedEvent");
  if (!interactionNode || this->m_Widget == NULL)
    {
    return;
    }
  this->m_Widget->updateWidgetFromInteractionMode(interactionNode);
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::OnInteractionModePersistenceChangedEvent(vtkMRMLInteractionNode *interactionNode)
{
  vtkDebugMacro("OnInteractionModePersistenceChangedEvent");
  if (!interactionNode || this->m_Widget == NULL)
    {
    return;
    }
  this->m_Widget->updateWidgetFromInteractionMode(interactionNode);
}

//---------------------------------------------------------------------------
//
//
// Placement of Annotations
//
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Start the place mouse mode
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  // a good time to add the observed events!
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events);
  events->Delete();

  if (this->GetMRMLScene() == NULL)
    {
    vtkWarningMacro("SetMRMLSceneInternal: no scene to listen to!");
    return;
    }

  // also observe the interaction node for changes
  vtkMRMLInteractionNode *interactionNode =
      vtkMRMLInteractionNode::SafeDownCast(
          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (interactionNode)
    {
    vtkIntArray *interactionEvents = vtkIntArray::New();
    interactionEvents->InsertNextValue(
        vtkMRMLInteractionNode::InteractionModeChangedEvent);
    interactionEvents->InsertNextValue(
        vtkMRMLInteractionNode::InteractionModePersistenceChangedEvent);
    vtkObserveMRMLNodeEventsMacro(interactionNode, interactionEvents);
    interactionEvents->Delete();
    }
  else
    {
    vtkWarningMacro("SetMRMLSceneInternal: No interaction node!");
    }
  vtkDebugMacro("SetMRMLSceneInternal: listeners added");

  // add known annotation types to the selection node
  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  if (selectionNode)
    {
    std::cout << "vtkSlicerAnnotationModuleLogic::InitializeEventListeners(): adding new annotation ids to selection node list" << std::endl;
    selectionNode->AddNewAnnotationIDToList("vtkMRMLAnnotationFiducialNode", ":/Icons/AnnotationPoint.png");
    selectionNode->AddNewAnnotationIDToList("vtkMRMLAnnotationTextNode", ":/Icons/AnnotationText.png");
    selectionNode->AddNewAnnotationIDToList("vtkMRMLAnnotationRulerNode", ":/Icons/AnnotationDistance.png");
    selectionNode->AddNewAnnotationIDToList("vtkMRMLAnnotationBidimensionalNode", ":/Icons/AnnotationBidimensional.png");
    selectionNode->AddNewAnnotationIDToList("vtkMRMLAnnotationROINode", ":/Icons/AnnotationROI.png");
//    selectionNode->AddNewAnnotationIDToList("vtkMRMLAnnotationAngleNode", ":/Icons/AnnotationAngle.png");
//    selectionNode->AddNewAnnotationIDToList("vtkMRMLAnnotationStickyNode", "");
//    selectionNode->AddNewAnnotationIDToList("vtkMRMLAnnotationSplineNode", ":/Icons/AnnotationSpline.png");
    }
}

//-----------------------------------------------------------------------------
// Add Annotation Node
//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::AddAnnotationNode(const char * nodeDescriptor, bool persistent)
{

  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  if (!selectionNode)
    {
    vtkErrorMacro("AddAnnotationNode: No selection node in the scene.");
    return;
    }

  selectionNode->SetActiveAnnotationID(nodeDescriptor);

  this->StartPlaceMode(persistent);

}

//---------------------------------------------------------------------------
// Start the place mouse mode
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::StartPlaceMode(bool persistent)
{

  vtkMRMLInteractionNode *interactionNode =
      vtkMRMLInteractionNode::SafeDownCast(
          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (!interactionNode)
    {
    vtkErrorMacro ( "StartPlaceMode: No interaction node in the scene." );
    return;
    }

  //  this->InitializeEventListeners();

  interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);

  if (persistent)
    {
    interactionNode->SetPlaceModePersistence(1);
    }
  else
    {
    interactionNode->SetPlaceModePersistence(0);
    }

  if (interactionNode->GetCurrentInteractionMode()
      != vtkMRMLInteractionNode::Place)
    {

    vtkErrorMacro("StartPlaceMode: Could not set place mode!");
    return;

    }
}

//---------------------------------------------------------------------------
// called after a new annotation node was added, now add it to the table in the GUI
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::AddNodeCompleted(vtkMRMLAnnotationNode* annotationNode)
{

  if (!annotationNode)
    {
    return;
    }

  if (!this->m_Widget)
    {
    return;
    }

  if (this->m_Widget)
    {
    // refresh the hierarchy tree
    this->m_Widget->refreshTree();
    }

  this->m_LastAddedAnnotationNode = annotationNode;

}

//---------------------------------------------------------------------------
// Exit the place mode
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::StopPlaceMode(bool persistent)
{

  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  if (!selectionNode)
    {
    vtkErrorMacro("StopPlaceMode: No selection node in the scene.");
    return;
    }

  vtkMRMLInteractionNode *interactionNode =
      vtkMRMLInteractionNode::SafeDownCast(
          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (interactionNode == NULL)
    {
    vtkErrorMacro ( "StopPlaceMode: No interaction node in the scene." );
    return;
    }

  if (persistent)
    {
    // if persistent placement was activated in the Annotation GUI, we do not want to reset it
    interactionNode->SetPlaceModePersistence(1);
    }
  else
    {
    // if persistent placement was not activated in the Annotation GUI, then we want to reset it
    interactionNode->SetPlaceModePersistence(0);
    }

  interactionNode->SwitchToViewTransformMode();
  if (interactionNode->GetCurrentInteractionMode()
      != vtkMRMLInteractionNode::ViewTransform)
    {

    vtkErrorMacro("StopPlaceMode: Could not set transform mode!");

    }
  // reset the active annotation id after switching to view transform mode,
  // since this is checked in the displayable managers
  selectionNode->SetActiveAnnotationID("");
}

//---------------------------------------------------------------------------
// Cancel the current placement or remove the last placed node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::CancelCurrentOrRemoveLastAddedAnnotationNode()
{

  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(0,"vtkMRMLInteractionNode"));

  if (!interactionNode)
    {
    vtkErrorMacro("CancelCurrentOrRemoveLastAddedAnnotationNode: No interaction node")
    return;
    }

  interactionNode->InvokeEvent(vtkMRMLInteractionNode::CancelPlacementEvent);

}

//---------------------------------------------------------------------------
/// Remove an AnnotationNode and also its 1-1 IS-A hierarchyNode, if found.
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::RemoveAnnotationNode(vtkMRMLAnnotationNode* annotationNode)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("RemoveAnnotationNode: No MRML Scene found.")
    return;
    }

  // remove the 1-1 IS-A hierarchy node first
  vtkMRMLDisplayableHierarchyNode* displayableHierarchyNode =
      vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
          annotationNode->GetScene(), annotationNode->GetID());
  if (displayableHierarchyNode)
    {
    // there is a parent
    this->GetMRMLScene()->RemoveNode(displayableHierarchyNode);

    }

  this->GetMRMLScene()->RemoveNode(annotationNode);

}

//---------------------------------------------------------------------------
//
//
// Annotation Properties as an interface to MRML
//
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Register the MRML node classes to the attached scene.
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::RegisterNodes()
{
  if (!this->GetMRMLScene())
    {
    vtkWarningMacro("RegisterNodes: no scene");
    return;
    }

  //
  // The core nodes
  //

  // base nodes
  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationNode);
  annotationNode->Delete();

  vtkMRMLAnnotationDisplayNode* annotationDisplayNode =
      vtkMRMLAnnotationDisplayNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationDisplayNode);
  annotationDisplayNode->Delete();

  vtkMRMLAnnotationStorageNode* annotationStorageNode =
      vtkMRMLAnnotationStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationStorageNode);
  annotationStorageNode->Delete();

  // Control Points
  vtkMRMLAnnotationControlPointsNode* annotationControlPointsNode =
      vtkMRMLAnnotationControlPointsNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationControlPointsNode);
  annotationControlPointsNode->Delete();

  vtkMRMLAnnotationControlPointsStorageNode
      * annotationControlPointsStorageNode =
          vtkMRMLAnnotationControlPointsStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationControlPointsStorageNode);
  annotationControlPointsStorageNode->Delete();

  vtkMRMLAnnotationPointDisplayNode* annotationControlPointsDisplayNode =
      vtkMRMLAnnotationPointDisplayNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationControlPointsDisplayNode);
  annotationControlPointsDisplayNode->Delete();

  // Lines
  vtkMRMLAnnotationLinesNode* annotationLinesNode =
      vtkMRMLAnnotationLinesNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationLinesNode);
  annotationLinesNode->Delete();

  vtkMRMLAnnotationLinesStorageNode* annotationLinesStorageNode =
      vtkMRMLAnnotationLinesStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationLinesStorageNode);
  annotationLinesStorageNode->Delete();

  vtkMRMLAnnotationLineDisplayNode* annotationLinesDisplayNode =
      vtkMRMLAnnotationLineDisplayNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationLinesDisplayNode);
  annotationLinesDisplayNode->Delete();

  // Text
  vtkMRMLAnnotationTextDisplayNode* annotationTextDisplayNode =
      vtkMRMLAnnotationTextDisplayNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationTextDisplayNode);
  annotationTextDisplayNode->Delete();

  //
  // Now the actual Annotation tool nodes
  //

  // Snapshot annotation
  vtkMRMLAnnotationSnapshotNode* annotationSnapshotNode =
      vtkMRMLAnnotationSnapshotNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationSnapshotNode);
  annotationSnapshotNode->Delete();

  vtkMRMLAnnotationSnapshotStorageNode* annotationSnapshotStorageNode =
      vtkMRMLAnnotationSnapshotStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationSnapshotStorageNode);
  annotationSnapshotStorageNode->Delete();

  // Text annotation
  vtkMRMLAnnotationTextNode* annotationTextNode =
      vtkMRMLAnnotationTextNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationTextNode);
  annotationTextNode->Delete();

  // Ruler annotation
  vtkMRMLAnnotationRulerNode* annotationRulerNode =
      vtkMRMLAnnotationRulerNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationRulerNode);
  annotationRulerNode->Delete();

  vtkMRMLAnnotationRulerStorageNode* annotationRulerStorageNode =
      vtkMRMLAnnotationRulerStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationRulerStorageNode);
  annotationRulerStorageNode->Delete();

  // ROI annotation
  vtkMRMLAnnotationROINode* annotationROINode = vtkMRMLAnnotationROINode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationROINode);
  annotationROINode->Delete();

  // ROI annotation backwards compatibility
  vtkMRMLAnnotationROINode* oldAnnotationROINode =
      vtkMRMLAnnotationROINode::New();
  this->GetMRMLScene()->RegisterNodeClass(oldAnnotationROINode, "MRMLROINode");
  oldAnnotationROINode->Delete();

  // Bidimensional annotation
  vtkMRMLAnnotationBidimensionalNode* annotationBidimensionalNode =
      vtkMRMLAnnotationBidimensionalNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationBidimensionalNode);
  annotationBidimensionalNode->Delete();

  // Fiducial annotation
  vtkMRMLAnnotationFiducialNode* annotationFiducialNode =
      vtkMRMLAnnotationFiducialNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationFiducialNode);
  annotationFiducialNode->Delete();

  //
  // Annotation hierarchies
  //
  vtkMRMLAnnotationHierarchyNode* annotationHierarchyNode =
      vtkMRMLAnnotationHierarchyNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationHierarchyNode);
  annotationHierarchyNode->Delete();
}

//---------------------------------------------------------------------------
// Check if the id points to an annotation node
//---------------------------------------------------------------------------
bool vtkSlicerAnnotationModuleLogic::IsAnnotationNode(const char* id)
{
  if (!id)
    {
    return false;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(id));

  if (annotationNode)
    {
    return true;
    }

  return false;

}

//---------------------------------------------------------------------------
// Return the name of an annotation MRML Node
//---------------------------------------------------------------------------
const char * vtkSlicerAnnotationModuleLogic::GetAnnotationName(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationName: no id");
    return 0;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationName: Could not get the MRML node with id " << id)
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationName: Could not get the annotationMRML node.")
    return 0;
    }

  return annotationNode->GetName();
}

//---------------------------------------------------------------------------
// Return the text of an annotation MRML Node
//---------------------------------------------------------------------------
vtkStdString vtkSlicerAnnotationModuleLogic::GetAnnotationText(const char* id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationText: no id supplid");
    return 0;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationText: Could not get the MRML node with id " << id)
    return 0;
    }

  // special case for annotation snapShots
  vtkMRMLAnnotationSnapshotNode* snapshotNode =
      vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);
  if (snapshotNode)
    {
    return snapshotNode->GetName();
    }
  // end of special case for annotation snapShots

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationText: Could not get the annotationMRML node.")
    return 0;
    }

  return annotationNode->GetText(0);

}

//---------------------------------------------------------------------------
// Set the text of an annotation MRML Node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationText(const char* id, const char * newtext)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationText: no id specified");
    return;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationText: Could not get the MRML node with id " << id)
    return;
    }

  // special case for snapShots
  vtkMRMLAnnotationSnapshotNode* snapshotNode =
      vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);
  if (snapshotNode)
    {

    snapshotNode->SetSnapshotDescription(vtkStdString(newtext));

    // now bail out
    return;
    }
  // end of special case for snapShots

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationText: Could not get the annotationMRML node.")
    return;
    }

  if (!newtext)
    {
    vtkErrorMacro("SetAnnotationText: No text supplied, using an empty string.")
    annotationNode->SetText(0, "", 1, 1);
    return;
    }

  annotationNode->SetText(0, newtext, 1, 1);
  if (annotationNode->IsA("vtkMRMLAnnotationFiducialNode"))
    {
    // also set the name
    annotationNode->SetName(newtext);
    }

}

//---------------------------------------------------------------------------
// Get the textScale of a MRML Annotation node
//---------------------------------------------------------------------------
double vtkSlicerAnnotationModuleLogic::GetAnnotationTextScale(const char* id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationTextScale: no id specified");
    return 0;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the MRML node with id " << id)
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the annotation MRML node.")
    return 0;
    }

  return annotationNode->GetTextScale();

}

//---------------------------------------------------------------------------
// Set the textScale of a MRML Annotation node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationTextScale(const char* id, double textScale)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationTextScale: no id given");
    return;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the MRML node with id " << id)
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the annotation MRML node.")
    return;
    }

  annotationNode->SetTextScale(textScale);

}

//---------------------------------------------------------------------------
// Get the selected text color of a MRML Annotation node
//---------------------------------------------------------------------------
double * vtkSlicerAnnotationModuleLogic::GetAnnotationTextSelectedColor(const char* id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationTextSelectedColor: no id specified");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextSelectedColor: Could not get the MRML node with id " << id)
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextSelectedColor: Could not get the annotation MRML node.")
    return 0;
    }

  if (!annotationNode->GetAnnotationTextDisplayNode())
    {
    return 0;
    }

  return annotationNode->GetAnnotationTextDisplayNode()->GetSelectedColor();

}

//---------------------------------------------------------------------------
// Set the selected text color of a MRML Annotation node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationTextSelectedColor(const char* id, double * color)
{
  if (!id)
    {
    return;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextSelectedColor: Could not get the MRML node with id " << id)
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextSelectedColor: Could not get the annotation MRML node.")
    return;
    }

  annotationNode->GetAnnotationTextDisplayNode()->SetSelectedColor(color);

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// Get the unselected text color of a MRML Annotation node
//---------------------------------------------------------------------------
double * vtkSlicerAnnotationModuleLogic::GetAnnotationTextUnselectedColor(const char* id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationTextUnselectedColor: no id given");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextUnselectedColor: Could not get the MRML node with id " << id)
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextUnselectedColor: Could not get the annotation MRML node.")
    return 0;
    }

  if (!annotationNode->GetAnnotationTextDisplayNode())
    {
    return 0;
    }

  return annotationNode->GetAnnotationTextDisplayNode()->GetColor();

}

//---------------------------------------------------------------------------
// Set the unselected text color of a MRML Annotation node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationTextUnselectedColor(const char* id, double * color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationTextUnselectedColor: no id given");
    return;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextUnselectedColor: Could not get the MRML node with id " << id)
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextUnselectedColor: Could not get the annotation MRML node with id " << id)
    return;
    }

  annotationNode->GetAnnotationTextDisplayNode()->SetColor(color);

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// Get the color of an annotation node
//---------------------------------------------------------------------------
double * vtkSlicerAnnotationModuleLogic::GetAnnotationColor(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationColor: no id given, cannot get color");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationColor: Could not get the MRML node for id " << id);
    return 0;
    }

  vtkMRMLDisplayableNode* annotationNode =
      vtkMRMLDisplayableNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationColor: Could not get the displayable MRML node for id " << id);
    return 0;
    }

  if (annotationNode->GetDisplayNode() == NULL)
    {
    vtkErrorMacro("GetAnnotationColor: Could not get the display node for node " << id);
    return 0;
    }

  return annotationNode->GetDisplayNode()->GetSelectedColor();
}

//---------------------------------------------------------------------------
/// Set the color of an annotation mrml node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationColor(const char *id, double *color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationColor: no id given, cannot set color");
    return;
    }
  if (!color)
    {
    vtkErrorMacro("SetAnnotationColor: no color given, cannot set color for node " << id);
    return;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationColor: Could not get the MRML node for id " << id);
    return;
    }

  vtkMRMLDisplayableNode* annotationNode =
      vtkMRMLDisplayableNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationColor: Could not get the displayable MRML node for id " << id);
    return;
    }

  if (annotationNode->GetDisplayNode() == NULL)
    {
    vtkErrorMacro("SetAnnotationColor: Could not get the display node for node " << id);
    return;
    }

  annotationNode->GetDisplayNode()->SetSelectedColor(color);
  // this should trigger a display modified event, but it's not being caught
  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);
}

//---------------------------------------------------------------------------
// Get the unselected color of an annotation node
//---------------------------------------------------------------------------
double * vtkSlicerAnnotationModuleLogic::GetAnnotationUnselectedColor(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationUnselectedColor: no id given, cannot get color");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationUnselectedColor: Could not get the MRML node for id " << id);
    return 0;
    }

  vtkMRMLDisplayableNode* annotationNode =
      vtkMRMLDisplayableNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationUnselectedColor: Could not get the displayable MRML node for id " << id);
    return 0;
    }

  if (annotationNode->GetDisplayNode() == NULL)
    {
    vtkErrorMacro("GetAnnotationUnselectedColor: Could not get the display node for node " << id);
    return 0;
    }

  return annotationNode->GetDisplayNode()->GetColor();
}

//---------------------------------------------------------------------------
/// Set the color of an annotation mrml node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationUnselectedColor(const char *id, double *color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationUnselectedColor: no id given, cannot set color");
    return;
    }
  if (!color)
    {
    vtkErrorMacro("SetAnnotationUnselectedColor: no color given, cannot set color for node " << id);
    return;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationUnselectedColor: Could not get the MRML node for id " << id);
    return;
    }

  vtkMRMLDisplayableNode* annotationNode =
      vtkMRMLDisplayableNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationUnselectedColor: Could not get the displayable MRML node for id " << id);
    return;
    }

  if (annotationNode->GetDisplayNode() == NULL)
    {
    vtkErrorMacro("SetAnnotationUnselectedColor: Could not get the display node for node " << id);
    return;
    }

  annotationNode->GetDisplayNode()->SetColor(color);
  // this should trigger a display modified event, but it's not being caught
  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);
}

//---------------------------------------------------------------------------
// Get the color of an annotation point node
//---------------------------------------------------------------------------
double * vtkSlicerAnnotationModuleLogic::GetAnnotationPointColor(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationPointColor: no id given, cannot get color");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationPointColor: Could not get the MRML node for id " << id);
    return 0;
    }

  vtkMRMLAnnotationControlPointsNode* annotationNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationPointColor: Could not get the displayable control points MRML node for id " << id);
    return 0;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == NULL)
    {
    vtkErrorMacro("GetAnnotationPointColor: Could not get the display node for node " << id);
    return 0;
    }

  return annotationNode->GetAnnotationPointDisplayNode()->GetSelectedColor();
}

//---------------------------------------------------------------------------
/// Set the color of an annotation point node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationPointColor(const char *id, double *color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationPointColor: no id given, cannot set color");
    return;
    }
  if (!color)
    {
    vtkErrorMacro("SetAnnotationPointColor: no color given, cannot set color for node " << id);
    return;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationPointColor: Could not get the MRML node for id " << id);
    return;
    }

  vtkMRMLAnnotationControlPointsNode* annotationNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationPointColor: Could not get the displayable control points MRML node for id " << id);
    return;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == NULL)
    {
    vtkErrorMacro("SetAnnotationPointColor: Could not get the display node for node " << id);
    return;
    }

  annotationNode->GetAnnotationPointDisplayNode()->SetSelectedColor(color);
  // this should trigger a display modified event, but it's not being caught
  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);
}

//---------------------------------------------------------------------------
// Get the unselected color of an annotation point node
//---------------------------------------------------------------------------
double * vtkSlicerAnnotationModuleLogic::GetAnnotationPointUnselectedColor(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationPointUnselectedColor: no id given, cannot get color");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationPointUnselectedColor: Could not get the MRML node for id " << id);
    return 0;
    }

  vtkMRMLAnnotationControlPointsNode* annotationNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationPointUnselectedColor: Could not get the displayable control points MRML node for id " << id);
    return 0;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == NULL)
    {
    vtkErrorMacro("GetAnnotationPointUnselectedColor: Could not get the display node for node " << id);
    return 0;
    }

  return annotationNode->GetAnnotationPointDisplayNode()->GetColor();
}

//---------------------------------------------------------------------------
/// Set the color of an annotation point node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationPointUnselectedColor(const char *id, double *color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationPointUnselectedColor: no id given, cannot set color");
    return;
    }
  if (!color)
    {
    vtkErrorMacro("SetAnnotationPointUnselectedColor: no color given, cannot set color for node " << id);
    return;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationPointUnselectedColor: Could not get the MRML node for id " << id);
    return;
    }

  vtkMRMLAnnotationControlPointsNode* annotationNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationPointUnselectedColor: Could not get the displayable control points MRML node for id " << id);
    return;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == NULL)
    {
    vtkErrorMacro("SetAnnotationPointUnselectedColor: Could not get the display node for node " << id);
    return;
    }

  annotationNode->GetAnnotationPointDisplayNode()->SetColor(color);
}

//---------------------------------------------------------------------------
// Get the glyph type of the annotation point node as a string
//---------------------------------------------------------------------------
const char * vtkSlicerAnnotationModuleLogic::GetAnnotationPointGlyphTypeAsString(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: no id given, cannot get glyph type");
    return NULL;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the MRML node for id " << id);
    return NULL;
    }

  vtkMRMLAnnotationControlPointsNode* annotationNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the displayable control points MRML node for id " << id);
    return NULL;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == NULL)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the display node for node " << id);
    return NULL;
    }

  return annotationNode->GetAnnotationPointDisplayNode()->GetGlyphTypeAsString();
}

//---------------------------------------------------------------------------
// Get the glyph type of the annotation point node 
//---------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetAnnotationPointGlyphType(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationPointGlyphType: no id given, cannot get glyph type");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the MRML node for id " << id);
    return NULL;
    }

  vtkMRMLAnnotationControlPointsNode* annotationNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the displayable control points MRML node for id " << id);
    return NULL;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == NULL)
    {
    vtkErrorMacro("GetAnnotationPointGlyphType: Could not get the display node for node " << id);
    return NULL;
    }

  return annotationNode->GetAnnotationPointDisplayNode()->GetGlyphType();
}

//---------------------------------------------------------------------------
// Set the glyph type of the annotation point node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationPointGlyphTypeFromString(const char *id, const char *glyphType)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationPointGlyphTypeFromString: no id given, cannot set glyph type");
    return;
    }
  if (!glyphType)
    {
    vtkErrorMacro("SetAnnotationPointGlyphTypeFromString: no glyph type given, cannot set glyph type for node " << id);
    return;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationPointGlyphTypeFromString: Could not get the MRML node for id " << id);
    return;
    }

  vtkMRMLAnnotationControlPointsNode* annotationNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationPointGlyphTypeFromString: Could not get the displayable control points MRML node for id " << id);
    return;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == NULL)
    {
    vtkErrorMacro("SetAnnotationPointGlyphTypeFromString: Could not get the display node for node " << id);
    return;
    }

  annotationNode->GetAnnotationPointDisplayNode()->SetGlyphTypeFromString(
      glyphType);
}

//---------------------------------------------------------------------------
// Set the glyph type of the annotation point node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationPointGlyphType(const char *id, int glyphType)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationPointGlyphType: no id given, cannot set glyph type to " << glyphType);
    return;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationPointGlyphType: Could not get the MRML node for id " << id);
    return;
    }

  vtkMRMLAnnotationControlPointsNode* annotationNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationPointGlyphType: Could not get the displayable control points MRML node for id " << id);
    return;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == NULL)
    {
    vtkErrorMacro("SetAnnotationPointGlyphType: Could not get the display node for node " << id);
    return;
    }

  annotationNode->GetAnnotationPointDisplayNode()->SetGlyphType(glyphType);
}

//---------------------------------------------------------------------------
// Get the color of an annotation line node
//---------------------------------------------------------------------------
double * vtkSlicerAnnotationModuleLogic::GetAnnotationLineColor(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationLineColor: no id given, cannot get color");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationLineColor: Could not get the MRML node for id " << id);
    return 0;
    }

  vtkMRMLAnnotationLinesNode* annotationNode =
      vtkMRMLAnnotationLinesNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationLineColor: Could not get the displayable control points MRML node for id " << id);
    return 0;
    }

  if (annotationNode->GetAnnotationLineDisplayNode() == NULL)
    {
    vtkErrorMacro("GetAnnotationLineColor: Could not get the display node for node " << id);
    return 0;
    }

  return annotationNode->GetAnnotationLineDisplayNode()->GetSelectedColor();
}

//---------------------------------------------------------------------------
/// Set the color of an annotation point node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationLineColor(const char *id, double *color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationLineColor: no id given, cannot set color");
    return;
    }
  if (!color)
    {
    vtkErrorMacro("SetAnnotationLineColor: no color given, cannot set color for node " << id);
    return;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationLineColor: Could not get the MRML node for id " << id);
    return;
    }

  vtkMRMLAnnotationLinesNode* annotationNode =
      vtkMRMLAnnotationLinesNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationLineColor: Could not get the displayable control points MRML node for id " << id);
    return;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == NULL)
    {
    vtkErrorMacro("SetAnnotationLineColor: Could not get the display node for node " << id);
    return;
    }

  annotationNode->GetAnnotationLineDisplayNode()->SetSelectedColor(color);
  // this should trigger a display modified event, but it's not being caught
  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);
}

//---------------------------------------------------------------------------
// Get the unselected color of an annotation point node
//---------------------------------------------------------------------------
double * vtkSlicerAnnotationModuleLogic::GetAnnotationLineUnselectedColor(const char *id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationLineUnselectedColor: no id given, cannot get color");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationLineUnselectedColor: Could not get the MRML node for id " << id);
    return 0;
    }

  vtkMRMLAnnotationLinesNode* annotationNode =
      vtkMRMLAnnotationLinesNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationLineUnselectedColor: Could not get the displayable control points MRML node for id " << id);
    return 0;
    }

  if (annotationNode->GetAnnotationLineDisplayNode() == NULL)
    {
    vtkErrorMacro("GetAnnotationLineUnselectedColor: Could not get the display node for node " << id);
    return 0;
    }

  return annotationNode->GetAnnotationLineDisplayNode()->GetColor();
}

//---------------------------------------------------------------------------
/// Set the color of an annotation point node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationLineUnselectedColor(const char *id, double *color)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationLineUnselectedColor: no id given, cannot set color");
    return;
    }
  if (!color)
    {
    vtkErrorMacro("SetAnnotationLineUnselectedColor: no color given, cannot set color for node " << id);
    return;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationLineUnselectedColor: Could not get the MRML node for id " << id);
    return;
    }

  vtkMRMLAnnotationLinesNode* annotationNode =
      vtkMRMLAnnotationLinesNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationLineUnselectedColor: Could not get the displayable control points MRML node for id " << id);
    return;
    }

  if (annotationNode->GetAnnotationLineDisplayNode() == NULL)
    {
    vtkErrorMacro("SetAnnotationLineUnselectedColor: Could not get the display node for node " << id);
    return;
    }

  annotationNode->GetAnnotationLineDisplayNode()->SetColor(color);
}

//---------------------------------------------------------------------------
// Get the measurement value of a MRML Annotation node
//---------------------------------------------------------------------------
const char * vtkSlicerAnnotationModuleLogic::GetAnnotationMeasurement(const char* id, bool showUnits)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationMeasurement: no id given");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationMeasurement: Could not get the MRML node with id " << id)
    return 0;
    }

  // reset stringHolder
  this->m_StringHolder = "";

  // special case for annotation snapShots
  vtkMRMLAnnotationSnapshotNode* snapshotNode =
      vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);
  if (snapshotNode)
    {
    return m_StringHolder.c_str();
    }
  // end of special case for annotation snapShots

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationMeasurement: Could not get the annotation MRML node.")
    return 0;
    }

  if (node->IsA("vtkMRMLAnnotationRulerNode"))
    {
    std::ostringstream ss;

    char string[512];
    sprintf(
        string,
        this->m_MeasurementFormat,
        vtkMRMLAnnotationRulerNode::SafeDownCast(annotationNode)->GetDistanceMeasurement());

    ss << string;

    if (showUnits)
      {
      ss << " [mm]";
      }

    this->m_StringHolder = ss.str();
    }
  else if (node->IsA("vtkMRMLAnnotationAngleNode"))
    {
    std::ostringstream ss;
    ss
        << vtkMRMLAnnotationAngleNode::SafeDownCast(annotationNode)->GetAngleMeasurement();
    if (showUnits)
      {
      ss << " [degrees]";
      }

    this->m_StringHolder = ss.str();
    }
  else if (node->IsA("vtkMRMLAnnotationFiducialNode"))
    {
    std::ostringstream ss;
    double
        * tmpPtr =
            vtkMRMLAnnotationFiducialNode::SafeDownCast(annotationNode)->GetControlPointCoordinates(
                0);
    double coordinates[3] =
    { tmpPtr[0], tmpPtr[1], tmpPtr[2] };

    char string[512];
    sprintf(string, this->m_CoordinateFormat, coordinates[0]);
    char string2[512];
    sprintf(string2, this->m_CoordinateFormat, coordinates[1]);
    char string3[512];
    sprintf(string3, this->m_CoordinateFormat, coordinates[2]);

    ss << string << ", " << string2 << ", " << string3;

    this->m_StringHolder = ss.str();
    }
  else if (node->IsA("vtkMRMLAnnotationBidimensionalNode"))
    {
    double measurement1;
    double measurement2;
    if (vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node)->GetBidimensionalMeasurement().size()
        != 2)
      {
      // measurement is not ready
      measurement1 = 0;
      measurement2 = 0;
      }
    else
      {
      // measurement is ready
      measurement1
          = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node)->GetBidimensionalMeasurement()[0];
      measurement2
          = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node)->GetBidimensionalMeasurement()[1];
      }

    std::ostringstream ss;

    // the greatest measurement should appear first..
    bool measurement1first = false;

    char string[512];
    if (measurement1 > measurement2)
      {
      sprintf(string, this->m_MeasurementFormat, measurement1);
      measurement1first = true;
      }
    else
      {
      sprintf(string, this->m_MeasurementFormat, measurement2);
      }

    ss << string;

    if (showUnits)
      {
      ss << " [mm]";
      }
    ss << " x ";

    char string2[512];
    if (measurement1first)
      {
      sprintf(string2, this->m_MeasurementFormat, measurement2);
      }
    else
      {
      sprintf(string2, this->m_MeasurementFormat, measurement1);
      }

    ss << string2;
    if (showUnits)
      {
      ss << " [mm]";
      }

    this->m_StringHolder = ss.str();
    }

  return this->m_StringHolder.c_str();

}

//---------------------------------------------------------------------------
// Return the icon of an annotation MRML Node
//---------------------------------------------------------------------------
const char * vtkSlicerAnnotationModuleLogic::GetAnnotationIcon(const char* id)
{
  if (!this->GetMRMLScene() || id == NULL)
    {
    return 0;
    }
  vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(id);
  if (!mrmlNode)
    {
    return 0;
    }
  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      mrmlNode);

  if (annotationNode)
    {
    return annotationNode->GetIcon();
    }

  vtkMRMLAnnotationHierarchyNode* hierarchyNode =
      vtkMRMLAnnotationHierarchyNode::SafeDownCast(
          this->GetMRMLScene()->GetNodeByID(id));

  if (hierarchyNode)
    {
    return hierarchyNode->GetIcon();
    }

  vtkMRMLAnnotationSnapshotNode* snapshotNode =
      vtkMRMLAnnotationSnapshotNode::SafeDownCast(
          this->GetMRMLScene()->GetNodeByID(id));

  if (snapshotNode)
    {
    return snapshotNode->GetIcon();
    }

  return 0;

}

//---------------------------------------------------------------------------
// Return the lock flag for this annotation
//---------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetAnnotationLockedUnlocked(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationLockedUnlocked: no id");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationLockedUnlocked: Could not get the MRML node with id " << id);
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationLockedUnlocked: Could not get the annotationMRML node.")
    return 0;
    }

  // lock this annotation
  return annotationNode->GetLocked();

}

//---------------------------------------------------------------------------
// Toggles the lock on this annotation
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationLockedUnlocked(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationLockedUnlocked: no id");
    return;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationLockedUnlocked: Could not get the MRML node with id " << id);
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationLockedUnlocked: Could not get the annotationMRML node.")
    return;
    }

  // lock this annotation
  annotationNode->SetLocked(!annotationNode->GetLocked());

}

//---------------------------------------------------------------------------
// Return the visibility flag for this annotation
//---------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetAnnotationVisibility(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationVisibility: no id given");
    return 0;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationVisibility: Could not get the MRML node with id " << id);
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationVisibility: Could not get the annotationMRML node.")
    return 0;
    }

  // lock this annotation
  return annotationNode->GetVisible();

}

//---------------------------------------------------------------------------
// Toggles the visibility of this annotation
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationVisibility(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationVisibility: no id given");
    return;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationVisibility: Could not get the MRML node with id " << id);
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationVisibility: Could not get the annotationMRML node.")
    return;
    }

  // show/hide this annotation
  annotationNode->SetVisible(!annotationNode->GetVisible());

}

//---------------------------------------------------------------------------
// Toggles the selected flag of this annotation
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationSelected(const char * id, bool selected)
{
  if (!id)
    {
    vtkErrorMacro("SetAnnotationSelected: no id given");
    return;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationSelected: Could not get the MRML node with id " << id);
    return;
    }

  // special case for snapshot nodes
  if (node->IsA("vtkMRMLAnnotationSnapshotNode"))
    {
    // directly bail out
    return;
    }
  // end of special case for snapshot nodes

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationSelected: Could not get the annotationMRML node.")
    return;
    }

  // select this annotation
  annotationNode->SetSelected(selected);

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// find all annotation nodes and annotation hierarchy nodes and set the selected flag on them
void vtkSlicerAnnotationModuleLogic::SetAllAnnotationsSelected(bool selected)
{
  if (this->GetMRMLScene() == NULL)
    {
    return;
    }

  int numberOfHierarchyNodes =  this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLAnnotationHierarchyNode");
  for (int i = 0; i < numberOfHierarchyNodes; i++)
    {
    vtkMRMLNode *node = this->GetMRMLScene()->GetNthNodeByClass(i, "vtkMRMLAnnotationHierarchyNode");
    node->SetSelected(selected);
    }
  
  int numberOfAnnotationNodes =  this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLAnnotationNode");
  for (int i = 0; i < numberOfAnnotationNodes; i++)
    {
    vtkMRMLNode *node = this->GetMRMLScene()->GetNthNodeByClass(i, "vtkMRMLAnnotationNode");
    // use the helper method to localise special cases on annotation nodes
    this->SetAnnotationSelected(node->GetID(), selected);
    }

  
}

//---------------------------------------------------------------------------
// Backup an AnnotationMRML node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::BackupAnnotationNode(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("BackupAnnotationNode: no id given");
    return;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("BackupAnnotationNode: Could not get the MRML node with id " << id);
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("BackupAnnotationNode: Could not get the annotationMRML node.")
    return;
    }

  annotationNode->CreateBackup();

  for (int i = 0; i < annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkMRMLAnnotationDisplayNode * displayNode =
        vtkMRMLAnnotationDisplayNode::SafeDownCast(
            annotationNode->GetNthDisplayNode(i));

    if (!displayNode)
      {
      vtkErrorMacro("BackupAnnotationNode: Could not get the annotationMRMLDisplay node number " << i << " with ID:" << annotationNode->GetID())
      }
    else
      {
      displayNode->CreateBackup();
      }

    }
}

//---------------------------------------------------------------------------
// Restore a backup version of a AnnotationMRML node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::RestoreAnnotationNode(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("RestoreAnnotationNode: no id given");
    return;
    }

  vtkDebugMacro("RestoreAnnotationNode: " << id);

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("RestoreAnnotationNode: Could not get the MRML node with id " << id);
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("RestoreAnnotationNode: Could not get the annotationMRML node.")
    return;
    }

  for (int i = 0; i < annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkMRMLAnnotationDisplayNode * displayNode =
        vtkMRMLAnnotationDisplayNode::SafeDownCast(
            annotationNode->GetNthDisplayNode(i));

    if (!displayNode)
      {
      vtkErrorMacro("BackupAnnotationNode: Could not get the annotationMRMLDisplay node:" << displayNode)
      }
    else
      {
      // now restore
      displayNode->RestoreBackup();
      }

    } // end of displayNodes

  // now restore
  annotationNode->RestoreBackup();

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// Delete all backups of a AnnotationMRML node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::DeleteBackupNodes(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("DeleteBackupNodes: no id given");
    return;
    }

  vtkDebugMacro("DeleteBackupNodes: " << id);

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("DeleteBackupNodes: Could not get the MRML node with id " << id);
    return;
    }

  vtkMRMLAnnotationNode* annotationNode =
      vtkMRMLAnnotationNode::SafeDownCast(node);

  if (!annotationNode)
    {
    vtkErrorMacro("DeleteBackupNodes: Could not get the annotationMRML node.")
    return;
    }

  for (int i = 0; i < annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkMRMLAnnotationDisplayNode * displayNode =
        vtkMRMLAnnotationDisplayNode::SafeDownCast(
            annotationNode->GetNthDisplayNode(i));

    if (!displayNode)
      {
      vtkErrorMacro("DeleteBackupNodes: Could not get the annotationMRMLDisplay node.")
      }
    else
      {
      displayNode->ClearBackup();
      }

    } // end of displayNodes

  annotationNode->ClearBackup();

}

  //---------------------------------------------------------------------------
  // Restore the view when it was last modified of an AnnotationMRML node
  //---------------------------------------------------------------------------
  void vtkSlicerAnnotationModuleLogic::RestoreAnnotationView(const char * id)
  {
    if (!id)
      {
      vtkErrorMacro("RestoreAnnotationView: no id given");
      return;
      }

    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

    if (!node)
      {
      vtkErrorMacro("RestoreAnnotationView: Could not get the MRML node with id " << id);
      return;
      }

    vtkMRMLAnnotationNode* annotationNode =
        vtkMRMLAnnotationNode::SafeDownCast(node);

    if (!annotationNode)
      {
      vtkErrorMacro("RestoreAnnotationView: Could not get the annotationMRML node.")
      return;
      }

    // do not restore anything if this is a snapshot node
    if (annotationNode->IsA("vtkMRMLAnnotationSnapshotNode"))
      {
      return;
      }

    this->GetMRMLScene()->SaveStateForUndo();

    vtkMRMLAnnotationControlPointsNode* controlpointsNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(annotationNode);

    if (!controlpointsNode)
      {
      // we don't have a controlpointsNode so we can not jump the slices
      return;
      }

    vtkMRMLSliceNode *sliceNode = vtkMRMLSliceNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(0,"vtkMRMLSliceNode"));
    if (sliceNode)
      {
      // TODO for now only consider the first control point
      double *rasCoordinates = controlpointsNode->GetControlPointCoordinates(0);
      sliceNode->JumpAllSlices(rasCoordinates[0], rasCoordinates[1], rasCoordinates[2]);
      }


    annotationNode->RestoreView();

  }

  //---------------------------------------------------------------------------
  const char * vtkSlicerAnnotationModuleLogic::MoveAnnotationUp(const char* id)
  {

    // reset stringHolder
    this->m_StringHolder = "";

    if (!id)
      {
      return this->m_StringHolder.c_str();
      }

    this->m_StringHolder = id;

    if (!this->GetMRMLScene())
      {
      vtkErrorMacro("No scene set.")
      return this->m_StringHolder.c_str();
      }

    vtkMRMLAnnotationNode* annotationNode =
        vtkMRMLAnnotationNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(
            id));

    if (!annotationNode)
      {
      vtkErrorMacro("MoveAnnotationUp: Could not get annotation node!")
      return this->m_StringHolder.c_str();
      }

    // get the corresponding hierarchy
    vtkMRMLAnnotationHierarchyNode* hNode =
        vtkMRMLAnnotationHierarchyNode::SafeDownCast(
            vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
                this->GetMRMLScene(), annotationNode->GetID()));

    if (!hNode)
      {
      vtkErrorMacro("MoveAnnotationUp: Could not get hierarchy node!")
      return this->m_StringHolder.c_str();
      }

    // where is it in the parent's list?
    int currentIndex = hNode->GetIndexInParent();
    vtkDebugMacro("MoveAnnotationUp: currentIndex = " << currentIndex);
    // now move it up one
    hNode->SetIndexInParent(currentIndex - 1);
    vtkDebugMacro("MoveAnnotationUp: after moving to " << currentIndex - 1 << ", current index is " << hNode->GetIndexInParent());
    // trigger an update on the q widget
    // done in the hierarchy node now when set the sorting value
    //annotationNode->Modified();
    // the id should be the same now
    this->m_StringHolder = annotationNode->GetID();
    return this->m_StringHolder.c_str();
  }

  //---------------------------------------------------------------------------
  const char* vtkSlicerAnnotationModuleLogic::MoveAnnotationDown(const char* id)
  {

    // reset stringHolder
    this->m_StringHolder = "";

    if (!id)
      {
      return this->m_StringHolder.c_str();
      }

    this->m_StringHolder = id;

    if (!this->GetMRMLScene())
      {
      vtkErrorMacro("MoveAnnotationDown: No scene set.")
      return this->m_StringHolder.c_str();
      }

    vtkMRMLAnnotationNode* annotationNode =
        vtkMRMLAnnotationNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(
            id));

    if (!annotationNode)
      {
      vtkErrorMacro("MoveAnnotationDown: Could not get annotation node!")
      return this->m_StringHolder.c_str();
      }

    // get the corrsponding hierarchy
    vtkMRMLAnnotationHierarchyNode* hNode =
        vtkMRMLAnnotationHierarchyNode::SafeDownCast(
            vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
                this->GetMRMLScene(), annotationNode->GetID()));

    if (!hNode)
      {
      vtkErrorMacro("MoveAnnotationDown: Could not get hierarchy node!")
      return this->m_StringHolder.c_str();
      }

    // where is it in the parent's list?
    int currentIndex = hNode->GetIndexInParent();
    // now move it down one
    hNode->SetIndexInParent(currentIndex + 1);
    // trigger an update on the q widget
    annotationNode->Modified();
    // the id should be the same now
    this->m_StringHolder = annotationNode->GetID();
    return this->m_StringHolder.c_str();

  }

  //---------------------------------------------------------------------------
  //
  //
  // Annotation Hierarchy Functionality
  //
  //
  //---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Return the toplevel Annotation hierarchy node ID or create one if there is none.
// If an optional annotationNode is given, insert the toplevel hierarchy before it. If not,
// just add the toplevel hierarchy node.
//---------------------------------------------------------------------------
char * vtkSlicerAnnotationModuleLogic::GetTopLevelHierarchyNodeID(vtkMRMLNode* node)
{
  if (this->GetMRMLScene() == NULL)
    {
    return NULL;
    }
  const char *topLevelName = "Main Hierarchy";
  char *toplevelNodeID = NULL;
  vtkCollection *col = this->GetMRMLScene()->GetNodesByClass("vtkMRMLAnnotationHierarchyNode");
  vtkMRMLAnnotationHierarchyNode *toplevelNode = NULL;
  unsigned int numNodes = col->GetNumberOfItems();
  if (numNodes != 0)
    {
    // iterate through the hierarchy nodes to find one with a name starting
    // with the top level name
    for (unsigned int n = 0; n < numNodes; n++)
      {
      vtkMRMLNode *thisNode = vtkMRMLNode::SafeDownCast(col->GetItemAsObject(n));
      if (thisNode && thisNode->GetName() &&
          strncmp(thisNode->GetName(), topLevelName, strlen(topLevelName)) == 0)
        {
        toplevelNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(col->GetItemAsObject(n));
        break;
        }
      }
    }
  
  if (!toplevelNode)
    {
    // no top level hierarchy node is currently in the scene, create a new one
    toplevelNode = vtkMRMLAnnotationHierarchyNode::New();
    
    toplevelNode->HideFromEditorsOff();
    toplevelNode->SetName(this->GetMRMLScene()->GetUniqueNameByString(topLevelName));
    
    if (!node)
      {
      this->GetMRMLScene()->AddNode(toplevelNode);
      }
    else
      {
      this->GetMRMLScene()->InsertBeforeNode(node, toplevelNode);
      }
    toplevelNodeID = toplevelNode->GetID();
    toplevelNode->Delete();
    }
  else
    {
    toplevelNodeID = toplevelNode->GetID();
    }
  col->RemoveAllItems();
  col->Delete();
  return toplevelNodeID;
}

//---------------------------------------------------------------------------
char * vtkSlicerAnnotationModuleLogic::GetTopLevelHierarchyNodeIDForNodeClass(vtkMRMLAnnotationNode *annotationNode)
{
  if (!annotationNode)
    {
    return NULL;
    }
  if (this->GetMRMLScene() == NULL)
    {
    return NULL;
    }

  // get the set of hierarchy nodes to check through
  vtkCollection *col = NULL;
  if (this->GetActiveHierarchyNode())
    {
    // get all the hierarchy nodes under the active one
    std::vector< vtkMRMLHierarchyNode *> childrenNodes;
    this->GetActiveHierarchyNode()->GetAllChildrenNodes(childrenNodes);
    // add them to the collection
    col = vtkCollection::New();
    for (unsigned int i = 0; i < childrenNodes.size(); i++)
      {
      col->AddItem(childrenNodes[i]);
      }
    // and add the active one!
    col->AddItem(this->GetActiveHierarchyNode());
    }
  else
    {
    // look for any annotation hierarchy nodes in the scene
    col = this->GetMRMLScene()->GetNodesByClass("vtkMRMLAnnotationHierarchyNode");
    }
  unsigned int numNodes = col->GetNumberOfItems();

  // iterate through the hierarchy nodes to find one with an attribute matching the input node's classname
  vtkMRMLAnnotationHierarchyNode *toplevelNode = NULL;
  char *toplevelNodeID = NULL;
  const char *attributeName = "MainChildType";
  for (unsigned int n = 0; n < numNodes; n++)
    {
    vtkMRMLNode *thisNode = vtkMRMLNode::SafeDownCast(col->GetItemAsObject(n));
    if (thisNode && thisNode->GetAttribute(attributeName) &&
        strcmp(thisNode->GetAttribute(attributeName), annotationNode->GetClassName()) == 0)
      {
      toplevelNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(col->GetItemAsObject(n));
      break;
      }
    }
  if (!toplevelNode)
    {
    // no hierarchy node mainly for this node class, create a new one
    toplevelNode = vtkMRMLAnnotationHierarchyNode::New();
    toplevelNode->SetAttribute(attributeName, annotationNode->GetClassName());
    toplevelNode->HideFromEditorsOff();
    // get the node tag name, remove the Annotation string, append List
    std::string nodeName = std::string(annotationNode->GetNodeTagName()).replace(0, strlen("Annotation"), "") + std::string(" List");
    toplevelNode->SetName(this->GetMRMLScene()->GetUniqueNameByString(nodeName.c_str()));
    // make it a child of the active hierarchy
    if (this->GetActiveHierarchyNode())
      {
      toplevelNode->SetParentNodeID(this->GetActiveHierarchyNode()->GetID());
      }
    this->GetMRMLScene()->AddNode(toplevelNode);
    toplevelNodeID = toplevelNode->GetID();
    toplevelNode->Delete();
    }
 else
    {
    toplevelNodeID = toplevelNode->GetID();
    }
  col->RemoveAllItems();
  col->Delete();
  return toplevelNodeID;
}

  //---------------------------------------------------------------------------
  // Add a new annotation hierarchy node for a given annotationNode.
  // The active hierarchy node will be the parent. If there is no
  // active hierarchy node, use the top-level annotation hierarchy node as the parent.
  // If there is no top-level annotation hierarchy node, create additionally a top-level hierarchy node which serves as
  // a parent to the new hierarchy node. Return true on success, false on failure.
  //---------------------------------------------------------------------------
  bool vtkSlicerAnnotationModuleLogic::AddHierarchyNodeForAnnotation(vtkMRMLAnnotationNode* annotationNode)
  {

    // check that there isn't already a hierarchy node for this node
    if (annotationNode && annotationNode->GetScene() && annotationNode->GetID())
      {
      vtkMRMLHierarchyNode *hnode =
          vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(
              annotationNode->GetScene(), annotationNode->GetID());
      if (hnode != NULL)
        {
        vtkMRMLAnnotationHierarchyNode *ahnode =
            vtkMRMLAnnotationHierarchyNode::SafeDownCast(hnode);
        if (ahnode != NULL)
          {
          vtkWarningMacro("AddHierarchyNodeForAnnotation: annotation node " << annotationNode->GetID() << " already has a hierarchy node, returning.");
          return true;
          }
        else
          {
          vtkWarningMacro("AddHierarchyNodeForAnnotation: found a hierarchy node for this annotation node, but it's not an annotation hierarchy node, so adding a new one");
          }
        }
      }
    else
      {
      vtkErrorMacro("AddHierarchyNodeForAnnotation: annotation node is null or has no scene or id, not checking for existing hierarchy node");
      }

    if (!this->GetActiveHierarchyNodeID())
      {
      // no active hierarchy node, this means we create the new node directly under the top-level hierarchy node
      char * toplevelHierarchyNodeID = NULL;
      if (!annotationNode)
        {
        // we just add a new toplevel hierarchy node
        toplevelHierarchyNodeID = this->GetTopLevelHierarchyNodeID(0);
        }
      else
        {
        // we need to insert the new toplevel hierarchy before the given annotationNode
        toplevelHierarchyNodeID = this->GetTopLevelHierarchyNodeID(annotationNode);
        }

      if (!toplevelHierarchyNodeID)
        {
        vtkErrorMacro("AddNewHierarchyNode: Toplevel hierarchy node was NULL.")
        return false;
        }
      this->SetActiveHierarchyNodeID(toplevelHierarchyNodeID);
      }

    char *toplevelIDForThisClass = this->GetTopLevelHierarchyNodeIDForNodeClass(annotationNode);
    
    // Create a hierarchy node
    vtkMRMLAnnotationHierarchyNode* hierarchyNode =
        vtkMRMLAnnotationHierarchyNode::New();

    if (toplevelIDForThisClass)
      {
      hierarchyNode->SetParentNodeID(toplevelIDForThisClass);
      }
    else
      {
      hierarchyNode->SetParentNodeID(this->GetActiveHierarchyNodeID());
      }
    hierarchyNode->SetScene(this->GetMRMLScene());

    if (!annotationNode)
      {
      // this is a user created hierarchy!

      // we want to see that!
      hierarchyNode->HideFromEditorsOff();

      hierarchyNode->SetName(
          this->GetMRMLScene()->GetUniqueNameByString("List"));

      this->GetMRMLScene()->AddNode(hierarchyNode);

      // we want it to be the active hierarchy from now on
      this->SetActiveHierarchyNodeID(hierarchyNode->GetID());
      }
    else
      {
      // this is the 1-1 hierarchy node for a given annotation node
      hierarchyNode->AllowMultipleChildrenOff();
      
      // we do not want to see that!
      hierarchyNode->HideFromEditorsOn();


      hierarchyNode->SetName(this->GetMRMLScene()->GetUniqueNameByString(
          "AnnotationHierarchy"));

      this->GetMRMLScene()->InsertBeforeNode(annotationNode, hierarchyNode);

      // set the displayable node id to point to this annotation node
      hierarchyNode->SetDisplayableNodeID(annotationNode->GetID());
      annotationNode->Modified();
      }
    // it's been added to the scene, delete this pointer
    hierarchyNode->Delete();
    return true;

  }

//---------------------------------------------------------------------------
// Add a new visible annotation hierarchy.
// The active hierarchy node will be the parent. If there is no
// active hierarchy node, use the top-level annotation hierarchy node as the parent.
// If there is no top-level annotation hierarchy node, create additionally a top-level hierarchy node which serves as
// a parent to the new hierarchy node. The newly added hierarchy node will be the
// active hierarchy node. Return true on success, false on failure.
//---------------------------------------------------------------------------
bool vtkSlicerAnnotationModuleLogic::AddHierarchy()
{
  return this->AddHierarchyNodeForAnnotation(0);
}



//---------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode *vtkSlicerAnnotationModuleLogic::GetActiveHierarchyNode()
{
  if (!this->GetActiveHierarchyNodeID())
    {
    // there was no active hierarchy
    // we then use the toplevel hierarchyNode
    char* toplevelNodeID = this->GetTopLevelHierarchyNodeID();

    if (!toplevelNodeID)
      {
      vtkErrorMacro("GetActiveHierarchyNode: Could not find or create any hierarchy.")
      return NULL;
      }

    this->SetActiveHierarchyNodeID(toplevelNodeID);
    }
  if (this->GetMRMLScene()->GetNodeByID(this->GetActiveHierarchyNodeID()) == NULL)
    {
    return NULL;
    }
  return vtkMRMLAnnotationHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetActiveHierarchyNodeID()));
}

  //---------------------------------------------------------------------------
  //
  //
  // Annotation SnapShot Functionality
  //
  //
  //---------------------------------------------------------------------------

  //---------------------------------------------------------------------------
  // Create a snapShot. This includes a screenshot of a specific view (see \ref GrabScreenShot(int screenshotWindow)),
  // a multiline text description.
  //---------------------------------------------------------------------------
  void vtkSlicerAnnotationModuleLogic::CreateSnapShot(const char* name, const char* description, int screenshotType, double scaleFactor, vtkImageData* screenshot)
  {

    if (!screenshot)
      {
      vtkErrorMacro("CreateSnapShot: No screenshot was set.")
      return;
      }

    if (!this->m_Widget)
      {
      vtkErrorMacro("CreateSnapShot: We need the widget here.")
      return;
      }

    vtkStdString nameString = vtkStdString(name);

    vtkMRMLAnnotationSnapshotNode * newSnapshotNode =
        vtkMRMLAnnotationSnapshotNode::New();
    newSnapshotNode->SetScene(this->GetMRMLScene());
    if (strcmp(nameString, ""))
      {
      // a name was specified
      newSnapshotNode->SetName(nameString.c_str());
      }
    else
      {
      // if no name is specified, generate a new unique one
      newSnapshotNode->SetName(this->GetMRMLScene()->GetUniqueNameByString(
          "Screenshot"));
      }
    newSnapshotNode->SetSnapshotDescription(description);
    newSnapshotNode->SetScreenShotType(screenshotType);
    newSnapshotNode->SetScreenShot(screenshot);
    newSnapshotNode->ModifiedSinceReadOn();
    newSnapshotNode->SetScaleFactor(scaleFactor);
    newSnapshotNode->HideFromEditorsOff();
    this->GetMRMLScene()->AddNode(newSnapshotNode);
    newSnapshotNode->Delete();
  }

  //---------------------------------------------------------------------------
  // Modify an existing annotation snapShot.
  //---------------------------------------------------------------------------
  void vtkSlicerAnnotationModuleLogic::ModifySnapShot(vtkStdString id, const char* name, const char* description, int screenshotType, double scaleFactor, vtkImageData* screenshot)
  {

    if (!screenshot)
      {
      vtkErrorMacro("ModifySnapShot: No screenshot was set.")
      return;
      }

    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id.c_str());

    if (!node)
      {
      vtkErrorMacro("ModifySnapShot: Could not get node: " << id.c_str())
      return;
      }

    vtkMRMLAnnotationSnapshotNode* snapshotNode =
        vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

    if (!snapshotNode)
      {
      vtkErrorMacro("ModifySnapShot: Could not get snapshot node.")
      return;
      }

    vtkStdString nameString = vtkStdString(name);

    if (strcmp(nameString, ""))
      {
      // a name was specified
      snapshotNode->SetName(nameString.c_str());
      }
    else
      {
      // if no name is specified, generate a new unique one
      snapshotNode->SetName(this->GetMRMLScene()->GetUniqueNameByString(
          "Snapshot"));
      }
    snapshotNode->SetSnapshotDescription(description);
    snapshotNode->SetScreenShotType(screenshotType);
    snapshotNode->SetScreenShot(screenshot);
    snapshotNode->ModifiedSinceReadOn();
    snapshotNode->SetScaleFactor(scaleFactor);
    snapshotNode->Modified();
    snapshotNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent,
        snapshotNode);

  }

  //---------------------------------------------------------------------------
  // Return the description of an existing Annotation snapShot node.
  //---------------------------------------------------------------------------
  vtkStdString vtkSlicerAnnotationModuleLogic::GetSnapShotDescription(const char* id)
  {
    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

    if (!node)
      {
      vtkErrorMacro("GetSnapShotDescription: Could not get mrml node!")
      return 0;
      }

    vtkMRMLAnnotationSnapshotNode* snapshotNode =
        vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

    if (!snapshotNode)
      {
      vtkErrorMacro("GetSnapShotDescription: Could not get snapshot node!")
      return 0;
      }

    return snapshotNode->GetSnapshotDescription();
  }

  //---------------------------------------------------------------------------
  // Return the screenshotType of an existing Annotation snapShot node.
  //---------------------------------------------------------------------------
  int vtkSlicerAnnotationModuleLogic::GetSnapShotScreenshotType(const char* id)
  {
    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

    if (!node)
      {
      vtkErrorMacro("GetSnapShotScreenshotType: Could not get mrml node!")
      return 0;
      }

    vtkMRMLAnnotationSnapshotNode* snapshotNode =
        vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

    if (!snapshotNode)
      {
      vtkErrorMacro("GetSnapShotScreenshotType: Could not get snapshot node!")
      return 0;
      }

    return snapshotNode->GetScreenShotType();
  }

  //---------------------------------------------------------------------------
  // Return the screenshotType of an existing Annotation snapShot node.
  //---------------------------------------------------------------------------
  double vtkSlicerAnnotationModuleLogic::GetSnapShotScaleFactor(const char* id)
  {
    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

    if (!node)
      {
      vtkErrorMacro("GetSnapShotScaleFactor: Could not get mrml node!")
      return 0;
      }

    vtkMRMLAnnotationSnapshotNode* snapshotNode =
        vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

    if (!snapshotNode)
      {
      vtkErrorMacro("GetSnapShotScaleFactor: Could not get snapshot node!")
      return 0;
      }

    return snapshotNode->GetScaleFactor();
  }

  //---------------------------------------------------------------------------
  // Return the screenshot of an existing Annotation snapShot node.
  //---------------------------------------------------------------------------
  vtkImageData* vtkSlicerAnnotationModuleLogic::GetSnapShotScreenshot(const char* id)
  {
    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

    if (!node)
      {
      vtkErrorMacro("GetSnapShotScreenshot: Could not get mrml node!")
      return 0;
      }

    vtkMRMLAnnotationSnapshotNode* snapshotNode =
        vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

    if (!snapshotNode)
      {
      vtkErrorMacro("GetSnapShotScreenshot: Could not get snapshot node!")
      return 0;
      }

    return snapshotNode->GetScreenShot();
  }

  //---------------------------------------------------------------------------
  // Check if node id corresponds to a snapShot node.
  //---------------------------------------------------------------------------
  bool vtkSlicerAnnotationModuleLogic::IsSnapshotNode(const char* id)
  {

    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

    if (!node)
      {
      vtkErrorMacro("IsSnapshotNode: Invalid node.")
      return false;
      }

    return node->IsA("vtkMRMLAnnotationSnapshotNode");

  }

  //---------------------------------------------------------------------------
  //
  //
  // Place Annotations programmatically
  //
  //
  //---------------------------------------------------------------------------

  //---------------------------------------------------------------------------
  //
  //
  // Report functionality
  //
  //
  //---------------------------------------------------------------------------

  //---------------------------------------------------------------------------
  const char* vtkSlicerAnnotationModuleLogic::GetHTMLRepresentation(vtkMRMLAnnotationHierarchyNode* hierarchyNode, int level)
  {
    if (!hierarchyNode)
      {
      vtkErrorMacro("GetHTMLRepresentation: We need a hierarchy Node here.")
      return 0;
      }

    vtkStdString html =
        vtkStdString("<tr bgcolor=#E0E0E0><td valign='middle'>");

    // level
    for (int i = 0; i < level; ++i)
      {
      html += "&nbsp;&nbsp;&nbsp;&nbsp;";
      }

    // icon
    html += "<img src='";
    html += this->GetAnnotationIcon(hierarchyNode->GetID());
    html += "'>";

    html += "</td><td valign='middle'>";

    html += "&nbsp;";

    html += "</td><td valign='middle'>";

    // text
    html += hierarchyNode->GetName();

    html += "</td></tr>";

    this->m_StringHolder = html;

    return this->m_StringHolder.c_str();

  }

  //---------------------------------------------------------------------------
  const char* vtkSlicerAnnotationModuleLogic::GetHTMLRepresentation(vtkMRMLAnnotationNode* annotationNode, int level)
  {
    if (!annotationNode)
      {
      vtkErrorMacro("GetHTMLRepresentation: We need an annotation Node here.")
      return 0;
      }

    vtkStdString html = vtkStdString("<tr><td valign='middle'>");

    // level
    for (int i = 0; i < level; ++i)
      {
      html += "&nbsp;&nbsp;&nbsp;&nbsp;";
      }

    // icon
    html += "<img src='";
    html += this->GetAnnotationIcon(annotationNode->GetID());
    html += "'>";

    html += "</td><td valign='middle'>";

    // if this is a snapshotNode, we want to include the image here
    if (annotationNode->IsA("vtkMRMLAnnotationSnapshotNode"))
      {
      vtkImageData* image =
          this->GetSnapShotScreenshot(annotationNode->GetID());

      if (image)
        {

        QString tempPath =
            qSlicerCoreApplication::application()->temporaryPath();
        tempPath.append(annotationNode->GetID());
        tempPath.append(".png");

        QByteArray tempPathArray = tempPath.toLatin1();
        VTK_CREATE(vtkPNGWriter,w);
        w->SetInput(image);
        w->SetFileName(tempPathArray.data());
        w->Write();

        html += "<img src='";
        html += tempPathArray.data();
        html += "' width='400'>";

        }
      }
    else
      {
      html += this->GetAnnotationMeasurement(annotationNode->GetID(), true);
      }
    html += "</td><td valign='middle'>";

    // text
    html += this->GetAnnotationText(annotationNode->GetID());
    // if this is a snapshotNode, we want to include the image here
    if (annotationNode->IsA("vtkMRMLAnnotationSnapshotNode"))
      {
      html += "<br><br>";
      html += this->GetSnapShotDescription(annotationNode->GetID());
      }

    html += "</td></tr>";

    this->m_StringHolder = html;

    return this->m_StringHolder.c_str();

  }

  //---------------------------------------------------------------------------
  vtkMRMLAnnotationTextDisplayNode *vtkSlicerAnnotationModuleLogic::GetTextDisplayNode(const char *id)
  {
    if (!id)
      {
      return NULL;
      }
    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);
    if (!node)
      {
      return NULL;
      }
    vtkMRMLAnnotationNode *textNode = vtkMRMLAnnotationNode::SafeDownCast(node);
    if (!textNode)
      {
      return NULL;
      }
    return textNode->GetAnnotationTextDisplayNode();
  }

  //---------------------------------------------------------------------------
  vtkMRMLAnnotationPointDisplayNode *vtkSlicerAnnotationModuleLogic::GetPointDisplayNode(const char *id)
  {
    if (!id)
      {
      return NULL;
      }
    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);
    if (!node)
      {
      return NULL;
      }
    vtkMRMLAnnotationControlPointsNode *pointsNode =
        vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
    if (!pointsNode)
      {
      return NULL;
      }
    // get the point display node
    return pointsNode->GetAnnotationPointDisplayNode();
  }

  //---------------------------------------------------------------------------
  vtkMRMLAnnotationLineDisplayNode *vtkSlicerAnnotationModuleLogic::GetLineDisplayNode(const char *id)
  {
    if (!id)
      {
      return NULL;
      }
    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);
    if (!node)
      {
      return NULL;
      }
    vtkMRMLAnnotationLinesNode *linesNode =
        vtkMRMLAnnotationLinesNode::SafeDownCast(node);
    if (!linesNode)
      {
      return NULL;
      }

    return linesNode->GetAnnotationLineDisplayNode();
  }
