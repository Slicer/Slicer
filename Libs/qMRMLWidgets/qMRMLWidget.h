#ifndef __qMRMLWidget_h
#define __qMRMLWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

class vtkMRMLScene;
class qMRMLWidgetPrivate;

class QMRML_WIDGETS_EXPORT qMRMLWidget : public QWidget
{
  Q_OBJECT

public:

  typedef QWidget Superclass;
  explicit qMRMLWidget(QWidget *parent=0, Qt::WindowFlags f=0);
  virtual ~qMRMLWidget();

  /// 
  /// Return a pointer on the current MRML scene
  vtkMRMLScene* mrmlScene() const;

public slots:

  /// 
  /// Set the MRML \a scene associated with the widget
  virtual void setMRMLScene(vtkMRMLScene* scene);

signals:
  void mrmlSceneChanged(vtkMRMLScene*);

private:
  CTK_DECLARE_PRIVATE(qMRMLWidget);
};

#endif

