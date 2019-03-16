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

#ifndef __qMRMLPlotViewControllerWidget_h
#define __qMRMLPlotViewControllerWidget_h

// CTK includes
#include <ctkVTKObject.h>

// qMRMLWidget includes
#include "qMRMLViewControllerBar.h"
class qMRMLPlotViewControllerWidgetPrivate;
class qMRMLPlotView;

// MRML includes
class vtkMRMLPlotViewNode;

///
/// qMRMLPlotViewControllerWidget offers controls to a Plot view
/// (vtkMRMLPlotChartNode and vtkMRMLPlotSeriesNode). This controller
/// allows for the content (data) and style (properties) of a plot to
/// be defined.
class QMRML_WIDGETS_EXPORT qMRMLPlotViewControllerWidget
  : public qMRMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Superclass typedef
  typedef qMRMLViewControllerBar Superclass;

  /// Constructors
  explicit qMRMLPlotViewControllerWidget(QWidget* parent = nullptr);
  ~qMRMLPlotViewControllerWidget() override;

  /// Set the label for the Plot view (abbreviation for the view name).
  void setViewLabel(const QString& newViewLabel);

  /// Get the label for the view (abbreviation for the view name).
  QString viewLabel()const;

public slots:
  /// Set the scene.
  void setMRMLScene(vtkMRMLScene* newScene) override;

  /// Set the PlotView with which this controller interacts.
  void setPlotView(qMRMLPlotView* PlotView);

  /// Set the PlotViewNode associated with this PlotViewController.
  /// PlotViewNodes are 1-to-1 with PlotViews
  void setMRMLPlotViewNode(vtkMRMLPlotViewNode* PlotViewNode);

  /// Adjust the chart viewer's field of view to match
  /// the extent of the chart axes.
  void fitPlotToAxes();

  /// Save the selected plot to a file
  void onSaveButton();

protected slots:
  void updateWidgetFromMRML();

private:
  Q_DECLARE_PRIVATE(qMRMLPlotViewControllerWidget);
  Q_DISABLE_COPY(qMRMLPlotViewControllerWidget);
};

#endif
