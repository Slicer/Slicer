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

#ifndef __qMRMLPlotChartPropertiesWidget_h
#define __qMRMLPlotChartPropertiesWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// Plots Widgets includes
#include "qSlicerPlotsModuleWidgetsExport.h"
#include "ui_qMRMLPlotChartPropertiesWidget.h"

class qMRMLPlotChartPropertiesWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLPlotChartNode;

class Q_SLICER_MODULE_PLOTS_WIDGETS_EXPORT qMRMLPlotChartPropertiesWidget : public qMRMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLPlotChartPropertiesWidget(QWidget* parent = nullptr);
  ~qMRMLPlotChartPropertiesWidget() override;

  /// Get \a PlotViewNode
  vtkMRMLPlotChartNode* mrmlPlotChartNode()const;

public slots:

  /// Set the scene.
  void setMRMLScene(vtkMRMLScene* newScene) override;

  /// Set a new PlotViewNode.
  void setMRMLPlotChartNode(vtkMRMLNode* node);

  /// Set a new PlotViewNode.
  void setMRMLPlotChartNode(vtkMRMLPlotChartNode* plotChartNode);

  /// Control the display of a grid in the chart.
  void setGridVisibility(bool show);

  /// Control the display of the legend in the chart.
  void setLegendVisibility(bool show);

  /// Set the title.
  /// \sa TitleVisibility
  void setTitle(const QString& str);

  /// Set the label along the X-Axis.
  /// \sa showXAxisLabel
  void setXAxisLabel(const QString& str);

  /// Set the label along the Y-Axis.
  /// \sa showYAxisLabel
  void setYAxisLabel(const QString& str);

  /// Change the type of font used in the plot.
  void setFontType(const QString& type);

  /// Change the font size of the title of the plot.
  void setTitleFontSize(double size);

  /// Change the font size of the legend of the plot.
  void setLegendFontSize(double size);

  /// Change the font size of the title of the axes.
  void setAxisTitleFontSize(double size);

  /// Change the font size of the labels of the axes.
  void setAxisLabelFontSize(double size);

  void setXAxisManualRangeEnabled(bool);
  void setXAxisRangeMin(double);
  void setXAxisRangeMax(double);

  void setYAxisManualRangeEnabled(bool);
  void setYAxisRangeMin(double);
  void setYAxisRangeMax(double);

  void setXAxisLogScale(bool);
  void setYAxisLogScale(bool);

signals:

  /// Signal fired when the user adds a new plot series node to the scene
  /// using the widget's plot series selector.
  void seriesNodeAddedByUser(vtkMRMLNode*);

protected:
  QScopedPointer<qMRMLPlotChartPropertiesWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLPlotChartPropertiesWidget);
  Q_DISABLE_COPY(qMRMLPlotChartPropertiesWidget);
};

#endif
