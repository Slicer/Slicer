#include "qMRMLNodeFactoryButtonPlugin.h"

// qMRML includes
#include "qMRMLNodeFactoryButton.h"

// QT includes
#include <QDebug>

// --------------------------------------------------------------------------
qMRMLNodeFactoryButtonPlugin::qMRMLNodeFactoryButtonPlugin(QObject *_parent)
  : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLNodeFactoryButtonPlugin::createWidget(QWidget *_parent)
{
  qMRMLNodeFactoryButton* _widget = new qMRMLNodeFactoryButton(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLNodeFactoryButtonPlugin::domXml() const
{
  return "<widget class=\"qMRMLNodeFactoryButton\" \
          name=\"MRMLNodeFactoryButton\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QString qMRMLNodeFactoryButtonPlugin::includeFile() const
{
  return "qMRMLNodeFactoryButtonPlugin.h";
}

// --------------------------------------------------------------------------
bool qMRMLNodeFactoryButtonPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLNodeFactoryButtonPlugin::name() const
{
  return "qMRMLNodeFactoryButton";
}
