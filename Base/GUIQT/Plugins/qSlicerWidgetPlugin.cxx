#include "qSlicerWidgetPlugin.h"
#include "qSlicerWidget.h"

qSlicerWidgetPlugin::qSlicerWidgetPlugin(QObject *_parent)
        : QObject(_parent)
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
