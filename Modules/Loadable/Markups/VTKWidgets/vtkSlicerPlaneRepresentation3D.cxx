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
#include <vtkArcSource.h>
#include <vtkArrayCalculator.h>
#include <vtkArrowSource.h>
#include <vtkCellLocator.h>
#include <vtkDoubleArray.h>
#include <vtkEllipseArcSource.h>
#include <vtkGlyph3DMapper.h>
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
#include "vtkMRMLMarkupsDisplayNode.h"
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

//----------------------------------------------------------------------
bool vtkSlicerPlaneRepresentation3D::GetTransformationReferencePoint(double referencePointWorld[3])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetNumberOfControlPoints() < 2)
    {
    return false;
    }
  markupsNode->GetNthControlPointPositionWorld(1, referencePointWorld);
  return true;
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::BuildPlane()
{
  vtkMRMLMarkupsPlaneNode* markupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || markupsNode->GetNumberOfControlPoints() != 3)
    {
    this->PlaneActor->SetVisibility(false);
    this->PlaneOccludedActor->SetVisibility(false);
    return;
    }

  double xAxis_World[3] = { 0.0 };
  double yAxis_World[3] = { 0.0 };
  double zAxis_World[3] = { 0.0 };
  markupsNode->GetAxesWorld(xAxis_World, yAxis_World, zAxis_World);

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
  markupsNode->GetOriginWorld(origin_World);
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
  double bounds_Plane[6] = { 0.0 };
  markupsNode->GetPlaneBounds(bounds_Plane);

  double planePoint1_World[3] = { 0.0 };
  double planePoint2_World[3] = { 0.0 };
  double planePoint3_World[3] = { 0.0 };
  double planePoint4_World[3] = { 0.0 };
  double planePoint5_World[3] = { 0.0 };
  for (int i = 0; i < 3; ++i)
    {
    planePoint1_World[i] = origin_World[i]
      + (xAxis_World[i] * bounds_Plane[0])
      + (yAxis_World[i] * bounds_Plane[2]); // Bottom left corner (Plane filter origin)

    planePoint2_World[i] = origin_World[i]
      + (xAxis_World[i] * bounds_Plane[0])
      + (yAxis_World[i] * bounds_Plane[3]); // Top left corner

    planePoint3_World[i] = origin_World[i]
      + (xAxis_World[i] * bounds_Plane[1])
      + (yAxis_World[i] * bounds_Plane[3]); // Top right corner

    planePoint4_World[i] = origin_World[i]
      + (xAxis_World[i] * bounds_Plane[1])
      + (yAxis_World[i] * bounds_Plane[2]); // Bottom right corner

    planePoint5_World[i] = (planePoint1_World[i] + planePoint4_World[i]) / 2.0; // Between bottom left and bottom right
    }
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

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !this->IsDisplayable() || !displayNode)
    {
    this->VisibilityOff();
    return;
    }

  this->VisibilityOn();
  this->PickableOn();

  // Update plane geometry
  this->BuildPlane();

  // Update plane display properties
  this->PlaneActor->SetVisibility(markupsNode->GetNumberOfControlPoints() >= 3);
  this->PlaneOccludedActor->SetVisibility(this->PlaneActor->GetVisibility() && displayNode->GetOccludedVisibility());

  this->TextActor->SetVisibility(this->MarkupsDisplayNode->GetPropertiesLabelVisibility()
    && markupsNode->GetNumberOfControlPoints() > 0);

  this->UpdateRelativeCoincidentTopologyOffsets(this->PlaneMapper, this->PlaneOccludedMapper);

  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentPlane)
    {
    controlPointType = this->GetAllControlPointsSelected() ? vtkSlicerMarkupsWidgetRepresentation::Selected : vtkSlicerMarkupsWidgetRepresentation::Unselected;
    }
  this->PlaneActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);
  this->PlaneOccludedActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->OccludedProperty);
  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);

  double color[3] = { 0.0 };
  this->GetControlPointsPipeline(controlPointType)->Property->GetColor(color);
  double opacity = this->GetControlPointsPipeline(controlPointType)->Property->GetOpacity();
  this->PlaneColorLUT->SetNumberOfTableValues(3);
  this->PlaneColorLUT->SetTableRange(0.0, 2.0);
  this->PlaneColorLUT->Build();

  double outlineOpacity = this->MarkupsDisplayNode->GetOutlineVisibility() ? opacity * displayNode->GetOutlineOpacity() : 0.0;
  double fillOpacity = this->MarkupsDisplayNode->GetFillVisibility() ? opacity * displayNode->GetFillOpacity() : 0.0;
  this->PlaneColorLUT->SetTableValue(ARROW_COMPONENT, color[0], color[1], color[2], opacity);
  this->PlaneColorLUT->SetTableValue(OUTLINE_COMPONENT, color[0], color[1], color[2], outlineOpacity);
  this->PlaneColorLUT->SetTableValue(FILL_COMPONENT, color[0], color[1], color[2], fillOpacity);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::UpdateInteractionPipeline()
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!planeNode || planeNode->GetNumberOfControlPoints() < 3)
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }
  // Final visibility handled by superclass in vtkSlicerMarkupsWidgetRepresentation
  Superclass::UpdateInteractionPipeline();
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::GetActors(vtkPropCollection *pc)
{
  this->Superclass::GetActors(pc);
  this->PlaneActor->GetActors(pc);
  this->PlaneOccludedActor->GetActors(pc);
  this->TextActor->GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneRepresentation3D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->Superclass::ReleaseGraphicsResources(win);
  this->PlaneActor->ReleaseGraphicsResources(win);
  this->PlaneOccludedActor->ReleaseGraphicsResources(win);
  this->TextActor->ReleaseGraphicsResources(win);
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
  if (this->TextActor->GetVisibility())
    {
    count += this->TextActor->RenderOverlay(viewport);
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
  if (this->TextActor->GetVisibility())
    {
    count += this->TextActor->RenderOpaqueGeometry(viewport);
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
  if (this->TextActor->GetVisibility())
    {
    count += this->TextActor->RenderTranslucentPolygonalGeometry(viewport);
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
  if (this->TextActor->GetVisibility() && this->TextActor->HasTranslucentPolygonalGeometry())
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
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if ( !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1
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
  vtkSmartPointer<vtkCellLocator> cellLocator =
    vtkSmartPointer<vtkCellLocator>::New();
  this->PlaneFillFilter->Update();
  if (this->PlaneFillFilter->GetOutput() && this->PlaneFillFilter->GetOutput()->GetNumberOfPoints() == 0)
    {
    return;
    }

  cellLocator->SetDataSet(this->PlaneFillFilter->GetOutput());
  cellLocator->BuildLocator();

  const double* worldPosition = interactionEventData->GetWorldPosition();
  double closestPoint[3];//the coordinates of the closest point will be returned here
  double distance2; //the squared distance to the closest point will be returned here
  vtkIdType cellId; //the cell id of the cell containing the closest point will be returned here
  int subId; //this is rarely used (in triangle strips only, I believe)
  cellLocator->FindClosestPoint(worldPosition, closestPoint, cellId, subId, distance2);

  double toleranceWorld = this->ControlPointSize / 2;
  if (distance2 < toleranceWorld)
    {
    closestDistance2 = distance2;
    foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentPlane;
    foundComponentIndex = 0;
    }
}

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
  if (this->TextActor)
    {
    os << indent << "Text Visibility: " << this->TextActor->GetVisibility() << "\n";
    }
  else
    {
    os << indent << "Text Visibility: (none)\n";
    }
}
