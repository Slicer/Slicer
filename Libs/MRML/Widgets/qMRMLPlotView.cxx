/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QToolButton>

// STD includes
#include <algorithm>
#include <sstream>
#include <vector>

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkLogger.h>
//#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLPlotView_p.h"

// MRML includes
#include <vtkMRMLPlotSeriesNode.h>
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLPlotViewNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTableNode.h>

// VTK includes
#include <vtkAxis.h>
#include <vtkBrush.h>
#include <vtkChartLegend.h>
#include <vtkChartXY.h>
#include <vtkCollection.h>
#include <vtkContextMouseEvent.h>
#include <vtkContextScene.h>
#include <vtkContextView.h>
#include <vtkGL2PSExporter.h>
#include <vtkNew.h>
#include <vtkPen.h>
#include <vtkPlot.h>
#include <vtkPlotLine.h>
#include <vtkPlotBar.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSelection.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkTextProperty.h>

//--------------------------------------------------------------------------
// qMRMLPlotViewPrivate methods

//---------------------------------------------------------------------------
qMRMLPlotViewPrivate::qMRMLPlotViewPrivate(qMRMLPlotView& object)
  : q_ptr(&object)
{
  this->MRMLScene = nullptr;
  this->MRMLPlotViewNode = nullptr;
  this->MRMLPlotChartNode = nullptr;
  //this->PinButton = 0;
//  this->PopupWidget = 0;
  this->UpdatingWidgetFromMRML = false;
}

//---------------------------------------------------------------------------
qMRMLPlotViewPrivate::~qMRMLPlotViewPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLPlotViewPrivate::init()
{
  Q_Q(qMRMLPlotView);

  if (!q->chart())
    {
    return;
    }

  qvtkConnect(q->chart(), vtkCommand::SelectionChangedEvent, this, SLOT(emitSelection()));
  qvtkConnect(q->chart(), vtkCommand::InteractionEvent, q, SLOT(updateMRMLChartAxisRangeFromWidget()));

  if (!q->chart()->GetBackgroundBrush() ||
      !q->chart()->GetTitleProperties() ||
      !q->chart()->GetLegend()          ||
      !q->scene())
    {
    return;
    }

  if (!q->chart()->GetLegend()->GetLabelProperties() ||
      !q->scene()->GetRenderer())
    {
    return;
    }

  vtkColor4ub color;
  color.Set(255., 253., 246., 255.);
  q->chart()->GetBackgroundBrush()->SetColor(color);
  q->chart()->GetTitleProperties()->SetFontFamilyToArial();
  q->chart()->GetTitleProperties()->SetFontSize(20);
  q->chart()->GetLegend()->GetLabelProperties()->SetFontFamilyToArial();
  q->scene()->GetRenderer()->SetUseDepthPeeling(true);
  q->scene()->GetRenderer()->SetUseFXAA(true);

  vtkAxis* axis = q->chart()->GetAxis(vtkAxis::LEFT);
  if (axis)
    {
    axis->GetTitleProperties()->SetFontFamilyToArial();
    axis->GetTitleProperties()->SetFontSize(16);
    axis->GetTitleProperties()->SetBold(false);
    axis->GetLabelProperties()->SetFontFamilyToArial();
    axis->GetLabelProperties()->SetFontSize(12);
    }
  axis = q->chart()->GetAxis(vtkAxis::BOTTOM);
  if (axis)
    {
    axis->GetTitleProperties()->SetFontFamilyToArial();
    axis->GetTitleProperties()->SetFontSize(16);
    axis->GetTitleProperties()->SetBold(false);
    axis->GetLabelProperties()->SetFontFamilyToArial();
    axis->GetLabelProperties()->SetFontSize(12);
    }
  axis = q->chart()->GetAxis(vtkAxis::RIGHT);
  if (axis)
    {
    axis->GetTitleProperties()->SetFontFamilyToArial();
    axis->GetTitleProperties()->SetFontSize(16);
    axis->GetTitleProperties()->SetBold(false);
    axis->GetLabelProperties()->SetFontFamilyToArial();
    axis->GetLabelProperties()->SetFontSize(12);
    }
  axis = q->chart()->GetAxis(vtkAxis::TOP);
  if (axis)
    {
    axis->GetTitleProperties()->SetFontFamilyToArial();
    axis->GetTitleProperties()->SetFontSize(16);
    axis->GetTitleProperties()->SetBold(false);
    axis->GetLabelProperties()->SetFontFamilyToArial();
    axis->GetLabelProperties()->SetFontSize(12);
    }

}

//---------------------------------------------------------------------------
void qMRMLPlotViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  if (newScene == this->MRMLScene)
    {
    return;
    }

  this->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::StartBatchProcessEvent, this, SLOT(startProcessing()));

  this->qvtkReconnect(
    this->mrmlScene(), newScene,
    vtkMRMLScene::EndBatchProcessEvent, this, SLOT(endProcessing()));

  this->MRMLScene = newScene;

  // Update chart (just in case plot view node was set before the scene)
  this->onPlotChartNodeChanged();
}

// --------------------------------------------------------------------------
vtkMRMLPlotSeriesNode* qMRMLPlotViewPrivate::plotSeriesNodeFromPlot(vtkPlot* plot)
{
  if (plot == nullptr)
    {
    return nullptr;
    }
  QMap< vtkPlot*, QString >::iterator plotIt = this->MapPlotToPlotSeriesNodeID.find(plot);
  if (plotIt == this->MapPlotToPlotSeriesNodeID.end())
    {
    return nullptr;
    }
  QString plotSeriesNodeID = plotIt.value();
  if (plotSeriesNodeID.isEmpty())
    {
    return nullptr;
    }
  vtkMRMLPlotSeriesNode* plotSeriesNode = vtkMRMLPlotSeriesNode::SafeDownCast(this->mrmlScene()->GetNodeByID(plotSeriesNodeID.toUtf8().constData()));
  if (plotSeriesNode == nullptr)
    {
    // node is not in the scene anymore
    this->MapPlotToPlotSeriesNodeID.erase(plotIt);
    }
  return plotSeriesNode;
}

// --------------------------------------------------------------------------
void qMRMLPlotViewPrivate::adjustRangeForLogScale(double range[2], double computedLimit[2])
{
  // Log scale is only enabled for positive axis range,
  // as vtkAxis behavior is inconsistent for negative values.
  if (range[1] <= 0)
    {
    computedLimit[0] = 0.1;
    computedLimit[1] = 1;
    range[0] = 0.1;
    range[1] = 1;
    return;
    }

  computedLimit[0] = 0;
  computedLimit[1] = vtkMath::Inf();
  if (range[0] <= 0)
    {
    // The minimum value is set to either 4 decades below the max or to 1,
    // regardless of the true minimum value (which is less than 0)
    range[0] = (range[1] < 1.e4 ? range[1] / 1.e4 : 1.);
    }
}

// --------------------------------------------------------------------------
vtkSmartPointer<vtkPlot> qMRMLPlotViewPrivate::updatePlotFromPlotSeriesNode(vtkMRMLPlotSeriesNode* plotSeriesNode, vtkPlot* existingPlot)
{
  if (plotSeriesNode == nullptr)
    {
    return nullptr;
    }
  vtkMRMLTableNode* tableNode = plotSeriesNode->GetTableNode();
  if (tableNode == nullptr || tableNode->GetTable() == nullptr)
    {
    return nullptr;
    }
  vtkTable *table = tableNode->GetTable();
  std::string yColumnName = plotSeriesNode->GetYColumnName();
  if (yColumnName.empty())
    {
    return nullptr;
    }
  vtkAbstractArray* yColumn = table->GetColumnByName(yColumnName.c_str());
  if (!yColumn)
    {
    return nullptr;
    }
  int yColumnType = yColumn->GetDataType();
  if (yColumnType == VTK_STRING || yColumnType == VTK_BIT)
    {
    qWarning() << Q_FUNC_INFO << ": Y column has unsupported data type: 'string' or 'bit'";
    return nullptr;
    }

  std::string xColumnName = plotSeriesNode->GetXColumnName();
  vtkAbstractArray* xColumn = nullptr;
  if (!xColumnName.empty())
    {
    xColumn = table->GetColumnByName(xColumnName.c_str());
    }

  if (plotSeriesNode->IsXColumnRequired())
    {
    if (!xColumn)
      {
      return nullptr;
      }
    int xColumnType = xColumn->GetDataType();
    if (xColumnType == VTK_STRING || xColumnType == VTK_BIT)
      {
      qWarning() << Q_FUNC_INFO << ": X column has unsupported data type for scatter plot: 'string' or 'bit'";
      return nullptr;
      }
    }

  vtkSmartPointer<vtkPlot> newPlot = existingPlot;
  int plotType = plotSeriesNode->GetPlotType();
  switch (plotType)
    {
    case vtkMRMLPlotSeriesNode::PlotTypeScatter:
    case vtkMRMLPlotSeriesNode::PlotTypeLine:
      if (!existingPlot || !existingPlot->IsA("vtkPlotLine"))
        {
        newPlot = vtkSmartPointer<vtkPlotLine>::New();
        }
      break;
    case vtkMRMLPlotSeriesNode::PlotTypeBar:
    case vtkMRMLPlotSeriesNode::PlotTypeScatterBar:
      if (!existingPlot || !existingPlot->IsA("vtkPlotBar"))
        {
        newPlot = vtkSmartPointer<vtkPlotBar>::New();
        }
      break;
    default:
      return nullptr;
    }

  // Common properties
  newPlot->SetWidth(plotSeriesNode->GetLineWidth());
  double* color = plotSeriesNode->GetColor();
  newPlot->SetColor(color[0], color[1], color[2]);
  newPlot->SetOpacity(plotSeriesNode->GetOpacity());
  if (newPlot->GetPen())
    {
    newPlot->GetPen()->SetOpacityF(plotSeriesNode->GetOpacity());
    if (plotType == vtkMRMLPlotSeriesNode::PlotTypeBar)
      {
      newPlot->GetPen()->SetLineType(vtkPen::SOLID_LINE);
      }
    else
      {
      int lineStyleVtk = vtkPen::NO_PEN;
      switch (plotSeriesNode->GetLineStyle())
      {
      case vtkMRMLPlotSeriesNode::LineStyleNone: lineStyleVtk = vtkPen::NO_PEN; break;
      case vtkMRMLPlotSeriesNode::LineStyleSolid: lineStyleVtk = vtkPen::SOLID_LINE; break;
      case vtkMRMLPlotSeriesNode::LineStyleDash: lineStyleVtk = vtkPen::DASH_LINE; break;
      case vtkMRMLPlotSeriesNode::LineStyleDot: lineStyleVtk = vtkPen::DOT_LINE; break;
      case vtkMRMLPlotSeriesNode::LineStyleDashDot: lineStyleVtk = vtkPen::DASH_DOT_LINE; break;
      case vtkMRMLPlotSeriesNode::LineStyleDashDotDot: lineStyleVtk = vtkPen::DASH_DOT_DOT_LINE; break;
      default:
        lineStyleVtk = vtkPen::NO_PEN;
      }
      newPlot->GetPen()->SetLineType(lineStyleVtk);
      }
    }

  // Type-specific properties
  vtkPlotLine* plotLine = vtkPlotLine::SafeDownCast(newPlot);
  if (plotLine)
    {
    plotLine->SetMarkerSize(plotSeriesNode->GetMarkerSize());

    int markerStyleVtk = VTK_MARKER_UNKNOWN;
    switch (plotSeriesNode->GetMarkerStyle())
      {
      case vtkMRMLPlotSeriesNode::MarkerStyleNone: markerStyleVtk = VTK_MARKER_NONE; break;
      case vtkMRMLPlotSeriesNode::MarkerStyleCross: markerStyleVtk = VTK_MARKER_CROSS; break;
      case vtkMRMLPlotSeriesNode::MarkerStylePlus: markerStyleVtk = VTK_MARKER_PLUS; break;
      case vtkMRMLPlotSeriesNode::MarkerStyleSquare: markerStyleVtk = VTK_MARKER_SQUARE; break;
      case vtkMRMLPlotSeriesNode::MarkerStyleCircle: markerStyleVtk = VTK_MARKER_CIRCLE; break;
      case vtkMRMLPlotSeriesNode::MarkerStyleDiamond: markerStyleVtk = VTK_MARKER_DIAMOND; break;
      default:
        markerStyleVtk = VTK_MARKER_UNKNOWN;
      }
    plotLine->SetMarkerStyle(markerStyleVtk);
    }

  vtkStringArray* labelArray = nullptr;
  std::string labelColumnName = plotSeriesNode->GetLabelColumnName();
  if (!labelColumnName.empty())
    {
    labelArray = vtkStringArray::SafeDownCast(table->GetColumnByName(labelColumnName.c_str()));
    }
  newPlot->SetIndexedLabels(labelArray);

  if (plotSeriesNode->IsXColumnRequired())
    {
    newPlot->SetUseIndexForXSeries(false);
    newPlot->SetInputData(table, xColumnName, yColumnName);
    if (labelArray)
      {
      newPlot->SetTooltipLabelFormat("%l = (%x, %y) %i");
      }
    else
      {
      newPlot->SetTooltipLabelFormat("%l = (%x, %y)");
      }
    }
  else
    {
    newPlot->SetUseIndexForXSeries(true);
    // In the case of Indexes, SetInputData still needs a proper Column.
    newPlot->SetInputData(table, yColumnName, yColumnName);
    if (labelArray)
      {
      newPlot->SetTooltipLabelFormat("%i: %l = %y");
      }
    else
      {
      newPlot->SetTooltipLabelFormat("%l = %y");
      }
    }

  if (plotSeriesNode->GetName())
    {
    newPlot->SetLabel(plotSeriesNode->GetName());
    }
  else
    {
    newPlot->SetLabel(yColumnName);
    }

  // TODO: set labels as axis text using vtkAxis::SetCustomTickPositions(vtkDoubleArray *positions, vtkStringArray *labels)

  return newPlot;
}

// --------------------------------------------------------------------------
void qMRMLPlotViewPrivate::startProcessing()
{
}

//
// --------------------------------------------------------------------------
void qMRMLPlotViewPrivate::endProcessing()
{
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLPlotViewPrivate::mrmlScene()
{
  return this->MRMLScene;
}

// --------------------------------------------------------------------------
void qMRMLPlotViewPrivate::onPlotChartNodeChanged()
{
  vtkMRMLPlotChartNode *newPlotChartNode = nullptr;

  if (this->MRMLScene && this->MRMLPlotViewNode && this->MRMLPlotViewNode->GetPlotChartNodeID())
    {
    newPlotChartNode = vtkMRMLPlotChartNode::SafeDownCast
      (this->MRMLScene->GetNodeByID(this->MRMLPlotViewNode->GetPlotChartNodeID()));
    }

  this->qvtkReconnect(this->MRMLPlotChartNode, newPlotChartNode,
    vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  this->MRMLPlotChartNode = newPlotChartNode;

  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLPlotViewPrivate::RecalculateBounds()
{
  Q_Q(qMRMLPlotView);

  if (!q->chart())
    {
    return;
    }

  // The following is a clone of vtkChartXY::RecalculatePlotBounds(),
  // with the difference that axis is rescaled even if its behavior is not set to vtkAxis::AUTO
  // and how log scale range and limits are computed.

  // Get the bounds of each plot, and each axis  - ordering as laid out below
  double y1[] = { 0.0, 0.0 }; // left -> 0
  double x1[] = { 0.0, 0.0 }; // bottom -> 1
  double y2[] = { 0.0, 0.0 }; // right -> 2
  double x2[] = { 0.0, 0.0 }; // top -> 3
                              // Store whether the ranges have been initialized - follows same order
  bool initialized[] = { false, false, false, false };
  double bounds[4] = { 0.0, 0.0, 0.0, 0.0 };
  for (int plotIndex = 0; plotIndex < q->chart()->GetNumberOfPlots(); plotIndex++)
    {
    vtkPlot* plot = q->chart()->GetPlot(plotIndex);
    if (!plot->GetVisible())
      {
      continue;
      }
    plot->GetUnscaledInputBounds(bounds);
    if (bounds[1] - bounds[0] < 0.0)
      {
      // skip uninitialized bounds.
      continue;
      }
    int corner = q->chart()->GetPlotCorner(plot);

    // Initialize the appropriate ranges, or push out the ranges
    if ((corner == 0 || corner == 3)) // left
      {
      if (!initialized[0])
        {
        y1[0] = bounds[2];
        y1[1] = bounds[3];
        initialized[0] = true;
        }
      else
        {
        if (y1[0] > bounds[2]) // min
          {
          y1[0] = bounds[2];
          }
        if (y1[1] < bounds[3]) // max
          {
          y1[1] = bounds[3];
          }
        }
      }
    if ((corner == 0 || corner == 1)) // bottom
      {
      if (!initialized[1])
        {
        x1[0] = bounds[0];
        x1[1] = bounds[1];
        initialized[1] = true;
        }
      else
        {
        if (x1[0] > bounds[0]) // min
          {
          x1[0] = bounds[0];
          }
        if (x1[1] < bounds[1]) // max
          {
          x1[1] = bounds[1];
          }
        }
      }
    if ((corner == 1 || corner == 2)) // right
      {
      if (!initialized[2])
        {
        y2[0] = bounds[2];
        y2[1] = bounds[3];
        initialized[2] = true;
        }
      else
        {
        if (y2[0] > bounds[2]) // min
          {
          y2[0] = bounds[2];
          }
        if (y2[1] < bounds[3]) // max
          {
          y2[1] = bounds[3];
          }
        }
      }
    if ((corner == 2 || corner == 3)) // top
    {
      if (!initialized[3])
        {
        x2[0] = bounds[0];
        x2[1] = bounds[1];
        initialized[3] = true;
        }
      else
        {
        if (x2[0] > bounds[0]) // min
          {
          x2[0] = bounds[0];
          }
        if (x2[1] < bounds[1]) // max
          {
          x2[1] = bounds[1];
          }
        }
      }
    }

  // Now set the newly calculated bounds on the axes
  for (int i = 0; i < 4; ++i)
  {
    vtkAxis* axis = q->chart()->GetAxis(i);
    double* range = nullptr;
    switch (i)
      {
    case 0:
      range = y1;
      break;
    case 1:
      range = x1;
      break;
    case 2:
      range = y2;
      break;
    case 3:
      range = x2;
      break;
    default:
      return;
      }

    double limit[2] = { -vtkMath::Inf(), vtkMath::Inf() };
    if (axis->GetLogScale())
     {
      this->adjustRangeForLogScale(range, limit);
      }
    axis->SetUnscaledMinimumLimit(limit[0]);
    axis->SetUnscaledMaximumLimit(limit[1]);
    axis->SetUnscaledRange(range[0], range[1]);
    axis->AutoScale();
  }

  //q->chart()->RecalculatePlotTransforms();

  q->updateMRMLChartAxisRangeFromWidget();
}

// --------------------------------------------------------------------------
void qMRMLPlotViewPrivate::switchInteractionMode()
{
  Q_Q(qMRMLPlotView);

  if (!this->MRMLPlotViewNode)
    {
    return;
    }
  int interactionMode = this->MRMLPlotViewNode->GetInteractionMode();
  interactionMode = (interactionMode + 1) % vtkMRMLPlotViewNode::InteractionMode_Last;
  this->MRMLPlotViewNode->SetInteractionMode(interactionMode);
}

// --------------------------------------------------------------------------
void qMRMLPlotViewPrivate::emitSelection()
{
  Q_Q(qMRMLPlotView);

  if (!q->chart())
    {
    return;
    }

  const char *PlotChartNodeID = this->MRMLPlotViewNode->GetPlotChartNodeID();

  vtkMRMLPlotChartNode* plotChartNode = vtkMRMLPlotChartNode::SafeDownCast
    (this->MRMLScene->GetNodeByID(PlotChartNodeID));
  if (!plotChartNode)
    {
    return;
    }

  vtkNew<vtkStringArray> mrmlPlotSeriesIDs;
  vtkNew<vtkCollection> selectionCol;

  for (int plotIndex = 0; plotIndex < q->chart()->GetNumberOfPlots(); plotIndex++)
    {
    vtkPlot *plot = q->chart()->GetPlot(plotIndex);
    if (!plot)
      {
      continue;
      }
    vtkIdTypeArray *selection = plot->GetSelection();
    if (!selection)
      {
      continue;
      }

    if (selection->GetNumberOfValues() > 0)
      {
      selectionCol->AddItem(selection);
      vtkMRMLPlotSeriesNode* plotSeriesNode = this->plotSeriesNodeFromPlot(plot);
      if (plotSeriesNode)
        {
        // valid plot data node found
        mrmlPlotSeriesIDs->InsertNextValue(plotSeriesNode->GetID());
        }
      }
    }

  // Repaint the chart scene
  q->scene()->SetDirty(true);
  q->update();

  // emit the signal
  emit q->dataSelected(mrmlPlotSeriesIDs.GetPointer(), selectionCol.GetPointer());
}

// --------------------------------------------------------------------------
void qMRMLPlotViewPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLPlotView);

  if (!this->MRMLScene || !this->MRMLPlotViewNode
    || !q->isEnabled() || !q->chart() || !q->chart()->GetLegend())
    {
    return;
    }

  if (this->UpdatingWidgetFromMRML)
    {
    return;
    }
  this->UpdatingWidgetFromMRML = true;

  // Set interaction mode
  int interactionMode = this->MRMLPlotViewNode->GetInteractionMode();
  switch (interactionMode)
  {
  case vtkMRMLPlotViewNode::InteractionModePanView:
    q->chart()->SetClickActionToButton(vtkChart::SELECT, vtkContextMouseEvent::LEFT_BUTTON);
    q->chart()->SetActionToButton(vtkChart::PAN, vtkContextMouseEvent::LEFT_BUTTON);
    q->chart()->SetActionToButton(vtkChart::ZOOM, vtkContextMouseEvent::MIDDLE_BUTTON);
    q->chart()->SetActionToButton(vtkChart::ZOOM_AXIS, vtkContextMouseEvent::RIGHT_BUTTON);
    break;
  case vtkMRMLPlotViewNode::InteractionModeSelectPoints:
    q->chart()->SetClickActionToButton(vtkChart::SELECT, vtkContextMouseEvent::LEFT_BUTTON);
    q->chart()->SetActionToButton(vtkChart::SELECT, vtkContextMouseEvent::LEFT_BUTTON);
    q->chart()->SetActionToButton(vtkChart::PAN, vtkContextMouseEvent::MIDDLE_BUTTON);
    q->chart()->SetActionToButton(vtkChart::ZOOM_AXIS, vtkContextMouseEvent::RIGHT_BUTTON);
    break;
  case vtkMRMLPlotViewNode::InteractionModeFreehandSelectPoints:
    q->chart()->SetClickActionToButton(vtkChart::SELECT, vtkContextMouseEvent::LEFT_BUTTON);
    q->chart()->SetActionToButton(vtkChart::SELECT_POLYGON, vtkContextMouseEvent::LEFT_BUTTON);
    q->chart()->SetActionToButton(vtkChart::PAN, vtkContextMouseEvent::MIDDLE_BUTTON);
    q->chart()->SetActionToButton(vtkChart::ZOOM_AXIS, vtkContextMouseEvent::RIGHT_BUTTON);
    break;
  case vtkMRMLPlotViewNode::InteractionModeMovePoints:
    q->chart()->SetClickActionToButton(vtkChart::SELECT, vtkContextMouseEvent::NO_BUTTON);
    q->chart()->SetActionToButton(vtkChart::CLICK_AND_DRAG, vtkContextMouseEvent::LEFT_BUTTON);
    q->chart()->SetActionToButton(vtkChart::PAN, vtkContextMouseEvent::MIDDLE_BUTTON);
    q->chart()->SetActionToButton(vtkChart::ZOOM_AXIS, vtkContextMouseEvent::RIGHT_BUTTON);
    break;
  }

  // Get the PlotChartNode
  const char *plotChartNodeID = this->MRMLPlotViewNode->GetPlotChartNodeID();
  vtkMRMLPlotChartNode* plotChartNode = vtkMRMLPlotChartNode::SafeDownCast(this->MRMLScene->GetNodeByID(plotChartNodeID));
  if (!plotChartNode)
    {
    // Clean all the plots in vtkChartXY
    while(q->chart()->GetNumberOfPlots() > 0)
      {
      // This if is necessary for a BUG at VTK level:
      // in the case of a plot removed with corner ID 0,
      // when successively the addPlot method is called
      // (to add the same plot instance to vtkChartXY) it will
      // fail to setup the graph in the vtkChartXY render.
      if (q->chart()->GetPlotCorner(q->chart()->GetPlot(0)) == 0)
        {
        q->chart()->SetPlotCorner(q->chart()->GetPlot(0), 1);
        }
      q->removePlot(q->chart()->GetPlot(0));
      }
    this->MapPlotToPlotSeriesNodeID.clear();
    this->UpdatingWidgetFromMRML = false;
    return;
    }

  // Enable moving of data points by drag-and-drop if point moving is enabled
  // both in the plot chart and view nodes.
  q->chart()->SetDragPointAlongX(this->MRMLPlotViewNode->GetEnablePointMoveAlongX()
    && plotChartNode->GetEnablePointMoveAlongX());
  q->chart()->SetDragPointAlongY(this->MRMLPlotViewNode->GetEnablePointMoveAlongY()
    && plotChartNode->GetEnablePointMoveAlongY());

  vtkSmartPointer<vtkCollection> allPlotSeriesNodesInScene = vtkSmartPointer<vtkCollection>::Take
    (this->mrmlScene()->GetNodesByClass("vtkMRMLPlotSeriesNode"));

  std::vector<std::string> plotSeriesNodesIDs;
  plotChartNode->GetPlotSeriesNodeIDs(plotSeriesNodesIDs);

  // Plot data nodes that should not be added to the chart
  // because they are already added or because they should not be added
  // (as not all necessary table data are available).
  std::set< vtkMRMLPlotSeriesNode* > plotSeriesNodesNotToAdd;

  // Remove plots from chart that are no longer needed or available
  for (int chartPlotSeriesNodesIndex = q->chart()->GetNumberOfPlots()-1; chartPlotSeriesNodesIndex >= 0; chartPlotSeriesNodesIndex--)
    {
    vtkPlot *plot = q->chart()->GetPlot(chartPlotSeriesNodesIndex);
    if (!plot)
      {
      continue;
      }
    // If it is nullptr then it means that there is no usable associated plot data node
    // and so the plot should be removed.
    vtkMRMLPlotSeriesNode* plotSeriesNode = this->plotSeriesNodeFromPlot(plot);
    if (plotSeriesNode != nullptr)
      {
      plotSeriesNodesNotToAdd.insert(plotSeriesNode);
      if (std::find(plotSeriesNodesIDs.begin(), plotSeriesNodesIDs.end(),
        plotSeriesNode->GetID()) == plotSeriesNodesIDs.end())
        {
        // plot data node is no longer associated with this chart
        plotSeriesNode = nullptr;
        }
      }

    bool deletePlot = true;
    if (plotSeriesNode)
      {
      vtkSmartPointer<vtkPlot> newPlot = this->updatePlotFromPlotSeriesNode(plotSeriesNode, plot);
      if (newPlot == plot)
        {
        // keep current plot
        deletePlot = false;
        }
      else
        {
        this->MapPlotToPlotSeriesNodeID[plot] = plotSeriesNode->GetID();
        q->addPlot(newPlot);
        }
      }

    if (deletePlot)
      {
      // This if is necessary for a BUG at VTK level:
      // in the case of a plot removed with corner ID 0,
      // when successively the addPlot method is called
      // (to add the same plot instance to vtkChartXY) it will
      // fail to setup the graph in the vtkChartXY render.
      if (q->chart()->GetPlotCorner(plot) == 0)
        {
        q->chart()->SetPlotCorner(plot, 1);
        }

      q->removePlot(plot);
      this->MapPlotToPlotSeriesNodeID.remove(plot);
      }
    }

  // Add missing plots to the chart
  for (std::vector<std::string>::iterator it = plotSeriesNodesIDs.begin(); it != plotSeriesNodesIDs.end(); ++it)
    {
    vtkMRMLPlotSeriesNode* plotSeriesNode = vtkMRMLPlotSeriesNode::SafeDownCast(this->mrmlScene()->GetNodeByID(it->c_str()));
    if (!plotSeriesNode || plotSeriesNodesNotToAdd.find(plotSeriesNode) != plotSeriesNodesNotToAdd.end())
      {
      // node is invalid or need not to be added
      continue;
      }
    vtkSmartPointer<vtkPlot> newPlot = this->updatePlotFromPlotSeriesNode(plotSeriesNode, nullptr);
    if (!newPlot)
      {
      continue;
      }
    this->MapPlotToPlotSeriesNodeID[newPlot] = plotSeriesNode->GetID();
    q->addPlot(newPlot);
    }

  int fontTypeIndex = q->chart()->GetTitleProperties()->GetFontFamilyFromString(plotChartNode->GetFontType() ? plotChartNode->GetFontType() : "Arial");

  // Setting Title
  if (plotChartNode->GetTitleVisibility())
    {
    q->chart()->SetTitle(plotChartNode->GetTitle() ? plotChartNode->GetTitle() : "");
    }
  else
    {
    q->chart()->SetTitle("");
    }
  q->chart()->GetTitleProperties()->SetFontFamily(fontTypeIndex);
  q->chart()->GetTitleProperties()->SetFontSize(plotChartNode->GetTitleFontSize());

  // Setting Legend
  q->chart()->SetShowLegend(plotChartNode->GetLegendVisibility());
  q->chart()->GetLegend()->GetLabelProperties()->SetFontSize(plotChartNode->GetLegendFontSize());
  q->chart()->GetLegend()->GetLabelProperties()->SetFontFamily(fontTypeIndex);

  // Setting Axes
  const unsigned int numberOfAxisIDs = 4;
  int axisIDs[numberOfAxisIDs] = { vtkAxis::BOTTOM, vtkAxis::TOP, vtkAxis::LEFT, vtkAxis::RIGHT };
  for (unsigned int axisIndex = 0; axisIndex < numberOfAxisIDs; ++axisIndex)
    {
    int axisID = axisIDs[axisIndex];
    vtkAxis *axis = q->chart()->GetAxis(axisID);
    if (!axis)
      {
      continue;
      }
    // Assuming the the Top and Bottom axes are the "X" axis
    bool mrmlLogScale = false;
    bool mrmlRangeAuto = true;
    double mrmlRange[2] = { 0.0, 1.0 };
    if (axisID == vtkAxis::BOTTOM || axisID == vtkAxis::TOP)
      {
      if (plotChartNode->GetXAxisTitleVisibility())
        {
        axis->SetTitle(plotChartNode->GetXAxisTitle() ? plotChartNode->GetXAxisTitle() : "");
        }
      else
        {
        axis->SetTitle("");
        }
      mrmlLogScale = plotChartNode->GetXAxisLogScale();
      mrmlRangeAuto = plotChartNode->GetXAxisRangeAuto();
      plotChartNode->GetXAxisRange(mrmlRange);
      }
    else // (axisID == vtkAxis::LEFT || axisID == vtkAxis::RIGHT)
      {
      if (plotChartNode->GetYAxisTitleVisibility())
        {
        axis->SetTitle(plotChartNode->GetYAxisTitle() ? plotChartNode->GetYAxisTitle() : "");
        }
      else
        {
        axis->SetTitle("");
        }
      mrmlLogScale = plotChartNode->GetYAxisLogScale();
      mrmlRangeAuto = plotChartNode->GetYAxisRangeAuto();
      plotChartNode->GetYAxisRange(mrmlRange);
      }

    if (mrmlRangeAuto)
      {
      // Automatic range
      if (axis->GetLogScale() != mrmlLogScale)
        {
        double unscaledRange[2] = { 0.0, 1.0 };
        double limit[2] = { -vtkMath::Inf(), vtkMath::Inf() };
        axis->GetUnscaledRange(unscaledRange);
        axis->SetLogScale(mrmlLogScale);
        if (mrmlLogScale)
          {
          this->adjustRangeForLogScale(unscaledRange, limit);
          }
        axis->SetUnscaledMinimumLimit(limit[0]);
        axis->SetUnscaledMaximumLimit(limit[1]);
        axis->SetUnscaledRange(unscaledRange);
        axis->Update();
        }
      axis->SetBehavior(vtkAxis::AUTO);
      }
    else
      {
      // Manual range
      axis->SetBehavior(vtkAxis::FIXED);
      axis->SetLogScale(mrmlLogScale);
      double limit[2] = { -vtkMath::Inf(), vtkMath::Inf() };
      if (mrmlLogScale)
        {
        this->adjustRangeForLogScale(mrmlRange, limit);
        }
      axis->SetUnscaledMinimumLimit(limit[0]);
      axis->SetUnscaledMaximumLimit(limit[1]);
      axis->SetUnscaledRange(mrmlRange);
      axis->Update();
      }

    axis->SetGridVisible(plotChartNode->GetGridVisibility());
    axis->GetTitleProperties()->SetFontFamily(fontTypeIndex);
    axis->GetTitleProperties()->SetFontSize(plotChartNode->GetAxisTitleFontSize());
    axis->GetLabelProperties()->SetFontFamily(fontTypeIndex);
    axis->GetLabelProperties()->SetFontSize(plotChartNode->GetAxisLabelFontSize());
    }

  q->scene()->SetDirty(true);
  this->UpdatingWidgetFromMRML = false;
}

// --------------------------------------------------------------------------
// qMRMLPlotView methods

// --------------------------------------------------------------------------
qMRMLPlotView::qMRMLPlotView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLPlotViewPrivate(*this))
{
  Q_D(qMRMLPlotView);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLPlotView::~qMRMLPlotView()
{
  this->setMRMLScene(nullptr);
}


//------------------------------------------------------------------------------
void qMRMLPlotView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLPlotView);
  if (newScene == d->MRMLScene)
    {
    return;
    }

  d->setMRMLScene(newScene);

  if (d->MRMLPlotViewNode && newScene != d->MRMLPlotViewNode->GetScene())
    {
    this->setMRMLPlotViewNode(nullptr);
    }

  emit mrmlSceneChanged(newScene);
}

//---------------------------------------------------------------------------
void qMRMLPlotView::setMRMLPlotViewNode(vtkMRMLPlotViewNode* newPlotViewNode)
{
  Q_D(qMRMLPlotView);
  if (d->MRMLPlotViewNode == newPlotViewNode)
    {
    return;
    }

  // connect modified event on PlotViewNode to updating the widget
  d->qvtkReconnect(d->MRMLPlotViewNode, newPlotViewNode,
    vtkMRMLPlotViewNode::PlotChartNodeChangedEvent, d, SLOT(updateWidgetFromMRML()));

  // update when plot view interaction mode is changed
  d->qvtkReconnect(d->MRMLPlotViewNode, newPlotViewNode,
    vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromMRML()));

  // connect on PlotSeriesNodeChangedEvent (e.g. PlotView is looking at a
  // different PlotSeriesNode
  d->qvtkReconnect(d->MRMLPlotViewNode, newPlotViewNode,
    vtkMRMLPlotViewNode::PlotChartNodeChangedEvent, d, SLOT(onPlotChartNodeChanged()));

  // cache the PlotViewNode
  d->MRMLPlotViewNode = newPlotViewNode;

  // ... and connect modified event on the PlotViewNode's PlotChartNode
  // to update the widget
  d->onPlotChartNodeChanged();

  // make sure the gui is up to date
  d->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
vtkMRMLPlotViewNode* qMRMLPlotView::mrmlPlotViewNode()const
{
  Q_D(const qMRMLPlotView);
  return d->MRMLPlotViewNode;
}

//---------------------------------------------------------------------------
vtkMRMLScene* qMRMLPlotView::mrmlScene()const
{
  Q_D(const qMRMLPlotView);
  return d->MRMLScene;
}

//---------------------------------------------------------------------------
QSize qMRMLPlotView::sizeHint()const
{
  // return a default size hint (invalid size)
  return QSize();
}

// --------------------------------------------------------------------------
void qMRMLPlotView::keyPressEvent(QKeyEvent *event)
{
  Q_D(qMRMLPlotView);
  this->Superclass::keyPressEvent(event);

  if (event->key() == Qt::Key_S)
    {
    d->switchInteractionMode();
    }
  if (event->key() == Qt::Key_R)
    {
    d->RecalculateBounds();
    }
  if (event->key() == Qt::Key_U)
    {
    this->RemovePlotSelections();
    }
}

// --------------------------------------------------------------------------
void qMRMLPlotView::keyReleaseEvent(QKeyEvent *event)
{
  Q_D(qMRMLPlotView);
  this->Superclass::keyPressEvent(event);
}

// --------------------------------------------------------------------------
void qMRMLPlotView::fitToContent()
{
  Q_D(qMRMLPlotView);
  d->RecalculateBounds();
  // Repaint the chart scene
  this->scene()->SetDirty(true);
  this->update();
}

// --------------------------------------------------------------------------
void qMRMLPlotView::RemovePlotSelections()
{
  if (!this->chart())
    {
    return;
    }
#if VTK_MAJOR_VERSION <= 7 || (VTK_MAJOR_VERSION <= 8 && VTK_MINOR_VERSION <= 1)
  vtkWarningWithObjectMacro(this->chart(), "Removing plot selections not available in this version of VTK");
#else
  this->chart()->RemovePlotSelections();
#endif
}

// --------------------------------------------------------------------------
void qMRMLPlotView::updateMRMLChartAxisRangeFromWidget()
{
  Q_D(qMRMLPlotView);
  if (!d->MRMLPlotChartNode)
    {
    return;
    }
  if (d->UpdatingWidgetFromMRML)
    {
    return;
    }
  int wasModified = d->MRMLPlotChartNode->StartModify();
  // Setting Axes
  vtkAxis *bottomAxis = this->chart()->GetAxis(vtkAxis::BOTTOM);
  if (bottomAxis)
    {
    double range[2] = { 0, 1 };
    bottomAxis->GetUnscaledRange(range);
    d->MRMLPlotChartNode->SetXAxisRange(range);
    }
  vtkAxis *leftAxis = this->chart()->GetAxis(vtkAxis::LEFT);
  if (leftAxis)
    {
    double range[2] = { 0, 1 };
    leftAxis->GetUnscaledRange(range);
    d->MRMLPlotChartNode->SetYAxisRange(range);
    }
  d->MRMLPlotChartNode->EndModify(wasModified);
}

// ----------------------------------------------------------------------------
void qMRMLPlotView::saveAsSVG(const QString &filePathPrefix)
{
  vtkNew<vtkGL2PSExporter> exporter;

  exporter->SetFileFormatToSVG();
  exporter->CompressOff();
  exporter->SetRenderWindow(this->GetRenderWindow());
  exporter->SetFilePrefix(filePathPrefix.toStdString().c_str());
  exporter->Update();
}
