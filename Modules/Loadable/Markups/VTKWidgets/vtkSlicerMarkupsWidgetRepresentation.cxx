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
  this->CurveClosed = 0;

  this->TextActor = vtkSmartPointer<vtkTextActor>::New();
  // hide by default, if a concrete class implements properties display, it will enable it
  this->TextActor->SetVisibility(false);

  this->PointPlacer = vtkSmartPointer<vtkFocalPlanePointPlacer>::New();

  for (int i = 0; i < NumberOfControlPointTypes; i++)
  {
    this->ControlPoints[i] = nullptr;
  }

  this->AlwaysOnTop = false;
}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation::~vtkSlicerMarkupsWidgetRepresentation()
{
  for (int i = 0; i < NumberOfControlPointTypes; i++)
  {
    delete this->ControlPoints[i];
    this->ControlPoints[i] = nullptr;
  }
  // Force deleting variables to prevent circular dependency keeping objects alive
  this->PointPlacer = nullptr;
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
int vtkSlicerMarkupsWidgetRepresentation::FindClosestPointOnWidget(const int displayPos[2],
                                                                   double closestWorldPos[3],
                                                                   int* idx)
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
  double p3[4] = { 0.0, 0.0, 0.0, 1.0 };
  double p4[4] = { 0.0, 0.0, 0.0, 1.0 };
  vtkPoints* curvePointsWorld = this->GetMarkupsNode()->GetCurvePointsWorld();
  vtkIdType numberOfPoints = curvePointsWorld->GetNumberOfPoints();
  for (vtkIdType i = 0; i < numberOfPoints; i++)
  {
    curvePointsWorld->GetPoint(i, p3);
    if (i + 1 < numberOfPoints)
    {
      curvePointsWorld->GetPoint(i + 1, p4);
    }
    else
    {
      if (!this->CurveClosed)
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
      p5[0] = p1[0] + u * (p2[0] - p1[0]);
      p5[1] = p1[1] + u * (p2[1] - p1[1]);
      p5[2] = p1[2] + u * (p2[2] - p1[2]);

      p6[0] = p3[0] + v * (p4[0] - p3[0]);
      p6[1] = p3[1] + v * (p4[1] - p3[1]);
      p6[2] = p3[2] + v * (p4[2] - p3[2]);

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
    if (closestNode < markupsNode->GetNumberOfControlPoints() - 1)
    {
      *idx = closestNode + 1;
      return 1;
    }
    else if (this->CurveClosed)
    {
      *idx = 0;
      return 1;
    }
  }

  return 0;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::UpdateCenterOfRotation()
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
  double inv_N = 1. / static_cast<double>(markupsNode->GetNumberOfControlPoints());
  centerWorldPos[0] *= inv_N;
  centerWorldPos[1] *= inv_N;
  centerWorldPos[2] *= inv_N;

  markupsNode->SetCenterOfRotationWorld(centerWorldPos);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::SetMarkupsDisplayNode(vtkMRMLMarkupsDisplayNode* markupsDisplayNode)
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
vtkMRMLMarkupsDisplayNode* vtkSlicerMarkupsWidgetRepresentation::GetMarkupsDisplayNode()
{
  return this->MarkupsDisplayNode;
}

//----------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkSlicerMarkupsWidgetRepresentation::GetMarkupsNode()
{
  if (!this->MarkupsDisplayNode)
  {
    return nullptr;
  }
  return vtkMRMLMarkupsNode::SafeDownCast(this->MarkupsDisplayNode->GetDisplayableNode());
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::SetMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
{
  this->MarkupsNode = markupsNode;
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  // Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Point Placer: " << this->PointPlacer << "\n";
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::CanInteract(vtkMRMLInteractionEventData* vtkNotUsed(interactionEventData),
                                                       int& foundComponentType,
                                                       int& vtkNotUsed(foundComponentIndex),
                                                       double& vtkNotUsed(closestDistance2))
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
  this->UpdateCenterOfRotation();
  markupsNode->GetCenterOfRotationWorld(referencePointWorld);
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
  line->InsertNextCell(numberOfLines + 1);

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
void vtkSlicerMarkupsWidgetRepresentation::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void* callData)
{
  this->UpdateFromMRMLInternal(caller, event, callData);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::UpdateFromMRMLInternal(vtkMRMLNode* vtkNotUsed(caller),
                                                                  unsigned long event,
                                                                  void* vtkNotUsed(callData))
{
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
    std::string labelText;
    if (this->MarkupsNode->GetName())
    {
      labelText = this->MarkupsNode->GetName();
    }
    std::string properties = this->MarkupsNode->GetPropertiesLabelText();
    if (!properties.empty())
    {
      labelText += ":" + properties;
    }
    this->TextActor->SetInput(labelText.c_str());
  }
  else
  {
    this->TextActor->SetInput("");
  }

  this->NeedToRenderOn(); // TODO: to improve performance, call this only if it is actually needed
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
    if (!(markupsNode->GetNthControlPointPositionVisibility(controlPointIndex)
          && (markupsNode->GetNthControlPointVisibility(controlPointIndex))))
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
      if (controlPointType == Active)
      {
        this->MarkupsDisplayNode->GetActiveColor(color);
        return color;
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
    case Active:
      this->MarkupsDisplayNode->GetActiveColor(color);
      break;
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
    vtkErrorMacro("vtkSlicerMarkupsWidgetRepresentation::GetControlPointsPolyData failed: invalid controlPointType: "
                  << controlPointType);
    return nullptr;
  }
  return this->ControlPoints[controlPointType]->ControlPointsPolyData;
}

//----------------------------------------------------------------------
vtkPolyData* vtkSlicerMarkupsWidgetRepresentation::GetLabelControlPointsPolyData(int controlPointType)
{
  if (controlPointType < 0 || controlPointType >= NumberOfControlPointTypes)
  {
    vtkErrorMacro(
      "vtkSlicerMarkupsWidgetRepresentation::GetLabelControlPointsPolyData failed: invalid controlPointType: "
      << controlPointType);
    return nullptr;
  }
  return this->ControlPoints[controlPointType]->LabelControlPointsPolyData;
}

//----------------------------------------------------------------------
vtkStringArray* vtkSlicerMarkupsWidgetRepresentation::GetLabels(int controlPointType)
{
  if (controlPointType < 0 || controlPointType >= NumberOfControlPointTypes)
  {
    vtkErrorMacro("vtkSlicerMarkupsWidgetRepresentation::GetControlPointsPolyData failed: invalid controlPointType: "
                  << controlPointType);
    return nullptr;
  }
  return this->ControlPoints[controlPointType]->Labels;
}

//-----------------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation::IsDisplayable()
{
  if (!this->MarkupsDisplayNode || !this->ViewNode || !this->MarkupsDisplayNode->GetVisibility()
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
void vtkSlicerMarkupsWidgetRepresentation::GetActors(vtkPropCollection* vtkNotUsed(pc)) {}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation::ReleaseGraphicsResources(vtkWindow* vtkNotUsed(window)) {}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation::RenderOverlay(vtkViewport* vtkNotUsed(viewport))
{
  return 0;
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation::RenderOpaqueGeometry(vtkViewport* vtkNotUsed(viewport))
{
  return 0;
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* vtkNotUsed(viewport))
{
  return 0;
}

//----------------------------------------------------------------------
vtkTypeBool vtkSlicerMarkupsWidgetRepresentation::HasTranslucentPolygonalGeometry()
{
  return false;
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation::GetGlyphTypeSourceFromDisplay(int glyphTypeDisplay)
{
  switch (glyphTypeDisplay)
  {
    case vtkMRMLMarkupsDisplayNode::GlyphTypeInvalid:
      return vtkMarkupsGlyphSource2D::GlyphNone;
    case vtkMRMLMarkupsDisplayNode::StarBurst2D:
      return vtkMarkupsGlyphSource2D::GlyphStarBurst;
    case vtkMRMLMarkupsDisplayNode::Cross2D:
      return vtkMarkupsGlyphSource2D::GlyphCross;
    case vtkMRMLMarkupsDisplayNode::CrossDot2D:
      return vtkMarkupsGlyphSource2D::GlyphCrossDot;
    case vtkMRMLMarkupsDisplayNode::ThickCross2D:
      return vtkMarkupsGlyphSource2D::GlyphThickCross;
    case vtkMRMLMarkupsDisplayNode::Dash2D:
      return vtkMarkupsGlyphSource2D::GlyphDash;
    case vtkMRMLMarkupsDisplayNode::Sphere3D:
      return vtkMarkupsGlyphSource2D::GlyphCircle;
    case vtkMRMLMarkupsDisplayNode::Vertex2D:
      return vtkMarkupsGlyphSource2D::GlyphVertex;
    case vtkMRMLMarkupsDisplayNode::Circle2D:
      return vtkMarkupsGlyphSource2D::GlyphCircle;
    case vtkMRMLMarkupsDisplayNode::Triangle2D:
      return vtkMarkupsGlyphSource2D::GlyphTriangle;
    case vtkMRMLMarkupsDisplayNode::Square2D:
      return vtkMarkupsGlyphSource2D::GlyphSquare;
    case vtkMRMLMarkupsDisplayNode::Diamond2D:
      return vtkMarkupsGlyphSource2D::GlyphDiamond;
    case vtkMRMLMarkupsDisplayNode::Arrow2D:
      return vtkMarkupsGlyphSource2D::GlyphArrow;
    case vtkMRMLMarkupsDisplayNode::ThickArrow2D:
      return vtkMarkupsGlyphSource2D::GlyphThickArrow;
    case vtkMRMLMarkupsDisplayNode::HookedArrow2D:
      return vtkMarkupsGlyphSource2D::GlyphHookedArrow;
    default:
      return -1;
  }
}
