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

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>

// Transform MRMLDM includes
#include "vtkMRMLTransformHandleWidgetRepresentation.h"

// MRML includes
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLInteractionEventData.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLViewNode.h>

// vtkAddon includes
#include <vtkAddonMathUtilities.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLTransformHandleWidgetRepresentation);

//----------------------------------------------------------------------
vtkMRMLTransformHandleWidgetRepresentation::vtkMRMLTransformHandleWidgetRepresentation() {}

//----------------------------------------------------------------------
vtkMRMLTransformHandleWidgetRepresentation::~vtkMRMLTransformHandleWidgetRepresentation() = default;

//-----------------------------------------------------------------------------
void vtkMRMLTransformHandleWidgetRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------
vtkMRMLTransformDisplayNode* vtkMRMLTransformHandleWidgetRepresentation::GetDisplayNode()
{
  return this->DisplayNode;
}

//----------------------------------------------------------------------
void vtkMRMLTransformHandleWidgetRepresentation::SetDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
{
  this->DisplayNode = displayNode;
}

//----------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLTransformHandleWidgetRepresentation::GetTransformNode()
{
  if (!this->GetDisplayNode())
  {
    return nullptr;
  }
  return vtkMRMLTransformNode::SafeDownCast(this->GetDisplayNode()->GetDisplayableNode());
}

//----------------------------------------------------------------------
int vtkMRMLTransformHandleWidgetRepresentation::GetActiveComponentType()
{
  return this->DisplayNode->GetActiveInteractionType();
}

//----------------------------------------------------------------------
void vtkMRMLTransformHandleWidgetRepresentation::SetActiveComponentType(int type)
{
  this->DisplayNode->SetActiveInteractionType(type);
}

//----------------------------------------------------------------------
int vtkMRMLTransformHandleWidgetRepresentation::GetActiveComponentIndex()
{
  return this->DisplayNode->GetActiveInteractionIndex();
}

//----------------------------------------------------------------------
void vtkMRMLTransformHandleWidgetRepresentation::SetActiveComponentIndex(int index)
{
  this->DisplayNode->SetActiveInteractionIndex(index);
}

//----------------------------------------------------------------------
bool vtkMRMLTransformHandleWidgetRepresentation::IsDisplayable()
{
  if (!this->GetDisplayNode() || !this->GetTransformNode() || !this->GetTransformNode()->IsLinear())
  {
    return false;
  }

  if (!this->GetDisplayNode()->GetEditorVisibility())
  {
    return false;
  }

  if (this->GetSliceNode())
  {
    return this->GetDisplayNode()->GetEditorSliceIntersectionVisibility();
  }

  return this->GetDisplayNode()->GetEditorVisibility3D();
}

//----------------------------------------------------------------------
void vtkMRMLTransformHandleWidgetRepresentation::UpdateInteractionPipeline()
{
  vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(this->ViewNode);
  vtkProp* actor = this->GetInteractionActor();
  if (!viewNode || !this->GetTransformNode())
  {
    actor->SetVisibility(false);
    return;
  }

  // Final visibility handled by superclass in vtkMRMLInteractionWidgetRepresentation
  Superclass::UpdateInteractionPipeline();
}

//----------------------------------------------------------------------
void vtkMRMLTransformHandleWidgetRepresentation::UpdateHandleToWorldTransform(vtkTransform* handleToWorldTransform)
{
  vtkMRMLTransformNode* transformNode = this->GetTransformNode();
  if (!transformNode)
  {
    handleToWorldTransform->Identity();
    return;
  }

  vtkNew<vtkGeneralTransform> nodeToWorldTransform;
  vtkMRMLTransformNode::GetTransformBetweenNodes(transformNode, nullptr, nodeToWorldTransform);

  double centerOfTransformationNode[4] = { 0.0, 0.0, 0.0, 1.0 };
  transformNode->GetCenterOfTransformation(centerOfTransformationNode);

  double centerOfTransformationWorld[4] = { 0.0, 0.0, 0.0, 1.0 };
  nodeToWorldTransform->TransformPoint(centerOfTransformationNode, centerOfTransformationWorld);

  double xDirectionNode[4] = { 1.0, 0.0, 0.0, 0.0 };
  double xDirectionWorld[4] = { 1.0, 0.0, 0.0, 0.0 };
  nodeToWorldTransform->TransformVectorAtPoint(centerOfTransformationNode, xDirectionNode, xDirectionWorld);

  double yDirectionNode[4] = { 0.0, 1.0, 0.0, 0.0 };
  double yDirectionWorld[4] = { 0.0, 1.0, 0.0, 0.0 };
  nodeToWorldTransform->TransformVectorAtPoint(centerOfTransformationNode, yDirectionNode, yDirectionWorld);

  double zDirectionNode[4] = { 0.0, 0.0, 1.0, 0.0 };
  double zDirectionWorld[4] = { 0.0, 0.0, 1.0, 0.0 };
  nodeToWorldTransform->TransformVectorAtPoint(centerOfTransformationNode, zDirectionNode, zDirectionWorld);

  vtkNew<vtkMatrix4x4> nodeToWorldMatrix;
  for (int i = 0; i < 3; i++)
  {
    nodeToWorldMatrix->SetElement(i, 0, xDirectionWorld[i]);
    nodeToWorldMatrix->SetElement(i, 1, yDirectionWorld[i]);
    nodeToWorldMatrix->SetElement(i, 2, zDirectionWorld[i]);
    nodeToWorldMatrix->SetElement(i, 3, centerOfTransformationWorld[i]);
  }

  // The vtkMRMLInteractionWidgetRepresentation::UpdateHandleToWorldTransform() method will orthogonalize the matrix.
  handleToWorldTransform->Identity();
  handleToWorldTransform->Concatenate(nodeToWorldMatrix);
}

//----------------------------------------------------------------------
double vtkMRMLTransformHandleWidgetRepresentation::GetInteractionScalePercent()
{
  return this->GetDisplayNode()->GetInteractionScalePercent();
}

//----------------------------------------------------------------------
double vtkMRMLTransformHandleWidgetRepresentation::GetInteractionSizeMm()
{
  return this->GetDisplayNode()->GetInteractionSizeMm();
}

//----------------------------------------------------------------------
bool vtkMRMLTransformHandleWidgetRepresentation::GetInteractionSizeAbsolute()
{
  return this->GetDisplayNode()->GetInteractionSizeAbsolute();
}

//----------------------------------------------------------------------
double vtkMRMLTransformHandleWidgetRepresentation::GetInteractionHandleOpacity()
{
  return this->GetDisplayNode()->GetInteractionHandleOpacity();
}

//----------------------------------------------------------------------
double vtkMRMLTransformHandleWidgetRepresentation::GetTranslationScaleFactor()
{
  vtkMRMLTransformDisplayNode* displayNode = this->GetDisplayNode();
  if (this->LastInteractionWasWideHit && displayNode)
  {
    return displayNode->GetEditorTranslationSliceAnywhereSensitivity();
  }
  return 1.0;
}

//----------------------------------------------------------------------
int vtkMRMLTransformHandleWidgetRepresentation::GetHandleGlyphType(int type, int index)
{
  vtkMRMLTransformDisplayNode* displayNode = this->GetDisplayNode();
  if (type == InteractionTranslationHandle && index == HandleIndexViewPlane && this->GetSliceNode() //
      && displayNode && displayNode->GetEditorTranslationSliceAnywhereEnabled())
  {
    return GlyphCrosshair;
  }
  return this->Superclass::GetHandleGlyphType(type, index);
}

//----------------------------------------------------------------------
void vtkMRMLTransformHandleWidgetRepresentation::GetHandleColor(int type, int index, double color[4])
{
  if (!color)
  {
    return;
  }

  // Covers both the view plane translation handle (rendered as a plain circle, or as a crosshair
  // when wide translation is enabled, see GetHandleGlyphType()) and the in-plane rotation handle.
  bool isViewPlaneTranslation = (type == InteractionTranslationHandle && index == HandleIndexViewPlane);
  bool isInPlaneRotation = (type == InteractionRotationHandle && index == HandleIndexViewPlane);
  if (!isViewPlaneTranslation && !isInPlaneRotation)
  {
    this->Superclass::GetHandleColor(type, index, color);
    return;
  }

  bool selected = this->GetActiveComponentType() == type && this->GetActiveComponentIndex() == index;
  // Same value (0.8) and saturation (0.5625) as the red/green/blue axis colors below, at the cyan hue.
  double cyan[3] = { 0.35, 0.80, 0.80 };
  double cyanSelected[3] = { 0.07, 0.70, 0.70 };
  double* currentColor = selected ? cyanSelected : cyan;

  double opacity = this->GetHandleOpacity(type, index);
  if (selected)
  {
    opacity = this->GetInteractionHandleOpacity();
  }

  for (int i = 0; i < 3; ++i)
  {
    color[i] = currentColor[i];
  }
  color[3] = opacity;
}

//----------------------------------------------------------------------
void vtkMRMLTransformHandleWidgetRepresentation::CanInteract(vtkMRMLInteractionEventData* interactionEventData,
                                                             int& foundComponentType,
                                                             int& foundComponentIndex,
                                                             double& closestDistance2)
{
  this->LastInteractionWasWideHit = false;

  this->Superclass::CanInteract(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType != InteractionNone)
  {
    // A handle was hit precisely, prefer that over the wide hit-test area below.
    return;
  }

  vtkMRMLTransformDisplayNode* displayNode = this->GetDisplayNode();
  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
  if (!this->IsDisplayable() || !displayNode || !displayNode->GetEditorTranslationSliceAnywhereEnabled() //
      || !sliceNode                                                                                      // wide dragging is only supported in slice views
      || !interactionEventData || !interactionEventData->IsDisplayPositionValid()                        //
      || !this->GetHandleVisibility(InteractionTranslationHandle, HandleIndexViewPlane))                 // view plane translation handle must be enabled
  {
    return;
  }

  // Compute the (squared) display-coordinate distance between the event position and the transform
  // center, so that if multiple transforms can be dragged from anywhere in the same slice view, the
  // one whose center is closest to the cursor can be given priority.
  double handleWorldPos[3] = { 0.0, 0.0, 0.0 };
  this->GetInteractionHandlePositionWorld(InteractionTranslationHandle, HandleIndexViewPlane, handleWorldPos);
  double handleWorldPos4[4] = { handleWorldPos[0], handleWorldPos[1], handleWorldPos[2], 1.0 };

  vtkNew<vtkMatrix4x4> rasToxyMatrix;
  vtkMatrix4x4::Invert(sliceNode->GetXYToRAS(), rasToxyMatrix);
  double handleDisplayPos[4] = { 0.0, 0.0, 0.0, 1.0 };
  rasToxyMatrix->MultiplyPoint(handleWorldPos4, handleDisplayPos);
  handleDisplayPos[2] = 0.0; // Handles are always projected

  const int* displayPosition = interactionEventData->GetDisplayPosition();
  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };

  foundComponentType = InteractionTranslationHandle;
  foundComponentIndex = HandleIndexViewPlane;
  this->LastSliceWideHitDistance2 = vtkMath::Distance2BetweenPoints(handleDisplayPos, displayPosition3);
  this->LastInteractionWasWideHit = true;
}

//----------------------------------------------------------------------
bool vtkMRMLTransformHandleWidgetRepresentation::GetHandleVisibility(int type, int index)
{
  vtkMRMLTransformDisplayNode* displayNode = this->GetDisplayNode();
  if (!displayNode)
  {
    return false;
  }

  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();

  bool visible = Superclass::GetHandleVisibility(type, index);
  if (type == InteractionRotationHandle)
  {
    visible &= sliceNode ? displayNode->GetEditorRotationSliceEnabled() : displayNode->GetEditorRotationEnabled();
  }
  else if (type == InteractionTranslationHandle)
  {
    visible &= sliceNode ? displayNode->GetEditorTranslationSliceEnabled() : displayNode->GetEditorTranslationEnabled();
  }
  else if (type == InteractionScaleHandle)
  {
    visible &= sliceNode ? displayNode->GetEditorScalingSliceEnabled() : displayNode->GetEditorScalingEnabled();
  }

  bool handleVisibility[4] = { false, false, false, false };
  if (type == InteractionRotationHandle)
  {
    sliceNode ? this->GetDisplayNode()->GetRotationHandleComponentVisibilitySlice(handleVisibility)
              : this->GetDisplayNode()->GetRotationHandleComponentVisibility3D(handleVisibility);
  }
  else if (type == InteractionScaleHandle)
  {
    sliceNode ? this->GetDisplayNode()->GetScaleHandleComponentVisibilitySlice(handleVisibility) : this->GetDisplayNode()->GetScaleHandleComponentVisibility3D(handleVisibility);
  }
  else if (type == InteractionTranslationHandle)
  {
    sliceNode ? this->GetDisplayNode()->GetTranslationHandleComponentVisibilitySlice(handleVisibility)
              : this->GetDisplayNode()->GetTranslationHandleComponentVisibility3D(handleVisibility);
  }

  if (index >= 0 && index <= 3)
  {
    visible &= handleVisibility[index];
  }

  return visible;
}
