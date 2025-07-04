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
#include <vtkAppendPolyData.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkEllipseArcSource.h>
#include <vtkFloatArray.h>
#include <vtkFocalPlanePointPlacer.h>
#include <vtkGlyph3D.h>
#include <vtkLine.h>
#include <vtkLineSource.h>
#include <vtkLookupTable.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkStringArray.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTensorGlyph.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkTubeFilter.h>

// MRML includes
#include <vtkMRMLAbstractThreeDViewDisplayableManager.h>
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLInteractionEventData.h>
#include <vtkMRMLTransformNode.h>

#include <vtkMRMLInteractionWidgetRepresentation.h>

//----------------------------------------------------------------------
static const double INTERACTION_HANDLE_SCALE_RADIUS = 0.1;
static const double INTERACTION_HANDLE_SCALE_DISTANCE_FROM_CENTER = 1.6;

static const double INTERACTION_HANDLE_ROTATION_ARC_THICKNESS = 0.1;
static const double INTERACTION_HANDLE_RADIUS = 1.15;
static const double INTERACTION_HANDLE_ROTATION_ARC_OUTER_RADIUS =
  INTERACTION_HANDLE_RADIUS + 0.5 * INTERACTION_HANDLE_ROTATION_ARC_THICKNESS;
static const double INTERACTION_HANDLE_ROTATION_ARC_INNER_RADIUS =
  INTERACTION_HANDLE_RADIUS - 0.5 * INTERACTION_HANDLE_ROTATION_ARC_THICKNESS;
static const double INTERACTION_HANDLE_ROTATION_ARC_DEGREES = 360.0;
static const int INTERACTION_HANDLE_ROTATION_ARC_RESOLUTION = 30;

static const double INTERACTION_TRANSLATION_HANDLE_LENGTH = 0.75;
static const double INTERACTION_TRANSLATION_HANDLE_TIP_RADIUS = 0.15;
static const double INTERACTION_TRANSLATION_HANDLE_SHAFT_RADIUS = 0.05;

//----------------------------------------------------------------------
vtkMRMLInteractionWidgetRepresentation::vtkMRMLInteractionWidgetRepresentation()
{
  this->ViewScaleFactorMmPerPixel = 1.0;
  this->ScreenSizePixel = 1000;

  this->NeedToRender = false;

  this->PointPlacer = vtkSmartPointer<vtkFocalPlanePointPlacer>::New();

  this->AlwaysOnTop = true;

  // Using the minimum value of -66000 creates a lot of rendering artifacts on the occluded objects, as all of the
  // pixels in the occluded object will have the same depth buffer value (0.0).
  // Using a default value of -25000 strikes a balance between rendering the occluded objects on top of other objects,
  // while still providing enough leeway to ensure that occluded actors are rendered correctly relative to themselves
  // and to other occluded actors.
  this->AlwaysOnTopRelativeOffsetUnits = -25000.0;

  this->Pipeline = nullptr;

  this->SlicePlane = vtkSmartPointer<vtkPlane>::New();
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::SetupInteractionPipeline()
{
  this->Pipeline = new InteractionPipeline();
  this->InitializePipeline();
  if (this->GetSliceNode())
  {
    this->Pipeline->WorldToSliceTransformFilter->SetInputConnection(
      this->Pipeline->HandleToWorldTransformFilter->GetOutputPort());
    this->Pipeline->Mapper3D->SetInputConnection(this->Pipeline->WorldToSliceTransformFilter->GetOutputPort());
  }
  this->NeedToRenderOn();
}

//----------------------------------------------------------------------
vtkMRMLInteractionWidgetRepresentation::~vtkMRMLInteractionWidgetRepresentation()
{
  // Force deleting variables to prevent circular dependency keeping objects alive
  if (this->Pipeline != nullptr)
  {
    delete this->Pipeline;
    this->Pipeline = nullptr;
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------
double vtkMRMLInteractionWidgetRepresentation::GetMaximumHandlePickingDistance2()
{
  double maximumHandlePickingDistance =
    this->InteractionSize / 10.0 + this->PickingTolerance * this->GetScreenScaleFactor();
  return maximumHandlePickingDistance * maximumHandlePickingDistance;
}

//-----------------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::CanInteract(vtkMRMLInteractionEventData* interactionEventData,
                                                         int& foundComponentType,
                                                         int& foundComponentIndex,
                                                         double& closestDistance2)
{
  foundComponentType = InteractionNone;
  closestDistance2 = VTK_DOUBLE_MAX; // in display coordinate system
  foundComponentIndex = -1;

  vtkMRMLAbstractViewNode* viewNode = this->GetViewNode();
  if (!viewNode || !this->IsDisplayable() || !interactionEventData)
  {
    return;
  }

  HandleInfoList handleInfoList = this->GetHandleInfoList();
  for (HandleInfo handleInfo : handleInfoList)
  {
    if (!handleInfo.IsVisible())
    {
      continue;
    }

    if (handleInfo.GlyphType == GlyphCircle)
    {
      this->CanInteractWithCircleHandle(
        interactionEventData, foundComponentType, foundComponentIndex, closestDistance2, handleInfo);
    }
    else if (handleInfo.GlyphType == GlyphRing)
    {
      this->CanInteractWithRingHandle(
        interactionEventData, foundComponentType, foundComponentIndex, closestDistance2, handleInfo);
    }
    else if (handleInfo.GlyphType == GlyphArrow)
    {
      this->CanInteractWithArrowHandle(
        interactionEventData, foundComponentType, foundComponentIndex, closestDistance2, handleInfo);
    }
  }
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::CanInteractWithCircleHandle(
  vtkMRMLInteractionEventData* interactionEventData,
  int& foundComponentType,
  int& foundComponentIndex,
  double& closestDistance2,
  HandleInfo& handleInfo)
{
  vtkMRMLAbstractViewNode* viewNode = this->GetViewNode();
  if (!viewNode || !this->IsDisplayable() || !interactionEventData || !handleInfo.IsVisible() ||
      !interactionEventData->IsDisplayPositionValid())
  {
    return;
  }

  double maxPickingDistanceFromControlPoint2 = this->GetMaximumHandlePickingDistance2();
  double displayPosition3[3] = { 0.0, 0.0, 0.0 };
  // Display position is valid in case of desktop interactions. Otherwise it is a 3D only context such as
  // virtual reality, and then we expect a valid world position in the absence of display position.
  const int* displayPosition = interactionEventData->GetDisplayPosition();
  displayPosition3[0] = static_cast<double>(displayPosition[0]);
  displayPosition3[1] = static_cast<double>(displayPosition[1]);

  double handleDisplayPos[4] = { 0.0, 0.0, 0.0, 1.0 };
  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
  if (sliceNode)
  {
    vtkNew<vtkMatrix4x4> rasToxyMatrix;
    vtkMatrix4x4::Invert(sliceNode->GetXYToRAS(), rasToxyMatrix);

    double* handleWorldPos = handleInfo.PositionWorld;
    rasToxyMatrix->MultiplyPoint(handleWorldPos, handleDisplayPos);
    handleDisplayPos[2] = 0.0; // Handles are always projected
  }
  else
  {
    double* handleWorldPos = handleInfo.PositionWorld;
    this->Renderer->SetWorldPoint(handleWorldPos);
    this->Renderer->WorldToDisplay();
    this->Renderer->GetDisplayPoint(handleDisplayPos);
    handleDisplayPos[2] = 0.0; // Handles are always projected
  }

  double dist2 = vtkMath::Distance2BetweenPoints(handleDisplayPos, displayPosition3);
  if (dist2 < maxPickingDistanceFromControlPoint2 && dist2 <= closestDistance2)
  {
    closestDistance2 = dist2;
    foundComponentType = handleInfo.ComponentType;
    foundComponentIndex = handleInfo.Index;
  }
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::CanInteractWithArrowHandle(
  vtkMRMLInteractionEventData* interactionEventData,
  int& foundComponentType,
  int& foundComponentIndex,
  double& closestDistance2,
  HandleInfo& handleInfo)
{
  if (!handleInfo.IsVisible() || !interactionEventData->IsDisplayPositionValid())
  {
    return;
  }

  // Check the arrow tip.
  this->CanInteractWithCircleHandle(
    interactionEventData, foundComponentType, foundComponentIndex, closestDistance2, handleInfo);

  // Display position is valid in case of desktop interactions. Otherwise it is a 3D only context such as
  // virtual reality, and then we expect a valid world position in the absence of display position.
  const int* displayPosition = interactionEventData->GetDisplayPosition();
  double displayPosition3[3] = { 0.0, 0.0, 0.0 };
  displayPosition3[0] = static_cast<double>(displayPosition[0]);
  displayPosition3[1] = static_cast<double>(displayPosition[1]);

  double* handleWorldPos = handleInfo.PositionWorld;
  double handleWorldPos4[4] = { handleWorldPos[0], handleWorldPos[1], handleWorldPos[2], 1.0 };
  double handleDisplayPos4[4] = { 0.0, 0.0, 0.0, 1.0 };
  double originWorldPos4[4] = { 0.0, 0.0, 0.0, 1.0 };
  this->GetInteractionHandleOriginWorld(originWorldPos4);
  double originDisplayPos4[4] = { 0.0, 0.0, 0.0, 1.0 };

  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
  if (sliceNode)
  {
    vtkNew<vtkMatrix4x4> rasToxyMatrix;
    vtkMatrix4x4::Invert(sliceNode->GetXYToRAS(), rasToxyMatrix);

    rasToxyMatrix->MultiplyPoint(handleWorldPos4, handleDisplayPos4);
    handleDisplayPos4[2] = 0.0; // Handles are always projected.

    rasToxyMatrix->MultiplyPoint(originWorldPos4, originDisplayPos4);
    originDisplayPos4[2] = 0.0; // Handles are always projected.
  }
  else
  {
    this->Renderer->SetWorldPoint(handleWorldPos);
    this->Renderer->WorldToDisplay();
    this->Renderer->GetDisplayPoint(handleDisplayPos4);
    handleDisplayPos4[2] = 0.0; // Handles are always projected.

    this->Renderer->SetWorldPoint(originWorldPos4);
    this->Renderer->WorldToDisplay();
    this->Renderer->GetDisplayPoint(originDisplayPos4);
    originDisplayPos4[2] = 0.0; // Handles are always projected.
  }

  double t = 0.0;
  double lineDistance = vtkLine::DistanceToLine(displayPosition3, originDisplayPos4, handleDisplayPos4, t);
  if (t > 1.0 - INTERACTION_TRANSLATION_HANDLE_LENGTH && t < 1.0)
  {
    double lineDistance2 = lineDistance * lineDistance;
    if (lineDistance < this->GetMaximumHandlePickingDistance2() && lineDistance2 <= closestDistance2)
    {
      closestDistance2 = lineDistance2;
      foundComponentType = handleInfo.ComponentType;
      foundComponentIndex = handleInfo.Index;
    }
  }
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::CanInteractWithRingHandle(
  vtkMRMLInteractionEventData* interactionEventData,
  int& foundComponentType,
  int& foundComponentIndex,
  double& closestDistance2,
  HandleInfo& handleInfo)
{
  double handleNormalWorld[4] = { 0.0, 0.0, 0.0, 0.0 };
  this->GetInteractionHandleAxisWorld(handleInfo.ComponentType, handleInfo.Index, handleNormalWorld);
  if (handleNormalWorld[0] == 0.0 && handleNormalWorld[1] == 0.0 && handleNormalWorld[2] == 0.0)
  {
    // Axis not valid.
    return;
  }


  // Display position is valid in case of desktop interactions. Otherwise it is a 3D only context such as
  // virtual reality, and then we expect a valid world position in the absence of display position.
  if (!interactionEventData->IsDisplayPositionValid())
  {
    return;
  }

  double* handleWorldPos = handleInfo.PositionWorld;

  vtkNew<vtkPlane> plane;
  plane->SetOrigin(handleWorldPos);
  plane->SetNormal(handleNormalWorld);

  const int* displayPosition = interactionEventData->GetDisplayPosition();
  double displayPosition4[4] = { 0.0, 0.0, 0.0, 1.0 };
  displayPosition4[0] = static_cast<double>(displayPosition[0]);
  displayPosition4[1] = static_cast<double>(displayPosition[1]);
  double worldPosition4[4] = { 0.0, 0.0, 0.0, 0.0 };

  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
  if (sliceNode)
  {
    sliceNode->GetXYToRAS()->MultiplyPoint(displayPosition4, worldPosition4);
  }
  else
  {
    this->Renderer->SetDisplayPoint(displayPosition4);
    this->Renderer->DisplayToWorld();
    this->Renderer->GetWorldPoint(worldPosition4);
  }

  double viewDirection_World[3] = { 0.0, 0.0, 0.0 };
  this->GetHandleToCameraVectorWorld(handleWorldPos, viewDirection_World);

  double t = 0; // not used
  double interactionPointOnPlane_World[3] = { 0.0, 0.0, 0.0 };
  double worldPositionAlongView[4] = { worldPosition4[0] + viewDirection_World[0],
                                       worldPosition4[1] + viewDirection_World[1],
                                       worldPosition4[2] + viewDirection_World[2],
                                       1.0 };
  plane->IntersectWithLine(worldPosition4, worldPositionAlongView, t, interactionPointOnPlane_World);

  double closestPointOnRing_World[4] = { 0.0, 0.0, 0.0, 1.0 };
  vtkMath::Subtract(interactionPointOnPlane_World, handleWorldPos, closestPointOnRing_World);
  vtkMath::Normalize(closestPointOnRing_World);

  double radius = INTERACTION_HANDLE_RADIUS;
  if (handleInfo.Index == 3)
  {
    radius *= 1.2;
  }

  vtkMath::MultiplyScalar(closestPointOnRing_World, this->WidgetScale * radius);
  vtkMath::Add(handleWorldPos, closestPointOnRing_World, closestPointOnRing_World);

  double closestPointOnRing_Display4[4] = { 0.0, 0.0, 0.0, 1.0 };
  if (sliceNode)
  {
    vtkNew<vtkMatrix4x4> rasToxyMatrix;
    vtkMatrix4x4::Invert(sliceNode->GetXYToRAS(), rasToxyMatrix);
    rasToxyMatrix->MultiplyPoint(closestPointOnRing_World, closestPointOnRing_Display4);
  }
  else
  {
    this->Renderer->SetWorldPoint(closestPointOnRing_World);
    this->Renderer->WorldToDisplay();
    this->Renderer->GetDisplayPoint(closestPointOnRing_Display4);
  }
  closestPointOnRing_Display4[2] = 0.0;

  double dist2 = vtkMath::Distance2BetweenPoints(displayPosition4, closestPointOnRing_Display4);
  if (dist2 < this->GetMaximumHandlePickingDistance2() && dist2 <= closestDistance2)
  {
    closestDistance2 = dist2;
    foundComponentType = handleInfo.ComponentType;
    foundComponentIndex = handleInfo.Index;
  }
}

//----------------------------------------------------------------------
bool vtkMRMLInteractionWidgetRepresentation::GetTransformationReferencePoint(double referencePointWorld[3])
{
  double origin[3] = { 0.0, 0.0, 0.0 };
  this->GetHandleToWorldTransform()->TransformPoint(origin, referencePointWorld);
  return true;
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::UpdateFromMRML(vtkMRMLNode* vtkNotUsed(caller),
                                                            unsigned long vtkNotUsed(event),
                                                            void* vtkNotUsed(callData))
{
  if (!this->Pipeline)
  {
    this->SetupInteractionPipeline();
  }

  if (this->GetSliceNode())
  {
    this->UpdateSlicePlaneFromSliceNode();
  }

  if (this->Pipeline)
  {
    this->UpdateInteractionPipeline();
  }

  this->NeedToRenderOn();
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::UpdateInteractionPipeline()
{
  this->NeedToRenderOn();
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::UpdateHandleToWorldTransform()
{
  vtkTransform* handleToWorldTransform = this->GetHandleToWorldTransform();
  this->UpdateHandleToWorldTransform(handleToWorldTransform);
  this->OrthoganalizeTransform(handleToWorldTransform);
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::OrthoganalizeTransform(vtkTransform* transform)
{
  double x[3] = { 1.0, 0.0, 0.0 };
  double y[3] = { 0.0, 1.0, 0.0 };
  double z[3] = { 0.0, 0.0, 1.0 };

  transform->TransformVector(x, x);
  transform->TransformVector(y, y);
  transform->TransformVector(z, z);
  vtkMath::Normalize(x);
  vtkMath::Normalize(y);
  vtkMath::Normalize(z);

  double xOrthogonal[3] = { 1.0, 0.0, 0.0 };
  double yOrthogonal[3] = { 0.0, 1.0, 0.0 };
  double zOrthogonal[3] = { z[0], z[1], z[2] };
  vtkMath::Cross(zOrthogonal, x, yOrthogonal);
  vtkMath::Normalize(yOrthogonal);
  vtkMath::Cross(yOrthogonal, zOrthogonal, xOrthogonal);
  vtkMath::Normalize(xOrthogonal);

  vtkNew<vtkMatrix4x4> orthogonalMatrix;
  orthogonalMatrix->DeepCopy(transform->GetMatrix());
  for (int i = 0; i < 3; ++i)
  {
    orthogonalMatrix->SetElement(i, 0, xOrthogonal[i]);
    orthogonalMatrix->SetElement(i, 1, yOrthogonal[i]);
    orthogonalMatrix->SetElement(i, 2, zOrthogonal[i]);
  }

  transform->Identity();
  transform->Concatenate(orthogonalMatrix);
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::UpdateHandlePolyData()
{
  vtkNew<vtkTransform> transform;
  transform->PostMultiply();

  vtkNew<vtkTransformPolyDataFilter> transformArrowGlyph;
  transformArrowGlyph->SetInputData(this->Pipeline->ArrowPolyData);
  transformArrowGlyph->SetTransform(transform);

  vtkNew<vtkTransformPolyDataFilter> transformArrowOutlineGlyph;
  transformArrowOutlineGlyph->SetInputData(this->Pipeline->ArrowOutlinePolyData);
  transformArrowOutlineGlyph->SetTransform(transform);

  vtkNew<vtkTransformPolyDataFilter> transformCircleGlyph;
  transformCircleGlyph->SetInputData(this->Pipeline->CirclePolyData);
  transformCircleGlyph->SetTransform(transform);

  vtkNew<vtkTransformPolyDataFilter> transformCircleOutlineGlyph;
  transformCircleOutlineGlyph->SetInputData(this->Pipeline->CircleOutlinePolyData);
  transformCircleOutlineGlyph->SetTransform(transform);

  vtkNew<vtkTransformPolyDataFilter> transformRingGlyph;
  transformRingGlyph->SetInputData(this->Pipeline->RingPolyData);
  transformRingGlyph->SetTransform(transform);

  vtkNew<vtkTransformPolyDataFilter> transformRingOutlineGlyph;
  transformRingOutlineGlyph->SetInputData(this->Pipeline->RingOutlinePolyData);
  transformRingOutlineGlyph->SetTransform(transform);

  vtkTransformPolyDataFilter* transformGlyph = transformCircleGlyph;
  vtkTransformPolyDataFilter* transformOutlineGlyph = transformCircleOutlineGlyph;

  bool initializeAppendFilter = this->Pipeline->Append->GetInput() == nullptr;
  if (initializeAppendFilter)
  {
    this->Pipeline->Append->RemoveAllInputs();
  }

  HandleInfoList handleInfoList = this->GetHandleInfoList();
  for (HandleInfo handleInfo : handleInfoList)
  {
    switch (handleInfo.GlyphType)
    {
      case GlyphArrow:
        transformGlyph = transformArrowGlyph;
        transformOutlineGlyph = transformArrowOutlineGlyph;
        break;
      case GlyphCircle:
        transformGlyph = transformCircleGlyph;
        transformOutlineGlyph = transformCircleOutlineGlyph;
        break;
      case GlyphRing:
        transformGlyph = transformRingGlyph;
        transformOutlineGlyph = transformRingOutlineGlyph;
        break;
      default:
        break;
    }

    vtkPolyData* handlePolyData = this->GetHandlePolydata(handleInfo.ComponentType);
    if (!handlePolyData)
    {
      continue;
    }

    double point[3] = { 0.0, 0.0, 0.0 };
    handlePolyData->GetPoints()->GetPoint(handleInfo.Index, point);
    if (handleInfo.ApplyScaleToPosition)
    {
      vtkMath::MultiplyScalar(point, this->WidgetScale);
    }

    transform->Identity();
    vtkDoubleArray* orientationArray =
      vtkDoubleArray::SafeDownCast(handlePolyData->GetPointData()->GetArray("orientation"));
    if (orientationArray)
    {
      double orientation[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
      orientationArray->GetTuple(handleInfo.Index, orientation);
      vtkNew<vtkMatrix4x4> orientationMatrix;
      for (int i = 0; i < 9; ++i)
      {
        orientationMatrix->SetElement(i % 3, i / 3, orientation[i]);
      }
      transform->Concatenate(orientationMatrix);
    }

    double scale = this->WidgetScale;
    if (handleInfo.ComponentType == InteractionRotationHandle && handleInfo.Index == 3)
    {
      scale *= 1.2;
    }
    transform->Scale(scale, scale, scale);
    transform->Translate(point[0], point[1], point[2]);

    vtkSmartPointer<vtkPolyData> handleGlyphPolyData = nullptr;
    std::pair<int, int> handlePolyDataMapKey = std::make_pair(handleInfo.ComponentType, handleInfo.Index);
    auto handlePolyDataMapIterator = this->Pipeline->HandleGlyphPolyDataMap.find(handlePolyDataMapKey);
    if (handlePolyDataMapIterator != this->Pipeline->HandleGlyphPolyDataMap.end())
    {
      handleGlyphPolyData = handlePolyDataMapIterator->second;
    }
    else
    {
      handleGlyphPolyData = vtkSmartPointer<vtkPolyData>::New();
      this->Pipeline->HandleGlyphPolyDataMap[handlePolyDataMapKey] = handleGlyphPolyData;
    }

    transformGlyph->SetOutput(handleGlyphPolyData);
    transformGlyph->Update();
    if (initializeAppendFilter)
    {
      this->Pipeline->Append->AddInputData(handleGlyphPolyData);
    }

    vtkSmartPointer<vtkPolyData> handleOutlineGlyphPolyData = nullptr;
    std::pair<int, int> handleOutlinePolyDataMapKey = std::make_pair(handleInfo.ComponentType, handleInfo.Index);
    auto handleOutlinePolyDataMapIterator =
      this->Pipeline->HandleOutlineGlyphPolyDataMap.find(handleOutlinePolyDataMapKey);
    if (handleOutlinePolyDataMapIterator != this->Pipeline->HandleOutlineGlyphPolyDataMap.end())
    {
      handleOutlineGlyphPolyData = handleOutlinePolyDataMapIterator->second;
    }
    else
    {
      handleOutlineGlyphPolyData = vtkSmartPointer<vtkPolyData>::New();
      this->Pipeline->HandleOutlineGlyphPolyDataMap[handleOutlinePolyDataMapKey] = handleOutlineGlyphPolyData;
    }
    transformOutlineGlyph->SetOutput(handleOutlineGlyphPolyData);
    transformOutlineGlyph->Update();
    if (initializeAppendFilter)
    {
      this->Pipeline->Append->AddInputData(handleOutlineGlyphPolyData);
    }

    // Update color arrays
    vtkSmartPointer<vtkFloatArray> handleColorArray =
      vtkFloatArray::SafeDownCast(handlePolyData->GetPointData()->GetAbstractArray("colorIndex"));
    float handleColorIndex = handleColorArray->GetValue(handleInfo.Index);

    // Glyph color array
    vtkSmartPointer<vtkFloatArray> glyphColorArray =
      vtkFloatArray::SafeDownCast(handleGlyphPolyData->GetPointData()->GetAbstractArray("colorIndex"));
    if (!glyphColorArray)
    {
      glyphColorArray = vtkSmartPointer<vtkFloatArray>::New();
      glyphColorArray->SetName("colorIndex");
      handleGlyphPolyData->GetPointData()->AddArray(glyphColorArray);
    }
    glyphColorArray->SetNumberOfComponents(1);
    glyphColorArray->SetNumberOfTuples(handleGlyphPolyData->GetNumberOfPoints());
    glyphColorArray->Fill(handleColorIndex);
    handleGlyphPolyData->GetPointData()->SetActiveScalars("colorIndex");

    // Glyph outline color array
    vtkSmartPointer<vtkFloatArray> outlineGlyphColorArray =
      vtkFloatArray::SafeDownCast(handleOutlineGlyphPolyData->GetPointData()->GetAbstractArray("colorIndex"));
    if (!outlineGlyphColorArray)
    {
      outlineGlyphColorArray = vtkSmartPointer<vtkFloatArray>::New();
      outlineGlyphColorArray->SetName("colorIndex");
      handleOutlineGlyphPolyData->GetPointData()->AddArray(outlineGlyphColorArray);
    }
    outlineGlyphColorArray->SetNumberOfComponents(1);
    outlineGlyphColorArray->SetNumberOfTuples(handleOutlineGlyphPolyData->GetNumberOfPoints());
    outlineGlyphColorArray->Fill(handleColorIndex + 1);
    handleOutlineGlyphPolyData->GetPointData()->SetActiveScalars("colorIndex");
  }
}

//----------------------------------------------------------------------
vtkPointPlacer* vtkMRMLInteractionWidgetRepresentation::GetPointPlacer()
{
  return this->PointPlacer;
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::GetActors(vtkPropCollection* pc)
{
  vtkProp* actor = this->GetInteractionActor();
  if (actor)
  {
    actor->GetActors(pc);
  }
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::ReleaseGraphicsResources(vtkWindow* window)
{
  vtkProp* actor = this->GetInteractionActor();
  if (actor)
  {
    actor->ReleaseGraphicsResources(window);
  }
}

//----------------------------------------------------------------------
int vtkMRMLInteractionWidgetRepresentation::RenderOverlay(vtkViewport* viewport)
{
  int count = 0;
  vtkProp* actor = this->GetInteractionActor();
  if (this->Pipeline && actor->GetVisibility())
  {
    count += actor->RenderOverlay(viewport);
  }
  return count;
}

//----------------------------------------------------------------------
int vtkMRMLInteractionWidgetRepresentation::RenderOpaqueGeometry(vtkViewport* viewport)
{
  if (!this->Pipeline)
  {
    this->SetupInteractionPipeline();
  }

  int count = 0;
  vtkProp* actor = this->GetInteractionActor();
  if (actor && actor->GetVisibility())
  {
    this->UpdateHandleToWorldTransform();
    this->UpdateSlicePlaneFromSliceNode();
    this->UpdateHandleColors();
    this->UpdateViewScaleFactor();
    this->UpdateInteractionPipeline();
    this->UpdateHandleOrientation();
    this->UpdateHandleSize();
    this->UpdateHandlePolyData();
    count += actor->RenderOpaqueGeometry(viewport);
    this->Pipeline->HandleToWorldTransformFilter->Update();
  }
  return count;
}

//----------------------------------------------------------------------
int vtkMRMLInteractionWidgetRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
{
  int count = 0;
  vtkProp* actor = this->GetInteractionActor();
  if (actor && actor->GetVisibility())
  {
    actor->SetPropertyKeys(this->GetPropertyKeys());
    count += actor->RenderTranslucentPolygonalGeometry(viewport);
  }
  return count;
}

//----------------------------------------------------------------------
vtkTypeBool vtkMRMLInteractionWidgetRepresentation::HasTranslucentPolygonalGeometry()
{
  vtkProp* actor = this->GetInteractionActor();
  if (actor && actor->GetVisibility() && actor->HasTranslucentPolygonalGeometry())
  {
    return true;
  }
  return false;
}

//----------------------------------------------------------------------
vtkMRMLInteractionWidgetRepresentation::InteractionPipeline::InteractionPipeline()
{
  /// Rotation pipeline
  this->RotationHandlePoints = vtkSmartPointer<vtkPolyData>::New();

  vtkNew<vtkEllipseArcSource> outerArcSource;
  outerArcSource->SetMajorRadiusVector(-INTERACTION_HANDLE_ROTATION_ARC_OUTER_RADIUS, 0.0, 0.0);
  outerArcSource->SetResolution(INTERACTION_HANDLE_ROTATION_ARC_RESOLUTION);
  outerArcSource->SetCenter(0.0, 0.0, 0.0);
  outerArcSource->SetNormal(0.0, 0.0, 1.0);
  outerArcSource->SetRatio(1.0);
  outerArcSource->SetStartAngle(180 - INTERACTION_HANDLE_ROTATION_ARC_DEGREES / 2.0);
  outerArcSource->SetSegmentAngle(INTERACTION_HANDLE_ROTATION_ARC_DEGREES);
  outerArcSource->Update();

  vtkNew<vtkEllipseArcSource> innerArcSource;
  innerArcSource->SetMajorRadiusVector(-INTERACTION_HANDLE_ROTATION_ARC_INNER_RADIUS, 0.0, 0.0);
  innerArcSource->SetResolution(INTERACTION_HANDLE_ROTATION_ARC_RESOLUTION);
  innerArcSource->SetCenter(0.0, 0.0, 0.0);
  innerArcSource->SetNormal(0.0, 0.0, 1.0);
  innerArcSource->SetRatio(1.0);
  innerArcSource->SetStartAngle(180 - INTERACTION_HANDLE_ROTATION_ARC_DEGREES / 2.0);
  innerArcSource->SetSegmentAngle(INTERACTION_HANDLE_ROTATION_ARC_DEGREES);
  innerArcSource->Update();

  vtkNew<vtkPoints> rotationPts;

  this->RingOutlinePolyData = vtkSmartPointer<vtkPolyData>::New();
  this->RingOutlinePolyData->SetPoints(rotationPts);
  this->RingOutlinePolyData->SetLines(vtkNew<vtkCellArray>());

  this->RingPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->RingPolyData->SetPoints(rotationPts);
  this->RingPolyData->SetPolys(vtkNew<vtkCellArray>());

  if (INTERACTION_HANDLE_ROTATION_ARC_DEGREES < 360.0)
  {
    vtkNew<vtkIdList> rotationPoly;
    vtkNew<vtkIdList> rotationLine;

    for (int i = 0; i < outerArcSource->GetOutput()->GetNumberOfPoints(); ++i)
    {
      double point[3];
      outerArcSource->GetOutput()->GetPoint(i, point);
      vtkIdType id = rotationPts->InsertNextPoint(point);
      rotationPoly->InsertNextId(id);
      rotationLine->InsertNextId(id);
    }
    for (int i = innerArcSource->GetOutput()->GetNumberOfPoints() - 1; i >= 0; --i)
    {
      double point[3];
      innerArcSource->GetOutput()->GetPoint(i, point);
      vtkIdType id = rotationPts->InsertNextPoint(point);
      rotationPoly->InsertNextId(id);
      rotationLine->InsertNextId(id);
    }
    rotationLine->InsertNextId(0);
    this->ArrowOutlinePolyData->InsertNextCell(VTK_POLY_LINE, rotationLine);
    this->ArrowPolyData->InsertNextCell(VTK_POLYGON, rotationPoly);
  }
  else
  {
    vtkNew<vtkCellArray> rotationTriangles;
    vtkNew<vtkIdList> outerLine;
    vtkNew<vtkIdList> innerLine;

    vtkIdType previousInnerPoint = -1;
    vtkIdType previousOuterPoint = -1;
    for (int index = 0; index < outerArcSource->GetOutput()->GetNumberOfPoints(); ++index)
    {
      double outerLinePoint[3] = { 0.0, 0.0, 0.0 };
      outerArcSource->GetOutput()->GetPoint(index, outerLinePoint);
      vtkIdType outerPointId = rotationPts->InsertNextPoint(outerLinePoint);
      outerLine->InsertNextId(outerPointId);

      double innerLinePoint[3] = { 0.0, 0.0, 0.0 };
      innerArcSource->GetOutput()->GetPoint(index, innerLinePoint);
      vtkIdType innerPointId = rotationPts->InsertNextPoint(innerLinePoint);
      innerLine->InsertNextId(innerPointId);

      if (previousInnerPoint >= 0 && previousOuterPoint >= 0)
      {
        vtkNew<vtkIdList> rotationTriangleA;
        rotationTriangleA->InsertNextId(previousInnerPoint);
        rotationTriangleA->InsertNextId(previousOuterPoint);
        rotationTriangleA->InsertNextId(outerPointId);
        rotationTriangles->InsertNextCell(rotationTriangleA);

        vtkNew<vtkIdList> rotationTriangleB;
        rotationTriangleB->InsertNextId(previousInnerPoint);
        rotationTriangleB->InsertNextId(outerPointId);
        rotationTriangleB->InsertNextId(innerPointId);
        rotationTriangles->InsertNextCell(rotationTriangleB);
      }
      previousInnerPoint = innerPointId;
      previousOuterPoint = outerPointId;
    }

    if (previousInnerPoint > 0 && previousOuterPoint > 0)
    {
      vtkNew<vtkIdList> rotationTriangleA;
      rotationTriangleA->InsertNextId(previousInnerPoint);
      rotationTriangleA->InsertNextId(previousOuterPoint);
      rotationTriangleA->InsertNextId(0);
      rotationTriangles->InsertNextCell(rotationTriangleA);

      vtkNew<vtkIdList> rotationTriangleB;
      rotationTriangleB->InsertNextId(previousInnerPoint);
      rotationTriangleB->InsertNextId(0);
      rotationTriangleB->InsertNextId(1);
      rotationTriangles->InsertNextCell(rotationTriangleB);
    }

    this->RingOutlinePolyData->InsertNextCell(VTK_POLY_LINE, outerLine);
    this->RingOutlinePolyData->InsertNextCell(VTK_POLY_LINE, innerLine);
    this->RingPolyData->SetPolys(rotationTriangles);
  }

  vtkNew<vtkPoints> translationHandlePoints;
  vtkNew<vtkIdList> translationHandlePoly;
  translationHandlePoly->InsertNextId(translationHandlePoints->InsertNextPoint(0.00, 0.00, 0.00));
  translationHandlePoly->InsertNextId(translationHandlePoints->InsertNextPoint(
    -INTERACTION_TRANSLATION_HANDLE_TIP_RADIUS, INTERACTION_TRANSLATION_HANDLE_TIP_RADIUS, 0.00));
  translationHandlePoly->InsertNextId(translationHandlePoints->InsertNextPoint(
    -INTERACTION_TRANSLATION_HANDLE_TIP_RADIUS, INTERACTION_TRANSLATION_HANDLE_SHAFT_RADIUS, 0.00));
  translationHandlePoly->InsertNextId(translationHandlePoints->InsertNextPoint(
    -INTERACTION_TRANSLATION_HANDLE_LENGTH, INTERACTION_TRANSLATION_HANDLE_SHAFT_RADIUS, 0.00));
  translationHandlePoly->InsertNextId(translationHandlePoints->InsertNextPoint(
    -INTERACTION_TRANSLATION_HANDLE_LENGTH, -INTERACTION_TRANSLATION_HANDLE_SHAFT_RADIUS, 0.00));
  translationHandlePoly->InsertNextId(translationHandlePoints->InsertNextPoint(
    -INTERACTION_TRANSLATION_HANDLE_TIP_RADIUS, -INTERACTION_TRANSLATION_HANDLE_SHAFT_RADIUS, 0.00));
  translationHandlePoly->InsertNextId(translationHandlePoints->InsertNextPoint(
    -INTERACTION_TRANSLATION_HANDLE_TIP_RADIUS, -INTERACTION_TRANSLATION_HANDLE_TIP_RADIUS, 0.00));

  vtkNew<vtkIdList> translationHandleLine;
  translationHandleLine->DeepCopy(translationHandlePoly);
  translationHandleLine->InsertNextId(0);

  this->ArrowPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->ArrowPolyData->SetPoints(translationHandlePoints);
  this->ArrowPolyData->SetPolys(vtkNew<vtkCellArray>());
  this->ArrowPolyData->InsertNextCell(VTK_POLYGON, translationHandlePoly);

  this->ArrowOutlinePolyData = vtkSmartPointer<vtkPolyData>::New();
  this->ArrowOutlinePolyData->SetPoints(translationHandlePoints);
  this->ArrowOutlinePolyData->SetLines(vtkNew<vtkCellArray>());
  this->ArrowOutlinePolyData->InsertNextCell(VTK_POLY_LINE, translationHandleLine);

  /// Translation pipeline
  this->TranslationHandlePoints = vtkSmartPointer<vtkPolyData>::New();

  /// Scale pipeline
  vtkNew<vtkEllipseArcSource> scaleArcSource;
  scaleArcSource->SetMajorRadiusVector(INTERACTION_HANDLE_SCALE_RADIUS, 0.0, 0.0);
  scaleArcSource->SetResolution(100);
  scaleArcSource->SetCenter(0.0, 0.0, 0.0);
  scaleArcSource->SetNormal(0.0, 0.0, 1.0);
  scaleArcSource->SetRatio(1.0);
  scaleArcSource->SetStartAngle(0);
  scaleArcSource->SetSegmentAngle(360);
  scaleArcSource->Update();

  vtkNew<vtkPoints> scalePoints;
  vtkNew<vtkIdList> scalePoly;
  vtkNew<vtkIdList> scaleLine;
  for (int i = 0; i < scaleArcSource->GetOutput()->GetNumberOfPoints(); ++i)
  {
    double point[3];
    scaleArcSource->GetOutput()->GetPoint(i, point);
    vtkIdType id = scalePoints->InsertNextPoint(point);
    scalePoly->InsertNextId(id);
    scaleLine->InsertNextId(id);
  }
  scaleLine->InsertNextId(0);

  this->CirclePolyData = vtkSmartPointer<vtkPolyData>::New();
  this->CirclePolyData->SetPoints(scalePoints);
  this->CirclePolyData->SetPolys(vtkNew<vtkCellArray>());
  this->CirclePolyData->InsertNextCell(VTK_POLYGON, scalePoly);

  this->CircleOutlinePolyData = vtkSmartPointer<vtkPolyData>::New();
  this->CircleOutlinePolyData->SetPoints(scalePoints);
  this->CircleOutlinePolyData->SetLines(vtkNew<vtkCellArray>());
  this->CircleOutlinePolyData->InsertNextCell(VTK_POLY_LINE, scaleLine);

  this->ScaleHandlePoints = vtkSmartPointer<vtkPolyData>::New();

  this->Append = vtkSmartPointer<vtkAppendPolyData>::New();

  this->HandleToWorldTransform = vtkSmartPointer<vtkTransform>::New();
  this->HandleToWorldTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->HandleToWorldTransformFilter->SetInputConnection(this->Append->GetOutputPort());
  this->HandleToWorldTransformFilter->SetTransform(this->HandleToWorldTransform);

  this->ColorTable = vtkSmartPointer<vtkLookupTable>::New();

  this->Mapper3D = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->Mapper3D->SetInputConnection(this->HandleToWorldTransformFilter->GetOutputPort());
  this->Mapper3D->SetColorModeToMapScalars();
  this->Mapper3D->ColorByArrayComponent("colorIndex", 0);
  this->Mapper3D->SetLookupTable(this->ColorTable);
  this->Mapper3D->ScalarVisibilityOn();
  this->Mapper3D->UseLookupTableScalarRangeOn();

  this->Property3D = vtkSmartPointer<vtkProperty>::New();
  this->Property3D->SetPointSize(
    1.e-6); // NOTE: The point size value must be greater than zero. Refer to vtkOpenGLState::vtkglPointSize(float).
  this->Property3D->SetLineWidth(2.0);
  this->Property3D->SetDiffuse(0.0);
  this->Property3D->SetAmbient(1.0);
  this->Property3D->SetMetallic(0.0);
  this->Property3D->SetSpecular(0.0);
  this->Property3D->SetEdgeVisibility(false);
  this->Property3D->SetOpacity(1.0);

  this->Actor3D = vtkSmartPointer<vtkActor>::New();
  this->Actor3D->SetProperty(this->Property3D);
  this->Actor3D->SetMapper(this->Mapper3D);

  this->WorldToSliceTransform = vtkSmartPointer<vtkTransform>::New();

  this->WorldToSliceTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->WorldToSliceTransformFilter->SetTransform(this->WorldToSliceTransform);
}

//----------------------------------------------------------------------
vtkMRMLInteractionWidgetRepresentation::InteractionPipeline::~InteractionPipeline() = default;

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::InitializePipeline()
{
  if (vtkMapper::GetResolveCoincidentTopology() != VTK_RESOLVE_POLYGON_OFFSET)
  {
    vtkGenericWarningMacro(
      "Unexpected resolve coincident topology value: " << vtkMapper::GetResolveCoincidentTopology());
  }
  this->UpdateRelativeCoincidentTopologyOffsets(this->Pipeline->Mapper3D);

  this->CreateRotationHandles();
  this->CreateTranslationHandles();
  this->CreateScaleHandles();
  this->UpdateHandleColors();
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::CreateRotationHandles()
{
  vtkNew<vtkPoints> points;

  double xRotationHandle[3] = { 0.0, 0.0, 0.0 }; // X-axis
  points->InsertNextPoint(xRotationHandle);
  double yRotationHandle[3] = { 0.0, 0.0, 0.0 }; // Y-axis
  vtkMath::Normalize(yRotationHandle);
  points->InsertNextPoint(yRotationHandle);
  double zRotationHandle[3] = { 0.0, 0.0, 0.0 }; // Z-axis
  vtkMath::Normalize(zRotationHandle);
  points->InsertNextPoint(zRotationHandle);
  this->Pipeline->RotationHandlePoints->SetPoints(points);
  double viewPlaneRotationHandle[3] = { 0.0, 0.0, 0.0 }; // View
  this->Pipeline->RotationHandlePoints->GetPoints()->InsertNextPoint(viewPlaneRotationHandle);

  vtkNew<vtkDoubleArray> orientationArray;
  orientationArray->SetName("orientation");
  orientationArray->SetNumberOfComponents(9);
  vtkNew<vtkTransform> xRotationOrientation;
  xRotationOrientation->RotateX(90);
  xRotationOrientation->RotateY(90);
  xRotationOrientation->RotateZ(45);
  vtkMatrix4x4* xRotationMatrix = xRotationOrientation->GetMatrix();
  orientationArray->InsertNextTuple9(xRotationMatrix->GetElement(0, 0),
                                     xRotationMatrix->GetElement(1, 0),
                                     xRotationMatrix->GetElement(2, 0),
                                     xRotationMatrix->GetElement(0, 1),
                                     xRotationMatrix->GetElement(1, 1),
                                     xRotationMatrix->GetElement(2, 1),
                                     xRotationMatrix->GetElement(0, 2),
                                     xRotationMatrix->GetElement(1, 2),
                                     xRotationMatrix->GetElement(2, 2));
  vtkNew<vtkTransform> yRotationOrientation;
  yRotationOrientation->RotateX(90);
  yRotationOrientation->RotateZ(45);
  vtkMatrix4x4* yRotationMatrix = yRotationOrientation->GetMatrix();
  orientationArray->InsertNextTuple9(yRotationMatrix->GetElement(0, 0),
                                     yRotationMatrix->GetElement(1, 0),
                                     yRotationMatrix->GetElement(2, 0),
                                     yRotationMatrix->GetElement(0, 1),
                                     yRotationMatrix->GetElement(1, 1),
                                     yRotationMatrix->GetElement(2, 1),
                                     yRotationMatrix->GetElement(0, 2),
                                     yRotationMatrix->GetElement(1, 2),
                                     yRotationMatrix->GetElement(2, 2));
  vtkNew<vtkTransform> zRotationOrientation;
  zRotationOrientation->RotateZ(45);
  vtkMatrix4x4* zRotationMatrix = zRotationOrientation->GetMatrix();
  orientationArray->InsertNextTuple9(zRotationMatrix->GetElement(0, 0),
                                     zRotationMatrix->GetElement(1, 0),
                                     zRotationMatrix->GetElement(2, 0),
                                     zRotationMatrix->GetElement(0, 1),
                                     zRotationMatrix->GetElement(1, 1),
                                     zRotationMatrix->GetElement(2, 1),
                                     zRotationMatrix->GetElement(0, 2),
                                     zRotationMatrix->GetElement(1, 2),
                                     zRotationMatrix->GetElement(2, 2));

  orientationArray->InsertNextTuple9(1, 0, 0, 0, 1, 0, 0, 0, 1); // View
  this->Pipeline->RotationHandlePoints->GetPointData()->AddArray(orientationArray);

  vtkNew<vtkIdTypeArray> visibilityArray;
  visibilityArray->SetName("visibility");
  visibilityArray->SetNumberOfComponents(1);
  visibilityArray->SetNumberOfValues(this->Pipeline->RotationHandlePoints->GetNumberOfPoints());
  visibilityArray->Fill(1);
  this->Pipeline->RotationHandlePoints->GetPointData()->AddArray(visibilityArray);
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::CreateTranslationHandles()
{
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(1.0, 0.0, 0.0); // X-axis
  points->InsertNextPoint(0.0, 1.0, 0.0); // Y-axis
  points->InsertNextPoint(0.0, 0.0, 1.0); // Z-axis
  points->InsertNextPoint(0.0, 0.0, 0.0); // Free translation
  this->Pipeline->TranslationHandlePoints->SetPoints(points);

  vtkNew<vtkDoubleArray> orientationArray;
  orientationArray->SetName("orientation");
  orientationArray->SetNumberOfComponents(9);
  orientationArray->InsertNextTuple9(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
  orientationArray->InsertNextTuple9(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
  orientationArray->InsertNextTuple9(1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
  orientationArray->InsertNextTuple9(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0); // Free translation
  this->Pipeline->TranslationHandlePoints->GetPointData()->AddArray(orientationArray);

  vtkNew<vtkIdTypeArray> visibilityArray;
  visibilityArray->SetName("visibility");
  visibilityArray->SetNumberOfComponents(1);
  visibilityArray->SetNumberOfValues(this->Pipeline->TranslationHandlePoints->GetNumberOfPoints());
  visibilityArray->Fill(1);
  this->Pipeline->TranslationHandlePoints->GetPointData()->AddArray(visibilityArray);
}

//----------------------------------------------------------------------
vtkProp* vtkMRMLInteractionWidgetRepresentation::GetInteractionActor()
{
  if (!this->Pipeline)
  {
    return nullptr;
  }
  return this->Pipeline->Actor3D;
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::UpdateHandleOrientation()
{
  this->UpdateTranslationHandleOrientation();
  this->UpdateScaleHandleOrientation();
  this->UpdateRotationHandleOrientation();
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::UpdateTranslationHandleOrientation()
{
  vtkPolyData* handlePolyData = this->GetHandlePolydata(InteractionTranslationHandle);
  if (!handlePolyData)
  {
    return;
  }

  vtkDoubleArray* orientationArray =
    vtkDoubleArray::SafeDownCast(handlePolyData->GetPointData()->GetAbstractArray("orientation"));
  if (!orientationArray)
  {
    return;
  }

  vtkSmartPointer<vtkTransform> worldToHandleTransform =
    vtkTransform::SafeDownCast(this->Pipeline->HandleToWorldTransform->GetInverse());
  vtkCamera* camera = this->GetRenderer()->GetActiveCamera();

  double viewDirection_World[3] = { 0.0, 0.0, 0.0 };
  double viewDirection_Handle[3] = { 0.0, 0.0, 0.0 };
  double viewUp_World[3] = { 0.0, 1.0, 0.0 };
  double viewUp_Handle[3] = { 0.0, 1.0, 0.0 };
  if (this->GetSliceNode())
  {
    double viewUp[4] = { 0.0, 1.0, 0.0, 0.0 };
    double viewUp2[4] = { 0.0, 1.0, 0.0, 0.0 };
    this->GetSliceNode()->GetXYToRAS()->MultiplyPoint(viewUp, viewUp2);
    viewUp_World[0] = viewUp2[0];
    viewUp_World[1] = viewUp2[1];
    viewUp_World[2] = viewUp2[2];
  }
  else
  {
    camera->GetViewUp(viewUp_World);
  }
  worldToHandleTransform->TransformVector(viewUp_World, viewUp_Handle);

  for (int i = 0; i < 3; ++i)
  {
    double interactionHandlePosition[3] = { 0.0, 0.0, 0.0 };
    this->GetInteractionHandlePositionWorld(InteractionTranslationHandle, i, interactionHandlePosition);
    this->GetHandleToCameraVectorWorld(interactionHandlePosition, viewDirection_World);
    worldToHandleTransform->TransformVector(viewDirection_World, viewDirection_Handle);

    double xAxis[3] = { 0.0, 0.0, 0.0 };
    double yAxis[3] = { 0.0, 0.0, 0.0 };
    double zAxis[3] = { 0.0, 0.0, 0.0 };
    xAxis[i] = 1.0;

    vtkMath::Cross(viewDirection_Handle, xAxis, yAxis);
    vtkMath::Normalize(yAxis);

    vtkMath::Cross(xAxis, yAxis, zAxis);
    vtkMath::Normalize(zAxis);

    orientationArray->SetTuple9(
      i, xAxis[0], xAxis[1], xAxis[2], yAxis[0], yAxis[1], yAxis[2], zAxis[0], zAxis[1], zAxis[2]);
  }

  double interactionHandlePosition[3] = { 0.0, 0.0, 0.0 };
  this->GetInteractionHandlePositionWorld(InteractionTranslationHandle, 3, interactionHandlePosition);
  this->GetHandleToCameraVectorWorld(interactionHandlePosition, viewDirection_World);
  worldToHandleTransform->TransformVector(viewDirection_World, viewDirection_Handle);

  double xAxis[3] = { 0.0, 0.0, 0.0 };
  double yAxis[3] = { 0.0, 0.0, 0.0 };
  double zAxis[3] = { 0.0, 0.0, 0.0 };
  zAxis[0] = viewDirection_Handle[0];
  zAxis[1] = viewDirection_Handle[1];
  zAxis[2] = viewDirection_Handle[2];
  vtkMath::Normalize(zAxis);

  vtkMath::Cross(viewUp_Handle, viewDirection_Handle, xAxis);
  vtkMath::Normalize(xAxis);

  vtkMath::Cross(viewDirection_Handle, xAxis, yAxis);
  vtkMath::Normalize(yAxis);

  orientationArray->SetTuple9(
    3, xAxis[0], xAxis[1], xAxis[2], yAxis[0], yAxis[1], yAxis[2], zAxis[0], zAxis[1], zAxis[2]);
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::UpdateScaleHandleOrientation()
{
  vtkPolyData* handlePolyData = this->GetHandlePolydata(InteractionScaleHandle);
  if (!handlePolyData)
  {
    return;
  }

  vtkDoubleArray* orientationArray =
    vtkDoubleArray::SafeDownCast(handlePolyData->GetPointData()->GetAbstractArray("orientation"));
  if (!orientationArray)
  {
    return;
  }

  vtkSmartPointer<vtkTransform> worldToHandleTransform =
    vtkTransform::SafeDownCast(this->Pipeline->HandleToWorldTransform->GetInverse());
  vtkCamera* camera = this->GetRenderer()->GetActiveCamera();

  double viewDirection_World[3] = { 0.0, 0.0, 0.0 };
  double viewDirection_Handle[3] = { 0.0, 0.0, 0.0 };
  double viewUp_World[3] = { 0.0, 1.0, 0.0 };
  double viewUp_Handle[3] = { 0.0, 0.0, 0.0 };
  if (this->GetSliceNode())
  {
    double viewUp[4] = { 0.0, 1.0, 0.0, 0.0 };
    double viewUp2[4] = { 0.0, 1.0, 0.0, 0.0 };
    this->GetSliceNode()->GetXYToRAS()->MultiplyPoint(viewUp, viewUp2);
    viewUp_World[0] = viewUp2[0];
    viewUp_World[1] = viewUp2[1];
    viewUp_World[2] = viewUp2[2];
  }
  else
  {
    camera->GetViewUp(viewUp_World);
  }
  worldToHandleTransform->TransformVector(viewUp_World, viewUp_Handle);

  for (int i = 0; i < orientationArray->GetNumberOfTuples(); ++i)
  {
    double interactionHandlePosition[3] = { 0.0, 0.0, 0.0 };
    this->GetInteractionHandlePositionWorld(InteractionScaleHandle, i, interactionHandlePosition);
    this->GetHandleToCameraVectorWorld(interactionHandlePosition, viewDirection_World);
    worldToHandleTransform->TransformVector(viewDirection_World, viewDirection_Handle);

    double xAxis[3] = { 1.0, 0.0, 0.0 };
    double yAxis[3] = { 0.0, 1.0, 0.0 };
    double zAxis[3] = { 0.0, 0.0, 1.0 };
    zAxis[0] = viewDirection_Handle[0];
    zAxis[1] = viewDirection_Handle[1];
    zAxis[2] = viewDirection_Handle[2];
    vtkMath::Normalize(zAxis);

    vtkMath::Cross(viewUp_Handle, viewDirection_Handle, xAxis);
    vtkMath::Normalize(xAxis);

    vtkMath::Cross(viewDirection_Handle, xAxis, yAxis);
    vtkMath::Normalize(yAxis);

    orientationArray->SetTuple9(
      i, xAxis[0], xAxis[1], xAxis[2], yAxis[0], yAxis[1], yAxis[2], zAxis[0], zAxis[1], zAxis[2]);
  }
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::UpdateRotationHandleOrientation()
{
  vtkPolyData* handlePolyData = this->GetHandlePolydata(InteractionRotationHandle);
  if (!handlePolyData)
  {
    return;
  }

  vtkDoubleArray* orientationArray =
    vtkDoubleArray::SafeDownCast(handlePolyData->GetPointData()->GetAbstractArray("orientation"));
  if (!orientationArray)
  {
    return;
  }

  double viewDirection_World[3] = { 0.0, 0.0, 0.0 };
  double viewUp_World[3] = { 0.0, 1.0, 0.0 };
  if (this->GetSliceNode())
  {
    this->SlicePlane->GetNormal(viewDirection_World);
    double viewup[4] = { 0.0, 1.0, 0.0, 0.0 };
    double viewup2[4] = { 0.0, 1.0, 0.0, 0.0 };
    this->GetSliceNode()->GetXYToRAS()->MultiplyPoint(viewup, viewup2);
    viewUp_World[0] = viewup2[0];
    viewUp_World[1] = viewup2[1];
    viewUp_World[2] = viewup2[2];
  }
  else
  {
    double handlePosition_World[3] = { 0.0, 0.0, 0.0 };
    this->GetInteractionHandlePositionWorld(InteractionRotationHandle, 3, handlePosition_World);
    this->GetHandleToCameraVectorWorld(handlePosition_World, viewDirection_World);

    vtkCamera* camera = this->GetRenderer()->GetActiveCamera();
    camera->GetViewUp(viewUp_World);
  }

  vtkSmartPointer<vtkTransform> worldToHandleTransform =
    vtkTransform::SafeDownCast(this->Pipeline->HandleToWorldTransform->GetInverse());

  double viewDirection_Handle[3] = { 0.0, 0.0, 0.0 };
  worldToHandleTransform->TransformVector(viewDirection_World, viewDirection_Handle);

  double viewUp_Handle[3] = { 0.0, 0.0, 0.0 };
  worldToHandleTransform->TransformVector(viewUp_World, viewUp_Handle);

  double xAxis[3] = { 1.0, 0.0, 0.0 };
  double yAxis[3] = { 0.0, 1.0, 0.0 };
  double zAxis[3] = { 0.0, 0.0, 1.0 };
  zAxis[0] = viewDirection_Handle[0];
  zAxis[1] = viewDirection_Handle[1];
  zAxis[2] = viewDirection_Handle[2];
  vtkMath::Normalize(zAxis);

  vtkMath::Cross(viewUp_Handle, viewDirection_Handle, xAxis);
  vtkMath::Normalize(xAxis);

  vtkMath::Cross(viewDirection_Handle, xAxis, yAxis);
  vtkMath::Normalize(yAxis);

  orientationArray->SetTuple9(
    3, xAxis[0], xAxis[1], xAxis[2], yAxis[0], yAxis[1], yAxis[2], zAxis[0], zAxis[1], zAxis[2]);
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::CreateScaleHandles()
{
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(INTERACTION_HANDLE_SCALE_DISTANCE_FROM_CENTER, 0.0, 0.0); // X-axis
  points->InsertNextPoint(0.0, INTERACTION_HANDLE_SCALE_DISTANCE_FROM_CENTER, 0.0); // Y-axis
  points->InsertNextPoint(0.0, 0.0, INTERACTION_HANDLE_SCALE_DISTANCE_FROM_CENTER); // Z-axis
  this->Pipeline->ScaleHandlePoints->SetPoints(points);

  vtkNew<vtkDoubleArray> orientationArray;
  orientationArray->SetName("orientation");
  orientationArray->SetNumberOfComponents(9);
  orientationArray->InsertNextTuple9(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
  orientationArray->InsertNextTuple9(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
  orientationArray->InsertNextTuple9(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
  orientationArray->InsertNextTuple9(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0); // Free translation
  this->Pipeline->ScaleHandlePoints->GetPointData()->AddArray(orientationArray);

  vtkNew<vtkIdTypeArray> visibilityArray;
  visibilityArray->SetName("visibility");
  visibilityArray->SetNumberOfComponents(1);
  visibilityArray->SetNumberOfValues(this->Pipeline->ScaleHandlePoints->GetNumberOfPoints());
  visibilityArray->Fill(1);
  this->Pipeline->ScaleHandlePoints->GetPointData()->AddArray(visibilityArray);
}

//----------------------------------------------------------------------
int vtkMRMLInteractionWidgetRepresentation::GetNumberOfHandles()
{
  int numberOfHandles = 0;
  for (int type = InteractionNone + 1; type < Interaction_Last; ++type)
  {
    numberOfHandles += this->GetNumberOfHandles(type);
  }
  return numberOfHandles;
}

//----------------------------------------------------------------------
int vtkMRMLInteractionWidgetRepresentation::GetNumberOfHandles(int type)
{
  vtkPolyData* handlePolyData = this->GetHandlePolydata(type);
  if (!handlePolyData)
  {
    vtkErrorMacro("GetNumberOfHandles: Invalid handle type: " << type);
    return 0;
  }
  return handlePolyData->GetNumberOfPoints();
}

//----------------------------------------------------------------------
vtkPolyData* vtkMRMLInteractionWidgetRepresentation::GetHandlePolydata(int type)
{
  if (type == InteractionRotationHandle)
  {
    return this->Pipeline->RotationHandlePoints;
  }
  else if (type == InteractionTranslationHandle)
  {
    return this->Pipeline->TranslationHandlePoints;
  }
  else if (type == InteractionScaleHandle)
  {
    return this->Pipeline->ScaleHandlePoints;
  }
  return nullptr;
}

//----------------------------------------------------------------------
int vtkMRMLInteractionWidgetRepresentation::UpdateHandleColors(int type, int colorIndex)
{
  vtkPolyData* handlePolyData = this->GetHandlePolydata(type);
  if (!handlePolyData)
  {
    return colorIndex;
  }

  vtkPointData* pointData = handlePolyData->GetPointData();
  if (!pointData)
  {
    return colorIndex;
  }

  vtkSmartPointer<vtkFloatArray> colorArray = vtkFloatArray::SafeDownCast(pointData->GetAbstractArray("colorIndex"));
  if (!colorArray)
  {
    colorArray = vtkSmartPointer<vtkFloatArray>::New();
    colorArray->SetName("colorIndex");
    colorArray->SetNumberOfComponents(1);
    pointData->AddArray(colorArray);
    pointData->SetActiveScalars("colorIndex");
  }
  colorArray->Initialize();
  colorArray->SetNumberOfTuples(handlePolyData->GetNumberOfPoints() * 2);

  double color[4] = { 0.0, 0.0, 0.0, 0.0 };
  for (int i = 0; i < handlePolyData->GetNumberOfPoints(); ++i)
  {
    this->GetHandleColor(type, i, color);
    this->Pipeline->ColorTable->SetTableValue(colorIndex, color);
    colorArray->SetTuple1(i, colorIndex);
    ++colorIndex;

    double grey = 0.3;
    bool selected = this->GetActiveComponentType() == type && this->GetActiveComponentIndex() == i;
    if (selected)
    {
      grey = 0.0;
    }
    this->Pipeline->ColorTable->SetTableValue(colorIndex, grey, grey, grey, color[3]);
    ++colorIndex;
  }

  return colorIndex;
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::UpdateHandleColors()
{
  int numberOfHandles = this->GetNumberOfHandles();
  int numberOfColors = numberOfHandles * 2; // alternate fill and outline colors
  numberOfColors += 2;                      // Additional colors for minimal fill.

  this->Pipeline->ColorTable->SetNumberOfTableValues(numberOfColors);
  this->Pipeline->ColorTable->SetTableRange(0, double(numberOfColors) - 1);

  int colorIndex = 0;
  colorIndex = this->UpdateHandleColors(InteractionRotationHandle, colorIndex);
  colorIndex = this->UpdateHandleColors(InteractionTranslationHandle, colorIndex);
  colorIndex = this->UpdateHandleColors(InteractionScaleHandle, colorIndex);

  this->Pipeline->ColorTable->Build();
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::GetHandleColor(int type, int index, double color[4])
{
  if (!color)
  {
    return;
  }

  double red[3] = { 0.80, 0.35, 0.35 };
  double redSelected[3] = { 0.70, 0.07, 0.07 };

  double green[3] = { 0.35, 0.70, 0.35 };
  double greenSelected[3] = { 0.00, 0.50, 0.00 };

  double blue[3] = { 0.35, 0.35, 0.8 };
  double blueSelected[3] = { 0.07, 0.07, 0.70 };

  double white[3] = { 0.80, 0.80, 0.80 };
  double whiteSelected[3] = { 1.00, 1.00, 1.00 };

  bool selected = this->GetActiveComponentType() == type && this->GetActiveComponentIndex() == index;

  double* currentColor = white;
  switch (index)
  {
    case 0:
      currentColor = selected ? redSelected : red;
      break;
    case 1:
      currentColor = selected ? greenSelected : green;
      break;
    case 2:
      currentColor = selected ? blueSelected : blue;
      break;
    case 3:
      currentColor = selected ? whiteSelected : white;
      break;
    default:
      currentColor = selected ? whiteSelected : white;
      break;
  }

  double opacity = this->GetHandleOpacity(type, index);
  if (selected)
  {
    opacity = 1.0;
  }

  for (int i = 0; i < 3; ++i)
  {
    color[i] = currentColor[i];
  }
  color[3] = opacity;
}

//----------------------------------------------------------------------
int vtkMRMLInteractionWidgetRepresentation::GetHandleGlyphType(int type, int index)
{
  if (type == InteractionRotationHandle)
  {
    return GlyphRing;
  }
  else if (type == InteractionTranslationHandle && index < 3)
  {
    return GlyphArrow;
  }
  return GlyphCircle;
}

//----------------------------------------------------------------------
bool vtkMRMLInteractionWidgetRepresentation::GetHandleVisibility(int type, int index)
{
  vtkPolyData* handlePolyData = this->GetHandlePolydata(type);
  vtkIdTypeArray* visibilityArray = nullptr;
  if (handlePolyData)
  {
    visibilityArray = vtkIdTypeArray::SafeDownCast(handlePolyData->GetPointData()->GetArray("visibility"));
  }
  if (visibilityArray && index < visibilityArray->GetNumberOfValues())
  {
    return visibilityArray->GetValue(index) != 0;
  }
  return true;
}

//----------------------------------------------------------------------
double vtkMRMLInteractionWidgetRepresentation::GetHandleOpacity(int type, int index)
{
  // Determine if the handle should be displayed
  bool handleVisible = this->GetHandleVisibility(type, index);
  if (!handleVisible)
  {
    return 0.0;
  }

  double opacity = 1.0;

  double axis_World[3] = { 0.0, 0.0, 0.0 };
  this->GetInteractionHandleAxisWorld(type, index, axis_World);
  if (axis_World[0] == 0.0 && axis_World[1] == 0.0 && axis_World[2] == 0.0)
  {
    // No axis specified. The handle should be viewable from any direction.
    return opacity;
  }

  double handlePosition_World[3] = { 0.0, 0.0, 0.0 };
  this->GetInteractionHandlePositionWorld(type, index, handlePosition_World);

  double viewNormal_World[3] = { 0.0, 0.0, 0.0 };
  this->GetHandleToCameraVectorWorld(handlePosition_World, viewNormal_World);
  if (vtkMath::Dot(viewNormal_World, axis_World) < 0.0)
  {
    vtkMath::MultiplyScalar(axis_World, -1.0);
  }

  double fadeAngleRange = this->StartFadeAngleDegrees - this->EndFadeAngleDegrees;
  double angle = vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(viewNormal_World, axis_World));
  if (type == InteractionRotationHandle)
  {
    // Fade happens when the axis approaches 90 degrees from the view normal
    if (angle > 90.0 - this->EndFadeAngleDegrees)
    {
      opacity = 0.0;
    }
    else if (angle > 90.0 - this->StartFadeAngleDegrees)
    {
      double difference = angle - (90.0 - this->StartFadeAngleDegrees);
      opacity = 1.0 - (difference / fadeAngleRange);
    }
  }
  else if (type == InteractionTranslationHandle || type == InteractionScaleHandle)
  {
    // Fade happens when the axis approaches 0 degrees from the view normal
    if (angle < this->EndFadeAngleDegrees)
    {
      opacity = 0.0;
    }
    else if (angle < this->StartFadeAngleDegrees)
    {
      double difference = angle - this->EndFadeAngleDegrees;
      opacity = (difference / fadeAngleRange);
    }
  }
  return opacity;
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::GetHandleToCameraVectorWorld(double handlePosition_World[3],
                                                                          double normal_World[3])
{
  if (!handlePosition_World || !normal_World)
  {
    vtkErrorMacro("GetHandleToCameraVectorWorld: Invalid arguments");
    return;
  }

  if (this->GetSliceNode())
  {
    double slicePlaneNormalWorld4[4] = { 0.0, 0.0, 1.0, 0.0 };
    vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
    if (sliceNode)
    {
      sliceNode->GetXYToRAS()->MultiplyPoint(slicePlaneNormalWorld4, slicePlaneNormalWorld4);
    }
    normal_World[0] = slicePlaneNormalWorld4[0];
    normal_World[1] = slicePlaneNormalWorld4[1];
    normal_World[2] = slicePlaneNormalWorld4[2];
    vtkMath::Normalize(normal_World);
  }
  else if (this->GetRenderer() && this->GetRenderer()->GetActiveCamera())
  {
    // 3D and VR views
    vtkCamera* camera = this->GetRenderer()->GetActiveCamera();
    if (camera->GetParallelProjection())
    {
      camera->GetViewPlaneNormal(normal_World);
    }
    else
    {
      double cameraPosition_World[3] = { 0.0, 0.0, 0.0 };
      camera->GetPosition(cameraPosition_World);
      vtkMath::Subtract(cameraPosition_World, handlePosition_World, normal_World);
      vtkMath::Normalize(normal_World);
    }
  }
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::SetWidgetScale(double scale)
{
  if (this->WidgetScale == scale)
  {
    return;
  }
  this->WidgetScale = scale;
  this->NeedToRenderOn();
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::GetInteractionHandleOriginWorld(double originWorld[3])
{
  if (!originWorld)
  {
    return;
  }

  double handleOrigin[3] = { 0.0, 0.0, 0.0 };
  this->Pipeline->HandleToWorldTransform->TransformPoint(handleOrigin, originWorld);
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::GetInteractionHandleAxisLocal(int type, int index, double axisLocal[3])
{
  if (!axisLocal)
  {
    vtkErrorWithObjectMacro(nullptr, "GetInteractionHandleAxisLocal: Invalid axis argument");
    return;
  }

  axisLocal[0] = 0.0;
  axisLocal[1] = 0.0;
  axisLocal[2] = 0.0;

  bool faceCamera = false;

  switch (index)
  {
    case 0:
      axisLocal[0] = 1.0;
      break;
    case 1:
      axisLocal[1] = 1.0;
      break;
    case 2:
      axisLocal[2] = 1.0;
      break;
    default:
      faceCamera = type == InteractionRotationHandle;
      break;
  }

  if (faceCamera)
  {
    double handlePosition_World[3] = { 0.0, 0.0, 0.0 };
    this->GetInteractionHandlePositionWorld(type, index, handlePosition_World);

    double axisWorld[3] = { 0.0, 0.0, 0.0 };
    this->GetHandleToCameraVectorWorld(handlePosition_World, axisWorld);

    vtkNew<vtkTransform> worldToHandleTransform;
    worldToHandleTransform->DeepCopy(this->GetHandleToWorldTransform());
    worldToHandleTransform->Inverse();
    worldToHandleTransform->TransformVector(axisWorld, axisLocal);
    vtkMath::Normalize(axisLocal);
  }
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::GetInteractionHandleAxisWorld(int type, int index, double axisWorld[3])
{
  if (!axisWorld)
  {
    vtkErrorWithObjectMacro(nullptr, "GetInteractionHandleVectorWorld: Invalid axis argument");
    return;
  }

  double origin[3] = { 0.0, 0.0, 0.0 };
  this->GetInteractionHandleAxisLocal(type, index, axisWorld);
  this->Pipeline->HandleToWorldTransform->TransformVectorAtPoint(origin, axisWorld, axisWorld);
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::GetInteractionHandlePositionLocal(int type,
                                                                               int index,
                                                                               double positionLocal[3])
{
  if (!positionLocal)
  {
    vtkErrorMacro("GetInteractionHandlePositionLocal: Invalid position argument");
    return;
  }

  vtkPolyData* handlePolyData = this->GetHandlePolydata(type);
  if (!handlePolyData)
  {
    return;
  }

  if (index < 0 || index >= handlePolyData->GetNumberOfPoints())
  {
    return;
  }

  handlePolyData->GetPoint(index, positionLocal);
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::GetInteractionHandlePositionWorld(int type,
                                                                               int index,
                                                                               double positionWorld[3])
{
  if (!positionWorld)
  {
    vtkErrorWithObjectMacro(nullptr, "GetInteractionHandlePositionWorld: Invalid position argument");
  }

  vtkPolyData* handlePolyData = this->GetHandlePolydata(type);
  if (!handlePolyData)
  {
    return;
  }
  handlePolyData->GetPoint(index, positionWorld);

  vtkNew<vtkTransform> handleScaleTransform;
  handleScaleTransform->Scale(this->WidgetScale, this->WidgetScale, this->WidgetScale);
  handleScaleTransform->TransformPoint(positionWorld, positionWorld);

  this->Pipeline->HandleToWorldTransform->TransformPoint(positionWorld, positionWorld);
}

//----------------------------------------------------------------------
vtkMRMLInteractionWidgetRepresentation::HandleInfo vtkMRMLInteractionWidgetRepresentation::GetHandleInfo(int type,
                                                                                                         int index)
{
  double handlePositionLocal[3] = { 0.0, 0.0, 0.0 };
  this->GetInteractionHandlePositionLocal(type, index, handlePositionLocal);

  double handlePositionWorld[3] = { 0.0, 0.0, 0.0 };
  this->GetInteractionHandlePositionWorld(type, index, handlePositionWorld);

  double color[4] = { 0.0, 0.0, 0.0, 0.0 };
  this->GetHandleColor(type, index, color);

  int glyphType = this->GetHandleGlyphType(type, index);

  bool applyScaleToPosition = this->GetApplyScaleToPosition(type, index);

  return HandleInfo(index, type, handlePositionWorld, handlePositionLocal, color, glyphType, applyScaleToPosition);
}

//----------------------------------------------------------------------
bool vtkMRMLInteractionWidgetRepresentation::GetApplyScaleToPosition(int vtkNotUsed(type), int vtkNotUsed(index))
{
  return true;
}

//----------------------------------------------------------------------
vtkMRMLInteractionWidgetRepresentation::HandleInfoList vtkMRMLInteractionWidgetRepresentation::GetHandleInfoList()
{
  HandleInfoList handleInfoList;

  for (int index = 0; index < this->GetNumberOfHandles(InteractionRotationHandle); ++index)
  {
    handleInfoList.push_back(this->GetHandleInfo(InteractionRotationHandle, index));
  }

  for (int index = 0; index < this->GetNumberOfHandles(InteractionTranslationHandle); ++index)
  {
    handleInfoList.push_back(this->GetHandleInfo(InteractionTranslationHandle, index));
  }

  for (int index = 0; index < this->GetNumberOfHandles(InteractionScaleHandle); ++index)
  {
    handleInfoList.push_back(this->GetHandleInfo(InteractionScaleHandle, index));
  }

  return handleInfoList;
}

//----------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLInteractionWidgetRepresentation::GetSliceNode()
{
  return vtkMRMLSliceNode::SafeDownCast(this->ViewNode);
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::GetSliceToWorldCoordinates(const double slicePos[2], double worldPos[3])
{
  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
  if (!this->Renderer || !sliceNode)
  {
    return;
  }

  double xyzw[4] = {
    slicePos[0] - this->Renderer->GetOrigin()[0], slicePos[1] - this->Renderer->GetOrigin()[1], 0.0, 1.0
  };
  double rasw[4] = { 0.0, 0.0, 0.0, 1.0 };

  this->GetSliceNode()->GetXYToRAS()->MultiplyPoint(xyzw, rasw);

  worldPos[0] = rasw[0] / rasw[3];
  worldPos[1] = rasw[1] / rasw[3];
  worldPos[2] = rasw[2] / rasw[3];
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::UpdateSlicePlaneFromSliceNode()
{
  if (!this->GetSliceNode())
  {
    return;
  }

  if (!this->Renderer)
  {
    return;
  }

  vtkMatrix4x4* sliceXYToRAS = this->GetSliceNode()->GetXYToRAS();
  if (this->Pipeline)
  {
    // Update transformation to slice
    vtkNew<vtkMatrix4x4> rasToXY;
    rasToXY->DeepCopy(sliceXYToRAS);
    rasToXY->Invert();
    this->Pipeline->WorldToSliceTransform->Identity();
    this->Pipeline->WorldToSliceTransform->PostMultiply();
    this->Pipeline->WorldToSliceTransform->Concatenate(rasToXY);

    // TODO: Use 'vtkCamera::GetCompositeProjectionTransformMatrix()'
    // int* size = this->Renderer->GetSize();
    // double aspect = double(size[1]) / double(size[0]);
    // vtkNew<vtkMatrix4x4> compositeProjectionTransformMatrix;
    // compositeProjectionTransformMatrix->DeepCopy(this->Renderer->GetActiveCamera()->GetCompositeProjectionTransformMatrix(aspect,
    // -1, +1)); compositeProjectionTransformMatrix->Invert();
    // this->Pipeline->WorldToSliceTransform->Concatenate(compositeProjectionTransformMatrix);

    int* dimensions = this->GetSliceNode()->GetDimensions();
    if (this->Renderer->GetActiveCamera()->GetUseHorizontalViewAngle())
    {
      this->Pipeline->WorldToSliceTransform->Scale(2.0 / dimensions[0], 2.0 / dimensions[0], 2.0 / dimensions[0]);
      this->Pipeline->WorldToSliceTransform->Translate(-1.0, -1.0 * dimensions[1] / dimensions[0], 0.0);
    }
    else
    {
      this->Pipeline->WorldToSliceTransform->Scale(2.0 / dimensions[1], 2.0 / dimensions[1], 2.0 / dimensions[1]);
      this->Pipeline->WorldToSliceTransform->Translate(-1.0 * dimensions[0] / dimensions[1], -1.0, 0.0);
    }

    // Move the interaction handle to the slice plane to prevent it from being clipped.
    double handleCenterPos_Slice[3] = { 0.0, 0.0, 0.0 };
    this->Pipeline->HandleToWorldTransform->TransformPoint(handleCenterPos_Slice, handleCenterPos_Slice);
    this->Pipeline->WorldToSliceTransform->TransformPoint(handleCenterPos_Slice, handleCenterPos_Slice);
    this->Pipeline->WorldToSliceTransform->Translate(0.0, 0.0, -handleCenterPos_Slice[2]);
  }

  // Update slice plane (for distance computation)
  double normal[3] = { 0.0, 0.0, 0.0 };
  double origin[3] = { 0.0, 0.0, 0.0 };
  const double planeOrientation = 1.0; // +/-1: orientation of the normal
  for (int i = 0; i < 3; i++)
  {
    normal[i] = planeOrientation * sliceXYToRAS->GetElement(i, 2);
    origin[i] = sliceXYToRAS->GetElement(i, 3);
  }
  vtkMath::Normalize(normal);

  // Compare slice normal and new normal
  double normalDifferenceAngle = vtkMath::AngleBetweenVectors(normal, this->SlicePlane->GetNormal());
  double originDifferenceMm = vtkMath::Distance2BetweenPoints(origin, this->SlicePlane->GetOrigin());
  double epsilon = 1e-6;
  if (normalDifferenceAngle < epsilon && originDifferenceMm < epsilon)
  {
    // No change in slice plane
    return;
  }

  this->SlicePlane->SetNormal(normal);
  this->SlicePlane->SetOrigin(origin);
  this->NeedToRenderOn();
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::UpdateViewScaleFactor()
{
  this->ViewScaleFactorMmPerPixel = 1.0;
  this->ScreenSizePixel = 1000.0;
  if (!this->Renderer || !this->Renderer->GetActiveCamera())
  {
    return;
  }

  int* screenSize = this->Renderer->GetRenderWindow()->GetScreenSize();
  double screenSizePixel = sqrt(screenSize[0] * screenSize[0] + screenSize[1] * screenSize[1]);
  if (screenSizePixel < 1.0)
  {
    // render window is not fully initialized yet
    return;
  }
  this->ScreenSizePixel = screenSizePixel;

  if (this->GetSliceNode())
  {
    vtkMatrix4x4* xyToSlice = this->GetSliceNode()->GetXYToSlice();
    this->ViewScaleFactorMmPerPixel = sqrt(xyToSlice->GetElement(0, 1) * xyToSlice->GetElement(0, 1) +
                                           xyToSlice->GetElement(1, 1) * xyToSlice->GetElement(1, 1));
  }
  else
  {
    double handlePoint_World[3] = { 0.0, 0.0, 0.0 };
    this->GetHandleToWorldTransform()->TransformPoint(handlePoint_World, handlePoint_World);

    double cameraPos_World[3] = { 0.0, 0.0, 0.0 };
    this->Renderer->GetActiveCamera()->GetPosition(cameraPos_World);

    double distance = sqrt(vtkMath::Distance2BetweenPoints(handlePoint_World, cameraPos_World));

    double cameraDirection_World[3] = { 0.0, 0.0, 0.0 };
    this->Renderer->GetActiveCamera()->GetDirectionOfProjection(cameraDirection_World);
    vtkMath::Normalize(cameraDirection_World);
    vtkMath::MultiplyScalar(cameraDirection_World, distance);

    double handleFocalPoint_World[3] = { 0.0, 0.0, 0.0 };
    vtkMath::Add(cameraPos_World, cameraDirection_World, handleFocalPoint_World);

    // What we are interested in is the Mm to pixel conversion for an object that is as far away from the camera as the
    // handle. In VR we can't use the scale factor at the handle position since the scale will change when the user
    // rotates their head. The solution is to find the mm to pixel conversion for a point that is as far away from the
    // camera as the handle, but in the camera view direction.
    this->ViewScaleFactorMmPerPixel =
      vtkMRMLAbstractThreeDViewDisplayableManager::GetViewScaleFactorAtPosition(this->Renderer, handleFocalPoint_World);
  }
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidgetRepresentation::UpdateHandleSize()
{
  if (!this->GetInteractionSizeAbsolute())
  {
    this->InteractionSize = this->ScreenSizePixel * this->GetScreenScaleFactor() * this->GetInteractionScalePercent() /
                            100.0 * this->ViewScaleFactorMmPerPixel;
  }
  else
  {
    this->InteractionSize = this->GetInteractionSizeMm();
    if (this->GetSliceNode())
    {
      // Convert to pixels for slice views
      this->InteractionSize /= this->ViewScaleFactorMmPerPixel;
    }
  }
  this->SetWidgetScale(this->InteractionSize);
}

//----------------------------------------------------------------------
double vtkMRMLInteractionWidgetRepresentation::GetInteractionScalePercent()
{
  return 3.0;
}

//----------------------------------------------------------------------
double vtkMRMLInteractionWidgetRepresentation::GetInteractionSizeMm()
{
  return this->InteractionSize;
}

//----------------------------------------------------------------------
bool vtkMRMLInteractionWidgetRepresentation::GetInteractionSizeAbsolute()
{
  return false;
}

//----------------------------------------------------------------------
vtkTransform* vtkMRMLInteractionWidgetRepresentation::GetHandleToWorldTransform()
{
  if (!this->Pipeline)
  {
    return nullptr;
  }
  return this->Pipeline->HandleToWorldTransform;
}
