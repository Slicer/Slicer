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

#ifndef __qMRMLThreeDWidget_h
#define __qMRMLThreeDWidget_h

// qMRMLWidget includes
#include "qMRMLAbstractViewWidget.h"
class qMRMLThreeDViewControllerWidget;
class qMRMLThreeDView;
class qMRMLThreeDWidgetPrivate;
class qMRMLViewControllerBar;

// MRML includes
class vtkMRMLViewNode;

// MRMLLogic includes
class vtkMRMLViewLogic;

// VTK includes
class vtkCollection;

class QMRML_WIDGETS_EXPORT qMRMLThreeDWidget : public qMRMLAbstractViewWidget
{
  Q_OBJECT

public:
  /// Superclass typedef
  typedef qMRMLAbstractViewWidget Superclass;

  /// Constructors
  explicit qMRMLThreeDWidget(QWidget* parent = nullptr);
  ~qMRMLThreeDWidget() override;

  /// Get slice controller
  Q_INVOKABLE qMRMLThreeDViewControllerWidget* threeDController() const;
  Q_INVOKABLE qMRMLViewControllerBar* controllerWidget() const override;

  /// Get the 3D View node observed by view.
  Q_INVOKABLE vtkMRMLViewNode* mrmlViewNode() const;
  Q_INVOKABLE vtkMRMLAbstractViewNode* mrmlAbstractViewNode() const override;

  /// \sa qMRMLSliceControllerWidget::viewLogic()
  Q_INVOKABLE vtkMRMLViewLogic* viewLogic() const;
  Q_INVOKABLE vtkMRMLAbstractLogic* logic() const override;

  /// Get a reference to the underlying ThreeD View
  /// Be careful if you change the threeDView, you might
  /// unsynchronize the view from the nodes/logics.
  Q_INVOKABLE qMRMLThreeDView* threeDView() const;
  Q_INVOKABLE QWidget* viewWidget() const override;

  /// \sa qMRMLThreeDView::addDisplayableManager
  Q_INVOKABLE void addDisplayableManager(const QString& displayableManager);
  Q_INVOKABLE void getDisplayableManagers(vtkCollection* displayableManagers);

  /// \sa qMRMLThreeDViewControllerWidget::setQuadBufferStereoSupportEnabled
  Q_INVOKABLE void setQuadBufferStereoSupportEnabled(bool value);

public slots:
  /// Set the current \a viewNode to observe
  void setMRMLViewNode(vtkMRMLViewNode* newViewNode);
  void setMRMLAbstractViewNode(vtkMRMLAbstractViewNode* newViewNode) override;

protected:
  QScopedPointer<qMRMLThreeDWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLThreeDWidget);
  Q_DISABLE_COPY(qMRMLThreeDWidget);
};

#endif
