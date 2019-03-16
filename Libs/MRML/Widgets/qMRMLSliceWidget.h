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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLSliceWidget_h
#define __qMRMLSliceWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <vtkVersion.h>

// qMRMLWidget includes
#include "qMRMLWidget.h"

#include "qMRMLWidgetsExport.h"

class qMRMLSliceView;
class qMRMLSliceWidgetPrivate;
class qMRMLSliceControllerWidget;
class vtkCollection;
class vtkMRMLScene;
class vtkMRMLNode;
class vtkMRMLSliceLogic;
class vtkMRMLSliceNode;
class vtkMRMLSliceCompositeNode;

class vtkAlgorithmOutput;
class vtkImageData;
class vtkInteractorObserver;
class vtkCornerAnnotation;
class vtkCollection;

class QMRML_WIDGETS_EXPORT qMRMLSliceWidget : public qMRMLWidget
{
  Q_OBJECT
  Q_PROPERTY(QString sliceOrientation READ sliceOrientation WRITE setSliceOrientation)
  Q_PROPERTY(QString sliceViewName READ sliceViewName WRITE setSliceViewName)
  Q_PROPERTY(QString sliceViewLabel READ sliceViewLabel WRITE setSliceViewLabel)
  Q_PROPERTY(QColor sliceViewColor READ sliceViewColor WRITE setSliceViewColor)

public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLSliceWidget(QWidget* parent = nullptr);
  ~qMRMLSliceWidget() override;

  /// Get slice controller
  Q_INVOKABLE qMRMLSliceControllerWidget* sliceController()const;

  /// \sa qMRMLSliceControllerWidget::mrmlSliceNode()
  /// \sa setMRMLSliceNode()
  Q_INVOKABLE vtkMRMLSliceNode* mrmlSliceNode()const;

  /// \sa qMRMLSliceControllerWidget::sliceLogic()
  Q_INVOKABLE vtkMRMLSliceLogic* sliceLogic()const;

  /// \sa qMRMLSliceControllerWidget::sliceOrientation()
  /// \sa setSliceOrientation()
  Q_INVOKABLE QString sliceOrientation()const;

  /// \sa qMRMLSliceControllerWidget::imageData()
  /// \sa setImageData();
  Q_INVOKABLE vtkAlgorithmOutput* imageDataConnection()const;

  /// \sa qMRMLSliceControllerWidget::mrmlSliceCompositeNode()
  Q_INVOKABLE vtkMRMLSliceCompositeNode* mrmlSliceCompositeNode()const;

  /// \sa qMRMLSliceControllerWidget::sliceViewName()
  /// \sa setSliceViewName()
  QString sliceViewName()const;

  /// \sa qMRMLSliceControllerWidget::sliceViewName()
  /// \sa sliceViewName()
  void setSliceViewName(const QString& newSliceViewName);

  /// \sa qMRMLSliceControllerWidget::sliceViewLabel()
  /// \sa setSliceViewLabel()
  QString sliceViewLabel()const;

  /// \sa qMRMLSliceControllerWidget::sliceViewLabel()
  /// \sa sliceViewLabel()
  void setSliceViewLabel(const QString& newSliceViewLabel);

  /// \sa qMRMLSliceControllerWidget::sliceViewColor()
  /// \sa setSliceViewColor()
  QColor sliceViewColor()const;

  /// \sa qMRMLSliceControllerWidget::sliceViewColor()
  /// \sa sliceViewColor()
  void setSliceViewColor(const QColor& newSliceViewColor);

  /// Returns the interactor style of the view
  /// A const vtkInteractorObserver pointer is returned as you shouldn't
  /// mess too much with it. If you do, be aware that you are probably
  /// unsynchronizing the view from the nodes/logics.
  Q_INVOKABLE vtkInteractorObserver* interactorStyle()const;

  /// Return the overlay corner annotation of the view
  Q_INVOKABLE vtkCornerAnnotation* overlayCornerAnnotation()const;

  /// propagates the logics to the qMRMLSliceControllerWidget
  Q_INVOKABLE void setSliceLogics(vtkCollection* logics);

  /// Get a reference to the underlying slice view. It is the widget that
  /// renders the view (contains vtkRenderWindow).
  /// \sa sliceController()
  Q_INVOKABLE qMRMLSliceView* sliceView()const;

public slots:
  void setMRMLScene(vtkMRMLScene * newScene) override;

  /// \sa qMRMLSliceControllerWidget::setMRMLSliceNode()
  /// \sa mrmlSliceNode()
  void setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode);

  /// \sa qMRMLSliceControllerWidget::setImageData()
  /// \sa imageData()
  void setImageDataConnection(vtkAlgorithmOutput* newImageDataConnection);

  /// \sa qMRMLSliceControllerWidget::setSliceOrientation()
  /// \sa sliceOrientation()
  void setSliceOrientation(const QString& orientation);

  /// Fit slices to background
  void fitSliceToBackground();

signals:
  /// Signal emitted when editing of a node is requested from within the slice widget
  void nodeAboutToBeEdited(vtkMRMLNode* node);

protected:
  void showEvent(QShowEvent *) override;

  QScopedPointer<qMRMLSliceWidgetPrivate> d_ptr;

  /// Constructor allowing derived class to specify a specialized pimpl.
  ///
  /// \note You are responsible to call init() in the constructor of
  /// derived class. Doing so ensures the derived class is fully
  /// instantiated in case virtual method are called within init() itself.
  qMRMLSliceWidget(qMRMLSliceWidgetPrivate* obj, QWidget* parent);

private:
  Q_DECLARE_PRIVATE(qMRMLSliceWidget);
  Q_DISABLE_COPY(qMRMLSliceWidget);
};

#endif
