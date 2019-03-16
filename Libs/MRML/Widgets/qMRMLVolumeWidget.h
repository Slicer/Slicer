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

#ifndef __qMRMLVolumeWidget_h
#define __qMRMLVolumeWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>
#include "qMRMLWidgetsExport.h"

class vtkMRMLNode;
class vtkMRMLScalarVolumeDisplayNode;
class vtkMRMLScalarVolumeNode;
class qMRMLVolumeWidgetPrivate;

/// \brief Abstract widget to represent and control the properties of a scalar
/// volume node.
class QMRML_WIDGETS_EXPORT qMRMLVolumeWidget : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  /// Constructors
  typedef QWidget Superclass;
  explicit qMRMLVolumeWidget(QWidget* parentWidget = nullptr);
  ~qMRMLVolumeWidget() override;

  ///
  /// Return the current MRML node of interest
  vtkMRMLScalarVolumeNode* mrmlVolumeNode()const;

public slots:
  /// Set the volume to observe
  void setMRMLVolumeNode(vtkMRMLScalarVolumeNode* displayNode);

  /// Utility slot to conveniently connect a SIGNAL(vtkMRMLNode*) with the
  /// widget.
  void setMRMLVolumeNode(vtkMRMLNode* node);

protected slots:
  /// Update the widget from volume node properties.
  /// The slot gets called when a volume node is set or when
  /// it has been modified.
  /// To be reimplemented in subclasses.
  /// \sa updateWidgetFromMRMLDisplayNode(), setMRMLVolumeNode()
  virtual void updateWidgetFromMRMLVolumeNode();

  /// Update the widget from volume display node properties.
  /// The slot gets called when a volume display node is set or when
  /// it has been modified.
  /// \sa updateWidgetFromMRMLDisplayNode(), setMRMLVolumeDisplayNode()
  virtual void updateWidgetFromMRMLDisplayNode();

protected:
  QScopedPointer<qMRMLVolumeWidgetPrivate> d_ptr;
  qMRMLVolumeWidget(qMRMLVolumeWidgetPrivate* ptr,
                    QWidget* parentWidget = nullptr);

  /// Return the volume display node.
  vtkMRMLScalarVolumeDisplayNode* mrmlDisplayNode()const;

private:
  Q_DECLARE_PRIVATE(qMRMLVolumeWidget);
  Q_DISABLE_COPY(qMRMLVolumeWidget);

  /// Observe volume display node
  void setMRMLVolumeDisplayNode(vtkMRMLScalarVolumeDisplayNode* displayNode);
};

#endif
