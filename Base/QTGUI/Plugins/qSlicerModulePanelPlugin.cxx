#include "qSlicerModulePanelPlugin.h"
#include "qSlicerModulePanel.h"

qSlicerModulePanelPlugin::qSlicerModulePanelPlugin()
{
}

QWidget *qSlicerModulePanelPlugin::createWidget(QWidget *_parent)
{
  qSlicerModulePanel* _widget = new qSlicerModulePanel(_parent);
  return _widget;
}

QString qSlicerModulePanelPlugin::domXml() const
{
  return "<widget class=\"qSlicerModulePanel\" \
          name=\"SlicerModulePanel\">\n"
          "</widget>\n";
}

QString qSlicerModulePanelPlugin::includeFile() const
{
  return "qSlicerModulePanel.h";
}

bool qSlicerModulePanelPlugin::isContainer() const
{
  return false;
}

QString qSlicerModulePanelPlugin::name() const
{
  return "qSlicerModulePanel";
}
