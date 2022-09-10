// Annotation includes
#include "vtkSlicerAnnotationModuleLogic.h"

// Annotation MRML includes
#include "vtkMRMLAnnotationControlPointsStorageNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationFiducialsStorageNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationLinesStorageNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationRulerStorageNode.h"
#include "vtkMRMLAnnotationStorageNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationSnapshotNode.h"
#include "vtkMRMLAnnotationSnapshotStorageNode.h"

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLUnitNode.h>

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
}

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic::~vtkSlicerAnnotationModuleLogic()
{
}

//-----------------------------------------------------------------------------
void vtkSlicerAnnotationModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

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

  // Ruler annotation
  vtkNew<vtkMRMLAnnotationRulerNode> annotationRulerNode;
  scene->RegisterNodeClass(annotationRulerNode.GetPointer());

  vtkNew<vtkMRMLAnnotationRulerStorageNode> annotationRulerStorageNode;
  scene->RegisterNodeClass(annotationRulerStorageNode.GetPointer());

  // ROI annotation
  vtkNew<vtkMRMLAnnotationROINode> annotationROINode;
  scene->RegisterNodeClass(annotationROINode.GetPointer());
  // ROI annotation backwards compatibility
#if MRML_APPLICATION_SUPPORT_VERSION < MRML_VERSION_CHECK(4, 0, 0)
  scene->RegisterNodeClass(annotationROINode.GetPointer(), "ROI");
#endif

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
