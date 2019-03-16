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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLDisplayNodeViewComboBox_h
#define __qMRMLDisplayNodeViewComboBox_h

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLCheckableNodeComboBox.h"
class qMRMLDisplayNodeViewComboBoxPrivate;

// VTK includes
class vtkMRMLDisplayNode;
class vtkMRMLAbstractViewNode;

/// \brief Combobox of display node view nodes.
/// Observe the view nodes of a display node and mark them as checked in the
/// scene view node list. View nodes can be vtkMRMLViewNode for the 3D view
/// or vtkMRMLSliceNode for the 2d case, or vtkMRMLChartViewNodes for charts
/// \sa vtkMRMLDisplayNode::GetNthViewNodeID()
class QMRML_WIDGETS_EXPORT qMRMLDisplayNodeViewComboBox
  : public qMRMLCheckableNodeComboBox
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLCheckableNodeComboBox Superclass;

  /// Construct an empty qMRMLDisplayNodeViewComboBox with a null scene,
  /// no nodeType, where the hidden nodes are not forced on display.
  explicit qMRMLDisplayNodeViewComboBox(QWidget* parent = nullptr);
  ~qMRMLDisplayNodeViewComboBox() override;

  vtkMRMLDisplayNode* mrmlDisplayNode()const;

  /// Return a list of view nodes the display node is visible into.
  QList<vtkMRMLAbstractViewNode*> checkedViewNodes()const;
  /// Return a list of view nodes the display node is not visible into.
  QList<vtkMRMLAbstractViewNode*> uncheckedViewNodes()const;

public slots:
  /// Set the display node to observe.
  void setMRMLDisplayNode(vtkMRMLDisplayNode* node);
  /// Utility function to conveniently connect the combobox with other
  /// qMRMLWidgets.
  void setMRMLDisplayNode(vtkMRMLNode* displayNode);

protected slots:
  void updateWidgetFromMRML();
  void updateMRMLFromWidget();

protected:
  QScopedPointer<qMRMLDisplayNodeViewComboBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLDisplayNodeViewComboBox);
  Q_DISABLE_COPY(qMRMLDisplayNodeViewComboBox);
};

#endif
