#include "qSlicerQTGUIAbstractPlugin.h"

qSlicerQTGUIAbstractPlugin::qSlicerQTGUIAbstractPlugin()
{
}

QString qSlicerQTGUIAbstractPlugin::group() const 
{ 
  return "Slicer [GUI Widgets]"; 
}

QIcon qSlicerQTGUIAbstractPlugin::icon() const
{
  return QIcon(); 
}

QString qSlicerQTGUIAbstractPlugin::toolTip() const 
{ 
  return QString(); 
}

QString qSlicerQTGUIAbstractPlugin::whatsThis() const 
{
  return QString(); 
}


