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

// STD includes
#include <vector>
#include <algorithm>

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkLogger.h>
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLChartView_p.h"

// MRML includes
#include <vtkMRMLChartNode.h>
#include <vtkMRMLChartViewNode.h>
#include <vtkMRMLColorLogic.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDoubleArrayNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLChartView");
//--------------------------------------------------------------------------


const char *plotPreamble =
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<title>Chart</title>"
  "<link class=\"include\" rel=\"stylesheet\" type=\"text/css\" href=\"qrc:/jqPlot/jquery.jqplot.min.css\" />"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/jquery.min.js\"></script>"
  "</head>"
  "<body>";

const char *plotPostscript =
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/jquery.jqplot.min.js\"></script>"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.canvasTextRenderer.min.js\"></script>"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.barRenderer.min.js\"></script>"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.boxplotRenderer.min.js\"></script>"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.canvasAxisLabelRenderer.min.js\"></script>"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.canvasAxisTickRenderer.min.js\"></script>"
  "<script class=\"include\" type=\"text/javascript\" src=\"qrc:/jqPlot/plugins/jqplot.categoryAxisRenderer.min.js\"></script>"
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
  this->MRMLScene = nullptr;
  this->MRMLChartViewNode = nullptr;
  this->MRMLChartNode = nullptr;
  this->ColorLogic = nullptr;
  this->PinButton = nullptr;
  this->PopupWidget = nullptr;
}

//---------------------------------------------------------------------------
qMRMLChartViewPrivate::~qMRMLChartViewPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLChartViewPrivate::init()
{
  Q_Q(qMRMLChartView);

  // Let the QWebView expand in both directions
  q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // Expose the ChartView class to Javascript
  // q->page()->mainFrame()->addToJavaScriptWindowObject(QString("qtobject"), this);
  // XXX Change to webchannel

  this->PopupWidget = new ctkPopupWidget;
  QHBoxLayout* popupLayout = new QHBoxLayout;
  popupLayout->addWidget(new QToolButton);
  this->PopupWidget->setLayout(popupLayout);

  q->setHtml("");
  //q->show();
}

//---------------------------------------------------------------------------
void qMRMLChartViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  //Q_Q(qMRMLChartView);
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
void qMRMLChartViewPrivate::startProcessing()
{
//  logger.trace("startProcessing");
//  Q_Q(qMRMLChartView);
}

//
// --------------------------------------------------------------------------
void qMRMLChartViewPrivate::endProcessing()
{
//  logger.trace("endProcessing");
//  Q_Q(qMRMLChartView);
  this->updateWidgetFromMRML();
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

  vtkMRMLChartNode *newChartNode=nullptr;

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
    q->setHtml("");
    //q->show();
    return;
    }

  vtkMRMLChartNode* cn = vtkMRMLChartNode::SafeDownCast(this->MRMLScene->GetNodeByID(chartnodeid));

  if (!cn)
    {
    q->setHtml("");
    //q->show();
    return;
    }


  // Generate javascript for the data, ticks, options
  //
  //
  QStringList plotSeries;
  QStringList plotXAxisTicks;
  QStringList plotOptions;

  const char *type = cn->GetProperty("default", "type");

  if (!type || (type && !strcmp(type, "Line")))
    {
    // line charts are the default
    plotSeries << this->lineData(cn);
    plotXAxisTicks << this->lineXAxisTicks(cn);
    plotOptions << this->lineOptions(cn);
    }
  else if (type && !strcmp(type, "Scatter"))
    {
    plotSeries << this->scatterData(cn);
    plotXAxisTicks << this->scatterXAxisTicks(cn);
    plotOptions << this->scatterOptions(cn);
    }
  else if (type && !strcmp(type, "Bar"))
    {
    plotSeries << this->barData(cn);
    plotXAxisTicks << this->barXAxisTicks(cn);
    plotOptions << this->barOptions(cn);
    }
  else if (type && !strcmp(type, "Box"))
    {
    plotSeries << this->boxData(cn);
    plotXAxisTicks << this->boxXAxisTicks(cn);
    plotOptions << this->boxOptions(cn);
    }

  // resize slot - represented in javascript
  // pass in resetAxes: true option to get rid of old ticks and axis properties
  QStringList plotResizeSlot;
  plotResizeSlot <<
    "var resizeSlot = function() {"
    "$('#chart').css('width', 0.95*$(window).width());"
    "$('#chart').css('height', 0.95*$(window).height());"
    "var opts = {resetAxes: true};"
    "if (plot1.series[0].renderer && plot1.series[0].renderer.constructor === $.jqplot.BoxplotRenderer){"
    "  opts = {resetAxes: true, axes: plot1.series[0].renderer.replot.axes};}"
    "plot1.replot( opts );"
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

  // data mouse over slot - represented in javascript
  QStringList plotSeriesMouseOverSlot;
  plotSeriesMouseOverSlot <<
    "var dataMouseOverSlot = function(ev, seriesIndex, pointIndex, data) {"
    "try {"
    "qtobject.onDataMouseOver(seriesIndex, pointIndex, data[0], data[1]);"
    "} catch(error) {}"
    "};";

  // data point click slot - represented in javascript
  QStringList plotSeriesPointClickedSlot;
  plotSeriesPointClickedSlot <<
    "var dataPointClickedSlot = function(ev, seriesIndex, pointIndex, data) {"
    "try {"
    "qtobject.onDataPointClicked(seriesIndex, pointIndex, data[0], data[1]);"
    "} catch(error) {}"
    "};";

  // bind a data point clicked to the slot
  QStringList plotSeriesMouseOverHook;
  plotSeriesMouseOverHook <<
    "$('#chart').bind('jqplotSeriesMouseOver', dataMouseOverSlot);";


  // bind a data point clicked to the slot
  QStringList plotSeriesPointClickedHook;
  plotSeriesPointClickedHook <<
    "$('#chart').bind('jqplotSeriesClick', dataPointClickedSlot);";

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
  plot << plotSeries;     // insert data
  plot << plotXAxisTicks;  // insert ticks if needed
  plot << plotOptions;  // insert options
  plot <<
    "var plot1 = $.jqplot ('chart', data, options);";  // call the plot
  plot << plotResizeSlot;        // insert definition of the resizeSlot
  plot << plotInitialResize;     // insert an initial call to resizeSlot
  plot << plotResizeHook;        // insert hook to call resizeSlot on page resize
  plot << plotSeriesMouseOverSlot; // insert definition of the data mouse over slot
  plot << plotSeriesMouseOverHook; // insert the binding to the slot
  plot << plotSeriesPointClickedSlot; // insert definition of the data clicked slot
  plot << plotSeriesPointClickedHook; // insert the binding to the slot

  plot <<
    "});"                   // end of function and end of call to ready()
    "</script>";            // end of the javascript

  plot << plotPostscript;   // 5. page postscript, additional javascript

  //qDebug() << plot.join("");

  // show the plot
  q->setHtml(plot.join(""));
  //q->show();


  // expose this object to the Javascript code so Javascript can call
  // slots in this Qt object, e.g. onDataPointClicked()
  // q->page()->mainFrame()->addToJavaScriptWindowObject(QString("qtobject"), this);
  // XXX Change to webchannel

}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::seriesColorsString(vtkMRMLColorNode *colorNode)
{
  QStringList seriesColors;

  seriesColors << "[";

  if (colorNode)
    {
    double c[4];
    QColor qc;
    QString sc;

    for (int i=0; i < colorNode->GetNumberOfColors(); ++i)
      {
      colorNode->GetColor(i, c);
      qc.setRgbF(c[0], c[1], c[2]);
      seriesColors << "'" << qc.name() << "'";
      if (i < colorNode->GetNumberOfColors()-1)
        {
        seriesColors << ", ";
        }
      }
    }

  seriesColors << "]";

  return seriesColors.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::seriesColorsString(vtkMRMLColorNode *colorNode, vtkMRMLDoubleArrayNode *arrayNode)
{
  QStringList seriesColors;

  seriesColors << "[";

  if (colorNode)
    {
    double c[4], x, y;
    QColor qc;
    QString sc;

    // Loop over the values in the array and lookup their color from
    // the colortable
    for (unsigned int i=0; i < arrayNode->GetSize(); ++i)
      {
      arrayNode->GetXYValue(i, &x, &y);
      colorNode->GetColor(x, c);
      qc.setRgbF(c[0], c[1], c[2]);
      seriesColors << "'" << qc.name() << "'";
      if (i < arrayNode->GetSize()-1)
        {
        seriesColors << ", ";
        }
      }
    }

  seriesColors << "]";

  return seriesColors.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::seriesTicksString(vtkMRMLDoubleArrayNode *dn)
{
  QStringList data;

  data << "[";

  if (dn)
    {
    double x, y;

    // for each value
    for (unsigned int j = 0; j < dn->GetSize(); ++j)
      {
      dn->GetXYValue(j, &x, &y);
      data << QString("%1").arg(x);
      if (j < dn->GetSize()-1)
        {
        data << ",";
        }
      }
    }

  data << "]";

  return data.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::arrayTicksString(vtkStringArray *arrayNames)
{
  QStringList data;

  data << "[";

  for (int idx = 0; idx < arrayNames->GetNumberOfValues(); idx++)
    {
    data << "'" << QString(arrayNames->GetValue(idx)) << "'";
    if (idx < arrayNames->GetNumberOfValues()-1)
      {
      data << ", ";
      }
    }

  data << "]";

  return data.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::seriesLabelTicksString(vtkMRMLDoubleArrayNode *dn, vtkMRMLColorNode *cn)
{
  QStringList data;

  data << "[";

  if (dn)
    {
    double x, y;

    // for each value
    for (unsigned int j = 0; j < dn->GetSize(); ++j)
      {
      dn->GetXYValue(j, &x, &y);
      data << "'" << cn->GetColorName((int)x) << "'";
      if (j < dn->GetSize()-1)
        {
        data << ",";
        }
      }
    }

  data << "]";

  return data.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::seriesDateTicksString(vtkMRMLDoubleArrayNode *dn)
{
  QStringList data;

  data << "[";

  if (dn)
    {
    double x, y;

    // for each value
    for (unsigned int j = 0; j < dn->GetSize(); ++j)
      {
      dn->GetXYValue(j, &x, &y);
      // convert from unix timestamp (seconds) to javascript timestamp (ms)
      data << QString("(new Date(%1)).toISOString().slice(0,10)").arg(x * 1000.0);
      if (j < dn->GetSize()-1)
        {
        data << ",";
        }
      }
    }

  data << "]";

  return data.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::seriesDataString(vtkMRMLDoubleArrayNode *dn)
{
  QStringList data;

  data << "[";

  if (dn)
    {
    double x, y;

    // for each value
    for (unsigned int j = 0; j < dn->GetSize(); ++j)
      {
      dn->GetXYValue(j, &x, &y);
      data << "[" << QString("%1").arg(x) << ", " << QString("%1").arg(y) << "]";
      if (j < dn->GetSize()-1)
        {
        data << ",";
        }
      }
    }

  data << "]";

  return data.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::seriesLabelDataString(vtkMRMLDoubleArrayNode *dn, vtkMRMLColorNode *cn)
{
  QStringList data;

  data << "[";

  if (dn)
    {
    double x, y;

    // for each value
    for (unsigned int j = 0; j < dn->GetSize(); ++j)
      {
      dn->GetXYValue(j, &x, &y);
      data << "['" << cn->GetColorName((int)x) << "', " << QString("%1").arg(y) << "]";
      if (j < dn->GetSize()-1)
        {
        data << ",";
        }
      }
    }

  data << "]";

  return data.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::seriesDependentDataString(vtkMRMLDoubleArrayNode *dn)
{
  QStringList data;

  data << "[";

  if (dn)
    {
    double x, y;

    // for each value
    for (unsigned int j = 0; j < dn->GetSize(); ++j)
      {
      dn->GetXYValue(j, &x, &y);
      // only emit the y axis value, x axis defined elsewhere by the "ticks"
      data << QString("%1").arg(y);
      if (j < dn->GetSize()-1)
        {
        data << ",";
        }
      }
    }

  data << "]";

  return data.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::lineData(vtkMRMLChartNode *cn)
{
  QStringList data;

  vtkStringArray *arrayIDs = cn->GetArrays();
  const char *xAxisType = cn->GetProperty("default", "xAxisType");

  data << "var data = [";

  // for each curve
  for (int idx = 0; idx < arrayIDs->GetNumberOfValues(); idx++)
    {
    vtkMRMLDoubleArrayNode *dn = vtkMRMLDoubleArrayNode::SafeDownCast(this->MRMLScene->GetNodeByID( arrayIDs->GetValue(idx).c_str() ));

    if (dn)
      {
      if (xAxisType && !strcmp(xAxisType, "date"))
        {
        // convert the data array into a string using just the
        // dependent variables.  the dates will be specified using the
        // ticks along the x-axis
        data << this->seriesDependentDataString(dn);
        }
      else
        {
        // convert the data array into a string of quantitative values
        data << this->seriesDataString(dn);
        }

      if (idx < arrayIDs->GetNumberOfValues()-1)
        {
        data << ",";
        }
      }
    }

  data << "];";

  return data.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::lineXAxisTicks(vtkMRMLChartNode *cn)
{
  QStringList ticks;

  vtkStringArray *arrayIDs = cn->GetArrays();
  const char *xAxisType = cn->GetProperty("default", "xAxisType");

  if (!arrayIDs || arrayIDs->GetNumberOfValues() <= 0)
    {
    // no axis ticks by default
    }
  else if (!xAxisType || (xAxisType && !strcmp(xAxisType, "categorical")))
    {
    // without any other information, all we can do it use the x-data
    // as categories

    // define the ticks from the first curve (could do better)
    vtkMRMLDoubleArrayNode *dn = vtkMRMLDoubleArrayNode::SafeDownCast(
      this->MRMLScene->GetNodeByID( arrayIDs->GetValue(0).c_str() ));

    if (dn)
      {
      ticks << "var xAxisTicks = "
            << this->seriesTicksString(dn)
            << ";";
      }
    }
  else if (xAxisType && !strcmp(xAxisType, "date"))
    {
    // define the ticks from the first curve (could do better)
    vtkMRMLDoubleArrayNode *dn = vtkMRMLDoubleArrayNode::SafeDownCast(
      this->MRMLScene->GetNodeByID( arrayIDs->GetValue(0).c_str() ));

    if (dn)
      {
      ticks << "var xAxisTicks = "
            << this->seriesDateTicksString(dn)
            << ";";
      }
    }

  return ticks.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::genericOptions(vtkMRMLChartNode *cn, bool rotateXTickLabels)
{
  QStringList options;

  // plot level properties: title, axis labels, grid, ...
  options <<
    "highlighter: {show: true, useAxesFormatters: false, formatString: '%.3g, %.3g'}, cursor: {show: true, zoom: true}";

  // title
  const char *showTitle = cn->GetProperty("default", "showTitle");
  const char *title = cn->GetProperty("default", "title");

  if (showTitle && !strcmp(showTitle, "on") && title)
    {
    options << ", title: '" << title << "'";
    }

  // axes labels
  const char *showXAxisLabel = cn->GetProperty("default", "showXAxisLabel");
  const char *xAxisLabel = cn->GetProperty("default", "xAxisLabel");
  const char *xAxisPad = cn->GetProperty("default", "xAxisPad");
  const char *showYAxisLabel = cn->GetProperty("default", "showYAxisLabel");
  const char *yAxisLabel = cn->GetProperty("default", "yAxisLabel");
  const char *yAxisPad = cn->GetProperty("default", "yAxisPad");
  const char *xAxisType = cn->GetProperty("default", "xAxisType");
  const char *yAxisType = cn->GetProperty("default", "yAxisType");

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
    options << ", axes: {";
    if (showx)
      {
      options << "xaxis: {label: '" << xAxisLabel << "'";
      options << ", labelRenderer: $.jqplot.CanvasAxisLabelRenderer";
      if (xAxisPad)
      {
        options << ", pad: " << xAxisPad;
      }
      if (xAxisType && !strcmp(xAxisType, "categorical"))
        {
        options << ", renderer: $.jqplot.CategoryAxisRenderer";
        options << ", ticks: xAxisTicks";
        options << ", tickRenderer: $.jqplot.CanvasAxisTickRenderer";
        if (rotateXTickLabels)
          {
          options << ", tickOptions: { angle: -30 }";
          }
        }
      if (xAxisType && !strcmp(xAxisType, "date"))
        {
        // date axis, use a category axis
        options << ", renderer: $.jqplot.CategoryAxisRenderer"
                    << ", tickRenderer: $.jqplot.CanvasAxisTickRenderer"
                    << ", ticks: xAxisTicks"
                    << ", tickOptions: { angle: -30 }";
        }
      options << "}";
      if (showy)
        {
        options << ", ";
        }
      }
    if (showy)
      {
      options << "yaxis: {label: '" << yAxisLabel << "'";
      options << ", labelRenderer: $.jqplot.CanvasAxisLabelRenderer";
      if (yAxisPad)
      {
        options << ", pad: " << yAxisPad;
      }
      if (yAxisType && !strcmp(yAxisType, "categorical"))
        {
        options << ", renderer: $.jqplot.CategoryAxisRenderer";
        }
      options << "}";
      }
    options << "}";
    }

  // grid
  const char *grid = cn->GetProperty("default", "showGrid");

  if (grid && !strcmp(grid, "on"))
    {
    options << ", grid: {drawGridlines: true}";
    }
  else
    {
    options << ", grid: {drawGridlines: false}";
    }


  return options.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::lineOptions(vtkMRMLChartNode *cn)
{
  QStringList options;

  vtkStringArray *arrayNames = cn->GetArrayNames();

  options << "var options = {";

  // add the options that are the same for all charts
  options << this->genericOptions(cn, false);

  // legend
  const char *legend = cn->GetProperty("default", "showLegend");

  if (legend && !strcmp(legend, "on"))
    {
    options << ", legend: {show: true}";
    }
  else
    {
    options << ", legend: {show: false}";
    }

  // Use a default set of colors defined by Slicer or specified by the
  // chart node. We define the seriesColors here for line charts to
  // work. If seriesColors is defined in seriesDefaults, then only bar
  // charts observe it.
  const char* defaultChartColorNodeID =
    this->ColorLogic ? this->ColorLogic->GetDefaultChartColorNodeID() : nullptr;
  vtkMRMLColorNode *defaultColorNode = vtkMRMLColorNode::SafeDownCast(
    this->MRMLScene->GetNodeByID(defaultChartColorNodeID));
  vtkMRMLColorNode *colorNode = defaultColorNode;
  const char *lookupTable = cn->GetProperty("default", "lookupTable");
  if (lookupTable)
    {
    colorNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(lookupTable));
    }

  if (colorNode)
    {
    options << ", seriesColors: " << this->seriesColorsString(colorNode);
    }

  // markerOptions
  options << ", markerOptions: {" ;
  // markers size
  const char *markersSize = cn->GetProperty("default", "size");
  if (markersSize && QString(markersSize).toInt() > 0)  // Checks if given size is an integer and if larger than 0
    {
    options << "size: " << markersSize;
    }
  options << "}" ;
  // end of markerOptions

  // default properties for a series
  //
  //
  options << ", seriesDefaults: {show: true";

  // chart type
  int defaultMarkers = 0; // 0 = not set, 1 = on, -1 = off
  int defaultLines = 0;   // 0 = not set, 1 = on, -1 = off
  const char *type = cn->GetProperty("default", "type");
  if (type && !strcmp(type, "Line"))
    {
    defaultLines = 1; // lines on, markers don't care
    }
  if (type && !strcmp(type, "Scatter"))
    {
    defaultMarkers = 1; // markers on
    defaultLines = -1;  // lines off
    }

  // markers
  const char *markers = cn->GetProperty("default", "showMarkers");

  if ((markers && !strcmp(markers, "on")) || defaultMarkers == 1)
    {
    options << ", showMarker: true";
    }
  else if (markers && !strcmp(markers, "off"))
    {
    options << ", showMarker: false";
    }

  // lines
  const char *lines = cn->GetProperty("default", "showLines");

  if ((lines && !strcmp(lines, "on") && defaultLines != -1) || defaultLines == 1)
    {
    options << ", showLine: true";
    }
  else if ((lines && !strcmp(lines, "off")) || defaultLines == -1)
    {
    options << ", showLine: false";
    }

  // line pattern
  const char *linePattern = cn->GetProperty("default", "linePattern");

  if (linePattern && !strcmp(linePattern, "solid"))
    {
    // By default the line pattern is solid
    }
  else if (linePattern && !strcmp(linePattern, "dashed"))
    {
    options << ", linePattern: 'dashed'";
    }
  else if (linePattern && !strcmp(linePattern, "dotted"))
    {
    options << ", linePattern: 'dotted'";
    }
  else if (linePattern && !strcmp(linePattern, "dashed-dotted"))
    {
    options << ", linePattern: '-.'";
    }

  // line width
  const char *lineWidth = cn->GetProperty("default", "lineWidth");
  if (lineWidth && QString(lineWidth).toInt() > 0)  // Checks if given lineWidth is an integer and if larger than 0
    {
    options << ", lineWidth: " << lineWidth;
    }

  // end of seriesDefaults properties
  options << "}";

  // series level properties
  //
  //
  options << ", series: [";
  for (int idx = 0; idx < arrayNames->GetNumberOfValues(); idx++)
    {
    std::string arrayName = arrayNames->GetValue(idx);

    // for each series
    options << "{";
    // legend
    options << "label: '" << arrayName.c_str() << "'";

    // markers
    const char *markers = cn->GetProperty(arrayName.c_str(), "showMarkers");

    if (markers && !strcmp(markers, "on"))
      {
      options << ", showMarker: true";
      }
    else if (markers && !strcmp(markers, "off"))
      {
      options << ", showMarker: false";
      }

    // lines
    const char *lines = cn->GetProperty(arrayName.c_str(), "showLines");

    if (lines && !strcmp(lines, "on"))
      {
      options << ", showLine: true";
      }
    else if (lines && !strcmp(lines, "off"))
      {
      options << ", showLine: false";
      }

    // line pattern
    const char *linePattern = cn->GetProperty(arrayName.c_str(), "linePattern");

    if (linePattern && !strcmp(linePattern, "solid"))
      {
      options << ", linePattern: 'solid'";
      }
    if (linePattern && !strcmp(linePattern, "dashed"))
      {
      options << ", linePattern: 'dashed'";
      }
    else if (linePattern && !strcmp(linePattern, "dotted"))
      {
      options << ", linePattern: 'dotted'";
      }
    else if (linePattern && !strcmp(linePattern, "dashed-dotted"))
      {
      options << ", linePattern: '-.'";
      }

    // line width
  const char *lineWidth = cn->GetProperty(arrayName.c_str(), "lineWidth");
  if (lineWidth && QString(lineWidth).toInt() > 0)  // Checks if given lineWidth is an integer and if larger than 0
    {
    options << ", lineWidth: " << lineWidth;
    }

    // color
    const char *color = cn->GetProperty(arrayName.c_str(), "color");

    if (color)
      {
      options << ", color: '" << color << "'";
      }

    // markerOptions
    options << ", markerOptions: {" ;
    // markers size
    const char *markersSize = cn->GetProperty(arrayName.c_str(), "size");
    if (markersSize && QString(markersSize).toInt() > 0)  // Checks if given size is an integer and if larger than 0
      {
      options << "size: " << markersSize;
      }
    options << "}" ;
    // end of markerOptions
    // end of a series
    options << "}";
    if (idx < arrayNames->GetNumberOfValues()-1)
      {
      options << ",";
      }
    }
  // end of series properties
  options << "]";
  // end of properties
  options << "};";


  return options.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::scatterData(vtkMRMLChartNode *cn)
{
  return this->lineData(cn);
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::scatterXAxisTicks(vtkMRMLChartNode *cn)
{
  return this->lineXAxisTicks(cn);
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::scatterOptions(vtkMRMLChartNode *cn)
{
  return this->lineOptions(cn);
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::barData(vtkMRMLChartNode *cn)
{
  QStringList data;

  vtkStringArray *arrayIDs = cn->GetArrays();
  vtkStringArray *arrayNames = cn->GetArrayNames();
  const char *xAxisType = cn->GetProperty("default", "xAxisType");

  data << "var data = [";

  // for each curve
  for (int idx = 0; idx < arrayIDs->GetNumberOfValues(); idx++)
    {
    vtkMRMLDoubleArrayNode *dn = vtkMRMLDoubleArrayNode::SafeDownCast(this->MRMLScene->GetNodeByID( arrayIDs->GetValue(idx).c_str() ));

    if (dn)
      {
      vtkMRMLColorNode *seriesColorNode = nullptr;
      const char *seriesLookupTable
        = cn->GetProperty(arrayNames->GetValue(idx).c_str(), "lookupTable");
      if (seriesLookupTable)
        {
        seriesColorNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(seriesLookupTable));
        }

      if (xAxisType && !strcmp(xAxisType, "categorical")
          && seriesColorNode)
        {
        // convert the data into a string by using just the dependent
        // variables. we'll use the color names (tissue names) as
        // "ticks" along the x-axis
        data << this->seriesDependentDataString(dn);
        if (dn->GetSize() > 6)
          {
          //rotateXTickLabels = true;
          }
        }
      else if (xAxisType && !strcmp(xAxisType, "date"))
        {
        // convert the data array into a string using just the
        // dependent variables. we'll use the dates as "ticks" along
        // the x-axis
        data << this->seriesDependentDataString(dn);
        }
      else
        {
        // convert the data array into a string of quantitative values
        data << this->seriesDataString(dn);
        }

      if (idx < arrayIDs->GetNumberOfValues()-1)
        {
        data << ",";
        }
      }
    }

  data << "];";

  return data.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::barXAxisTicks(vtkMRMLChartNode *cn)
{
  QStringList ticks;

  vtkStringArray *arrayIDs = cn->GetArrays();
  vtkStringArray *arrayNames = cn->GetArrayNames();
  const char *xAxisType = cn->GetProperty("default", "xAxisType");

  if (!arrayIDs || arrayIDs->GetNumberOfValues() <= 0)
    {
    // no axis ticks by default
    }
  else if (!xAxisType || (xAxisType && !strcmp(xAxisType, "categorical")))
    {
    // define the ticks from the first curve (could do better)
    vtkMRMLDoubleArrayNode *dn = vtkMRMLDoubleArrayNode::SafeDownCast(this->MRMLScene->GetNodeByID( arrayIDs->GetValue(0).c_str() ));

    if (dn)
      {
      const char *seriesLookupTable = cn->GetProperty(arrayNames->GetValue(0).c_str(), "lookupTable");
      if (seriesLookupTable)
        {
        vtkMRMLColorNode *seriesColorNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(seriesLookupTable));

        ticks << "var xAxisTicks = "
              << this->seriesLabelTicksString(dn, seriesColorNode)
              << ";";
        }
      }
    }
  else if (xAxisType && !strcmp(xAxisType, "date"))
    {
    // define the ticks from the first curve (could do better)
    vtkMRMLDoubleArrayNode *dn = vtkMRMLDoubleArrayNode::SafeDownCast(this->MRMLScene->GetNodeByID( arrayIDs->GetValue(0).c_str() ));

    if (dn)
      {
      ticks << "var xAxisTicks = "
            << this->seriesDateTicksString(dn)
            << ";";
      }
    }

  return ticks.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::barOptions(vtkMRMLChartNode *cn)
{
  QStringList options;

  vtkStringArray *arrayIDs = cn->GetArrays();
  vtkStringArray *arrayNames = cn->GetArrayNames();
  const char *xAxisType = cn->GetProperty("default", "xAxisType");

  options << "var options = {";

  // Do we need to rotate the x-axis tick labels?
  bool rotateXTickLabels = false;
  if (!arrayIDs || arrayIDs->GetNumberOfValues() <= 0)
    {
    // no bar options by default
    }
  else if (xAxisType && !strcmp(xAxisType, "categorical"))
    {
    // if we have a color table for the first series
    vtkMRMLDoubleArrayNode *dn = vtkMRMLDoubleArrayNode::SafeDownCast(this->MRMLScene->GetNodeByID( arrayIDs->GetValue(0).c_str() ));
    if (dn)
      {
      const char *seriesLookupTable = cn->GetProperty(arrayNames->GetValue(0).c_str(), "lookupTable");
      if (seriesLookupTable)
        {
        if (dn->GetSize() > 6)
          {
          rotateXTickLabels = true;
          }
        }
      }
    }

  // add the options that are the same for all charts
  options << this->genericOptions(cn, rotateXTickLabels);

  // jqplot bar charts (BarRenderer) were designed for categorical
  // data.  For numeric x-axis data, the bar widths are calculated
  // incorrectly. But if the bar charts are set to be "stacked", then
  // bar widths are calculated properly. Defaulting to stacked bar
  // charts for now.  May write our own renderer for the types of bar
  // charts we want.
  //
  // Note that this also affects how varyBarColor
  // works. varyBarColor is disabled for stacked bar charts. So you
  // can only assign a color node to nonquantitative data.
  if (!xAxisType || (xAxisType && !strcmp(xAxisType, "quantitative")))
    {
    // use stacking for "quantitative"
    options << ", stackSeries: true";
    }

  // legend. should we  show if not categorical?
  const char *legend = cn->GetProperty("default", "showLegend");

  if (legend && !strcmp(legend, "on"))
    {
    options << ", legend: {show: true}";
    }
  else
    {
    options << ", legend: {show: false}";
    }

  // Use a default set of colors defined by Slicer or specified by the
  // chart node. We define the seriesColors here for line charts to
  // work. If seriesColors is defined in seriesDefaults, then only bar
  // charts observe it.
  const char* defaultChartColorNodeID =
    this->ColorLogic ? this->ColorLogic->GetDefaultChartColorNodeID() : nullptr;
  vtkMRMLColorNode *defaultColorNode = vtkMRMLColorNode::SafeDownCast(
    this->MRMLScene->GetNodeByID(defaultChartColorNodeID));
  vtkMRMLColorNode *colorNode = defaultColorNode;
  const char *lookupTable = cn->GetProperty("default", "lookupTable");
  if (lookupTable)
    {
    colorNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(lookupTable));
    }

  if (colorNode)
    {
    options << ", seriesColors: " << this->seriesColorsString(colorNode);
    }

  // default properties for a series
  //
  //
  options << ", seriesDefaults: {show: true";

  options << ", renderer: $.jqplot.BarRenderer";
  options << ", rendererOptions: {barWidth: null, fillToZero: true, varyBarColor: false, useNegativeColors: false}";

  // end of seriesDefaults properties
  options << "}";


  // series level properties
  //
  //
  options << ", series: [";
  for (int idx = 0; idx < arrayNames->GetNumberOfValues(); idx++)
    {
    std::string arrayName = arrayNames->GetValue(idx);

    // for each series
    options << "{";
    // legend
    options << "label: '" << arrayName.c_str() << "'";

    // color
    const char *color = cn->GetProperty(arrayName.c_str(), "color");

    if (color)
      {
      options << ", color: '" << color << "'";
      }

    // seriesColors, only use for bar
    const char *seriesLookupTable = cn->GetProperty(arrayName.c_str(), "lookupTable");
    if (seriesLookupTable)
      {
      vtkMRMLColorNode *seriesColorNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(seriesLookupTable));
      vtkMRMLDoubleArrayNode *arrayNode = vtkMRMLDoubleArrayNode::SafeDownCast(this->MRMLScene->GetNodeByID(arrayIDs->GetValue(idx).c_str()));
      if (seriesColorNode)
        {
        options << ", seriesColors: "
                    << this->seriesColorsString(seriesColorNode, arrayNode);
        options << ", rendererOptions: {varyBarColor: true}";
        }
      }

    // end of a series
    options << "}";
    if (idx < arrayNames->GetNumberOfValues()-1)
      {
      options << ",";
      }
    }
  // end of series properties
  options << "]";
  // end of properties
  options << "};";


  return options.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::boxData(vtkMRMLChartNode *cn)
{
  // Box charts are a bit different.  Each MRML series defines one
  // box. The boxes from each series form the "series" sent to jqPlot.

  QStringList data;

  vtkStringArray *arrayIDs = cn->GetArrays();
  const char *xAxisType = cn->GetProperty("default", "xAxisType");

  data << "var data = [";

  // for each curve
  for (int idx = 0; idx < arrayIDs->GetNumberOfValues(); idx++)
    {
    vtkMRMLDoubleArrayNode *dn = vtkMRMLDoubleArrayNode::SafeDownCast(this->MRMLScene->GetNodeByID( arrayIDs->GetValue(idx).c_str() ));

    if (dn)
      {
      if (xAxisType && !strcmp(xAxisType, "categorical"))
        {
        // all the MRML series get lumped into one jqPlot series
        // (need to surround the data with an extra [])
        if (idx == 0)
          {
          data << "[";
          }

        data << this->seriesBoxDataString(dn, idx);

        if (idx == arrayIDs->GetNumberOfValues()-1)
          {
          data << "]";
          }
        }
      else if (xAxisType && !strcmp(xAxisType, "date"))
        {
        // JVM - Not currently handled.  Need to work out how the dates are
        // associated with each series.  This will get used for the
        // ticks.

        // all the MRML series get lumped into one jqPlot series
        // (need to surround the data with an extra [])
        if (idx == 0)
          {
          data << "[";
          }

        data << this->seriesBoxDataString(dn, idx);

        if (idx == arrayIDs->GetNumberOfValues()-1)
          {
          data << "]";
          }
        }

      if (idx < arrayIDs->GetNumberOfValues()-1)
        {
        data << ",";
        }
      }
    }

  data << "];";

  return data.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::boxXAxisTicks(vtkMRMLChartNode *cn)
{
  // JVM - Need to define ticks for case "date" axes

  QStringList ticks;

  vtkStringArray *arrayIDs = cn->GetArrays();
  vtkStringArray *arrayNames = cn->GetArrayNames();
  const char *xAxisType = cn->GetProperty("default", "xAxisType");

  if (!arrayIDs || arrayIDs->GetNumberOfValues() <= 0)
    {
    // no axis ticks by default
    }
  else if (xAxisType && !strcmp(xAxisType, "categorical"))
    {
    // define the ticks from the first curve (could do better)
    vtkMRMLDoubleArrayNode *dn = vtkMRMLDoubleArrayNode::SafeDownCast(this->MRMLScene->GetNodeByID( arrayIDs->GetValue(0).c_str() ));

    if (dn)
      {
      ticks << "var xAxisTicks = "
            << this->arrayTicksString(arrayNames)
            << ";";
      }
    }

  return ticks.join("");
}

//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::boxOptions(vtkMRMLChartNode *cn)
{
  // Since only series is sent to jqPlot, the options are a bit different
  QStringList options;

  vtkStringArray *arrayIDs = cn->GetArrays();
  const char *xAxisType = cn->GetProperty("default", "xAxisType");

  options << "var options = {";

  // Do we need to rotate the x-axis tick labels?
  bool rotateXTickLabels = false;
  if (xAxisType && !strcmp(xAxisType, "categorical"))
    {
    if (arrayIDs->GetSize() > 6)
      {
      rotateXTickLabels = true;
      }
    }

  // add the options that are the same for all charts
  options << this->genericOptions(cn, rotateXTickLabels);

  // Use a default set of colors defined by Slicer or specified by the
  // chart node.
  const char* defaultChartColorNodeID =
    this->ColorLogic ? this->ColorLogic->GetDefaultChartColorNodeID() : nullptr;
  vtkMRMLColorNode *defaultColorNode = vtkMRMLColorNode::SafeDownCast(
    this->MRMLScene->GetNodeByID(defaultChartColorNodeID));
  vtkMRMLColorNode *colorNode = defaultColorNode;
  const char *lookupTable = cn->GetProperty("default", "lookupTable");
  if (lookupTable)
    {
    colorNode = vtkMRMLColorNode::SafeDownCast(this->MRMLScene->GetNodeByID(lookupTable));
    }

  if (colorNode)
    {
    options << ", seriesColors: " << this->seriesColorsString(colorNode);
    }

  // default properties for a series
  //
  //
  options << ", seriesDefaults: {show: true";

  // chart type
  options << ", renderer: $.jqplot.BoxplotRenderer";

  // end of seriesDefaults properties
  options << "}";

  // series level properties. only one jqplot series in a box plot
  //
  //
  options << ", series: [";
  options << "{";

  // color
  const char *color = cn->GetProperty("default", "color");

  if (color)
    {
    options << ", color: '" << color << "'";
    }

  // end of a series
  options << "}";
  // end of series properties
  options << "]";
  // end of properties
  options << "};";

  return options.join("");
}



//---------------------------------------------------------------------------
QString qMRMLChartViewPrivate::seriesBoxDataString(vtkMRMLDoubleArrayNode *dn, unsigned int idx)
{
  QStringList data;

  data << "[";

  if (dn)
    {
    // Find the median, first and third quantiles, high, low values
    // (e.g. guards:  q1 - 1.5 IQR, q3 + 1.5 IQR).
    //
    // first quantile is the median of the lower half of the data
    // third quantile is the median of the upper half of the data
    double x, y;
    double q1, q3, median, low, high;

    // copy data into an STL vector
    std::vector<double> values;
    for (unsigned int j = 0; j < dn->GetSize(); ++j)
      {
      dn->GetXYValue(j, &x, &y);
      values.push_back(y);
      }

    // find the median position
    std::vector<double>::iterator medIt = values.begin() + values.size() / 2;
    std::nth_element(values.begin(), medIt, values.end());

    // calculate the median value and define the quantile positions
    std::vector<double>::iterator q1It, q3It;
    std::vector<double>::iterator q1BeginIt, q1EndIt, q3BeginIt, q3EndIt;
    if (values.size() % 2 == 1)
      {
      // odd number of elements, just select median
      median = *medIt;

      // median is a datum, include it in both halves
      q1BeginIt = values.begin();
      q1EndIt = medIt + 1; // one past median so median is included

      q3BeginIt = medIt; // median is included
      q3EndIt = values.end();
      }
    else
      {
      // even number of values, median is the average of the middle
      // two items
      median = (*medIt + *(medIt-1))/2.0;

      // median is not a datum, exclude it from both halves
      q1BeginIt = values.begin();
      q1EndIt = medIt; // excludes the median

      q3BeginIt = medIt + 1; // median is excluded
      q3EndIt = values.end();
      }

    q1It = q1BeginIt + (q1EndIt - q1BeginIt)/2;
    q3It = q3BeginIt + (q3EndIt - q3BeginIt)/2;

    // find the quantiles
    //
    std::nth_element(q1BeginIt, q1It, q1EndIt);
    std::nth_element(q3BeginIt, q3It, q3EndIt);

    if ((q1EndIt - q1BeginIt) % 2 == 1)
      {
      // odd number of elements in lower half
      q1 = *q1It;
      }
    else
      {
      // even number of elements, average
      q1 = (*q1It + *(q1It-1))/2.0;
      }


    if ((q3EndIt - q3BeginIt) % 2 == 1)
      {
      // odd number of elements in upper half
      q3 = *q3It;
      }
    else
      {
      // even number of elements, average
      q3 = (*q3It + *(q3It-1))/2.0;
      }

    // find the fences
    double IQR;

    IQR = q3 - q1;
    low = q1 - 1.5 * IQR;
    high = q3 + 1.5 * IQR;

    // output the values....
    //

    // Output the values: index, low, q1, median, q3, high
    data << QString("%1").arg(idx+1) << ", "
         << QString("%1").arg(low) << ", "
         << QString("%1").arg(q1) << ", "
         << QString("%1").arg(median) << ", "
         << QString("%1").arg(q3) << ", "
         << QString("%1").arg(high);

    // Output any outliers, points outside [low, high].
    for (std::vector<double>::iterator vit = values.begin();
         vit != values.end(); ++vit)
      {
      if (*vit < low || *vit > high)
        {
        data << ", " << QString("%1").arg(*vit);
        }
      }
    }

  data << "]";

  return data.join("");
}


//---------------------------------------------------------------------------
void qMRMLChartViewPrivate::onDataMouseOver(int series, int pointidx, double x, double y)
{
  Q_Q(qMRMLChartView);

  //qDebug() << "Series: " << series << ", Pointid: " << pointidx << ": " << x << ", " << y;

  // map from series to MRML ID
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

  // Get the array ids
  vtkStringArray *arrayIDs = cn->GetArrays();

  // emit the real signal
  if (series >= 0 && series < arrayIDs->GetNumberOfValues())
    {
    //qDebug() << "Array: " << arrayIDs->GetValue(series) << ", Pointidx: " << pointidx << ": " << x << ", " << y;
    emit q->dataMouseOver(arrayIDs->GetValue(series), pointidx, x, y);
    }
}


//---------------------------------------------------------------------------
void qMRMLChartViewPrivate::onDataPointClicked(int series, int pointidx, double x, double y)
{
  Q_Q(qMRMLChartView);

  //qDebug() << "Series: " << series << ", Pointid: " << pointidx << ": " << x << ", " << y;

  // map from series to MRML ID
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

  // Get the array ids
  vtkStringArray *arrayIDs = cn->GetArrays();

  // emit the real signal
  if (series >= 0 && series < arrayIDs->GetNumberOfValues())
    {
    //qDebug() << "Array: " << arrayIDs->GetValue(series) << ", Pointidx: " << pointidx << ": " << x << ", " << y;
    emit q->dataPointClicked(arrayIDs->GetValue(series), pointidx, x, y);
    }
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
  this->setMRMLScene(nullptr);
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
    this->setMRMLChartViewNode(nullptr);
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

//---------------------------------------------------------------------------
void qMRMLChartView::setColorLogic(vtkMRMLColorLogic* colorLogic)
{
  Q_D(qMRMLChartView);
  d->ColorLogic = colorLogic;
}

//---------------------------------------------------------------------------
vtkMRMLColorLogic* qMRMLChartView::colorLogic()const
{
  Q_D(const qMRMLChartView);
  return d->ColorLogic;
}

//---------------------------------------------------------------------------
QSize qMRMLChartView::sizeHint()const
{
  // return a default size hint (invalid size)
  return QSize();
}
