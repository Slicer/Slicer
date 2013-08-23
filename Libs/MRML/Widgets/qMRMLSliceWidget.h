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

class vtkImageData;
class vtkInteractorObserver;
class vtkCornerAnnotation;
class vtkCollection;
class QMRML_WIDGETS_EXPORT qMRMLSliceWidget : public qMRMLWidget
{
  Q_OBJECT
  Q_PROPERTY(QString sliceOrientation READ sliceOrientation WRITE setSliceOrientation)
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLSliceWidget(QWidget* parent = 0);
  virtual ~qMRMLSliceWidget();

  /// Get slice controller
  Q_INVOKABLE qMRMLSliceControllerWidget* sliceController()const;

  /// \sa qMRMLSliceControllerWidget::mrmlSliceNode()
  /// \sa setMRMLSliceNode()
  Q_INVOKABLE vtkMRMLSliceNode* mrmlSliceNode()const;

  // \sa qMRMLSliceControllerWidget::sliceLogic()
  Q_INVOKABLE vtkMRMLSliceLogic* sliceLogic()const;

  /// \sa qMRMLSliceControllerWidget::sliceOrientation()
  /// \sa setSliceOrientation()
  QString sliceOrientation()const;

  /// \sa qMRMLSliceControllerWidget::imageData()
  /// \sa setImageData();
  Q_INVOKABLE vtkImageData* imageData()const;

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
  vtkCornerAnnotation* overlayCornerAnnotation()const;

  /// propagates the logics to the qMRMLSliceControllerWidget
  void setSliceLogics(vtkCollection* logics);

  /// Get a reference to the underlying Slice View
  /// A const ctkVTKSliceView pointer is returned as you shouldn't
  /// mess too much with it. If you do, be aware that you are probably
  /// unsynchronizing the view from the nodes/logics.
  Q_INVOKABLE const qMRMLSliceView* sliceView()const;

  //virtual bool eventFilter(QObject* object, QEvent* event);
public slots:

  void setMRMLScene(vtkMRMLScene * newScene);

  /// \sa qMRMLSliceControllerWidget::setMRMLSliceNode()
  /// \sa mrmlSliceNode()
  void setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode);

  /// \sa qMRMLSliceControllerWidget::setImageData()
  /// \sa imageData()
  void setImageData(vtkImageData* newImageData);
  
  /// \sa qMRMLSliceControllerWidget::setSliceOrientation()
  /// \sa sliceOrientation()
  void setSliceOrientation(const QString& orienation);

  /// Fit slices to background
  void fitSliceToBackground();

protected:
  QScopedPointer<qMRMLSliceWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSliceWidget);
  Q_DISABLE_COPY(qMRMLSliceWidget);
};

#endif
