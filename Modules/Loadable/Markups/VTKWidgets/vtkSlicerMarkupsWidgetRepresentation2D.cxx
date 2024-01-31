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
#include "vtkCamera.h"
#include "vtkCellLocator.h"
#include "vtkDiscretizableColorTransferFunction.h"
#include "vtkGlyph2D.h"
#include "vtkLabelPlacementMapper.h"
#include "vtkLine.h"
#include "vtkLineSource.h"
#include "vtkMarkupsGlyphSource2D.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPlane.h"
#include "vtkPointData.h"
#include "vtkPointSetToLabelHierarchy.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSlicerMarkupsWidgetRepresentation2D.h"
#include "vtkSphereSource.h"
#include "vtkStringArray.h"
#include "vtkTensorGlyph.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"

// MRML includes
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLInteractionEventData.h>

vtkSlicerMarkupsWidgetRepresentation2D::ControlPointsPipeline2D::ControlPointsPipeline2D()
{
  this->Glypher = vtkSmartPointer<vtkGlyph2D>::New();
  this->Glypher->SetInputData(this->ControlPointsPolyData);
  this->Glypher->SetScaleFactor(1.0);

  // By default the Points are rendered as spheres
  this->Glypher->SetSourceConnection(this->GlyphSourceSphere->GetOutputPort());

  this->Property = vtkSmartPointer<vtkProperty2D>::New();
  this->Property->SetColor(0.4, 1.0, 1.0);
  this->Property->SetPointSize(3.);
  this->Property->SetLineWidth(3.);
  this->Property->SetOpacity(1.);

  this->Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->Mapper->SetInputConnection(this->Glypher->GetOutputPort());
  this->Mapper->ScalarVisibilityOff();
  vtkNew<vtkCoordinate> coordinate;
  //coordinate->SetViewport(this->Renderer); TODO: check this
  // World coordinates of the node can not be used to build the actors of the representation
  // Because the world coordinate in the node are the 3D ones.
  coordinate->SetCoordinateSystemToDisplay();
  this->Mapper->SetTransformCoordinate(coordinate);

  this->Actor = vtkSmartPointer<vtkActor2D>::New();
  this->Actor->SetMapper(this->Mapper);
  this->Actor->SetProperty(this->Property);

  // Labels
  this->LabelsMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();

  // PointSetToLabelHierarchyFilter perturbs (slightly modifies) label positions around
  // coincident points, but the result is far from optimal (labels are quite far from
  // the control point and not at the same distance).
  // We set maximum depth from the default 5 to 10 to essentially eliminate
  // this coincident label position perturbation (perturbation may still be visible
  // when zooming into the view very much). Probably it would be cleaner to have on option
  // to disable this label position adjustment in PointSetToLabelHierarchyFilter.
  this->PointSetToLabelHierarchyFilter->SetMaximumDepth(10);

  this->LabelsMapper->PlaceAllLabelsOn();

  this->LabelsMapper->SetInputConnection(this->PointSetToLabelHierarchyFilter->GetOutputPort());
  // Here it will be the best to use Display coordinate system
  // in that way we would not need the additional copy of the polydata in LabelFocalData (in Viewport coordinates)
  // However the LabelsMapper seems not working with the Display coordinates.
  this->LabelsMapper->GetAnchorTransform()->SetCoordinateSystemToNormalizedViewport();
  this->LabelsActor = vtkSmartPointer<vtkActor2D>::New();
  this->LabelsActor->SetMapper(this->LabelsMapper);

  // Labels line polydata
  this->LabelsLinePolyData = vtkSmartPointer<vtkPolyData>::New();

  this->LabelsLineProperty = vtkSmartPointer<vtkProperty2D>::New();
  //this->LabelsLineProperty->SetColor(0.4, 1.0, 1.0);
  //this->LabelsLineProperty->SetPointSize(3.);
  this->LabelsLineProperty->SetOpacity(1.);
  this->LabelsLineProperty->SetLineWidth(this->LabelsLineProperty->GetLineWidth());
  this->LabelsLineMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  this->LabelsLineMapper->SetInputData(this->LabelsLinePolyData);

  this->LabelsLineMapper->SetScalarVisibility(true);

  this->LabelsLineActor = vtkSmartPointer<vtkActor2D>::New();
  this->LabelsLineActor->SetMapper(this->LabelsLineMapper);
  this->LabelsLineActor->SetProperty(this->LabelsLineProperty);
}

vtkSlicerMarkupsWidgetRepresentation2D::ControlPointsPipeline2D::~ControlPointsPipeline2D() = default;

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation2D::vtkSlicerMarkupsWidgetRepresentation2D()
{
  for (int i = 0; i<NumberOfControlPointTypes; i++)
    {
    this->ControlPoints[i] = new ControlPointsPipeline2D;
    }

  this->ControlPoints[Selected]->TextProperty->SetColor(1.0, 0.5, 0.5);
  reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[Selected])->Property->SetColor(1.0, 0.5, 0.5);

  this->ControlPoints[Active]->TextProperty->SetColor(0.4, 1.0, 0.); // bright green
  reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[Active])->Property->SetColor(0.4, 1.0, 0.);

  this->TextActor->SetTextProperty(this->GetControlPointsPipeline(Unselected)->TextProperty);

  this->PointsVisibilityOnSlice = vtkSmartPointer<vtkIntArray>::New();
  this->PointsVisibilityOnSlice->SetName("pointsVisibilityOnSlice");
  this->PointsVisibilityOnSlice->Allocate(100);
  this->PointsVisibilityOnSlice->SetNumberOfValues(1);
  this->PointsVisibilityOnSlice->SetValue(0,0);

  this->SlicePlane = vtkSmartPointer<vtkPlane>::New();
  this->WorldToSliceTransform = vtkSmartPointer<vtkTransform>::New();
}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation2D::~vtkSlicerMarkupsWidgetRepresentation2D() = default;

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::GetSliceToWorldCoordinates(const double slicePos[2],
                                                                        double worldPos[3])
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!this->Renderer || !sliceNode)
    {
    return;
    }

  double xyzw[4] =
  {
    slicePos[0] - this->Renderer->GetOrigin()[0],
    slicePos[1] - this->Renderer->GetOrigin()[1],
    0.0,
    1.0
  };
  double rasw[4] = { 0.0, 0.0, 0.0, 1.0 };

  this->GetSliceNode()->GetXYToRAS()->MultiplyPoint(xyzw, rasw);

  worldPos[0] = rasw[0]/rasw[3];
  worldPos[1] = rasw[1]/rasw[3];
  worldPos[2] = rasw[2]/rasw[3];
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::GetBackgroundVolumeXYCenter(double centerPos[2])
{
  if (!this->GetSliceNode())
    {
    return;
    }

  // TODO: get RAS bounds of the scene from the view node
  double centerPosRAS[4] = { 0.0, 0.0, 0.0, 1.0 };

  double centerPosDisplay[4] = { 0.0, 0.0, 0.0, 1.0 };
  this->GetWorldToSliceCoordinates(centerPosRAS, centerPosDisplay);

  centerPos[0] = centerPosDisplay[0];
  centerPos[1] = centerPosDisplay[1];
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::GetWorldToSliceCoordinates(const double worldPos[3], double slicePos[2])
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return;
    }

  double sliceCoordinates[4], worldCoordinates[4];
  worldCoordinates[0] = worldPos[0];
  worldCoordinates[1] = worldPos[1];
  worldCoordinates[2] = worldPos[2];
  worldCoordinates[3] = 1.0;

  vtkNew<vtkMatrix4x4> rasToxyMatrix;
  sliceNode->GetXYToRAS()->Invert(sliceNode->GetXYToRAS(), rasToxyMatrix.GetPointer());

  rasToxyMatrix->MultiplyPoint(worldCoordinates, sliceCoordinates);

  slicePos[0] = sliceCoordinates[0];
  slicePos[1] = sliceCoordinates[1];
}


//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::GetLabelTextXYOffsets(double backgroundVolumeCenterXY[2], double  slicePos[2], double leadersLineOffset[2])
{
    double pi = 3.14159;

    class myTriangle
    {
    public:
        double a = 0;
        double b = 0;
        double c = 0;
        double AlphaRad = 0;
        double BetaRad = 0;
        double GammaRad = 0;
        double AlphaDeg = 0;
        double BetaDeg = 0;
        double GammaDeg = 0;
    };

    const int* screenSize = this->Renderer->GetRenderWindow()->GetSize();
    int screenWidth = screenSize[0];
    int screenHeight = screenSize[1];

    double sliceXCenterOffsAbs = abs(slicePos[0] - backgroundVolumeCenterXY[0]);
    double sliceYCenterOffsAbs = abs(slicePos[1] - backgroundVolumeCenterXY[1]);

    // calculating a triangle between control point and displaycenter
    // known are two sides of the triangle
    // a (x-offset between center and ctrl point pos)
    // c (y-offset between center and ctrl point pos)
    // and the 90 degree angle (beta)

    myTriangle tr;

    tr.a = sliceXCenterOffsAbs;
    tr.c = sliceYCenterOffsAbs;
    tr.BetaDeg = 90.;
    tr.BetaRad = (tr.BetaDeg * pi) / 180.;

    tr.b = sqrt(pow(tr.a, 2.) + pow(tr.c, 2.) - (2. * tr.a * tr.c * cos(tr.BetaRad)));
    tr.AlphaRad = acos((pow(tr.b, 2.) + pow(tr.c, 2.) - pow(tr.a, 2.)) / (2 * tr.b * tr.c));
    tr.GammaRad = acos((pow(tr.b, 2.) + pow(tr.a, 2.) - pow(tr.c, 2.)) / (2 * tr.b * tr.a));
    tr.AlphaDeg = tr.AlphaRad * (180 / pi);
    tr.GammaDeg = tr.GammaRad * (180 / pi);


    double screenDiagonal = sqrt(pow(screenWidth, 2.) + pow(screenHeight, 2.));
    double leaderLinesLength = (this->MarkupsDisplayNode->GetPointLabelsDistanceScale() * screenDiagonal) / 100.;

    // calculating a triangle between control point and labeltext
    // known are two angles of the triangle (alpha,gamma, see above)
    // and one side (b, leader line length) )

    myTriangle tr2;

    tr2.b = leaderLinesLength;
    tr2.GammaDeg = tr.GammaDeg;
    tr2.GammaRad = (tr2.GammaDeg * pi) / 180.;
    tr2.BetaDeg = 90.;
    tr2.BetaRad = (tr2.BetaDeg * pi) / 180.;


    tr2.AlphaDeg = 180. - tr2.GammaDeg - tr2.BetaDeg;
    tr2.AlphaRad = (tr2.AlphaDeg * pi) / 180.;
    tr2.a = tr2.b * sin(tr2.AlphaRad) / tr2.BetaRad;
    tr2.c = tr2.b * sin(tr2.GammaRad) / tr2.BetaRad;

    leadersLineOffset[0] = tr2.a;
    leadersLineOffset[1] = tr2.c;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::UpdateAllPointsAndLabelsFromMRML(double labelsOffset)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!this->ViewNode || !markupsNode || !this->MarkupsDisplayNode || !this->Renderer)
    {
    return;
    }

  // Use first active control point for jumping //TODO: Have an 'even more active' point concept
  std::vector<int> activeControlPointIndices;
  this->MarkupsDisplayNode->GetActiveControlPoints(activeControlPointIndices);
  int activeControlPointIndex = -1;
  if (!activeControlPointIndices.empty())
    {
    activeControlPointIndex = activeControlPointIndices[0];
    }

  vtkNew<vtkPoints> pts;
  vtkNew<vtkCellArray> line;
  int ptcnt = 0;

  int numPoints = markupsNode->GetNumberOfControlPoints();

  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
    {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[controlPointType]);

    controlPoints->ControlPoints->Reset();
    controlPoints->ControlPointsPolyData->GetPointData()->GetNormals()->Reset();

    controlPoints->LabelControlPoints->Reset();
    controlPoints->LabelControlPointsPolyData->GetPointData()->GetNormals()->Reset();
    controlPoints->Labels->Reset();
    controlPoints->LabelsPriority->Reset();

    int startIndex = 0;
    int stopIndex = numPoints - 1;
    if (controlPointType == Active)
      {
      if (activeControlPointIndex >= 0 && activeControlPointIndex < numPoints &&
          markupsNode->GetNthControlPointPositionVisibility(activeControlPointIndex) &&
          markupsNode->GetNthControlPointPositionVisibility(activeControlPointIndex) &&
          ((this->PointsVisibilityOnSlice->GetValue(activeControlPointIndex) &&
           !this->MarkupsDisplayNode->GetSliceProjection()) ||
            this->MarkupsDisplayNode->GetSliceProjection())  )
        {
        startIndex = activeControlPointIndex;
        stopIndex = startIndex;
        }
      else
        {
        controlPoints->Actor->VisibilityOff();
        controlPoints->LabelsActor->VisibilityOff();
        controlPoints->LabelsLineActor->VisibilityOff();
        continue;
        }
      }

    if (controlPointType >= Project &&
        !this->MarkupsDisplayNode->GetSliceProjection())
      {
      continue;
      }

    if (controlPointType == ProjectBack &&
        !this->MarkupsDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane())
      {
      continue;
      }

    for (int pointIndex = startIndex; pointIndex <= stopIndex; pointIndex++)
      {
      if (!(markupsNode->GetNthControlPointPositionVisibility(pointIndex)
        && markupsNode->GetNthControlPointVisibility(pointIndex)) ||
          (controlPointType < Active &&
           !this->PointsVisibilityOnSlice->GetValue(pointIndex)) ||
          (controlPointType > Active &&
           this->PointsVisibilityOnSlice->GetValue(pointIndex)))
        {
        continue;
        }
      if (controlPointType == Project &&
          this->MarkupsDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane() &&
          !this->IsPointInFrontSlice(markupsNode, pointIndex))
        {
        continue;
        }
      if (controlPointType == ProjectBack &&
          this->MarkupsDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane() &&
          !this->IsPointBehindSlice(markupsNode, pointIndex))
        {
        continue;
        }
      if (controlPointType < Active)
        {
        bool thisNodeSelected = (markupsNode->GetNthControlPointSelected(pointIndex) != 0);
        if((controlPointType == Selected) != thisNodeSelected)
          {
          continue;
          }
        if (pointIndex == activeControlPointIndex)
          {
          continue;
          }
        }
      if (controlPointType > Active && pointIndex == activeControlPointIndex)
        {
        continue;
        }

      // get control point position
      double slicePos[3] = { 0.0 };
      this->GetNthControlPointDisplayPosition(pointIndex, slicePos);
      controlPoints->ControlPoints->InsertNextPoint(slicePos);

      // set normal label position
      double labelPos[3] = { 0.0 };
      labelPos[0] = slicePos[0] + labelsOffset / sqrt(2.0);
      labelPos[1] = slicePos[1] + labelsOffset / sqrt(2.0);

      std::string labelStr;
      int labelLength = 0;

      labelStr = markupsNode->GetNthControlPointLabel(pointIndex);

      // make label texts multiline if <br> is used
      std::string from = "<br>";
      std::string to = "\n";
      int start_pos = 0;
      while ((start_pos = labelStr.find(from, start_pos)) != std::string::npos)
        {
        labelStr.replace(start_pos, from.length(), to);
        start_pos += to.length();
        }

      // find length of longest string segment after splitting up into multiline segments
      std::string s = labelStr;
      std::string delimiter = "\n";
      int longestStringLen = 0;
      int numberLines = 1;

      size_t pos = 0;
      std::string token;
      while ((pos = s.find(delimiter)) != std::string::npos)
        {
        numberLines++;
        token = s.substr(0, pos);
        if (token.length() > longestStringLen)
          {
          longestStringLen = token.length();
          }
        s.erase(0, pos + delimiter.length());
        }
      if (s.length() > longestStringLen)
        {
        longestStringLen = s.length();
        }

      if (this->MarkupsDisplayNode->GetPointLabelsDistanceScale() > 0.0 &&
        markupsNode->GetNthControlPointPositionVisibility(pointIndex) &&
          this->MarkupsDisplayNode->GetPointLabelsVisibility() &&
                !markupsNode->GetNthControlPointLabel(pointIndex).empty()) // add not occluded
        {
        double lineStartPos[3] = { 0.0 };
        double lineEndPos[3] = { 0.0 };
        const int* screenSize = this->Renderer->GetRenderWindow()->GetSize();
        int screenWidth = screenSize[0];
        int screenHeight = screenSize[1];
        labelLength = longestStringLen;

        // Not working correctly - giving the exact same values in _bb  wherever you place this code
        //double _bb[4] = { 0.0 };
        //this->TextActor->GetBoundingBox(this->Renderer, _bb);
        //this->TextActor->SetTextScaleModeToViewport();
        double labelCharWidth =  this->MarkupsDisplayNode->GetTextScale() * 3.;
        double labelTextWidth = labelCharWidth * labelLength;
        double labelTextHeight = (this->MarkupsDisplayNode->GetTextScale() * 3.) * numberLines;

        // center of background volume in display XY coordinates
        double backgroundVolumeCenterXY[2] = { 0.0 };
        this->GetBackgroundVolumeXYCenter(backgroundVolumeCenterXY);

        double leadersLineOffset[2] = { 0.0 };

        this->GetLabelTextXYOffsets(backgroundVolumeCenterXY, slicePos, leadersLineOffset);

        // Add a function to determine the exact bounds of the label text in 2D view
        double lineXSpacer = labelCharWidth;

        // set connector lineStartPos x and y according to location of fiducial
        lineStartPos[0] = slicePos[0];
        lineStartPos[1] = slicePos[1];

        // set lineEndPos and new labelPos according to location of fiducial

        if (slicePos[0] < backgroundVolumeCenterXY[0])
          {
          lineEndPos[0] = slicePos[0] + labelsOffset - leadersLineOffset[0];
          labelPos[0] = lineEndPos[0] - labelTextWidth - lineXSpacer;
          }
        else
          {
          lineEndPos[0] = slicePos[0] + labelsOffset + leadersLineOffset[0];
          labelPos[0] = lineEndPos[0] + (lineXSpacer / 2.);
          }

        if (slicePos[1] < backgroundVolumeCenterXY[1])
          {
          lineEndPos[1] = (slicePos[1] + labelsOffset) - leadersLineOffset[1];
          labelPos[1] = lineEndPos[1] - labelTextHeight;
          }
        else
          {
          lineEndPos[1] = (slicePos[1] + labelsOffset) + leadersLineOffset[1];
          labelPos[1] = lineEndPos[1] - (labelTextHeight/ 2.);
          }

        // handle label text touching slice view borders

        // left screen border
        if (labelPos[0] < 0.)
          {
          labelPos[0] = 0.;
          lineEndPos[0] = labelPos[0] + labelTextWidth + lineXSpacer;
          }
        // right screen border
        if (labelPos[0] > screenWidth - labelTextWidth)
          {
          labelPos[0] = screenWidth - labelTextWidth;
          lineEndPos[0] = labelPos[0] - lineXSpacer;
          }
        // upper screen border
        if (labelPos[1] > screenHeight - labelTextHeight * 2.)
          {
          labelPos[1] = screenHeight - labelTextHeight * 2.;
          lineEndPos[1] = labelPos[1] + labelTextHeight;
          }
        // lower screen border
        if (labelPos[1] < 0.)
          {
          labelPos[1] = 0.;
          lineEndPos[1] = labelPos[1] + labelTextHeight;
          }

        pts->InsertPoint(ptcnt, lineStartPos[0], lineStartPos[1], 0.);
        pts->InsertPoint(ptcnt + 1, lineEndPos[0], lineEndPos[1], 0.);
        line->InsertNextCell(2);
        line->InsertCellPoint(ptcnt);
        line->InsertCellPoint(ptcnt + 1);
        ptcnt += 2;

        // make the connector line color a bit lighter then the selected glyph/text color
        double color[3] = { 0,0,0 };
        this->MarkupsDisplayNode->GetSelectedColor(color);
        double hsv[3] = { 0,0,0 };
        vtkMath::RGBToHSV(color, hsv);
        hsv[0] *= 0.8;
        hsv[1] *= 0.3;
        vtkMath::HSVToRGB(hsv, color);
        controlPoints->LabelsLineProperty->SetColor(color);
        controlPoints->LabelsLineProperty->SetLineWidth(this->ControlPointSize * .25);

        controlPoints->LabelsLineActor->SetVisibility(true);
        }

      this->Renderer->SetDisplayPoint(labelPos);

      this->Renderer->DisplayToView();
      double viewPos[3] = { 0.0 };
      this->Renderer->GetViewPoint(viewPos);
      this->Renderer->ViewToNormalizedViewport(viewPos[0], viewPos[1], viewPos[2]);
      controlPoints->LabelControlPoints->InsertNextPoint(viewPos);

      double pointNormalWorld[3] = { 0.0, 0.0, 1.0 };
      markupsNode->GetNthControlPointNormalWorld(pointIndex, pointNormalWorld);
      // probably we should transform this orientation to display coordinate system
      controlPoints->ControlPointsPolyData->GetPointData()->GetNormals()->InsertNextTuple(pointNormalWorld);
      controlPoints->LabelControlPointsPolyData->GetPointData()->GetNormals()->InsertNextTuple(pointNormalWorld);

      controlPoints->Labels->InsertNextValue(labelStr);
      controlPoints->LabelsPriority->InsertNextValue(std::to_string(pointIndex));
      }

    controlPoints->ControlPoints->Modified();
    controlPoints->ControlPointsPolyData->GetPointData()->GetNormals()->Modified();
    controlPoints->ControlPointsPolyData->Modified();

    controlPoints->LabelControlPoints->Modified();
    controlPoints->LabelControlPointsPolyData->GetPointData()->GetNormals()->Modified();
    controlPoints->LabelControlPointsPolyData->Modified();

    controlPoints->LabelsLinePolyData->Modified();
    controlPoints->LabelsLinePolyData->SetPoints(pts);
    controlPoints->LabelsLinePolyData->SetLines(line);

    if (controlPointType == Active)
      {
      controlPoints->Actor->VisibilityOn();
      // For backward compatibility, we hide labels if text scale is set to 0.
      controlPoints->LabelsActor->SetVisibility(this->MarkupsDisplayNode->GetPointLabelsVisibility()
        && this->MarkupsDisplayNode->GetTextScale() > 0.0);
      }
    }
}

//----------------------------------------------------------------------
double vtkSlicerMarkupsWidgetRepresentation2D::GetWidgetOpacity(int controlPointType)
{
  if (!this->MarkupsDisplayNode)
    {
    return 1.0;
    }

  // Use hierarchy information if any, and if overriding is allowed for the current display node
  double hierarchyOpacity = 1.0;
  if (this->MarkupsDisplayNode->GetFolderDisplayOverrideAllowed())
    {
    vtkMRMLDisplayableNode* displayableNode = this->MarkupsDisplayNode->GetDisplayableNode();
    hierarchyOpacity = vtkMRMLFolderDisplayNode::GetHierarchyOpacity(displayableNode);
    }

  switch (controlPointType)
    {
    case Unselected: return this->MarkupsDisplayNode->GetOpacity() * hierarchyOpacity;
    case Selected: return this->MarkupsDisplayNode->GetOpacity() * hierarchyOpacity;
    case Active: return this->MarkupsDisplayNode->GetOpacity() * hierarchyOpacity;
    case Project: return this->MarkupsDisplayNode->GetSliceProjectionOpacity() * hierarchyOpacity;
    case ProjectBack: return this->MarkupsDisplayNode->GetSliceProjectionOpacity() * hierarchyOpacity;
    default:
      return 1.0;
    }
}

//----------------------------------------------------------------------
vtkMRMLSliceNode *vtkSlicerMarkupsWidgetRepresentation2D::GetSliceNode()
{
  return vtkMRMLSliceNode::SafeDownCast(this->ViewNode);
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation2D::GetNthControlPointDisplayPosition(int n, double slicePos[2])
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || n<0 || n>=markupsNode->GetNumberOfControlPoints() || !sliceNode)
    {
    return 0;
    }
  double worldPos[3];
  markupsNode->GetNthControlPointPositionWorld(n, worldPos);
  this->GetWorldToSliceCoordinates(worldPos, slicePos);
  return 1;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::SetNthControlPointSliceVisibility(int n, bool visibility)
{
  this->PointsVisibilityOnSlice->InsertValue(n, visibility);
  this->Modified();
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation2D::GetNthControlPointViewVisibility(int n)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || !this->GetVisibility())
    {
    return false;
    }
  if (!(markupsNode->GetNthControlPointPositionVisibility(n)
    && (markupsNode->GetNthControlPointVisibility(n))))
    {
    return false;
    }
  return (this->PointsVisibilityOnSlice->GetValue(n) != 0 || this->MarkupsDisplayNode->GetSliceProjection());
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::SetCenterSliceVisibility(bool visibility)
{
  this->CenterVisibilityOnSlice = visibility;
  this->Modified();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::UpdateFromMRMLInternal(vtkMRMLNode* caller, unsigned long event, void *callData/*=nullptr*/)
{
  Superclass::UpdateFromMRMLInternal(caller, event, callData);

  // Update from slice node
  if (!caller || caller == this->ViewNode.GetPointer())
    {
    this->UpdateViewScaleFactor();
    this->UpdatePlaneFromSliceNode();
    }

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode || !this->IsDisplayable())
    {
    this->VisibilityOff();
    return;
    }

  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
    {
    double* color = this->GetWidgetColor(controlPointType);
    double opacity = this->GetWidgetOpacity(controlPointType);

    ControlPointsPipeline2D* controlPoints = this->GetControlPointsPipeline(controlPointType);
    controlPoints->Property->SetColor(color);
    controlPoints->Property->SetOpacity(opacity);
    controlPoints->LabelsLineProperty->SetOpacity(opacity);

    controlPoints->TextProperty->ShallowCopy(this->MarkupsDisplayNode->GetTextProperty());
    if (this->GetApplicationLogic())
      {
      this->GetApplicationLogic()->UseCustomFontFile(controlPoints->TextProperty);
      }
    controlPoints->TextProperty->SetColor(color);
    controlPoints->TextProperty->SetOpacity(opacity);
    controlPoints->TextProperty->SetFontSize(static_cast<int>(this->MarkupsDisplayNode->GetTextProperty()->GetFontSize()
      * this->MarkupsDisplayNode->GetTextScale()));
    controlPoints->TextProperty->SetBackgroundOpacity(opacity * this->MarkupsDisplayNode->GetTextProperty()->GetBackgroundOpacity());

    vtkMarkupsGlyphSource2D* glyphSource = this->GetControlPointsPipeline(controlPointType)->GlyphSource2D;
    if (this->MarkupsDisplayNode->GlyphTypeIs3D())
      {
      // Sphere is the only 3D glyph, use 2D circle in default
      glyphSource->SetGlyphTypeToCircle();
      }
    else
      {
      glyphSource->SetGlyphType(this->GetGlyphTypeSourceFromDisplay(this->MarkupsDisplayNode->GetGlyphType()));
      }

    if (this->MarkupsDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane() && controlPointType == ProjectBack)
      {
      glyphSource->FilledOff();
      }
    else
      {
      glyphSource->FilledOn();
      }
      this->GetControlPointsPipeline(controlPointType)->Glypher->SetSourceConnection(glyphSource->GetOutputPort());
    }

  this->UpdateControlPointSize();
  this->UpdateInteractionHandleSize();

  // Points widgets have only one Markup/Representation
  this->AnyPointVisibilityOnSlice = false;
  for (int pointIndex = 0; pointIndex < markupsNode->GetNumberOfControlPoints(); pointIndex++)
    {
    bool visibility =  this->IsControlPointDisplayableOnSlice(markupsNode, pointIndex);
    if (visibility)
      {
      this->AnyPointVisibilityOnSlice = true;
      }
    this->SetNthControlPointSliceVisibility(pointIndex, visibility);
    }

  if (markupsNode->GetCurveClosed())
    {
    bool visibility = this->IsCenterDisplayableOnSlice(markupsNode);
    this->SetCenterSliceVisibility(visibility);
    }

  for (int controlPointType = 0; controlPointType < NumberOfControlPointTypes; ++controlPointType)
    {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[controlPointType]);
    // For backward compatibility, we hide labels if text scale is set to 0.
    controlPoints->LabelsActor->SetVisibility(this->MarkupsDisplayNode->GetPointLabelsVisibility()
      && this->MarkupsDisplayNode->GetTextScale() > 0.0);
    controlPoints->Glypher->SetScaleFactor(this->ControlPointSize);
    }

  // by default (no connector lines) put the labels near the boundary of the glyph, slightly away from it (by half picking tolarance)
  double labelsOffset = this->ControlPointSize * 0.5 + this->PickingTolerance * 0.5 * this->ScreenScaleFactor;
  this->UpdateAllPointsAndLabelsFromMRML(labelsOffset);

  this->VisibilityOn();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::CanInteract(
  vtkMRMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!sliceNode || !markupsNode || markupsNode->GetLocked() || !this->GetVisibility() || !interactionEventData)
    {
    return;
    }

  const int* displayPosition = interactionEventData->GetDisplayPosition();
  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };

  this->UpdateControlPointSize();
  double maxPickingDistanceFromControlPoint2 = this->GetMaximumControlPointPickingDistance2();

  closestDistance2 = VTK_DOUBLE_MAX; // in display coordinate system
  foundComponentIndex = -1;

  // We can interact with the handle if the mouse is hovering over one of the handles (translation or rotation), in display coordinates.
  this->CanInteractWithHandles(interactionEventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType != vtkMRMLMarkupsDisplayNode::ComponentNone)
    {
    // if mouse is near a handle then select that (ignore the line + control points)
    return;
    }

  if (markupsNode->GetNumberOfControlPoints() > 2 && this->CurveClosed && this->CenterVisibilityOnSlice)
    {
    // Check if center is selected
    double centerPosWorld[3], centerPosDisplay[3];
    markupsNode->GetCenterOfRotationWorld(centerPosWorld);
    this->GetWorldToSliceCoordinates(centerPosWorld, centerPosDisplay);

    double dist2 = vtkMath::Distance2BetweenPoints(centerPosDisplay, displayPosition3);
    if (dist2 < maxPickingDistanceFromControlPoint2)
      {
      closestDistance2 = dist2;
      foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentCenterPoint;
      foundComponentIndex = 0;
      }
    }

  vtkIdType numberOfPoints = markupsNode->GetNumberOfControlPoints();

  double pointDisplayPos[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointWorldPos[4] = { 0.0, 0.0, 0.0, 1.0 };

  vtkNew<vtkMatrix4x4> rasToxyMatrix;
  sliceNode->GetXYToRAS()->Invert(sliceNode->GetXYToRAS(), rasToxyMatrix.GetPointer());
  for (int i = 0; i < numberOfPoints; i++)
    {
    if (!this->GetNthControlPointViewVisibility(i))
      {
      continue;
      }
    markupsNode->GetNthControlPointPositionWorld(i, pointWorldPos);
    rasToxyMatrix->MultiplyPoint(pointWorldPos, pointDisplayPos);
    if (this->MarkupsDisplayNode->GetSliceProjection())
      {
      pointDisplayPos[2] = displayPosition3[2];
      }
    double dist2 = vtkMath::Distance2BetweenPoints(pointDisplayPos, displayPosition3);
    if (dist2 < maxPickingDistanceFromControlPoint2 && dist2 < closestDistance2)
      {
      closestDistance2 = dist2;
      foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentControlPoint;
      foundComponentIndex = i;
      }
    }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::CanInteractWithHandles(
  vtkMRMLInteractionEventData* interactionEventData,
  int& foundComponentType, int& foundComponentIndex, double& closestDistance2)
{
  if (!this->InteractionPipeline || !this->InteractionPipeline->Actor->GetVisibility())
    {
    return;
    }

  double maxPickingDistanceFromInteractionHandle2 = this->GetMaximumInteractionHandlePickingDistance2();

  const int* displayPosition = interactionEventData->GetDisplayPosition();
  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };

  double handleDisplayPos[4] = { 0.0, 0.0, 0.0, 1.0 };

  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
  vtkNew<vtkMatrix4x4> rasToxyMatrix;
  vtkMatrix4x4::Invert(sliceNode->GetXYToRAS(), rasToxyMatrix);

  bool handlePicked = false;
  vtkSlicerMarkupsWidgetRepresentation::HandleInfoList handleInfoList = this->InteractionPipeline->GetHandleInfoList();
  for (vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::HandleInfo handleInfo : handleInfoList)
    {
    if (!handleInfo.IsVisible())
      {
      continue;
      }
    double* handleWorldPos = handleInfo.PositionWorld;
    rasToxyMatrix->MultiplyPoint(handleWorldPos, handleDisplayPos);
    handleDisplayPos[2] = displayPosition3[2]; // Handles are always projected
    double dist2 = vtkMath::Distance2BetweenPoints(handleDisplayPos, displayPosition3);
    if (dist2 < maxPickingDistanceFromInteractionHandle2 && dist2 < closestDistance2)
      {
      closestDistance2 = dist2;
      foundComponentType = handleInfo.ComponentType;
      foundComponentIndex = handleInfo.Index;
      handlePicked = true;
      }
    }

  if (!handlePicked)
    {
    // Detect translation handle shaft
    for (vtkSlicerMarkupsWidgetRepresentation::MarkupsInteractionPipeline::HandleInfo handleInfo : handleInfoList)
      {
      if (!handleInfo.IsVisible() || handleInfo.ComponentType != vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle)
        {
        continue;
        }

      double* handleWorldPos = handleInfo.PositionWorld;
      rasToxyMatrix->MultiplyPoint(handleWorldPos, handleDisplayPos);
      handleDisplayPos[2] = displayPosition3[2]; // Handles are always projected

      double originWorldPos[4] = { 0.0, 0.0, 0.0, 1.0 };
      this->InteractionPipeline->GetInteractionHandleOriginWorld(originWorldPos);
      double originDisplayPos[4] = { 0.0 };
      rasToxyMatrix->MultiplyPoint(originWorldPos, originDisplayPos);
      originDisplayPos[2] = displayPosition3[2]; // Handles are always projected

      double t = 0;
      double lineDistance = vtkLine::DistanceToLine(displayPosition3, originDisplayPos, handleDisplayPos, t);
      double lineDistance2 = lineDistance * lineDistance;
      if (lineDistance2 < maxPickingDistanceFromInteractionHandle2 / 2.0 && lineDistance2 < closestDistance2)
        {
        closestDistance2 = lineDistance2;
        foundComponentType = handleInfo.ComponentType;
        foundComponentIndex = handleInfo.Index;
        }
      }
    }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::CanInteractWithLine(
  vtkMRMLInteractionEventData* interactionEventData,
  int &foundComponentType, int &foundComponentIndex, double &closestDistance2)
{
  foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;

  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if ( !sliceNode || !markupsNode || markupsNode->GetLocked() || markupsNode->GetNumberOfControlPoints() < 1
    || !this->GetVisibility() || !interactionEventData )
    {
    return;
    }

  const int* displayPosition = interactionEventData->GetDisplayPosition();
  double displayPosition3[3] = { static_cast<double>(displayPosition[0]), static_cast<double>(displayPosition[1]), 0.0 };
  double maxPickingDistanceFromControlPoint2 = this->GetMaximumControlPointPickingDistance2();

  vtkIdType numberOfPoints = markupsNode->GetNumberOfControlPoints();

  double pointDisplayPos1[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointWorldPos1[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointDisplayPos2[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointWorldPos2[4] = { 0.0, 0.0, 0.0, 1.0 };

  vtkNew<vtkMatrix4x4> rasToxyMatrix;
  sliceNode->GetXYToRAS()->Invert(sliceNode->GetXYToRAS(), rasToxyMatrix.GetPointer());
  for (int i = 0; i < numberOfPoints - 1; i++)
    {
    if (!this->PointsVisibilityOnSlice->GetValue(i))
      {
      continue;
      }
    if (!this->PointsVisibilityOnSlice->GetValue(i + 1))
      {
      i++; // skip one more, as the next iteration would use (i+1)-th point
      continue;
      }
    markupsNode->GetNthControlPointPositionWorld(i, pointWorldPos1);
    rasToxyMatrix->MultiplyPoint(pointWorldPos1, pointDisplayPos1);
    markupsNode->GetNthControlPointPositionWorld(i + 1, pointWorldPos2);
    rasToxyMatrix->MultiplyPoint(pointWorldPos2, pointDisplayPos2);

    double relativePositionAlongLine = -1.0; // between 0.0-1.0 if between the endpoints of the line segment
    double distance2 = vtkLine::DistanceToLine(displayPosition3, pointDisplayPos1, pointDisplayPos2, relativePositionAlongLine);
    if (distance2 < maxPickingDistanceFromControlPoint2 && distance2 < closestDistance2 && relativePositionAlongLine >= 0 && relativePositionAlongLine <= 1)
      {
      closestDistance2 = distance2;
      foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentLine;
      foundComponentIndex = i;
      }
    }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::GetActors(vtkPropCollection *pc)
{
  Superclass::GetActors(pc);
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    controlPoints->Actor->GetActors(pc);
    controlPoints->LabelsActor->GetActors(pc);
    controlPoints->LabelsLineActor->GetActors(pc);
    }
  this->TextActor->GetActors(pc);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::ReleaseGraphicsResources(
  vtkWindow *win)
{
  Superclass::ReleaseGraphicsResources(win);
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    controlPoints->Actor->ReleaseGraphicsResources(win);
    controlPoints->LabelsActor->ReleaseGraphicsResources(win);
    controlPoints->LabelsLineActor->ReleaseGraphicsResources(win);
    }
  this->TextActor->ReleaseGraphicsResources(win);
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation2D::RenderOverlay(vtkViewport *viewport)
{
  int count = Superclass::RenderOverlay(viewport);
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    if (controlPoints->Actor->GetVisibility())
      {
      count += controlPoints->Actor->RenderOverlay(viewport);
      }
    if (controlPoints->LabelsActor->GetVisibility())
      {
      count += controlPoints->LabelsActor->RenderOverlay(viewport);
      }
    if (controlPoints->LabelsLineActor->GetVisibility())
      {
      count += controlPoints->LabelsLineActor->RenderOverlay(viewport);
      }
    }
  if (this->TextActor->GetVisibility())
    {
    count += this->TextActor->RenderOverlay(viewport);
    }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation2D::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  int count = 0;
  if (this->InteractionPipeline && this->InteractionPipeline->Actor->GetVisibility())
    {
    this->InteractionPipeline->UpdateHandleColors();
    this->UpdateInteractionHandleSize();
    this->InteractionPipeline->SetWidgetScale(this->InteractionPipeline->InteractionHandleSize);
    count += this->InteractionPipeline->Actor->RenderOpaqueGeometry(viewport);
    }
  if (this->TextActor->GetVisibility())
    {
    count += this->TextActor->RenderOpaqueGeometry(viewport);
    }
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    if (controlPoints->Actor->GetVisibility())
      {
      count += controlPoints->Actor->RenderOpaqueGeometry(viewport);
      }
    if (controlPoints->LabelsActor->GetVisibility())
      {
      count += controlPoints->LabelsActor->RenderOpaqueGeometry(viewport);
      }
    if (controlPoints->LabelsLineActor->GetVisibility())
      {
      count += controlPoints->LabelsLineActor->RenderOpaqueGeometry(viewport);
      }
    }
  return count;
}

//-----------------------------------------------------------------------------
int vtkSlicerMarkupsWidgetRepresentation2D::RenderTranslucentPolygonalGeometry(
  vtkViewport *viewport)
{
  int count = Superclass::RenderTranslucentPolygonalGeometry(viewport);
  if (this->TextActor->GetVisibility())
    {
    count += this->TextActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    if (controlPoints->Actor->GetVisibility())
      {
      count += controlPoints->Actor->RenderTranslucentPolygonalGeometry(viewport);
      }
    if (controlPoints->LabelsActor->GetVisibility())
      {
      count += controlPoints->LabelsActor->RenderTranslucentPolygonalGeometry(viewport);
      }
    if (controlPoints->LabelsLineActor->GetVisibility())
      {
      count += controlPoints->LabelsLineActor->RenderTranslucentPolygonalGeometry(viewport);
      }
    }
  return count;
}

//-----------------------------------------------------------------------------
vtkTypeBool vtkSlicerMarkupsWidgetRepresentation2D::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  if (this->TextActor->GetVisibility() && this->TextActor->HasTranslucentPolygonalGeometry())
    {
    return true;
    }
  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    if (controlPoints->Actor->GetVisibility() && controlPoints->Actor->HasTranslucentPolygonalGeometry())
      {
      return true;
      }
    if (controlPoints->LabelsActor->GetVisibility() && controlPoints->LabelsActor->HasTranslucentPolygonalGeometry())
      {
      return true;
      }
    if (controlPoints->LabelsLineActor->GetVisibility() && controlPoints->LabelsLineActor->HasTranslucentPolygonalGeometry())
      {
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  if (this->TextActor)
    {
    os << indent << "Text Visibility: " << this->TextActor->GetVisibility() << "\n";
    }
  else
    {
    os << indent << "Text Visibility: (none)\n";
    }

  for (int i = 0; i < NumberOfControlPointTypes; i++)
    {
    ControlPointsPipeline2D* controlPoints = reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[i]);
    os << indent << "Pipeline " << i << "\n";
    if (controlPoints->Actor)
      {
      os << indent << "Points Visibility: " << controlPoints->Actor->GetVisibility() << "\n";
      }
    else
      {
      os << indent << "Points: (none)\n";
      }
    if (controlPoints->LabelsActor)
      {
      os << indent << "Labels Visibility: " << controlPoints->LabelsActor->GetVisibility() << "\n";
      }
    if (controlPoints->LabelsLineActor)
      {
      os << indent << "Labels Line Visibility: " << controlPoints->LabelsLineActor->GetVisibility() << "\n";
      }
    else
      {
      os << indent << "Labels Points: (none)\n";
      }
    if (controlPoints->Property)
      {
      os << indent << "Property: " << controlPoints->Property << "\n";
      }
    else
      {
      os << indent << "Property: (none)\n";
      }
    }
}

//-----------------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation2D::ControlPointsPipeline2D* vtkSlicerMarkupsWidgetRepresentation2D::GetControlPointsPipeline(int controlPointType)
{
  return reinterpret_cast<ControlPointsPipeline2D*>(this->ControlPoints[controlPointType]);
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation2D::IsControlPointDisplayableOnSlice(vtkMRMLMarkupsNode *markupsNode, int pointIndex)
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return false;
    }

  // if there's no node, it's not visible
  if (!markupsNode)
    {
    vtkErrorMacro("IsWidgetDisplayableOnSlice: Could not get the markups node.");
    return false;
    }

  bool showPoint = true;

  // allow nodes to appear only in designated viewers
  vtkMRMLDisplayNode *displayNode = markupsNode->GetDisplayNode();
  if (!displayNode)
    {
    return false;
    }

  double transformedWorldCoordinates[4];
  markupsNode->GetNthControlPointPositionWorld(pointIndex, transformedWorldCoordinates);

  // now get the displayCoordinates for the transformed worldCoordinates
  double displayCoordinates[4];
  this->GetWorldToDisplayCoordinates(transformedWorldCoordinates, displayCoordinates);

  // check if the point is close enough to the slice to be shown
  if (showPoint)
    {
    // the third coordinate of the displayCoordinates is the distance to the slice
    double distanceToSlice = displayCoordinates[2];
    double maxDistance = 0.5 + (sliceNode->GetDimensions()[2] - 1);
    /*vtkDebugMacro("Slice node " << sliceNode->GetName()
      << ": distance to slice = " << distanceToSlice
      << ", maxDistance = " << maxDistance
      << "\n\tslice node dimensions[2] = "
      << sliceNode->GetDimensions()[2]);*/
    if (distanceToSlice < -0.5 || distanceToSlice >= maxDistance)
      {
      // if the distance to the slice is more than 0.5mm, we know that at least one coordinate of the widget is outside the current activeSlice
      // hence, we do not want to show this widget
      showPoint = false;
      }
    }

  return showPoint;
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation2D::IsPointBehindSlice(vtkMRMLMarkupsNode *markupsNode, int pointIndex)
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return false;
    }

  // if there's no node, it's not visible
  if (!markupsNode)
    {
    vtkErrorMacro("IsWidgetDisplayableOnSlice: Could not get the markups node.");
    return false;
    }

  // allow nodes to appear only in designated viewers
  vtkMRMLDisplayNode *displayNode = markupsNode->GetDisplayNode();
  if (!displayNode)
    {
    return false;
    }

  double transformedWorldCoordinates[4];
  markupsNode->GetNthControlPointPositionWorld(pointIndex, transformedWorldCoordinates);

  // now get the displayCoordinates for the transformed worldCoordinates
  double displayCoordinates[4];
  this->GetWorldToDisplayCoordinates(transformedWorldCoordinates, displayCoordinates);

  // the third coordinate of the displayCoordinates is the distance to the slice
  double distanceToSlice = displayCoordinates[2];
  bool pointBehind = distanceToSlice < -0.5;
  return pointBehind;
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation2D::IsPointInFrontSlice(vtkMRMLMarkupsNode *markupsNode, int pointIndex)
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    return false;
    }

  // if there's no node, it's not visible
  if (!markupsNode)
    {
    vtkErrorMacro("IsWidgetDisplayableOnSlice: Could not get the markups node.");
    return false;
    }

  // allow nodes to appear only in designated viewers
  vtkMRMLDisplayNode *displayNode = markupsNode->GetDisplayNode();
  if (!displayNode)
    {
    return false;
    }

  double transformedWorldCoordinates[4];
  markupsNode->GetNthControlPointPositionWorld(pointIndex, transformedWorldCoordinates);

  // now get the displayCoordinates for the transformed worldCoordinates
  double displayCoordinates[4];
  this->GetWorldToDisplayCoordinates(transformedWorldCoordinates, displayCoordinates);

  // the third coordinate of the displayCoordinates is the distance to the slice
  double distanceToSlice = displayCoordinates[2];
  bool pointFront = distanceToSlice > 0.5;
  return pointFront;
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation2D::IsCenterDisplayableOnSlice(vtkMRMLMarkupsNode *markupsNode)
{
  // if no slice node, it doesn't constrain the visibility, so return that
  // it's visible
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    vtkErrorMacro("IsWidgetDisplayableOnSlice: Could not get the sliceNode.");
    return false;
    }

  // if there's no node, it's not visible
  if (!markupsNode)
    {
    vtkErrorMacro("IsWidgetDisplayableOnSlice: Could not get the markups node.");
    return false;
    }

  bool showPoint = true;

  // allow nodes to appear only in designated viewers
  if (!markupsNode || !this->IsDisplayable())
    {
    return false;
    }

  // down cast the node as a controlpoints node to get the coordinates
  double transformedWorldCoordinates[4];
  markupsNode->GetCenterOfRotationWorld(transformedWorldCoordinates);

  // now get the displayCoordinates for the transformed worldCoordinates
  double displayCoordinates[4];
  this->GetWorldToDisplayCoordinates(transformedWorldCoordinates, displayCoordinates);

  // check if the point is close enough to the slice to be shown
  if (showPoint)
    {
    // the third coordinate of the displayCoordinates is the distance to the slice
    double distanceToSlice = displayCoordinates[2];
    double maxDistance = 0.5 + (sliceNode->GetDimensions()[2] - 1);
    /*vtkDebugMacro("Slice node " << sliceNode->GetName()
      << ": distance to slice = " << distanceToSlice
      << ", maxDistance = " << maxDistance
      << "\n\tslice node dimensions[2] = "
      << sliceNode->GetDimensions()[2]);*/
    if (distanceToSlice < -0.5 || distanceToSlice >= maxDistance)
      {
      // if the distance to the slice is more than 0.5mm, we know that at least one coordinate of the widget is outside the current activeSlice
      // hence, we do not want to show this widget
      showPoint = false;
      }
    }

  return showPoint;
}


//---------------------------------------------------------------------------
/// Convert world to display coordinates
void vtkSlicerMarkupsWidgetRepresentation2D::GetWorldToDisplayCoordinates(double r, double a, double s, double * displayCoordinates)
{
  vtkMRMLSliceNode *sliceNode = this->GetSliceNode();
  if (!sliceNode)
    {
    vtkErrorMacro("GetWorldToDisplayCoordinates: no slice node!");
    return;
    }

  // we will get the transformation matrix to convert world coordinates to the display coordinates of the specific sliceNode

  vtkMatrix4x4 * xyToRasMatrix = sliceNode->GetXYToRAS();
  vtkNew<vtkMatrix4x4> rasToXyMatrix;

  // we need to invert this matrix
  xyToRasMatrix->Invert(xyToRasMatrix, rasToXyMatrix.GetPointer());

  double worldCoordinates[4];
  worldCoordinates[0] = r;
  worldCoordinates[1] = a;
  worldCoordinates[2] = s;
  worldCoordinates[3] = 1;

  rasToXyMatrix->MultiplyPoint(worldCoordinates, displayCoordinates);
  xyToRasMatrix = nullptr;
}

//---------------------------------------------------------------------------
// Convert world to display coordinates
void vtkSlicerMarkupsWidgetRepresentation2D::GetWorldToDisplayCoordinates(double * worldCoordinates, double * displayCoordinates)
{
  if (worldCoordinates == nullptr)
    {
    return;
    }

  this->GetWorldToDisplayCoordinates(worldCoordinates[0], worldCoordinates[1], worldCoordinates[2], displayCoordinates);
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation2D::GetAllControlPointsVisible()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return false;
    }

  for (int controlPointIndex = 0; controlPointIndex < markupsNode->GetNumberOfControlPoints(); controlPointIndex++)
    {
    if (!this->PointsVisibilityOnSlice->GetValue(controlPointIndex) ||
      !(markupsNode->GetNthControlPointPositionVisibility(controlPointIndex)
        && (markupsNode->GetNthControlPointVisibility(controlPointIndex))))
      {
      return false;
      }
   }
  return true;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::UpdateDistanceColorMap(
  vtkDiscretizableColorTransferFunction* colormap, double color[3])
{
  if (colormap == nullptr || this->MarkupsDisplayNode == nullptr)
    {
    return;
    }

  double limit = this->MarkupsDisplayNode->GetLineColorFadingEnd();
  double tolerance = this->MarkupsDisplayNode->GetLineColorFadingStart();
  vtkPiecewiseFunction* opacityFunction = colormap->GetScalarOpacityFunction();
  if (!opacityFunction)
    {
    opacityFunction = vtkPiecewiseFunction::New();
    colormap->SetScalarOpacityFunction(opacityFunction);
    opacityFunction->Delete();
    }
  double opacity = this->MarkupsDisplayNode->GetOpacity();
  opacityFunction->RemoveAllPoints();
  opacityFunction->AddPoint(-limit, 0.);
  opacityFunction->AddPoint(-tolerance, opacity);
  opacityFunction->AddPoint(tolerance, opacity);
  opacityFunction->AddPoint(limit, 0.);

  colormap->RemoveAllPoints();
  double color1[3] =
    {
    std::min(color[0] * 0.5, 1.0),
    std::min(color[1] * 0.5, 1.0),
    std::min(color[2] * 2.0, 1.0)
    };
  double color3[3] =
    {
    std::min(color[0] * 2.0, 1.0),
    std::min(color[1] * 0.5, 1.0),
    std::min(color[2] * 0.5, 1.0)
    };

  double colorHsv1[3];
  double colorHsv2[3];
  double colorHsv3[3];
  vtkMath::RGBToHSV(color1, colorHsv1);
  vtkMath::RGBToHSV(color, colorHsv2);
  vtkMath::RGBToHSV(color3, colorHsv3);

  double hueOffset = this->MarkupsDisplayNode->GetLineColorFadingHueOffset();
  double saturation = this->MarkupsDisplayNode->GetLineColorFadingSaturation();

  colormap->AddHSVPoint(-tolerance * 2.0, colorHsv1[0] - hueOffset, saturation * colorHsv1[1], colorHsv1[2]);
  colormap->AddHSVPoint(-tolerance, colorHsv2[0] - hueOffset, saturation * colorHsv2[1], colorHsv2[2]);
  colormap->AddHSVPoint( tolerance, colorHsv2[0] - hueOffset, saturation * colorHsv2[1], colorHsv2[2]);
  colormap->AddHSVPoint( tolerance * 2.0, colorHsv3[0] - hueOffset, saturation * colorHsv3[1], colorHsv3[2]);
  colormap->SetEnableOpacityMapping(true);
  colormap->SetClamping(true);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::UpdatePlaneFromSliceNode()
{
  vtkMatrix4x4* sliceXYToRAS = this->GetSliceNode()->GetXYToRAS();

  // Update transformation to slice
  vtkNew<vtkMatrix4x4> rasToSliceXY;
  vtkMatrix4x4::Invert(sliceXYToRAS, rasToSliceXY.GetPointer());
  // Project all points to the slice plane (slice Z coordinate = 0)
  rasToSliceXY->SetElement(2, 0, 0);
  rasToSliceXY->SetElement(2, 1, 0);
  rasToSliceXY->SetElement(2, 2, 0);
  this->WorldToSliceTransform->SetMatrix(rasToSliceXY.GetPointer());

  // Update slice plane (for distance computation)
  double normal[3];
  double origin[3];
  const double planeOrientation = 1.0; // +/-1: orientation of the normal
  for (int i = 0; i < 3; i++)
    {
    normal[i] = planeOrientation * sliceXYToRAS->GetElement(i, 2);
    origin[i] = sliceXYToRAS->GetElement(i, 3);
    }
  vtkMath::Normalize(normal);
  this->SlicePlane->SetNormal(normal);
  this->SlicePlane->SetOrigin(origin);
  this->SlicePlane->Modified();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::UpdateViewScaleFactor()
{
  this->ViewScaleFactorMmPerPixel = 1.0;
  this->ScreenSizePixel = 1000.0;
  if (!this->Renderer || !this->Renderer->GetActiveCamera() || !this->GetSliceNode())
    {
    return;
    }

  const int* screenSize = this->Renderer->GetRenderWindow()->GetScreenSize();
  this->ScreenSizePixel = sqrt(screenSize[0] * screenSize[0] + screenSize[1] * screenSize[1]);

  vtkMatrix4x4* xyToSlice = this->GetSliceNode()->GetXYToSlice();
  this->ViewScaleFactorMmPerPixel = sqrt(xyToSlice->GetElement(0, 1) * xyToSlice->GetElement(0, 1)
    + xyToSlice->GetElement(1, 1) * xyToSlice->GetElement(1, 1));
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::UpdateControlPointSize()
{
  // Since we use parallel camera projection and the camera scale is 1.0,
  // the renderer coordinate system is the same as the display coordinate system, therefore
  // ControlPointSize is specified in pixels.
  if (this->MarkupsDisplayNode->GetUseGlyphScale())
    {
    // relative
    this->ControlPointSize = this->ScreenSizePixel * this->ScreenScaleFactor * this->MarkupsDisplayNode->GetGlyphScale() / 100.0;
    }
  else
    {
    // absolute
    this->ControlPointSize = this->MarkupsDisplayNode->GetGlyphSize() / this->ViewScaleFactorMmPerPixel;
    }
}

//----------------------------------------------------------------------
double vtkSlicerMarkupsWidgetRepresentation2D::GetMaximumControlPointPickingDistance2()
{
  double maximumControlPointPickingDistance = this->ControlPointSize / 2.0 + this->PickingTolerance * this->ScreenScaleFactor;
  return maximumControlPointPickingDistance * maximumControlPointPickingDistance;
}

//----------------------------------------------------------------------
double vtkSlicerMarkupsWidgetRepresentation2D::GetMaximumInteractionHandlePickingDistance2()
{
  if (!this->InteractionPipeline)
    {
    return 0.0;
    }
  double maximumInteractionHandlePickingDistance = this->InteractionPipeline->InteractionHandleSize / 2.0 + this->PickingTolerance * this->ScreenScaleFactor;
  return maximumInteractionHandlePickingDistance * maximumInteractionHandlePickingDistance;
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsWidgetRepresentation2D::IsRepresentationIntersectingSlice(vtkPolyData* representation, const char* arrayName)
{
  if (!representation || !representation->GetPointData() || representation->GetNumberOfPoints() <= 0)
    {
    return false;
    }

  double sliceNormal_XY[4] = { 0.0, 0.0, 1.0, 0.0 };
  double sliceNormal_World[4] = { 0, 0, 1, 0 };
  vtkMatrix4x4* xyToRAS = this->GetSliceNode()->GetXYToRAS();
  xyToRAS->MultiplyPoint(sliceNormal_XY, sliceNormal_World);
  double sliceThicknessMm = vtkMath::Norm(sliceNormal_World);

  vtkDataArray* distanceArray = representation->GetPointData()->GetArray(arrayName);
  if (!distanceArray)
    {
    return false;
    }
  double* scalarRange = distanceArray->GetRange();

  // If the closest point on the line is further than a half-slice thickness, then hide the markup in 2D
  if (scalarRange[0] > 0.5 * sliceThicknessMm || scalarRange[1] < -0.5 * sliceThicknessMm)
    {
    return false;
    }
  return true;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::SetupInteractionPipeline()
{
  this->InteractionPipeline = new MarkupsInteractionPipeline2D(this);
  this->InteractionPipeline->InitializePipeline();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::UpdateInteractionPipeline()
{
  MarkupsInteractionPipeline2D* interactionPipeline = dynamic_cast<MarkupsInteractionPipeline2D*>(this->InteractionPipeline);
  if (!interactionPipeline)
    {
    return;
    }
  interactionPipeline->WorldToSliceTransformFilter->SetTransform(this->WorldToSliceTransform);
  // Final visibility handled by superclass in vtkSlicerMarkupsWidgetRepresentation
  Superclass::UpdateInteractionPipeline();
}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation2D::MarkupsInteractionPipeline2D::MarkupsInteractionPipeline2D(vtkSlicerMarkupsWidgetRepresentation* representation)
  : MarkupsInteractionPipeline(representation)
{
  this->WorldToSliceTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->WorldToSliceTransformFilter->SetTransform(vtkNew<vtkTransform>());
  this->WorldToSliceTransformFilter->SetInputConnection(this->HandleToWorldTransformFilter->GetOutputPort());
  this->Mapper->SetInputConnection(this->WorldToSliceTransformFilter->GetOutputPort());
  this->Mapper->SetTransformCoordinate(nullptr);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidgetRepresentation2D::MarkupsInteractionPipeline2D::GetViewPlaneNormal(double viewPlaneNormal[3])
{
  if (!viewPlaneNormal)
    {
    return;
    }

  double viewPlaneNormal4[4] = { 0, 0, 1, 0 };
  if (this->Representation)
    {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(this->Representation->GetViewNode());
    if (sliceNode)
      {
      sliceNode->GetSliceToRAS()->MultiplyPoint(viewPlaneNormal4, viewPlaneNormal4);
      }
    }
  viewPlaneNormal[0] = viewPlaneNormal4[0];
  viewPlaneNormal[1] = viewPlaneNormal4[1];
  viewPlaneNormal[2] = viewPlaneNormal4[2];
}
