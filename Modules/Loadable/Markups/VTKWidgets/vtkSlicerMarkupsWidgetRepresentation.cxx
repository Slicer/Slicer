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
#include "vtkAppendPolyData.h"
#include "vtkArcSource.h"
#include "vtkArrowSource.h"
#include "vtkSlicerMarkupsWidgetRepresentation.h"
#include "vtkCamera.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkFocalPlanePointPlacer.h"
#include "vtkGlyph3D.h"
#include "vtkLine.h"
#include "vtkLineSource.h"
#include "vtkLookupTable.h"
#include "vtkMarkupsGlyphSource2D.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkPointData.h"
#include "vtkPointSetToLabelHierarchy.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"
#include "vtkStringArray.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkTensorGlyph.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTubeFilter.h"

// MRML includes
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLInteractionEventData.h>
#include <vtkMRMLTransformNode.h>

//----------------------------------------------------------------------
static const double INTERACTION_HANDLE_RADIUS = 0.0625;
static const double INTERACTION_HANDLE_DIAMETER = INTERACTION_HANDLE_RADIUS * 2.0;
static const double INTERACTION_HANDLE_ROTATION_ARC_TUBE_RADIUS = INTERACTION_HANDLE_RADIUS * 0.4;
static const double INTERACTION_HANDLE_ROTATION_ARC_RADIUS = 0.80;
static const double INTERACTION_HANDLE_SCALE_FACTOR = 3.5;

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation::ControlPointsPipeline::ControlPointsPipeline()
{
  this->TextProperty = vtkSmartPointer<vtkTextProperty>::New();
  this->TextProperty->SetFontSize(15);
  this->TextProperty->SetFontFamily(vtkTextProperty::GetFontFamilyFromString("Arial"));
  this->TextProperty->SetColor(0.4, 1.0, 1.0);
  this->TextProperty->SetOpacity(1.);

  this->ControlPoints = vtkSmartPointer<vtkPoints>::New();
  this->ControlPoints->Allocate(100);
  this->ControlPoints->SetNumberOfPoints(1);
  this->ControlPoints->SetPoint(0, 0.0, 0.0, 0.0);

  vtkNew<vtkDoubleArray> controlPointNormals;
  controlPointNormals->SetNumberOfComponents(3);
  controlPointNormals->Allocate(100);
  controlPointNormals->SetNumberOfTuples(1);
  double n[3] = { 0, 0, 0 };
  controlPointNormals->SetTuple(0, n);

  this->ControlPointsPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->ControlPointsPolyData->SetPoints(this->ControlPoints);
  this->ControlPointsPolyData->GetPointData()->SetNormals(controlPointNormals);

  this->LabelControlPoints = vtkSmartPointer<vtkPoints>::New();
  this->LabelControlPoints->Allocate(100);
  this->LabelControlPoints->SetNumberOfPoints(1);
  this->LabelControlPoints->SetPoint(0, 0.0, 0.0, 0.0);

  vtkNew<vtkDoubleArray> labelNormals;
  labelNormals->SetNumberOfComponents(3);
  labelNormals->Allocate(100);
  labelNormals->SetNumberOfTuples(1);
  labelNormals->SetTuple(0, n);

  this->LabelControlPointsPolyData = vtkSmartPointer<vtkPolyData>::New();
  this->LabelControlPointsPolyData->SetPoints(this->LabelControlPoints);
  this->LabelControlPointsPolyData->GetPointData()->SetNormals(labelNormals);

  this->Labels = vtkSmartPointer<vtkStringArray>::New();
  this->Labels->SetName("labels");
  this->Labels->Allocate(100);
  this->Labels->SetNumberOfValues(1);
  this->Labels->SetValue(0, "F");
  this->LabelsPriority = vtkSmartPointer<vtkStringArray>::New();
  this->LabelsPriority->SetName("priority");
  this->LabelsPriority->Allocate(100);
  this->LabelsPriority->SetNumberOfValues(1);
  this->LabelsPriority->SetValue(0, "1");
  this->LabelControlPointsPolyData->GetPointData()->AddArray(this->Labels);
  this->LabelControlPointsPolyData->GetPointData()->AddArray(this->LabelsPriority);
  this->PointSetToLabelHierarchyFilter = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
  this->PointSetToLabelHierarchyFilter->SetTextProperty(this->TextProperty);
  this->PointSetToLabelHierarchyFilter->SetLabelArrayName("labels");
  this->PointSetToLabelHierarchyFilter->SetPriorityArrayName("priority");
  this->PointSetToLabelHierarchyFilter->SetInputData(this->LabelControlPointsPolyData);

  this->GlyphSource2D = vtkSmartPointer<vtkMarkupsGlyphSource2D>::New();

  this->GlyphSourceSphere = vtkSmartPointer<vtkSphereSource>::New();
  this->GlyphSourceSphere->SetRadius(0.5);
};

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation::ControlPointsPipeline::~ControlPointsPipeline() = default;

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation::vtkSlicerMarkupsWidgetRepresentation()
{
  this->ViewScaleFactorMmPerPixel = 1.0;
  this->ScreenSizePixel = 1000;

  this->ControlPointSize = 3.0;
  this->NeedToRender = false;
  this->ClosedLoop = 0;

  this->TextActor = vtkSmartPointer<vtkTextActor>::New();
  // hide by default, if a concrete class implements properties display, it will enable it
  this->TextActor->SetVisibility(false);

  this->PointPlacer = vtkSmartPointer<vtkFocalPlanePointPlacer>::New();

  for (int i = 0; i<NumberOfControlPointTypes; i++)
    {
    this->ControlPoints[i] = nullptr;
    }

  this->AlwaysOnTop = false;

  this->InteractionPipeline = nullptr;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::SetupInteractionPipeline()
{
  this->InteractionPipeline = new MarkupsInteractionPipeline(this);
  this->InteractionPipeline->InitializePipeline();
}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation::~vtkSlicerMarkupsWidgetRepresentation()
{
  for (int i=0; i<NumberOfControlPointTypes; i++)
    {
    delete this->ControlPoints[i];
    this->ControlPoints[i] = nullptr;
    }
  // Force deleting variables to prevent circular dependency keeping objects alive
  this->PointPlacer = nullptr;

  if (this->InteractionPipeline != nullptr)
    {
    delete this->InteractionPipeline;
    this->InteractionPipeline = nullptr;
    }
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation::GetNumberOfControlPoints()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return 0;
    }
  return markupsNode->GetNumberOfControlPoints();
}

//----------------------------------------------------------------------
// The display position for a given world position must be re-computed
// from the world positions... It should not be queried from the renderer
// whose camera position may have changed
int vtkSlicerMarkupsWidgetRepresentation::GetNthControlPointDisplayPosition(int n, double displayPos[2])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || n < 0 || n >= markupsNode->GetNumberOfControlPoints())
    {
    return 0;
    }

  double pos[4] = { 0.0, 0.0, 0.0, 1.0 };
  markupsNode->TransformPointToWorld(this->GetNthControlPoint(n)->Position, pos);

  this->Renderer->SetWorldPoint(pos);
  this->Renderer->WorldToDisplay();
  this->Renderer->GetDisplayPoint(pos);

  displayPos[0] = pos[0];
  displayPos[1] = pos[1];
  return 1;
}

//----------------------------------------------------------------------
vtkMRMLMarkupsNode::ControlPoint* vtkSlicerMarkupsWidgetRepresentation::GetNthControlPoint(int n)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || n < 0 || n >= markupsNode->GetNumberOfControlPoints())
    {
    return nullptr;
    }
  return markupsNode->GetNthControlPoint(n);
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation::FindClosestPointOnWidget(
  const int displayPos[2], double closestWorldPos[3], int *idx)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return 0;
    }

  // Make a line out of this viewing ray
  double p1[4] = { 0.0, 0.0, 0.0, 1.0 };
  double tmp1[4] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]), 0.0, 1.0 };
  this->Renderer->SetDisplayPoint(tmp1);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(p1);

  double p2[4] = { 0.0, 0.0, 0.0, 1.0 };
  tmp1[2] = 1.0;
  this->Renderer->SetDisplayPoint(tmp1);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(p2);

  double closestDistance2 = VTK_DOUBLE_MAX;
  int closestNode = 0;

  // compute a world tolerance based on pixel
  // tolerance on the focal plane
  double fp[4] = { 0.0, 0.0, 0.0, 1.0 };
  this->Renderer->GetActiveCamera()->GetFocalPoint(fp);
  this->Renderer->SetWorldPoint(fp);
  this->Renderer->WorldToDisplay();
  this->Renderer->GetDisplayPoint(tmp1);

  tmp1[0] = 0;
  tmp1[1] = 0;
  double tmp2[4] = { 0.0, 0.0, 0.0, 1.0 };
  this->Renderer->SetDisplayPoint(tmp1);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(tmp2);

  tmp1[0] = this->PickingTolerance * this->ScreenScaleFactor;
  this->Renderer->SetDisplayPoint(tmp1);
  this->Renderer->DisplayToWorld();
  this->Renderer->GetWorldPoint(tmp1);

  double wt2 = vtkMath::Distance2BetweenPoints(tmp1, tmp2);


  // Now loop through all lines and look for closest one within tolerance
  double p3[4] = {0.0, 0.0, 0.0, 1.0};
  double p4[4] = {0.0, 0.0, 0.0, 1.0};
  vtkPoints* curvePointsWorld = this->GetMarkupsNode()->GetCurvePointsWorld();
  vtkIdType numberOfPoints = curvePointsWorld->GetNumberOfPoints();
  for(vtkIdType i = 0; i < numberOfPoints; i++)
    {
    curvePointsWorld->GetPoint(i, p3);
    if (i + 1 < numberOfPoints)
      {
      curvePointsWorld->GetPoint(i + 1, p4);
      }
    else
      {
      if (!this->ClosedLoop)
        {
        continue;
        }
      curvePointsWorld->GetPoint(0, p4);
      }

    // Now we have the four points - check closest intersection
    double u, v;
    if (vtkLine::Intersection(p1, p2, p3, p4, u, v))
      {
      double p5[3], p6[3];
      p5[0] = p1[0] + u*(p2[0]-p1[0]);
      p5[1] = p1[1] + u*(p2[1]-p1[1]);
      p5[2] = p1[2] + u*(p2[2]-p1[2]);

      p6[0] = p3[0] + v*(p4[0]-p3[0]);
      p6[1] = p3[1] + v*(p4[1]-p3[1]);
      p6[2] = p3[2] + v*(p4[2]-p3[2]);

      double d = vtkMath::Distance2BetweenPoints(p5, p6);

      if (d < wt2 && d < closestDistance2)
        {
        closestWorldPos[0] = p6[0];
        closestWorldPos[1] = p6[1];
        closestWorldPos[2] = p6[2];
        closestDistance2 = d;
        closestNode = static_cast<int>(i);
        }
      }
    else
      {
      double d = vtkLine::DistanceToLine(p3, p1, p2);
      if (d < wt2 && d < closestDistance2)
        {
        closestWorldPos[0] = p3[0];
        closestWorldPos[1] = p3[1];
        closestWorldPos[2] = p3[2];
        closestDistance2 = d;
        closestNode = static_cast<int>(i);
        }

      d = vtkLine::DistanceToLine(p4, p1, p2);
      if (d < wt2 && d < closestDistance2)
        {
        closestWorldPos[0] = p4[0];
        closestWorldPos[1] = p4[1];
        closestWorldPos[2] = p4[2];
        closestDistance2 = d;
        closestNode = static_cast<int>(i);
        }
      }
    }

  if (closestDistance2 < VTK_DOUBLE_MAX)
    {
    if (closestNode < markupsNode->GetNumberOfControlPoints() -1)
      {
      *idx = closestNode+1;
      return 1;
      }
    else if (this->ClosedLoop)
      {
      *idx = 0;
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::UpdateCenter()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetNumberOfControlPoints() < 1)
    {
    return;
    }
  double centerWorldPos[3] = { 0.0 };

  for (int i = 0; i < markupsNode->GetNumberOfControlPoints(); i++)
    {
    double p[4];
    markupsNode->GetNthControlPointPositionWorld(i, p);
    centerWorldPos[0] += p[0];
    centerWorldPos[1] += p[1];
    centerWorldPos[2] += p[2];
    }
  double inv_N = 1. / static_cast< double >(markupsNode->GetNumberOfControlPoints());
  centerWorldPos[0] *= inv_N;
  centerWorldPos[1] *= inv_N;
  centerWorldPos[2] *= inv_N;

  markupsNode->SetCenterPositionFromPointer(centerWorldPos);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::SetMarkupsDisplayNode(vtkMRMLMarkupsDisplayNode *markupsDisplayNode)
{
  if (this->MarkupsDisplayNode == markupsDisplayNode)
    {
    return;
    }

  this->MarkupsDisplayNode = markupsDisplayNode;

  vtkMRMLMarkupsNode* markupsNode = nullptr;
  if (this->MarkupsDisplayNode)
    {
    markupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->MarkupsDisplayNode->GetDisplayableNode());
    }
  this->SetMarkupsNode(markupsNode);
}

//----------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode *vtkSlicerMarkupsWidgetRepresentation::GetMarkupsDisplayNode()
{
  return this->MarkupsDisplayNode;
}

//----------------------------------------------------------------------
vtkMRMLMarkupsNode *vtkSlicerMarkupsWidgetRepresentation::GetMarkupsNode()
{
  if (!this->MarkupsDisplayNode)
    {
    return nullptr;
    }
  return vtkMRMLMarkupsNode::SafeDownCast(this->MarkupsDisplayNode->GetDisplayableNode());
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::SetMarkupsNode(vtkMRMLMarkupsNode *markupsNode)
{
  this->MarkupsNode = markupsNode;
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::PrintSelf(ostream& os,
                                                      vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Point Placer: " << this->PointPlacer << "\n";
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::CanInteract(
  vtkMRMLInteractionEventData* vtkNotUsed(interactionEventData),
  int &foundComponentType, int &vtkNotUsed(foundComponentIndex), double &vtkNotUsed(closestDistance2))
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation::GetTransformationReferencePoint(double referencePointWorld[3])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return false;
    }
  this->UpdateCenter();
  markupsNode->GetCenterPosition(referencePointWorld);
  return true;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::BuildLine(vtkPolyData* linePolyData, bool displayPosition)
{
  vtkNew<vtkPoints> points;
  vtkNew<vtkCellArray> line;

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    linePolyData->SetPoints(points);
    linePolyData->SetLines(line);
    return;
    }
  int numberOfControlPoints = markupsNode->GetNumberOfControlPoints();
  vtkIdType numberOfLines = numberOfControlPoints - 1;
  bool loop = (markupsNode->GetCurveClosed() && numberOfControlPoints > 2);
  if (loop)
    {
    numberOfLines++;
    }
  if (numberOfLines <= 0)
    {
    return;
    }

  double pos[3] = { 0.0 };
  vtkIdType index = 0;
  line->InsertNextCell(numberOfLines+1);

  for (int i = 0; i < numberOfControlPoints; i++)
    {
    // Add the node
    if (displayPosition)
      {
      this->GetNthControlPointDisplayPosition(i, pos);
      }
    else
      {
      markupsNode->GetNthControlPointPositionWorld(i, pos);
      }
    points->InsertNextPoint(pos);
    line->InsertCellPoint(i);
    index++;
    }

  if (loop)
    {
    if (displayPosition)
      {
      this->GetNthControlPointDisplayPosition(0, pos);
      }
    else
      {
      markupsNode->GetNthControlPointPositionWorld(0, pos);
      }
    points->InsertPoint(index, pos);
    line->InsertCellPoint(0);
    }

  linePolyData->SetPoints(points);
  linePolyData->SetLines(line);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::UpdateFromMRML(
    vtkMRMLNode* vtkNotUsed(caller), unsigned long event, void *vtkNotUsed(callData))
{
  if (!this->InteractionPipeline)
    {
    this->SetupInteractionPipeline();
    }

  if (!event || event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    this->MarkupsTransformModifiedTime.Modified();
    }

  if (!event || event == vtkMRMLDisplayableNode::DisplayModifiedEvent)
    {
    // Update MRML data node from display node
    vtkMRMLMarkupsNode* markupsNode = nullptr;
    if (this->MarkupsDisplayNode)
      {
      markupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->MarkupsDisplayNode->GetDisplayableNode());
      }
    this->SetMarkupsNode(markupsNode);
    }

  if (this->MarkupsNode)
    {
    this->TextActor->SetInput(this->MarkupsNode->GetPropertiesLabelText().c_str());
    }
  else
    {
    this->TextActor->SetInput("");
    }


  this->NeedToRenderOn(); // TODO: to improve performance, call this only if it is actually needed

  if (this->InteractionPipeline)
    {
    this->UpdateInteractionPipeline();
    }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::UpdateInteractionPipeline()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || markupsNode->GetNumberOfControlPoints() < 1)
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }

  if (!this->MarkupsDisplayNode)
    {
    this->InteractionPipeline->Actor->SetVisibility(false);
    return;
    }

  this->InteractionPipeline->Actor->SetVisibility(this->MarkupsDisplayNode->GetHandlesInteractive());

  vtkNew<vtkTransform> handleToWorldTransform;
  handleToWorldTransform->SetMatrix(markupsNode->GetInteractionHandleToWorldMatrix());
  this->InteractionPipeline->HandleToWorldTransform->DeepCopy(handleToWorldTransform);
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation::GetAllControlPointsVisible()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return false;
    }

  for (int controlPointIndex = 0; controlPointIndex < markupsNode->GetNumberOfControlPoints(); controlPointIndex++)
    {
    if (!markupsNode->GetNthControlPointVisibility(controlPointIndex))
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation::GetAllControlPointsSelected()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return false;
    }

  for (int controlPointIndex = 0; controlPointIndex < markupsNode->GetNumberOfControlPoints(); controlPointIndex++)
    {
    if (!markupsNode->GetNthControlPointSelected(controlPointIndex))
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------
double* vtkSlicerMarkupsWidgetRepresentation::GetWidgetColor(int controlPointType)
{
  static double invalidColor[3] = { 0.5, 0.5, 0.5 }; // gray
  static double activeColor[3] = { 0.4, 1.0, 0. }; // bright green
  static double color[3];

  if (!this->MarkupsDisplayNode)
    {
    return invalidColor;
    }

  // If a folder is overriding display properties then return the color defined by the folder
  if (this->MarkupsDisplayNode->GetFolderDisplayOverrideAllowed())
    {
    vtkMRMLDisplayableNode* displayableNode = this->MarkupsDisplayNode->GetDisplayableNode();
    vtkMRMLDisplayNode* overrideHierarchyDisplayNode =
      vtkMRMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(displayableNode);
    if (overrideHierarchyDisplayNode)
      {
      overrideHierarchyDisplayNode->GetColor(color);
      if (controlPointType == Active)
        {
        return activeColor;
        }
      else
        {
        // Folder display has "selected" color but since usually for folders we only set
        // the plain Color property, we always use this plain color, regardless of selected state
        // of the control point.
        overrideHierarchyDisplayNode->GetColor(color);
        return color;
        }
      }
    }

  switch (controlPointType)
    {
    case Unselected:
      this->MarkupsDisplayNode->GetColor(color);
      break;
    case Selected:
      this->MarkupsDisplayNode->GetSelectedColor(color);
      break;
    case Active: return activeColor;
    case Project:
      if (this->MarkupsDisplayNode->GetSliceProjectionUseFiducialColor())
        {
        if (this->GetAllControlPointsSelected())
          {
          this->MarkupsDisplayNode->GetSelectedColor(color);
          }
        else
          {
          this->MarkupsDisplayNode->GetColor(color);
          }
        }
      else
        {
        this->MarkupsDisplayNode->GetSliceProjectionColor(color);
        }
      break;
    case ProjectBack:
      if (this->MarkupsDisplayNode->GetSliceProjectionUseFiducialColor())
        {
        if (this->GetAllControlPointsSelected())
          {
          this->MarkupsDisplayNode->GetSelectedColor(color);
          }
        else
          {
          this->MarkupsDisplayNode->GetColor(color);
          }
        }
      else
        {
        this->MarkupsDisplayNode->GetSliceProjectionColor(color);
        }
      for (int i = 0; i < 3; i++)
        {
        if (fabs(color[1] - 1.) > 0.001)
          {
          color[i] = 1. - color[i];
          }
        }
      break;
    default:
      return invalidColor;
    }

  return color;
}

//----------------------------------------------------------------------
vtkPointPlacer* vtkSlicerMarkupsWidgetRepresentation::GetPointPlacer()
{
  return this->PointPlacer;
}

//----------------------------------------------------------------------
vtkPolyData* vtkSlicerMarkupsWidgetRepresentation::GetControlPointsPolyData(int controlPointType)
{
  if (controlPointType < 0 || controlPointType >= NumberOfControlPointTypes)
    {
    vtkErrorMacro("vtkSlicerMarkupsWidgetRepresentation::GetControlPointsPolyData failed: invalid controlPointType: " << controlPointType);
    return nullptr;
    }
  return this->ControlPoints[controlPointType]->ControlPointsPolyData;
}

//----------------------------------------------------------------------
vtkPolyData* vtkSlicerMarkupsWidgetRepresentation::GetLabelControlPointsPolyData(int controlPointType)
{
  if (controlPointType < 0 || controlPointType >= NumberOfControlPointTypes)
    {
    vtkErrorMacro("vtkSlicerMarkupsWidgetRepresentation::GetLabelControlPointsPolyData failed: invalid controlPointType: " << controlPointType);
    return nullptr;
    }
  return this->ControlPoints[controlPointType]->LabelControlPointsPolyData;
}

//----------------------------------------------------------------------
vtkStringArray* vtkSlicerMarkupsWidgetRepresentation::GetLabels(int controlPointType)
{
  if (controlPointType < 0 || controlPointType >= NumberOfControlPointTypes)
    {
    vtkErrorMacro("vtkSlicerMarkupsWidgetRepresentation::GetControlPointsPolyData failed: invalid controlPointType: " << controlPointType);
    return nullptr;
    }
  return this->ControlPoints[controlPointType]->Labels;
}

//-----------------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation::IsDisplayable()
{
  if (!this->MarkupsDisplayNode
    || !this->ViewNode
    || !this->MarkupsDisplayNode->GetVisibility()
    || !this->MarkupsDisplayNode->IsDisplayableInView(this->ViewNode->GetID()))
    {
    return false;
    }

  // If parent folder visibility is set to false then the markups is not visible
  if (this->MarkupsDisplayNode->GetFolderDisplayOverrideAllowed())
    {
    vtkMRMLDisplayableNode* displayableNode = this->MarkupsDisplayNode->GetDisplayableNode();
    // Visibility is applied regardless the fact whether there is override or not.
    // Visibility of items defined by hierarchy is off if any of the ancestors is explicitly hidden.
    // However, this does not apply on display nodes that do not allow overrides (FolderDisplayOverrideAllowed)
    if (!vtkMRMLFolderDisplayNode::GetHierarchyVisibility(displayableNode))
      {
      return false;
      }
    }
  if (vtkMRMLSliceNode::SafeDownCast(this->ViewNode))
    {
    if (!this->MarkupsDisplayNode->GetVisibility2D())
      {
      return false;
      }
    }
  if (vtkMRMLViewNode::SafeDownCast(this->ViewNode))
    {
    if (!this->MarkupsDisplayNode->GetVisibility3D())
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::GetActors(vtkPropCollection* pc)
{
  if (this->InteractionPipeline)
    {
    this->InteractionPipeline->Actor->GetActors(pc);
    }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::ReleaseGraphicsResources(vtkWindow* window)
{
  if (this->InteractionPipeline)
    {
    this->InteractionPipeline->Actor->ReleaseGraphicsResources(window);
    }
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation::RenderOverlay(vtkViewport* viewport)
{
  int count = 0;
  if (this->InteractionPipeline && this->InteractionPipeline->Actor->GetVisibility())
    {
    count += this->InteractionPipeline->Actor->RenderOverlay(viewport);
    }
  return count;
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation::RenderOpaqueGeometry(vtkViewport* viewport)
{
  int count = 0;
  if (this->InteractionPipeline && this->InteractionPipeline->Actor->GetVisibility())
    {
    this->InteractionPipeline->UpdateHandleColors();
    double interactionWidgetScale = INTERACTION_HANDLE_SCALE_FACTOR * this->ControlPointSize;
    this->InteractionPipeline->SetWidgetScale(interactionWidgetScale);
    count += this->InteractionPipeline->Actor->RenderOpaqueGeometry(viewport);
    }
  return count;
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
{
  int count = 0;
  if (this->InteractionPipeline && this->InteractionPipeline->Actor->GetVisibility())
    {
    this->InteractionPipeline->Actor->SetPropertyKeys(this->GetPropertyKeys());
    count += this->InteractionPipeline->Actor->RenderTranslucentPolygonalGeometry(viewport);
    }
  return count;
}

//----------------------------------------------------------------------
vtkTypeBool vtkSlicerMarkupsWidgetRepresentation::HasTranslucentPolygonalGeometry()
{
  if (this->InteractionPipeline && this->InteractionPipeline->Actor->GetVisibility() &&
    this->InteractionPipeline->Actor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::GetInteractionHandleAxisWorld(int index, double axis[3])
{
  this->InteractionPipeline->GetInteractionHandleAxisWorld(index, axis);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::GetInteractionHandleOriginWorld(double origin[3])
{
  this->InteractionPipeline->GetInteractionHandleOriginWorld(origin);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::GetInteractionHandleVector(int type, int index, double axis[3])
{
  vtkPolyData* handles = nullptr;
  if (type == vtkMRMLMarkupsDisplayNode::ComponentRotationHandle)
    {
    handles = this->InteractionPipeline->RotationHandlePoints;
    }
  else if (type == vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle)
    {
    handles = this->InteractionPipeline->TranslationHandlePoints;
    }

  if (!handles)
    {
    vtkErrorMacro("GetInteractionHandleVector: Could not find interaction handles!");
    return;
    }

  if (index  < 0 || index >= handles->GetNumberOfPoints())
    {
    vtkErrorMacro("GetInteractionHandleVector: Handle index out of range!");
    return;
    }

  handles->GetPoint(index, axis);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::GetInteractionHandleVectorWorld(int type, int index, double axisWorld[3])
{
  if (!axisWorld)
    {
    vtkErrorMacro("GetInteractionHandleVectorWorld: Invalid axis argument!");
    }

  this->GetInteractionHandleVector(type, index, axisWorld);
  double origin[3] = { 0 };
  this->InteractionPipeline->HandleToWorldTransform->TransformVectorAtPoint(origin, axisWorld, axisWorld);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::GetInteractionHandlePositionWorld(int type, int index, double positionWorld[3])
{
  if (!positionWorld)
    {
    vtkErrorMacro("GetInteractionHandlePositionWorld: Invalid position argument!");
    }

  if (type == vtkMRMLMarkupsDisplayNode::ComponentRotationHandle)
    {
    this->InteractionPipeline->RotationHandlePoints->GetPoint(index, positionWorld);
    this->InteractionPipeline->RotationScaleTransform->GetTransform()->TransformPoint(positionWorld, positionWorld);
    this->InteractionPipeline->HandleToWorldTransform->TransformPoint(positionWorld, positionWorld);
    }
  else if (type == vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle)
    {
    this->InteractionPipeline->TranslationHandlePoints->GetPoint(index, positionWorld);
    this->InteractionPipeline->TranslationScaleTransform->GetTransform()->TransformPoint(positionWorld, positionWorld);
    this->InteractionPipeline->HandleToWorldTransform->TransformPoint(positionWorld, positionWorld);
    }
}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::MarkupsInteractionPipeline(vtkMRMLAbstractWidgetRepresentation* representation)
{
  this->Representation = representation;

  this->AxisRotationHandleSource = vtkSmartPointer<vtkSphereSource>::New();
  this->AxisRotationHandleSource->SetRadius(INTERACTION_HANDLE_RADIUS);
  this->AxisRotationHandleSource->SetPhiResolution(16);
  this->AxisRotationHandleSource->SetThetaResolution(16);

  this->AxisRotationArcSource = vtkSmartPointer<vtkArcSource>::New();
  this->AxisRotationArcSource->SetAngle(90);
  this->AxisRotationArcSource->SetCenter(-INTERACTION_HANDLE_ROTATION_ARC_RADIUS, 0, 0);
  this->AxisRotationArcSource->SetPoint1(
    INTERACTION_HANDLE_ROTATION_ARC_RADIUS / sqrt(2) - INTERACTION_HANDLE_ROTATION_ARC_RADIUS,
   -INTERACTION_HANDLE_ROTATION_ARC_RADIUS/sqrt(2), 0);
  this->AxisRotationArcSource->SetPoint2(
    INTERACTION_HANDLE_ROTATION_ARC_RADIUS / sqrt(2) - INTERACTION_HANDLE_ROTATION_ARC_RADIUS,
    INTERACTION_HANDLE_ROTATION_ARC_RADIUS/sqrt(2), 0);
  this->AxisRotationArcSource->SetResolution(16);

  this->AxisRotationTubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  this->AxisRotationTubeFilter->SetInputConnection(this->AxisRotationArcSource->GetOutputPort());
  this->AxisRotationTubeFilter->SetRadius(INTERACTION_HANDLE_ROTATION_ARC_TUBE_RADIUS);
  this->AxisRotationTubeFilter->SetNumberOfSides(16);
  this->AxisRotationTubeFilter->SetCapping(true);

  this->AxisRotationGlyphSource = vtkSmartPointer <vtkAppendPolyData>::New();
  this->AxisRotationGlyphSource->AddInputConnection(this->AxisRotationHandleSource->GetOutputPort());
  this->AxisRotationGlyphSource->AddInputConnection(this->AxisRotationTubeFilter->GetOutputPort());

  this->AxisTranslationGlyphSource = vtkSmartPointer<vtkArrowSource>::New();
  this->AxisTranslationGlyphSource->SetTipRadius(INTERACTION_HANDLE_RADIUS);
  this->AxisTranslationGlyphSource->SetTipLength(INTERACTION_HANDLE_DIAMETER);
  this->AxisTranslationGlyphSource->SetTipResolution(16);
  this->AxisTranslationGlyphSource->SetShaftResolution(16);
  this->AxisTranslationGlyphSource->InvertOn();

  vtkNew<vtkTransform> translationGlyphTransformer;
  translationGlyphTransformer->Translate(INTERACTION_HANDLE_RADIUS, 0, 0);
  translationGlyphTransformer->RotateY(180);

  this->AxisTranslationGlyphTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->AxisTranslationGlyphTransformer->SetTransform(translationGlyphTransformer);
  this->AxisTranslationGlyphTransformer->SetInputConnection(this->AxisTranslationGlyphSource->GetOutputPort());

  this->RotationHandlePoints = vtkSmartPointer<vtkPolyData>::New();
  this->TranslationHandlePoints = vtkSmartPointer<vtkPolyData>::New();

  this->RotationScaleTransform = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->RotationScaleTransform->SetInputData(this->RotationHandlePoints);
  this->RotationScaleTransform->SetTransform(vtkNew<vtkTransform>());

  this->TranslationScaleTransform = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->TranslationScaleTransform->SetInputData(this->TranslationHandlePoints);
  this->TranslationScaleTransform->SetTransform(vtkNew<vtkTransform>());

  this->AxisRotationGlypher = vtkSmartPointer<vtkTensorGlyph>::New();
  this->AxisRotationGlypher->SetInputConnection(this->RotationScaleTransform->GetOutputPort());
  this->AxisRotationGlypher->SetSourceConnection(this->AxisRotationGlyphSource->GetOutputPort());
  this->AxisRotationGlypher->ScalingOff();
  this->AxisRotationGlypher->ExtractEigenvaluesOff();
  this->AxisRotationGlypher->SetInputArrayToProcess(0, 0, 0, 0, "orientation"); // Orientation direction array

  this->AxisTranslationGlypher = vtkSmartPointer<vtkGlyph3D>::New();
  this->AxisTranslationGlypher->SetInputConnection(this->TranslationScaleTransform->GetOutputPort());
  this->AxisTranslationGlypher->SetSourceConnection(0, this->AxisTranslationGlyphTransformer->GetOutputPort());
  this->AxisTranslationGlypher->SetSourceConnection(1, this->AxisRotationHandleSource->GetOutputPort());
  this->AxisTranslationGlypher->ScalingOn();
  this->AxisTranslationGlypher->SetScaleModeToDataScalingOff();
  this->AxisTranslationGlypher->SetIndexModeToScalar();
  this->AxisTranslationGlypher->SetColorModeToColorByScalar();
  this->AxisTranslationGlypher->OrientOn();
  this->AxisTranslationGlypher->SetInputArrayToProcess(0, 0, 0, 0, "glyphIndex"); // Glyph shape
  this->AxisTranslationGlypher->SetInputArrayToProcess(1, 0, 0, 0, "orientation"); // Orientation direction array

  this->Append = vtkSmartPointer<vtkAppendPolyData>::New();
  this->Append->AddInputConnection(this->AxisRotationGlypher->GetOutputPort());
  this->Append->AddInputConnection(this->AxisTranslationGlypher->GetOutputPort());

  this->HandleToWorldTransform = vtkSmartPointer<vtkTransform>::New();
  this->HandleToWorldTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->HandleToWorldTransformFilter->SetInputConnection(this->Append->GetOutputPort());
  this->HandleToWorldTransformFilter->SetTransform(this->HandleToWorldTransform);

  this->ColorTable = vtkSmartPointer<vtkLookupTable>::New();

  vtkNew<vtkCoordinate> coordinate;
  coordinate->SetCoordinateSystemToWorld();

  this->Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->Mapper->SetInputConnection(this->HandleToWorldTransformFilter->GetOutputPort());
  this->Mapper->SetColorModeToMapScalars();
  this->Mapper->ColorByArrayComponent("colorIndex", 0);
  this->Mapper->SetLookupTable(this->ColorTable);
  this->Mapper->ScalarVisibilityOn();
  this->Mapper->UseLookupTableScalarRangeOn();
  this->Mapper->SetTransformCoordinate(coordinate);

  this->Property = vtkSmartPointer<vtkProperty2D>::New();
  this->Property->SetPointSize(0.0);
  this->Property->SetLineWidth(0.0);

  this->Actor = vtkSmartPointer<vtkActor2D>::New();
  this->Actor->SetProperty(this->Property);
  this->Actor->SetMapper(this->Mapper);

  this->StartFadeAngle = 30;
  this->EndFadeAngle = 20;
}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::~MarkupsInteractionPipeline() = default;

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::InitializePipeline()
{
  this->CreateRotationHandles();
  this->CreateTranslationHandles();
  this->UpdateHandleColors();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::CreateRotationHandles()
{
  vtkNew<vtkPoints> points;

  double xRotationHandle[3] = { 0, 1, 1 }; // X-axis
  vtkMath::Normalize(xRotationHandle);
  points->InsertNextPoint(xRotationHandle);
  double yRotationHandle[3] = { 1, 0, 1 }; // Y-axis
  vtkMath::Normalize(yRotationHandle);
  points->InsertNextPoint(yRotationHandle);
  double zRotationHandle[3] = { 1, 1, 0 }; // Z-axis
  vtkMath::Normalize(zRotationHandle);
  points->InsertNextPoint(zRotationHandle);
  this->RotationHandlePoints->SetPoints(points);

  vtkNew<vtkDoubleArray> orientationArray;
  orientationArray->SetName("orientation");
  orientationArray->SetNumberOfComponents(9);
  vtkNew<vtkTransform> xRotationOrientation;
  xRotationOrientation->RotateX(90);
  xRotationOrientation->RotateY(90);
  xRotationOrientation->RotateZ(45);
  vtkMatrix4x4* xRotationMatrix = xRotationOrientation->GetMatrix();
  orientationArray->InsertNextTuple9(xRotationMatrix->GetElement(0, 0), xRotationMatrix->GetElement(1, 0), xRotationMatrix->GetElement(2, 0),
                                     xRotationMatrix->GetElement(0, 1), xRotationMatrix->GetElement(1, 1), xRotationMatrix->GetElement(2, 1),
                                     xRotationMatrix->GetElement(0, 2), xRotationMatrix->GetElement(1, 2), xRotationMatrix->GetElement(2, 2));
  vtkNew<vtkTransform> yRotationOrientation;
  yRotationOrientation->RotateX(90);
  yRotationOrientation->RotateZ(45);
  vtkMatrix4x4* yRotationMatrix = yRotationOrientation->GetMatrix();
  orientationArray->InsertNextTuple9(yRotationMatrix->GetElement(0, 0), yRotationMatrix->GetElement(1, 0), yRotationMatrix->GetElement(2, 0),
                                     yRotationMatrix->GetElement(0, 1), yRotationMatrix->GetElement(1, 1), yRotationMatrix->GetElement(2, 1),
                                     yRotationMatrix->GetElement(0, 2), yRotationMatrix->GetElement(1, 2), yRotationMatrix->GetElement(2, 2));
  vtkNew<vtkTransform> zRotationOrientation;
  zRotationOrientation->RotateZ(45);
  vtkMatrix4x4* zRotationMatrix = zRotationOrientation->GetMatrix();
  orientationArray->InsertNextTuple9(zRotationMatrix->GetElement(0, 0), zRotationMatrix->GetElement(1, 0), zRotationMatrix->GetElement(2, 0),
                                     zRotationMatrix->GetElement(0, 1), zRotationMatrix->GetElement(1, 1), zRotationMatrix->GetElement(2, 1),
                                     zRotationMatrix->GetElement(0, 2), zRotationMatrix->GetElement(1, 2), zRotationMatrix->GetElement(2, 2));
  this->RotationHandlePoints->GetPointData()->AddArray(orientationArray);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::CreateTranslationHandles()
{
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(1, 0, 0); // X-axis
  points->InsertNextPoint(0, 1, 0); // Y-axis
  points->InsertNextPoint(0, 0, 1); // Z-axis
  points->InsertNextPoint(0, 0, 0); // Free translation
  this->TranslationHandlePoints->SetPoints(points);

  vtkNew<vtkDoubleArray> orientationArray;
  orientationArray->SetName("orientation");
  orientationArray->SetNumberOfComponents(3);
  orientationArray->InsertNextTuple3(1, 0, 0);
  orientationArray->InsertNextTuple3(0, 1, 0);
  orientationArray->InsertNextTuple3(0, 0, 1);
  orientationArray->InsertNextTuple3(1, 0, 0); // Free translation
  this->TranslationHandlePoints->GetPointData()->AddArray(orientationArray);

  vtkNew<vtkDoubleArray> glyphIndexArray;
  glyphIndexArray->SetName("glyphIndex");
  glyphIndexArray->SetNumberOfComponents(1);
  glyphIndexArray->InsertNextTuple1(0);
  glyphIndexArray->InsertNextTuple1(0);
  glyphIndexArray->InsertNextTuple1(0);
  glyphIndexArray->InsertNextTuple1(1);
  this->TranslationHandlePoints->GetPointData()->AddArray(glyphIndexArray);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::UpdateHandleColors()
{
  if (!this->ColorTable)
    {
    return;
    }

  int numberOfHandles = this->RotationHandlePoints->GetNumberOfPoints() + this->TranslationHandlePoints->GetNumberOfPoints();
  this->ColorTable->SetNumberOfTableValues(numberOfHandles);
  this->ColorTable->SetTableRange(0, numberOfHandles - 1);

  int colorIndex = 0;
  double color[4] = { 0 };

  // Rotation handles
  vtkSmartPointer<vtkFloatArray> rotationColorArray = vtkFloatArray::SafeDownCast(
    this->RotationHandlePoints->GetPointData()->GetAbstractArray("colorIndex"));
  if (!rotationColorArray)
    {
    rotationColorArray = vtkSmartPointer<vtkFloatArray>::New();
    rotationColorArray->SetName("colorIndex");
    rotationColorArray->SetNumberOfComponents(1);
    this->RotationHandlePoints->GetPointData()->AddArray(rotationColorArray);
    this->RotationHandlePoints->GetPointData()->SetActiveScalars("colorIndex");
    }
  rotationColorArray->Initialize();
  rotationColorArray->SetNumberOfTuples(this->RotationHandlePoints->GetNumberOfPoints());
  for (int i = 0; i < this->RotationHandlePoints->GetNumberOfPoints(); ++i)
    {
    this->GetHandleColor(vtkMRMLMarkupsDisplayNode::ComponentRotationHandle, i, color);
    this->ColorTable->SetTableValue(colorIndex, color);
    rotationColorArray->SetTuple1(i, colorIndex);
    ++colorIndex;
    }

  // Translation handles
  vtkSmartPointer<vtkFloatArray> translationColorArray = vtkFloatArray::SafeDownCast(
    this->TranslationHandlePoints->GetPointData()->GetAbstractArray("colorIndex"));
  if (!translationColorArray)
    {
    translationColorArray = vtkSmartPointer<vtkFloatArray>::New();
    translationColorArray->SetName("colorIndex");
    translationColorArray->SetNumberOfComponents(1);
    this->TranslationHandlePoints->GetPointData()->AddArray(translationColorArray);
    this->TranslationHandlePoints->GetPointData()->SetActiveScalars("colorIndex");
    }
  translationColorArray->Initialize();
  translationColorArray->SetNumberOfTuples(this->TranslationHandlePoints->GetNumberOfPoints());
  for (int i = 0; i < this->TranslationHandlePoints->GetNumberOfPoints(); ++i)
    {
    this->GetHandleColor(vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle, i, color);
    this->ColorTable->SetTableValue(colorIndex, color);
    translationColorArray->SetTuple1(i, colorIndex);
    ++colorIndex;
    }

  this->ColorTable->Build();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::GetHandleColor(int type, int index, double color[4])
{
  if (!color)
    {
    return;
    }

  double red[4]    = { 1.00, 0.00, 0.00, 1.00 };
  double green[4]  = { 0.00, 1.00, 0.00, 1.00 };
  double blue[4]   = { 0.00, 0.00, 1.00, 1.00 };
  double orange[4] = { 1.00, 0.50, 0.00, 1.00 };
  double white[4]  = { 1.00, 1.00, 1.00, 1.00 };
  double yellow[4] = { 1.00, 1.00, 0.00, 1.00 };

  double* currentColor = red;
  switch (index)
    {
    case 0:
      currentColor = red;
      break;
    case 1:
      currentColor = green;
      break;
    case 2:
      currentColor = blue;
      break;
    case 3:
      currentColor = orange;
      break;
    default:
      currentColor = white;
      break;
    }

  bool highlighted = false;
  // Highlighted
  vtkSlicerMarkupsWidgetRepresentation* markupsRepresentation = vtkSlicerMarkupsWidgetRepresentation::SafeDownCast(this->Representation);
  vtkMRMLMarkupsDisplayNode* displayNode = nullptr;
  if (markupsRepresentation)
    {
    displayNode = markupsRepresentation->GetMarkupsDisplayNode();
    }
  if (displayNode && displayNode->GetActiveComponentType() == type && displayNode->GetActiveComponentIndex() == index)
    {
    highlighted = true;
    currentColor = yellow;
    }

  for (int i = 0; i < 3; ++i)
    {
    color[i] = currentColor[i];
    }

  double opacity = 1.0;
  if (!highlighted)
    {
    opacity = this->GetOpacity(type, index);
    }
  color[3] = opacity;
}

//----------------------------------------------------------------------
double vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::GetOpacity(int type, int index)
{
  double viewNormal[3] = { 0 };
  this->GetViewPlaneNormal(viewNormal);

  double opacity = 1.0;
  if (type == vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle && index == 3)
    {
    // Free transform handle
    return opacity;
    }

  double axis[3] = { 0 };
  this->GetInteractionHandleAxisWorld(index, axis);
  if (vtkMath::Dot(viewNormal, axis) < 0)
    {
    vtkMath::MultiplyScalar(axis, -1);
    }

  double fadeAngleRange = this->StartFadeAngle - this->EndFadeAngle;
  double angle = vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(viewNormal, axis));
  if (type == vtkMRMLMarkupsDisplayNode::ComponentRotationHandle)
    {
    // Fade for rotation handles happens when the rotation axis is approaching 90 degrees from the view normal
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
  else if (type == vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle)
    {
    // Fade for translation handles happens when the rotation axis is approaching 0 degrees from the view normal
    if (angle < this->EndFadeAngle)
      {
      opacity = 0.0;
      }
    else if (angle < this->StartFadeAngle)
      {
      double difference = angle - this->EndFadeAngle;
      opacity = (difference / fadeAngleRange);
      }
    }
  return opacity;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::GetViewPlaneNormal(double normal[3])
{
  if (!normal)
    {
    return;
    }
  if (this->Representation && this->Representation->GetRenderer() && this->Representation->GetRenderer()->GetActiveCamera())
    {
    vtkCamera* camera = this->Representation->GetRenderer()->GetActiveCamera();
    camera->GetViewPlaneNormal(normal);
    }
}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation::HandleInfoList vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::GetHandleInfoList()
{
  vtkSlicerMarkupsWidgetRepresentation::HandleInfoList handleInfoList;
  for (int i = 0; i < this->RotationHandlePoints->GetNumberOfPoints(); ++i)
    {
    double handlePositionLocal[3] = { 0 };
    double handlePositionWorld[3] = { 0 };
    this->RotationHandlePoints->GetPoint(i, handlePositionLocal);
    this->RotationScaleTransform->GetTransform()->TransformPoint(handlePositionLocal, handlePositionWorld);
    this->HandleToWorldTransform->TransformPoint(handlePositionWorld, handlePositionWorld);
    double color[4] = { 0 };
    this->GetHandleColor(vtkMRMLMarkupsDisplayNode::ComponentRotationHandle, i, color);
    HandleInfo info(i, vtkMRMLMarkupsDisplayNode::ComponentRotationHandle, handlePositionWorld, handlePositionLocal, color);
    handleInfoList.push_back(info);
    }

  for (int i = 0; i < this->TranslationHandlePoints->GetNumberOfPoints(); ++i)
    {
    double handlePositionLocal[3] = { 0 };
    double handlePositionWorld[3] = { 0 };
    this->TranslationHandlePoints->GetPoint(i, handlePositionLocal);
    this->TranslationScaleTransform->GetTransform()->TransformPoint(handlePositionLocal, handlePositionWorld);
    this->HandleToWorldTransform->TransformPoint(handlePositionWorld, handlePositionWorld);
    double color[4] = { 0 };
    this->GetHandleColor(vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle, i, color);
    HandleInfo info(i, vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle, handlePositionWorld, handlePositionLocal, color);
    handleInfoList.push_back(info);
    }
  return handleInfoList;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::SetWidgetScale(double scale)
{
  vtkNew<vtkTransform> scaleTransform;
  scaleTransform->Scale(scale, scale, scale);
  this->RotationScaleTransform->SetTransform(scaleTransform);
  this->TranslationScaleTransform->SetTransform(scaleTransform);
  this->AxisRotationGlypher->SetScaleFactor(scale);
  this->AxisTranslationGlypher->SetScaleFactor(scale);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::GetInteractionHandleAxisWorld(int index, double axisWorld[3])
{
  if (!axisWorld || index < 0 || index > 2)
    {
    return;
    }

  double handleAxis[3] = { 0 };
  handleAxis[index] = 1;
  double origin[3] = { 0,0,0 };
  this->HandleToWorldTransform->TransformVectorAtPoint(origin, handleAxis, axisWorld);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::GetInteractionHandleOriginWorld(double originWorld[3])
{
  if (!originWorld)
    {
    return;
    }

  double handleOrigin[3] = { 0,0,0 };
  this->HandleToWorldTransform->TransformPoint(handleOrigin, originWorld);
}

int vtkSlicerMarkupsWidgetRepresentation::GetGlyphTypeSourceFromDisplay(int glyphTypeDisplay)
{
  switch (glyphTypeDisplay)
    {
    case vtkMRMLMarkupsDisplayNode::GlyphTypeInvalid: return vtkMarkupsGlyphSource2D::GlyphNone;
    case vtkMRMLMarkupsDisplayNode::StarBurst2D: return vtkMarkupsGlyphSource2D::GlyphStarBurst;
    case vtkMRMLMarkupsDisplayNode::Cross2D: return vtkMarkupsGlyphSource2D::GlyphCross;
    case vtkMRMLMarkupsDisplayNode::CrossDot2D: return vtkMarkupsGlyphSource2D::GlyphCrossDot;
    case vtkMRMLMarkupsDisplayNode::ThickCross2D: return vtkMarkupsGlyphSource2D::GlyphThickCross;
    case vtkMRMLMarkupsDisplayNode::Dash2D: return vtkMarkupsGlyphSource2D::GlyphDash;
    case vtkMRMLMarkupsDisplayNode::Sphere3D: return vtkMarkupsGlyphSource2D::GlyphCircle;
    case vtkMRMLMarkupsDisplayNode::Vertex2D: return vtkMarkupsGlyphSource2D::GlyphVertex;
    case vtkMRMLMarkupsDisplayNode::Circle2D: return vtkMarkupsGlyphSource2D::GlyphCircle;
    case vtkMRMLMarkupsDisplayNode::Triangle2D: return vtkMarkupsGlyphSource2D::GlyphTriangle;
    case vtkMRMLMarkupsDisplayNode::Square2D: return vtkMarkupsGlyphSource2D::GlyphSquare;
    case vtkMRMLMarkupsDisplayNode::Diamond2D: return vtkMarkupsGlyphSource2D::GlyphDiamond;
    case vtkMRMLMarkupsDisplayNode::Arrow2D: return vtkMarkupsGlyphSource2D::GlyphArrow;
    case vtkMRMLMarkupsDisplayNode::ThickArrow2D: return vtkMarkupsGlyphSource2D::GlyphThickArrow;
    case vtkMRMLMarkupsDisplayNode::HookedArrow2D: return vtkMarkupsGlyphSource2D::GlyphHookedArrow;
    default:
      return -1;
    }
}
