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

#include "qMRMLTreeWidget.h"
#include "qSlicerCoreApplication.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>

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
  this->m_ActiveHierarchy = 0;

  this->m_MeasurementFormat = new char[8];
  sprintf(this->m_MeasurementFormat,"%s","%.1f");

  this->m_CoordinateFormat = new char[8];
  sprintf(this->m_CoordinateFormat,"%s","%.0f");

}

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic::~vtkSlicerAnnotationModuleLogic()
{

  if (this->m_Widget)
    {
    //delete this->m_Widget->;
    this->m_Widget = 0;
    }

  if (this->m_LastAddedAnnotationNode)
    {
    this->m_LastAddedAnnotationNode->Delete();
    this->m_LastAddedAnnotationNode = 0;
    }

  if (this->m_ActiveHierarchy)
    {
    this->m_ActiveHierarchy->Delete();
    this->m_ActiveHierarchy = 0;
    }

  if (this->m_MeasurementFormat)
    {
    delete [] this->m_MeasurementFormat;
    this->m_MeasurementFormat = NULL;
    }

  if (this->m_CoordinateFormat)
    {
    delete [] this->m_CoordinateFormat;
    this->m_CoordinateFormat = NULL;
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
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
  
  // be sure to listen to the mrml events, will undo any other observed events
  // to add new ones (double call doesn't hurt)
  this->InitializeEventListeners();
}

//-----------------------------------------------------------------------------
//
//
// MRML event handling
//
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::ProcessMRMLEvents(
  vtkObject *caller, unsigned long event, void *callData)
{
  vtkDebugMacro("ProcessMRMLEvents: Event "<< event);

  vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*> (callData);

  if (event==vtkMRMLScene::SceneClosedEvent)
    {
    this->OnMRMLSceneClosedEvent();
    return;
    }

  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast(caller);
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
        vtkWarningMacro("ProcessMRMLEvents: unhandled event on interaction node: " << event);
      }
    return;
    }
  
  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
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

  vtkMRMLAnnotationHierarchyNode* hierarchyNode = this->AddHierarchyNodeForAnnotation(annotationNode);
  if (!hierarchyNode)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: No hierarchyNode found.")
    return;
    }
  hierarchyNode->SetDisplayableNodeID(annotationNode->GetID());
  annotationNode->Modified();

  // we pass the hierarchy node along - it includes the pointer to the actual annotationNode
  this->AddNodeCompleted(hierarchyNode);
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

  if (this->m_ActiveHierarchy)
    {
    this->m_ActiveHierarchy = 0;
    }

  if (this->m_Widget)
    {
    this->m_Widget->refreshTree();
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::OnInteractionModeChangedEvent(vtkMRMLInteractionNode *interactionNode)
{
  vtkDebugMacro("OnInteractionModeChangedEvent");
  if (!interactionNode ||
      this->m_Widget == NULL)
    {
    return;
    }
  this->m_Widget->updateWidgetFromInteractionMode(interactionNode);
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::OnInteractionModePersistenceChangedEvent(vtkMRMLInteractionNode *interactionNode)
{
  vtkDebugMacro("OnInteractionModePersistenceChangedEvent");
  if (!interactionNode ||
      this->m_Widget == NULL)
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
void vtkSlicerAnnotationModuleLogic::InitializeEventListeners()
{
  if (this->GetMRMLScene() == NULL)
    {
    vtkWarningMacro("InitializeEventListeners: no scene to listen to!");
    return;
    }
      
  // a good time to add the observed events!
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(this->GetMRMLScene(), events);
  events->Delete();

  // also observe the interaction node for changes
  vtkMRMLInteractionNode *interactionNode =
      vtkMRMLInteractionNode::SafeDownCast(
          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (interactionNode)
    {
    vtkIntArray *interactionEvents = vtkIntArray::New();
    interactionEvents->InsertNextValue(vtkMRMLInteractionNode::InteractionModeChangedEvent);
    interactionEvents->InsertNextValue(vtkMRMLInteractionNode::InteractionModePersistenceChangedEvent);
    vtkSetAndObserveMRMLNodeEventsMacro(interactionNode, interactionNode, interactionEvents);
    interactionEvents->Delete();
    }
  else { vtkWarningMacro("InitializeEventListeners: No interaction node!"); }
  vtkDebugMacro("InitializeEventListeners: listeners added");

}

//-----------------------------------------------------------------------------
// Add Annotation Node
//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::AddAnnotationNode(const char * nodeDescriptor)
{

  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  if (!selectionNode)
    {
    vtkErrorMacro("AddAnnotationNode: No selection node in the scene.");
    return;
    }

  selectionNode->SetActiveAnnotationID(nodeDescriptor);

  this->StartPlaceMode();

}

//---------------------------------------------------------------------------
// Start the place mouse mode
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::StartPlaceMode()
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
  interactionNode->SetPlaceModePersistence(1);

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
void vtkSlicerAnnotationModuleLogic::AddNodeCompleted(vtkMRMLAnnotationHierarchyNode* hierarchyNode)
{

  if (!hierarchyNode)
    {
    return;
    }

  if (!this->m_Widget)
    {
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(hierarchyNode->GetDisplayableNode());

  if (!annotationNode)
    {
    vtkErrorMacro("AddNodeCompleted: Could not get annotationNode.")
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
void vtkSlicerAnnotationModuleLogic::StopPlaceMode()
{

  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  if (!selectionNode)
    {
    vtkErrorMacro("StopPlaceMode: No selection node in the scene.");
    return;
    }

  selectionNode->SetActiveAnnotationID("");

  vtkMRMLInteractionNode *interactionNode =
      vtkMRMLInteractionNode::SafeDownCast(
          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (interactionNode == NULL)
    {
    vtkErrorMacro ( "StopPlaceMode: No interaction node in the scene." );
    return;
    }

  interactionNode->SetPlaceModePersistence(0);
  interactionNode->SetCurrentInteractionMode(
      vtkMRMLInteractionNode::ViewTransform);

  if (interactionNode->GetCurrentInteractionMode()
      != vtkMRMLInteractionNode::ViewTransform)
    {

    vtkErrorMacro("AddTextNode: Could not set transform mode!");

    }

}

//---------------------------------------------------------------------------
// Cancel the current placement or remove the last placed node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::CancelCurrentOrRemoveLastAddedAnnotationNode()
{

  // fire cancel placement event
  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  if (!selectionNode)
    {
    vtkErrorMacro("CancelCurrentOrRemoveLastAddedAnnotationNode: No selection node in the scene.");
    return;
    }

  selectionNode->SetActiveAnnotationID("CancelPlacement");

  vtkMRMLAnnotationNode* tmpNode = vtkMRMLAnnotationNode::New();
  tmpNode->SetScene(this->GetMRMLScene());
  this->GetMRMLScene()->AddNode(tmpNode);
  this->GetMRMLScene()->RemoveNode(tmpNode);
  // end of cancel placement event

  if (!this->m_LastAddedAnnotationNode)
    {
    return;
    }

  if (this->m_LastAddedAnnotationNode->IsA("vtkMRMLAnnotationTextNode") || this->m_LastAddedAnnotationNode->IsA("vtkMRMLAnnotationFiducialNode"))
    {
    // for text annotations or fiducials, just remove the last node
    this->GetMRMLScene()->RemoveNode(this->m_LastAddedAnnotationNode);
    this->m_LastAddedAnnotationNode = 0;
    }


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
  vtkMRMLDisplayableHierarchyNode* displayableHierarchyNode = vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(annotationNode->GetScene(), annotationNode->GetID());
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
  if(!this->GetMRMLScene())
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

  vtkMRMLAnnotationDisplayNode* annotationDisplayNode = vtkMRMLAnnotationDisplayNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationDisplayNode);
  annotationDisplayNode->Delete();

  vtkMRMLAnnotationStorageNode* annotationStorageNode = vtkMRMLAnnotationStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationStorageNode);
  annotationStorageNode->Delete();

  // Control Points
  vtkMRMLAnnotationControlPointsNode* annotationControlPointsNode = vtkMRMLAnnotationControlPointsNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationControlPointsNode);
  annotationControlPointsNode->Delete();

  vtkMRMLAnnotationControlPointsStorageNode* annotationControlPointsStorageNode = vtkMRMLAnnotationControlPointsStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationControlPointsStorageNode);
  annotationControlPointsStorageNode->Delete();

  vtkMRMLAnnotationPointDisplayNode* annotationControlPointsDisplayNode = vtkMRMLAnnotationPointDisplayNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationControlPointsDisplayNode);
  annotationControlPointsDisplayNode->Delete();

  // Lines
  vtkMRMLAnnotationLinesNode* annotationLinesNode = vtkMRMLAnnotationLinesNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationLinesNode);
  annotationLinesNode->Delete();

  vtkMRMLAnnotationLinesStorageNode* annotationLinesStorageNode = vtkMRMLAnnotationLinesStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationLinesStorageNode);
  annotationLinesStorageNode->Delete();

  vtkMRMLAnnotationLineDisplayNode* annotationLinesDisplayNode = vtkMRMLAnnotationLineDisplayNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationLinesDisplayNode);
  annotationLinesDisplayNode->Delete();

  // Text
  vtkMRMLAnnotationTextDisplayNode* annotationTextDisplayNode = vtkMRMLAnnotationTextDisplayNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationTextDisplayNode);
  annotationTextDisplayNode->Delete();

  //
  // Now the actual Annotation tool nodes
  //

  // Snapshot annotation
  vtkMRMLAnnotationSnapshotNode* annotationSnapshotNode = vtkMRMLAnnotationSnapshotNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationSnapshotNode);
  annotationSnapshotNode->Delete();

  // Text annotation
  vtkMRMLAnnotationTextNode* annotationTextNode = vtkMRMLAnnotationTextNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationTextNode);
  annotationTextNode->Delete();

  // Ruler annotation
  vtkMRMLAnnotationRulerNode* annotationRulerNode = vtkMRMLAnnotationRulerNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationRulerNode);
  annotationRulerNode->Delete();

  vtkMRMLAnnotationRulerStorageNode* annotationRulerStorageNode = vtkMRMLAnnotationRulerStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationRulerStorageNode);
  annotationRulerStorageNode->Delete();

  // ROI annotation
  vtkMRMLAnnotationROINode* annotationROINode = vtkMRMLAnnotationROINode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationROINode);
  annotationROINode->Delete();

  // ROI annotation backwards compatibility
  vtkMRMLAnnotationROINode* oldAnnotationROINode = vtkMRMLAnnotationROINode::New();
  this->GetMRMLScene()->RegisterNodeClass(oldAnnotationROINode,"MRMLROINode");
  oldAnnotationROINode->Delete();

  // Bidimensional annotation
  vtkMRMLAnnotationBidimensionalNode* annotationBidimensionalNode = vtkMRMLAnnotationBidimensionalNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationBidimensionalNode);
  annotationBidimensionalNode->Delete();

  // Fiducial annotation
  vtkMRMLAnnotationFiducialNode* annotationFiducialNode = vtkMRMLAnnotationFiducialNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationFiducialNode);
  annotationFiducialNode->Delete();

  //
  // Annotation hierarchies
  //
  vtkMRMLAnnotationHierarchyNode* annotationHierarchyNode = vtkMRMLAnnotationHierarchyNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationHierarchyNode);
  annotationHierarchyNode->Delete();

  // set up the event listeners now
  this->InitializeEventListeners();
}

//---------------------------------------------------------------------------
// Check if the id points to an annotation node
//---------------------------------------------------------------------------
bool vtkSlicerAnnotationModuleLogic::IsAnnotationNode(const char* id)
{
  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

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
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationName: Could not get the MRML node.")
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
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationText: Could not get the MRML node.")
    return 0;
    }

  // special case for annotation snapShots
  vtkMRMLAnnotationSnapshotNode* snapshotNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);
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
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationText: Could not get the MRML node.")
    return;
    }

  // special case for snapShots
  vtkMRMLAnnotationSnapshotNode* snapshotNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);
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
    vtkErrorMacro("SetAnnotationText: Could not get the text.")
    return;
    }

  annotationNode->SetText(0,newtext,1,1);

}

//---------------------------------------------------------------------------
// Get the textScale of a MRML Annotation node
//---------------------------------------------------------------------------
double vtkSlicerAnnotationModuleLogic::GetAnnotationTextScale(const char* id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the MRML node.")
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
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the MRML node.")
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
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the MRML node.")
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the annotation MRML node.")
    return 0;
    }

  return annotationNode->GetAnnotationTextDisplayNode()->GetSelectedColor();

}

//---------------------------------------------------------------------------
// Set the selected text color of a MRML Annotation node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationTextSelectedColor(const char* id, double * color)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the annotation MRML node.")
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
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the MRML node.")
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the annotation MRML node.")
    return 0;
    }

  return annotationNode->GetAnnotationTextDisplayNode()->GetColor();

}

//---------------------------------------------------------------------------
// Set the unselected text color of a MRML Annotation node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetAnnotationTextUnselectedColor(const char* id, double * color)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the annotation MRML node.")
    return;
    }

  annotationNode->GetAnnotationTextDisplayNode()->SetColor(color);

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// Get the measurement value of a MRML Annotation node
//---------------------------------------------------------------------------
const char * vtkSlicerAnnotationModuleLogic::GetAnnotationMeasurement(const char* id, bool showUnits)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationMeasurement: Could not get the MRML node.")
    return 0;
    }

  // reset stringHolder
  this->m_StringHolder = "";

  // special case for annotation snapShots
  vtkMRMLAnnotationSnapshotNode* snapshotNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);
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
    sprintf(string, this->m_MeasurementFormat, vtkMRMLAnnotationRulerNode::SafeDownCast(annotationNode)->GetDistanceMeasurement());

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
    ss << vtkMRMLAnnotationAngleNode::SafeDownCast(annotationNode)->GetAngleMeasurement();
    if (showUnits)
      {
      ss << " [degrees]";
      }

    this->m_StringHolder = ss.str();
    }
  else if (node->IsA("vtkMRMLAnnotationFiducialNode"))
    {
    std::ostringstream ss;
    double* tmpPtr = vtkMRMLAnnotationFiducialNode::SafeDownCast(annotationNode)->GetControlPointCoordinates(0);
    double coordinates[3] = {tmpPtr[0], tmpPtr[1], tmpPtr[2]};

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
    if (vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node)->GetBidimensionalMeasurement().size() != 2)
      {
      // measurement is not ready
      measurement1 = 0;
      measurement2 = 0;
      }
    else
      {
      // measurement is ready
      measurement1 = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node)->GetBidimensionalMeasurement()[0];
      measurement2 = vtkMRMLAnnotationBidimensionalNode::SafeDownCast(node)->GetBidimensionalMeasurement()[1];
      }

    std::ostringstream ss;

    // the greatest measurement should appear first..
    bool measurement1first = false;

    char string[512];
    if (measurement1>measurement2)
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
  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (annotationNode)
    {
    return annotationNode->GetIcon();
    }

  vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (hierarchyNode)
    {
    return hierarchyNode->GetIcon();
    }

  vtkMRMLAnnotationSnapshotNode* snapshotNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

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
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationLockedUnlocked: Could not get the MRML node.")
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
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationLockedUnlocked: Could not get the MRML node.")
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
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationVisibility: Could not get the MRML node.")
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
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationVisibility: Could not get the MRML node.")
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
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationSelected: Could not get the MRML node.")
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
// Backup an AnnotationMRML node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::BackupAnnotationNode(const char * id)
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("BackupAnnotationNode: Could not get the MRML node.")
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

  for(int i=0; i<annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkMRMLAnnotationDisplayNode * displayNode = vtkMRMLAnnotationDisplayNode::SafeDownCast(annotationNode->GetNthDisplayNode(i));

    if(!displayNode)
      {
      vtkErrorMacro("BackupAnnotationNode: Could not get the annotationMRMLDisplay node.")
      return;
      }

    displayNode->CreateBackup();

    }

}

//---------------------------------------------------------------------------
// Restore a backup version of a AnnotationMRML node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::RestoreAnnotationNode(const char * id)
{
  vtkDebugMacro("RestoreAnnotationNode: " << id)

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("RestoreAnnotationNode: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("RestoreAnnotationNode: Could not get the annotationMRML node.")
    return;
    }

  for(int i=0; i<annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkMRMLAnnotationDisplayNode * displayNode = vtkMRMLAnnotationDisplayNode::SafeDownCast(annotationNode->GetNthDisplayNode(i));

    if(!displayNode)
      {
      vtkErrorMacro("BackupAnnotationNode: Could not get the annotationMRMLDisplay node.")
      return;
      }

    if (displayNode->IsA("vtkMRMLAnnotationTextDisplayNode"))
      {
      // restore display nodes, one by one
       vtkMRMLAnnotationTextDisplayNode * backupDisplayNode = vtkMRMLAnnotationTextDisplayNode::SafeDownCast(displayNode->GetBackup());

       if (!backupDisplayNode)
         {
         vtkErrorMacro("RestoreAnnotationNode: There was no backup display node.")
         return;
         }

       // now restore
       displayNode->Copy(backupDisplayNode);
      }

    }

  vtkMRMLAnnotationNode * backupNode = annotationNode->GetBackup();

  if (!backupNode)
    {
    vtkErrorMacro("RestoreAnnotationNode: There was no backup.")
    return;
    }

  // now restore
  annotationNode->Copy(backupNode);

  annotationNode->InvokeEvent(vtkCommand::ModifiedEvent);

}

//---------------------------------------------------------------------------
// Restore the view when it was last modified of an AnnotationMRML node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::RestoreAnnotationView(const char * id)
{

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("RestoreAnnotationView: Could not get the MRML node.")
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

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

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!annotationNode)
    {
    vtkErrorMacro("MoveAnnotationUp: Could not get annotation node!")
    return this->m_StringHolder.c_str();
    }

  // get the corresponding hierarchy
  vtkMRMLAnnotationHierarchyNode* hNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(this->GetMRMLScene(),annotationNode->GetID()));

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

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!annotationNode)
    {
    vtkErrorMacro("MoveAnnotationDown: Could not get annotation node!")
    return this->m_StringHolder.c_str();
    }

  // get the corrsponding hierarchy
  vtkMRMLAnnotationHierarchyNode* hNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(this->GetMRMLScene(),annotationNode->GetID()));

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
// Return the toplevel Annotation hierarchy node or create one if there is none.
// If an optional annotationNode is given, insert the toplevel hierarchy before it. If not,
// just add the toplevel hierarchy node.
//---------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode* vtkSlicerAnnotationModuleLogic::GetTopLevelHierarchyNode(vtkMRMLNode* node)
{

  vtkMRMLAnnotationHierarchyNode* toplevelNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(0,"vtkMRMLAnnotationHierarchyNode"));

  if (!toplevelNode)
    {
    // no annotation hierarchy node is currently in the scene, create a new one
    toplevelNode = vtkMRMLAnnotationHierarchyNode::New();
    toplevelNode->SetScene(this->GetMRMLScene());
    toplevelNode->HideFromEditorsOff();
    toplevelNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationToplevelHierarchyNode"));

    if (!node)
      {
      this->GetMRMLScene()->AddNode(toplevelNode);
      }
    else
      {
      this->GetMRMLScene()->InsertBeforeNode(node,toplevelNode);
      }
    }

  return toplevelNode;
}

//---------------------------------------------------------------------------
// Add a new annotation hierarchy node for a given annotationNode.
// The active hierarchy node will be the parent. If there is no
// active hierarchy node, use the top-level annotation hierarchy node as the parent.
// If there is no top-level annotation hierarchy node, create additionally a top-level hierarchy node which serves as
// a parent to the new hierarchy node. Return the new hierarchy node.
//---------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode* vtkSlicerAnnotationModuleLogic::AddHierarchyNodeForAnnotation(vtkMRMLAnnotationNode* annotationNode)
{

  // check that there isn't already a hierarchy node for this node
  if (annotationNode && annotationNode->GetScene() && annotationNode->GetID())
    {
    vtkMRMLHierarchyNode *hnode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(annotationNode->GetScene(), annotationNode->GetID());
    if (hnode != NULL)
      {
      vtkMRMLAnnotationHierarchyNode *ahnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(hnode);
      if (ahnode != NULL)
        {
        vtkWarningMacro("AddHierarchyNodeForAnnotation: annotation node " << annotationNode->GetID() << " already has a hierarchy node, returning.");
        return ahnode;
        }
      else { vtkWarningMacro("AddHierarchyNodeForAnnotation: found a hierarchy node for this annotation node, but it's not an annotatin hierarchy node, so adding a new one"); }
      }
    }
  else { vtkErrorMacro("AddHierarchyNodeForAnnotation: annotation node is null or has no scene or id, not checking for existing hierarchy node"); }
  
  if (!this->m_ActiveHierarchy)
    {
    // no active hierarchy node, this means we create the new node directly under the top-level hierarchy node
    vtkMRMLAnnotationHierarchyNode* toplevelHierarchyNode = 0;
    if (!annotationNode)
      {
      // we just add a new toplevel hierarchy node
      toplevelHierarchyNode = this->GetTopLevelHierarchyNode(0);
      }
    else
      {
      // we need to insert the new toplevel hierarchy before the given annotationNode
      toplevelHierarchyNode = this->GetTopLevelHierarchyNode(annotationNode);
      }

    this->m_ActiveHierarchy = toplevelHierarchyNode;

    if (!toplevelHierarchyNode)
      {
      vtkErrorMacro("AddNewHierarchyNode: Toplevel hierarchy node was NULL.")
      return 0;
      }

    }

  // Create a hierarchy node
  vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::New();

  hierarchyNode->SetParentNodeID(this->m_ActiveHierarchy->GetID());
  hierarchyNode->SetScene(this->GetMRMLScene());

  if (!annotationNode)
    {
    // this is a user created hierarchy!

    // we want to see that!
    hierarchyNode->HideFromEditorsOff();

    hierarchyNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("List"));

    this->GetMRMLScene()->AddNode(hierarchyNode);

    // we want it to be the active hierarchy from now on
    this->m_ActiveHierarchy = hierarchyNode;
    }
  else
    {
    // this is the 1-1 hierarchy node for a given annotation node

    // we do not want to see that!
    hierarchyNode->HideFromEditorsOn();

    hierarchyNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationHierarchy"));

    this->GetMRMLScene()->InsertBeforeNode(annotationNode,hierarchyNode);
    }

  return hierarchyNode;

}

//---------------------------------------------------------------------------
// Add a new visible annotation hierarchy.
// The active hierarchy node will be the parent. If there is no
// active hierarchy node, use the top-level annotation hierarchy node as the parent.
// If there is no top-level annotation hierarchy node, create additionally a top-level hierarchy node which serves as
// a parent to the new hierarchy node. Return the new hierarchy node.
//---------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode* vtkSlicerAnnotationModuleLogic::AddHierarchy()
{

  return this->AddHierarchyNodeForAnnotation(0);

}

//---------------------------------------------------------------------------
// Set the active hierarchy node which will be used as a parent for new annotations
void vtkSlicerAnnotationModuleLogic::SetActiveHierarchyNode(vtkMRMLAnnotationHierarchyNode* hierarchyNode)
{

  if (!hierarchyNode)
    {
    // there was no node as input
    // we then use the toplevel hierarchyNode
    vtkMRMLAnnotationHierarchyNode* toplevelNode = this->GetTopLevelHierarchyNode();

    if (!toplevelNode)
      {
      vtkErrorMacro("SetActiveHierarchyNodeByID: Could not find or create any hierarchy.")
      return;
      }

    this->m_ActiveHierarchy = toplevelNode;

    return;
    }

  this->m_ActiveHierarchy = hierarchyNode;
}

//---------------------------------------------------------------------------
// Set the active hierarchy node which will be used as a parent for new annotations
void vtkSlicerAnnotationModuleLogic::SetActiveHierarchyNodeByID(const char* id)
{
  vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  this->SetActiveHierarchyNode(hierarchyNode);
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
// a multiline text description and the creation of a Scene SnapShot.
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::CreateSnapShot(const char* name, const char* description, int screenshotType, vtkImageData* screenshot)
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

  vtkMRMLAnnotationSnapshotNode * newSnapshotNode = vtkMRMLAnnotationSnapshotNode::New();
  newSnapshotNode->SetScene(this->GetMRMLScene());
  if (strcmp(nameString,""))
    {
    // a name was specified
    newSnapshotNode->SetName(nameString.c_str());
    }
  else
    {
    // if no name is specified, generate a new unique one
    newSnapshotNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("Screenshot"));
    }
  newSnapshotNode->SetSnapshotDescription(description);
  newSnapshotNode->SetScreenshotType(screenshotType);
  newSnapshotNode->SetScreenshot(screenshot);
  newSnapshotNode->HideFromEditorsOff();
  this->GetMRMLScene()->AddNode(newSnapshotNode);

}

//---------------------------------------------------------------------------
// Modify an existing annotation snapShot.
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::ModifySnapShot(vtkStdString id, const char* name, const char* description, int screenshotType, vtkImageData* screenshot)
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

  vtkMRMLAnnotationSnapshotNode* snapshotNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("ModifySnapShot: Could not get snapshot node.")
    return;
    }

  vtkStdString nameString = vtkStdString(name);

  if (strcmp(nameString,""))
    {
    // a name was specified
    snapshotNode->SetName(nameString.c_str());
    }
  else
    {
    // if no name is specified, generate a new unique one
    snapshotNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("Snapshot"));
    }
  snapshotNode->SetSnapshotDescription(description);
  snapshotNode->SetScreenshotType(screenshotType);
  snapshotNode->SetScreenshot(screenshot);

  snapshotNode->Modified();
  snapshotNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, snapshotNode);

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

  vtkMRMLAnnotationSnapshotNode* snapshotNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

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

  vtkMRMLAnnotationSnapshotNode* snapshotNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("GetSnapShotScreenshotType: Could not get snapshot node!")
    return 0;
    }

  return snapshotNode->GetScreenshotType();
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

  vtkMRMLAnnotationSnapshotNode* snapshotNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("GetSnapShotScreenshot: Could not get snapshot node!")
    return 0;
    }

  return snapshotNode->GetScreenshot();
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

  vtkStdString html = vtkStdString("<tr bgcolor=#E0E0E0><td valign='middle'>");

  // level
  for (int i=0; i<level; ++i)
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
  for (int i=0; i<level; ++i)
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
    vtkImageData* image = this->GetSnapShotScreenshot(annotationNode->GetID());

    if (image)
      {

      QString tempPath = qSlicerCoreApplication::application()->temporaryPath();
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
    html += this->GetAnnotationMeasurement(annotationNode->GetID(),true);
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
