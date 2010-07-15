

// Qt includes
#include <QLatin1String>

// qMRML includes
#include "qMRMLEventLoggerWidget.h"
#include "ui_qMRMLEventLoggerWidget.h"
#include "qMRMLEventLogger.h"

// MRML includes
#include <vtkMRMLNode.h>

// VTK includes
#include <vtkObject.h>

//-----------------------------------------------------------------------------
class qMRMLEventLoggerWidgetPrivate: public ctkPrivate<qMRMLEventLoggerWidget>,
                                     public Ui_qMRMLEventLoggerWidget
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
{
  CTK_INIT_PRIVATE(qMRMLEventLoggerWidget);
  CTK_D(qMRMLEventLoggerWidget);
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
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneClosing);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneEdited);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(MetadataAdded);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(LoadProgressFeedback);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SaveProgressFeedback);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneLoadStart);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneLoadEnd);
  QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO(SceneRestored);
}

//------------------------------------------------------------------------------
// Unregister helper macro
#undef QMRMLEVENTLOGGERWIDGET_CONNECT_SLOT_MACRO

//------------------------------------------------------------------------------
qMRMLEventLoggerWidget::~qMRMLEventLoggerWidget()
{
}

//------------------------------------------------------------------------------
void qMRMLEventLoggerWidget::setMRMLScene(vtkMRMLScene* scene)
{
  CTK_D(qMRMLEventLoggerWidget);
  d->MRMLEventLogger->setMRMLScene(scene);
}

//------------------------------------------------------------------------------
void qMRMLEventLoggerWidget::onNodeAddedEvent(vtkObject* caller,
                                              vtkObject* call_data)
{
  Q_UNUSED(caller);
  CTK_D(qMRMLEventLoggerWidget);
  vtkMRMLNode * node = vtkMRMLNode::SafeDownCast(call_data);
  Q_ASSERT(node);
  d->log(QString("NodeAdded: %1").arg(node->GetClassName()));
}

//------------------------------------------------------------------------------
void qMRMLEventLoggerWidget::onNodeRemovedEvent(vtkObject* caller,
                                                vtkObject* call_data)
{
  Q_UNUSED(caller);
  CTK_D(qMRMLEventLoggerWidget);
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
  CTK_D(qMRMLEventLoggerWidget);                              \
  d->log(#_EVENT_NAME);                                        \
}

//------------------------------------------------------------------------------
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(NewScene);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneClosed);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneClosing);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneEdited);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(MetadataAdded);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(LoadProgressFeedback);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SaveProgressFeedback);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneLoadStart);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneLoadEnd);
QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO(SceneRestored);

//------------------------------------------------------------------------------
// Unregister helper macro
#undef QMRMLEVENTLOGGERWIDGET_ONEVENT_SLOT_MACRO

//------------------------------------------------------------------------------
// qMRMLEventLoggerWidgetPrivate methods

//------------------------------------------------------------------------------
void qMRMLEventLoggerWidgetPrivate::log(const char* text)
{
  this->log(QLatin1String(text));
}

//------------------------------------------------------------------------------
void qMRMLEventLoggerWidgetPrivate::log(const QString& text)
{
  this->TextEdit->append(text);
}
