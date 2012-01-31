/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QToolButton>

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkLogger.h>
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLChartView_p.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLChartViewDisplayableManagerFactory.h>
#include <vtkChartViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLDoubleArrayNode.h>
#include <vtkMRMLChartNode.h>
#include <vtkMRMLChartViewNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLChartView");
//--------------------------------------------------------------------------

const char *examplePlot = 
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<title>Line Charts and Options</title>"
  "<link class=\"include\" rel=\"stylesheet\" type=\"text/css\" href=\"qrc:/jqPlot/jquery.jqplot.min.css\" />"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/jquery.min.js\"></script>"
  "</head>"
  "<body>"
  "<div id=\"chart1\"></div>"
  "<script class=\"code\" type=\"text/javascript\">"
  "$(document).ready(function(){"
  "var plot1 = $.jqplot ('chart1', [[3,7,9,1,5,3,8,2,5]],"
  "{title: 'An example plot', highlighter: {show: true}, cursor: {show: false}}"
  ");"
  "});"
  "</script>"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/jquery.jqplot.min.js\"></script>"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.canvasTextRenderer.min.js\"></script>"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.canvasAxisLabelRenderer.min.js\"></script>"
  "<script type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.highlighter.min.js\"></script>"
  "<script type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.cursor.min.js\"></script>"
  "</body>"
  "</html>";

const char *plotPreamble = 
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<title>Line Charts and Options</title>"
  "<link class=\"include\" rel=\"stylesheet\" type=\"text/css\" href=\"qrc:/jqPlot/jquery.jqplot.min.css\" />"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/jquery.min.js\"></script>"
  "</head>"
  "<body>";

const char *plotPostscript =
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/jquery.jqplot.min.js\"></script>"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.canvasTextRenderer.min.js\"></script>"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.barRenderer.min.js\"></script>"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.canvasAxisLabelRenderer.min.js\"></script>"
  "<script type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.highlighter.min.js\"></script>"
  "<script type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.cursor.min.js\"></script>"
  "</body>"
  "</html>";



//--------------------------------------------------------------------------
// qMRMLChartViewPrivate methods

//---------------------------------------------------------------------------
qMRMLChartViewPrivate::qMRMLChartViewPrivate(qMRMLChartView& object)
  : q_ptr(&object)
{
  this->DisplayableManagerGroup = 0;
  this->MRMLScene = 0;
  this->MRMLChartViewNode = 0;
  this->MRMLChartNode = 0;
  this->PinButton = 0;
  this->PopupWidget = 0;
}

//---------------------------------------------------------------------------
qMRMLChartViewPrivate::~qMRMLChartViewPrivate()
{
  if (this->DisplayableManagerGroup)
    {
    this->DisplayableManagerGroup->Delete();
    }
}

//---------------------------------------------------------------------------
void qMRMLChartViewPrivate::init()
{
  Q_Q(qMRMLChartView);
//  q->setRenderEnabled(this->MRMLScene != 0);
  q->setEnabled(this->MRMLScene != 0);

  this->PopupWidget = new ctkPopupWidget;
  QHBoxLayout* popupLayout = new QHBoxLayout;
  popupLayout->addWidget(new QToolButton);
  this->PopupWidget->setLayout(popupLayout);

  // VTK_CREATE(vtkChartViewInteractorStyle, interactorStyle);
  // q->interactor()->SetInteractorStyle(interactorStyle);

//  this->initDisplayableManagers();

//  q->setHtml(examplePlot); 
  q->setHtml("");
  q->show();
}

//---------------------------------------------------------------------------
void qMRMLChartViewPrivate::initDisplayableManagers()
{
  // Q_Q(qMRMLChartView);
  // vtkMRMLChartViewDisplayableManagerFactory* factory
  //   = vtkMRMLChartViewDisplayableManagerFactory::GetInstance();

  // QStringList displayableManagers;
  // displayableManagers << "vtkMRMLChartViewDisplayableManager";
  //                     //<< "vtkMRMLCameraDisplayableManager"
  //                     //<< "vtkMRMLViewDisplayableManager"
  //                     //<< "vtkMRMLModelDisplayableManager";
  // foreach(const QString& displayableManager, displayableManagers)
  //   {
  //   if(!factory->IsDisplayableManagerRegistered(displayableManager.toLatin1()))
  //     {
  //     factory->RegisterDisplayableManager(displayableManager.toLatin1());
  //     }
  //   }

  // this->DisplayableManagerGroup
  //   = factory->InstantiateDisplayableManagers(q->renderer());
  // // Observe displayable manager group to catch RequestRender events
  // this->qvtkConnect(this->DisplayableManagerGroup, vtkCommand::UpdateEvent,
  //                   q, SLOT(scheduleRender()));
}

//---------------------------------------------------------------------------
void qMRMLChartViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qMRMLChartView);
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
//  q->setRenderEnabled(this->MRMLScene != 0);
  q->setEnabled(this->MRMLScene != 0);
}


// --------------------------------------------------------------------------
void qMRMLChartViewPrivate::startProcessing()
{
  logger.trace("startProcessing");
  Q_Q(qMRMLChartView);
//  q->setRenderEnabled(false);
  q->setEnabled(false);
}

//
// --------------------------------------------------------------------------
void qMRMLChartViewPrivate::endProcessing()
{
  logger.trace("endProcessing");
  Q_Q(qMRMLChartView);
//  q->setRenderEnabled(true);
  q->setEnabled(true);
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLChartViewPrivate::mrmlScene()
{
  return this->MRMLScene;
}

// --------------------------------------------------------------------------
void qMRMLChartViewPrivate::onChartNodeChanged()
{
  //qDebug() << "onChartNodeChanged()";

  vtkMRMLChartNode *newChartNode=0;

  if (this->MRMLChartViewNode && this->MRMLChartViewNode->GetChartNodeID())
    {
    newChartNode = vtkMRMLChartNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->MRMLChartViewNode->GetChartNodeID()));
    }

  this->qvtkReconnect(
    this->MRMLChartNode, newChartNode,
    vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  this->MRMLChartNode = newChartNode;
}

// --------------------------------------------------------------------------
void qMRMLChartViewPrivate::updateWidgetFromMRML()
{
  //qDebug() << "qMRMLChartViewPrivate::updateWidgetFromMRML()";

  Q_Q(qMRMLChartView);
  if (!this->MRMLScene || !this->MRMLChartViewNode)
    {
    return;
    }

  if (!q->isEnabled())
    {
    return;
    }

  // Get the ChartNode
  char *chartnodeid = this->MRMLChartViewNode->GetChartNodeID();

  if (!chartnodeid)
    {
    return;
    }

  vtkMRMLChartNode* cn = vtkMRMLChartNode::SafeDownCast(this->MRMLScene->GetNodeByID(chartnodeid));

  if (!cn)
    {
    return;
    }

  vtkStringArray *arrayIDs = cn->GetArrays();
  vtkStringArray *arrayNames = cn->GetArrayNames();

  // What type of chart?
  //
  //
  const char *type = cn->GetProperty("default", "type");

  // data to plot - represented in javascript
  //
  //
  QStringList plotData;
  plotData << "var data = [";

  // for each curve
  for (int idx = 0; idx < arrayIDs->GetNumberOfValues(); idx++)
    {
    vtkMRMLDoubleArrayNode *dn = vtkMRMLDoubleArrayNode::SafeDownCast(this->MRMLScene->GetNodeByID( arrayIDs->GetValue(idx).c_str() ));

    if (dn)
      {
      double x, y;

      plotData << "[";
      
      // for each value
      for (unsigned int j = 0; j < dn->GetSize(); ++j)
        {
        dn->GetXYValue(j, &x, &y);
        if (false && type && !strcmp(type, "Bar"))
          {
          // DISABLED. For bar plots, only take the y coordinate.
          plotData << QString("%1").arg(y);
          }
        else
          {
          plotData << "[" << QString("%1").arg(x) << ", " << QString("%1").arg(y) << "]";
          }
        if (j < dn->GetSize()-1)
          {
          plotData << ",";
          }
        }

      plotData<< "]";

      if (idx < arrayIDs->GetNumberOfValues()-1)
        {
        plotData << ",";
        }
      }
    }

  plotData << "];";
  

  // properties for the plot - represented in javascript
  //
  //
  
  QStringList plotOptions;
  plotOptions << "var options = {";

  // plot level properties: title, axis labels, grid, ...
  plotOptions << 
    "highlighter: {show: true, useAxesFormatters: false, formatString: '%.3g, %.3g'}, cursor: {show: false, zoom: true}";

  // title
  const char *showTitle = cn->GetProperty("default", "showTitle");
  const char *title = cn->GetProperty("default", "title");

  if (showTitle && !strcmp(showTitle, "on") && title)
    {
    plotOptions << ", title: '" << title << "'";
    }

  // jqplot bar charts (BarRenderer) was designed for categorical
  // data.  For numeric x-axis data, the bar widths are calculated
  // incorrectly. But if the bar charts are set to be "stacked", then
  // bar widths are calculated properly. Defaulting to stacked bar
  // charts for now.  May write our own renderer for the types of bar
  // charts we want.
  if (type && !strcmp(type, "Bar"))
    {
    plotOptions << ", stackSeries: true";
    }
  
  // axes labels
  const char *showXAxisLabel = cn->GetProperty("default", "showXAxisLabel");
  const char *xAxisLabel = cn->GetProperty("default", "xAxisLabel");
  const char *showYAxisLabel = cn->GetProperty("default", "showYAxisLabel");
  const char *yAxisLabel = cn->GetProperty("default", "yAxisLabel");
  
  bool showx = false, showy = false;
  if (showXAxisLabel && !strcmp(showXAxisLabel, "on") && xAxisLabel)
    {
    showx = true;
    }
  if (showYAxisLabel && !strcmp(showYAxisLabel, "on") && yAxisLabel)
    {
    showy = true;
    }
  if (showx || showy)
    {
    plotOptions << ", axes: {";
    if (showx)
      {
      plotOptions << "xaxis: {label: '" << xAxisLabel << "'";
      plotOptions << ", labelRenderer: $.jqplot.CanvasAxisLabelRenderer";
      // if (type && !strcmp(type, "Bar"))
      //   {
      //   plotOptions << ", renderer: $.jqplot.CategoryAxisRenderer";
      //   }
      plotOptions << "}";
      if (showy)
        {
        plotOptions << ", ";
        }
      }
    if (showy)
      {
      plotOptions << "yaxis: {label: '" << yAxisLabel << "'";
      plotOptions << ", labelRenderer: $.jqplot.CanvasAxisLabelRenderer}";
      }
    plotOptions << "}";
    }
    
  
  // grid
  const char *grid = cn->GetProperty("default", "showGrid");

  if (grid && !strcmp(grid, "on"))
    {
    plotOptions << ", grid: {drawGridlines: true}";
    }
  else
    {
    plotOptions << ", grid: {drawGridlines: false}";
    }

  // legend
  const char *legend = cn->GetProperty("default", "showLegend");

  if (legend && !strcmp(legend, "on"))
    {
    plotOptions << ", legend: {show: true}";
    }
  else
    {
    plotOptions << ", legend: {show: false}";
    }

  // default properties for a series
  //
  //
  plotOptions << ", seriesDefaults: {show: true";
  
  // chart type
  int defaultMarkers = 0; // 0 = not set, 1 = on, -1 = off
  int defaultLines = 0;   // 0 = not set, 1 = on, -1 = off
  if (type && !strcmp(type, "Line"))
    {
    defaultLines = 1; // lines on, markers don't care
    }
  if (type && !strcmp(type, "Scatter"))
    {
    defaultMarkers = 1; // markers on
    defaultLines = -1;  // lines off
    }
  if (type && !strcmp(type, "Bar"))
    {
    plotOptions << ", renderer: $.jqplot.BarRenderer";
    plotOptions << ", rendererOptions: {barWidth: null, fillToZero: true}";
    }

  // markers
  const char *markers = cn->GetProperty("default", "showMarkers");
    
  if ((markers && !strcmp(markers, "on")) || defaultMarkers == 1)
    {
    plotOptions << ", showMarker: true";
    }
  else if (markers && !strcmp(markers, "off"))
    {
    plotOptions << ", showMarker: false";
    }
  
  // lines
  const char *lines = cn->GetProperty("default", "showLines");
  
  if ((lines && !strcmp(lines, "on") && defaultLines != -1) || defaultLines == 1)
    {
    plotOptions << ", showLine: true";
    }
  else if ((lines && !strcmp(lines, "off")) || defaultLines == -1)
    {
    plotOptions << ", showLine: false";
    }

  // end of seriesDefaults properties
  plotOptions << "}";
  

  // series level properties
  //
  //
  plotOptions << ", series: [";
  for (int idx = 0; idx < arrayNames->GetNumberOfValues(); idx++)
    {
    std::string arrayName = arrayNames->GetValue(idx);

    // for each series
    plotOptions << "{";
    // legend
    plotOptions << "label: '" << arrayName.c_str() << "'";

    // markers
    const char *markers = cn->GetProperty(arrayName.c_str(), "showMarkers");
    
    if (markers && !strcmp(markers, "on"))
      {
      plotOptions << ", showMarker: true";
      }
    else if (markers && !strcmp(markers, "off"))
      {
      plotOptions << ", showMarker: false";
      }

    // lines
    const char *lines = cn->GetProperty(arrayName.c_str(), "showLines");

    if (lines && !strcmp(lines, "on"))
      {
      plotOptions << ", showLine: true";
      }
    else if (lines && !strcmp(lines, "off"))
      {
      plotOptions << ", showLine: false";
      }

    // color
    const char *color = cn->GetProperty(arrayName.c_str(), "color");
    
    if (color)
      {
      plotOptions << ", color: '" << color << "'";
      }

    // end of a series
    plotOptions << "}";
    if (idx < arrayNames->GetNumberOfValues()-1)
      {
      plotOptions << ",";
      }
    }
  // end of series properties
  plotOptions << "]";
  // end of properties
  plotOptions << "};";


  // resize slot - represented in javascript
  // pass in resetAxes: true option to get rid of old ticks and axis properties
  QStringList plotResizeSlot;
  plotResizeSlot << 
    "var resizeSlot = function() {"
    "$('#chart').css('width', 0.95*$(window).width());"
    "$('#chart').css('height', 0.95*$(window).height());"
    "plot1.replot( {resetAxes: true} );"
    "};";

  // an initial call to the resize slot - represented in javascript
  QStringList plotInitialResize;
  plotInitialResize <<
    "resizeSlot();";

  // resize hook - represented in javascript
  // resize function should be bound to the #chart not to the window
  QStringList plotResizeHook;
  plotResizeHook <<
    "$(window).resize( resizeSlot );";


  // Assemble the plot
  //
  // 1. HTML page preamble
  // 2. Div container for the chart
  // 3. Script container
  // 4. Definition of the "ready" function inside the script container
  // 5. HTML page poscript
  //
  QStringList plot;
  plot << plotPreamble;       // 1. page header, css, javascript

  plot << 
    "<div id=\"chart\"></div>"                       // 2. container for the chart
    "<script class=\"code\" type=\"text/javascript\">"    // 3. container for js
    "$(document).ready(function(){";                 // 4. ready function     
  plot << plotData;     // insert data
  plot << plotOptions;  // insert options
  plot << 
    "var plot1 = $.jqplot ('chart', data, options);";  // call the plot
  plot << plotResizeSlot;        // insert definition of the resizeSlot
  plot << plotInitialResize;     // insert an initial call to resizeSlot 
  plot << plotResizeHook;        // insert hook to call resizeSlot on page resize

  plot << 
    "});"                   // end of function and end of call to ready()
    "</script>";            // end of the javascript
      
  plot << plotPostscript;   // 5. page postscript, additional javascript
  
  // qDebug() << plot.join("");

  // show the plot
  q->setHtml(plot.join("")); 
  q->show();

}

// --------------------------------------------------------------------------
// qMRMLChartView methods

// --------------------------------------------------------------------------
qMRMLChartView::qMRMLChartView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLChartViewPrivate(*this))
{
  Q_D(qMRMLChartView);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLChartView::~qMRMLChartView()
{
}

//------------------------------------------------------------------------------
void qMRMLChartView::addDisplayableManager(const QString&) //displayableManagerName)
{
  // Q_D(qMRMLChartView);
  // vtkSmartPointer<vtkMRMLAbstractDisplayableManager> displayableManager;
  // displayableManager.TakeReference(
  //   vtkMRMLDisplayableManagerGroup::InstantiateDisplayableManager(
  //     displayableManagerName.toLatin1()));
  // d->DisplayableManagerGroup->AddDisplayableManager(displayableManager);
}

//------------------------------------------------------------------------------
void qMRMLChartView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLChartView);
  if (newScene == d->MRMLScene)
    {
    return;
    }

  d->setMRMLScene(newScene);

  if (d->MRMLChartViewNode && newScene != d->MRMLChartViewNode->GetScene())
    {
    this->setMRMLChartViewNode(0);
    }

  emit mrmlSceneChanged(newScene);
}

//---------------------------------------------------------------------------
void qMRMLChartView::setMRMLChartViewNode(vtkMRMLChartViewNode* newChartViewNode)
{
  Q_D(qMRMLChartView);
  if (d->MRMLChartViewNode == newChartViewNode)
    {
    return;
    }

  // connect modified event on ChartViewNode to updating the widget
  d->qvtkReconnect(
    d->MRMLChartViewNode, newChartViewNode,
    vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromMRML()));

  // connect on ChartNodeChangedEvent (e.g. ChartView is looking at a
  // different ChartNode
  d->qvtkReconnect(
    d->MRMLChartViewNode, newChartViewNode, 
    vtkMRMLChartViewNode::ChartNodeChangedEvent, d, SLOT(onChartNodeChanged()));

  // cache the ChartViewNode
  d->MRMLChartViewNode = newChartViewNode;

  // ... and connect modified event on the ChartViewNode's ChartNode
  // to update the widget
  d->onChartNodeChanged();
  
  // d->DisplayableManagerGroup->SetMRMLDisplayableNode(newChartViewNode);

  // make sure the gui is up to date
  d->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
vtkMRMLChartViewNode* qMRMLChartView::mrmlChartViewNode()const
{
  Q_D(const qMRMLChartView);
  return d->MRMLChartViewNode;
}

//---------------------------------------------------------------------------
vtkMRMLScene* qMRMLChartView::mrmlScene()const
{
  Q_D(const qMRMLChartView);
  return d->MRMLScene;
}


