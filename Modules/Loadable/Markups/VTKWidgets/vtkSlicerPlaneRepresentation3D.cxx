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
#include <vtkArcSource.h>
#include <vtkArrayCalculator.h>
#include <vtkArrowSource.h>
#include <vtkDoubleArray.h>
#include <vtkEllipseArcSource.h>
#include <vtkGlyph3DMapper.h>
#include <vtkLine.h>
#include <vtkLookupTable.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkRegularPolygonSource.h>
#include <vtkRenderer.h>
#include <vtkSlicerPlaneRepresentation3D.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTubeFilter.h>

// MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsPlaneDisplayNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLViewNode.h"

vtkStandardNewMacro(vtkSlicerPlaneRepresentation3D);

enum
{
  ARROW_COMPONENT,
  OUTLINE_COMPONENT,
  FILL_COMPONENT,
};

//----------------------------------------------------------------------
vtkSlicerPlaneRepresentation3D::vtkSlicerPlaneRepresentation3D()
{
  this->ArrowFilter->SetTipResolution(50);

  std::stringstream arrowFunctionSS;
  arrowFunctionSS << ARROW_COMPONENT;
  std::string arrowFunction = arrowFunctionSS.str();

  this->ArrowGlypher->SetSourceConnection(this->ArrowFilter->GetOutputPort());
  this->ArrowGlypher->OrientOn();
  this->ArrowGlypher->ScalingOn();
  this->ArrowGlypher->SetVectorModeToUseVector();
  this->ArrowGlypher->SetScaleModeToDataScalingOff();
  this->ArrowGlypher->SetInputArrayToProcess(1, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);

  this->PlaneOutlineFilter->CappingOff();

  // Color filters
  this->ArrowColorFilter->SetInputConnection(this->ArrowGlypher->GetOutputPort());
  this->ArrowColorFilter->SetFunction(arrowFunction.c_str());
  this->ArrowColorFilter->SetResultArrayName("component");
  this->ArrowColorFilter->SetResultArrayType(VTK_UNSIGNED_CHAR);
  this->ArrowColorFilter->SetAttributeTypeToPointData();

  std::stringstream outlineFunctionSS;
  outlineFunctionSS << OUTLINE_COMPONENT;
  std::string outlineFunction = outlineFunctionSS.str();

  this->PlaneOutlineColorFilter->SetInputConnection(this->PlaneOutlineFilter->GetOutputPort());
  this->PlaneOutlineColorFilter->SetFunction(outlineFunction.c_str());
  this->PlaneOutlineColorFilter->SetResultArrayName("component");
  this->PlaneOutlineColorFilter->SetResultArrayType(VTK_UNSIGNED_CHAR);
  this->PlaneOutlineColorFilter->SetAttributeTypeToPointData();

  std::stringstream fillFunctionSS;
  fillFunctionSS << FILL_COMPONENT;
  std::string fillFunction = fillFunctionSS.str();

  this->PlaneFillColorFilter->SetInputConnection(this->PlaneFillFilter->GetOutputPort());
  this->PlaneFillColorFilter->SetFunction(fillFunction.c_str());
  this->PlaneFillColorFilter->SetResultArrayName("component");
  this->PlaneFillColorFilter->SetResultArrayType(VTK_UNSIGNED_CHAR);
  this->PlaneFillColorFilter->SetAttributeTypeToPointData();

  this->Append->AddInputConnection(this->ArrowColorFilter->GetOutputPort());
  this->Append->AddInputConnection(this->PlaneOutlineColorFilter->GetOutputPort());
  this->Append->AddInputConnection(this->PlaneFillColorFilter->GetOutputPort());

  // Mappers
  this->PlaneMapper->SetInputConnection(this->Append->GetOutputPort());
  this->PlaneMapper->SetColorModeToMapScalars();
  this->PlaneMapper->SetScalarModeToUsePointFieldData();
  this->PlaneMapper->UseLookupTableScalarRangeOn();
  this->PlaneMapper->ScalarVisibilityOn();
  this->PlaneMapper->SetLookupTable(this->PlaneColorLUT);
  this->PlaneMapper->SetArrayAccessMode(VTK_GET_ARRAY_BY_NAME);
  this->PlaneMapper->SetArrayName("component");

  this->PlaneOccludedMapper->SetInputConnection(this->Append->GetOutputPort());
  this->PlaneOccludedMapper->SetColorModeToMapScalars();
  this->PlaneOccludedMapper->SetScalarModeToUsePointFieldData();
  this->PlaneOccludedMapper->UseLookupTableScalarRangeOn();
  this->PlaneOccludedMapper->ScalarVisibilityOn();
  this->PlaneOccludedMapper->SetLookupTable(this->PlaneColorLUT);
  this->PlaneOccludedMapper->SetArrayAccessMode(VTK_GET_ARRAY_BY_NAME);
  this->PlaneOccludedMapper->SetArrayName("component");

  // Actors
  this->PlaneActor->SetMapper(this->PlaneMapper);
  this->PlaneActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->PlaneOccludedActor->SetMapper(this->PlaneOccludedMapper);
  this->PlaneOccludedActor->SetProperty(this->GetControlPointsPipeline(Unselected)->OccludedProperty);
}

//----------------------------------------------------------------------
vtkSlicerPlaneRepresentation3D::~vtkSlicerPlaneRepresentation3D() = default;

//-----------------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  if (this->PlaneActor)
    {
    os << indent << "Plane Visibility: " << this->PlaneActor->GetVisibility() << "\n";
    }
  else
    {
    os << indent << "Plane Visibility: (none)\n";
    }
}


//----------------------------------------------------------------------
bool vtkSlicerPlaneRepresentation3D::GetTransformationReferencePoint(double referencePointWorld[3])
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!planeNode || !planeNode->GetIsPlaneValid())
    {
    return false;
    }
  planeNode->GetOriginWorld(referencePointWorld);
  return true;
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::BuildPlane()
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!planeNode || !planeNode->GetIsPlaneValid())
    {
    this->PlaneActor->SetVisibility(false);
    this->PlaneOccludedActor->SetVisibility(false);
    return;
    }

  double xAxis_World[3] = { 0.0 };
  double yAxis_World[3] = { 0.0 };
  double zAxis_World[3] = { 0.0 };
  planeNode->GetAxesWorld(xAxis_World, yAxis_World, zAxis_World);

  double epsilon = 1e-5;
  if (vtkMath::Norm(xAxis_World) <= epsilon ||
      vtkMath::Norm(yAxis_World) <= epsilon ||
      vtkMath::Norm(zAxis_World) <= epsilon)
    {
    this->PlaneActor->SetVisibility(false);
    this->PlaneOccludedActor->SetVisibility(false);
    return;
    }

  vtkNew<vtkPoints> arrowPoints_World;

  double origin_World[3] = { 0.0 };
  planeNode->GetOriginWorld(origin_World);
  arrowPoints_World->InsertNextPoint(origin_World);

  vtkNew<vtkDoubleArray> arrowDirectionArray_World;
  arrowDirectionArray_World->SetNumberOfComponents(3);
  arrowDirectionArray_World->InsertNextTuple3(zAxis_World[0], zAxis_World[1], zAxis_World[2]);
  arrowDirectionArray_World->SetName("direction");

  vtkNew<vtkPolyData> arrowPositionPolyData_World;
  arrowPositionPolyData_World->SetPoints(arrowPoints_World);
  arrowPositionPolyData_World->GetPointData()->SetScalars(arrowDirectionArray_World);

  this->ArrowGlypher->SetInputData(arrowPositionPolyData_World);

  // Update the plane
  double bounds_Object[4] = { 0.0, -1.0, 0.0, -1.0 };
  planeNode->GetPlaneBounds(bounds_Object);

  vtkNew<vtkMatrix4x4> objectToWorldMatrix;
  planeNode->GetObjectToWorldMatrix(objectToWorldMatrix);

  vtkNew<vtkTransform> objectToWorldTransform;
  objectToWorldTransform->SetMatrix(objectToWorldMatrix);
  double planePoint1_Object[3] = { bounds_Object[0], bounds_Object[2], 0.0 };
  double planePoint1_World[3] = { 0.0, 0.0, 0.0 };
  objectToWorldTransform->TransformPoint(planePoint1_Object, planePoint1_World);

  double planePoint2_Object[3] = { bounds_Object[0], bounds_Object[3], 0.0 };
  double planePoint2_World[3] = { 0.0, 0.0, 0.0 };
  objectToWorldTransform->TransformPoint(planePoint2_Object, planePoint2_World);

  double planePoint3_Object[3] = { bounds_Object[1], bounds_Object[3], 0.0 };
  double planePoint3_World[3] = { 0.0, 0.0, 0.0 };
  objectToWorldTransform->TransformPoint(planePoint3_Object, planePoint3_World);

  double planePoint4_Object[3] = { bounds_Object[1], bounds_Object[2], 0.0 };
  double planePoint4_World[3] = { 0.0, 0.0, 0.0 };
  objectToWorldTransform->TransformPoint(planePoint4_Object, planePoint4_World);

  double planePoint5_World[3] = { 0.0 };
  vtkMath::Add(planePoint1_World, planePoint4_World, planePoint5_World);
  vtkMath::MultiplyScalar(planePoint5_World, 0.5);

  this->PlaneFillFilter->SetOrigin(planePoint1_World);
  this->PlaneFillFilter->SetPoint1(planePoint2_World);
  this->PlaneFillFilter->SetPoint2(planePoint4_World);

  vtkNew<vtkPoints> planePoints_World;
  planePoints_World->InsertNextPoint(planePoint1_World);
  planePoints_World->InsertNextPoint(planePoint2_World);
  planePoints_World->InsertNextPoint(planePoint3_World);
  planePoints_World->InsertNextPoint(planePoint4_World);
  planePoints_World->InsertNextPoint(planePoint5_World);

  // Set the order of plane outline points
  vtkNew<vtkIdList> line;
  line->SetNumberOfIds(6);
  line->SetId(0, 4);
  for (vtkIdType i = 0; i < 4; ++i)
    {
    line->SetId(i + 1, i);
    }
  line->SetId(5, 4);

  vtkNew<vtkCellArray> lines;
  lines->InsertNextCell(line);

  vtkNew<vtkPolyData> outlinePolyData;
  outlinePolyData->SetPoints(planePoints_World);
  outlinePolyData->SetLines(lines);
  this->PlaneOutlineFilter->SetInputDataObject(outlinePolyData);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
{
  Superclass::UpdateFromMRML(caller, event, callData);

  this->NeedToRenderOn();

  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  if (!planeNode || !this->IsDisplayable() || !displayNode)
    {
    this->VisibilityOff();
    return;
    }

  this->VisibilityOn();
  this->PickableOn();

  // Update plane geometry
  this->BuildPlane();

  // Update plane display properties
  this->PlaneActor->SetVisibility(planeNode->GetIsPlaneValid());
  this->PlaneOccludedActor->SetVisibility(this->PlaneActor->GetVisibility() && displayNode->GetOccludedVisibility());

  this->UpdateRelativeCoincidentTopologyOffsets(this->PlaneMapper, this->PlaneOccludedMapper);

  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentPlane)
    {
    controlPointType = this->GetAllControlPointsSelected() ? vtkSlicerMarkupsWidgetRepresentation::Selected : vtkSlicerMarkupsWidgetRepresentation::Unselected;
    }
  this->PlaneActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->PlaneOccludedActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->OccludedProperty);

  // Properties label display
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);
  if (this->MarkupsDisplayNode->GetPropertiesLabelVisibility()
    && planeNode->GetIsPlaneValid()) // including preview
    {
    planeNode->GetOriginWorld(this->TextActorPositionWorld);
    this->TextActor->SetVisibility(true);
    }
  else
    {
    this->TextActor->SetVisibility(false);
    }

  double color[3] = { 0.0 };
  this->GetControlPointsPipeline(controlPointType)->Property->GetColor(color);
  double opacity = this->GetControlPointsPipeline(controlPointType)->Property->GetOpacity();
  this->PlaneColorLUT->SetNumberOfTableValues(3);
  this->PlaneColorLUT->SetTableRange(0.0, 2.0);
  this->PlaneColorLUT->Build();

  double outlineOpacity = this->MarkupsDisplayNode->GetOutlineVisibility() ? opacity * displayNode->GetOutlineOpacity() : 0.0;
  double fillOpacity = this->MarkupsDisplayNode->GetFillVisibility() ? opacity * displayNode->GetFillOpacity() : 0.0;
  double arrowOpacity = opacity * displayNode->GetFillOpacity();

  vtkMRMLMarkupsPlaneDisplayNode* planeDisplayNode = vtkMRMLMarkupsPlaneDisplayNode::SafeDownCast(this->MarkupsDisplayNode);
  if (planeDisplayNode)
    {
    arrowOpacity = planeDisplayNode->GetNormalOpacity();
    arrowOpacity = planeDisplayNode->GetNormalVisibility() ? arrowOpacity : 0.0;
    }

  this->PlaneColorLUT->SetTableValue(ARROW_COMPONENT, color[0], color[1], color[2], arrowOpacity);
  this->PlaneColorLUT->SetTableValue(OUTLINE_COMPONENT, color[0], color[1], color[2], outlineOpacity);
  this->PlaneColorLUT->SetTableValue(FILL_COMPONENT, color[0], color[1], color[2], fillOpacity);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::UpdateInteractionPipeline()
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!planeNode || !planeNode->GetIsPlaneValid())
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }

  MarkupsInteractionPipelinePlane* interactionPipeline = static_cast<MarkupsInteractionPipelinePlane*>(this->InteractionPipeline);
  interactionPipeline->UpdateScaleHandles();

  // Final visibility handled by superclass in vtkSlicerMarkupsWidgetRepresentation
  Superclass::UpdateInteractionPipeline();
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::GetActors(vtkPropCollection *pc)
{
  this->Superclass::GetActors(pc);
  this->PlaneActor->GetActors(pc);
  this->PlaneOccludedActor->GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->Superclass::ReleaseGraphicsResources(win);
  this->PlaneActor->ReleaseGraphicsResources(win);
  this->PlaneOccludedActor->ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerPlaneRepresentation3D::RenderOverlay(vtkViewport *viewport)
{
  int count = this->Superclass::RenderOverlay(viewport);
  if (this->PlaneActor->GetVisibility())
    {
    count += this->PlaneActor->RenderOverlay(viewport);
    }
  if (this->PlaneOccludedActor->GetVisibility())
    {
    count += this->PlaneOccludedActor->RenderOverlay(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerPlaneRepresentation3D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count = this->Superclass::RenderOpaqueGeometry(viewport);
  if (this->PlaneActor->GetVisibility())
    {
    this->ArrowGlypher->SetScaleFactor(this->ControlPointSize * 3);
    this->PlaneOutlineFilter->SetRadius(this->ControlPointSize * 0.25);
    count += this->PlaneActor->RenderOpaqueGeometry(viewport);
    }
  if (this->PlaneOccludedActor->GetVisibility())
    {
    count += this->PlaneOccludedActor->RenderOpaqueGeometry(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerPlaneRepresentation3D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count = this->Superclass::RenderTranslucentPolygonalGeometry(viewport);
  if (this->PlaneActor->GetVisibility())
    {
    // The internal actor needs to share property keys.
    // This ensures the mapper state is consistent and allows depth peeling to work as expected.
    this->PlaneActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->PlaneActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->PlaneOccludedActor->GetVisibility())
    {
    // The internal actor needs to share property keys.
    // This ensures the mapper state is consistent and allows depth peeling to work as expected.
    this->PlaneOccludedActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->PlaneOccludedActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerPlaneRepresentation3D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->PlaneActor->GetVisibility() && this->PlaneActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->PlaneOccludedActor->GetVisibility() && this->PlaneOccludedActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------
double *vtkSlicerPlaneRepresentation3D::GetBounds()
{
  vtkBoundingBox boundingBox;
  const std::vector<vtkProp*> actors({ this->PlaneActor });
  this->AddActorsBounds(boundingBox, actors, Superclass::GetBounds());
  boundingBox.GetBounds(this->Bounds);
  return this->Bounds;
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::CanInteract(
  vtkMRMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if ( !planeNode || planeNode->GetLocked() || !planeNode->GetIsPlaneValid()
    || !interactionEventData )
    {
    return;
    }

  Superclass::CanInteract(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType != vtkMRMLMarkupsDisplayNode::ComponentNone)
    {
    return;
    }

  this->CanInteractWithPlane(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
}

//-----------------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::CanInteractWithPlane(
  vtkMRMLInteractionEventData* interactionEventData,
  int& foundComponentType, int& foundComponentIndex, double& closestDistance2)
{
  // Create the tree
  this->PlaneOutlineFilter->Update();
  if (this->PlaneOutlineFilter->GetOutput() && this->PlaneOutlineFilter->GetOutput()->GetNumberOfPoints() == 0)
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

    double dist2Display = VTK_DOUBLE_MAX;
    double closestPointDisplay[3] = { 0.0, 0.0, 0.0 };

    vtkPolyData* planeOutline = vtkPolyData::SafeDownCast(this->PlaneOutlineFilter->GetInputDataObject(0, 0));
    if (planeOutline)
      {
      for (int pointIndex = 0; pointIndex < planeOutline->GetNumberOfPoints(); ++pointIndex)
        {
        double edgePoint0Display[3] = { 0.0, 0.0, 0.0 };
        planeOutline->GetPoints()->GetPoint(pointIndex, edgePoint0Display);
        this->Renderer->SetWorldPoint(edgePoint0Display);
        this->Renderer->WorldToDisplay();
        this->Renderer->GetDisplayPoint(edgePoint0Display);

        double edgePoint1Display[3] = { 0.0, 0.0, 0.0 };
        if (pointIndex < planeOutline->GetNumberOfPoints() - 1)
          {
          planeOutline->GetPoints()->GetPoint(pointIndex + 1, edgePoint1Display);
          }
        else
          {
          planeOutline->GetPoints()->GetPoint(0, edgePoint1Display);
          }
        this->Renderer->SetWorldPoint(edgePoint1Display);
        this->Renderer->WorldToDisplay();
        this->Renderer->GetDisplayPoint(edgePoint1Display);

        double t;
        double currentClosestPointDisplay[3] = { 0.0, 0.0, 0.0 };
        double currentDistDisplay = vtkLine::DistanceToLine(displayPosition3, edgePoint0Display, edgePoint1Display, t, currentClosestPointDisplay);
        if (currentDistDisplay < dist2Display)
          {
          dist2Display = currentDistDisplay;
          closestPointDisplay[0] = currentClosestPointDisplay[0];
          closestPointDisplay[1] = currentClosestPointDisplay[1];
          closestPointDisplay[2] = currentClosestPointDisplay[2];
          }
        }
      }

    double closestPointWorld[4] = { 0.0, 0.0, 0.0, 1.0 };
    this->Renderer->SetDisplayPoint(closestPointDisplay);
    this->Renderer->DisplayToWorld();
    this->Renderer->GetWorldPoint(closestPointWorld);

    double pixelTolerance = this->PlaneOutlineFilter->GetRadius() / 2.0 / this->GetViewScaleFactorAtPosition(closestPointWorld)
      + this->PickingTolerance * this->ScreenScaleFactor;
    if (dist2Display < pixelTolerance * pixelTolerance && dist2Display < closestDistance2)
      {
      closestDistance2 = dist2Display;
      foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentPlane;
      foundComponentIndex = 0;
      return;
      }
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::SetupInteractionPipeline()
{
  this->InteractionPipeline = new MarkupsInteractionPipelinePlane(this);
  this->InteractionPipeline->InitializePipeline();
}

//-----------------------------------------------------------------------------
vtkSlicerPlaneRepresentation3D::MarkupsInteractionPipelinePlane::MarkupsInteractionPipelinePlane(vtkSlicerMarkupsWidgetRepresentation* representation)
  : MarkupsInteractionPipeline(representation)
{

}

//-----------------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::MarkupsInteractionPipelinePlane::GetHandleColor(int type, int index, double color[4])
{
  if (type != vtkMRMLMarkupsDisplayNode::ComponentScaleHandle)
    {
    MarkupsInteractionPipeline::GetHandleColor(type, index, color);
    return;
    }

  double red[4]       = { 1.00, 0.00, 0.00, 1.00 };
  double green[4]     = { 0.00, 1.00, 0.00, 1.00 };
  double yellow[4]    = { 1.00, 1.00, 0.00, 1.00 };
  double lightGrey[4] = { 0.90, 0.90, 0.90, 1.00 };

  double* currentColor = lightGrey;
  switch (index)
    {
    case vtkMRMLMarkupsPlaneDisplayNode::HandleLEdge:
    case vtkMRMLMarkupsPlaneDisplayNode::HandleREdge:
      currentColor = red;
      break;
    case vtkMRMLMarkupsPlaneDisplayNode::HandlePEdge:
    case vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge:
      currentColor = green;
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

//----------------------------------------------------------------------
double vtkSlicerPlaneRepresentation3D::MarkupsInteractionPipelinePlane::GetHandleOpacity(int type, int index)
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(
    vtkSlicerMarkupsWidgetRepresentation::SafeDownCast(this->Representation)->GetMarkupsNode());
  if (!planeNode)
    {
    return 0.0;
    }

  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(planeNode->GetDisplayNode());

  double opacity = 1.0;
  if (displayNode && type == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle && !displayNode->GetScaleHandleVisibility())
    {
    opacity = 0.0;
    }
  else if (type == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle && index > vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge)
    {
    double viewNormal_World[3] = { 0.0, 0.0, 0.0 };
    this->GetViewPlaneNormal(viewNormal_World);

    double planeNormal_World[3] = { 0.0, 0.0, 0.0 };
    planeNode->GetNormalWorld(planeNormal_World);
    if (vtkMath::Dot(viewNormal_World, planeNormal_World) < 0)
      {
      vtkMath::MultiplyScalar(planeNormal_World, -1);
      }

    double fadeAngleRange = this->StartFadeAngle - this->EndFadeAngle;
    double angle = vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(viewNormal_World, planeNormal_World));
    // Fade happens when the axis approaches 90 degrees from the view normal
    if (angle > 90 - this->EndFadeAngle)
      {
      opacity = 0.0;
      }
    else if (angle > 90 - this->StartFadeAngle)
      {
      double difference = angle - (90 - this->StartFadeAngle);
      opacity = 1.0 - (difference / fadeAngleRange);
      }
    }
  else
    {
    opacity = MarkupsInteractionPipeline::GetHandleOpacity(type, index);
    }

  return opacity;
}

//-----------------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::MarkupsInteractionPipelinePlane::CreateScaleHandles()
{
  MarkupsInteractionPipeline::CreateScaleHandles();
  this->AxisScaleGlypher->SetInputData(this->ScaleHandlePoints);
  this->UpdateScaleHandles();
}

//-----------------------------------------------------------------------------
vtkSlicerPlaneRepresentation3D::HandleInfoList vtkSlicerPlaneRepresentation3D::MarkupsInteractionPipelinePlane::GetHandleInfoList()
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
void vtkSlicerPlaneRepresentation3D::MarkupsInteractionPipelinePlane::UpdateScaleHandles()
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(
    vtkSlicerMarkupsWidgetRepresentation::SafeDownCast(this->Representation)->GetMarkupsNode());
  if (!planeNode)
    {
    return;
    }

  double bounds_Object[4] = { 0.0,  0.0, 0.0, 0.0};
  planeNode->GetPlaneBounds(bounds_Object);

  if (bounds_Object[1] <= bounds_Object[0] || bounds_Object[3] <= bounds_Object[2])
    {
    this->ScaleHandlePoints->SetPoints(vtkNew<vtkPoints>());
    return;
    }

  vtkNew<vtkPoints> roiPoints;
  roiPoints->SetNumberOfPoints(8);
  roiPoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleLEdge, bounds_Object[0], (bounds_Object[2] + bounds_Object[3]) / 2.0, 0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleREdge, bounds_Object[1], (bounds_Object[2] + bounds_Object[3]) / 2.0, 0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge, (bounds_Object[0] + bounds_Object[1]) / 2.0, bounds_Object[2], 0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandlePEdge, (bounds_Object[0] + bounds_Object[1]) / 2.0, bounds_Object[3], 0.0);

  roiPoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner, bounds_Object[0], bounds_Object[2], 0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner, bounds_Object[0], bounds_Object[3], 0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner, bounds_Object[1], bounds_Object[2], 0.0);
  roiPoints->SetPoint(vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner, bounds_Object[1], bounds_Object[3], 0.0);

  this->ScaleHandlePoints->SetPoints(roiPoints);

  vtkIdTypeArray* visibilityArray = vtkIdTypeArray::SafeDownCast(this->ScaleHandlePoints->GetPointData()->GetArray("visibility"));
  visibilityArray->SetNumberOfValues(roiPoints->GetNumberOfPoints());
  visibilityArray->Fill(1);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::MarkupsInteractionPipelinePlane::GetInteractionHandleAxisWorld(int type, int index, double axisWorld[3])
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
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLEdge:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner:
        axisWorld[0] = -1.0;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleREdge:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner:
        axisWorld[0] = 1.0;
        break;
      default:
        break;
      }

    switch (index)
      {
      case vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner:
        axisWorld[1] = -1.0;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandlePEdge:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner:
        axisWorld[1] = 1.0;
        break;
      default:
        break;
      }
    }
  double origin[3] = { 0.0, 0.0, 0.0 };
  this->HandleToWorldTransform->TransformVectorAtPoint(origin, axisWorld, axisWorld);
}
