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

#ifndef __qMRMLChartViewControllerWidget_h
#define __qMRMLChartViewControllerWidget_h

// CTK includes
#include <ctkVTKObject.h>

// qMRMLWidget includes
#include "qMRMLViewControllerBar.h"
class qMRMLChartViewControllerWidgetPrivate;
class qMRMLChartView;

// MRML includes
class vtkMRMLChartViewNode;

class QMRML_WIDGETS_EXPORT qMRMLChartViewControllerWidget
  : public qMRMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Superclass typedef
  typedef qMRMLViewControllerBar Superclass;

  /// Constructors
  explicit qMRMLChartViewControllerWidget(QWidget* parent = 0);
  virtual ~qMRMLChartViewControllerWidget();

  /// Set the label for the chart view (abbreviation for the view
  /// name)
  void setViewLabel(const QString& newViewLabel);

  /// Get the label for the view (abbreviation for the view name)
  QString viewLabel()const;

public slots:
  virtual void setMRMLScene(vtkMRMLScene* newScene);
  void setChartView(qMRMLChartView* ChartView);
  void setMRMLChartViewNode(vtkMRMLChartViewNode* chartViewNode);
  void showLines(bool show);
  void showMarkers(bool show);
  void showGrid(bool show);
  void showLegend(bool show);
  void showTitle(bool show);
  void showXAxisLabel(bool show);
  void showYAxisLabel(bool show);
  void setTitle(const QString&);
  void setXAxisLabel(const QString&);
  void setYAxisLabel(const QString&);
  void editTitle();
  void editXAxisLabel();
  void editYAxisLabel();

protected slots:
  void updateWidgetFromMRML();

private:
  Q_DECLARE_PRIVATE(qMRMLChartViewControllerWidget);
  Q_DISABLE_COPY(qMRMLChartViewControllerWidget);
};

#endif
