#ifndef __qMRMLSliceWidget_h
#define __qMRMLSliceWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// qMRMLWidget includes
#include "qMRMLWidget.h"

#include "qMRMLWidgetsExport.h"


class ctkVTKSliceView;
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

class QMRML_WIDGETS_EXPORT qMRMLSliceWidget : public qMRMLWidget
{
  Q_OBJECT
  Q_PROPERTY(QString sliceOrientation READ sliceOrientation WRITE setSliceOrientation)
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;
  
  /// Constructors
  explicit qMRMLSliceWidget(QWidget* parent = 0);
  virtual ~qMRMLSliceWidget(){}

  /// Register DisplayableManagers
  /// \a scriptedDisplayableManagerDirectory is the based directory from which
  /// scripted DisplayableManager should be sourced from.
  void registerDisplayableManagers(const QString& scriptedDisplayableManagerDirectory);

  /// Get slice controller
  qMRMLSliceControllerWidget* sliceController()const;

  /// \sa qMRMLSliceControllerWidget::mrmlSliceNode()
  /// \sa setMRMLSliceNode()
  vtkMRMLSliceNode* mrmlSliceNode()const;

  // \sa qMRMLSliceControllerWidget::sliceLogic()
  vtkMRMLSliceLogic* sliceLogic()const;

  /// \sa qMRMLSliceControllerWidget::sliceOrientation()
  /// \sa setSliceOrientation()
  QString sliceOrientation()const;

  /// \sa qMRMLSliceControllerWidget::imageData()
  /// \sa setImageData();
  vtkImageData* imageData()const;

  /// \sa qMRMLSliceControllerWidget::mrmlSliceCompositeNode()
  vtkMRMLSliceCompositeNode* mrmlSliceCompositeNode()const;

  /// \sa qMRMLSliceControllerWidget::sliceViewName()
  /// \sa setSliceViewName()
  QString sliceViewName()const;

  /// \sa qMRMLSliceControllerWidget::sliceViewName()
  /// \sa sliceViewName()
  void setSliceViewName(const QString& newSliceViewName);

  /// Returns the interactor style of the view
  vtkInteractorObserver* interactorStyle()const;

  /// propagates the logics to the qMRMLSliceControllerWidget
  void setSliceLogics(vtkCollection* logics);

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

  /// Get a reference to the underlying Slice View
  ctkVTKSliceView * sliceView()const;

private:
  CTK_DECLARE_PRIVATE(qMRMLSliceWidget);
};

#endif
