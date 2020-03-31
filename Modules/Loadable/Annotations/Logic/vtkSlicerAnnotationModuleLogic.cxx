// Annotation includes
#include "vtkSlicerAnnotationModuleLogic.h"

// Annotation/MRML includes
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationRulerStorageNode.h"
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLAnnotationAngleStorageNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationFiducialsStorageNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationStickyNode.h"
#include "vtkMRMLAnnotationTextNode.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLAnnotationBidimensionalNode.h"
#include "vtkMRMLAnnotationSplineNode.h"
#include "vtkMRMLAnnotationSnapshotNode.h"
#include "vtkMRMLAnnotationSnapshotStorageNode.h"
#include "vtkMRMLAnnotationLinesStorageNode.h"

// MRML includes
#include <vtkMRMLFiducialListNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLUnitNode.h>

// Logic includes
#include <vtkSlicerFiducialsLogic.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPNGWriter.h>
#include <vtkVersion.h>

// STD includes
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerAnnotationModuleLogic)

//-----------------------------------------------------------------------------
// vtkSlicerAnnotationModuleLogic methods
//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic::vtkSlicerAnnotationModuleLogic()
{
  this->m_LastAddedAnnotationNode = nullptr;
  this->ActiveHierarchyNodeID = nullptr;

  this->m_MeasurementFormat = new char[8];
  sprintf(this->m_MeasurementFormat, "%s", "%.1f");

  this->m_CoordinateFormat = new char[8];
  sprintf(this->m_CoordinateFormat, "%s", "%.1f");

}

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic::~vtkSlicerAnnotationModuleLogic()
{

  if (this->m_LastAddedAnnotationNode)
    {
    this->m_LastAddedAnnotationNode = nullptr;
    }

  if (this->ActiveHierarchyNodeID)
    {
    delete [] this->ActiveHierarchyNodeID;
    this->ActiveHierarchyNodeID = nullptr;
    }

  if (this->m_MeasurementFormat)
    {
    delete[] this->m_MeasurementFormat;
    this->m_MeasurementFormat = nullptr;
    }

  if (this->m_CoordinateFormat)
    {
    delete[] this->m_CoordinateFormat;
    this->m_CoordinateFormat = nullptr;
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
// Load a fiducial list from file and make it into a set of fiducial annotations
//-----------------------------------------------------------------------------
char *vtkSlicerAnnotationModuleLogic::LoadFiducialList(const char *filename)
{
  char *nodeID = nullptr;
  std::string idList;
  if (!filename)
    {
    vtkErrorMacro("LoadFiducialList: null file name, cannot load");
    return nodeID;
    }

  // turn on batch processing
  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  // first off, load it as a fiducial list
  vtkSlicerFiducialsLogic* fiducialsLogic = vtkSlicerFiducialsLogic::New();
  fiducialsLogic->SetMRMLScene(this->GetMRMLScene());
  vtkMRMLFiducialListNode* node = fiducialsLogic->LoadFiducialList(filename);
  if (!node)
    {
    vtkErrorMacro("Unable to load fiducial list from : " << filename);
    return nodeID;
    }
  // get the list name and make a hierarchy node with that name to add the
  // fids to
  char *fidListName = node->GetName();
  vtkMRMLAnnotationHierarchyNode* fidListHierarchyNode =
      vtkMRMLAnnotationHierarchyNode::New();
  fidListHierarchyNode->HideFromEditorsOff();
  if (fidListName)
    {
    fidListHierarchyNode->SetName(fidListName);
    }
  this->GetMRMLScene()->AddNode(fidListHierarchyNode);
  // make it a child of the top level node
  fidListHierarchyNode->SetParentNodeID(this->GetTopLevelHierarchyNodeID());
  // and make it active so that the fids will be added to it
  this->SetActiveHierarchyNodeID(fidListHierarchyNode->GetID());

  // now iterate through the list and make fiducials
  int numFids = node->GetNumberOfFiducials();
  double *color = node->GetColor();
  double *selColor = node->GetSelectedColor();
  double symbolScale = node->GetSymbolScale();
  double textScale = node->GetTextScale();
  int locked = node->GetLocked();
  int glyphType = node->GetGlyphType();
  for (int n = 0; n < numFids; n++)
    {
    float *xyz = node->GetNthFiducialXYZ(n);
    int sel = node->GetNthFiducialSelected(n);
    int vis = node->GetNthFiducialVisibility(n);
    const char *labelText = node->GetNthFiducialLabelText(n);

    // now make an annotation
    vtkMRMLAnnotationFiducialNode * fnode = vtkMRMLAnnotationFiducialNode::New();
    fnode->SetName(labelText);
    double coord[3] = {(double)xyz[0], (double)xyz[1], (double)xyz[2]};
    fnode->AddControlPoint(coord, sel, vis);
    fnode->SetSelected(sel);
    fnode->SetLocked(locked);

    this->GetMRMLScene()->AddNode(fnode);
    if (n != 0)
      {
      idList += std::string(",");
      }
    idList += std::string(fnode->GetID());
    fnode->CreateAnnotationTextDisplayNode();
    fnode->CreateAnnotationPointDisplayNode();
    fnode->SetTextScale(textScale);
    fnode->GetAnnotationPointDisplayNode()->SetGlyphScale(symbolScale);
    fnode->GetAnnotationPointDisplayNode()->SetGlyphType(glyphType);
    fnode->GetAnnotationPointDisplayNode()->SetColor(color);
    fnode->GetAnnotationPointDisplayNode()->SetSelectedColor(selColor);
    fnode->GetAnnotationTextDisplayNode()->SetColor(color);
    fnode->GetAnnotationTextDisplayNode()->SetSelectedColor(selColor);
    fnode->SetDisplayVisibility(vis);
    fnode->Delete();
    }
  // clean up
  fidListHierarchyNode->Delete();
  // remove the legacy node
  this->GetMRMLScene()->RemoveNode(node->GetStorageNode());
  this->GetMRMLScene()->RemoveNode(node);

  // turn off batch processing
  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);

  if (idList.length())
    {
    nodeID = (char *)malloc(sizeof(char) * (idList.length() + 1));
    strcpy(nodeID, idList.c_str());
    }
  return nodeID;
}

//-----------------------------------------------------------------------------
// Load an annotation from file
//-----------------------------------------------------------------------------
char *vtkSlicerAnnotationModuleLogic::LoadAnnotation(const char *filename, const char *name, int fileType)
{
  char *nodeID = nullptr;
  if (!filename)
    {
    vtkErrorMacro("LoadAnnotation: null filename, cannot load");
    return nodeID;
    }
  vtkDebugMacro("LoadAnnotation: filename = " << filename << ", fileType = " << fileType);
//  std::cout << "LoadAnnotation: filename = " << filename << ", fileType = " << fileType << std::endl;

  // turn on batch processing
  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  if (fileType == this->Fiducial)
    {
    vtkNew<vtkMRMLAnnotationFiducialsStorageNode> fStorageNode;
    vtkNew<vtkMRMLAnnotationFiducialNode> fnode;
    fnode->SetName(name);

    fStorageNode->SetFileName(filename);

    // add the storage node to the scene
    this->GetMRMLScene()->AddNode(fStorageNode.GetPointer());
    fnode->SetScene(this->GetMRMLScene());

    this->GetMRMLScene()->AddNode(fnode.GetPointer());
    fnode->SetAndObserveStorageNodeID(fStorageNode->GetID());

    if (fStorageNode->ReadData(fnode.GetPointer()))
      {
      vtkDebugMacro("LoadAnnotation: fiducial storage node read " << filename);
      nodeID =  fnode->GetID();
      }
    }
  else if (fileType == this->Ruler)
    {
    vtkNew<vtkMRMLAnnotationRulerStorageNode> rStorageNode;
    vtkNew<vtkMRMLAnnotationRulerNode> rNode;
    rNode->SetName(name);

    rStorageNode->SetFileName(filename);

    // add to the scene
    this->GetMRMLScene()->AddNode(rStorageNode.GetPointer());
    rNode->Initialize(this->GetMRMLScene());
    rNode->SetAndObserveStorageNodeID(rStorageNode->GetID());

    if (rStorageNode->ReadData(rNode.GetPointer()))
      {
      vtkDebugMacro("LoadAnnotation: ruler storage node read " << filename);
      nodeID = rNode->GetID();
      }
    }
  else if (fileType == this->ROI)
    {
    vtkNew<vtkMRMLAnnotationLinesStorageNode> roiStorageNode;
    vtkNew<vtkMRMLAnnotationROINode> roiNode;
    roiNode->SetName(name);

    roiStorageNode->SetFileName(filename);

    // add the storage node to the scene
    this->GetMRMLScene()->AddNode(roiStorageNode.GetPointer());
    roiNode->Initialize(this->GetMRMLScene());
    roiNode->SetAndObserveStorageNodeID(roiStorageNode->GetID());

    if (roiStorageNode->ReadData(roiNode.GetPointer()))
      {
      vtkDebugMacro("LoadAnnotation: fiducial storage node read " << filename);
      nodeID =  roiNode->GetID();
      }
    }
  else
    {
    vtkErrorMacro("LoadAnnotation: unknown file type " << fileType << ", cannot read " << filename);
    }
  // turn off batch processing
  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);

  return nodeID;
}

//-----------------------------------------------------------------------------
char *vtkSlicerAnnotationModuleLogic::AddFiducial(double r, double a, double s,
                                                  const char *label)
{
  char *nodeID = nullptr;
  vtkNew<vtkMRMLAnnotationFiducialNode> fnode;

  if (label != nullptr)
    {
    fnode->SetName(label);
    }
  fnode->SetFiducialCoordinates(r, a, s);
  fnode->Initialize(this->GetMRMLScene());

  nodeID = fnode->GetID();

  return nodeID;
}

//-----------------------------------------------------------------------------
//
//
// MRML event handling
//
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::ProcessMRMLNodesEvents(vtkObject *vtkNotUsed(caller),
                                                            unsigned long event,
                                                            void *callData)
{
  vtkDebugMacro("ProcessMRMLNodesEvents: Event "<< event);

  vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*> (callData);

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);
  if (annotationNode)
    {
    switch (event)
      {
      case vtkMRMLScene::NodeAddedEvent:
        this->OnMRMLSceneNodeAdded(annotationNode);
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
void vtkSlicerAnnotationModuleLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLSceneNodeAddedEvent");
  // don't respond if the scene is importing as the nodes will have hierarchy
  // nodes already defined
  if (this->GetMRMLScene() &&
      (this->GetMRMLScene()->IsImporting() ||
       this->GetMRMLScene()->IsRestoring()))
    {
    return;
    }

  vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);
  if (!annotationNode)
    {
    return;
    }

  // check for missing display nodes (if we're not in batch processing mode)
  if (this->GetMRMLScene() &&
      !this->GetMRMLScene()->IsBatchProcessing())
    {
    // check if no display nodes have been added already via calls to Initialize
    if (annotationNode->GetDisplayNode() == nullptr)
      {
      // keep it down to one modify event from the node (will be node added
      // events from the new nodes)
      int modifyFlag = annotationNode->StartModify();
      vtkDebugMacro("OnMRMLSceneNodeAddedEvent: adding display nodes for " << annotationNode->GetName());
      if (vtkMRMLAnnotationLinesNode::SafeDownCast(annotationNode))
        {
        vtkMRMLAnnotationLinesNode::SafeDownCast(annotationNode)->CreateAnnotationLineDisplayNode();
        }
      if (vtkMRMLAnnotationControlPointsNode::SafeDownCast(annotationNode))
        {
        vtkMRMLAnnotationControlPointsNode::SafeDownCast(annotationNode)->CreateAnnotationPointDisplayNode();
        }
      annotationNode->CreateAnnotationTextDisplayNode();
      annotationNode->EndModify(modifyFlag);
      }
    }

  // set up the hierarchy for the new annotation node if necessary
  bool retval = this->AddHierarchyNodeForAnnotation(annotationNode);
  if (!retval)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: No hierarchyNode added.");
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

  this->InvokeEvent(RefreshRequestEvent);
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::OnMRMLSceneEndClose()
{
  if (this->m_LastAddedAnnotationNode)
    {
    this->m_LastAddedAnnotationNode = nullptr;
    }

  if (this->GetActiveHierarchyNodeID())
    {
    this->SetActiveHierarchyNodeID(nullptr);
    }
}

//---------------------------------------------------------------------------
//
//
// Placement of Annotations
//
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Set the internal mrml scene and observe events on it
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  // a good time to add the observed events!
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
//  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLScene::EndCloseEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::ObserveMRMLScene()
{
  // add known annotation types to the selection node
  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
  if (selectionNode)
    {
    // got into batch mode
    this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

    vtkDebugMacro("vtkSlicerAnnotationModuleLogic::ObserveMRMLScene(): adding new annotation class names to selection node place list");
    /// Markups handle placement of new fiducials
    // selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationFiducialNode", ":/Icons/AnnotationPointWithArrow.png", "Fiducial");
//    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationTextNode",  ":/Icons/AnnotationTextWithArrow.png", "Text");
    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationRulerNode", ":/Icons/AnnotationDistanceWithArrow.png", "Ruler");
//    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationBidimensionalNode", ":/Icons/AnnotationBidimensionalWithArrow.png", "Bidimensional");
    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationROINode", ":/Icons/AnnotationROIWithArrow.png", "ROI");
//    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationAngleNode", ":/Icons/AnnotationAngle.png", "Angle");
//    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationStickyNode", "", "Sticky");
//    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationSplineNode", ":/Icons/AnnotationSpline.png", "Spline");

    // stop batch add
    this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
    }
  // Superclass::ObserveMRMLScene calls UpdateFromMRMLScene();
  this->Superclass::ObserveMRMLScene();
}

//-----------------------------------------------------------------------------
// Add Annotation Node
//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::AddAnnotationNode(const char * nodeDescriptor, bool persistent)
{

  vtkMRMLSelectionNode *selectionNode = nullptr;
  if (this->GetMRMLScene())
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
    }
  if (!selectionNode)
    {
    vtkErrorMacro("AddAnnotationNode: No selection node in the scene.");
    return;
    }

  selectionNode->SetActivePlaceNodeClassName(nodeDescriptor);

  this->StartPlaceMode(persistent);

}

//---------------------------------------------------------------------------
// Start the place mouse mode
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::StartPlaceMode(bool persistent, vtkMRMLInteractionNode* interactionNode)
{
  if (!interactionNode && this->GetMRMLScene())
    {
    interactionNode = vtkMRMLInteractionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
    }
  if (!interactionNode)
    {
    vtkErrorMacro ( "StartPlaceMode: No interaction node in the scene." );
    return;
    }

  interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);

  interactionNode->SetPlaceModePersistence(persistent ? 1 : 0);

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

  this->InvokeEvent(RefreshRequestEvent);

  this->m_LastAddedAnnotationNode = annotationNode;

}

//---------------------------------------------------------------------------
// Exit the place mode
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::StopPlaceMode(bool persistent, vtkMRMLInteractionNode* interactionNode)
{

  vtkMRMLSelectionNode *selectionNode = nullptr;
  if (this->GetMRMLScene())
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
    }
  if (!selectionNode)
    {
    vtkErrorMacro("StopPlaceMode: No selection node in the scene.");
    return;
    }

  if (!interactionNode && this->GetMRMLScene())
    {
    interactionNode = vtkMRMLInteractionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
    }
  if (interactionNode == nullptr)
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
  selectionNode->SetActivePlaceNodeClassName("");
}

//---------------------------------------------------------------------------
// Cancel the current placement or remove the last placed node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::CancelCurrentOrRemoveLastAddedAnnotationNode(vtkMRMLInteractionNode* interactionNode)
{
  if (!interactionNode && this->GetMRMLScene())
    {
    interactionNode = vtkMRMLInteractionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
    }
  if (!interactionNode)
    {
    vtkErrorMacro("CancelCurrentOrRemoveLastAddedAnnotationNode: No interaction node");
    return;
    }

  interactionNode->InvokeEvent(vtkMRMLInteractionNode::EndPlacementEvent);

}

//---------------------------------------------------------------------------
/// Remove an AnnotationNode and also its 1-1 IS-A hierarchyNode, if found.
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::RemoveAnnotationNode(vtkMRMLAnnotationNode* annotationNode)
{
  if (!annotationNode)
    {
    vtkErrorMacro("RemoveAnnotationNode: no node to remove.");
    return;
    }
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("RemoveAnnotationNode: No MRML Scene found.");
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

  vtkMRMLScene *scene = this->GetMRMLScene();

  //
  // The core nodes
  //

  // base nodes
  vtkNew<vtkMRMLAnnotationNode> annotationNode;
  scene->RegisterNodeClass(annotationNode.GetPointer());

  vtkNew<vtkMRMLAnnotationDisplayNode> annotationDisplayNode;
  scene->RegisterNodeClass(annotationDisplayNode.GetPointer());

  vtkNew<vtkMRMLAnnotationStorageNode> annotationStorageNode;
  scene->RegisterNodeClass(annotationStorageNode.GetPointer());

  // Control Points
  vtkNew<vtkMRMLAnnotationControlPointsNode> annotationControlPointsNode;
  scene->RegisterNodeClass(annotationControlPointsNode.GetPointer());

  vtkNew<vtkMRMLAnnotationControlPointsStorageNode> annotationControlPointsStorageNode;
  scene->RegisterNodeClass(annotationControlPointsStorageNode.GetPointer());

  vtkNew<vtkMRMLAnnotationPointDisplayNode> annotationControlPointsDisplayNode;
  scene->RegisterNodeClass(annotationControlPointsDisplayNode.GetPointer());

  // Lines
  vtkNew<vtkMRMLAnnotationLinesNode> annotationLinesNode;
  scene->RegisterNodeClass(annotationLinesNode.GetPointer());

  vtkNew<vtkMRMLAnnotationLinesStorageNode> annotationLinesStorageNode;
  scene->RegisterNodeClass(annotationLinesStorageNode.GetPointer());

  vtkNew<vtkMRMLAnnotationLineDisplayNode> annotationLinesDisplayNode;
  scene->RegisterNodeClass(annotationLinesDisplayNode.GetPointer());

  // Text
  vtkNew<vtkMRMLAnnotationTextDisplayNode> annotationTextDisplayNode;
  scene->RegisterNodeClass(annotationTextDisplayNode.GetPointer());

  //
  // Now the actual Annotation tool nodes
  //

  // Snapshot annotation
  vtkNew<vtkMRMLAnnotationSnapshotNode> annotationSnapshotNode;
  scene->RegisterNodeClass(annotationSnapshotNode.GetPointer());

  vtkNew<vtkMRMLAnnotationSnapshotStorageNode> annotationSnapshotStorageNode;
  scene->RegisterNodeClass(annotationSnapshotStorageNode.GetPointer());

  // Text annotation
  vtkNew<vtkMRMLAnnotationTextNode> annotationTextNode;
  scene->RegisterNodeClass(annotationTextNode.GetPointer());

  // Ruler annotation
  vtkNew<vtkMRMLAnnotationRulerNode> annotationRulerNode;
  scene->RegisterNodeClass(annotationRulerNode.GetPointer());

  vtkNew<vtkMRMLAnnotationRulerStorageNode> annotationRulerStorageNode;
  scene->RegisterNodeClass(annotationRulerStorageNode.GetPointer());

  // ROI annotation
  vtkNew<vtkMRMLAnnotationROINode> annotationROINode;
  scene->RegisterNodeClass(annotationROINode.GetPointer());
  // ROI annotation backwards compatibility
#if MRML_SUPPORT_VERSION < 0x040000
  scene->RegisterNodeClass(annotationROINode.GetPointer(), "ROI");
#endif

  // Bidimensional annotation
  vtkNew<vtkMRMLAnnotationBidimensionalNode> annotationBidimensionalNode;
  scene->RegisterNodeClass(annotationBidimensionalNode.GetPointer());

  // Fiducial annotation
  vtkNew<vtkMRMLAnnotationFiducialNode> annotationFiducialNode;
  scene->RegisterNodeClass(annotationFiducialNode.GetPointer());

  vtkNew<vtkMRMLAnnotationFiducialsStorageNode> annotationFiducialsStorageNode;
  scene->RegisterNodeClass(annotationFiducialsStorageNode.GetPointer());

  //
  // Annotation hierarchies
  //
  vtkNew<vtkMRMLAnnotationHierarchyNode> annotationHierarchyNode;
  scene->RegisterNodeClass(annotationHierarchyNode.GetPointer());
}

//---------------------------------------------------------------------------
// Check if the id points to an annotation node
//---------------------------------------------------------------------------
bool vtkSlicerAnnotationModuleLogic::IsAnnotationNode(const char* id)
{
  if (!id ||
      !this->GetMRMLScene())
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
// Check if the id points to an annotation hierarchy node
//---------------------------------------------------------------------------
bool vtkSlicerAnnotationModuleLogic::IsAnnotationHierarchyNode(const char* id)
{
  if (!id || !this->GetMRMLScene())
    {
    return false;
    }

  vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(id));

  if (hierarchyNode)
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
    return nullptr;
    }

  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationName: Could not get the MRML node with id " << id);
    return nullptr;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);

  if (annotationNode)
    {
    return annotationNode->GetName();
    }

  vtkMRMLAnnotationHierarchyNode *hierarchyNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
  if (hierarchyNode)
    {
    return hierarchyNode->GetName();
    }
  return nullptr;
}

//---------------------------------------------------------------------------
// Return the text of an annotation MRML Node
//---------------------------------------------------------------------------
vtkStdString vtkSlicerAnnotationModuleLogic::GetAnnotationText(const char* id)
{
  if (!id)
    {
    vtkErrorMacro("GetAnnotationText: no id supplied");
    return "";
    }

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("GetAnnotationText: no mrml scene.");
    return "";
    }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetAnnotationText: Could not get the MRML node with id " << id);
    return "";
    }

  // special case for annotation snapShots
  vtkMRMLAnnotationSnapshotNode* snapshotNode =
      vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);
  if (snapshotNode)
    {
    return snapshotNode->GetSnapshotDescription();
    }
  // end of special case for annotation snapShots

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationText: Could not get the annotationMRML node.");
    return "";
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

  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationText: Could not get the MRML node with id " << id);
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
    vtkErrorMacro("SetAnnotationText: Could not get the annotationMRML node.");
    return;
    }

  if (!newtext)
    {
    vtkErrorMacro("SetAnnotationText: No text supplied, using an empty string.");
    annotationNode->SetText(0, "", 1, 1);
    return;
    }

  annotationNode->SetText(0, newtext, 1, 1);
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

  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the MRML node with id " << id);
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextScale: Could not get the annotation MRML node.");
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
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }
  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the MRML node with id " << id);
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextScale: Could not get the annotation MRML node.");
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
    return nullptr;
    }
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextSelectedColor: Could not get the MRML node with id " << id);
    return nullptr;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextSelectedColor: Could not get the annotation MRML node.");
    return nullptr;
    }

  if (!annotationNode->GetAnnotationTextDisplayNode())
    {
    return nullptr;
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
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextSelectedColor: Could not get the MRML node with id " << id);
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextSelectedColor: Could not get the annotation MRML node.");
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
    return nullptr;
    }
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationTextUnselectedColor: Could not get the MRML node with id " << id);
    return nullptr;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationTextUnselectedColor: Could not get the annotation MRML node.");
    return nullptr;
    }

  if (!annotationNode->GetAnnotationTextDisplayNode())
    {
    return nullptr;
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
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("SetAnnotationTextUnselectedColor: Could not get the MRML scene");
    return;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationTextUnselectedColor: Could not get the MRML node with id " << id);
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationTextUnselectedColor: Could not get the annotation MRML node with id " << id);
    return;
    }

  if (!annotationNode->GetAnnotationTextDisplayNode())
    {
    vtkErrorMacro("SetAnnotationTextUnselectedColor: Could not get the text display node for the annotation MRML node with id " << id);
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
    return nullptr;
    }
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationColor: Could not get the MRML node for id " << id);
    return nullptr;
    }

  vtkMRMLDisplayableNode* annotationNode =
      vtkMRMLDisplayableNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationColor: Could not get the displayable MRML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationColor: Could not get the display node for node " << id);
    return nullptr;
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

  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

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

  if (annotationNode->GetDisplayNode() == nullptr)
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
    return nullptr;
    }
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationUnselectedColor: Could not get the MRML node for id " << id);
    return nullptr;
    }

  vtkMRMLDisplayableNode* annotationNode =
      vtkMRMLDisplayableNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationUnselectedColor: Could not get the displayable MRML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationUnselectedColor: Could not get the display node for node " << id);
    return nullptr;
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

  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

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

  if (annotationNode->GetDisplayNode() == nullptr)
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
    return nullptr;
    }
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationPointColor: Could not get the MRML node for id " << id);
    return nullptr;
    }

  vtkMRMLAnnotationControlPointsNode* annotationNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationPointColor: Could not get the displayable control points MRML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationPointColor: Could not get the point display node for node " << id);
    return nullptr;
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

  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

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

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
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
    return nullptr;
    }
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationPointUnselectedColor: Could not get the MRML node for id " << id);
    return nullptr;
    }

  vtkMRMLAnnotationControlPointsNode* annotationNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationPointUnselectedColor: Could not get the displayable control points MRML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationPointUnselectedColor: Could not get the point display node for node " << id);
    return nullptr;
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

  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

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

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationPointUnselectedColor: Could not get the point display node for node " << id);
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
    return nullptr;
    }

  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the MRML node for id " << id);
    return nullptr;
    }

  vtkMRMLAnnotationControlPointsNode* annotationNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the displayable control points MRML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the point display node for node " << id);
    return nullptr;
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
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the MRML node for id " << id);
    return 0;
    }

  vtkMRMLAnnotationControlPointsNode* annotationNode =
      vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationPointGlyphTypeAsString: Could not get the displayable control points MRML node for id " << id);
    return 0;
    }

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationPointGlyphType: Could not get the point display node for node " << id);
    return 0;
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

  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

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

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationPointGlyphTypeFromString: Could not get the point display node for node " << id);
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
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

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

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationPointGlyphType: Could not get the point display node for node " << id);
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
    return nullptr;
    }
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }
  if (!node)
    {
    vtkErrorMacro("GetAnnotationLineColor: Could not get the MRML node for id " << id);
    return nullptr;
    }

  vtkMRMLAnnotationLinesNode* annotationNode =
      vtkMRMLAnnotationLinesNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationLineColor: Could not get the displayable control points MRML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetAnnotationLineDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationLineColor: Could not get the line display node for node " << id);
    return nullptr;
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

  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

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

  if (annotationNode->GetAnnotationPointDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationLineColor: Could not get the point display node for node " << id);
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
    return nullptr;
    }
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationLineUnselectedColor: Could not get the MRML node for id " << id);
    return nullptr;
    }

  vtkMRMLAnnotationLinesNode* annotationNode =
      vtkMRMLAnnotationLinesNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationLineUnselectedColor: Could not get the displayable control points MRML node for id " << id);
    return nullptr;
    }

  if (annotationNode->GetAnnotationLineDisplayNode() == nullptr)
    {
    vtkErrorMacro("GetAnnotationLineUnselectedColor: Could not get the line display node for node " << id);
    return nullptr;
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

  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

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

  if (annotationNode->GetAnnotationLineDisplayNode() == nullptr)
    {
    vtkErrorMacro("SetAnnotationLineUnselectedColor: Could not get the line display node for node " << id);
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
    return nullptr;
    }
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("GetAnnotationMeasurement: Could not get the MRML node with id " << id);
    return nullptr;
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
  vtkMRMLSelectionNode* selectionNode =  vtkMRMLSelectionNode::SafeDownCast(
    this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationMeasurement: Could not get the annotation MRML node.");
    return nullptr;
    }

  if (node->IsA("vtkMRMLAnnotationRulerNode"))
    {
    double length = vtkMRMLAnnotationRulerNode::SafeDownCast(annotationNode)->GetDistanceMeasurement();
    char string[512];
    sprintf(string, this->m_MeasurementFormat,length);

    std::string unit = string;
    if (showUnits)
      {
      vtkMRMLUnitNode* lengthUnit = selectionNode ? selectionNode->GetUnitNode("length") : nullptr;
      if (lengthUnit)
        {
        unit = lengthUnit->GetDisplayStringFromValue(length);
        }
      else
        {
        unit += " mm";
        }
      }
    std::ostringstream ss;
    ss << unit;

    this->m_StringHolder = ss.str();
    }
  else if (node->IsA("vtkMRMLAnnotationAngleNode"))
    {
    double angle = vtkMRMLAnnotationAngleNode::SafeDownCast(annotationNode)->GetAngleMeasurement();
    std::ostringstream ss;
    ss << angle;
    std::string measurement = ss.str();
    if (showUnits)
      {
      // Get Unit from node
      vtkMRMLUnitNode* angleUnit = selectionNode ? selectionNode->GetUnitNode("angle") : nullptr;
      if (angleUnit)
        {
        measurement = angleUnit->GetDisplayStringFromValue(angle);
        }
      else
        {
        measurement += " degrees";
        }
      }

    this->m_StringHolder = measurement;
    }
  else if (node->IsA("vtkMRMLAnnotationFiducialNode"))
    {
    std::ostringstream ss;
    double
        * tmpPtr =
            vtkMRMLAnnotationFiducialNode::SafeDownCast(annotationNode)->GetControlPointCoordinates(
                0);
    if (tmpPtr)
      {
      double coordinates[3] = { tmpPtr[0], tmpPtr[1], tmpPtr[2] };

      char string[512];
      sprintf(string, this->m_CoordinateFormat, coordinates[0]);
      char string2[512];
      sprintf(string2, this->m_CoordinateFormat, coordinates[1]);
      char string3[512];
      sprintf(string3, this->m_CoordinateFormat, coordinates[2]);

      ss << string << ", " << string2 << ", " << string3;

      this->m_StringHolder = ss.str();
      }
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
    double length1 = std::max(measurement1, measurement2);
    double length2 = std::min(measurement1, measurement2);

    char string[512];
    sprintf(string, this->m_MeasurementFormat, length1);

    std::string unit = string;
    if (showUnits)
      {
      vtkMRMLUnitNode* lengthUnit = selectionNode ? selectionNode->GetUnitNode("length") : nullptr;
      if (lengthUnit)
        {
        unit = lengthUnit->GetDisplayStringFromValue(length1);
        }
      else
        {
        unit += " mm";
        }
      }
    ss << unit << " x ";

    char string2[512];
    sprintf(string2, this->m_MeasurementFormat, length2);

    unit = string2;
    if (showUnits)
      {
      vtkMRMLUnitNode* lengthUnit = selectionNode ? selectionNode->GetUnitNode("length") : nullptr;
      if (lengthUnit)
        {
        unit = lengthUnit->GetDisplayStringFromValue(length2);
        }
      else
        {
        unit += " mm";
        }
      }
    ss << unit;

    this->m_StringHolder = ss.str();
    }

  return this->m_StringHolder.c_str();

}

//---------------------------------------------------------------------------
// Return the icon of an annotation MRML Node
//---------------------------------------------------------------------------
const char * vtkSlicerAnnotationModuleLogic::GetAnnotationIcon(const char* id)
{
  if (!this->GetMRMLScene() || id == nullptr)
    {
    return nullptr;
    }
  vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(id);
  return this->GetAnnotationIcon(mrmlNode);
}

//---------------------------------------------------------------------------
// Return the icon of an annotation MRML Node
//---------------------------------------------------------------------------
const char * vtkSlicerAnnotationModuleLogic
::GetAnnotationIcon(vtkMRMLNode* mrmlNode)
{
  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
    mrmlNode);

  if (annotationNode)
    {
    return annotationNode->GetIcon();
    }

  vtkMRMLAnnotationHierarchyNode* hierarchyNode =
    vtkMRMLAnnotationHierarchyNode::SafeDownCast(mrmlNode);

  if (hierarchyNode)
    {
    return hierarchyNode->GetIcon();
    }

  vtkMRMLAnnotationSnapshotNode* snapshotNode =
    vtkMRMLAnnotationSnapshotNode::SafeDownCast(mrmlNode);

  if (snapshotNode)
    {
    return snapshotNode->GetIcon();
    }

  return nullptr;
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
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }
  if (!node)
    {
    vtkErrorMacro("GetAnnotationLockedUnlocked: Could not get the MRML node with id " << id);
    return 0;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("GetAnnotationLockedUnlocked: Could not get the annotationMRML node.");
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
  vtkMRMLNode* node = nullptr;
  if (this->GetMRMLScene())
    {
    node = this->GetMRMLScene()->GetNodeByID(id);
    }

  if (!node)
    {
    vtkErrorMacro("SetAnnotationLockedUnlocked: Could not get the MRML node with id " << id);
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationLockedUnlocked: Could not get the annotationMRML node.");
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
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
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

  if (annotationNode)
    {
    return annotationNode->GetDisplayVisibility();
    }

  // is it a hierarchy node?
  vtkMRMLAnnotationHierarchyNode *hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
  if (hnode && hnode->GetDisplayNode())
    {
    return hnode->GetDisplayNode()->GetVisibility();
    }

  return 0;

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
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationVisibility: Could not get the MRML node with id " << id);
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);

  if (annotationNode)
    {
    // show/hide this annotation
    annotationNode->SetDisplayVisibility(!annotationNode->GetDisplayVisibility());
    return;
    }

  // or it might be a hierarchy node
  vtkMRMLAnnotationHierarchyNode *hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
  if (hnode && hnode->GetDisplayNode())
    {
    hnode->GetDisplayNode()->SetVisibility(!hnode->GetDisplayNode()->GetVisibility());
    }
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
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("SetAnnotationSelected: Could not get the MRML node with id " << id);
    return;
    }

  // special case for snapshot and hierarchy nodes
  if (node->IsA("vtkMRMLAnnotationSnapshotNode") ||
      node->IsA("vtkMRMLAnnotationHierarchyNode"))
    {
    // directly bail out
    return;
    }
  // end of special case

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(
      node);

  if (!annotationNode)
    {
    vtkErrorMacro("SetAnnotationSelected: Could not get the annotationMRML node.");
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
  if (this->GetMRMLScene() == nullptr)
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
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
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
    if (!node->IsA("vtkMRMLAnnotationHierarchyNode"))
      {
      vtkErrorMacro("BackupAnnotationNode: Could not get the annotationMRML node.");
      }
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
      vtkErrorMacro("BackupAnnotationNode: Could not get the annotationMRMLDisplay node number " << i << " with ID:" << annotationNode->GetID());
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

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return;
    }

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
    if (!node->IsA("vtkMRMLAnnotationHierarchyNode"))
      {
      vtkErrorMacro("RestoreAnnotationNode: Could not get the annotationMRML node.");
      }
    return;
    }

  for (int i = 0; i < annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkMRMLAnnotationDisplayNode * displayNode =
        vtkMRMLAnnotationDisplayNode::SafeDownCast(
            annotationNode->GetNthDisplayNode(i));

    if (!displayNode)
      {
      vtkErrorMacro("RestoreAnnotationNode: Could not get the annotationMRMLDisplay node:" << displayNode);
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

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return;
    }
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
    if (!node->IsA("vtkMRMLAnnotationHierarchyNode"))
      {
      vtkErrorMacro("DeleteBackupNodes: Could not get the annotationMRML node.");
      }
    return;
    }

  for (int i = 0; i < annotationNode->GetNumberOfDisplayNodes(); i++)
    {

    vtkMRMLAnnotationDisplayNode * displayNode =
        vtkMRMLAnnotationDisplayNode::SafeDownCast(
            annotationNode->GetNthDisplayNode(i));

    if (!displayNode)
      {
      vtkErrorMacro("DeleteBackupNodes: Could not get the annotationMRMLDisplay node.");
      }
    else
      {
      displayNode->ClearBackup();
      }

    } // end of displayNodes

  annotationNode->ClearBackup();

}

//---------------------------------------------------------------------------
// Jump the 2d slices to the first control point location of an AnnotationMRML node
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::JumpSlicesToAnnotationCoordinate(const char * id)
{
  if (!id)
    {
    vtkErrorMacro("JumpSlicesToAnnotationCoordinate: no id given");
    return;
    }
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("JumpSlicesToAnnotationCoordinate: Could not get the MRML node with id " << id);
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);

  if (!annotationNode)
    {
    vtkErrorMacro("JumpSlicesToAnnotationCoordinate: Could not get the annotationMRML node.");
      return;
    }

  // do not restore anything if this is a snapshot node
  if (annotationNode->IsA("vtkMRMLAnnotationSnapshotNode"))
    {
    return;
    }

  vtkMRMLAnnotationControlPointsNode* controlpointsNode = vtkMRMLAnnotationControlPointsNode::SafeDownCast(annotationNode);

  if (!controlpointsNode)
    {
    // we don't have a controlpointsNode so we can not jump the slices
    return;
    }

  // TODO for now only consider the first control point
  double *rasCoordinates = controlpointsNode->GetControlPointCoordinates(0);
  if (rasCoordinates)
    {
    double r = rasCoordinates[0];
    double a = rasCoordinates[1];
    double s = rasCoordinates[2];

    vtkMRMLSliceNode::JumpAllSlices(this->GetMRMLScene(), r, a, s);
    }
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
      vtkErrorMacro("No scene set.");
      return this->m_StringHolder.c_str();
      }

    vtkMRMLAnnotationNode* annotationNode =
        vtkMRMLAnnotationNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(
            id));

    if (!annotationNode)
      {
      vtkErrorMacro("MoveAnnotationUp: Could not get annotation node!");
      return this->m_StringHolder.c_str();
      }

    // get the corresponding hierarchy
    vtkMRMLAnnotationHierarchyNode* hNode =
        vtkMRMLAnnotationHierarchyNode::SafeDownCast(
            vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
                this->GetMRMLScene(), annotationNode->GetID()));

    if (!hNode)
      {
      vtkErrorMacro("MoveAnnotationUp: Could not get hierarchy node!");
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
      vtkErrorMacro("MoveAnnotationDown: No scene set.");
      return this->m_StringHolder.c_str();
      }

    vtkMRMLAnnotationNode* annotationNode =
        vtkMRMLAnnotationNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(
            id));

    if (!annotationNode)
      {
      vtkErrorMacro("MoveAnnotationDown: Could not get annotation node!");
      return this->m_StringHolder.c_str();
      }

    // get the corrsponding hierarchy
    vtkMRMLAnnotationHierarchyNode* hNode =
        vtkMRMLAnnotationHierarchyNode::SafeDownCast(
            vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
                this->GetMRMLScene(), annotationNode->GetID()));

    if (!hNode)
      {
      vtkErrorMacro("MoveAnnotationDown: Could not get hierarchy node!");
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
  if (this->GetMRMLScene() == nullptr)
    {
    return nullptr;
    }
  const char *topLevelName = "All Annotations";
  char *toplevelNodeID = nullptr;
  vtkCollection *col = this->GetMRMLScene()->GetNodesByClass("vtkMRMLAnnotationHierarchyNode");
  vtkMRMLAnnotationHierarchyNode *toplevelNode = nullptr;
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
    if (this->AddDisplayNodeForHierarchyNode(toplevelNode) == nullptr)
      {
      vtkErrorMacro("GetTopLevelHierarchyNodeID: error adding a display node for new top level node " << toplevelNodeID);
      }
    this->InvokeEvent(HierarchyNodeAddedEvent, toplevelNode);
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
    return nullptr;
    }
  if (this->GetMRMLScene() == nullptr)
    {
    return nullptr;
    }

  // get the set of hierarchy nodes to check through
  vtkCollection *col = nullptr;
  bool topLevelAnnotationIsActive = false;
  if (this->GetActiveHierarchyNode() && this->GetActiveHierarchyNode()->GetID() &&
      this->GetTopLevelHierarchyNodeID() &&
      !strcmp(this->GetActiveHierarchyNode()->GetID(), this->GetTopLevelHierarchyNodeID()))
    {
    topLevelAnnotationIsActive = true;
    }

  if (!topLevelAnnotationIsActive)
    {
    // just use the currently active annotation hierarchy if it exists
    if (this->GetActiveHierarchyNode())
      {
      return this->GetActiveHierarchyNode()->GetID();
      }
    }

  // find the per list annotation hierarchy nodes at the top level
  // look for any annotation hierarchy nodes in the scene
  col = this->GetMRMLScene()->GetNodesByClass("vtkMRMLAnnotationHierarchyNode");
  unsigned int numNodes = 0;
  if (col)
    {
    numNodes = col->GetNumberOfItems();
    }
  // iterate through the hierarchy nodes to find one with an attribute matching the input node's classname
  vtkMRMLAnnotationHierarchyNode *toplevelNode = nullptr;
  char *toplevelNodeID = nullptr;
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
    // make it a child of the top annotation hierarchy
    toplevelNode->SetParentNodeID(this->GetTopLevelHierarchyNodeID());
    this->GetMRMLScene()->AddNode(toplevelNode);
    this->InvokeEvent(HierarchyNodeAddedEvent, toplevelNode);
    toplevelNodeID = toplevelNode->GetID();
    if (this->AddDisplayNodeForHierarchyNode(toplevelNode) == nullptr)
      {
      vtkErrorMacro("GetTopLevelHierarchyNodeIDForNodeClass: error adding a display node for hierarchy node " << toplevelNodeID);
      }
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
      if (hnode != nullptr)
        {
        vtkMRMLAnnotationHierarchyNode *ahnode =
            vtkMRMLAnnotationHierarchyNode::SafeDownCast(hnode);
        if (ahnode != nullptr)
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
      if (annotationNode)
        {
        vtkErrorMacro("AddHierarchyNodeForAnnotation: annotation node has no scene or id, not checking for existing hierarchy node");
        }
      }
  // is there an associated node?
/*  if (annotationNode->GetAttribute("AssociatedNodeID"))
    {
    // add a hierarchy for that node
    // add/get another displayable hierarchy that encapsulates both the associated node and the new fid node
    }
*/
    if (!this->GetActiveHierarchyNodeID())
      {
      // no active hierarchy node, this means we create the new node directly under the top-level hierarchy node
      char * toplevelHierarchyNodeID = nullptr;
      if (!annotationNode)
        {
        // we just add a new toplevel hierarchy node
        toplevelHierarchyNodeID = this->GetTopLevelHierarchyNodeID(nullptr);
        }
      else
        {
        // we need to insert the new toplevel hierarchy before the given annotationNode
        toplevelHierarchyNodeID = this->GetTopLevelHierarchyNodeID(annotationNode);
        }

      if (!toplevelHierarchyNodeID)
        {
        vtkErrorMacro("AddNewHierarchyNode: Toplevel hierarchy node was nullptr.");
        return false;
        }
      this->SetActiveHierarchyNodeID(toplevelHierarchyNodeID);
      }

    char *toplevelIDForThisClass = this->GetTopLevelHierarchyNodeIDForNodeClass(annotationNode);

    // Create a hierarchy node
    vtkMRMLAnnotationHierarchyNode* hierarchyNode =
        vtkMRMLAnnotationHierarchyNode::New();

    //hierarchyNode->SetScene(this->GetMRMLScene());

    if (!annotationNode)
      {
      // this is a user created hierarchy!

      // we want to see that!
      hierarchyNode->HideFromEditorsOff();

      hierarchyNode->SetName(
          this->GetMRMLScene()->GetUniqueNameByString("List"));

      this->GetMRMLScene()->AddNode(hierarchyNode);
      this->InvokeEvent(HierarchyNodeAddedEvent, hierarchyNode);

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
      annotationNode->SetDisableModifiedEvent(1);
      hierarchyNode->SetDisplayableNodeID(annotationNode->GetID());
      annotationNode->SetDisableModifiedEvent(0);
      //annotationNode->Modified();
      }

    if (toplevelIDForThisClass)
      {
      hierarchyNode->SetParentNodeID(toplevelIDForThisClass);
      }
    else
      {
      hierarchyNode->SetParentNodeID(this->GetActiveHierarchyNodeID());
      }

    if (!annotationNode)
      {
      // we want it to be the active hierarchy from now on (do this after
      // setting the parent node id)
      this->SetActiveHierarchyNodeID(hierarchyNode->GetID());
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
  return this->AddHierarchyNodeForAnnotation(nullptr);
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
      vtkErrorMacro("GetActiveHierarchyNode: Could not find or create any hierarchy.");
      return nullptr;
      }

    this->SetActiveHierarchyNodeID(toplevelNodeID);
    }
  if (this->GetMRMLScene()->GetNodeByID(this->GetActiveHierarchyNodeID()) == nullptr)
    {
    // try finding the top level hierarchy
    char* toplevelNodeID = this->GetTopLevelHierarchyNodeID();
    if (!toplevelNodeID)
      {
      vtkErrorMacro("GetActiveHierarchyNode: the active hierarchy node id was invalid and can't find or make a top level hierarchy node");
      // if the node with the active id can't be found in the scene, reset it to
      // null
      this->SetActiveHierarchyNodeID(nullptr);
      return nullptr;
      }
    else
      {
      this->SetActiveHierarchyNodeID(toplevelNodeID);
      }
    }
  return vtkMRMLAnnotationHierarchyNode::SafeDownCast(
    this->GetMRMLScene()->GetNodeByID(this->GetActiveHierarchyNodeID()));
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
      vtkErrorMacro("CreateSnapShot: No screenshot was set.");
      return;
      }
    if (!this->GetMRMLScene())
      {
      vtkErrorMacro("No scene defined");
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
      vtkErrorMacro("ModifySnapShot: No screenshot was set.");
      return;
      }
    if (!this->GetMRMLScene())
      {
      vtkErrorMacro("No scene defined");
      return;
      }
    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id.c_str());

    if (!node)
      {
      vtkErrorMacro("ModifySnapShot: Could not get node: " << id.c_str());
      return;
      }

    vtkMRMLAnnotationSnapshotNode* snapshotNode =
        vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

    if (!snapshotNode)
      {
      vtkErrorMacro("ModifySnapShot: Could not get snapshot node.");
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
    snapshotNode->SetScaleFactor(scaleFactor);
    snapshotNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent,
        snapshotNode);

  }

//---------------------------------------------------------------------------
// Return the description of an existing Annotation snapShot node.
//---------------------------------------------------------------------------
vtkStdString vtkSlicerAnnotationModuleLogic::GetSnapShotName(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return nullptr;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetSnapShotDescription: Could not get mrml node!");
      return nullptr;
    }

  vtkMRMLAnnotationSnapshotNode* snapshotNode =
    vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("GetSnapShotDescription: Could not get snapshot node!");
      return nullptr;
    }

  return snapshotNode->GetName();
}


//---------------------------------------------------------------------------
// Return the description of an existing Annotation snapShot node.
//---------------------------------------------------------------------------
vtkStdString vtkSlicerAnnotationModuleLogic::GetSnapShotDescription(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return nullptr;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetSnapShotDescription: Could not get mrml node!");
      return nullptr;
    }

  vtkMRMLAnnotationSnapshotNode* snapshotNode =
    vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("GetSnapShotDescription: Could not get snapshot node!");
      return nullptr;
    }

  return snapshotNode->GetSnapshotDescription();
}

//---------------------------------------------------------------------------
// Return the screenshotType of an existing Annotation snapShot node.
//---------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogic::GetSnapShotScreenshotType(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetSnapShotScreenshotType: Could not get mrml node!");
      return 0;
    }

  vtkMRMLAnnotationSnapshotNode* snapshotNode =
    vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("GetSnapShotScreenshotType: Could not get snapshot node!");
      return 0;
    }

  return snapshotNode->GetScreenShotType();
}

//---------------------------------------------------------------------------
// Return the screenshotType of an existing Annotation snapShot node.
//---------------------------------------------------------------------------
double vtkSlicerAnnotationModuleLogic::GetSnapShotScaleFactor(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return 0;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("GetSnapShotScaleFactor: Could not get mrml node!");
      return 0;
    }

  vtkMRMLAnnotationSnapshotNode* snapshotNode =
    vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("GetSnapShotScaleFactor: Could not get snapshot node!");
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
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return nullptr;
    }
  if (!node)
    {
    vtkErrorMacro("GetSnapShotScreenshot: Could not get mrml node!");
      return nullptr;
    }

  vtkMRMLAnnotationSnapshotNode* snapshotNode =
    vtkMRMLAnnotationSnapshotNode::SafeDownCast(node);

  if (!snapshotNode)
    {
    vtkErrorMacro("GetSnapShotScreenshot: Could not get snapshot node!");
      return nullptr;
    }

  return snapshotNode->GetScreenShot();
}

//---------------------------------------------------------------------------
// Check if node id corresponds to a snapShot node.
//---------------------------------------------------------------------------
bool vtkSlicerAnnotationModuleLogic::IsSnapshotNode(const char* id)
{

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return false;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);

  if (!node)
    {
    vtkErrorMacro("IsSnapshotNode: Invalid node.");
    return false;
    }

  return node->IsA("vtkMRMLAnnotationSnapshotNode");

}
//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetHierarchyAnnotationsVisibleFlag(vtkMRMLAnnotationHierarchyNode* hierarchyNode, bool flag)
{
  if (hierarchyNode == nullptr)
    {
    // use the active one
    hierarchyNode = this->GetActiveHierarchyNode();
    }
  if (!hierarchyNode)
    {
    vtkErrorMacro("SetHierarchyAnnotationsVisibleFlag: no hierarchy node");
    return;
    }
  vtkNew<vtkCollection> children;
  hierarchyNode->GetChildrenDisplayableNodes(children);

  children->InitTraversal();
  for (int i=0; i<children->GetNumberOfItems(); ++i)
    {
    vtkMRMLAnnotationNode* childNode = vtkMRMLAnnotationNode::SafeDownCast(children->GetItemAsObject(i));
    if (childNode)
      {
      // this is a valid annotation child node
      childNode->SetDisplayVisibility((flag ? 1 : 0));
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::SetHierarchyAnnotationsLockFlag(vtkMRMLAnnotationHierarchyNode* hierarchyNode, bool flag)
{
  if (hierarchyNode == nullptr)
    {
    // use the active one
    hierarchyNode = this->GetActiveHierarchyNode();
    }
  if (!hierarchyNode)
    {
    vtkErrorMacro("SetHierarchyAnnotationsLockFlag: no hierarchy node");
    return;
    }
  vtkNew<vtkCollection> children;
  hierarchyNode->GetChildrenDisplayableNodes(children);

  children->InitTraversal();
  for (int i=0; i<children->GetNumberOfItems(); ++i)
    {
    vtkMRMLAnnotationNode* childNode = vtkMRMLAnnotationNode::SafeDownCast(children->GetItemAsObject(i));
    if (childNode)
      {
      // this is a valid annotation child node
      childNode->SetLocked((flag ? 1 : 0));
      }
    }
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
    vtkErrorMacro("GetHTMLRepresentation: We need a hierarchy Node here.");
    return nullptr;
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
  const char *icon = this->GetAnnotationIcon(hierarchyNode->GetID());
  html += (icon ? icon : "");
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
    vtkErrorMacro("GetHTMLRepresentation: We need an annotation Node here.");
    return nullptr;
    }

  vtkStdString html = vtkStdString("<tr><td valign='middle'>");

  // level
  for (int i = 0; i < level; ++i)
    {
    html += "&nbsp;&nbsp;&nbsp;&nbsp;";
    }

  // icon
  html += "<img src='";
  const char *icon = this->GetAnnotationIcon(annotationNode->GetID());
  if (icon)
    {
    html += icon;
    }
  html += "'>";

  html += "</td><td valign='middle'>";
  // if this is a snapshotNode, we want to include the image here
  if (annotationNode->IsA("vtkMRMLAnnotationSnapshotNode"))
    {
    vtkImageData* image =
      this->GetSnapShotScreenshot(annotationNode->GetID());

    if (image)
      {

      vtkStdString tempPath = vtkStdString(this->GetApplicationLogic()->GetTemporaryPath());
      tempPath.append(annotationNode->GetID());
      tempPath.append(".png");

      vtkNew<vtkPNGWriter> w;
      w->SetInputData(image);
      w->SetFileName(tempPath.c_str());
      w->Write();

      html += "<img src='";
      html += tempPath;
      html += "' width='400'>";

      }
    }
  else
    {
    const char *measurement =  this->GetAnnotationMeasurement(annotationNode->GetID(), true);
    if (measurement)
      {
      html += measurement;
      }
    }
  html += "</td><td valign='middle'>";

  // text
  vtkStdString txt = this->GetAnnotationText(annotationNode->GetID());
  if (txt)
    {
    html += txt;
    }
  // if this is a snapshotNode, we want to include the image here
  if (annotationNode->IsA("vtkMRMLAnnotationSnapshotNode"))
    {
    html += "<br><br>";
    vtkStdString desc =  this->GetSnapShotDescription(annotationNode->GetID());
    if (desc)
      {
      html += desc;
      }
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
    return nullptr;
    }
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return nullptr;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);
  if (!node)
    {
    return nullptr;
    }
  vtkMRMLAnnotationNode *textNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!textNode)
    {
    return nullptr;
    }
  return textNode->GetAnnotationTextDisplayNode();
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationPointDisplayNode *vtkSlicerAnnotationModuleLogic::GetPointDisplayNode(const char *id)
{
  if (!id)
    {
    return nullptr;
    }
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return nullptr;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);
  if (!node)
    {
    return nullptr;
    }
  vtkMRMLAnnotationControlPointsNode *pointsNode =
    vtkMRMLAnnotationControlPointsNode::SafeDownCast(node);
  if (!pointsNode)
    {
    return nullptr;
    }
  // get the point display node
  return pointsNode->GetAnnotationPointDisplayNode();
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationLineDisplayNode *vtkSlicerAnnotationModuleLogic::GetLineDisplayNode(const char *id)
{
  if (!id)
    {
    return nullptr;
    }
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene defined");
    return nullptr;
    }
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(id);
  if (!node)
    {
    return nullptr;
    }
  vtkMRMLAnnotationLinesNode *linesNode =
    vtkMRMLAnnotationLinesNode::SafeDownCast(node);
  if (!linesNode)
    {
    return nullptr;
    }

  return linesNode->GetAnnotationLineDisplayNode();
}

//---------------------------------------------------------------------------
const char * vtkSlicerAnnotationModuleLogic::AddDisplayNodeForHierarchyNode(vtkMRMLAnnotationHierarchyNode *hnode)
{
  if (!hnode)
    {
    vtkErrorMacro("AddDisplayNodeForHierarchyNode: null input hierarchy node");
    return nullptr;
    }
  if (hnode->GetDisplayNode() && hnode->GetDisplayNodeID())
    {
    // it already has a display node
    return hnode->GetDisplayNodeID();
    }
  vtkMRMLAnnotationDisplayNode *dnode = vtkMRMLAnnotationDisplayNode::New();
  if (!dnode)
    {
    vtkErrorMacro("AddDisplayNodeForHierarchyNode: error creating a new display node");
    return nullptr;
    }
  dnode->SetVisibility(1);
  if (this->GetMRMLScene())
    {
    this->GetMRMLScene()->AddNode(dnode);
    hnode->SetAndObserveDisplayNodeID(dnode->GetID());
    }
  const char *id = dnode->GetID();
  dnode->Delete();
  return id;
}
