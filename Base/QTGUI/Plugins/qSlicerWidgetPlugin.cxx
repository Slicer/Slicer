#include "qSlicerWidgetPlugin.h"
#include "qSlicerWidget.h"

qSlicerWidgetPlugin::qSlicerWidgetPlugin()
{
}

QWidget *qSlicerWidgetPlugin::createWidget(QWidget *_parent)
{
  qSlicerWidget* _widget = new qSlicerWidget(_parent);
  return _widget;
}

QString qSlicerWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerWidget\" \
          name=\"SlicerWidget\">\n"
          "</widget>\n";
}

QString qSlicerWidgetPlugin::includeFile() const
{
  return "qSlicerWidget.h";
}

bool qSlicerWidgetPlugin::isContainer() const
{
  return true;
}

QString qSlicerWidgetPlugin::name() const
{
  return "qSlicerWidget";
}
