/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qMRMLColorLegendDisplayNodeWidget_h
#define __qMRMLColorLegendDisplayNodeWidget_h

// Slicer includes
#include <qMRMLWidget.h>

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Colors Widgets includes
#include "qSlicerColorsModuleWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLColorLegendDisplayNode;
class qMRMLColorLegendDisplayNodeWidgetPrivate;
class QAbstractButton;

/// \ingroup Slicer_QtModules_Colors
class Q_SLICER_MODULE_COLORS_WIDGETS_EXPORT qMRMLColorLegendDisplayNodeWidget
  : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  explicit qMRMLColorLegendDisplayNodeWidget(QWidget *parent=0);
  ~qMRMLColorLegendDisplayNodeWidget() override;

public slots:
  void setMRMLScene(vtkMRMLScene*) override;
  /// Set ColorLegendDisplay MRML node (Parameter node)
  /// Set color legend display node
  void setMRMLColorLegendDisplayNode(vtkMRMLColorLegendDisplayNode* node);
  /// Get color legend display node
  vtkMRMLColorLegendDisplayNode* mrmlColorLegendDisplayNode();
  /// Utility function to be connected with generic signals
  void setMRMLColorLegendDisplayNode(vtkMRMLNode* node);

  void onColorLegendVisibilityToggled(bool);

  void onTitleTextChanged(const QString&);

  void onLabelFormatChanged(const QString&);

  void onMaximumNumberOfColorsChanged(int);
  void onNumberOfLabelsChanged(int);

protected slots:
  /// Update widget GUI from color legend parameters node
  void updateWidgetFromMRML();

  void onColorLegendOrientationButtonClicked(QAbstractButton*);
  void onLabelTextButtonClicked(QAbstractButton*);
  void onPositionChanged();
  void onSizeChanged();


protected:
  QScopedPointer<qMRMLColorLegendDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLColorLegendDisplayNodeWidget);
  Q_DISABLE_COPY(qMRMLColorLegendDisplayNodeWidget);
};

#endif
