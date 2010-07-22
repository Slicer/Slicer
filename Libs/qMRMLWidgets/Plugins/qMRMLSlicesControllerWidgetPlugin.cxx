#include "qMRMLSlicesControllerWidgetPlugin.h"
#include "qMRMLSlicesControllerWidget.h"

// --------------------------------------------------------------------------
qMRMLSlicesControllerWidgetPlugin::qMRMLSlicesControllerWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLSlicesControllerWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLSlicesControllerWidget* _widget = new qMRMLSlicesControllerWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSlicesControllerWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLSlicesControllerWidget\" \
          name=\"MRMLSlicesControllerWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLSlicesControllerWidgetPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qMRMLSlicesControllerWidgetPlugin::includeFile() const
{
  return "qMRMLSlicesControllerWidget.h";
}

// --------------------------------------------------------------------------
bool qMRMLSlicesControllerWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSlicesControllerWidgetPlugin::name() const
{
  return "qMRMLSlicesControllerWidget";
}

