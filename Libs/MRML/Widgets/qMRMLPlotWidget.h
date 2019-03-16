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
#include "qMRMLWidget.h"
class qMRMLPlotViewControllerWidget;
class qMRMLPlotView;
class qMRMLPlotWidgetPrivate;

// MRML includes
class vtkMRMLPlotViewNode;
class vtkMRMLScene;

/// \brief qMRMLPlotWidget is the toplevel Plotting widget that can be
/// packed in a layout.
///
/// qMRMLPlotWidget provides plotting capabilities with a display
/// canvas for the plot and a controller widget to control the
/// content and properties of the plot.
class QMRML_WIDGETS_EXPORT qMRMLPlotWidget : public qMRMLWidget
{
  Q_OBJECT
  Q_PROPERTY(QString viewLabel READ viewLabel WRITE setViewLabel)
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLPlotWidget(QWidget* parent = nullptr);
  ~qMRMLPlotWidget() override;

  /// Get the Plot node observed by view.
  Q_INVOKABLE vtkMRMLPlotViewNode* mrmlPlotViewNode()const;

  /// Get a reference to the underlying Plot View
  /// Becareful if you change the PlotView, you might
  /// unsynchronize the view from the nodes/logics.
  Q_INVOKABLE qMRMLPlotView* plotView()const;

    /// Get plot view controller widget
  Q_INVOKABLE qMRMLPlotViewControllerWidget* plotController()const;

  /// Get the view label for the Plot.
  /// \sa qMRMLPlotControllerWidget::PlotViewLabel()
  /// \sa setPlotViewLabel()
  QString viewLabel()const;

  /// Set the view label for the Plot.
  /// \sa qMRMLPlotControllerWidget::PlotViewLabel()
  /// \sa PlotViewLabel()
  void setViewLabel(const QString& newPlotViewLabel);

public slots:
  /// Set the current \a viewNode to observe
  void setMRMLPlotViewNode(vtkMRMLPlotViewNode* newPlotViewNode);

protected:
  QScopedPointer<qMRMLPlotWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLPlotWidget);
  Q_DISABLE_COPY(qMRMLPlotWidget);
};

#endif
