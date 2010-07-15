
// Qt includes
#include <QDebug>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLEventLogger.h"
#include "qMRMLEventLogger_p.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkObject.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLEventLogger");
//--------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLEventLogger::qMRMLEventLogger(QObject* _parent):Superclass(_parent)
{
  logger.setInfo();

  CTK_INIT_PRIVATE(qMRMLEventLogger);
  CTK_D(qMRMLEventLogger);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLEventLogger::~qMRMLEventLogger()
{
}

//------------------------------------------------------------------------------
void qMRMLEventLogger::setMRMLScene(vtkMRMLScene* scene)
{
  ctk_d()->setMRMLScene(scene);
}

//------------------------------------------------------------------------------
// Helper macro allowing to define function of the form 'bool ListeningEVENT_NAME()'
//
#define QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(_EVENT_NAME)   \
bool qMRMLEventLogger::listening##_EVENT_NAME##Event()        \
{                                                             \
  CTK_D(qMRMLEventLogger);                                   \
  return d->EventToListen.contains(#_EVENT_NAME);             \
}

//------------------------------------------------------------------------------
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(NodeAdded);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(NodeRemoved);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(NewScene);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneClosed);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneAboutToBeClosed);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneEdited);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(MetadataAdded);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(ImportProgressFeedback);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(SaveProgressFeedback);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneAboutToBeImported);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneImported);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneRestored);

//------------------------------------------------------------------------------
// Unregister helper macro
#undef QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO

//------------------------------------------------------------------------------
// Helper macro allowing to define function of the
// form void listenEVENT_NAMEEvent(bool listen)'
//
#define QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(_EVENT_NAME)            \
void qMRMLEventLogger::listen##_EVENT_NAME##Event(bool listen)      \
{                                                                   \
  CTK_D(qMRMLEventLogger);                                         \
                                                                    \
  Q_ASSERT(!d->EventNameToConnectionIdMap.contains(#_EVENT_NAME));  \
  QString cid = d->EventNameToConnectionIdMap[#_EVENT_NAME];        \
                                                                    \
  if (listen && !d->EventToListen.contains(#_EVENT_NAME))           \
    {                                                               \
    d->EventToListen << #_EVENT_NAME;                               \
    d->qvtkBlock(cid, false);                                       \
    }                                                               \
  if (!listen)                                                      \
    {                                                               \
    d->EventToListen.removeOne(#_EVENT_NAME);                       \
    d->qvtkBlock(cid, true);                                        \
    }                                                               \
}

//------------------------------------------------------------------------------
QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(NodeAdded);
QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(NodeRemoved);
QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(NewScene);
QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(SceneClosed);
QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(SceneAboutToBeClosed);
QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(SceneEdited);
QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(MetadataAdded);
QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(ImportProgressFeedback);
QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(SaveProgressFeedback);
QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(SceneAboutToBeImported);
QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(SceneImported);
QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO(SceneRestored);

//------------------------------------------------------------------------------
// Unregister helper macro
#undef QMRMLEVENTLOGGER_LISTEN_EVENT_MACRO

//------------------------------------------------------------------------------
void qMRMLEventLogger::onNodeAddedEvent(vtkObject* caller, vtkObject* call_data)
{
  logger.info(QString("onNodeAddedEvent: %1").arg(call_data->GetClassName()));
  emit this->signalNodeAddedEvent(caller, call_data);
}

//------------------------------------------------------------------------------
void qMRMLEventLogger::onNodeRemovedEvent(vtkObject* caller, vtkObject* call_data)
{
  logger.info(QString("onNodeRemovedEvent: %1").arg(call_data->GetClassName()));
  emit this->signalNodeRemovedEvent(caller, call_data);
}

//------------------------------------------------------------------------------
// Helper macro allowing to define function of the
// form void onEVENT_NAMEEvent()'
//
#define QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(_EVENT_NAME)  \
void qMRMLEventLogger::on##_EVENT_NAME##Event()           \
{                                                         \
  logger.info(QString("on%1Event").arg(#_EVENT_NAME));    \
  emit signal##_EVENT_NAME##Event();                      \
}

QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(NewScene);
QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneClosed);
QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneAboutToBeClosed);
QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneEdited);
QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(MetadataAdded);
QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(ImportProgressFeedback);
QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SaveProgressFeedback);
QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneAboutToBeImported);
QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneImported);
QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneRestored);

//------------------------------------------------------------------------------
// Unregister helper macro
#undef QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO

//------------------------------------------------------------------------------
// qMRMLEventLoggerPrivate methods

//------------------------------------------------------------------------------
void qMRMLEventLoggerPrivate::init()
{
  CTK_P(qMRMLEventLogger);
  p->listenNodeAddedEvent(true);
  p->listenNodeRemovedEvent(true);
  p->listenNewSceneEvent(true);
  p->listenSceneClosedEvent(true);
  p->listenSceneAboutToBeClosedEvent(true);
  p->listenSceneEditedEvent(true);
  p->listenMetadataAddedEvent(true);
  p->listenImportProgressFeedbackEvent(true);
  p->listenSaveProgressFeedbackEvent(true);
  p->listenSceneAboutToBeImportedEvent(true);
  p->listenSceneImportedEvent(true);
  p->listenSceneRestoredEvent(true);
}

//------------------------------------------------------------------------------
qMRMLEventLoggerPrivate::qMRMLEventLoggerPrivate():Superclass()
{
  this->MRMLScene = 0;
}
    
//------------------------------------------------------------------------------
void qMRMLEventLoggerPrivate::setMRMLScene(vtkMRMLScene* scene)
{
  CTK_P(qMRMLEventLogger);
  
  if (scene == this->MRMLScene)
    {
    return; 
    }
    
  QString cid; // connectionId

  this->EventNameToConnectionIdMap["NodeAdded"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::NodeAddedEvent, p,
    SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
                      
  this->EventNameToConnectionIdMap["NodeRemoved"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::NodeRemovedEvent, p,
    SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));

  this->EventNameToConnectionIdMap["NewScene"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::NewSceneEvent, p, SLOT(onNewSceneEvent()));

  this->EventNameToConnectionIdMap["SceneClosed"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneClosedEvent, p, SLOT(onSceneClosedEvent()));

  this->EventNameToConnectionIdMap["SceneAboutToBeClosed"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneAboutToBeClosedEvent, p, SLOT(onSceneAboutToBeClosedEvent()));

  this->EventNameToConnectionIdMap["SceneEdited"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneEditedEvent, p, SLOT(onSceneEditedEvent()));

  this->EventNameToConnectionIdMap["MetadataAdded"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::MetadataAddedEvent, p, SLOT(onMetadataAddedEvent()));

  this->EventNameToConnectionIdMap["ImportProgressFeedback"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::ImportProgressFeedbackEvent, p, SLOT(onImportProgressFeedbackEvent()));
                      
  this->EventNameToConnectionIdMap["SaveProgressFeedback"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SaveProgressFeedbackEvent, p, SLOT(onSaveProgressFeedbackEvent()));

  this->EventNameToConnectionIdMap["SceneAboutToBeImported"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneAboutToBeImportedEvent, p, SLOT(onSceneAboutToBeImportedEvent()));

  this->EventNameToConnectionIdMap["SceneImported"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneImportedEvent, p, SLOT(onSceneImportedEvent()));

  this->EventNameToConnectionIdMap["SceneRestored"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneRestoredEvent, p, SLOT(onSceneRestoredEvent()));

  this->MRMLScene = scene; 
}
