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
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLPlotView_p.h"

// MRML includes
#include <vtkMRMLPlotDataNode.h>
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLPlotViewNode.h>
#include <vtkMRMLColorLogic.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTableNode.h>

// VTK includes
#include <vtkAxis.h>
#include <vtkBrush.h>
#include <vtkChartLegend.h>
#include <vtkChartXY.h>
#include <vtkCollection.h>
#include <vtkColorSeries.h>
#include <vtkContextMouseEvent.h>
#include <vtkContextScene.h>
#include <vtkContextView.h>
#include <vtkNew.h>
#include <vtkPlot.h>
#include <vtkRenderer.h>
#include <vtkSelection.h>
#include <vtkStringArray.h>
#include <vtkTextProperty.h>

//--------------------------------------------------------------------------
// qMRMLPlotViewPrivate methods

//---------------------------------------------------------------------------
qMRMLPlotViewPrivate::qMRMLPlotViewPrivate(qMRMLPlotView& object)
  : q_ptr(&object)
{
  this->MRMLScene = 0;
  this->MRMLPlotViewNode = 0;
  this->MRMLPlotChartNode = 0;
  this->ColorLogic = 0;
  this->PinButton = 0;
  this->PopupWidget = 0;
}

//---------------------------------------------------------------------------
qMRMLPlotViewPrivate::~qMRMLPlotViewPrivate()
{
}

namespace
{
//----------------------------------------------------------------------------
template <typename T> T StringToNumber(const char* num)
{
  std::stringstream ss;
  ss << num;
  T result;
  return ss >> result ? result : 0;
}

//----------------------------------------------------------------------------
int StringToInt(const char* str)
{
  return StringToNumber<int>(str);
}

}// end namespace

//---------------------------------------------------------------------------
void qMRMLPlotViewPrivate::init()
{
  Q_Q(qMRMLPlotView);

  // Let the QWebView expand in both directions
  q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  this->PopupWidget = new ctkPopupWidget;
  QHBoxLayout* popupLayout = new QHBoxLayout;
  popupLayout->addWidget(new QToolButton);
  this->PopupWidget->setLayout(popupLayout);

  if (!q->chart())
    {
    return;
    }

  q->chart()->SetActionToButton(vtkChart::SELECT, vtkContextMouseEvent::LEFT_BUTTON);
  q->chart()->SetActionToButton(vtkChart::PAN, vtkContextMouseEvent::MIDDLE_BUTTON);
  q->chart()->SetActionToButton(vtkChart::ZOOM, vtkContextMouseEvent::RIGHT_BUTTON);

  qvtkConnect(q->chart(), vtkCommand::SelectionChangedEvent, this, SLOT(emitSelection()));

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
  Q_Q(qMRMLPlotView);
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
  Q_Q(qMRMLPlotView);
  vtkMRMLPlotChartNode *newPlotChartNode = NULL;

  if (this->MRMLPlotViewNode && this->MRMLPlotViewNode->GetPlotChartNodeID())
    {
    newPlotChartNode = vtkMRMLPlotChartNode::SafeDownCast
      (this->MRMLScene->GetNodeByID(this->MRMLPlotViewNode->GetPlotChartNodeID()));
    }

  this->qvtkReconnect(this->MRMLPlotChartNode, newPlotChartNode,
    vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  this->MRMLPlotChartNode = newPlotChartNode;
}

// --------------------------------------------------------------------------
void qMRMLPlotViewPrivate::RecalculateBounds()
{
  Q_Q(qMRMLPlotView);

  if (!q->chart())
    {
    return;
    }

  q->chart()->RecalculateBounds();
}

// --------------------------------------------------------------------------
void qMRMLPlotViewPrivate::switchSelectionMode()
{
  Q_Q(qMRMLPlotView);

  if (!q->chart())
    {
    return;
    }

  int buttonSelect, buttonSelectPoly, buttonSelectClickAndDrag;
  buttonSelect = q->chart()->GetActionToButton(vtkChart::SELECT);
  buttonSelectPoly = q->chart()->GetActionToButton(vtkChart::SELECT_POLYGON);
  buttonSelectClickAndDrag = q->chart()->GetActionToButton(vtkChart::CLICK_AND_DRAG);
  if (buttonSelect > 0 && buttonSelectPoly < 0 && buttonSelectClickAndDrag < 0)
    {
    q->chart()->SetActionToButton(vtkChart::SELECT_POLYGON, vtkContextMouseEvent::LEFT_BUTTON);
    }
  else if (buttonSelect < 0 && buttonSelectPoly > 0 && buttonSelectClickAndDrag < 0)
    {
    q->chart()->SetActionToButton(vtkChart::CLICK_AND_DRAG, vtkContextMouseEvent::LEFT_BUTTON);
    }
  else if (buttonSelect < 0 && buttonSelectPoly < 0 && buttonSelectClickAndDrag > 0)
    {
    q->chart()->SetActionToButton(vtkChart::SELECT, vtkContextMouseEvent::LEFT_BUTTON);
    }
  else if (buttonSelectClickAndDrag < 0 && buttonSelectPoly < 0 && buttonSelect < 0)
    {
    q->chart()->SetActionToButton(vtkChart::SELECT, vtkContextMouseEvent::LEFT_BUTTON);
  }
}

// --------------------------------------------------------------------------
void qMRMLPlotViewPrivate::switchLeftAndMiddleClick()
{
  Q_Q(qMRMLPlotView);

  if (!q->chart())
    {
    return;
    }

  int buttonPan, buttonSelect, buttonSelectPoly, buttonSelectClickAndDrag;
  buttonPan = q->chart()->GetActionToButton(vtkChart::PAN);
  buttonSelect = q->chart()->GetActionToButton(vtkChart::SELECT);
  buttonSelectPoly = q->chart()->GetActionToButton(vtkChart::SELECT_POLYGON);
  buttonSelectClickAndDrag = q->chart()->GetActionToButton(vtkChart::CLICK_AND_DRAG);

  if (buttonPan == 2)
    {
    q->chart()->SetActionToButton(vtkChart::PAN, vtkContextMouseEvent::LEFT_BUTTON);
    if (buttonSelect == 1)
      {
      q->chart()->SetActionToButton(vtkChart::SELECT, vtkContextMouseEvent::MIDDLE_BUTTON);
      }
    else if (buttonSelectPoly == 1)
      {
      q->chart()->SetActionToButton(vtkChart::SELECT_POLYGON, vtkContextMouseEvent::MIDDLE_BUTTON);
      }
    else if (buttonSelectClickAndDrag == 1)
      {
      q->chart()->SetActionToButton(vtkChart::CLICK_AND_DRAG, vtkContextMouseEvent::MIDDLE_BUTTON);
      }
    }
  else if (buttonPan == 1)
    {
    q->chart()->SetActionToButton(vtkChart::PAN, vtkContextMouseEvent::MIDDLE_BUTTON);
    if (buttonSelect == 2)
      {
      q->chart()->SetActionToButton(vtkChart::SELECT, vtkContextMouseEvent::LEFT_BUTTON);
      }
    else if (buttonSelectPoly == 2)
      {
      q->chart()->SetActionToButton(vtkChart::SELECT_POLYGON, vtkContextMouseEvent::LEFT_BUTTON);
      }
    else if (buttonSelectClickAndDrag == 2)
      {
      q->chart()->SetActionToButton(vtkChart::CLICK_AND_DRAG, vtkContextMouseEvent::LEFT_BUTTON);
      }
    }
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

  vtkMRMLPlotChartNode* pln = vtkMRMLPlotChartNode::SafeDownCast
    (this->MRMLScene->GetNodeByID(PlotChartNodeID));
  if (!pln)
    {
    return;
    }

  vtkNew<vtkStringArray> mrmlPlotDataIDs;
  vtkNew<vtkCollection> selectionCol;

  for (int plotIndex = 0; plotIndex < q->chart()->GetNumberOfPlots(); plotIndex++)
    {
    vtkPlot *Plot = q->chart()->GetPlot(plotIndex);
    if (!Plot)
      {
      continue;
      }
    vtkIdTypeArray *Selection = Plot->GetSelection();
    if (!Selection)
      {
      continue;
      }

    if (Selection->GetNumberOfValues() > 0)
      {
      selectionCol->AddItem(Selection);
      // get MRMLPlotDataNode from pln and find the Node with the same vtkPlot address
      int numPlotDataNodes = pln->GetNumberOfNodeReferences(pln->GetPlotDataNodeReferenceRole());
      for (int plotDataNodeIndex = 0; plotDataNodeIndex < numPlotDataNodes; plotDataNodeIndex++)
        {
        vtkMRMLPlotDataNode *PlotDataNode = pln->GetNthPlotDataNode(plotDataNodeIndex);
        if (!PlotDataNode)
          {
          continue;
          }
        if (PlotDataNode->GetPlot() == Plot)
          {
          mrmlPlotDataIDs->InsertNextValue(PlotDataNode->GetID());
          break;
          }
        }
      }
    }
  // emit the signal
  emit q->dataSelected(mrmlPlotDataIDs.GetPointer(), selectionCol.GetPointer());
}

// --------------------------------------------------------------------------
void qMRMLPlotViewPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLPlotView);

  if (!this->MRMLScene || !this->ColorLogic || !this->MRMLPlotViewNode
      || !q->isEnabled() || !q->chart() || !q->chart()->GetLegend())
    {
    return;
    }

  // Get the PlotChartNode
  const char *PlotChartNodeID = this->MRMLPlotViewNode->GetPlotChartNodeID();

  vtkMRMLPlotChartNode* pln = vtkMRMLPlotChartNode::SafeDownCast
    (this->MRMLScene->GetNodeByID(PlotChartNodeID));
  if (!pln)
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
    return;
    }

  const char* defaultPlotColorNodeID = this->ColorLogic->GetDefaultPlotColorNodeID();
  vtkMRMLColorNode *defaultColorNode = vtkMRMLColorNode::SafeDownCast
    (this->MRMLScene->GetNodeByID(defaultPlotColorNodeID));
  vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast
    (this->MRMLScene->GetNodeByID(pln->GetAttribute("LookupTable")));

  if (!colorNode)
    {
    colorNode = defaultColorNode;
    }

  if (!colorNode)
    {
    return;
    }

  vtkSmartPointer<vtkCollection> plotDataNodes = vtkSmartPointer<vtkCollection>::Take
    (this->mrmlScene()->GetNodesByClass("vtkMRMLPlotDataNode"));

  std::vector<std::string> plotDataNodesIDs;
  pln->GetPlotIDs(plotDataNodesIDs);

  for(int chartPlotDataNodesIndex = 0; chartPlotDataNodesIndex < q->chart()->GetNumberOfPlots(); chartPlotDataNodesIndex++)
    {
    bool plotFound = false;
    for(int plotDataNodesIndex = 0; plotDataNodesIndex < plotDataNodes->GetNumberOfItems(); plotDataNodesIndex++)
      {
      vtkMRMLPlotDataNode* plotDataNode = vtkMRMLPlotDataNode::SafeDownCast
        (plotDataNodes->GetItemAsObject(plotDataNodesIndex));
      if (!plotDataNode)
        {
        continue;
        }
      if (q->chart()->GetPlot(chartPlotDataNodesIndex) == plotDataNode->GetPlot())
        {
        plotFound = true;
        break;
        }
      }
    if (!plotFound)
      {
      q->removePlot(q->chart()->GetPlot(chartPlotDataNodesIndex));
      }
    }

  for(int plotDataNodesIndex = 0; plotDataNodesIndex < plotDataNodes->GetNumberOfItems(); plotDataNodesIndex++)
    {
    vtkMRMLPlotDataNode* plotDataNode = vtkMRMLPlotDataNode::SafeDownCast
      (plotDataNodes->GetItemAsObject(plotDataNodesIndex));
    if (!plotDataNode)
      {
      continue;
      }

    bool plotFound = false;
    std::vector<std::string>::iterator it = plotDataNodesIDs.begin();
    for (; it != plotDataNodesIDs.end(); ++it)
      {
      if ((*it).compare(plotDataNode->GetID()))
        {
        continue;
        }
      vtkPlot* plot = plotDataNode->GetPlot();
      if (!plot)
        {
        continue;
        }

      // Set color of the plot
      double color[4] = {0., 0., 0., 0.};

      vtkIdType plotIndex = pln->GetColorPlotIdexFromID(plotDataNode->GetID());
      if (plotIndex < 0)
        {
        plotIndex = 0;
        }
      colorNode->GetColor(plotIndex , color);
      plot->SetColor(color[0], color[1], color[2]);

      // Add plot if not already in chart
      if (q->plotIndex(plot) < vtkIdType(0))
        {
        q->addPlot(plot);
        }
      plotFound = true;
      break;
      }

    bool columnXFound = false;
    bool columnYFound = false;
    for (int columnIndex = 0; columnIndex < plotDataNode->GetTableNode()->GetNumberOfColumns(); columnIndex++)
      {
      if(!strcmp(plotDataNode->GetTableNode()->GetColumnName(columnIndex).c_str(), plotDataNode->GetXColumnName().c_str()))
        {
        columnXFound = true;
        }
      if(!strcmp(plotDataNode->GetTableNode()->GetColumnName(columnIndex).c_str(), plotDataNode->GetYColumnName().c_str()))
        {
        columnYFound = true;
        }
      }

    if (!plotFound || !columnXFound || !columnYFound)
      {
      // This if is necessary for a BUG at VTK level:
      // in the case of a plot removed with corner ID 0,
      // when successively the addPlot method is called
      // (to add the same plot instance to vtkChartXY) it will
      // fail to setup the graph in the vtkChartXY render.
      if (q->chart()->GetPlotCorner(plotDataNode->GetPlot()) == 0)
        {
        q->chart()->SetPlotCorner(plotDataNode->GetPlot(), 1);
        }
      q->removePlot(plotDataNode->GetPlot());
      }
    }

  // Setting Title
  if (!strcmp(pln->GetAttribute("ShowTitle"), "on"))
    {
    q->chart()->SetTitle(pln->GetAttribute("TitleName"));
    }
  else
    {
    q->chart()->SetTitle("");
    }

  // Setting Legend
  if (!strcmp(pln->GetAttribute("ShowLegend"), "on"))
    {
    q->chart()->SetShowLegend(true);
    }
  else
    {
    q->chart()->SetShowLegend(false);
    }

  // Setting Title and Legend Properties
  int VTKFont = q->chart()->GetTitleProperties()->GetFontFamilyFromString(pln->GetAttribute("FontType"));

  q->chart()->GetTitleProperties()->SetFontFamily(VTKFont);
  q->chart()->GetTitleProperties()->SetFontSize(StringToInt(pln->GetAttribute("TitleFontSize")));
  q->chart()->GetLegend()->GetLabelProperties()->SetFontFamily(VTKFont);

  // Setting ClickAndDrag action draggable along X and Y axes
  if (!strcmp(pln->GetAttribute("ClickAndDragAlongX"), "on"))
    {
    q->chart()->SetDragPointAlongX(true);
    }
  else
    {
    q->chart()->SetDragPointAlongX(false);
    }

  if (!strcmp(pln->GetAttribute("ClickAndDragAlongY"), "on"))
    {
    q->chart()->SetDragPointAlongY(true);
    }
  else
    {
    q->chart()->SetDragPointAlongY(false);
    }

  // Setting Axes
  // Assuming the the Top and Bottom axes are the "X" axis
  vtkAxis *axis = q->chart()->GetAxis(vtkAxis::BOTTOM);
  if (axis)
    {
    if (!strcmp(pln->GetAttribute("ShowXAxisLabel"), "on"))
      {
      axis->SetTitle(pln->GetAttribute("XAxisLabelName"));
      }
    else
      {
      axis->SetTitle("");
      }

    if (!strcmp(pln->GetAttribute("ShowGrid"), "on"))
      {
      axis->SetGridVisible(true);
      }
    else
      {
      axis->SetGridVisible(false);
      }

    axis->GetTitleProperties()->SetFontFamily(VTKFont);
    axis->GetTitleProperties()->SetFontSize(StringToInt(pln->GetAttribute("AxisTitleFontSize")));
    axis->GetLabelProperties()->SetFontFamily(VTKFont);
    axis->GetLabelProperties()->SetFontSize(StringToInt(pln->GetAttribute("AxisLabelFontSize")));
    }

  axis = q->chart()->GetAxis(vtkAxis::TOP);
  if (axis)
    {
    if (!strcmp(pln->GetAttribute("ShowXAxisLabel"), "on"))
      {
      axis->SetTitle(pln->GetAttribute("XAxisLabelName"));
      }
    else
      {
      axis->SetTitle("");
      }

    if (!strcmp(pln->GetAttribute("ShowGrid"), "on"))
      {
      axis->SetGridVisible(true);
      }
    else
      {
      axis->SetGridVisible(false);
      }

    axis->GetTitleProperties()->SetFontFamily(VTKFont);
    axis->GetTitleProperties()->SetFontSize(StringToInt(pln->GetAttribute("AxisTitleFontSize")));
    axis->GetLabelProperties()->SetFontFamily(VTKFont);
    axis->GetLabelProperties()->SetFontSize(StringToInt(pln->GetAttribute("AxisLabelFontSize")));
    }

  // Assuming the Left and Right axis are the "Y" axis
  axis = q->chart()->GetAxis(vtkAxis::LEFT);
  if (axis)
    {
    if (!strcmp(pln->GetAttribute("ShowYAxisLabel"), "on"))
      {
      axis->SetTitle(pln->GetAttribute("YAxisLabelName"));
      }
    else
      {
      axis->SetTitle("");
      }

    if (!strcmp(pln->GetAttribute("ShowGrid"), "on"))
      {
      axis->SetGridVisible(true);
      }
    else
      {
      axis->SetGridVisible(false);
      }

    axis->GetTitleProperties()->SetFontFamily(VTKFont);
    axis->GetTitleProperties()->SetFontSize(StringToInt(pln->GetAttribute("AxisTitleFontSize")));
    axis->GetLabelProperties()->SetFontFamily(VTKFont);
    axis->GetLabelProperties()->SetFontSize(StringToInt(pln->GetAttribute("AxisLabelFontSize")));
    }

  axis = q->chart()->GetAxis(vtkAxis::RIGHT);
  if (axis)
    {
    if (!strcmp(pln->GetAttribute("ShowYAxisLabel"), "on"))
      {
      axis->SetTitle(pln->GetAttribute("YAxisLabelName"));
      }
    else
      {
      axis->SetTitle("");
      }

    if (!strcmp(pln->GetAttribute("ShowGrid"), "on"))
      {
      axis->SetGridVisible(true);
      }
    else
      {
      axis->SetGridVisible(false);
      }

    axis->GetTitleProperties()->SetFontFamily(VTKFont);
    axis->GetTitleProperties()->SetFontSize(StringToInt(pln->GetAttribute("AxisTitleFontSize")));
    axis->GetLabelProperties()->SetFontFamily(VTKFont);
    axis->GetLabelProperties()->SetFontSize(StringToInt(pln->GetAttribute("AxisLabelFontSize")));
    }

  // Repaint the chart scene
  q->scene()->SetDirty(true);
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
  this->setMRMLScene(0);
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
    this->setMRMLPlotViewNode(0);
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

  // connect on PlotDataNodeChangedEvent (e.g. PlotView is looking at a
  // different PlotDataNode
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
void qMRMLPlotView::setColorLogic(vtkMRMLColorLogic *colorLogic)
{
  Q_D(qMRMLPlotView);
  d->ColorLogic = colorLogic;
}

//---------------------------------------------------------------------------
vtkMRMLColorLogic *qMRMLPlotView::colorLogic() const
{
  Q_D(const qMRMLPlotView);
  return d->ColorLogic;
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
    d->switchSelectionMode();
    }
  if (event->key() == Qt::Key_R)
    {
    d->RecalculateBounds();
    }
  if (event->key() == Qt::Key_Shift)
    {
    d->switchLeftAndMiddleClick();
    }
}

// --------------------------------------------------------------------------
void qMRMLPlotView::keyReleaseEvent(QKeyEvent *event)
{
  Q_D(qMRMLPlotView);
  this->Superclass::keyPressEvent(event);

  if (event->key() == Qt::Key_Shift)
    {
    d->switchLeftAndMiddleClick();
    }
}
