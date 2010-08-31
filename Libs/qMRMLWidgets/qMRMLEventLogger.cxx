/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

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

  // Set a high priority, doing so will force the Logger to be first to catch and
  // display the event associated with the scene.
  float priority = 100.0;

  this->EventNameToConnectionIdMap["NodeAdded"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::NodeAddedEvent, p,
    SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)), priority);

  this->EventNameToConnectionIdMap["NodeRemoved"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::NodeRemovedEvent, p,
    SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)), priority);

  this->EventNameToConnectionIdMap["NewScene"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::NewSceneEvent, p, SLOT(onNewSceneEvent()), priority);

  this->EventNameToConnectionIdMap["SceneClosed"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneClosedEvent, p, SLOT(onSceneClosedEvent()), priority);

  this->EventNameToConnectionIdMap["SceneAboutToBeClosed"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneAboutToBeClosedEvent, p, SLOT(onSceneAboutToBeClosedEvent()), priority);

  this->EventNameToConnectionIdMap["SceneEdited"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneEditedEvent, p, SLOT(onSceneEditedEvent()), priority);

  this->EventNameToConnectionIdMap["MetadataAdded"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::MetadataAddedEvent, p, SLOT(onMetadataAddedEvent()), priority);

  this->EventNameToConnectionIdMap["ImportProgressFeedback"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::ImportProgressFeedbackEvent, p, SLOT(onImportProgressFeedbackEvent()), priority);

  this->EventNameToConnectionIdMap["SaveProgressFeedback"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SaveProgressFeedbackEvent, p, SLOT(onSaveProgressFeedbackEvent()), priority);

  this->EventNameToConnectionIdMap["SceneAboutToBeImported"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneAboutToBeImportedEvent, p, SLOT(onSceneAboutToBeImportedEvent()), priority);

  this->EventNameToConnectionIdMap["SceneImported"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneImportedEvent, p, SLOT(onSceneImportedEvent()), priority);

  this->EventNameToConnectionIdMap["SceneRestored"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneRestoredEvent, p, SLOT(onSceneRestoredEvent()), priority);

  this->MRMLScene = scene;
}

//------------------------------------------------------------------------------
// qMRMLEventLogger methods

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
void qMRMLEventLogger::setConsoleOutputEnabled(bool enabled)
{
  if (enabled)
    {
    logger.setInfo();
    }
  else
    {
    logger.setOff();
    }
}

