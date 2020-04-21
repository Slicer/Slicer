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

// qMRML includes
#include "qMRMLEventLogger_p.h"

// MRML includes
#include <vtkMRMLScene.h>

//------------------------------------------------------------------------------
// qMRMLEventLoggerPrivate methods

//------------------------------------------------------------------------------
void qMRMLEventLoggerPrivate::init()
{
  Q_Q(qMRMLEventLogger);
  q->listenNodeAddedEvent(true);
  q->listenNodeRemovedEvent(true);
  q->listenNewSceneEvent(true);
  q->listenSceneClosedEvent(true);
  q->listenSceneAboutToBeClosedEvent(true);
  q->listenMetadataAddedEvent(true);
  q->listenImportProgressFeedbackEvent(true);
  q->listenSaveProgressFeedbackEvent(true);
  q->listenSceneAboutToBeImportedEvent(true);
  q->listenSceneImportedEvent(true);
  q->listenSceneRestoredEvent(true);
}

//------------------------------------------------------------------------------
qMRMLEventLoggerPrivate::qMRMLEventLoggerPrivate(qMRMLEventLogger& object)
  : q_ptr(&object)
{
  this->MRMLScene = nullptr;
  this->ConsoleOutputEnabled = true;
}

//------------------------------------------------------------------------------
void qMRMLEventLoggerPrivate::setMRMLScene(vtkMRMLScene* scene)
{
  Q_Q(qMRMLEventLogger);

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
    vtkMRMLScene::NodeAddedEvent, q,
    SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)), priority);

  this->EventNameToConnectionIdMap["NodeRemoved"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::NodeRemovedEvent, q,
    SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)), priority);

  this->EventNameToConnectionIdMap["NewScene"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::NewSceneEvent, q, SLOT(onNewSceneEvent()), priority);

  this->EventNameToConnectionIdMap["SceneClosed"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::EndCloseEvent, q, SLOT(onSceneClosedEvent()), priority);

  this->EventNameToConnectionIdMap["SceneAboutToBeClosed"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::StartCloseEvent, q, SLOT(onSceneAboutToBeClosedEvent()), priority);

  this->EventNameToConnectionIdMap["MetadataAdded"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::MetadataAddedEvent, q, SLOT(onMetadataAddedEvent()), priority);

  this->EventNameToConnectionIdMap["ImportProgressFeedback"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::ImportProgressFeedbackEvent, q, SLOT(onImportProgressFeedbackEvent()), priority);

  this->EventNameToConnectionIdMap["SaveProgressFeedback"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SaveProgressFeedbackEvent, q, SLOT(onSaveProgressFeedbackEvent()), priority);

  this->EventNameToConnectionIdMap["SceneAboutToBeImported"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::StartImportEvent, q, SLOT(onSceneAboutToBeImportedEvent()), priority);

  this->EventNameToConnectionIdMap["SceneImported"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::EndImportEvent, q, SLOT(onSceneImportedEvent()), priority);

  this->EventNameToConnectionIdMap["SceneRestored"] = this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::EndRestoreEvent, q, SLOT(onSceneRestoredEvent()), priority);

  this->MRMLScene = scene;
}

//------------------------------------------------------------------------------
// qMRMLEventLogger methods

//------------------------------------------------------------------------------
qMRMLEventLogger::qMRMLEventLogger(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLEventLoggerPrivate(*this))
{
  Q_D(qMRMLEventLogger);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLEventLogger::~qMRMLEventLogger() = default;

//------------------------------------------------------------------------------
void qMRMLEventLogger::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLEventLogger);
  d->setMRMLScene(scene);
}

//------------------------------------------------------------------------------
// Helper macro allowing to define function of the form 'bool ListeningEVENT_NAME()'
//
#define QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(_EVENT_NAME)   \
bool qMRMLEventLogger::listening##_EVENT_NAME##Event()        \
{                                                             \
  Q_D(qMRMLEventLogger);                                   \
  return d->EventToListen.contains(#_EVENT_NAME);             \
}

//------------------------------------------------------------------------------
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(NodeAdded);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(NodeRemoved);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(NewScene);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneClosed);
QMRMLEVENTLOGGER_LISTENING_EVENT_MACRO(SceneAboutToBeClosed);
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
  Q_D(qMRMLEventLogger);                                         \
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
  Q_D(qMRMLEventLogger);
  if (d->ConsoleOutputEnabled)
    {
    std::cout << qPrintable(QString("onNodeAddedEvent: %1").arg(call_data->GetClassName())) << std::endl;
    }
  emit this->signalNodeAddedEvent(caller, call_data);
}

//------------------------------------------------------------------------------
void qMRMLEventLogger::onNodeRemovedEvent(vtkObject* caller, vtkObject* call_data)
{
  Q_D(qMRMLEventLogger);
  if (d->ConsoleOutputEnabled)
    {
    std::cout << qPrintable(QString("onNodeRemovedEvent: %1").arg(call_data->GetClassName())) << std::endl;
    }
  emit this->signalNodeRemovedEvent(caller, call_data);
}

//------------------------------------------------------------------------------
// Helper macro allowing to define function of the
// form void onEVENT_NAMEEvent()'
//
#define QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(_EVENT_NAME)    \
void qMRMLEventLogger::on##_EVENT_NAME##Event()             \
{                                                           \
  Q_D(qMRMLEventLogger);                                    \
  if (d->ConsoleOutputEnabled)                              \
    {                                                       \
    std::cout << qPrintable(                                \
      QString("qMRMLEventLogger::on%1Event").               \
        arg(#_EVENT_NAME)) << std::endl;                    \
    }                                                       \
  emit signal##_EVENT_NAME##Event();                        \
}

QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(NewScene);
QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneClosed);
QMRMLEVENTLOGGER_ONEVENT_SLOT_MACRO(SceneAboutToBeClosed);
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
  Q_D(qMRMLEventLogger);
  d->ConsoleOutputEnabled = enabled;
}

