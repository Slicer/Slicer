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

// Qt includes
#include <QWidget>
class QResizeEvent;

// qMRMLWidget includes
#include "qMRMLWidget.h"
class qMRMLThreeDViewControllerWidget;
class qMRMLThreeDView;
class qMRMLThreeDWidgetPrivate;

// MRML includes
class vtkMRMLScene;
class vtkMRMLViewNode;

// MRMLLogic includes
class vtkMRMLViewLogic;

// VTK includes
class vtkCollection;

class QMRML_WIDGETS_EXPORT qMRMLThreeDWidget : public qMRMLWidget
{
  Q_OBJECT
  Q_PROPERTY(QString viewLabel READ viewLabel WRITE setViewLabel)
  Q_PROPERTY(QColor viewColor READ viewColor WRITE setViewColor)

public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLThreeDWidget(QWidget* parent = nullptr);
  ~qMRMLThreeDWidget() override;

  /// Get slice controller
  Q_INVOKABLE qMRMLThreeDViewControllerWidget* threeDController()const;

  /// Get the 3D View node observed by view.
  Q_INVOKABLE vtkMRMLViewNode* mrmlViewNode()const;

  /// \sa qMRMLSliceControllerWidget::viewLogic()
  Q_INVOKABLE vtkMRMLViewLogic* viewLogic()const;

  /// Get a reference to the underlying ThreeD View
  /// Becareful if you change the threeDView, you might
  /// unsynchronize the view from the nodes/logics.
  Q_INVOKABLE qMRMLThreeDView* threeDView()const;

  /// \sa qMRMLThreeDView::addDisplayableManager
  Q_INVOKABLE void addDisplayableManager(const QString& displayableManager);
  Q_INVOKABLE void getDisplayableManagers(vtkCollection* displayableManagers);

  /// \sa qMRMLThreeDViewControllerWidget::viewLabel()
  /// \sa setiewLabel()
  QString viewLabel()const;

  /// \sa qMRMLThreeDViewControllerWidget::viewLabel()
  /// \sa viewLabel()
  void setViewLabel(const QString& newViewLabel);

  /// \sa qMRMLThreeDViewControllerWidget::setQuadBufferStereoSupportEnabled
  Q_INVOKABLE void setQuadBufferStereoSupportEnabled(bool value);

  /// \sa qMRMLThreeDViewControllerWidget::viewColor()
  /// \sa setViewColor()
  QColor viewColor()const;

  /// \sa qMRMLThreeDViewControllerWidget::viewColor()
  /// \sa viewColor()
  void setViewColor(const QColor& newViewColor);

  /// propagates the logics to the qMRMLThreeDControllerWidget
  Q_INVOKABLE void setViewLogics(vtkCollection* logics);

public slots:
  void setMRMLScene(vtkMRMLScene* newScene) override;

  /// Set the current \a viewNode to observe
  void setMRMLViewNode(vtkMRMLViewNode* newViewNode);

protected:
  QScopedPointer<qMRMLThreeDWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLThreeDWidget);
  Q_DISABLE_COPY(qMRMLThreeDWidget);
};

#endif
