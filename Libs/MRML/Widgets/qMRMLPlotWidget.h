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

#ifndef __qMRMLPlotWidget_h
#define __qMRMLPlotWidget_h

// Qt includes
#include <QWidget>
class QResizeEvent;

// qMRMLWidget includes
#include "qMRMLAbstractViewWidget.h"
class qMRMLPlotViewControllerWidget;
class qMRMLPlotView;
class qMRMLPlotWidgetPrivate;

// MRML includes
class vtkMRMLPlotViewNode;
class vtkMRMLScene;

/// \brief qMRMLPlotWidget is the top-level Plotting widget that can be
/// packed in a layout.
///
/// qMRMLPlotWidget provides plotting capabilities with a display
/// canvas for the plot and a controller widget to control the
/// content and properties of the plot.
class QMRML_WIDGETS_EXPORT qMRMLPlotWidget : public qMRMLAbstractViewWidget
{
  Q_OBJECT

public:
  /// Superclass typedef
  typedef qMRMLAbstractViewWidget Superclass;

  /// Constructors
  explicit qMRMLPlotWidget(QWidget* parent = nullptr);
  ~qMRMLPlotWidget() override;

  /// Get the Plot node observed by view.
  Q_INVOKABLE vtkMRMLPlotViewNode* mrmlPlotViewNode() const;
  Q_INVOKABLE vtkMRMLAbstractViewNode* mrmlAbstractViewNode() const override;

  /// Get a reference to the underlying Plot View
  /// Be careful if you change the PlotView, you might
  /// unsynchronize the view from the nodes/logics.
  Q_INVOKABLE qMRMLPlotView* plotView() const;
  Q_INVOKABLE QWidget* viewWidget() const override;

  /// Get plot view controller widget
  Q_INVOKABLE qMRMLPlotViewControllerWidget* plotController() const;
  Q_INVOKABLE qMRMLViewControllerBar* controllerWidget() const override;

public slots:
  /// Set the current \a viewNode to observe
  void setMRMLPlotViewNode(vtkMRMLPlotViewNode* newPlotViewNode);
  void setMRMLAbstractViewNode(vtkMRMLAbstractViewNode* newViewNode) override;

protected:
  QScopedPointer<qMRMLPlotWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLPlotWidget);
  Q_DISABLE_COPY(qMRMLPlotWidget);
};

#endif
