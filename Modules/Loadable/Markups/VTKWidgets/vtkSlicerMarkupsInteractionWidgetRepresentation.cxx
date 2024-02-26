/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

// Transform MRMLDM includes
#include "vtkSlicerMarkupsInteractionWidgetRepresentation.h"

// VTK includes
#include <vtkPlane.h>
#include <vtkPointData.h>

// Markups MRML includes
#include <vtkMRMLMarkupsPlaneDisplayNode.h>
#include <vtkMRMLMarkupsPlaneNode.h>
#include <vtkMRMLMarkupsROIDisplayNode.h>
#include <vtkMRMLMarkupsROINode.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerMarkupsInteractionWidgetRepresentation);

//----------------------------------------------------------------------
vtkSlicerMarkupsInteractionWidgetRepresentation::vtkSlicerMarkupsInteractionWidgetRepresentation() {}

//----------------------------------------------------------------------
vtkSlicerMarkupsInteractionWidgetRepresentation::~vtkSlicerMarkupsInteractionWidgetRepresentation() = default;

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode* vtkSlicerMarkupsInteractionWidgetRepresentation::GetDisplayNode()
{
  return this->DisplayNode;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::SetDisplayNode(vtkMRMLMarkupsDisplayNode* displayNode)
{
  this->DisplayNode = displayNode;
}

//----------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkSlicerMarkupsInteractionWidgetRepresentation::GetMarkupsNode()
{
  if (!this->GetDisplayNode())
  {
    return nullptr;
  }
  return vtkMRMLMarkupsNode::SafeDownCast(this->GetDisplayNode()->GetDisplayableNode());
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsInteractionWidgetRepresentation::InteractionComponentToMarkupsComponent(
  int interactionComponentType)
{
  int markupsComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  if (interactionComponentType == InteractionRotationHandle)
  {
    markupsComponentType = vtkMRMLMarkupsDisplayNode::ComponentRotationHandle;
  }
  else if (interactionComponentType == InteractionScaleHandle)
  {
    markupsComponentType = vtkMRMLMarkupsDisplayNode::ComponentScaleHandle;
  }
  else if (interactionComponentType == InteractionTranslationHandle)
  {
    markupsComponentType = vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle;
  }
  return markupsComponentType;
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsInteractionWidgetRepresentation::MarkupsComponentToInteractionComponent(int markupsComponentType)
{
  int interactionComponentType = InteractionNone;
  if (markupsComponentType == vtkMRMLMarkupsDisplayNode::ComponentRotationHandle)
  {
    interactionComponentType = InteractionRotationHandle;
  }
  else if (markupsComponentType == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle)
  {
    interactionComponentType = InteractionScaleHandle;
  }
  else if (markupsComponentType == vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle)
  {
    interactionComponentType = InteractionTranslationHandle;
  }
  return interactionComponentType;
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsInteractionWidgetRepresentation::GetActiveComponentType()
{
  int activeComponentType = this->DisplayNode->GetActiveComponentType();
  return this->MarkupsComponentToInteractionComponent(activeComponentType);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::SetActiveComponentType(int type)
{
  int markupsComponentType = this->InteractionComponentToMarkupsComponent(type);
  this->DisplayNode->SetActiveComponent(markupsComponentType, this->GetActiveComponentIndex());
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsInteractionWidgetRepresentation::GetActiveComponentIndex()
{
  return this->DisplayNode->GetActiveComponentIndex();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::SetActiveComponentIndex(int index)
{
  this->DisplayNode->SetActiveComponent(this->DisplayNode->GetActiveComponentType(), index);
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsInteractionWidgetRepresentation::IsDisplayable()
{
  if (!this->GetDisplayNode() || !this->GetMarkupsNode())
  {
    return false;
  }

  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (planeNode && !planeNode->GetIsPlaneValid())
  {
    return false;
  }

  return this->GetDisplayNode()->GetVisibility() && this->GetDisplayNode()->GetHandlesInteractive();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::UpdateInteractionPipeline()
{
  bool currentVisibility = this->GetVisibility();
  vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(this->ViewNode);
  if (!viewNode || !this->GetMarkupsNode() || !this->IsDisplayable())
  {
    this->SetVisibility(false);
    if (currentVisibility)
    {
      // If we are changing visibility, we need to render.
      this->NeedToRenderOn();
    }
    return;
  }
  this->SetVisibility(true);

  // Final visibility handled by superclass in vtkMRMLInteractionWidgetRepresentation
  Superclass::UpdateInteractionPipeline();

  vtkMRMLMarkupsDisplayNode* displayNode = this->GetDisplayNode();
  if (!displayNode)
  {
    return;
  }

  for (int type = InteractionNone + 1; type < Interaction_Last; ++type)
  {
    int markupsComponentType = this->InteractionComponentToMarkupsComponent(type);
    vtkPolyData* handlePolyData = this->GetHandlePolydata(type);
    if (!handlePolyData)
    {
      continue;
    }
    vtkIdTypeArray* visibilityArray =
      vtkIdTypeArray::SafeDownCast(handlePolyData->GetPointData()->GetArray("visibility"));
    if (!visibilityArray)
    {
      continue;
    }
    visibilityArray->SetNumberOfValues(handlePolyData->GetNumberOfPoints());

    bool handleVisibility = displayNode->GetHandleVisibility(markupsComponentType);
    for (int i = 0; i < handlePolyData->GetNumberOfPoints(); ++i)
    {
      // bool handleIndexVisibility = displayNode->GetHandleVisibility(markupsComponentType, i);
      handleVisibility = true;
      visibilityArray->SetValue(i, handleVisibility);
    }
  }

  if (vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode()))
  {
    this->UpdatePlaneScaleHandles();
  }
  else if (vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode()))
  {
    this->UpdateROIScaleHandles();
  }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::UpdatePlaneScaleHandles()
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!planeNode)
  {
    return;
  }

  vtkNew<vtkPoints> planeCornerPoints_World;
  planeNode->GetPlaneCornerPointsWorld(planeCornerPoints_World);

  double lpCorner_World[3] = { 0.0, 0.0, 0.0 };
  planeCornerPoints_World->GetPoint(0, lpCorner_World);

  double laCorner_World[3] = { 0.0, 0.0, 0.0 };
  planeCornerPoints_World->GetPoint(1, laCorner_World);

  double raCorner_World[3] = { 0.0, 0.0, 0.0 };
  planeCornerPoints_World->GetPoint(2, raCorner_World);

  double rpCorner_World[3] = { 0.0, 0.0, 0.0 };
  planeCornerPoints_World->GetPoint(3, rpCorner_World);

  double lEdge_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(laCorner_World, lpCorner_World, lEdge_World);
  vtkMath::MultiplyScalar(lEdge_World, 0.5);

  double rEdge_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(raCorner_World, rpCorner_World, rEdge_World);
  vtkMath::MultiplyScalar(rEdge_World, 0.5);

  double aEdge_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(laCorner_World, raCorner_World, aEdge_World);
  vtkMath::MultiplyScalar(aEdge_World, 0.5);

  double pEdge_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(lpCorner_World, rpCorner_World, pEdge_World);
  vtkMath::MultiplyScalar(pEdge_World, 0.5);

  vtkNew<vtkPoints> scaleHandlePoints;
  scaleHandlePoints->SetNumberOfPoints(8);
  scaleHandlePoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleLEdge, lEdge_World);
  scaleHandlePoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleREdge, rEdge_World);
  scaleHandlePoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge, aEdge_World);
  scaleHandlePoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandlePEdge, pEdge_World);

  scaleHandlePoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner, lpCorner_World);
  scaleHandlePoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner, laCorner_World);
  scaleHandlePoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner, raCorner_World);
  scaleHandlePoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner, rpCorner_World);

  vtkNew<vtkTransform> worldToHandleTransform;
  worldToHandleTransform->DeepCopy(this->GetHandleToWorldTransform());
  worldToHandleTransform->Inverse();

  // Scale handles are expected in the handle coordinate system
  for (int i = 0; i < scaleHandlePoints->GetNumberOfPoints(); ++i)
  {
    double scaleHandlePoint[3] = { 0.0, 0.0, 0.0 };
    worldToHandleTransform->TransformPoint(scaleHandlePoints->GetPoint(i), scaleHandlePoint);
    scaleHandlePoints->SetPoint(i, scaleHandlePoint);
  }
  this->Pipeline->ScaleHandlePoints->SetPoints(scaleHandlePoints);
  this->NeedToRenderOn();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::UpdateROIScaleHandles()
{
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (!roiNode)
  {
    return;
  }

  if (this->GetSliceNode())
  {
    this->UpdateROIScaleHandles2D();
  }
  else
  {
    this->UpdateROIScaleHandles3D();
  }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::UpdateROIScaleHandles3D()
{
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (!roiNode)
  {
    return;
  }

  // 3D points
  double sideLengths[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetSizeWorld(sideLengths);
  vtkMath::MultiplyScalar(sideLengths, 0.5);

  vtkNew<vtkPoints> roiPoints;
  roiPoints->SetNumberOfPoints(14);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleLFace, -sideLengths[0], 0.0, 0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleRFace, sideLengths[0], 0.0, 0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandlePFace, 0.0, -sideLengths[1], 0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleAFace, 0.0, sideLengths[1], 0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleIFace, 0.0, 0.0, -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleSFace, 0.0, 0.0, sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleLPICorner, -sideLengths[0], -sideLengths[1], -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleRPICorner, sideLengths[0], -sideLengths[1], -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleLAICorner, -sideLengths[0], sideLengths[1], -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleRAICorner, sideLengths[0], sideLengths[1], -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner, -sideLengths[0], -sideLengths[1], sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleRPSCorner, sideLengths[0], -sideLengths[1], sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleLASCorner, -sideLengths[0], sideLengths[1], sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleRASCorner, sideLengths[0], sideLengths[1], sideLengths[2]);
  this->Pipeline->ScaleHandlePoints->SetPoints(roiPoints);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::UpdateROIScaleHandles2D()
{
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (!roiNode)
  {
    return;
  }

  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(roiNode->GetDisplayNode());
  if (!displayNode)
  {
    return;
  }

  double viewPlaneOrigin_World[3] = { 0.0, 0.0, 0.0 };
  this->SlicePlane->GetOrigin(viewPlaneOrigin_World);

  double viewPlaneNormal_World[3] = { 0.0, 0.0, 1.0 };
  this->SlicePlane->GetNormal(viewPlaneNormal_World);

  vtkMatrix4x4* objectToWorldMatrix = roiNode->GetObjectToWorldMatrix();
  vtkNew<vtkTransform> worldToObjectTransform;
  worldToObjectTransform->Concatenate(objectToWorldMatrix);
  worldToObjectTransform->Inverse();

  double viewPlaneOrigin_Object[3] = { 0.0, 0.0, 0.0 };
  double viewPlaneNormal_Object[3] = { 0.0, 0.0, 0.0 };
  worldToObjectTransform->TransformPoint(viewPlaneOrigin_World, viewPlaneOrigin_Object);
  worldToObjectTransform->TransformVector(viewPlaneNormal_World, viewPlaneNormal_Object);

  double sideLengths[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetSizeWorld(sideLengths);
  double sideRadius[3] = { sideLengths[0], sideLengths[1], sideLengths[2] };
  vtkMath::MultiplyScalar(sideRadius, 0.5);

  vtkNew<vtkPoints> scaleHandlePoints_Object;
  scaleHandlePoints_Object->SetNumberOfPoints(26);

  vtkIdTypeArray* visibilityArray =
    vtkIdTypeArray::SafeDownCast(this->Pipeline->ScaleHandlePoints->GetPointData()->GetArray("visibility"));
  visibilityArray->SetNumberOfValues(scaleHandlePoints_Object->GetNumberOfPoints());
  visibilityArray->Fill(displayNode ? displayNode->GetScaleHandleVisibility() : 1.0);

  // Corner handles are not visible in 2D
  for (int i = vtkMRMLMarkupsROIDisplayNode::HandleLPICorner; i <= vtkMRMLMarkupsROIDisplayNode::HandleRASCorner; ++i)
  {
    scaleHandlePoints_Object->SetPoint(i, 0.0, 0.0, 0.0);
    visibilityArray->SetValue(i, 0);
  }

  vtkNew<vtkPlane> plane;
  plane->SetNormal(viewPlaneNormal_Object);
  plane->SetOrigin(viewPlaneOrigin_Object);

  // Left face handle
  double lFacePoint_Object[3] = { -sideRadius[0], -sideRadius[1], -sideRadius[2] };
  double lFacePointX_Object[3] = { -sideRadius[0], sideRadius[1], -sideRadius[2] };
  double lFacePointY_Object[3] = { -sideRadius[0], -sideRadius[1], sideRadius[2] };
  double lFaceIntersection0_Object[3] = { 0.0, 0.0, 0.0 };
  double lFaceIntersection1_Object[3] = { 0.0, 0.0, 0.0 };
  if (vtkPlane::IntersectWithFinitePlane(viewPlaneNormal_Object,
                                         viewPlaneOrigin_Object,
                                         lFacePoint_Object,
                                         lFacePointX_Object,
                                         lFacePointY_Object,
                                         lFaceIntersection0_Object,
                                         lFaceIntersection1_Object))
  {
    vtkMath::Add(lFaceIntersection0_Object, lFaceIntersection1_Object, lFacePoint_Object);
    vtkMath::MultiplyScalar(lFacePoint_Object, 0.5);
  }
  else
  {
    // Face does not intersect with the slice. Handle should not be visible.
    visibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleLFace, false);
  }
  scaleHandlePoints_Object->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleLFace, lFacePoint_Object);

  // Right face handle
  double rFacePoint_Object[3] = { sideRadius[0], -sideRadius[1], -sideRadius[2] };
  double rFacePointX_Object[3] = { sideRadius[0], sideRadius[1], -sideRadius[2] };
  double rFacePointY_Object[3] = { sideRadius[0], -sideRadius[1], sideRadius[2] };
  double rFaceIntersection0_Object[3] = { 0.0, 0.0, 0.0 };
  double rFaceIntersection1_Object[3] = { 0.0, 0.0, 0.0 };
  if (vtkPlane::IntersectWithFinitePlane(viewPlaneNormal_Object,
                                         viewPlaneOrigin_Object,
                                         rFacePoint_Object,
                                         rFacePointX_Object,
                                         rFacePointY_Object,
                                         rFaceIntersection0_Object,
                                         rFaceIntersection1_Object))
  {
    vtkMath::Add(rFaceIntersection0_Object, rFaceIntersection1_Object, rFacePoint_Object);
    vtkMath::MultiplyScalar(rFacePoint_Object, 0.5);
  }
  else
  {
    // Face does not intersect with the slice. Handle should not be visible.
    visibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleRFace, false);
  }
  scaleHandlePoints_Object->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleRFace, rFacePoint_Object);

  // Posterior face handle
  double pFacePoint_Object[3] = { -sideRadius[0], -sideRadius[1], -sideRadius[2] };
  double pFacePointX_Object[3] = { sideRadius[0], -sideRadius[1], -sideRadius[2] };
  double pFacePointY_Object[3] = { -sideRadius[0], -sideRadius[1], sideRadius[2] };
  double pFaceIntersection0_Object[3] = { 0.0, 0.0, 0.0 };
  double pFaceIntersection1_Object[3] = { 0.0, 0.0, 0.0 };
  if (vtkPlane::IntersectWithFinitePlane(viewPlaneNormal_Object,
                                         viewPlaneOrigin_Object,
                                         pFacePoint_Object,
                                         pFacePointX_Object,
                                         pFacePointY_Object,
                                         pFaceIntersection0_Object,
                                         pFaceIntersection1_Object))
  {
    vtkMath::Add(pFaceIntersection0_Object, pFaceIntersection1_Object, pFacePoint_Object);
    vtkMath::MultiplyScalar(pFacePoint_Object, 0.5);
  }
  else
  {
    // Face does not intersect with the slice. Handle should not be visible.
    visibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandlePFace, false);
  }
  scaleHandlePoints_Object->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandlePFace, pFacePoint_Object);

  // Anterior face handle
  double aFacePoint_Object[3] = { -sideRadius[0], sideRadius[1], -sideRadius[2] };
  double aFacePointX_Object[3] = { sideRadius[0], sideRadius[1], -sideRadius[2] };
  double aFacePointY_Object[3] = { -sideRadius[0], sideRadius[1], sideRadius[2] };
  double aFaceIntersection0_Object[3] = { 0.0, 0.0, 0.0 };
  double aFaceIntersection1_Object[3] = { 0.0, 0.0, 0.0 };
  if (vtkPlane::IntersectWithFinitePlane(viewPlaneNormal_Object,
                                         viewPlaneOrigin_Object,
                                         aFacePoint_Object,
                                         aFacePointX_Object,
                                         aFacePointY_Object,
                                         aFaceIntersection0_Object,
                                         aFaceIntersection1_Object))
  {
    vtkMath::Add(aFaceIntersection0_Object, aFaceIntersection1_Object, aFacePoint_Object);
    vtkMath::MultiplyScalar(aFacePoint_Object, 0.5);
  }
  else
  {
    // Face does not intersect with the slice. Handle should not be visible.
    visibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleAFace, false);
  }
  scaleHandlePoints_Object->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleAFace, aFacePoint_Object);

  // Inferior face handle
  double iFacePoint_Object[3] = { -sideRadius[0], -sideRadius[1], -sideRadius[2] };
  double iFacePointX_Object[3] = { sideRadius[0], -sideRadius[1], -sideRadius[2] };
  double iFacePointY_Object[3] = { -sideRadius[0], sideRadius[1], -sideRadius[2] };
  double iFaceIntersection0_Object[3] = { 0.0, 0.0, 0.0 };
  double iFaceIntersection1_Object[3] = { 0.0, 0.0, 0.0 };
  if (vtkPlane::IntersectWithFinitePlane(viewPlaneNormal_Object,
                                         viewPlaneOrigin_Object,
                                         iFacePoint_Object,
                                         iFacePointX_Object,
                                         iFacePointY_Object,
                                         iFaceIntersection0_Object,
                                         iFaceIntersection1_Object))
  {
    vtkMath::Add(iFaceIntersection0_Object, iFaceIntersection1_Object, iFacePoint_Object);
    vtkMath::MultiplyScalar(iFacePoint_Object, 0.5);
  }
  else
  {
    // Face does not intersect with the slice. Handle should not be visible.
    visibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleIFace, false);
  }
  scaleHandlePoints_Object->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleIFace, iFacePoint_Object);

  // Superior face handle
  double sFacePoint_Object[3] = { -sideRadius[0], -sideRadius[1], sideRadius[2] };
  double sFacePointX_Object[3] = { sideRadius[0], -sideRadius[1], sideRadius[2] };
  double sFacePointY_Object[3] = { -sideRadius[0], sideRadius[1], sideRadius[2] };
  double sFaceIntersection0_Object[3] = { 0.0, 0.0, 0.0 };
  double sFaceIntersection1_Object[3] = { 0.0, 0.0, 0.0 };
  if (vtkPlane::IntersectWithFinitePlane(viewPlaneNormal_Object,
                                         viewPlaneOrigin_Object,
                                         sFacePoint_Object,
                                         sFacePointX_Object,
                                         sFacePointY_Object,
                                         sFaceIntersection0_Object,
                                         sFaceIntersection1_Object))
  {
    vtkMath::Add(sFaceIntersection0_Object, sFaceIntersection1_Object, sFacePoint_Object);
    vtkMath::MultiplyScalar(sFacePoint_Object, 0.5);
  }
  else
  {
    // Face does not intersect with the slice. Handle should not be visible.
    visibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleSFace, false);
  }
  scaleHandlePoints_Object->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleSFace, sFacePoint_Object);

  bool intersection = false;

  double sVector_Object[3] = { 0.0, 0.0, sideLengths[2] };
  double lpEdgePoint_Object[3] = { -sideRadius[0], -sideRadius[1], -sideRadius[2] };
  intersection |= this->AddScaleEdgeIntersection(vtkMRMLMarkupsROIDisplayNode::HandleLPEdge,
                                                 visibilityArray,
                                                 scaleHandlePoints_Object,
                                                 viewPlaneNormal_Object,
                                                 viewPlaneOrigin_Object,
                                                 lpEdgePoint_Object,
                                                 sVector_Object);
  double rpEdgePoint_Object[3] = { sideRadius[0], -sideRadius[1], -sideRadius[2] };
  intersection |= this->AddScaleEdgeIntersection(vtkMRMLMarkupsROIDisplayNode::HandleRPEdge,
                                                 visibilityArray,
                                                 scaleHandlePoints_Object,
                                                 viewPlaneNormal_Object,
                                                 viewPlaneOrigin_Object,
                                                 rpEdgePoint_Object,
                                                 sVector_Object);
  double laEdgePoint_Object[3] = { -sideRadius[0], sideRadius[1], -sideRadius[2] };
  intersection |= this->AddScaleEdgeIntersection(vtkMRMLMarkupsROIDisplayNode::HandleLAEdge,
                                                 visibilityArray,
                                                 scaleHandlePoints_Object,
                                                 viewPlaneNormal_Object,
                                                 viewPlaneOrigin_Object,
                                                 laEdgePoint_Object,
                                                 sVector_Object);
  double raEdgePoint_Object[3] = { sideRadius[0], sideRadius[1], -sideRadius[2] };
  intersection |= this->AddScaleEdgeIntersection(vtkMRMLMarkupsROIDisplayNode::HandleRAEdge,
                                                 visibilityArray,
                                                 scaleHandlePoints_Object,
                                                 viewPlaneNormal_Object,
                                                 viewPlaneOrigin_Object,
                                                 raEdgePoint_Object,
                                                 sVector_Object);

  double aVector_Object[3] = { 0.0, sideLengths[1], 0.0 };
  double liEdgePoint_Object[3] = { -sideRadius[0], -sideRadius[1], -sideRadius[2] };
  intersection |= this->AddScaleEdgeIntersection(vtkMRMLMarkupsROIDisplayNode::HandleLIEdge,
                                                 visibilityArray,
                                                 scaleHandlePoints_Object,
                                                 viewPlaneNormal_Object,
                                                 viewPlaneOrigin_Object,
                                                 liEdgePoint_Object,
                                                 aVector_Object);
  double riEdgePoint_Object[3] = { sideRadius[0], -sideRadius[1], -sideRadius[2] };
  intersection |= this->AddScaleEdgeIntersection(vtkMRMLMarkupsROIDisplayNode::HandleRIEdge,
                                                 visibilityArray,
                                                 scaleHandlePoints_Object,
                                                 viewPlaneNormal_Object,
                                                 viewPlaneOrigin_Object,
                                                 riEdgePoint_Object,
                                                 aVector_Object);
  double lsEdgePoint_Object[3] = { -sideRadius[0], -sideRadius[1], sideRadius[2] };
  intersection |= this->AddScaleEdgeIntersection(vtkMRMLMarkupsROIDisplayNode::HandleLSEdge,
                                                 visibilityArray,
                                                 scaleHandlePoints_Object,
                                                 viewPlaneNormal_Object,
                                                 viewPlaneOrigin_Object,
                                                 lsEdgePoint_Object,
                                                 aVector_Object);
  double rsEdgePoint_Object[3] = { sideRadius[0], -sideRadius[1], sideRadius[2] };
  intersection |= this->AddScaleEdgeIntersection(vtkMRMLMarkupsROIDisplayNode::HandleRSEdge,
                                                 visibilityArray,
                                                 scaleHandlePoints_Object,
                                                 viewPlaneNormal_Object,
                                                 viewPlaneOrigin_Object,
                                                 rsEdgePoint_Object,
                                                 aVector_Object);

  double rVector_Object[3] = { sideLengths[0], 0.0, 0.0 };
  double piEdgePoint_Object[3] = { -sideRadius[0], -sideRadius[1], -sideRadius[2] };
  intersection |= this->AddScaleEdgeIntersection(vtkMRMLMarkupsROIDisplayNode::HandlePIEdge,
                                                 visibilityArray,
                                                 scaleHandlePoints_Object,
                                                 viewPlaneNormal_Object,
                                                 viewPlaneOrigin_Object,
                                                 piEdgePoint_Object,
                                                 rVector_Object);
  double aiEdgePoint_Object[3] = { -sideRadius[0], sideRadius[1], -sideRadius[2] };
  intersection |= this->AddScaleEdgeIntersection(vtkMRMLMarkupsROIDisplayNode::HandleAIEdge,
                                                 visibilityArray,
                                                 scaleHandlePoints_Object,
                                                 viewPlaneNormal_Object,
                                                 viewPlaneOrigin_Object,
                                                 aiEdgePoint_Object,
                                                 rVector_Object);
  double psEdgePoint_Object[3] = { -sideRadius[0], -sideRadius[1], sideRadius[2] };
  intersection |= this->AddScaleEdgeIntersection(vtkMRMLMarkupsROIDisplayNode::HandlePSEdge,
                                                 visibilityArray,
                                                 scaleHandlePoints_Object,
                                                 viewPlaneNormal_Object,
                                                 viewPlaneOrigin_Object,
                                                 psEdgePoint_Object,
                                                 rVector_Object);
  double asEdgePoint_Object[3] = { -sideRadius[0], sideRadius[1], sideRadius[2] };
  intersection |= this->AddScaleEdgeIntersection(vtkMRMLMarkupsROIDisplayNode::HandleASEdge,
                                                 visibilityArray,
                                                 scaleHandlePoints_Object,
                                                 viewPlaneNormal_Object,
                                                 viewPlaneOrigin_Object,
                                                 asEdgePoint_Object,
                                                 rVector_Object);

  if (!intersection)
  {
    // None of the edges intersect with the slice. Handles should not be visible.
    this->SetVisibility(false);
  }
  else
  {
    this->SetVisibility(true);
  }

  this->Pipeline->ScaleHandlePoints->SetPoints(scaleHandlePoints_Object);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::UpdateHandleToWorldTransform(vtkTransform* handleToWorldTransform)
{
  handleToWorldTransform->Identity();
  handleToWorldTransform->Concatenate(this->GetMarkupsNode()->GetInteractionHandleToWorldMatrix());
}

//----------------------------------------------------------------------
double vtkSlicerMarkupsInteractionWidgetRepresentation::GetInteractionScalePercent()
{
  return this->GetDisplayNode()->GetGlyphScale() * 5.0;
}

//----------------------------------------------------------------------
double vtkSlicerMarkupsInteractionWidgetRepresentation::GetInteractionSizeMm()
{
  return this->GetDisplayNode()->GetGlyphSize() * 5.0;
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsInteractionWidgetRepresentation::GetInteractionSizeAbsolute()
{
  return !this->GetDisplayNode()->GetUseGlyphScale();
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsInteractionWidgetRepresentation::GetHandleVisibility(int type, int index)
{
  if (!this->GetDisplayNode())
  {
    return false;
  }

  int markupsComponentType = this->InteractionComponentToMarkupsComponent(type);
  if (!this->GetDisplayNode()->GetHandleVisibility(markupsComponentType))
  {
    return false;
  }

  bool handleVisibility[4] = { false, false, false, false };
  if (markupsComponentType == vtkMRMLMarkupsDisplayNode::ComponentRotationHandle)
  {
    this->GetDisplayNode()->GetRotationHandleComponentVisibility(handleVisibility);
  }
  else if (markupsComponentType == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle)
  {
    this->GetDisplayNode()->GetScaleHandleComponentVisibility(handleVisibility);
  }
  else if (markupsComponentType == vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle)
  {
    this->GetDisplayNode()->GetTranslationHandleComponentVisibility(handleVisibility);
  }

  int visibilityIndex = index;
  bool visibility = true;
  if (type == InteractionScaleHandle && vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode()))
  {
    switch (index)
    {
      case vtkMRMLMarkupsROIDisplayNode::HandleLFace:
      case vtkMRMLMarkupsROIDisplayNode::HandleRFace:
        visibilityIndex = 0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandlePFace:
      case vtkMRMLMarkupsROIDisplayNode::HandleAFace:
        visibilityIndex = 1;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleIFace:
      case vtkMRMLMarkupsROIDisplayNode::HandleSFace:
        visibilityIndex = 2;
        break;
      default:
        visibilityIndex = 3;
        break;
    }
  }
  else if (type == InteractionScaleHandle && vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode()))
  {
    switch (index)
    {
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLEdge:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleREdge:
        visibilityIndex = 0;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge:
      case vtkMRMLMarkupsPlaneDisplayNode::HandlePEdge:
        visibilityIndex = 1;
        break;
      default:
        visibilityIndex = 3;
        break;
    }
  }

  if (visibilityIndex >= 0 || visibilityIndex <= 3)
  {
    visibility = handleVisibility[visibilityIndex];
  }

  return visibility && Superclass::GetHandleVisibility(type, index);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::CreateScaleHandles()
{
  double distance = 1.5;
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (planeNode || roiNode)
  {
    vtkNew<vtkPoints> points;
    points->InsertNextPoint(distance, 0.0, 0.0);  // X-axis +ve
    points->InsertNextPoint(0.0, distance, 0.0);  // Y-axis +ve
    points->InsertNextPoint(0.0, 0.0, distance);  // Z-axis +ve
    points->InsertNextPoint(-distance, 0.0, 0.0); // X-axis -ve
    points->InsertNextPoint(0.0, -distance, 0.0); // Y-axis -ve
    points->InsertNextPoint(0.0, 0.0, -distance); // Z-axis -ve

    if (roiNode)
    {
      points->InsertNextPoint(distance, distance, distance);
      points->InsertNextPoint(distance, distance, -distance);
      points->InsertNextPoint(distance, -distance, distance);
      points->InsertNextPoint(distance, -distance, -distance);
      points->InsertNextPoint(-distance, distance, distance);
      points->InsertNextPoint(-distance, distance, -distance);
      points->InsertNextPoint(-distance, -distance, distance);
      points->InsertNextPoint(-distance, -distance, -distance);
      if (this->GetSliceNode())
      {
        for (unsigned long i = vtkMRMLMarkupsROIDisplayNode::HandleLPEdge;
             i < vtkMRMLMarkupsROIDisplayNode::HandleROI_Last;
             ++i)
        {
          points->InsertNextPoint(0.0, 0.0, 0.0);
        }
      }
    }
    else if (planeNode)
    {
      points->InsertNextPoint(distance, distance, 0.0);
      points->InsertNextPoint(-distance, distance, 0.0);
      points->InsertNextPoint(-distance, -distance, 0.0);
      points->InsertNextPoint(distance, -distance, 0.0);
    }

    this->Pipeline->ScaleHandlePoints->SetPoints(points);

    vtkNew<vtkIdTypeArray> visibilityArray;
    visibilityArray->SetName("visibility");
    visibilityArray->SetNumberOfComponents(1);
    visibilityArray->SetNumberOfValues(points->GetNumberOfPoints());
    visibilityArray->Fill(1.0);
    this->Pipeline->ScaleHandlePoints->GetPointData()->AddArray(visibilityArray);

    vtkNew<vtkDoubleArray> orientationArray;
    orientationArray->SetName("orientation");
    orientationArray->SetNumberOfComponents(9);
    orientationArray->SetNumberOfTuples(points->GetNumberOfPoints());
    orientationArray->Fill(0.0);
    this->Pipeline->ScaleHandlePoints->GetPointData()->AddArray(orientationArray);
  }
  else
  {
    Superclass::CreateScaleHandles();
  }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::GetInteractionHandleAxisLocal(int type,
                                                                                    int index,
                                                                                    double axis_Local[3])
{
  if (!axis_Local)
  {
    vtkErrorMacro("GetInteractionHandleAxisLocal: Invalid axis argument");
    return;
  }

  if (type != InteractionScaleHandle)
  {
    Superclass::GetInteractionHandleAxisLocal(type, index, axis_Local);
    return;
  }

  axis_Local[0] = 0.0;
  axis_Local[1] = 0.0;
  axis_Local[2] = 0.0;

  if (vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode()))
  {
    switch (index)
    {
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLEdge:
        axis_Local[0] = 1.0;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleREdge:
        axis_Local[0] = -1.0;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge:
        axis_Local[1] = 1.0;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandlePEdge:
        axis_Local[1] = -1.0;
        break;
      default:
        break;
    }
  }
  else if (vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode()))
  {
    switch (index)
    {
      case vtkMRMLMarkupsROIDisplayNode::HandleLFace:
        axis_Local[0] = -1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRFace:
        axis_Local[0] = 1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandlePFace:
        axis_Local[1] = -1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleAFace:
        axis_Local[1] = 1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleIFace:
        axis_Local[2] = -1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleSFace:
        axis_Local[2] = 1.0;
        break;
      default:
        // Other handles scaling are not restricted to a single axis
        break;
    }
  }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::GetHandleColor(int type, int index, double color[4])
{
  if (!color)
  {
    return;
  }

  if (type != InteractionScaleHandle)
  {
    Superclass::GetHandleColor(type, index, color);
    return;
  }

  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (!planeNode && !roiNode)
  {
    Superclass::GetHandleColor(type, index, color);
    return;
  }

  double red[3] = { 0.80, 0.35, 0.35 };
  double redSelected[3] = { 0.70, 0.07, 0.07 };

  double green[3] = { 0.35, 0.80, 0.35 };
  double greenSelected[3] = { 0.07, 0.70, 0.07 };

  double blue[3] = { 0.35, 0.35, 0.8 };
  double blueSelected[3] = { 0.07, 0.07, 0.70 };

  double white[3] = { 0.80, 0.80, 0.80 };
  double whiteSelected[3] = { 1.00, 1.00, 1.00 };

  double* currentColor = white;
  double* selectedColor = whiteSelected;

  if (planeNode)
  {
    switch (index)
    {
      case 0:
      case 1:
        currentColor = red;
        selectedColor = redSelected;
        break;
      case 2:
      case 3:
        currentColor = green;
        selectedColor = greenSelected;
        break;
      default:
        break;
    }
  }
  else if (roiNode)
  {
    switch (index)
    {
      case 0:
      case 1:
        currentColor = red;
        selectedColor = redSelected;
        break;
      case 2:
      case 3:
        currentColor = green;
        selectedColor = greenSelected;
        break;
      case 4:
      case 5:
        currentColor = blue;
        selectedColor = blueSelected;
        break;
      default:
        break;
    }
  }

  double opacity = this->GetHandleOpacity(type, index);
  if (this->GetActiveComponentType() == type && this->GetActiveComponentIndex() == index)
  {
    currentColor = selectedColor;
    opacity = 1.0;
  }

  for (int i = 0; i < 3; ++i)
  {
    color[i] = currentColor[i];
  }
  color[3] = opacity;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidgetRepresentation::GetInteractionHandlePositionWorld(int type,
                                                                                        int index,
                                                                                        double positionWorld[3])
{
  if (!positionWorld)
  {
    vtkErrorMacro("GetInteractionHandlePositionWorld: Invalid position argument");
  }

  if (type != InteractionScaleHandle)
  {
    Superclass::GetInteractionHandlePositionWorld(type, index, positionWorld);
    return;
  }

  vtkPolyData* handlePolyData = this->GetHandlePolydata(type);
  if (!handlePolyData)
  {
    return;
  }
  handlePolyData->GetPoint(index, positionWorld);
  this->Pipeline->HandleToWorldTransform->TransformPoint(positionWorld, positionWorld);
}

//-----------------------------------------------------------------------------
bool vtkSlicerMarkupsInteractionWidgetRepresentation::AddScaleEdgeIntersection(int pointIndex,
                                                                               vtkIdTypeArray* visibilityArray,
                                                                               vtkPoints* scaleHandlePoints_Object,
                                                                               double viewPlaneNormal_Object[3],
                                                                               double viewPlaneOrigin_Object[3],
                                                                               double edgePoint1_Object[3],
                                                                               double edgeVector_Object[3])
{
  double edgePoint2_Object[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(edgePoint1_Object, edgeVector_Object, edgePoint2_Object);

  double t = 0.0;
  double edgeIntersection_Object[3] = { 0.0, 0.0, 0.0 };
  if (!vtkPlane::IntersectWithLine(edgePoint1_Object,
                                   edgePoint2_Object,
                                   viewPlaneNormal_Object,
                                   viewPlaneOrigin_Object,
                                   t,
                                   edgeIntersection_Object))
  {
    scaleHandlePoints_Object->SetPoint(pointIndex, edgeIntersection_Object);
    visibilityArray->SetValue(pointIndex, false);
    return false;
  }
  scaleHandlePoints_Object->SetPoint(pointIndex, edgeIntersection_Object);
  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerMarkupsInteractionWidgetRepresentation::GetApplyScaleToPosition(int type, int index)
{
  if (type != InteractionScaleHandle)
  {
    return Superclass::GetApplyScaleToPosition(type, index);
  }

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (vtkMRMLMarkupsROINode::SafeDownCast(markupsNode) || vtkMRMLMarkupsPlaneNode::SafeDownCast(markupsNode))
  {
    return false;
  }

  return Superclass::GetApplyScaleToPosition(type, index);
}
