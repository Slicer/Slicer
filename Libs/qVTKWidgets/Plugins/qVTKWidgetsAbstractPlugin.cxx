#include "qVTKWidgetsAbstractPlugin.h"

qVTKWidgetsAbstractPlugin::qVTKWidgetsAbstractPlugin()
{
}

QString qVTKWidgetsAbstractPlugin::group() const
{ 
  return "Slicer [VTK Widgets]";
}

QIcon qVTKWidgetsAbstractPlugin::icon() const
{
  return QIcon(); 
}

QString qVTKWidgetsAbstractPlugin::toolTip() const
{ 
  return QString(); 
}

QString qVTKWidgetsAbstractPlugin::whatsThis() const
{
  return QString(); 
}


