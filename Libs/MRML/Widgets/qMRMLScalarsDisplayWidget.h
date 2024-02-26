
/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2020 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qMRMLScalarsDisplayWidget_h
#define __qMRMLScalarsDisplayWidget_h

// MRMLWidgets includes
#include "qMRMLWidget.h"

// MRML includes
#include "vtkMRMLDisplayNode.h"

// CTK includes
#include <ctkVTKObject.h>

class qMRMLScalarsDisplayWidgetPrivate;
class vtkMRMLColorNode;
class vtkMRMLNode;

class QMRML_WIDGETS_EXPORT qMRMLScalarsDisplayWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

  Q_PROPERTY(vtkMRMLDisplayNode::ScalarRangeFlagType scalarRangeMode READ scalarRangeMode WRITE setScalarRangeMode)

public:
  /// Constructors
  typedef qMRMLWidget Superclass;
  explicit qMRMLScalarsDisplayWidget(QWidget* parentWidget = nullptr);
  ~qMRMLScalarsDisplayWidget() override;

  /// Get the (first) current display node
  vtkMRMLDisplayNode* mrmlDisplayNode() const;
  /// Get current display nodes. This is for supporting changing properties
  /// in case of multiple nodes are selected (e.g., in subject hierarchy folders).
  QList<vtkMRMLDisplayNode*> mrmlDisplayNodes() const;

  bool scalarsVisibility() const;
  QString activeScalarName() const;
  vtkMRMLColorNode* scalarsColorNode() const;

  /// Set scalar range mode
  void setScalarRangeMode(vtkMRMLDisplayNode::ScalarRangeFlagType mode);
  vtkMRMLDisplayNode::ScalarRangeFlagType scalarRangeMode() const;

  /// Get minimum of the scalar display range
  double minimumValue() const;

  /// Get maximum of the scalar display range
  double maximumValue() const;

signals:
  /// Signal sent if the auto/manual value is updated
  void scalarRangeModeValueChanged(vtkMRMLDisplayNode::ScalarRangeFlagType mode);
  /// Signal sent if the any property in the display node is changed
  void displayNodeChanged();

public slots:
  /// Set the one display node
  void setMRMLDisplayNode(vtkMRMLDisplayNode* node);
  /// Utility function to be connected with generic signals
  void setMRMLDisplayNode(vtkMRMLNode* node);
  /// Set the current display nodes if more are managed.
  /// In case of multi-selection, the first item's display properties are
  /// displayed in the widget, but the changed settings are applied on all
  /// selected items if applicable.
  void setMRMLDisplayNodes(QList<vtkMRMLDisplayNode*> displayNodes);

  void setScalarsVisibility(bool);
  void setActiveScalarName(const QString&);
  void setScalarsColorNode(vtkMRMLNode*);
  void setScalarsColorNode(vtkMRMLColorNode*);
  void setScalarsDisplayRange(double min, double max);
  void setTresholdEnabled(bool b);
  void setThresholdRange(double min, double max);

  /// Set Auto/Manual mode
  void setScalarRangeMode(int scalarRangeMode);

  /// Set min/max of scalar range
  void setMinimumValue(double min);
  void setMaximumValue(double max);

protected slots:
  /// Update the widget from volume display node properties
  void updateWidgetFromMRML();

  /// Set active scalar when the user changes selection
  void onCurrentArrayActivated();

protected:
  QScopedPointer<qMRMLScalarsDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLScalarsDisplayWidget);
  Q_DISABLE_COPY(qMRMLScalarsDisplayWidget);
  friend class qMRMLModelDisplayNodeWidget;
  friend class qMRMLMarkupsDisplayNodeWidget;
};

#endif
