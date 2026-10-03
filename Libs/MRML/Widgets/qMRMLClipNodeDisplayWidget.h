/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/

#ifndef __qMRMLClipNodeDisplayWidget_h
#define __qMRMLClipNodeDisplayWidget_h

// Qt includes
#include <qMRMLWidget.h>

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"
#include "vtkMRMLClipNode.h"

class qMRMLClipNodeDisplayWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLDisplayNode;

/// \brief Clipping settings of a display node: clipping on/off, the nodes that clip it and how (from its clip node),
/// cap and outline of the clipped surface. The clip node is chosen next to the clipping check box.
/// When clipping is enabled and the display node has no clip node, the clip node of the scene is selected
/// (one is created if there is none).
/// Widgets that contain it can add options that are specific to their display node type, see addRow().

class QMRML_WIDGETS_EXPORT qMRMLClipNodeDisplayWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  qMRMLClipNodeDisplayWidget(QWidget* parent = nullptr);
  ~qMRMLClipNodeDisplayWidget() override;

  vtkMRMLDisplayNode* mrmlDisplayNode() const;

  /// Make sure that the display node has a clip node: if it has none, then use the first clip node
  /// of the scene (usually there is only one, used by all nodes), or create one if the scene has none.
  /// Returns the clip node of the display node (nullptr if the display node is not in a scene).
  static vtkMRMLClipNode* ensureClipNode(vtkMRMLDisplayNode* displayNode);

  /// Add a widget (for example, a status button) next to the clip node selector.
  /// The widget gets the height of the clip node selector (a button is made square, with its icon fitting in it).
  Q_INVOKABLE void addWidgetNextToClipNodeSelector(QWidget* widget);

  //@{
  /// Add a row directly below the row of the clip node selector (for example, details of the status
  /// that a widget next to the selector shows). Rows are added in the order of the calls.
  /// If label is nullptr then the field is only in the second column.
  /// The added widgets are reparented to this widget.
  Q_INVOKABLE void addRowBelowClipNodeSelector(const QString& labelText, QWidget* field);
  Q_INVOKABLE void addRowBelowClipNodeSelector(QWidget* label, QWidget* field);
  //@}

  //@{
  /// Add a row below the clipping options, for options that are specific to the type of the display node
  /// (for example, soft edge of volume rendering).
  /// If label is nullptr then the field is only in the second column.
  /// The added widgets are reparented to this widget.
  Q_INVOKABLE void addRow(const QString& labelText, QWidget* field);
  Q_INVOKABLE void addRow(QWidget* label, QWidget* field);
  /// Add a widget that spans both columns of the row
  Q_INVOKABLE void addRow(QWidget* widget);
  //@}

public slots:
  /// Set the clip node to represent
  void setMRMLDisplayNode(vtkMRMLDisplayNode* node);
  /// Utility function to be connected to signals/slots
  void setMRMLDisplayNode(vtkMRMLNode* node);

protected slots:
  void updateWidgetFromMRML();
  void updateMRMLFromWidget();
  void onClippingToggled(bool enabled);
  void onClipNodeSelected(vtkMRMLNode* node);

protected:
  QScopedPointer<qMRMLClipNodeDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLClipNodeDisplayWidget);
  Q_DISABLE_COPY(qMRMLClipNodeDisplayWidget);
};

#endif
