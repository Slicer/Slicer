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

// Qt includes

// qMRML includes
#include "qMRMLEventLoggerWidget.h"
#include "ui_qMRMLEventLoggerWidget.h"
#include "qMRMLEventLogger.h"

// MRML includes
#include <vtkMRMLNode.h>

// VTK includes

//-----------------------------------------------------------------------------
class qMRMLEventLoggerWidgetPrivate: public Ui_qMRMLEventLoggerWidget
{
public:
  void log(const QString& text);
  void log(const char* text);

  qMRMLEventLogger* MRMLEventLogger;
};

//------------------------------------------------------------------------------
// Helper macro allowing to connect signal from qMRMLEventLogger with the corresponding
// widget.
//
#define QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(_EVENT_NAME)               \
  QObject::connect(d->MRMLEventLogger, SIGNAL(signal##_EVENT_NAME##Event()), \
    this, SLOT(on##_EVENT_NAME##Event()));

//------------------------------------------------------------------------------
qMRMLEventLoggerWidget::qMRMLEventLoggerWidget(QWidget *_parent):Superclass(_parent)
  , d_ptr(new qMRMLEventLoggerWidgetPrivate)
{
  Q_D(qMRMLEventLoggerWidget);
  d->setupUi(this);
  d->MRMLEventLogger = new qMRMLEventLogger(this);

  this->connect(d->MRMLEventLogger,
                SIGNAL(signalNodeAddedEvent(vtkObject*,vtkObject*)),
                SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));

  this->connect(d->MRMLEventLogger,
                SIGNAL(signalNodeRemovedEvent(vtkObject*,vtkObject*)),
                SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)));

  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(NewScene);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneClosed);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneAboutToBeClosed);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(MetadataAdded);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(ImportProgressFeedback);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SaveProgressFeedback);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneAboutToBeImported);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneImported);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneRestored);
}

//------------------------------------------------------------------------------
// Unregister helper macro
#undef QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO

//------------------------------------------------------------------------------
qMRMLEventLoggerWidget::~qMRMLEventLoggerWidget() = default;

//------------------------------------------------------------------------------
void qMRMLEventLoggerWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLEventLoggerWidget);
  d->MRMLEventLogger->setMRMLScene(scene);
}

//------------------------------------------------------------------------------
void qMRMLEventLoggerWidget::setConsoleOutputEnabled(bool enabled)
{
  Q_D(qMRMLEventLoggerWidget);
  d->MRMLEventLogger->setConsoleOutputEnabled(enabled);
}

//------------------------------------------------------------------------------
void qMRMLEventLoggerWidget::onNodeAddedEvent(vtkObject* caller,
                                              vtkObject* call_data)
{
  Q_UNUSED(caller);
  Q_D(qMRMLEventLoggerWidget);
  vtkMRMLNode * node = vtkMRMLNode::SafeDownCast(call_data);
  Q_ASSERT(node);
  d->log(QString("NodeAdded: %1").arg(node->GetClassName()));
}

//------------------------------------------------------------------------------
void qMRMLEventLoggerWidget::onNodeRemovedEvent(vtkObject* caller,
                                                vtkObject* call_data)
{
  Q_UNUSED(caller);
  Q_D(qMRMLEventLoggerWidget);
  vtkMRMLNode * node = vtkMRMLNode::SafeDownCast(call_data);
  Q_ASSERT(node);
  d->log(QString("NodeRemoved: %1").arg(node->GetClassName()));
}

//------------------------------------------------------------------------------
// Helper macro allowing to define function of the
// form void listenEVENT_NAMEEvent(bool listen)'
//
#define QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(_EVENT_NAME) \
void qMRMLEventLoggerWidget::on##_EVENT_NAME##Event()          \
{                                                              \
  Q_D(qMRMLEventLoggerWidget);                              \
  d->log(#_EVENT_NAME);                                        \
}

//------------------------------------------------------------------------------
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(NewScene);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneClosed);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneAboutToBeClosed);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(MetadataAdded);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(ImportProgressFeedback);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SaveProgressFeedback);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneAboutToBeImported);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneImported);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneRestored);

//------------------------------------------------------------------------------
// Unregister helper macro
#undef QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO

//------------------------------------------------------------------------------
// qMRMLEventLoggerWidgetPrivate methods

//------------------------------------------------------------------------------
void qMRMLEventLoggerWidgetPrivate::log(const char* text)
{
  this->log(QString::fromUtf8(text));
}

//------------------------------------------------------------------------------
void qMRMLEventLoggerWidgetPrivate::log(const QString& text)
{
  this->TextEdit->append(text);
}
