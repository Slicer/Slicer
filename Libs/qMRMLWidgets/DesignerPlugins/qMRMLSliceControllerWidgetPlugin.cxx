#include "qMRMLSliceControllerWidgetPlugin.h"
#include "qMRMLSliceControllerWidget.h"

// --------------------------------------------------------------------------
qMRMLSliceControllerWidgetPlugin::qMRMLSliceControllerWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLSliceControllerWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLSliceControllerWidget* _widget = new qMRMLSliceControllerWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSliceControllerWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLSliceControllerWidget\" \
          name=\"MRMLSliceControllerWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLSliceControllerWidgetPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qMRMLSliceControllerWidgetPlugin::includeFile() const
{
  return "qMRMLSliceControllerWidget.h";
}

// --------------------------------------------------------------------------
bool qMRMLSliceControllerWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSliceControllerWidgetPlugin::name() const
{
  return "qMRMLSliceControllerWidget";
}

