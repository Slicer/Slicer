/*=========================================================================

 Copyright (c) ProxSim ltd., Kwun Tong, Hong Kong. All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
 and development was supported by ProxSim ltd.

=========================================================================*/

// VTK includes
#include "vtkActor2D.h"
#include "vtkCellLocator.h"
#include "vtkCleanPolyData.h"
#include "vtkGlyph3DMapper.h"
#include "vtkLookupTable.h"
#include "vtkPolyDataMapper.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkSlicerCurveRepresentation3D.h"
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include "vtkTubeFilter.h"

// MRML includes
#include "vtkMRMLColorNode.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsDisplayNode.h"

vtkStandardNewMacro(vtkSlicerCurveRepresentation3D);

//----------------------------------------------------------------------
vtkSlicerCurveRepresentation3D::vtkSlicerCurveRepresentation3D()
{
  this->Line = vtkSmartPointer<vtkPolyData>::New();
  this->TubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  this->TubeFilter->SetInputData(this->Line);
  this->TubeFilter->SetNumberOfSides(20);
  this->TubeFilter->SetRadius(1);

  // Mappers
  this->LineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->LineMapper->SetInputConnection(this->TubeFilter->GetOutputPort());

  this->LineOccludedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->LineOccludedMapper->SetInputConnection(this->TubeFilter->GetOutputPort());

  // Actors
  this->LineActor = vtkSmartPointer<vtkActor>::New();
  this->LineActor->SetMapper(this->LineMapper);
  this->LineActor->SetProperty(this->GetControlPointsPipeline(Unselected)->Property);

  this->LineOccludedActor = vtkSmartPointer<vtkActor>::New();
  this->LineOccludedActor->SetMapper(this->LineOccludedMapper);
  this->LineOccludedActor->SetProperty(this->GetControlPointsPipeline(Unselected)->OccludedProperty);

  this->CurvePointLocator = vtkSmartPointer<vtkCellLocator>::New();

  this->HideTextActorIfAllPointsOccluded = true;
}

//----------------------------------------------------------------------
vtkSlicerCurveRepresentation3D::~vtkSlicerCurveRepresentation3D() = default;

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation3D::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
{
  Superclass::UpdateFromMRML(caller, event, callData);

  this->NeedToRenderOn();

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || !this->IsDisplayable())
    {
    this->VisibilityOff();
    return;
    }

  this->VisibilityOn();

  // Properties label display
  // Display if there is at least one control point (even if preview)
  if (this->MarkupsDisplayNode->GetPropertiesLabelVisibility()
    && markupsNode->GetNumberOfDefinedControlPoints(true) > 0) // including preview
    {
    int controlPointIndex = 0;
    int numberOfDefinedControlPoints = markupsNode->GetNumberOfDefinedControlPoints(); // excluding previewed point
    if (numberOfDefinedControlPoints > 0)
      {
      // there is at least one placed point
      controlPointIndex = markupsNode->GetNthControlPointIndexByPositionStatus((numberOfDefinedControlPoints - 1) / 2, vtkMRMLMarkupsNode::PositionDefined);
      }
    else
      {
      // we only have a preview point
      controlPointIndex = markupsNode->GetNthControlPointIndexByPositionStatus(0, vtkMRMLMarkupsNode::PositionPreview);
      }
    // It would be better to show the properties label near a visible segment of the curve
    // but then we may need to iterate through many points whenever the camera rotates, so it may not worth the trouble.
    // Especially because labels could be still poorly positioned. Probably some more sophisticated auto-placement
    // with option for manual label position adjustment (and maybe anchor position adjustment) would be the best.
    markupsNode->GetNthControlPointPositionWorld(controlPointIndex, this->TextActorPositionWorld);
    this->TextActor->SetVisibility(true);
    }
  else
    {
    this->TextActor->SetVisibility(false);
    }

  // Line display

  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
    {
    ControlPointsPipeline3D* controlPoints = this->GetControlPointsPipeline(controlPointType);
    if (controlPointType == Project || controlPointType == ProjectBack)
      {
      // no projection display in 3D
      controlPoints->Actor->SetVisibility(false);
      controlPoints->OccludedActor->SetVisibility(false);
      controlPoints->LabelsActor->SetVisibility(false);
      controlPoints->LabelsOccludedActor->SetVisibility(false);
      continue;
      }

    // For backward compatibility, we hide labels if text scale is set to 0.
    controlPoints->LabelsActor->SetVisibility(this->MarkupsDisplayNode->GetPointLabelsVisibility()
      && this->MarkupsDisplayNode->GetTextScale() > 0.0);
    controlPoints->GlyphMapper->SetScaleFactor(this->ControlPointSize);

    this->UpdateRelativeCoincidentTopologyOffsets(controlPoints->GlyphMapper, controlPoints->OccludedGlyphMapper);
    }

  this->UpdateRelativeCoincidentTopologyOffsets(this->LineMapper, this->LineOccludedMapper);

  double diameter = ( this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ?
    this->MarkupsDisplayNode->GetLineDiameter() : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() );
  this->TubeFilter->SetRadius(diameter * 0.5);

  this->LineActor->SetVisibility(markupsNode->GetNumberOfControlPoints() >= 2);

  bool allControlPointsSelected = this->GetAllControlPointsSelected();
  int controlPointType = Active;
  if (this->MarkupsDisplayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentLine)
    {
    controlPointType = allControlPointsSelected ? Selected : Unselected;
    }
  this->LineActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->Property);

  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(controlPointType)->TextProperty);

  this->LineOccludedActor->SetProperty(this->GetControlPointsPipeline(controlPointType)->OccludedProperty);
  this->LineOccludedActor->SetVisibility(this->MarkupsDisplayNode
    && this->LineActor->GetVisibility()
    && this->MarkupsDisplayNode->GetOccludedVisibility());

  bool allNodesHidden = true;
  for (int controlPointIndex = 0; controlPointIndex < markupsNode->GetNumberOfControlPoints(); controlPointIndex++)
    {
    if (markupsNode->GetNthControlPointPositionVisibility(controlPointIndex)
      && (markupsNode->GetNthControlPointVisibility(controlPointIndex)))
      {
      allNodesHidden = false;
      break;
      }
    }

  if (this->CurveClosed && markupsNode->GetNumberOfControlPoints() > 2 && !allNodesHidden)
    {
    double centerPosWorld[3], orient[3] = { 0 };
    markupsNode->GetCenterOfRotationWorld(centerPosWorld);
    int centerControlPointType = allControlPointsSelected ? Selected : Unselected;
    if (this->MarkupsDisplayNode->GetActiveComponentType() == vtkMRMLMarkupsDisplayNode::ComponentCenterPoint)
      {
      centerControlPointType = Active;
      this->GetControlPointsPipeline(centerControlPointType)->ControlPoints->SetNumberOfPoints(0);
      this->GetControlPointsPipeline(centerControlPointType)->ControlPointsPolyData->GetPointData()->GetNormals()->SetNumberOfTuples(0);
      }
    this->GetControlPointsPipeline(centerControlPointType)->ControlPoints->InsertNextPoint(centerPosWorld);
    this->GetControlPointsPipeline(centerControlPointType)->ControlPointsPolyData->GetPointData()->GetNormals()->InsertNextTuple(orient);

    this->GetControlPointsPipeline(centerControlPointType)->ControlPoints->Modified();
    this->GetControlPointsPipeline(centerControlPointType)->ControlPointsPolyData->GetPointData()->GetNormals()->Modified();
    this->GetControlPointsPipeline(centerControlPointType)->ControlPointsPolyData->Modified();
    if (centerControlPointType == Active)
      {
      this->GetControlPointsPipeline(centerControlPointType)->Actor->VisibilityOn();
      this->GetControlPointsPipeline(centerControlPointType)->LabelsActor->VisibilityOff();
      }
    }

  // Scalars
  this->LineMapper->SetScalarVisibility(this->MarkupsDisplayNode->GetScalarVisibility());
  // if the scalars are visible, set active scalars, the lookup table and the scalar range
  if (this->MarkupsDisplayNode->GetScalarVisibility())
    {
    // Set active display property so that it can be distinguished, given that color cannot be used for this when scalars are visible
    vtkProperty* activePipelineProperty =
      reinterpret_cast<vtkSlicerMarkupsWidgetRepresentation3D::ControlPointsPipeline3D*>(this->ControlPoints[Active])->Property;
    this->PreviousSpecularLightingCoeff = activePipelineProperty->GetSpecular();
    activePipelineProperty->SetSpecular(1.0);

    if (this->LineActor->GetVisibility())
      {
      this->LineMapper->SetScalarModeToUsePointData();

      if (this->MarkupsDisplayNode->GetScalarRangeFlag() == vtkMRMLDisplayNode::UseDirectMapping)
        {
        this->LineMapper->UseLookupTableScalarRangeOn(); // avoid warning about bad table range
        this->LineMapper->SetColorModeToDirectScalars();
        this->LineMapper->SetLookupTable(nullptr);
        }
      else
        {
        this->LineMapper->UseLookupTableScalarRangeOff();
        this->LineMapper->SetColorModeToMapScalars();

        // The renderer uses the lookup table scalar range to render colors. By default, UseLookupTableScalarRange
        // is set to false and SetScalarRange can be used on the mapper to map scalars into the lookup table. When set
        // to true, SetScalarRange has no effect and it is necessary to force the scalarRange on the lookup table manually.
        // Whichever way is used, the look up table range needs to be changed to render the correct scalar values, thus
        // one lookup table can not be shared by multiple mappers if any of those mappers needs to map using its scalar
        // values range. It is therefore necessary to make a copy of the colorNode vtkLookupTable in order not to impact
        // that lookup table original range.
        vtkSmartPointer<vtkLookupTable> dNodeLUT = vtkSmartPointer<vtkLookupTable>::Take(
          this->MarkupsDisplayNode->GetColorNode() ? this->MarkupsDisplayNode->GetColorNode()->CreateLookupTableCopy() : nullptr);
        this->LineMapper->SetLookupTable(dNodeLUT);
        }

      // Set scalar range
      this->LineMapper->SetScalarRange(this->MarkupsDisplayNode->GetScalarRange());
      }
    }
  else
    {
    vtkProperty* activePipelineProperty =
      reinterpret_cast<vtkSlicerMarkupsWidgetRepresentation3D::ControlPointsPipeline3D*>(this->ControlPoints[Active])->Property;
    activePipelineProperty->SetSpecular(this->PreviousSpecularLightingCoeff);
    }
}

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation3D::GetActors(vtkPropCollection *pc)
{
  this->Superclass::GetActors(pc);
  this->LineActor->GetActors(pc);
  this->LineOccludedActor->GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation3D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  this->Superclass::ReleaseGraphicsResources(win);
  this->LineActor->ReleaseGraphicsResources(win);
  this->LineOccludedActor->ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerCurveRepresentation3D::RenderOverlay(vtkViewport *viewport)
{
  int count=0;
  count = this->Superclass::RenderOverlay(viewport);
  if (this->LineActor->GetVisibility())
    {
    count +=  this->LineActor->RenderOverlay(viewport);
    }
  if (this->LineOccludedActor->GetVisibility())
    {
    count +=  this->LineOccludedActor->RenderOverlay(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerCurveRepresentation3D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count=0;
  count = this->Superclass::RenderOpaqueGeometry(viewport);
  if (this->LineActor->GetVisibility())
    {
    double diameter = ( this->MarkupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter ?
      this->MarkupsDisplayNode->GetLineDiameter() : this->ControlPointSize * this->MarkupsDisplayNode->GetLineThickness() );
    this->TubeFilter->SetRadius(diameter * 0.5);
    count += this->LineActor->RenderOpaqueGeometry(viewport);
    }
  if (this->LineOccludedActor->GetVisibility())
    {
    count += this->LineOccludedActor->RenderOpaqueGeometry(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerCurveRepresentation3D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count=0;
  count = this->Superclass::RenderTranslucentPolygonalGeometry(viewport);
  if (this->LineActor->GetVisibility())
    {
    // The internal actor needs to share property keys.
    // This ensures the mapper state is consistent and allows depth peeling to work as expected.
    this->LineActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->LineActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->LineOccludedActor->GetVisibility())
    {
    // The internal actor needs to share property keys.
    // This ensures the mapper state is consistent and allows depth peeling to work as expected.
    this->LineOccludedActor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->LineOccludedActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerCurveRepresentation3D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->LineActor->GetVisibility() && this->LineActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->LineOccludedActor->GetVisibility() && this->LineOccludedActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------
double *vtkSlicerCurveRepresentation3D::GetBounds()
{
  vtkBoundingBox boundingBox;
  const std::vector<vtkProp*> actors({ this->LineActor });
  this->AddActorsBounds(boundingBox, actors, Superclass::GetBounds());
  boundingBox.GetBounds(this->Bounds);
  return this->Bounds;
}

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation3D::CanInteract(
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

  this->CanInteractWithCurve(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
}

//-----------------------------------------------------------------------------
void vtkSlicerCurveRepresentation3D::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  if (this->LineActor)
    {
    os << indent << "Line Visibility: " << this->LineActor->GetVisibility() << "\n";
    }
  else
    {
    os << indent << "Line Visibility: (none)\n";
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerCurveRepresentation3D::SetMarkupsNode(vtkMRMLMarkupsNode *markupsNode)
{
  if (this->MarkupsNode != markupsNode)
  {
    if (markupsNode)
      {
      vtkNew<vtkCleanPolyData> cleaner;
      cleaner->PointMergingOn();
      cleaner->SetInputConnection(markupsNode->GetCurveWorldConnection());
      this->TubeFilter->SetInputConnection(cleaner->GetOutputPort());
      }
    else
      {
      this->TubeFilter->SetInputData(this->Line);
      }
  }
  this->Superclass::SetMarkupsNode(markupsNode);
}

//----------------------------------------------------------------------
void vtkSlicerCurveRepresentation3D::CanInteractWithCurve(
  vtkMRMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &componentIndex, double &closestDistance2)
{
  if (!this->MarkupsNode || this->MarkupsNode->GetLocked()
    || this->MarkupsNode->GetNumberOfControlPoints() < 2
    || !this->GetVisibility() || !interactionEventData)
    {
    return;
    }

  vtkPolyData* curveWorld = this->MarkupsNode->GetCurveWorld();
  if (!curveWorld || curveWorld->GetNumberOfCells()<1)
    {
    return;
    }

  this->CurvePointLocator->SetDataSet(curveWorld);
  this->CurvePointLocator->Update();

  double closestPointDisplay[3] = { 0.0 };
  vtkIdType cellId = -1;
  int subId = -1;
  // dist2 is initialized to -1.0 because this is how FindClosestPoint indicates that no closest point is found
  double dist2 = -1.0;
  if (interactionEventData->IsWorldPositionValid())
    {
    const double* worldPosition = interactionEventData->GetWorldPosition();
    this->CurvePointLocator->FindClosestPoint(worldPosition, closestPointDisplay, cellId, subId, dist2);
    }

  if (dist2>=0 && dist2 < this->ControlPointSize + this->PickingTolerance * this->ScreenScaleFactor * this->ViewScaleFactorMmPerPixel)
    {
    closestDistance2 = dist2 / this->ViewScaleFactorMmPerPixel / this->ViewScaleFactorMmPerPixel;
    foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentLine;
    componentIndex = this->MarkupsNode->GetControlPointIndexFromInterpolatedPointIndex(subId);
    }
}
