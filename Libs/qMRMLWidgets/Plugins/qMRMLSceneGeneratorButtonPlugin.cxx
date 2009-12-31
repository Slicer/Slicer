#include "qMRMLSceneGeneratorButtonPlugin.h"
#include "qMRMLSceneGeneratorButton.h"

// qMRML includes
#include "qMRMLNodeFactory.h"

// QT includes
#include <QDebug>

// --------------------------------------------------------------------------
qMRMLSceneGeneratorButtonPlugin::qMRMLSceneGeneratorButtonPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLSceneGeneratorButtonPlugin::createWidget(QWidget *_parent)
{
  qMRMLSceneGeneratorButton* _widget = new qMRMLSceneGeneratorButton(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSceneGeneratorButtonPlugin::domXml() const
{
  return "<widget class=\"qMRMLSceneGeneratorButton\" \
          name=\"MRMLSceneGeneratorButton\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QString qMRMLSceneGeneratorButtonPlugin::includeFile() const
{
  return "qMRMLSceneGeneratorButtonPlugin.h";
}

// --------------------------------------------------------------------------
bool qMRMLSceneGeneratorButtonPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSceneGeneratorButtonPlugin::name() const
{
  return "qMRMLSceneGeneratorButton";
}
