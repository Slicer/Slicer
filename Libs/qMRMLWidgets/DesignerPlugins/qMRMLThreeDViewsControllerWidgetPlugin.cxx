#include "qMRMLThreeDViewsControllerWidgetPlugin.h"
#include "qMRMLThreeDViewsControllerWidget.h"

// --------------------------------------------------------------------------
qMRMLThreeDViewsControllerWidgetPlugin::qMRMLThreeDViewsControllerWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLThreeDViewsControllerWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLThreeDViewsControllerWidget* _widget = new qMRMLThreeDViewsControllerWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLThreeDViewsControllerWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLThreeDViewsControllerWidget\" \
          name=\"MRMLThreeDViewsControllerWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLThreeDViewsControllerWidgetPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qMRMLThreeDViewsControllerWidgetPlugin::includeFile() const
{
  return "qMRMLThreeDViewsControllerWidget.h";
}

// --------------------------------------------------------------------------
bool qMRMLThreeDViewsControllerWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLThreeDViewsControllerWidgetPlugin::name() const
{
  return "qMRMLThreeDViewsControllerWidget";
}

