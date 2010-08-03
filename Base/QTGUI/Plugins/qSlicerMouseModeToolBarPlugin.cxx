#include "qSlicerMouseModeToolBarPlugin.h"
#include "qSlicerMouseModeToolBar.h"

qSlicerMouseModeToolBarPlugin::qSlicerMouseModeToolBarPlugin()
{
}

QWidget *qSlicerMouseModeToolBarPlugin::createWidget(QWidget *_parent)
{
  qSlicerMouseModeToolBar* _widget = new qSlicerMouseModeToolBar(_parent);
  return _widget;
}

QString qSlicerMouseModeToolBarPlugin::domXml() const
{
  return "<widget class=\"qSlicerMouseModeToolBar\" \
          name=\"SlicerMouseModeToolBar\">\n"
          "</widget>\n";
}

QString qSlicerMouseModeToolBarPlugin::includeFile() const
{
  return "qSlicerMouseModeToolBar.h";
}

bool qSlicerMouseModeToolBarPlugin::isContainer() const
{
  return false;
}

QString qSlicerMouseModeToolBarPlugin::name() const
{
  return "qSlicerMouseModeToolBar";
}
