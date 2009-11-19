#include "qMRMLWidgetsAbstractPlugin.h"

qMRMLWidgetsAbstractPlugin::qMRMLWidgetsAbstractPlugin()
{
}

QString qMRMLWidgetsAbstractPlugin::group() const 
{ 
  return "Slicer [MRML Widgets]"; 
}

QIcon qMRMLWidgetsAbstractPlugin::icon() const
{
  return QIcon(); 
}

QString qMRMLWidgetsAbstractPlugin::toolTip() const 
{ 
  return QString(); 
}

QString qMRMLWidgetsAbstractPlugin::whatsThis() const 
{
  return QString(); 
}


