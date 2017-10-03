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

#ifndef __qMRMLPlotViewInformationWidget_p_h
#define __qMRMLPlotViewInformationWidget_p_h

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
#include "qMRMLPlotViewInformationWidget.h"
#include "ui_qMRMLPlotViewInformationWidget.h"

/// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkImageData.h>

class QAction;
class ctkVTKPlotViewView;
class vtkMRMLPlotChartNode;
class vtkMRMLPlotDataNode;
class vtkMRMLPlotViewNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLPlotViewInformationWidgetPrivate
 : public QObject
 , public Ui_qMRMLPlotViewInformationWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLPlotViewInformationWidget);
protected:
  qMRMLPlotViewInformationWidget* const q_ptr;
public:
  qMRMLPlotViewInformationWidgetPrivate(qMRMLPlotViewInformationWidget& object);
  ~qMRMLPlotViewInformationWidgetPrivate();

  void setupUi(qMRMLWidget* widget);

  vtkMRMLPlotChartNode* GetPlotChartNodeFromView();

public slots:
  /// Update widget state using the associated MRML PlotView node.
  void updateWidgetFromMRMLPlotViewNode();

  /// Called after a ColorNode is selected
  /// using the associated qMRMLNodeComboBox.
  void onColorNodeChanged(vtkMRMLNode* node);

  /// Change the type of font used in the plot.
  void onFontTypeChanged(const QString& type);

  /// Change the font size of the title of the plot.
  void onTitleFontSizeChanged(double size);

  /// Change the font size of the title of the axes.
  void onAxisTitleFontSizeChanged(double size);

  /// Change the font size of the labels of the axes.
  void onAxisLabelFontSizeChanged(double size);

  /// Activate Click and Drag for x-Axis.
  void activateClickAndDragX(bool activate);

  /// Activate Click and Drag for y-Axis.
  void activateClickAndDragY(bool activate);

  /// Update widget state using the associated MRML PlotData node.
  void updateWidgetFromMRMLPlotDataNode();

  /// Called after a PlotDataNode is selected
  /// using the associated qMRMLNodeComboBox.
  void onPlotDataNodeChanged(vtkMRMLNode* node);

  /// Called after a TableNode is selected
  /// using the associated qMRMLNodeComboBox.
  void onInputTableNodeChanged(vtkMRMLNode* node);

  /// Change the x-Axis of the plot.
  void onXAxisChanged(const QString& xAxis);

  /// Change the y-Axis of the plot.
  void onYAxisChanged(const QString& yAxis);

  /// Change the type of plot (Line, Scatter, Bar).
  void onPlotTypeChanged(const QString& type);

  /// Change markers style for Line and Scatter plots.
  void onMarkersStyleChanged(const QString& style);

  /// Change markers size for Line and Scatter plots.
  void onMarkersSizeChanged(double size);

  /// Change line widht for Line plots.
  void onLineWidthChanged(double width);

  /// Change the color of a single PlotDataNode.
  void onPlotDataColorChanged(const QColor& color);

  /// Copy the current PlotDataNode.
  /// Additionally, the node reference will be added
  /// to the active PlotChartNode.
  void onCopyPlotDataNodeClicked();

public:
  vtkWeakPointer<vtkMRMLPlotViewNode>    PlotViewNode;
  vtkWeakPointer<vtkMRMLPlotDataNode>    PlotDataNode;

};

#endif
