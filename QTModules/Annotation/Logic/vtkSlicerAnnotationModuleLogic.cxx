// Annotation includes
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// Annotation/MRML includes
#include "vtkMRMLAnnotationRulerNode.h"
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

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkSmartPointer.h>

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
}

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic::~vtkSlicerAnnotationModuleLogic()
{
  this->m_Widget = 0;
  this->m_LastAddedAnnotationNode = 0;
  this->m_ActiveHierarchy = 0;
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
  vtkObject * vtkNotUsed(caller), unsigned long event, void *callData)
{
  vtkDebugMacro("ProcessMRMLEvents: Event "<< event);

  vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*> (callData);

  // special case for vtkMRMLAnnotationSnapshotNodes
  vtkMRMLAnnotationSnapshotNode* snapshotNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

  if (snapshotNode)
    {

    switch (event)
      {
      case vtkMRMLScene::NodeAddedEvent:
        this->OnMRMLSceneSnapShotNodeAdded(snapshotNode);
        break;
      }

    // bail out
    return;
    }
  // end of special case for vtkMRMLAnnotationSnapshotNodes

  // handling of normal annotation MRML nodes
  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    return;
    }

  switch (event)
    {
    case vtkMRMLScene::NodeAddedEvent:
      this->OnMRMLSceneNodeAddedEvent(annotationNode);
      break;
    case vtkCommand::ModifiedEvent:
      this->OnMRMLAnnotationNodeModifiedEvent(annotationNode);
      break;

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

  vtkMRMLAnnotationHierarchyNode* hierarchyNode = this->AddNewHierarchyNode();
  if (!hierarchyNode)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: No hierarchyNode found.")
    return;
    }
  hierarchyNode->SetDisplayableNodeID(annotationNode->GetID());

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

  const char * measurementValue = this->GetAnnotationMeasurement(node->GetID(),false);
  const char* textValue = this->GetAnnotationText(node->GetID());

  this->m_Widget->updateAnnotationInTableByID(annotationNode->GetID(), measurementValue, textValue);

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
  // a good time to add the observed events!
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
  events->Delete();
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

  this->InitializeEventListeners();

  vtkMRMLInteractionNode *interactionNode =
      vtkMRMLInteractionNode::SafeDownCast(
          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (!interactionNode)
    {
    vtkErrorMacro ( "StartPlaceMode: No interaction node in the scene." );
    return;
    }

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

  this->m_Widget->addNodeToTable(annotationNode->GetID());
  this->m_LastAddedAnnotationNode = annotationNode;

}

//---------------------------------------------------------------------------
// Exit the place mode
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::StopPlaceMode()
{

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

  // Bidimensional annotation
  vtkMRMLAnnotationBidimensionalNode* annotationBidimensionalNode = vtkMRMLAnnotationBidimensionalNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationBidimensionalNode);
  annotationBidimensionalNode->Delete();

  // Fiducial annotation
  vtkMRMLAnnotationFiducialNode* annotationFiducialNode = vtkMRMLAnnotationFiducialNode::New();
  this->GetMRMLScene()->RegisterNodeClass(annotationFiducialNode);
  annotationFiducialNode->Delete();

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
    this->m_StringHolder = vtkStdString(snapshotNode->GetSnapshotDescription());
    return this->m_StringHolder;
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

  std::cout << "text" << std::endl;

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
    return snapshotNode->GetName();
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
    ss << vtkMRMLAnnotationRulerNode::SafeDownCast(annotationNode)->GetDistanceMeasurement();

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
    ss << measurement1;
    if (showUnits)
      {
      ss << " [mm]";
      }
    ss << " x ";
    ss << measurement2;
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
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationIcon: Could not get the MRML node.")
    return 0;
    }

  if (node->IsA("vtkMRMLAnnotationFiducialNode"))
    {

    return ":/Icons/AnnotationPoint.png";

    }
  else if (node->IsA("vtkMRMLAnnotationRulerNode"))
    {

    return ":/Icons/AnnotationDistance.png";

    }
  else if (node->IsA("vtkMRMLAnnotationAngleNode"))
    {

    return ":/Icons/AnnotationAngle.png";

    }
  else if (node->IsA("vtkMRMLAnnotationStickyNode"))
    {

    return ":/Icons/AnnotationNote.png";

    }
  else if (node->IsA("vtkMRMLAnnotationTextNode"))
    {

    return ":/Icons/AnnotationText.png";

    }
  else if (node->IsA("vtkMRMLAnnotationROINode"))
    {

    return ":/Icons/AnnotationROI.png";

    }
  else if (node->IsA("vtkMRMLAnnotationBidimensionalNode"))
    {

    return ":/Icons/AnnotationBidimensional.png";

    }
  else if (node->IsA("vtkMRMLAnnotationSplineNode"))
    {

    return ":/Icons/AnnotationSpline.png";

    }
  else if (node->IsA("vtkMRMLAnnotationSnapshotNode"))
    {

    return ":/Icons/ViewCamera.png";

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
//
//
// Annotation Hierarchy Functionality
//
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Return the toplevel Annotation hierarchy node or create one if there is none
//---------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode* vtkSlicerAnnotationModuleLogic::GetTopLevelHierarchyNode()
{

  vtkMRMLAnnotationHierarchyNode* toplevelNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(0,"vtkMRMLAnnotationHierarchyNode"));

  if (!toplevelNode)
    {
    // no annotation hierarchy node is currently in the scene, create a new one
    toplevelNode = vtkMRMLAnnotationHierarchyNode::New();
    toplevelNode->SetScene(this->GetMRMLScene());
    toplevelNode->HideFromEditorsOff();
    toplevelNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationToplevelHierarchyNode"));

    this->GetMRMLScene()->AddNode(toplevelNode);
    }

  return toplevelNode;
}

//---------------------------------------------------------------------------
// Add a new annotation hierarchy node under the active hierarchy node. If there is no
// active hierarchy node, use the top-level annotation hierarchy node as the parent.
// If there is no top-level annotation hierarchy node, create additionally a top-level hierarchy node which serves as
// a parent to the new hierarchy node. Return the new hierarchy node.
//---------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode* vtkSlicerAnnotationModuleLogic::AddNewHierarchyNode()
{

  if (!this->m_ActiveHierarchy)
    {
    // no active hierarchy node, this means we create the new node directly under the top-level hierarchy node
    vtkMRMLAnnotationHierarchyNode* toplevelHierarchyNode = this->GetTopLevelHierarchyNode();

    this->m_ActiveHierarchy = toplevelHierarchyNode;

    if (!toplevelHierarchyNode)
      {
      vtkErrorMacro("AddNewHierarchyNode: Toplevel hierarchy node was NULL.")
      return 0;
      }

    }

  // Create a hierarchy node
  vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::New();
  hierarchyNode->SetScene(this->GetMRMLScene());
  hierarchyNode->SetParentNodeID(this->m_ActiveHierarchy->GetID());
  hierarchyNode->SetScene(this->GetMRMLScene());
  hierarchyNode->HideFromEditorsOff();
  hierarchyNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationHierarchyNode"));

  this->GetMRMLScene()->AddNode(hierarchyNode);

  return hierarchyNode;

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
void vtkSlicerAnnotationModuleLogic::CreateSnapShot(const char* name, const char* description, vtkImageData* screenshot)
{

  if (!screenshot)
    {
    vtkErrorMacro("CreateSnapShot: No screenshot was set.")
    return;
    }

  vtkStdString nameString = vtkStdString(name);

  vtkMRMLAnnotationSnapshotNode * newSnapshotNode = vtkMRMLAnnotationSnapshotNode::New();
  newSnapshotNode->SetScene(this->GetMRMLScene());
  if (nameString)
    {
    // a name was specified
    newSnapshotNode->SetName(nameString.c_str());
    }
  else
    {
    // if no name is specified, generate a new unique one
    newSnapshotNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("AnnotationSnapshot"));
    }
  newSnapshotNode->GetSnapshotDescription(description);
  newSnapshotNode->SetScreenshot(screenshot);
  newSnapshotNode->StoreScene();
  this->GetMRMLScene()->AddNode(newSnapshotNode);
}

//---------------------------------------------------------------------------
// Convert QImage to vtkImageData
//---------------------------------------------------------------------------
bool vtkSlicerAnnotationModuleLogic::QImageToVtkImageData(const QImage& img, vtkImageData* vtkimage)
{
  int height = img.height();
  int width = img.width();
  int numcomponents = img.hasAlphaChannel() ? 4 : 3;

  vtkimage->SetWholeExtent(0, width-1, 0, height-1, 0, 0);
  vtkimage->SetSpacing(1.0, 1.0, 1.0);
  vtkimage->SetOrigin(0.0, 0.0, 0.0);
  vtkimage->SetNumberOfScalarComponents(numcomponents);
  vtkimage->SetScalarType(VTK_UNSIGNED_CHAR);
  vtkimage->SetExtent(vtkimage->GetWholeExtent());
  vtkimage->AllocateScalars();
  for(int i=0; i<height; i++)
    {
    unsigned char* row;
    row = static_cast<unsigned char*>(vtkimage->GetScalarPointer(0, height-i-1, 0));
    const QRgb* linePixels = reinterpret_cast<const QRgb*>(img.scanLine(i));
    for(int j=0; j<width; j++)
      {
      const QRgb& col = linePixels[j];
      row[j*numcomponents] = qRed(col);
      row[j*numcomponents+1] = qGreen(col);
      row[j*numcomponents+2] = qBlue(col);
      if(numcomponents == 4)
        {
        row[j*numcomponents+3] = qAlpha(col);
        }
      }
    }
  return true;
}

//---------------------------------------------------------------------------
// Restore an Annotation snapShot.
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::RestoreSnapShot(const char* id)
{
  vtkMRMLAnnotationSnapshotNode * snapshotNode = vtkMRMLAnnotationSnapshotNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(id));

  if (!snapshotNode)
    {
    vtkErrorMacro("RestoreSnapShot: Could not get snapshotNode.")
    return;
    }

  this->GetMRMLScene()->SaveStateForUndo();
  snapshotNode->RestoreScene();
}

//---------------------------------------------------------------------------
// Add snapShot node to GUI.
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::OnMRMLSceneSnapShotNodeAdded(vtkMRMLAnnotationSnapshotNode* snapshotNode)
{

  if (!this->m_Widget)
    {
    return;
    }

  if (!snapshotNode)
    {
    vtkErrorMacro("OnMRMLSceneSnapShotNodeAdded: Could not get snapshotNode.")
    return;
    }

  this->m_Widget->addNodeToTable(snapshotNode->GetID());
}
