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

#ifndef __qMRMLPlotSeriesPropertiesWidget_p_h
#define __qMRMLPlotSeriesPropertiesWidget_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLPlotSeriesPropertiesWidget.h"
#include "ui_qMRMLPlotSeriesPropertiesWidget.h"

/// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkImageData.h>

class QAction;
class ctkVTKPlotViewView;
class vtkMRMLPlotChartNode;
class vtkMRMLPlotSeriesNode;
class vtkMRMLPlotViewNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLPlotSeriesPropertiesWidgetPrivate
 : public QObject
 , public Ui_qMRMLPlotSeriesPropertiesWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLPlotSeriesPropertiesWidget);
protected:
  qMRMLPlotSeriesPropertiesWidget* const q_ptr;
public:
  qMRMLPlotSeriesPropertiesWidgetPrivate(qMRMLPlotSeriesPropertiesWidget& object);
  ~qMRMLPlotSeriesPropertiesWidgetPrivate() override;

  void setupUi(qMRMLWidget* widget);

  vtkMRMLPlotChartNode* GetPlotChartNodeFromView();

public slots:

  /// Update widget state using the associated MRML PlotSeries node.
  void updateWidgetFromMRML();

  /// Called after a PlotSeriesNode is selected
  /// using the associated qMRMLNodeComboBox.
  void onPlotSeriesNodeChanged(vtkMRMLNode* node);

  /// Called after a TableNode is selected
  /// using the associated qMRMLNodeComboBox.
  void onInputTableNodeChanged(vtkMRMLNode* node);

  /// Change the x-Axis of the plot.
  void onXAxisChanged(int index);

  /// Change the labels of the plot.
  void onLabelsChanged(int index);

  /// Change the y-Axis of the plot.
  void onYAxisChanged(int index);

  /// Change the type of plot (scatter, line, bar).
  void onPlotTypeChanged(int index);

  /// Change markers style for Line and Scatter plots.
  void onMarkersStyleChanged(const QString& style);

  /// Change markers size for Line and Scatter plots.
  void onMarkersSizeChanged(double size);

  /// Change line style for Line and Scatter plots.
  void onLineStyleChanged(const QString &style);

  /// Change line width for Line and Scatter plots.
  void onLineWidthChanged(double width);

  /// Change the color of a single PlotSeriesNode.
  void onPlotSeriesColorChanged(const QColor& color);

public:
  vtkWeakPointer<vtkMRMLPlotViewNode>    PlotViewNode;
  vtkWeakPointer<vtkMRMLPlotSeriesNode>    PlotSeriesNode;

};

#endif
