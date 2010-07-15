#ifndef __qMRMLEventLoggerWidget_h
#define __qMRMLEventLoggerWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

class qMRMLEventLoggerWidgetPrivate;
class vtkMRMLScene;
class vtkObject;

class QMRML_WIDGETS_EXPORT qMRMLEventLoggerWidget: public QWidget
{
  Q_OBJECT
public:
  typedef QWidget Superclass;
  explicit qMRMLEventLoggerWidget(QWidget *parent = 0);
  virtual ~qMRMLEventLoggerWidget();

public slots:

  /// 
  /// Set the MRML scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* scene);

protected slots:
  
  virtual void onNodeAddedEvent(vtkObject* caller, vtkObject* call_data);
  virtual void onNodeRemovedEvent(vtkObject* caller, vtkObject* call_data);
  virtual void onNewSceneEvent();
  virtual void onSceneClosedEvent();
  virtual void onSceneAboutToBeClosedEvent();
  virtual void onSceneEditedEvent();
  virtual void onMetadataAddedEvent();
  virtual void onLoadProgressFeedbackEvent();
  virtual void onSaveProgressFeedbackEvent();
  virtual void onSceneLoadStartEvent();
  virtual void onSceneLoadEndEvent();
  virtual void onSceneRestoredEvent();

private:
  CTK_DECLARE_PRIVATE(qMRMLEventLoggerWidget);
};

#endif
