#include "qMRMLSceneFactoryWidgetPlugin.h"
#include "qMRMLSceneFactoryWidget.h"

// qMRML includes
#include "qMRMLNodeFactory.h"

// QT includes
#include <QDebug>

// --------------------------------------------------------------------------
qMRMLSceneFactoryWidgetPlugin::qMRMLSceneFactoryWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLSceneFactoryWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLSceneFactoryWidget* _widget = new qMRMLSceneFactoryWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSceneFactoryWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLSceneFactoryWidget\" \
          name=\"MRMLSceneFactoryWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QString qMRMLSceneFactoryWidgetPlugin::includeFile() const
{
  return "qMRMLSceneFactoryWidgetPlugin.h";
}

// --------------------------------------------------------------------------
bool qMRMLSceneFactoryWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSceneFactoryWidgetPlugin::name() const
{
  return "qMRMLSceneFactoryWidget";
}
