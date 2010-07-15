#ifndef __qMRMLEventLogger_h
#define __qMRMLEventLogger_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

class qMRMLEventLoggerPrivate;
class vtkMRMLScene;
class vtkObject; 

class QMRML_WIDGETS_EXPORT qMRMLEventLogger: public QObject
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  explicit qMRMLEventLogger(QObject* parent = 0);
  virtual ~qMRMLEventLogger();

  /// 
  /// Set the MRML \a scene that should be listened for events
  void setMRMLScene(vtkMRMLScene* scene);

  /// 
  /// Return true if the corresponding event if listened by the eventLogger
  bool listeningNodeAddedEvent();
  bool listeningNodeRemovedEvent();
  bool listeningNewSceneEvent();
  bool listeningSceneClosedEvent();
  bool listeningSceneAboutToBeClosedEvent();
  bool listeningSceneEditedEvent();
  bool listeningMetadataAddedEvent();
  bool listeningLoadProgressFeedbackEvent();
  bool listeningSaveProgressFeedbackEvent();
  bool listeningSceneLoadStartEvent();
  bool listeningSceneLoadEndEvent();
  bool listeningSceneRestoredEvent();

public slots:
  /// 
  /// Allow to enable or disable the listening of specific event
  void listenNodeAddedEvent(bool listen);
  void listenNodeRemovedEvent(bool listen);
  void listenNewSceneEvent(bool listen);
  void listenSceneClosedEvent(bool listen);
  void listenSceneAboutToBeClosedEvent(bool listen);
  void listenSceneEditedEvent(bool listen);
  void listenMetadataAddedEvent(bool listen);
  void listenLoadProgressFeedbackEvent(bool listen);
  void listenSaveProgressFeedbackEvent(bool listen);
  void listenSceneLoadStartEvent(bool listen);
  void listenSceneLoadEndEvent(bool listen);
  void listenSceneRestoredEvent(bool listen);

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

signals:
  /// 
  /// Emitted when the associated MRML scene event is fired
  void signalNodeAddedEvent(vtkObject* calle, vtkObject* call_data);
  void signalNodeRemovedEvent(vtkObject* caller, vtkObject* call_data);
  void signalNewSceneEvent();
  void signalSceneClosedEvent();
  void signalSceneAboutToBeClosedEvent();
  void signalSceneEditedEvent();
  void signalMetadataAddedEvent();
  void signalLoadProgressFeedbackEvent();
  void signalSaveProgressFeedbackEvent();
  void signalSceneLoadStartEvent();
  void signalSceneLoadEndEvent();
  void signalSceneRestoredEvent();

private:
  CTK_DECLARE_PRIVATE(qMRMLEventLogger);
};

#endif
