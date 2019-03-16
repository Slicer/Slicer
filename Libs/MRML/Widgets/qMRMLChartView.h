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

#ifndef __qMRMLChartView_h
#define __qMRMLChartView_h

// Qt includes
#include <QWebEngineView>

#include "qMRMLWidgetsExport.h"

class qMRMLChartViewPrivate;

// MRML includes
class vtkMRMLChartViewNode;
class vtkMRMLColorLogic;
class vtkMRMLScene;

/// \brief qMRMLChartView is the display canvas for a Chart.
///
/// qMRMLChartView is currently implemented as a subclass of QWebView and charting
/// is implemented using a jQuery library called jqPlot. This is
/// subject to being made opaque, so that qMRMLChartView is merely a
/// subclass of QWidget and internally a variety of implementations
/// for charting may be provided.
class QMRML_WIDGETS_EXPORT qMRMLChartView :
    public QWebEngineView
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWebEngineView Superclass;

  /// Constructors
  explicit qMRMLChartView(QWidget* parent = nullptr);
  ~qMRMLChartView() override;

  /// Return a pointer on the current MRML scene
  vtkMRMLScene* mrmlScene() const;

  /// Get the ChartView node observed by view.
  vtkMRMLChartViewNode* mrmlChartViewNode()const;

  /// Set the application color logic for default node
  /// color.
  void setColorLogic(vtkMRMLColorLogic* colorLogic);

  /// Get the application color logic. 0 by default.
  vtkMRMLColorLogic* colorLogic()const;

  /// Redefine the sizeHint so layouts work properly.
  QSize sizeHint() const override;

public slots:

  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* newScene);

  /// Set the current \a viewNode to observe
  void setMRMLChartViewNode(vtkMRMLChartViewNode* newChartViewNode);

signals:

  /// Signal emitted when mouse moves over a data point. Returns the
  /// id of the MRMLDoubleArrayNode, the index of the point, and the values
  void dataMouseOver(const char *mrmlArrayID, int pointidx, double x, double y);

  /// Signal emitted when a data point has been clicked. Returns the
  /// id of the MRMLDoubleArrayNode, the index of the point, and the values
  void dataPointClicked(const char *mrmlArrayID, int pointidx, double x, double y);

  /// When designing custom qMRMLWidget in the designer, you can connect the
  /// mrmlSceneChanged signal directly to the aggregated MRML widgets that
  /// have a setMRMLScene slot.
  void mrmlSceneChanged(vtkMRMLScene*);

protected:
  QScopedPointer<qMRMLChartViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLChartView);
  Q_DISABLE_COPY(qMRMLChartView);
};

#endif
