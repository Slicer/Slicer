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
  Care Ontario, OpenAnatomy, and Brigham and Women�s Hospital through NIH grant R01MH112748.

==============================================================================*/

// VTK includes
#include <vtkActor2D.h>
#include <vtkAppendPolyData.h>
#include <vtkCubeSource.h>
#include <vtkDoubleArray.h>
#include <vtkGlyph3D.h>
#include <vtkOutlineFilter.h>
#include <vtkPassThroughFilter.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

// Markups VTK Widgets includes
#include <vtkSlicerROIRepresentation3D.h>

// MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLScene.h"

vtkStandardNewMacro(vtkSlicerROIRepresentation3D);

//----------------------------------------------------------------------
vtkSlicerROIRepresentation3D::vtkSlicerROIRepresentation3D()
{
  this->ROISource = nullptr;

  this->ROIPipelineInputFilter = vtkSmartPointer<vtkPassThroughFilter>::New();

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
      break;
    default:
      this->ROIActor->SetVisibility(false);
      return;
    }

  this->ROIToWorldTransform->SetMatrix(roiNode->GetInteractionHandleToWorldMatrix());

  this->ROIActor->SetVisibility(true);
  this->VisibilityOn();
  this->PickableOn();

  int controlPointType = Unselected;
  // TODO: Active
  controlPointType = this->GetAllControlPointsSelected() ? vtkSlicerMarkupsWidgetRepresentation::Selected : vtkSlicerMarkupsWidgetRepresentation::Unselected;

  double opacity = displayNode->GetOpacity();
  double fillOpacity = this->MarkupsDisplayNode->GetFillVisibility() ? displayNode->GetFillOpacity() : 0.0;
  this->ROIProperty->DeepCopy(this->GetControlPointsPipeline(controlPointType)->Property);
  this->ROIProperty->SetOpacity(opacity * fillOpacity);

  double occludedOpacity = displayNode->GetOccludedVisibility() ? fillOpacity * displayNode->GetOccludedOpacity() : 0.0;
  this->ROIOccludedProperty->DeepCopy(this->ROIProperty);
  this->ROIOccludedProperty->SetOpacity(opacity * fillOpacity * occludedOpacity);

  this->UpdateRelativeCoincidentTopologyOffsets(this->ROIMapper, this->ROIOccludedMapper);

  double outlineOpacity = this->MarkupsDisplayNode->GetOutlineVisibility()
    ? opacity * this->MarkupsDisplayNode->GetOutlineOpacity() : 0.0;
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
  if (type == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle && index > 5)
    {
    return 1.0;
    }
  return MarkupsInteractionPipeline::GetHandleOpacity(type, index);
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
  double orange[4]    = { 1.00, 0.50, 0.00, 1.00 };
  double purple[4]    = { 1.00, 0.00, 1.00, 1.00 };
  double white[4]     = { 1.00, 1.00, 1.00, 1.00 };
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
    double handlePositionLocal[3] = { 0.0, 0.0, 0.0 };
    double handlePositionWorld[3] = { 0.0, 0.0, 0.0 };
    this->RotationHandlePoints->GetPoint(i, handlePositionLocal);
    this->RotationScaleTransform->GetTransform()->TransformPoint(handlePositionLocal, handlePositionWorld);
    this->HandleToWorldTransform->TransformPoint(handlePositionWorld, handlePositionWorld);
    double color[4] = { 0.0, 0.0, 0.0 };
    this->GetHandleColor(vtkMRMLMarkupsDisplayNode::ComponentRotationHandle, i, color);
    HandleInfo info(i, vtkMRMLMarkupsDisplayNode::ComponentRotationHandle, handlePositionWorld, handlePositionLocal, color);
    handleInfoList.push_back(info);
    }

  for (int i = 0; i < this->TranslationHandlePoints->GetNumberOfPoints(); ++i)
    {
    double handlePositionLocal[3] = { 0.0, 0.0, 0.0 };
    double handlePositionWorld[3] = { 0.0, 0.0, 0.0 };
    this->TranslationHandlePoints->GetPoint(i, handlePositionLocal);
    this->TranslationScaleTransform->GetTransform()->TransformPoint(handlePositionLocal, handlePositionWorld);
    this->HandleToWorldTransform->TransformPoint(handlePositionWorld, handlePositionWorld);
    double color[4] = { 0 };
    this->GetHandleColor(vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle, i, color);
    HandleInfo info(i, vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle, handlePositionWorld, handlePositionLocal, color);
    handleInfoList.push_back(info);
    }

  for (int i = 0; i < this->ScaleHandlePoints->GetNumberOfPoints(); ++i)
    {
    double handlePositionLocal[3] = { 0.0, 0.0, 0.0 };
    double handlePositionWorld[3] = { 0.0, 0.0, 0.0 };
    this->ScaleHandlePoints->GetPoint(i, handlePositionLocal);
    this->HandleToWorldTransform->TransformPoint(handlePositionLocal, handlePositionWorld);
    double color[4] = { 0 };
    this->GetHandleColor(vtkMRMLMarkupsDisplayNode::ComponentScaleHandle, i, color);
    HandleInfo info(i, vtkMRMLMarkupsDisplayNode::ComponentScaleHandle, handlePositionWorld, handlePositionLocal, color);
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
  roiNode->GetSize(sideLengths);
  vtkMath::MultiplyScalar(sideLengths, 0.5);

  vtkNew<vtkPoints> roiPoints;
  roiPoints->SetNumberOfPoints(14);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleLFace,     -sideLengths[0],             0.0,             0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleRFace,      sideLengths[0],             0.0,             0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandlePFace,      0.0,            -sideLengths[1],             0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleAFace,      0.0,             sideLengths[1],             0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleIFace,      0.0,                        0.0, -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleSFace,      0.0,                        0.0,  sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleLPICorner, -sideLengths[0], -sideLengths[1], -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleRPICorner, sideLengths[0],  -sideLengths[1], -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleLAICorner, -sideLengths[0],  sideLengths[1], -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleRAICorner, sideLengths[0],   sideLengths[1], -sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleLPSCorner, -sideLengths[0], -sideLengths[1],  sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleRPSCorner, sideLengths[0],  -sideLengths[1],  sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleLASCorner, -sideLengths[0],  sideLengths[1],  sideLengths[2]);
  roiPoints->SetPoint(vtkMRMLMarkupsROINode::HandleRASCorner, sideLengths[0],   sideLengths[1],  sideLengths[2]);

  vtkMatrix4x4* roiToWorldMatrix = roiNode->GetInteractionHandleToWorldMatrix();
  vtkNew<vtkTransform> worldToHandleTransform;
  worldToHandleTransform->DeepCopy(this->HandleToWorldTransform);
  worldToHandleTransform->Inverse();

  vtkNew<vtkTransform> roiToHandleTransform;
  roiToHandleTransform->Concatenate(roiToWorldMatrix);
  roiToHandleTransform->Concatenate(worldToHandleTransform);

  vtkNew<vtkPolyData> scaleHandlePoints;
  scaleHandlePoints->SetPoints(roiPoints);

  vtkNew<vtkTransformPolyDataFilter> transform;
  transform->SetInputData(scaleHandlePoints);
  transform->SetTransform(roiToHandleTransform);
  transform->Update();

  this->ScaleHandlePoints->SetPoints(transform->GetOutput()->GetPoints());

  vtkIdTypeArray* visibilityArray = vtkIdTypeArray::SafeDownCast(this->ScaleHandlePoints->GetPointData()->GetArray("visibility"));
  visibilityArray->SetNumberOfValues(roiPoints->GetNumberOfPoints());
  visibilityArray->Fill(1);
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
      case vtkMRMLMarkupsROINode::HandleLFace:
        axisWorld[0] = -1.0;
        break;
      case  vtkMRMLMarkupsROINode::HandleRFace:
        axisWorld[0] = 1.0;
        break;
      case vtkMRMLMarkupsROINode::HandlePFace:
        axisWorld[1] = -1.0;
        break;
      case  vtkMRMLMarkupsROINode::HandleAFace:
        axisWorld[1] = 1.0;
        break;
      case vtkMRMLMarkupsROINode::HandleIFace:
        axisWorld[2] = -1.0;
        break;
      case  vtkMRMLMarkupsROINode::HandleSFace:
        axisWorld[2] = 1.0;
        break;
      case vtkMRMLMarkupsROINode::HandleLPICorner:
        axisWorld[0] = -1.0;
        axisWorld[1] = -1.0;
        axisWorld[2] = -1.0;
        break;
      case vtkMRMLMarkupsROINode::HandleRPICorner:
          axisWorld[0] = 1.0;
          axisWorld[1] = -1.0;
          axisWorld[2] = -1.0;
        break;
      case vtkMRMLMarkupsROINode::HandleLAICorner:
        axisWorld[0] = -1.0;
        axisWorld[1] = 1.0;
        axisWorld[2] = -1.0;
        break;
      case vtkMRMLMarkupsROINode::HandleRAICorner:
        axisWorld[0] = 1.0;
        axisWorld[1] = 1.0;
        axisWorld[2] = -1.0;
        break;
      case vtkMRMLMarkupsROINode::HandleLPSCorner:
        axisWorld[0] = -1.0;
        axisWorld[1] = -1.0;
        axisWorld[2] = 1.0;
        break;
      case vtkMRMLMarkupsROINode::HandleRPSCorner:
        axisWorld[0] = 1.0;
        axisWorld[1] = -1.0;
        axisWorld[2] = 1.0;
        break;
      case vtkMRMLMarkupsROINode::HandleLASCorner:
        axisWorld[0] = -1.0;
        axisWorld[1] = 1.0;
        axisWorld[2] = 1.0;
        break;
      case vtkMRMLMarkupsROINode::HandleRASCorner:
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