#include "qSlicerModuleSelectorWidgetPlugin.h"
#include "qSlicerModuleSelectorWidget.h"

qSlicerModuleSelectorWidgetPlugin::qSlicerModuleSelectorWidgetPlugin()
{
}

QWidget *qSlicerModuleSelectorWidgetPlugin::createWidget(QWidget *_parent)
{
  qSlicerModuleSelectorWidget* _widget = new qSlicerModuleSelectorWidget(_parent);
  return _widget;
}

QString qSlicerModuleSelectorWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerModuleSelectorWidget\" \
          name=\"SlicerModuleSelectorWidget\">\n"
          "</widget>\n";
}

QString qSlicerModuleSelectorWidgetPlugin::includeFile() const
{
  return "qSlicerModuleSelectorWidget.h";
}

bool qSlicerModuleSelectorWidgetPlugin::isContainer() const
{
  return false;
}

QString qSlicerModuleSelectorWidgetPlugin::name() const
{
  return "qSlicerModuleSelectorWidget";
}
