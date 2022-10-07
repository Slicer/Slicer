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
#include <vtkActor2D.h>
#include <vtkAppendPolyData.h>
#include <vtkCellLocator.h>
#include <vtkCubeSource.h>
#include <vtkDoubleArray.h>
#include <vtkGlyph3D.h>
#include <vtkLine.h>
#include <vtkOutlineFilter.h>
#include <vtkPassThrough.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

// Markups VTK Widgets includes
#include <vtkSlicerROIRepresentation3D.h>

// MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsROIDisplayNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLScene.h"

vtkStandardNewMacro(vtkSlicerROIRepresentation3D);

//----------------------------------------------------------------------
vtkSlicerROIRepresentation3D::vtkSlicerROIRepresentation3D()
{
  this->ROISource = nullptr;

  this->ROIPipelineInputFilter = vtkSmartPointer<vtkPassThrough>::New();

  this->ROIToWorldTransform = vtkSmartPointer<vtkTransform>::New();
  this->ROITransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->ROITransformFilter->SetTransform(this->ROIToWorldTransform);
  this->ROITransformFilter->SetInputConnection(this->ROIPipelineInputFilter->GetOutputPort());

  this->ROIMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ROIMapper->SetInputConnection(this->ROITransformFilter->GetOutputPort());
  this->ROIProperty = vtkSmartPointer<vtkProperty>::New();
  this->ROIProperty->DeepCopy(this->GetControlPointsPipeline(Selected)->Property);
  this->ROIActor = vtkSmartPointer<vtkActor>::New();
  this->ROIActor->SetMapper(this->ROIMapper);
  this->ROIActor->SetProperty(this->ROIProperty);

  this->ROIOccludedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ROIOccludedMapper->SetInputConnection(this->ROITransformFilter->GetOutputPort());
  this->ROIOccludedProperty = vtkSmartPointer<vtkProperty>::New();
  this->ROIOccludedProperty->DeepCopy(this->ROIProperty);
  this->ROIOccludedActor = vtkSmartPointer<vtkActor>::New();
  this->ROIOccludedActor->SetMapper(this->ROIOccludedMapper);
  this->ROIOccludedActor->SetProperty(this->ROIOccludedProperty);

  this->ROIOutlineFilter = vtkSmartPointer<vtkOutlineFilter>::New();
  this->ROIOutlineFilter->SetInputConnection(this->ROIPipelineInputFilter->GetOutputPort());

  this->ROIOutlineTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->ROIOutlineTransformFilter->SetTransform(this->ROIToWorldTransform);
  this->ROIOutlineTransformFilter->SetInputConnection(this->ROIOutlineFilter->GetOutputPort());

  this->ROIOutlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ROIOutlineMapper->SetInputConnection(this->ROIOutlineTransformFilter->GetOutputPort());
  this->ROIOutlineProperty = vtkSmartPointer<vtkProperty>::New();
  this->ROIOutlineProperty->DeepCopy(this->GetControlPointsPipeline(Selected)->Property);
  this->ROIOutlineActor = vtkSmartPointer<vtkActor>::New();
  this->ROIOutlineActor->SetMapper(this->ROIOutlineMapper);
  this->ROIOutlineActor->SetProperty(this->ROIOutlineProperty);

  this->ROIOutlineOccludedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->ROIOutlineOccludedMapper->SetInputConnection(this->ROIOutlineTransformFilter->GetOutputPort());
  this->ROIOutlineOccludedProperty = vtkSmartPointer<vtkProperty>::New();
  this->ROIOutlineOccludedProperty->DeepCopy(this->ROIOutlineProperty);
  this->ROIOutlineOccludedActor = vtkSmartPointer<vtkActor>::New();
  this->ROIOutlineOccludedActor->SetMapper(this->ROIOutlineOccludedMapper);
  this->ROIOutlineOccludedActor->SetProperty(this->ROIOutlineOccludedProperty);
  this->ROIOutlineOccludedActor->SetMapper(this->ROIOutlineOccludedMapper);

}

//----------------------------------------------------------------------
vtkSlicerROIRepresentation3D::~vtkSlicerROIRepresentation3D() = default;

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
{
  Superclass::UpdateFromMRML(caller, event, callData);

  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  if (!roiNode || !this->IsDisplayable() || !displayNode)
    {
    this->VisibilityOff();
    return;
    }

  switch (roiNode->GetROIType())
    {
    case vtkMRMLMarkupsROINode::ROITypeBox:
    case vtkMRMLMarkupsROINode::ROITypeBoundingBox:
      this->UpdateCubeSourceFromMRML(roiNode);
      break;
    default:
      this->ROIActor->SetVisibility(false);
      return;
    }

  this->ROIToWorldTransform->SetMatrix(roiNode->GetObjectToWorldMatrix());

  this->ROIActor->SetVisibility(roiNode->GetNumberOfControlPoints() > 0 && displayNode->GetFillVisibility());
  this->ROIOccludedActor->SetVisibility(this->ROIActor->GetVisibility() && displayNode->GetOccludedVisibility());

  this->ROIOutlineActor->SetVisibility(roiNode->GetNumberOfControlPoints() > 0 && displayNode->GetOutlineVisibility());
  this->ROIOutlineOccludedActor->SetVisibility(this->ROIOutlineActor->GetVisibility() && displayNode->GetOccludedVisibility());

  this->VisibilityOn();
  this->PickableOn();

  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsROIDisplayNode::ComponentROI)
    {
    controlPointType = this->GetAllControlPointsSelected() ? vtkSlicerMarkupsWidgetRepresentation::Selected : vtkSlicerMarkupsWidgetRepresentation::Unselected;
    }

  // Properties label display
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);
  if (this->MarkupsDisplayNode->GetPropertiesLabelVisibility()
    && roiNode->GetNumberOfDefinedControlPoints(true) > 0) // including preview
    {
    roiNode->GetNthControlPointPositionWorld(0, this->TextActorPositionWorld);
    this->TextActor->SetVisibility(true);
    }
  else
    {
    this->TextActor->SetVisibility(false);
    }

  double opacity = displayNode->GetOpacity();
  double fillOpacity = displayNode->GetFillOpacity();
  this->ROIProperty->DeepCopy(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ROIProperty->SetOpacity(opacity * fillOpacity);

  double occludedOpacity = displayNode->GetOccludedVisibility() ? fillOpacity * displayNode->GetOccludedOpacity() : 0.0;
  this->ROIOccludedProperty->DeepCopy(this->ROIProperty);
  this->ROIOccludedProperty->SetOpacity(opacity * fillOpacity * occludedOpacity);

  this->UpdateRelativeCoincidentTopologyOffsets(this->ROIMapper, this->ROIOccludedMapper);

  double outlineOpacity = opacity * this->MarkupsDisplayNode->GetOutlineOpacity();
  this->ROIOutlineProperty->DeepCopy(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ROIOutlineProperty->SetOpacity(outlineOpacity);

  this->ROIOutlineOccludedProperty->DeepCopy(this->ROIOutlineProperty);
  this->ROIOutlineOccludedProperty->SetOpacity(opacity * fillOpacity * occludedOpacity);

  this->UpdateRelativeCoincidentTopologyOffsets(this->ROIOutlineMapper, this->ROIOutlineOccludedMapper);
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::SetROISource(vtkPolyDataAlgorithm* roiSource)
{
  this->ROISource = roiSource;
  if (this->ROISource)
    {
    this->ROIPipelineInputFilter->SetInputConnection(roiSource->GetOutputPort());
    }
  else
    {
    this->ROIPipelineInputFilter->RemoveAllInputConnections(0);
    }
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::UpdateCubeSourceFromMRML(vtkMRMLMarkupsROINode* roiNode)
{
  if (!roiNode)
    {
    return;
    }

  vtkSmartPointer<vtkCubeSource> cubeSource = vtkCubeSource::SafeDownCast(this->ROISource);
  if (!cubeSource)
    {
    cubeSource = vtkSmartPointer<vtkCubeSource>::New();
    this->SetROISource(cubeSource);
    }

  double sideLengths[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetSize(sideLengths);
  cubeSource->SetXLength(sideLengths[0]);
  cubeSource->SetYLength(sideLengths[1]);
  cubeSource->SetZLength(sideLengths[2]);
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::GetActors(vtkPropCollection *pc)
{
  this->ROIActor->GetActors(pc);
  this->ROIOccludedActor->GetActors(pc);
  this->ROIOutlineActor->GetActors(pc);
  this->ROIOutlineOccludedActor->GetActors(pc);
  this->Superclass::GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->ROIActor->ReleaseGraphicsResources(win);
  this->ROIOccludedActor->ReleaseGraphicsResources(win);
  this->ROIOutlineActor->ReleaseGraphicsResources(win);
  this->ROIOutlineOccludedActor->ReleaseGraphicsResources(win);
  this->Superclass::ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerROIRepresentation3D::RenderOverlay(vtkViewport *viewport)
{
  int count = 0;
  if (this->ROIActor->GetVisibility())
    {
    count += this->ROIActor->RenderOverlay(viewport);
    }
  if (this->ROIOccludedActor->GetVisibility())
    {
    count += this->ROIOccludedActor->RenderOverlay(viewport);
    }
  if (this->ROIOutlineActor->GetVisibility())
    {
    count += this->ROIOutlineActor->RenderOverlay(viewport);
    }
  if (this->ROIOutlineOccludedActor->GetVisibility())
    {
    count += this->ROIOutlineOccludedActor->RenderOverlay(viewport);
    }
  count += this->Superclass::RenderOverlay(viewport);
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerROIRepresentation3D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count = 0;
  if (this->ROIActor->GetVisibility())
    {
    count += this->ROIActor->RenderOpaqueGeometry(viewport);
    }
  if (this->ROIOccludedActor->GetVisibility())
    {
    count += this->ROIOccludedActor->RenderOpaqueGeometry(viewport);
    }
  if (this->ROIOutlineActor->GetVisibility())
    {
    count += this->ROIOutlineActor->RenderOpaqueGeometry(viewport);
    }
  if (this->ROIOutlineOccludedActor->GetVisibility())
    {
    count += this->ROIOutlineOccludedActor->RenderOpaqueGeometry(viewport);
    }
  count += this->Superclass::RenderOpaqueGeometry(viewport);
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerROIRepresentation3D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count = this->Superclass::RenderTranslucentPolygonalGeometry(viewport);
  if (this->ROIActor->GetVisibility())
    {
    this->ROIActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->ROIActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->ROIOccludedActor->GetVisibility())
    {
    this->ROIOccludedActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->ROIOccludedActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->ROIOutlineActor->GetVisibility())
    {
    this->ROIOutlineActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->ROIOutlineActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->ROIOutlineOccludedActor->GetVisibility())
    {
    this->ROIOutlineOccludedActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->ROIOutlineOccludedActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  count += this->Superclass::RenderTranslucentPolygonalGeometry(viewport);
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerROIRepresentation3D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->ROIActor->GetVisibility() && this->ROIActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->ROIOccludedActor->GetVisibility() && this->ROIOccludedActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->ROIOutlineActor->GetVisibility() && this->ROIOutlineActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->ROIOutlineOccludedActor->GetVisibility() && this->ROIOutlineOccludedActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------
double *vtkSlicerROIRepresentation3D::GetBounds()
{
  vtkBoundingBox boundingBox;
  const std::vector<vtkProp*> actors({ this->ROIActor });
  this->AddActorsBounds(boundingBox, actors, Superclass::GetBounds());
  boundingBox.GetBounds(this->Bounds);
  return this->Bounds;
}

//-----------------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);
}


//----------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::CanInteract(
  vtkMRMLInteractionEventData* interactionEventData,
  int& foundComponentType, int& foundComponentIndex, double& closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetLocked() || !interactionEventData || !this->GetVisibility())
    {
    return;
    }

  Superclass::CanInteract(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType != vtkMRMLMarkupsDisplayNode::ComponentNone)
    {
    return;
    }

  this->CanInteractWithROI(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
}

//-----------------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::CanInteractWithROI(
  vtkMRMLInteractionEventData* interactionEventData,
  int& foundComponentType, int& foundComponentIndex, double& closestDistance2)
{
  this->ROIOutlineFilter->Update();
  if (this->ROIOutlineFilter->GetOutput() && this->ROIOutlineFilter->GetOutput()->GetNumberOfPoints() == 0)
    {
    return;
    }

  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->MarkupsNode);
  if (!roiNode)
    {
    return;
    }

  if (interactionEventData->IsDisplayPositionValid())
    {
    int displayPosition[2] = { 0, 0 };
    interactionEventData->GetDisplayPosition(displayPosition);

    double displayPosition3[3] = { 0.0, 0.0, 0.0 };
    displayPosition3[0] = static_cast<double>(displayPosition[0]);
    displayPosition3[1] = static_cast<double>(displayPosition[1]);

    vtkNew<vtkTransform> objectToNode;
    objectToNode->Concatenate(roiNode->GetObjectToNodeMatrix());

    double distance2Display = VTK_DOUBLE_MAX;

    vtkPolyData* roiOutline = this->ROIOutlineFilter->GetOutput();
    for (int lineIndex = 0; lineIndex < roiOutline->GetNumberOfCells(); ++lineIndex)
      {
      vtkLine* line = vtkLine::SafeDownCast(roiOutline->GetCell(lineIndex));
      if (!line)
        {
        continue;
        }

      double edgePoint0Display[3] = { 0.0, 0.0, 0.0 };
      line->GetPoints()->GetPoint(0, edgePoint0Display);
      objectToNode->TransformPoint(edgePoint0Display, edgePoint0Display);
      roiNode->TransformPointToWorld(edgePoint0Display, edgePoint0Display);
      this->Renderer->SetWorldPoint(edgePoint0Display);
      this->Renderer->WorldToDisplay();
      this->Renderer->GetDisplayPoint(edgePoint0Display);
      edgePoint0Display[2] = 0.0;

      double edgePoint1Display[3] = { 0.0, 0.0, 0.0 };
      line->GetPoints()->GetPoint(1, edgePoint1Display);
      objectToNode->TransformPoint(edgePoint1Display, edgePoint1Display);
      roiNode->TransformPointToWorld(edgePoint1Display, edgePoint1Display);
      this->Renderer->SetWorldPoint(edgePoint1Display);
      this->Renderer->WorldToDisplay();
      this->Renderer->GetDisplayPoint(edgePoint1Display);
      edgePoint1Display[2] = 0.0;

      double t;
      double currentClosestPointDisplay[3] = { 0.0, 0.0, 0.0 };
      double currentDist2Display = vtkLine::DistanceToLine(displayPosition3, edgePoint0Display, edgePoint1Display, t, currentClosestPointDisplay);
      if (currentDist2Display < distance2Display)
        {
        distance2Display = currentDist2Display;
        }
      }

    double pixelTolerance = this->PickingTolerance * this->ScreenScaleFactor;
    if (distance2Display < VTK_DOUBLE_MAX && distance2Display < pixelTolerance * pixelTolerance && distance2Display < closestDistance2)
      {
      closestDistance2 = distance2Display;
      foundComponentType = vtkMRMLMarkupsROIDisplayNode::ComponentROI;
      foundComponentIndex = 0;
      return;
      }
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::SetupInteractionPipeline()
{
  this->InteractionPipeline = new MarkupsInteractionPipelineROI(this);
  this->InteractionPipeline->InitializePipeline();
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::UpdateInteractionPipeline()
{
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (!roiNode || !this->MarkupsDisplayNode)
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }

  this->InteractionPipeline->Actor->SetVisibility(this->MarkupsDisplayNode->GetVisibility()
    && roiNode->GetNumberOfControlPoints() > 0
    && this->MarkupsDisplayNode->GetVisibility3D()
    && this->MarkupsDisplayNode->GetHandlesInteractive());

  vtkNew<vtkTransform> handleToWorldTransform;
  handleToWorldTransform->SetMatrix(roiNode->GetInteractionHandleToWorldMatrix());
  this->InteractionPipeline->HandleToWorldTransform->DeepCopy(handleToWorldTransform);

  MarkupsInteractionPipelineROI* interactionPipeline = static_cast<MarkupsInteractionPipelineROI*>(this->InteractionPipeline);
  interactionPipeline->UpdateScaleHandles();
}

//-----------------------------------------------------------------------------
vtkSlicerROIRepresentation3D::MarkupsInteractionPipelineROI::MarkupsInteractionPipelineROI(vtkSlicerMarkupsWidgetRepresentation* representation)
  : MarkupsInteractionPipeline(representation)
{

}

//----------------------------------------------------------------------
double vtkSlicerROIRepresentation3D::MarkupsInteractionPipelineROI::GetHandleOpacity(int type, int index)
{
  double opacity = 1.0;
  if (type == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle && index > 5)
    {
    vtkSlicerMarkupsWidgetRepresentation* markupsRepresentation = vtkSlicerMarkupsWidgetRepresentation::SafeDownCast(this->Representation);
    vtkMRMLMarkupsDisplayNode* displayNode = nullptr;
    if (markupsRepresentation)
      {
      displayNode = markupsRepresentation->GetMarkupsDisplayNode();
      }
    if (displayNode)
      {
      opacity = displayNode->GetScaleHandleVisibility() ? 1.0 : 0.0;
      }
    }
  else
    {
    opacity = MarkupsInteractionPipeline::GetHandleOpacity(type, index);
    }

  return opacity;
}

//-----------------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::MarkupsInteractionPipelineROI::GetHandleColor(int type, int index, double color[4])
{
  if (type != vtkMRMLMarkupsDisplayNode::ComponentScaleHandle)
    {
    MarkupsInteractionPipeline::GetHandleColor(type, index, color);
    return;
    }

  double red[4]       = { 1.00, 0.00, 0.00, 1.00 };
  double green[4]     = { 0.00, 1.00, 0.00, 1.00 };
  double blue[4]      = { 0.00, 0.00, 1.00, 1.00 };
  double yellow[4]    = { 1.00, 1.00, 0.00, 1.00 };
  double lightGrey[4] = { 0.90, 0.90, 0.90, 1.00 };

  double* currentColor = lightGrey;
  switch (index)
    {
    case 0: // L
    case 1: // R
      currentColor = red;
      break;
    case 2: // P
    case 3: // A
      currentColor = green;
      break;
    case 4: // I
    case 5: // S
      currentColor = blue;
      break;
    default:
      break;
    }
  vtkSlicerMarkupsWidgetRepresentation* markupsRepresentation = vtkSlicerMarkupsWidgetRepresentation::SafeDownCast(this->Representation);
  vtkMRMLMarkupsDisplayNode* displayNode = nullptr;
  if (markupsRepresentation)
    {
    displayNode = markupsRepresentation->GetMarkupsDisplayNode();
    }

  double opacity = this->GetHandleOpacity(type, index);
  if (displayNode && displayNode->GetActiveComponentType() == type && displayNode->GetActiveComponentIndex() == index)
    {
    currentColor = yellow;
    opacity = 1.0;
    }

  for (int i = 0; i < 3; ++i)
    {
    color[i] = currentColor[i];
    }

  vtkIdTypeArray* visibilityArray = vtkIdTypeArray::SafeDownCast(this->ScaleHandlePoints->GetPointData()->GetArray("visibility"));
  if (visibilityArray)
    {
    vtkIdType visibility = visibilityArray->GetValue(index);
    opacity = visibility ? opacity : 0.0;
    }
  color[3] = opacity;
}

//-----------------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::MarkupsInteractionPipelineROI::CreateScaleHandles()
{
  MarkupsInteractionPipeline::CreateScaleHandles();
  this->AxisScaleGlypher->SetInputData(this->ScaleHandlePoints);
  this->UpdateScaleHandles();
}

//-----------------------------------------------------------------------------
vtkSlicerROIRepresentation3D::HandleInfoList vtkSlicerROIRepresentation3D::MarkupsInteractionPipelineROI::GetHandleInfoList()
{
 vtkSlicerMarkupsWidgetRepresentation::HandleInfoList handleInfoList;
  for (int i = 0; i < this->RotationHandlePoints->GetNumberOfPoints(); ++i)
    {
    double handlePositionNode[3] = { 0.0, 0.0, 0.0 };
    double handlePositionWorld[3] = { 0.0, 0.0, 0.0 };
    this->RotationHandlePoints->GetPoint(i, handlePositionNode);
    this->RotationScaleTransform->GetTransform()->TransformPoint(handlePositionNode, handlePositionWorld);
    this->HandleToWorldTransform->TransformPoint(handlePositionWorld, handlePositionWorld);
    double color[4] = { 0.0, 0.0, 0.0 };
    this->GetHandleColor(vtkMRMLMarkupsDisplayNode::ComponentRotationHandle, i, color);
    HandleInfo info(i, vtkMRMLMarkupsDisplayNode::ComponentRotationHandle, handlePositionWorld, handlePositionNode, color);
    handleInfoList.push_back(info);
    }

  for (int i = 0; i < this->TranslationHandlePoints->GetNumberOfPoints(); ++i)
    {
    double handlePositionNode[3] = { 0.0, 0.0, 0.0 };
    double handlePositionWorld[3] = { 0.0, 0.0, 0.0 };
    this->TranslationHandlePoints->GetPoint(i, handlePositionNode);
    this->TranslationScaleTransform->GetTransform()->TransformPoint(handlePositionNode, handlePositionWorld);
    this->HandleToWorldTransform->TransformPoint(handlePositionWorld, handlePositionWorld);
    double color[4] = { 0 };
    this->GetHandleColor(vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle, i, color);
    HandleInfo info(i, vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle, handlePositionWorld, handlePositionNode, color);
    handleInfoList.push_back(info);
    }

  for (int i = 0; i < this->ScaleHandlePoints->GetNumberOfPoints(); ++i)
    {
    double handlePositionNode[3] = { 0.0, 0.0, 0.0 };
    double handlePositionWorld[3] = { 0.0, 0.0, 0.0 };
    this->ScaleHandlePoints->GetPoint(i, handlePositionNode);
    this->HandleToWorldTransform->TransformPoint(handlePositionNode, handlePositionWorld);
    double color[4] = { 0 };
    this->GetHandleColor(vtkMRMLMarkupsDisplayNode::ComponentScaleHandle, i, color);
    HandleInfo info(i, vtkMRMLMarkupsDisplayNode::ComponentScaleHandle, handlePositionWorld, handlePositionNode, color);
    handleInfoList.push_back(info);
    }

  return handleInfoList;
}

//-----------------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::MarkupsInteractionPipelineROI::UpdateScaleHandles()
{
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(
    vtkSlicerMarkupsWidgetRepresentation::SafeDownCast(this->Representation)->GetMarkupsNode());
  if (!roiNode)
    {
    return;
    }

  double sideLengths[3] = { 0.0,  0.0, 0.0 };
  roiNode->GetSizeWorld(sideLengths);
  vtkMath::MultiplyScalar(sideLengths, 0.5);

  vtkNew<vtkPoints> roiPoints;
  roiPoints->SetNumberOfPoints(14);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleLFace,     -sideLengths[0],             0.0,             0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleRFace,      sideLengths[0],             0.0,             0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandlePFace,      0.0,            -sideLengths[1],             0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleAFace,      0.0,             sideLengths[1],             0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleIFace,      0.0,                        0.0, -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleSFace,      0.0,                        0.0,  sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleLPICorner, -sideLengths[0], -sideLengths[1], -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleRPICorner, sideLengths[0],  -sideLengths[1], -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleLAICorner, -sideLengths[0],  sideLengths[1], -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleRAICorner, sideLengths[0],   sideLengths[1], -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner, -sideLengths[0], -sideLengths[1],  sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleRPSCorner, sideLengths[0],  -sideLengths[1],  sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleLASCorner, -sideLengths[0],  sideLengths[1],  sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROIDisplayNode::HandleRASCorner, sideLengths[0],   sideLengths[1],  sideLengths[2]);
  this->ScaleHandlePoints->SetPoints(roiPoints);

  vtkIdTypeArray* visibilityArray = vtkIdTypeArray::SafeDownCast(this->ScaleHandlePoints->GetPointData()->GetArray("visibility"));
  visibilityArray->SetNumberOfValues(roiPoints->GetNumberOfPoints());
  visibilityArray->Fill(1);
  this->UpdateHandleVisibility();
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::MarkupsInteractionPipelineROI::UpdateHandleVisibility()
{
  MarkupsInteractionPipeline::UpdateHandleVisibility();

  vtkSlicerMarkupsWidgetRepresentation* markupsRepresentation = vtkSlicerMarkupsWidgetRepresentation::SafeDownCast(this->Representation);
  vtkMRMLMarkupsDisplayNode* displayNode = nullptr;
  if (markupsRepresentation)
    {
    displayNode = markupsRepresentation->GetMarkupsDisplayNode();
    }
  if (!displayNode)
    {
    vtkGenericWarningMacro("UpdateHandleVisibility: Invalid display node");
    return;
    }

  bool* scaleHandleAxes = displayNode->GetScaleHandleComponentVisibility();

  vtkIdTypeArray* scaleVisibilityArray = vtkIdTypeArray::SafeDownCast(this->ScaleHandlePoints->GetPointData()->GetArray("visibility"));
  if (scaleVisibilityArray)
    {
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleLFace, scaleHandleAxes[0]);
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleRFace, scaleHandleAxes[0]);
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandlePFace, scaleHandleAxes[1]);
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleAFace, scaleHandleAxes[1]);
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleIFace, scaleHandleAxes[2]);
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleSFace, scaleHandleAxes[2]);

    bool viewPlaneScaleVisibility = scaleHandleAxes[3];
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleLPICorner, viewPlaneScaleVisibility);
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleRPICorner, viewPlaneScaleVisibility);
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleLAICorner, viewPlaneScaleVisibility);
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleRAICorner, viewPlaneScaleVisibility);
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner, viewPlaneScaleVisibility);
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleRPSCorner, viewPlaneScaleVisibility);
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleLASCorner, viewPlaneScaleVisibility);
    scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleRASCorner, viewPlaneScaleVisibility);

    if (scaleVisibilityArray->GetNumberOfValues() > vtkMRMLMarkupsROIDisplayNode::HandleASEdge)
      {
      scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleLPEdge, viewPlaneScaleVisibility);
      scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleRPEdge, viewPlaneScaleVisibility);
      scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleLAEdge, viewPlaneScaleVisibility);
      scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleRAEdge, viewPlaneScaleVisibility);
      scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleLIEdge, viewPlaneScaleVisibility);
      scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleRIEdge, viewPlaneScaleVisibility);
      scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleLSEdge, viewPlaneScaleVisibility);
      scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleRSEdge, viewPlaneScaleVisibility);
      scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandlePIEdge, viewPlaneScaleVisibility);
      scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleAIEdge, viewPlaneScaleVisibility);
      scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandlePSEdge, viewPlaneScaleVisibility);
      scaleVisibilityArray->SetValue(vtkMRMLMarkupsROIDisplayNode::HandleASEdge, viewPlaneScaleVisibility);
      }

    }
}

//----------------------------------------------------------------------
void vtkSlicerROIRepresentation3D::MarkupsInteractionPipelineROI::GetInteractionHandleAxisWorld(int type, int index, double axisWorld[3])
{
  if (!axisWorld)
    {
    vtkErrorWithObjectMacro(nullptr, "GetInteractionHandleVectorWorld: Invalid axis argument!");
    return;
    }

  axisWorld[0] = 0.0;
  axisWorld[1] = 0.0;
  axisWorld[2] = 0.0;

  if (type == vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle)
    {
    switch (index)
      {
      case 0:
        axisWorld[0] = 1.0;
        break;
      case 1:
        axisWorld[1] = 1.0;
        break;
      case 2:
        axisWorld[2] = 1.0;
        break;
      default:
        break;
      }
    }
  else if (type == vtkMRMLMarkupsDisplayNode::ComponentRotationHandle)
    {
    switch (index)
      {
    case 0:
      axisWorld[0] = 1.0;
      break;
    case 1:
      axisWorld[1] = 1.0;
      break;
    case 2:
      axisWorld[2] = 1.0;
        break;
      default:
        break;
      }
    }
  else if (type == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle)
    {
    switch (index)
      {
      case vtkMRMLMarkupsROIDisplayNode::HandleLFace:
        axisWorld[0] = -1.0;
        break;
      case  vtkMRMLMarkupsROIDisplayNode::HandleRFace:
        axisWorld[0] = 1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandlePFace:
        axisWorld[1] = -1.0;
        break;
      case  vtkMRMLMarkupsROIDisplayNode::HandleAFace:
        axisWorld[1] = 1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleIFace:
        axisWorld[2] = -1.0;
        break;
      case  vtkMRMLMarkupsROIDisplayNode::HandleSFace:
        axisWorld[2] = 1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLPICorner:
        axisWorld[0] = -1.0;
        axisWorld[1] = -1.0;
        axisWorld[2] = -1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRPICorner:
          axisWorld[0] = 1.0;
          axisWorld[1] = -1.0;
          axisWorld[2] = -1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLAICorner:
        axisWorld[0] = -1.0;
        axisWorld[1] = 1.0;
        axisWorld[2] = -1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRAICorner:
        axisWorld[0] = 1.0;
        axisWorld[1] = 1.0;
        axisWorld[2] = -1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner:
        axisWorld[0] = -1.0;
        axisWorld[1] = -1.0;
        axisWorld[2] = 1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRPSCorner:
        axisWorld[0] = 1.0;
        axisWorld[1] = -1.0;
        axisWorld[2] = 1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLASCorner:
        axisWorld[0] = -1.0;
        axisWorld[1] = 1.0;
        axisWorld[2] = 1.0;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRASCorner:
        axisWorld[0] = 1.0;
        axisWorld[1] = 1.0;
        axisWorld[2] = 1.0;
        break;
      default:
        break;
      }
    }
  double origin[3] = { 0.0, 0.0, 0.0 };
  this->HandleToWorldTransform->TransformVectorAtPoint(origin, axisWorld, axisWorld);
}
